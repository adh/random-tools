#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <gc/gc.h>

int tcp_bind(unsigned int port){
  struct sockaddr_in inet_addr;
  int fd;
  struct hostent* h;


  inet_addr.sin_family = AF_INET;
  inet_addr.sin_port = htons(port);
  inet_addr.sin_addr.s_addr = INADDR_ANY;

  fd = socket(PF_INET, SOCK_STREAM, 0);
  if (fd == -1){
    return -1;
  }
  
  if (bind(fd,(struct sockaddr*)&inet_addr,
           sizeof(inet_addr))==-1){
    close(fd);
    return -1;
  }
  
  return fd;
}

#define SEND_SIZE 1048576

int main(int argc, char**argv){
  int server_fd;
  int client_fd;
  char buf[SEND_SIZE];

  memset(buf, 0, SEND_SIZE);

  if (argc != 2){
    fprintf(stderr, "usage: %s <listen-port>", argv[0]);
    return 1;
  }

  server_fd = tcp_bind(atoi(argv[1]));
  listen(server_fd, 1);
  client_fd = accept(server_fd, NULL, NULL);
  while (write(client_fd, buf, SEND_SIZE) > 0){}
  close(client_fd);
  close(server_fd);
  return 1;
}
