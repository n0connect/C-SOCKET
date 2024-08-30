// SOCKET PROGRAMMING "TCP_CLIENT" //
// ------------------------    //
#include "socketutil.h"
#include "colorcodes.h"
#include <stdbool.h>
#include <pthread.h>

# define PORT 2000

void start_listening_messages_new_thread(int network_socket);
void *listening_messages_thread(int network_socket);


void *listening_messages_thread(int network_socket){
    char buffer[1024];

    while (true) {
        ssize_t amount_received = recv(network_socket, buffer, sizeof(buffer) - 1, 0);

        if (amount_received <= 0) {
            break;
        }

        buffer[amount_received] = '\0';
        fprintf(stdout, BGRN" - %s\n", buffer);
    }

}

void start_listening_messages_new_thread(int network_socket){
    pthread_t thread_id;
    int thread_result = pthread_create(&thread_id, NULL, listening_messages_thread, network_socket);
    
    if (thread_result != 0) {
        perror(BRED " - Failed to create thread");
        exit(EXIT_FAILURE);
    }
    pthread_detach(thread_id); // Automatically clean up the thread resources when it exits
}

int main(int argc, char **argv){
    // Create a socket, return value is socket descriptor 
    int network_socket = createTCPIp4Socket();

    // Specify an address for the network_socket
    struct sockaddr_in* server_address = createIPv4Address("127.0.0.1", PORT);

    // Connect to the server address
    if (connect(network_socket, (struct sockaddr*)server_address, sizeof(*server_address)) < 0) {
        fprintf(stderr, BRED" - Connection error.\n");
        free(server_address); // Free memory allocated by createIPv4Address
        exit(EXIT_FAILURE);
    } else {
        fprintf(stdout, BYEL" - Connection successfully.\n");
    }

    char *client_name = NULL;
    size_t client_name_size = 0;
    fprintf(stderr, BCYN"  - Please enter your username: ");
    ssize_t name_count = getline(&client_name, &client_name_size, stdin);
    if (name_count > 0) {
        client_name[strcspn(client_name, "\n")] = '\0'; // Remove newline character
    }

    // Chatting with server.
    char *line = NULL;
    size_t line_size = 0;
    fprintf(stderr, CYN"\ntype and we will send(type exit)...\n");

    start_listening_messages_new_thread(network_socket);

    char buffer[1024];
    while (true) {
        
        if (line) {
            free(line);
        }

        line = NULL;
        line_size = 0;

        ssize_t char_count = getline(&line, &line_size, stdin);
        if (char_count > 0) {
            line[strcspn(line, "\n")] = '\0'; // Remove newline character

            if (strcmp(line, "exit") == 0) {
                break;
            }

            snprintf(buffer, sizeof(buffer), "%s: %s", client_name, line);

            ssize_t amount_was_sent = send(network_socket, buffer, strlen(buffer), 0);
            if (amount_was_sent < 0) {
                fprintf(stderr, BRED" - Message send error.\n");
                free(server_address); // Free memory allocated by createIPv4Address
                exit(EXIT_FAILURE);
            } else {
                fprintf(stdout, BYEL" - Message sent successfully.\n");
            }
        }
    }

    // Clean up
    close(network_socket);
    free(server_address); // Free memory allocated by createIPv4Address
    free(client_name);   // Free memory allocated by getline
    free(line);          // Free memory allocated by getline
    exit(EXIT_SUCCESS);
}
