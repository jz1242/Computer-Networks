#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <csignal>
#include <cstring>
#include <sys/time.h>
#include <cstdlib>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <algorithm>



#define BACKLOG 10
#define APMAX 32000
#define MAXSIZE 32000

struct timeval  time_1, time_2;
int sockName;
double timer; 
char* logPath;


int setConnection(char* port);

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char** argv) {
  if (argc != 5)
  {
    perror("Usage: ./nameserver <log> <port> <geography_based> <servers>");
    return -1;
  }
  logPath = argv[1];
  char* port = argv[2];
  int geo = atoi(argv[3]);
  char* file = argv[4];
  char buf[1000];
  int numbytes;
  setConnection(port);
  while(1) {
  /*if(buf[numbytes - 1] == '1'){
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
  }*/
  numbytes = recv(sockName, buf, 1000, 0);
  buf[numbytes] = '\0';
  printf("%s", buf);

}
  return 0;
}

int setConnection(char* port){
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr; 
  socklen_t sin_size;
  struct sigaction sa;
  char s[INET6_ADDRSTRLEN];
  int yes=1;
  int rv;
  int sockfd;
  memset(&hints, 0, sizeof hints);
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
  sockName = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
  inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);

}