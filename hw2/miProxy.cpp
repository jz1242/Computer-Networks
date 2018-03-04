#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <iostream>
#include <signal.h>
#include <sys/time.h>

#define BACKLOG 10
#define MAXSIZE 1000

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char** argv){
  struct timeval  time_1, time_2;
  int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
  char* port;
  int isServer = 0;
  char* host;
  double timer; 
	char buf[MAXSIZE];
  int numbytes;
  long totalbytes = 0;
  char* logPath;
  float alpha;
  char* ip;

  if(argc < 5 || argc > 5){
    printf("Error, arguments not right\n");
    return 0;
  }
  logPath = argv[1];
  alpha = atof(argv[2]);
  port = argv[3];
  host = argv[4];
    
  /*memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  rv = getaddrinfo(NULL, port, &hints, &servinfo);

  for(p = servinfo; p != NULL; p = p->ai_next) {
      if ((sockfd = socket(p->ai_family, p->ai_socktype,
              p->ai_protocol)) == -1) {
          perror("server: socket");
          continue;
      }

      if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
              sizeof(int)) == -1) {
          perror("setsockopt");
          exit(1);
      }

      if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
          close(sockfd);
          perror("server: bind");
          continue;
      }
      break;
  }

  freeaddrinfo(servinfo);

  if (p == NULL)  {
      exit(1);
  }

  listen(sockfd, BACKLOG);
  sin_size = sizeof their_addr;
  new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
  inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
  gettimeofday(&time_1, NULL);
  while(1) {
    if(buf[numbytes - 1] == '1'){
      gettimeofday(&time_2, NULL);
      char end[MAXSIZE];
      memset(end, '1', sizeof(end));
      numbytes = send(new_fd, end, MAXSIZE, 0);
      close(new_fd);
      close(sockfd);
      totalbytes -= numbytes;
      printf("recieved=%ld KB ", totalbytes/1000);
      printf("rate=%lf Mbps\n", ((8*totalbytes)/1000000)/((double) (time_2.tv_usec - time_1.tv_usec) / 1000000 +
          (double) (time_2.tv_sec - time_1.tv_sec)));
      return 0;
    }

    numbytes = recv(new_fd, buf, MAXSIZE, 0);
    buf[numbytes] = '\0';
    totalbytes += numbytes;

  }
            
  else{*/
  char bufsend[MAXSIZE];
  memset(bufsend, '0', sizeof(bufsend));
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  
  rv = getaddrinfo(host, port, &hints, &servinfo);

  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
        p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      perror("client: connect");
      close(sockfd);
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);

  freeaddrinfo(servinfo); 
  gettimeofday(&time_1, NULL);
  gettimeofday(&time_2, NULL);
  while(((double) (time_2.tv_usec - time_1.tv_usec) / 1000000 + (double) (time_2.tv_sec - time_1.tv_sec)) < timer){
    numbytes = send(sockfd, bufsend, MAXSIZE, 0);
    totalbytes += numbytes;
    gettimeofday(&time_2, NULL);
  }
  char end[MAXSIZE];
  memset(end, '1', sizeof(end));
  numbytes = send(sockfd, end, MAXSIZE, 0);
  while(1){
    numbytes = recv(sockfd, buf, MAXSIZE, 0);
    if(buf[numbytes - 1] == '1'){
      break;
    }

  }
  gettimeofday(&time_2, NULL);
  printf("sent=%ld KB ", totalbytes/1000);
  printf("rate=%lf Mbps\n", ((8*totalbytes)/1000000)/((double) (time_2.tv_usec - time_1.tv_usec) / 1000000 +
          (double) (time_2.tv_sec - time_1.tv_sec)));
  close(sockfd);

  return 0;
  //}*/
} 