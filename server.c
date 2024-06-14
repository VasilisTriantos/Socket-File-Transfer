#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <sys/statvfs.h> 
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "socket_utilities.h"

#define BUFFER_SIZE 1024
#define LOG_PATH "../logs/server_log.txt"
#define MIN_FREE_SPACE (5LL * 1024 * 1024 * 1024) // 5 GB in bytes
#define MAX_FILE_SIZE (1LL * 1024 * 1024 * 1024) // 1 GB in bytes
// #define MAX_FILE_SIZE (1LL) // 1 byte for tests

// Structure to pass arguments to thread
struct ThreadArgs {
    int client_FD;
    unsigned long free_space;
};

// Function prototypes
void log_event(const char *event);
void *handle_client(void *arg);
void receive_file_contents(int client_FD, FILE *file, size_t file_size);
void ack_handler(int client_FD, char ack);


// Log an event with timestamp
void log_event(const char *event) 
{
    time_t current_time;
    char time_string[100];
    struct tm *time_info;

    time(&current_time);
    time_info = localtime(&current_time);
    strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", time_info);

    FILE *log_file = fopen(LOG_PATH, "a");
    if (log_file != NULL) 
    {
        fprintf(log_file, "[%s] %s\n", time_string, event);
        fclose(log_file);
    }
}

// Function to handle each client connection
void *handle_client(void *arg) {
    struct ThreadArgs *thread_args = (struct ThreadArgs *)arg;

    int client_FD = thread_args->client_FD;
    unsigned long free_space = thread_args->free_space;
    free(thread_args); // Free memory allocated for thread arguments

    // Receive the size of the file
    uint32_t file_size;
    int bytes_received = recv(client_FD, &file_size, sizeof(file_size), 0);
    if (bytes_received != sizeof(file_size)) {
        printf("Error receiving file size\n");
        close(client_FD);
        return NULL;
    }

    if (file_size == 0)
    {
        printf("There was no file from the client\n");
        close(client_FD);
        return NULL;
    }

    // Check if the file size exceeds the maximum allowed size
    if (file_size > MAX_FILE_SIZE) {
        printf("File size exceeds maximum allowed size. File transfer denied.\n");
        log_event("File size exceeds maximum allowed size. File transfer denied.\n");
        ack_handler(client_FD,'Q');
        close(client_FD); 
        return NULL;
    }
    
    // Calculate the remaining free space after receiving the file size
    unsigned long total_file_size = (unsigned long)file_size * 5LL;
    unsigned long remaining_space = free_space - total_file_size;
    if (remaining_space < MIN_FREE_SPACE) {
        printf("Insufficient remaining free space on server. File transfer denied.\n");
        log_event("Insufficient remaining free space on server. File transfer denied.\n");
        close(client_FD); 
        return NULL;
    }

    ack_handler(client_FD, 'A');

    FILE *file = fopen("../receive/received_file.txt", "wb"); // Open file for writing in binary mode (Write Binary)
    //error, the folder doesn't exist
    if (file == NULL) {
        printf("File opening failed\n"); // file opening fails
        close(client_FD); 
        return NULL;
    }

    // Receive the file contents
    receive_file_contents(client_FD, file, file_size);

    // Close the file
    fclose(file); // Close the file
    printf("File received and saved.\n"); 
    printf("file size is: %zu\n",file_size);
    
    close(client_FD);
    printf("client disconnected\n\n");
    return NULL;
}

// Function to receive file contents
void receive_file_contents(int client_FD, FILE *file, size_t file_size) {
    // Receive the file contents
    char buffer[BUFFER_SIZE];
    size_t total_bytes_received = 0;
    ssize_t bytes_received;
    while (total_bytes_received < file_size && (bytes_received = recv(client_FD, buffer, BUFFER_SIZE, 0)) > 0) { // Receive file data in chunks
        fwrite(buffer, 1, bytes_received, file); // Write received data to file
        total_bytes_received += bytes_received; // Update total bytes received
    }
}

//send the aknowledgement
void ack_handler(int client_FD, char ack){
    if (send(client_FD, &ack, sizeof(ack), 0) == -1) { // Send acknowledgment to client
        printf("Send acknowledgment failed\n"); // sending acknowledgment fails
    }
}

int main() {
    // Check disk space before starting the server
    struct statvfs vfs;
    if (statvfs("/", &vfs) != 0) { // Check the root filesystem
        printf("Failed to get file system information\n");
        return 1;
    }
    
    // Check if there is enough space to run the server
    unsigned long free_space = (unsigned long)vfs.f_frsize * (unsigned long)vfs.f_bfree;
    if (free_space < MIN_FREE_SPACE) {
        printf("Insufficient free space on server. Server shutting down.\n");
        log_event("Server shutdown: insufficient free space on server\n");
        return 1;
    }

    // Create a file descriptor for the server
    int server_FD = create_tcp_ipv4_socket();
    if (server_FD == -1) {
        printf("Socket creation failed\n"); // socket creation fails
        return 1;
    }

    // Set SO_REUSEADDR option to allow reusing the address
    int reuse_addr = 1;
    if (setsockopt(server_FD, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) == -1) { // Set socket options
        printf("Set SO_REUSEADDR failed\n"); // setting socket options fails
        close(server_FD); 
        return 1;
    }
    
    // Bind/lock the port if it's not taken
    struct sockaddr_in *server_addr = create_ipv4_address("", 2000); 
    if (bind(server_FD, (struct sockaddr *)server_addr, sizeof(*server_addr)) == -1) { 
        printf("Bind failed\n"); // binding fails
        close(server_FD); 
        return 1;
    }
    printf("Server socket bound.\n");

    // Start listening on the port for connections
    if (listen(server_FD, 10) == -1) { 
        printf("Listen failed\n"); // listen fails
        close(server_FD); 
        return 1;
    }
    printf("Listening for connections...\n"); // Print message indicating server is listening

    // Continuously accept incoming connections
    while (1) {
        // Accept the client and get their "file descriptor"
        struct sockaddr_in clientAddr; // Structure to store client's address information
        socklen_t client_addr_size = sizeof(struct sockaddr_in); // Size of client address structure
        int *client_FD = malloc(sizeof(int)); // Allocate memory to store client socket descriptor
        *client_FD = accept(server_FD, (struct sockaddr *)&clientAddr, &client_addr_size); // Accept connection and get client socket descriptor
        if (*client_FD < 0) { // Check if accept failed
            printf("Accept failed\n"); // Print error message
            close(server_FD); 
            return 1; // Return with error code
        }
        printf("Client connected.\n"); // Print message indicating client connection

        // Create a new thread to handle communication with the client
        pthread_t thread_id; 
        struct ThreadArgs *thread_args = malloc(sizeof(struct ThreadArgs));
        if (thread_args == NULL) {
            printf("Failed to allocate memory for thread arguments \n");
            close(server_FD); 
            return 1;
        }
        thread_args->client_FD = *client_FD;
        thread_args->free_space = free_space;
        if (pthread_create(&thread_id, NULL, handle_client, (void *)thread_args) != 0) { // Create thread
            close(server_FD);
            printf("Thread creation failed. \n"); 
            return 1; 
        }
        pthread_detach(thread_id); // Detach the thread to clean up its resources automatically
    }

    
    close(server_FD); 

    return 0;
}
