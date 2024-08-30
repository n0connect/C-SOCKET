// SOCKET PROGRAMMING "TCP_SERVER" //
// ------------------------    //
#include "socketutil.h"
#include "colorcodes.h"
#include <stdbool.h>
#include <pthread.h>

# define PORT 2000

struct AcceptedSocket {

    int accepted_socket_fd;
    struct sockaddr_in address;
    int error;
    bool accepted_succsess;

};

struct AcceptedSocket* AcceptIncomingConnections(int server_socket){

    struct sockaddr_in client_address;
    int client_address_size = sizeof(client_address);
    int client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_size);

    struct AcceptedSocket* accepted_return = malloc(sizeof(struct AcceptedSocket));
    accepted_return->accepted_socket_fd = client_socket;
    accepted_return->address = client_address;

    if(client_socket < 0){
        
        accepted_return->accepted_succsess = false;
        accepted_return->error = client_socket;
        fprintf(stderr, BRED" - Accept error.\n");
    } else {
        accepted_return->accepted_succsess = true;
        accepted_return->error = client_socket;
        fprintf(stdout, BYEL" - Accept is succesfully...\n");
        
    }

    return accepted_return;
}

// -------------------------------  //
struct AcceptedSocket accepted_sockets[10];
int accepted_sockets_count = 0;

// FDeclaration ------------------  //
void* recv_the_client(void* arg);
void recv_the_client_seperate_threads(struct AcceptedSocket* client_socket);
void start_accept_connections(int server_socket);
void send_the_buffer_other_clients(int client_socket, char *buffer);


void send_the_buffer_other_clients(int client_socket, char *buffer){

    for(size_t i = 0; i < accepted_sockets_count; i++){
        if(accepted_sockets[i].accepted_socket_fd != client_socket){
            ssize_t send_result = send(accepted_sockets[i].accepted_socket_fd, buffer, strlen(buffer), 0);

            if(send_result < 0){
                    fprintf(stderr, BRED" - Message send error (Client[%d]).\n", 
                    accepted_sockets[i].accepted_socket_fd%11);

            } else {
                fprintf(stdout, BYEL" - Message sent successfully. (Client[%d])\n",
                    accepted_sockets[i].accepted_socket_fd%11);
            }
        }
    }

}

void* recv_the_client(void* arg) {
    struct AcceptedSocket* client_socket = (struct AcceptedSocket*)arg;
    int client_socket_fd = client_socket->accepted_socket_fd;
    int sthread_id = client_socket->error % 11; // Simplified thread ID assignment

    char buffer[1024];
    while (true) {
        ssize_t amount_received = recv(client_socket_fd, buffer, sizeof(buffer) - 1, 0);

        if (amount_received <= 0) {
            break;
        }

        buffer[amount_received] = '\0';
        fprintf(stdout, BGRN" - Client[%d]: %s\n", sthread_id, buffer);
        send_the_buffer_other_clients(client_socket_fd, buffer);
    }

    close(client_socket_fd);
    free(client_socket); // Free the memory
    accepted_sockets_count--;

    return NULL;
}

void recv_the_client_separate_threads(struct AcceptedSocket* client_socket) {
    pthread_t sthread_id;
    if (pthread_create(&sthread_id, NULL, recv_the_client, client_socket) != 0) {
        perror(BRED " - Failed to create thread");
        exit(EXIT_FAILURE);
    }
    pthread_detach(sthread_id); // Automatically clean up the thread resources when it exits
}

void start_accept_connections(int server_socket) {
    while (true) {
        struct AcceptedSocket* client_socket = AcceptIncomingConnections(server_socket);
        if (client_socket->accepted_succsess) {
            accepted_sockets[accepted_sockets_count++] = *client_socket;
            recv_the_client_separate_threads(client_socket);
        }
    }
}

// -----------------------------------  //
int main(int argc, char **argv){
    
    const char server_message[256] = "\e[1;32mSuccesfully connected TCPServer :)";

    // Create the server socket
    int server_socket = createTCPIp4Socket();

    // Define the server address
    struct sockaddr_in *server_address = createIPv4Address("", PORT);

    // Bind the socket to our specified IP ad PORT
    if(bind(server_socket, (struct sockaddr*)server_address, sizeof(*server_address)) < 0){
        fprintf(stderr, "\e[1;31m - Connection error.\n");
        exit(EXIT_FAILURE);
    } else {
        fprintf(stdout, "\e[1;33m - Connection succesfully.\n");
    }

    // Listen the server socket
    if(listen(server_socket, 5) < 0){
        fprintf(stderr, "\e[1;31m - Listening error.\n");
    } else {
        fprintf(stdout, "\e[1;33m - Listening the requests...\n");
    }
    
    start_accept_connections(server_socket);

    shutdown(server_socket, SHUT_RDWR);
    exit(EXIT_SUCCESS);
}