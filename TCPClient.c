#include "colorcodes.h"
#include "socketutil.h"
#include <pthread.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

#include <unistd.h> // for the 'isatty' function
#include <sys/ioctl.h> // for the 'ioctl' function

void start_listening_messages_new_thread(int network_socket);
void *listening_messages_thread(void *arg);
bool send_the_buffer(int network_socket, char *buffer);

#define PORT 2000
#define MAX_LOGIN 5
#define MAX_SEND 5

char *username_ptr = NULL;

void newline_messagebox(){
    fprintf(stdout, "\n");
    if (username_ptr) {
        fprintf(stdout, BYEL "- [%s]: ", username_ptr);
    } else {
        fprintf(stdout, BYEL "- [unknown]: ");
    }
    fflush(stdout); // Yazar işleminden sonra tamponu boşalt
}

int get_terminal_width() {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1) {
        return ws.ws_col;
    }
    return 80; // Varsayılan genişlik
}

void *listening_messages_thread(void *arg) {
    int network_socket = *(int *)arg;
    free(arg); // Soket kopyasını serbest bırak
    char buffer[1024];

    int terminal_width = get_terminal_width(); // Terminal genişliğini dinamik olarak al
    int half_terminal_width = terminal_width / 2;

    while (true) {
        ssize_t amount_received = recv(network_socket, buffer, sizeof(buffer) - 1, 0);
        if (amount_received <= 0) {
            if (amount_received == 0) {
                fprintf(stderr, BRED " - Connection closed by server.\n");
            } else {
                perror(BRED " - Message receive error\n");
            }
            break; // Hata durumunda döngüden çık
        }

        buffer[amount_received] = '\0';
        size_t message_length = strlen(buffer);
        char temp_username[32];
        char temp_buffer[1024];
        sscanf(buffer, "%31[^:]:%1023[^\n]", temp_username, temp_buffer);
        size_t username_len = strlen(temp_username);
        message_length -= username_len;

        fprintf(stdout, "\r"); // Satırı başa döndür

        if (half_terminal_width < message_length) {
            size_t start_index = 0;
            size_t remaining_length = message_length;
            fprintf(stdout, BCYN "%*s-[%s]:", half_terminal_width, "", temp_username);

            while (remaining_length > 0) {
                size_t line_length = remaining_length > half_terminal_width ? half_terminal_width : remaining_length;
                fprintf(stdout, "\n");
                fprintf(stdout, BCYN "%*s", half_terminal_width, "");
                fwrite(temp_buffer + start_index, 1, line_length, stdout);
                remaining_length -= line_length;
                start_index += line_length;
            }
        } else {
            fprintf(stdout, BCYN "%*s%s\n", half_terminal_width, "", buffer);
        }
        fflush(stdout);
        newline_messagebox(); // Mesaj yazdırma işleminden sonra çağrılır
    }

    return NULL;
}

void start_listening_messages_new_thread(int network_socket) {
    pthread_t thread_id;
    int *socket_copy = malloc(sizeof(int));
    *socket_copy = network_socket;

    int thread_result = pthread_create(&thread_id, NULL, listening_messages_thread, socket_copy);

    if (thread_result != 0) {
        perror(BRED " - Failed to create thread");
        exit(EXIT_FAILURE);
    }
    pthread_detach(thread_id);
}

bool connect_the_adress(int network_socket, struct sockaddr *server_address) {
  size_t server_size = sizeof(*server_address);

  if (connect(network_socket, (struct sockaddr *)server_address, sizeof(*server_address)) < 0) {
    fprintf(stderr, BRED " - Connection error.\n");
    return false;
  }

  fprintf(stdout, BYEL " - Connection successfully.\n");
  return true;
}

char *get_username() {
    char *username = NULL;
    size_t username_size = 0;

    if (username_ptr == NULL) {
        username_ptr = malloc(32 * sizeof(char)); // Daha güvenli bellek ayırma
        if (username_ptr == NULL) {
            perror("Failed to allocate memory for username_ptr");
            exit(EXIT_FAILURE);
        }
    }

    while (true) {
      fprintf(stdout, BCYN "  - Please enter your username: ");
      getline(&username, &username_size, stdin);
      if (username == NULL) {
          perror("Failed to read username");
          exit(EXIT_FAILURE);
      }
      // Yeni satırı kaldır
      username[strcspn(username, "\n")] = '\0';
      if (strlen(username) > 3 && strlen(username) <= 32) {
          strncpy(username_ptr, username, 32);
          
          username_ptr[strcspn(username_ptr, "\n")] = '\0';
          printf("User name copied -> %s\n", username_ptr);
          free(username); // Bellek serbest bırakılır
          break;
      } else {
          fprintf(stdout, BRED "Username must be between 4 and 32 characters\n");
          sleep(3);
          system("clear");
      }
    }

    system("clear");
    return username_ptr;
}

char *get_password() {
  char *password = NULL;
  size_t password_size = 0;

  while (true) {
    fprintf(stdout, BCYN "  - Please enter your password: ");
    getline(&password, &password_size, stdin);

    if (strlen(password) >= 3) {
      break;
    } else {
      fprintf(stdout, BRED "Password must be at least 3 characters\n");
      sleep(3);
      system("clear");
    }
  }

  system("clear");
  return password;
}

bool send_the_buffer(int network_socket, char *buffer) {
  if (buffer == NULL || strlen(buffer) == 0) {
    fprintf(stderr, BRED " - Invalid buffer.\n");
    return false;
  }

  ssize_t amount_was_sent = send(network_socket, buffer, strlen(buffer), 0);
  if (amount_was_sent < 0) {
    fprintf(stderr, BRED " - Error sending message.\n");
    return false;
  }

  return true;
}

int main(int argc, char **argv) {
  int network_socket = createTCPIp4Socket();
  struct sockaddr_in *server_address = createIPv4Address("127.0.0.1", PORT);

  if (!connect_the_adress(network_socket, (struct sockaddr *)server_address)) {
    shutdown(network_socket, SHUT_RDWR);
    free(server_address);
    return -1;
  }

  char *username = NULL;
  char *password = NULL;
  char buffer[1024];
  bool login_successful = false;

  while (!login_successful) {
    username = get_username();
    username[strcspn(username, "\n")] = '\0';

    password = get_password();
    password[strcspn(password, "\n")] = '\0';

    int sn_ret = snprintf(buffer, sizeof(buffer), "%s:%s", username, password);
    if (sn_ret < 0 || sn_ret >= sizeof(buffer)) {
     fprintf(stderr, "Buffer error\n");
     exit(EXIT_FAILURE);
    }

    bool send_return = send_the_buffer(network_socket, buffer);
    if (!send_return) {
     for (size_t i = 0; i < MAX_SEND; ++i) {
      sleep(1);
      send_return = send_the_buffer(network_socket, buffer);
      if (send_return) {
        break;
      }
     }

     if (!send_return) {
       shutdown(network_socket, SHUT_RDWR);
       free(server_address);
       free(username);
       free(password);
       return -1;
     }
    }

    ssize_t amount_received = recv(network_socket, buffer, sizeof(buffer) - 1, 0);
    if (amount_received > 0) {
      buffer[amount_received] = '\0';
      if (strcmp(buffer, "Login:Successfully") == 0) {
        fprintf(stdout, BGRN " - %s\n", buffer);
        login_successful = true;
      } else {
        fprintf(stderr, BRED " - %s\n", buffer);
      }
    }
  }

  start_listening_messages_new_thread(network_socket);

  char *line = NULL;
  size_t line_size = 0;
  char stack_buffer[1024];

  newline_messagebox();

  while (true) {
    if (line) {
      free(line);
    }

    line = NULL;
    line_size = 0;

    ssize_t char_count = getline(&line, &line_size, stdin);
    if (char_count > 0) {
      line[strcspn(line, "\n")] = '\0';

      if (strcmp(line, "exit") == 0) {
        break;
      }

      snprintf(stack_buffer, sizeof(stack_buffer), "%s: %s", username, line);
      ssize_t amount_was_sent = send(network_socket, stack_buffer, strlen(stack_buffer), 0);

      if (amount_was_sent < 0) {
        fprintf(stderr, BRED " - Message send error.\n");
        break;
      } else {
        fprintf(stdout, BYEL "- successfully.");
        fflush(stdout);
        newline_messagebox();
      }
    }
  }

  shutdown(network_socket, SHUT_RDWR);
  free(server_address);
  free(username);
  free(password);
  free(line);
  exit(EXIT_SUCCESS);
}
