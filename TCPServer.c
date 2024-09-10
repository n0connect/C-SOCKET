// SOCKET PROGRAMMING "TCP_SERVER" //
// ------------------------    //
#include "colorcodes.h"
#include "database.h"
#include "socketutil.h"
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>

#define PORT 2000
#define MAX_CLIENTS 10 // Max 100 Client

// Create Global Client ID counter
int global_client_id = 0;
pthread_mutex_t id_mutex =
    PTHREAD_MUTEX_INITIALIZER; // ID'yi thread-safe yapmak için mutex

// Mutex to protect access to the accepted_sockets array
pthread_mutex_t accepted_sockets_mutex = PTHREAD_MUTEX_INITIALIZER;

// Safe ClientID++.
int generate_client_id() {
  pthread_mutex_lock(&id_mutex); // Thread-safe with Mutex.
  int new_id = global_client_id++;
  pthread_mutex_unlock(&id_mutex);
  return new_id;
}

struct AcceptedSocket {
  int accepted_socket_fd;
  struct sockaddr_in address;
  int error;
  bool accepted_success;
};

// FDeclaration ------------------  //
struct AcceptedSocket *AcceptIncomingConnections(int server_socket);
bool handle_login(int client_socket_fd, int sthread_id);
void handle_client_communication(int client_socket_fd, int sthread_id);
void *recv_the_client(void *arg);
void recv_the_client_separate_threads(struct AcceptedSocket *client_socket);
void start_accept_connections(int server_socket);
void send_the_buffer_other_clients(int client_socket, char *buffer);

// -------------------------------  //
size_t max_client = MAX_CLIENTS;
struct AcceptedSocket *accepted_sockets = NULL;
unsigned int accepted_sockets_count = 0;
extern int errno;


// Goz bebegim...
struct AcceptedSocket *AcceptIncomingConnections(int server_socket) {
  struct sockaddr_in client_address;
  socklen_t client_address_size = sizeof(client_address);

  int client_socket = accept(server_socket, (struct sockaddr *)&client_address,
                             &client_address_size);

  struct AcceptedSocket *accepted_return =
      malloc(sizeof(struct AcceptedSocket));

  if (accepted_return == NULL) {
    perror("Memory allocation for AcceptedSocket failed");
    exit(EXIT_FAILURE);
  }

  accepted_return->accepted_socket_fd = client_socket;
  accepted_return->address = client_address;

  if (client_socket < 0) {
    accepted_return->accepted_success = false;
    accepted_return->error = client_socket;
    fprintf(stderr, BRED " - Accept error: %s\n",
            strerror(errno));
  } else {

    accepted_return->accepted_success = true;
    accepted_return->error = 0;
    fprintf(stdout, BYEL " - Client connected successfully.\n");
  }

  return accepted_return;
}

// Forward all messages that reach the server to other clients
void send_the_buffer_other_clients(int client_socket, char *buffer) {
    pthread_mutex_lock(&accepted_sockets_mutex); // Lock access to accepted_sockets
    for (size_t i = 0; i < accepted_sockets_count; i++) {
        // Skip the client that sent the message
        if (accepted_sockets[i].accepted_socket_fd != client_socket) {
            ssize_t send_result = send(accepted_sockets[i].accepted_socket_fd, buffer, strlen(buffer), 0);

            if (send_result < 0) {
                fprintf(stderr, BRED " - Message send error (Client[%d]).\n", accepted_sockets[i].accepted_socket_fd % 11);
            } else {
                fprintf(stdout, BYEL " - Message sent successfully. (Client[%d])\n", accepted_sockets[i].accepted_socket_fd % 11);
            }
        }
    }
    pthread_mutex_unlock(&accepted_sockets_mutex); // Unlock access after done
}

// Get and Verify Login Information
bool handle_login(int client_socket_fd, int sthread_id) {
  char buffer[1024];
  bool login_successful = false;

  while (!login_successful) {
    ssize_t amount_received =
        recv(client_socket_fd, buffer, sizeof(buffer) - 1, 0);

    if (amount_received <= 0) {
      return false; // If the connection is lost or there is an error
    }

    buffer[amount_received] = '\0';
    fprintf(stdout, "[LOG]: Client[%d] sent login credentials: %s\n",
            sthread_id, buffer);

    // buffer format: "username:password"
    char username[32];
    char password[64];

    //  The ':' character will stop at the first occurrence (username)
    //  and take the rest of the character in its entirety (password)
    sscanf(buffer, "%[^:]:%[^\n]", username, password);

    if (check_credentials(username, password)) {
      const char *success_message = "Login:Successfully";
      send(client_socket_fd, success_message, strlen(success_message), 0);
      login_successful = true;
    } else {
      const char *fail_message = "Username or Password not correct, try again";
      send(client_socket_fd, fail_message, strlen(fail_message), 0);
    }
  }

  return true; // If login success.
}

// Receive and Share Messages Between Clients
void handle_client_communication(int client_socket_fd, int sthread_id) {
  char buffer[1024];

  while (true) {
    ssize_t amount_received =
        recv(client_socket_fd, buffer, sizeof(buffer) - 1, 0);

    if (amount_received <= 0) {
      break;
    }

    buffer[amount_received] = '\0';
    fprintf(stdout, "[LOG]: Client[%d]: %s\n", sthread_id, buffer);

    // Diğer istemcilere mesajı ilet
    send_the_buffer_other_clients(client_socket_fd, buffer);
  }
}

// Set the client
void *recv_the_client(void *arg) {
    struct AcceptedSocket *client_socket = (struct AcceptedSocket *)arg;
    int client_socket_fd = client_socket->accepted_socket_fd;

    // Create Unique ThreadID
    int client_id = generate_client_id();

    // Do the login
    if (!handle_login(client_socket_fd, client_id)) {
        fprintf(stderr, "[ERROR]: Client[%d] failed to login. Closing connection.\n", client_id);
        close(client_socket_fd);
        free(client_socket);

        pthread_mutex_lock(&accepted_sockets_mutex);
        accepted_sockets_count--;
        pthread_mutex_unlock(&accepted_sockets_mutex);

        return NULL;
    }

    // Receive messages and start communication
    handle_client_communication(client_socket_fd, client_id);

    // Find and remove client from the accepted list
    pthread_mutex_lock(&accepted_sockets_mutex);
    for (size_t i = 0; i < accepted_sockets_count; i++) {
        if (accepted_sockets[i].accepted_socket_fd == client_socket_fd) {
            close(client_socket_fd);
            
            // Shift the remaining clients down to fill the gap
            for (size_t j = i; j < accepted_sockets_count - 1; j++) {
                accepted_sockets[j] = accepted_sockets[j + 1];
            }

            accepted_sockets_count--; // Update client count after removing
            break;
        }
    }
    pthread_mutex_unlock(&accepted_sockets_mutex);

    free(client_socket); // Serbest bırakılacak doğru pointer
    return NULL;
}

// Recv all clients by diferent threads.
void recv_the_client_separate_threads(struct AcceptedSocket *client_socket) {
    pthread_t sthread_id;

    if (pthread_create(&sthread_id, NULL, recv_the_client, client_socket) != 0) {
        perror(BRED " - Failed to create thread");
        free(client_socket);
        exit(EXIT_FAILURE);
    }

    int detach_result = pthread_detach(sthread_id);
    if (detach_result != 0) {
        perror(BRED " - Failed to detach thread");
    }

    fprintf(stdout, BYEL " - Thread created and detached successfully for client.\n");
    return;
}

// Start --
void start_accept_connections(int server_socket) {
    while (true) {
        if (accepted_sockets_count >= MAX_CLIENTS) {
            fprintf(stderr, BRED " - Maximum number of clients reached. Rejecting new connections.\n");
            sleep(1); // Add the shorttime sleep
            continue;
        }

        struct AcceptedSocket *client_socket = AcceptIncomingConnections(server_socket);

        if (client_socket->accepted_success) {
            pthread_mutex_lock(&accepted_sockets_mutex); // Lock the array
            if (accepted_sockets_count < MAX_CLIENTS) {
                accepted_sockets[accepted_sockets_count++] = *client_socket;
                pthread_mutex_unlock(&accepted_sockets_mutex); // Unlock the array
                recv_the_client_separate_threads(client_socket);
            } else {
                pthread_mutex_unlock(&accepted_sockets_mutex); // Unlock the array in case of max clients
                shutdown(client_socket->accepted_socket_fd, SHUT_RDWR);
                free(client_socket);
            }
        }
    }
}

//
void __arg_options(int argc, char **argv) {
  if (argc <= 0) {
    fprintf(stdout, BHBLK " - There are not enough arguments\n");
    return;
  }

  if (strcmp(argv[0], "-adduser")) {
    // add_user(argv[1], argv[2]);
    // exit(EXIT_SUCCESS);
  }
}

// -----------------------------------  //
int main(int argc, char **argv) {

  // Call the __arg_options()
  // __arg_options(argc, argv);

  // Create the server socket
  int server_socket = createTCPIp4Socket();

  // Define the server address
  struct sockaddr_in *server_address = createIPv4Address("", PORT);

  // Bind the socket to our specified IP ad PORT
  if (bind(server_socket, (struct sockaddr *)server_address,
           sizeof(*server_address)) < 0) {
    fprintf(stderr, BRED " - Connection error.\n");
    shutdown(server_socket, SHUT_RDWR);
    free(server_address);
    return -1;
  } else {
    fprintf(stdout, BYEL " - Connection successfully.\n");
  }

  // Listen the server socket
  if (listen(server_socket, 5) < 0) {
    fprintf(stderr, BYEL " - Listening error.\n");
  } else {
    fprintf(stdout, BYEL " - Listening the requests...\n");
  }

  // Allocate memory blocks for the accepted socket's
  accepted_sockets = calloc(MAX_CLIENTS, sizeof(struct AcceptedSocket));
  start_accept_connections(server_socket);

  shutdown(server_socket, SHUT_RDWR);
  exit(EXIT_SUCCESS);
}
