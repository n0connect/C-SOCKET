// SOCKET PROGRAMMING "TCP_SERVER" //
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
    
    char server_message[256] = "Welcome from the TCPServer :)";

    // Create the server socket
    // ? Socket is succesfully created
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    
    if(server_socket < 0){
        fprintf(stderr, " **Socket error.\n");
        exit(EXIT_FAILURE);
    } else {
        fprintf(stdout, " - Socket created succesfully.\n");
    }

    // Define the server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind the socket to our specified IP ad PORT
    if(bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0){
        fprintf(stderr, " **Connection error.\n");
        return 1;
    } else {
        fprintf(stdout, " - Connection succesfully.\n");
    }

    // Listen the server socket
    if(listen(server_socket, 5) < 0){
        fprintf(stderr, "Listening error.\n");
    } else {
        fprintf(stdout, "Listening the requests...\n");
    }

    // Create Client Socket
    int client_socket = accept(server_socket, NULL, NULL);

    // Send the message
    send(client_socket, server_message, sizeof(server_message), 0);

    close(server_socket);
    
    return 0;
}