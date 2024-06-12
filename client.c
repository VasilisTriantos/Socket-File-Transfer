#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "socket_utilities.h"

#define BUFFER_SIZE 1024

// Function prototypes
int setup_Socket_And_Connect();
size_t get_File_Size(FILE *file);
int send_File(int socket_FD, FILE *file);

// Function to send the file contents
int send_File(int socket_FD, FILE *file) {
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        ssize_t bytes_sent = send(socket_FD, buffer, bytes_read, 0);
        if (bytes_sent == -1) {
            perror("Send failed");
            return -1;
        }
    }
    return 0;
}

// Function to get the size of the file
size_t get_File_Size(FILE *file) {
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);
    return file_size;
}

// Function to create socket and connect to the server
int setup_Socket_And_Connect() {
    int socket_FD = create_tcp_ipv4_socket();
    if (socket_FD == -1) {
        perror("Socket creation failed");
        return -1;
    }

    struct sockaddr_in *address = create_ipv4_address("127.0.0.1", 2000);
    if (address == NULL) {
        perror("Address creation failed");
        close(socket_FD);
        return -1;
    }

    if (connect(socket_FD, (struct sockaddr*)address, sizeof(*address)) == -1) {
        perror("Connection failed");
        close(socket_FD);
        return -1;
    }

    printf("Connection successful!\n");
    return socket_FD;
}

int main() {
    int socket_FD = setup_Socket_And_Connect();
    if (socket_FD == -1) {
        return 1;
    }

    FILE *file = fopen("../send/file_to_send.txt", "rb");
    if (file == NULL) {
        //send the sevrer a filesize of 0
        size_t file_size = 0;
        if (send(socket_FD, &file_size, sizeof(file_size), 0) == -1) {
            perror("Send file size failed");
            close(socket_FD);
            return 1;
        }
        perror("File opening failed");
        close(socket_FD);
        return -1;
    }

    size_t file_size = get_File_Size(file);
    printf("file size is: %d\n",file_size);
    if (send(socket_FD, &file_size, sizeof(file_size), 0) == -1) {
        perror("Send file size failed");
        fclose(file);
        close(socket_FD);
        return 1;
    }

    char ack;
    if (recv(socket_FD, &ack, sizeof(ack), 0) == -1) {
        perror("Receive acknowledgment failed");
        fclose(file);
        close(socket_FD);
        return 1;
    }
    if(ack == 'Q'){
        printf("server refused the file \n");
        fclose(file);
        close(socket_FD);
        return 1;
    }
    
    if (send_File(socket_FD, file) == -1) {
        fclose(file);
        close(socket_FD);
        return 1;
    }

    fclose(file);
    close(socket_FD);

    return 0;
}