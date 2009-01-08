/*
 * uscat - UNIX Domain sockets utility
 * Copyright (c) 2004-2008 Ales Hakl
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/select.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <stdio.h>


#define BUF_SIZE 65536

void my_fd_set(int* max, int fd, fd_set* fdset){
  if (fd>*max){
    *max=fd;
  }
  FD_SET(fd, fdset);
}

int unix_connect(char* path){
  int sock;
  struct sockaddr_un unix_addr;

  sock=socket(PF_UNIX,SOCK_STREAM,0);

  if (sock < 0){
    perror("socket");
    return -1;
  }

  unix_addr.sun_family=AF_UNIX;
  strcpy(unix_addr.sun_path,path);

  if (connect(sock,(struct sockaddr*)&unix_addr, sizeof(unix_addr)) < 0){
    perror("connect");
    close(sock);
    return -1;   
  }

  return sock;
}

int unix_listen(char* path){
  int sock;
  struct sockaddr_un unix_addr;

  sock=socket(PF_UNIX,SOCK_STREAM,0);

  if (sock < 0){
    perror("socket");
    return -1;
  }

  unix_addr.sun_family=AF_UNIX;
  strcpy(unix_addr.sun_path,path);

  if (bind(sock,(struct sockaddr*)&unix_addr, sizeof(unix_addr)) < 0){
    perror("bind");
    close(sock);
    return -1;  
  }

  if (listen(sock, 10) < 0){
    perror("listen");
    close(sock);
    return -1;  
  }

  return sock;
}

void uscat_loop(int sock){
  while (1){
    struct timeval timeout;
    fd_set read_set;
    int max=0;
    int ret;

    timeout.tv_sec=1;
    timeout.tv_usec=0;

    FD_ZERO(&read_set);

    my_fd_set(&max,0,&read_set);
    my_fd_set(&max,sock,&read_set);
    
    ret=select(max+1,&read_set,NULL,NULL,&timeout);

    if (ret==0) continue;
    if (ret==-1) {
      perror("select");
      exit(1);
    }
    
    if (FD_ISSET(sock,&read_set)){
      char buf[BUF_SIZE];

      int length;
      if ((length = recv(sock, (void *)buf, BUF_SIZE - 1, 0)) <= 0){
	if (length ==0){
          close(sock);
          return;
        } else {
          perror("recv");
          exit(1);
        }
      }
      
      write(0,buf,length);
      
    }
    if (FD_ISSET(0,&read_set)){
      char buf[BUF_SIZE];

      int length;
      if ((length = read(0, (void *)buf, BUF_SIZE - 1)) <= 0){
	if (length ==0){
          close(sock);
          return;
        } else {
          perror("read");
          exit(1);
        }
      }
      
      write(sock,buf,length);
      
    }

  }
  
  close(sock);

}

int uscat_client(char* path){
  int fd = unix_connect(path);

  if (fd==-1)
    exit(1);

  uscat_loop(fd);
}

char **argv_p;
int sock;

void uscat_exec(int fd){
  int ret;

  if ((ret = fork()) == 0){
    // In child process

    dup2(fd,0); // stdin
    dup2(fd,1); // stdout
    close(sock);
    close(fd);

    execvp(argv_p[3], argv_p+3);

    close(0);
    close(1);

    fprintf(stderr, "%s: ", argv_p[3]);
    perror("execvp");
    exit(127);
  }else if (ret < 0){
    perror("fork");
    exit(1);
  }

  close(fd);
}

void collect_zombies(){
  int status;

  if (waitpid(-1,&status,WNOHANG) > 0){
  
    status = WEXITSTATUS(status);

    if (status!=0){
      fprintf(stderr, "Child exited with status %d\n", status);
      if (status==127)
        fprintf(stderr, "    Exec failed???\n");
    }
  }
}

int sigchld_handler(int n){
  collect_zombies();
  signal(SIGCHLD, sigchld_handler);
}

int uscat_server(char* path, int loop, void(*cb)(int fd)){
  sock = unix_listen(path);
  if (sock == -1)
    exit(1);

  signal(SIGCHLD, sigchld_handler);

  do {
    int client = accept(sock, NULL, NULL);

    if (client < 0){
      perror("accept");
      exit(1);
    }

    cb(client);

  } while (loop);

  close(sock);
}

void usage(char* pname){
  fprintf(stderr, "usage: %s <socket-path>\n",pname);
  fprintf(stderr, "       %s -l <socket-path>\n",pname);
  fprintf(stderr, "       %s -ll <socket-path>\n",pname);
  fprintf(stderr, "       %s -l <socket-path> <server-program> ...\n",pname);
  fprintf(stderr, "       %s -ll <socket-path> <server-program> ...\n",pname);
  exit(1);
}

int main (int argc, char ** argv){

  if (argc < 2){
    usage(argv[0]);
  }

  if (argc == 2){
    uscat_client(argv[1]);
  }else if (argc == 3){

    if (strcmp(argv[1],"-l") == 0){
      uscat_server(argv[2], 0, uscat_loop);
    }else if (strcmp(argv[1], "-ll") == 0){
      uscat_server(argv[2], 1, uscat_loop);
    }else{
      usage(argv[0]);
    }
  }else{

    argv_p = argv;

    if (strcmp(argv[1],"-l") == 0){
      uscat_server(argv[2], 0, uscat_exec);
    }else if (strcmp(argv[1], "-ll") == 0){
      uscat_server(argv[2], 1, uscat_exec);
    }else{
      usage(argv[0]);
    }


  }
}
