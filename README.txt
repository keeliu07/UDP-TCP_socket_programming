2020-2021 Computer Networking
This is implementing a Remote Backup System with UDP/TCP socket programming.

* Dev Environment
 - OS: MacOS
 - complier: 'clang++ -std=c++11'

* Usage
  - w/ makefile under folder skeleton
    - type 'make', following commands will be executed;
      'clang++ server.cc -g -std=c++11 -o ../excecutable/server'
      'clang++ client.cc -g -std=c++11 -o ../excecutable/client'
  - or use g++ complier is okay as well;
    'g++ -std=c++11 server.cc -o server'
    'g++ -std=c++11 client.cc -o client'

  Run the Server/Client with:
    ./server [-p port]
    ./client [-s server_address] -p port

* Folder Structure
  - backup
  - executable
    - extension w/ '.dSYM' are auto-generated while compiling, please ignore.
    - TEST.pdf
    - client
    - server
  - skeleton
    - makefile
    - server.h
    - socket-server.h
    - server.cc
    - client.h
    - socket-client.h
    - client.cc
    - message.h
  - README.txt

* Compulsory functions are all Implemented (except bonus part)
  * Client Command Functions
    - ls
    - send [filename w/path]
      * it accepts relative or absolute path or just filename if you are in the same level;
      - e.g. 'send ./executable/TEST.pdf' or 'send Desktop/TEST.pdf' or simply 'send TEST.pdf'
    - remove [filename]
    - rename [filename] [target filename]
    - quit
    - shutdown

  * Server Response Functions
    - ls
    - send
    - remove
    - rename
    - shutdown
