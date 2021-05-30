#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

using namespace std;

constexpr const char* FILE_TO_WRITE = "/tmp/somefile";

int main()
{
    ofstream ofs(FILE_TO_WRITE, ofstream::out | ofstream::trunc);
    int i = 0;
    while (true) {
        ++i;
        ostringstream oss;
        oss << "line" << i << "\n";
        string line(oss.str());
        cout << "Writing: " << line;
        ofs << line;
        this_thread::sleep_for(chrono::milliseconds(250));
        ofs.flush();
    }
    ofs.close();
    return 0;
}
