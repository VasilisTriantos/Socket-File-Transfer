#ifndef SOCKET_UTILITIES_H
#define SOCKET_UTILITIES_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h> // For malloc

struct sockaddr_in* create_ipv4_address(char *ip, int port);

int create_tcp_ipv4_socket();

#endif /* SOCKET_UTILITIES_H */