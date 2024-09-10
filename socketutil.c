// SOCKET PROGRAMMING "SOCKET_HEADER_FILE" //
// ------------------------    //
#include "socketutil.h"
// ------------------------    //

int createTCPIp4Socket(){

    // @params AF_INET: IPV4, SOCK_STREAM: TCP, PROTOCOL: 0
    int socket_return = socket(AF_INET, SOCK_STREAM, 0);

    // ? Socket is succesfully created
    if(socket_return < 0){
        fprintf(stderr, "\e[1;31m - Socket error.\n");
        exit(EXIT_FAILURE);
    } else {
        fprintf(stdout, "\e[1;33m - Socket created succesfully.\n");
    }

    return socket_return;
}

struct sockaddr_in* createIPv4Address(char *ip_address, unsigned short int port){

    struct sockaddr_in *address = malloc(sizeof(struct sockaddr_in));
    
    // Reset the created server_address using the memset
    // memset((struct sockaddr *)address, 0, sizeof(*address));

    address->sin_family = AF_INET;
    address->sin_port = htons(port);
    
    if(strlen(ip_address) == 0)
        address->sin_addr.s_addr = (INADDR_ANY);
    else
        inet_pton(AF_INET, ip_address, &(address->sin_addr));
        

    return address;
}
