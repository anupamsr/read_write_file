# read_write_file
Different ways to watch a file for modification (and print its content)

Each ways is implemented as a commit. The descriptions of all commits are listed below. The code is only for learning, and is released under GPL v 3.

Following ways to detect a change in file have been implemented:
1. Check if the file has been changed in last 5 seconds. If not, it is probably not being written anymore, so quit.
2. Use inotify socket to read (blocking). Quit on detecting IN_CLOSE event or IN_OPEN event, the idea being that the monitor (read_file) should be openened while the file is being modified.
3. Converted inotify to a object-oriented design, including an iterator.
4. Use inotify class in multithreaded environment - a different thread to monitor changes, while the main thread waits for its signal to read the file.
5. Move inotify to main thread and reading of file to different thread
6. Use IN_NONBLOCK inotify socket. Main thread calls non-blocking read() every second, and if a change is detected, tells another thread to read rest of the file.
7. Use epoll to know when to call read, instead of relying on read to fail.
