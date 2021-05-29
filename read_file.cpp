#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

using namespace std;

constexpr const char* FILE_TO_READ = "/tmp/somefile";

int main()
{
    ifstream ifs(FILE_TO_READ, ofstream::in);
    if (!ifs.is_open()) {
        perror((string("error while opening file ") + FILE_TO_READ).c_str());
    }
    string line;
    auto last_time = chrono::system_clock::now();
    while (true) {
        while (getline(ifs, line)) {
            last_time = chrono::system_clock::now();
            cout << "Read: " << line << endl;
        }
        // Instead of checking for ifs.bad(), we allow eof, so as to continue reading file
        if (ifs.eof()) {
            if (chrono::duration_cast<chrono::seconds>(chrono::system_clock::now() - last_time) > chrono::seconds(5)) {
                cout << "No change detected in last 5 seconds, assuming file closed" << endl;
                break;
            }
        } else {
            perror((string("error while reading file ") + FILE_TO_READ).c_str());
            break;
        }
        ifs.clear();
        this_thread::sleep_for(chrono::milliseconds(500));
    }
    ifs.close();
    return 0;
}
