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

int main()
{
    try {
        ifstream ifs(FILE_TO_READ, ofstream::in);
        check_error(ifs.is_open() ? 0 : -1, (string("error while opening file ") + FILE_TO_READ));
        string line;
        while (getline(ifs, line)) {
            cout << "Read: " << line << endl;
        }
        auto fd = inotify_init();
        check_error(fd, "inotify_init");
        auto wd = inotify_add_watch(fd, FILE_TO_READ, IN_MODIFY | IN_CLOSE | IN_OPEN);
        check_error(wd, "inotify_add_watch");
        bool closed = false;
        while (!closed) {
            char buffer[EVENT_BUF_LEN];
            auto length = read(fd, buffer, EVENT_BUF_LEN);
            check_error(length, "read");
            int i = 0;
            while (i < length) {
                auto* event = (struct inotify_event*)&buffer[i];
                if (event->mask & IN_OPEN) {
                    cerr << "read_file should be opened during or after file is modified, not before" << endl;
                    closed = true;
                } else if (event->mask & IN_MODIFY) {
                    while (getline(ifs, line)) {
                        cout << "Read: " << line << endl;
                    }
                    ifs.clear();
                } else if (event->mask & IN_CLOSE) {
                    closed = true;
                } else {
                    if (event->mask & IN_ISDIR) {
                        cout << "Directory " << event->name << " had uncatched event.\n";
                    } else {
                        cout << "File " << event->name << " had uncatched event.\n";
                    }
                }
                i += EVENT_SIZE + event->len;
            }
            check_error(ifs.bad() ? -1 : 0, (string("error while reading file ") + FILE_TO_READ));
        }
        inotify_rm_watch(fd, wd);
        close(fd);
        ifs.close();
    } catch (...) {
        return -1;
    }
    return 0;
}
