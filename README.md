# Socket Programming Project

## Overview
This project demonstrates socket programming in C. It includes a client and server implementation. It uses a TCP/IP connection between the client and the server.
This was made as a university project.

### Server
The server checks if there's more than 5gb available for stability reasons. 
If it has enough space available then it starts listening for client connections and denies any file that would make the operation of the server unsafe. 
If if everything is fine then the server saves the file on `/recieve` with the name `received_file.txt`

### Client
It tries to connect to the server. 
It expecs the file name to be `file_to_send.txt`
Sends the file size of the file on the directory `/send`
if the server accepts it then sends the file and closes the connection.

## Socket_utilites.h  Socket_utilites.c
It contains helper functions and definitions that are used by both the client and the server to handle the common socket operations.


## Files
- `client.c`: Client-side code
- `server.c`: Server-side code
- `socket_utilities.c` and `socket_utilities.h`: Utility functions for socket operations
- `client_comp.sh`: Script to compile the client code
- `serv_comp.sh`: Script to compile the server code

## Usage
### Compile the code
To compile and run the client and server code, run the following scripts on different terminals:
```sh
./client_comp.sh
./serv_comp.sh
```

## Tested on:
This code was run and tested on Arch linux with GCC. I can't guarantee that it will run on Windows or any other linux distro


## Shortcomings
The major drawbacks of this implementation is mostly that it expects everything to be on very specific directories and very specific filenames.
