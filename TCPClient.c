// SOCKET PROGRAMMING "TCP_CLIENT" //
// ------------------------    //
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
// ------------------------    //
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
// ------------------------    //
# define PORT 1453

int main(int argc, char **argv){
    
    // Create a socket return value is socket descriptor 
    // @params AF_INET: IPV4, SOCK_STREAM: TCP, PROTOCOL: 0  
    int network_socket = socket(AF_INET, SOCK_STREAM, 0);

    // ? Socket is succesfully created
    if(network_socket < 0){
        fprintf(stderr, " **Socket error.\n");
        exit(EXIT_FAILURE);
    } else {
        fprintf(stdout, " - Socket created succesfully.\n");
    }

    // Specify an address for the network_socket
    // sockaddr_in for the IPV4 Protocol
    struct sockaddr_in server_address;
    
    // Reset the created server_address using the memset
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // or inet_addr("127.0.0.1")

    // ? Connect the server address
    // @param SOCKET, server_address, size of the server adress 
    if(connect(network_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0){
        fprintf(stderr, " - Connection error.\n");
        exit(EXIT_FAILURE);
    } else {
        fprintf(stdout, " - Connection succesfully.\n");
    }

    // ? Recieve data from the server
    // @param SOCKET, data address, data size, FLAGS;
    char server_response[256];
    
    if(recv(network_socket, &server_response, sizeof(server_response), 0) < 0){
        fprintf(stderr, " ** Server Recieve error");
        exit(EXIT_FAILURE);
    } else {
        fprintf(stderr, " - Recieve is successfully");
    }

    // Dump the server's response
    fprintf(stdout, " - SERVER RESPONSE: %s\n", server_response);

    close(network_socket);
    exit(EXIT_SUCCESS);
    return 0;
}