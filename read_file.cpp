#include <fstream>
#include <iostream>
#include <sys/inotify.h>
#include <unistd.h>

using namespace std;

constexpr size_t EVENT_SIZE = sizeof(inotify_event);
constexpr size_t EVENT_BUF_LEN = 1024 * (EVENT_SIZE + 16);
constexpr const char* FILE_TO_READ = "/tmp/somefile";

void check_error(const int descriptor, const string& msg)
{
    if (descriptor < 0) {
        perror(msg.c_str());
        throw runtime_error(msg);
    }
}

class Inotify {
public:
    class iterator {
        using value_type = inotify_event;
        using pointer = value_type*;

    public:
        iterator(const char* _buffer, const size_t& _i, const size_t& _length)
            : buffer(_buffer)
            , i(_i)
            , length(_length)
            , value_ptr((inotify_event*)&buffer[i])
        {
        }

        iterator(const iterator&) = delete;

        iterator& operator=(const iterator&) = delete;

        iterator(iterator&& it)
        {
            *this = std::move(it);
        }

        iterator& operator=(iterator&& it)
        {
            if (this != &it) {
                using std::move;
                buffer = move(it.buffer);
                i = move(it.i);
                length = move(it.length);
                value_ptr = move(it.value_ptr);
            }
            return *this;
        }

        pointer operator->()
        {
            return value_ptr;
        }

        bool operator!=(const iterator& it)
        {
            return buffer != it.buffer || i != it.i;
        }

        void operator++()
        {
            i = min(i + EVENT_SIZE + value_ptr->len, length);
            value_ptr = (inotify_event*)&buffer[i];
        }

    private:
        const char* buffer = nullptr;
        size_t i;
        size_t length;
        pointer value_ptr = nullptr;
    };

    Inotify(const string& file, const uint32_t& mask)
        : fd(inotify_init())
        , wd(inotify_add_watch(fd, file.c_str(), mask))
    {
        check_error(fd, "inotify_init");
        check_error(wd, "inotify_add_watch");
    }

    iterator wait()
    {
        length = read(fd, buffer, EVENT_BUF_LEN);
        check_error(length, "read");
        return iterator(buffer, 0, length);
    }

    iterator begin() const = delete;

    iterator end() const
    {
        return iterator(buffer, length, length);
    }

    ~Inotify()
    {
        inotify_rm_watch(fd, wd);
        close(fd);
    }

private:
    int fd;
    int wd;
    char buffer[EVENT_BUF_LEN] = { 0 };
    int length;
};

int main()
{
    try {
        ifstream ifs(FILE_TO_READ, ofstream::in);
        check_error(ifs.is_open() ? 0 : -1, (string("error while opening file ") + FILE_TO_READ));
        string line;
        while (getline(ifs, line)) {
            cout << "Read: " << line << endl;
        }
        Inotify inotify(FILE_TO_READ, IN_MODIFY | IN_CLOSE | IN_OPEN);
        bool closed = false;
        while (!closed) {
            for (auto it = inotify.wait(); it != inotify.end(); ++it) {
                if (it->mask & IN_OPEN) {
                    cerr << "read_file should be opened during or after file is modified, not before" << endl;
                    closed = true;
                    break;
                } else if (it->mask & IN_MODIFY) {
                    while (getline(ifs, line)) {
                        cout << "Read: " << line << endl;
                    }
                    ifs.clear();
                } else if (it->mask & IN_CLOSE) {
                    closed = true;
                    break;
                } else {
                    if (it->mask & IN_ISDIR) {
                        cout << "Directory " << it->name << " had uncatched event.\n";
                    } else {
                        cout << "File " << it->name << " had uncatched event.\n";
                    }
                }
            }
            check_error(ifs.bad() ? -1 : 0, (string("error while reading file ") + FILE_TO_READ));
        }
        ifs.close();
    } catch (...) {
        return -1;
    }
    return 0;
}
