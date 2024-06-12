#include "socket_utilities.h"


struct sockaddr_in* create_ipv4_address(char *ip, int port) {
    struct sockaddr_in *address = malloc(sizeof(struct sockaddr_in));
    address->sin_family = AF_INET;
    address->sin_port = htons(port);

    if(strlen(ip)== 0)
    {
        address->sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
    inet_pton(AF_INET, ip, &address->sin_addr);
    }
    return address;
}


//          creating a socket
int create_tcp_ipv4_socket(){
    return socket(AF_INET, SOCK_STREAM, 0);
}
