/*#include <stdio.h>
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
#include <sys/time.h>*/

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
#define MAXSIZE 32000
#define APMAX 32000
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int sendall(int s, char *buf, int len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = len; // how many we have left to send
    int n;

    while(total < len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 on failure, 0 on success
}
int get_content_length(const std::string &text) {
    unsigned long content_loc = text.find("Content-Length: ");
    char * cstr = new char [text.length()+1];
    std::strcpy(cstr, text.substr(content_loc, 26).c_str());

    char * p = std::strtok (cstr," ");
    while (p!=0)
    {
        if (atoi(p)) {
            return atoi(p);
        }
        p = std::strtok(NULL," ");
    }

    return 0;
}

int main(int argc, char** argv){
  struct timeval  time_1, time_2;
  int sockProx, sockBrow;  // listen on sock_fd, new connection on new_fd
  int sockServ;
	struct addrinfo hints, *servinfo, *p;
  struct addrinfo hints2, *servinfo2, *p2;
	struct sockaddr_storage their_addr; // connector's address information
  struct sockaddr_storage their_addr2;
	socklen_t sin_size;
	struct sigaction sa;
  socklen_t sin_size2;
	struct sigaction sa2;
	char s[INET6_ADDRSTRLEN];
  char s2[INET6_ADDRSTRLEN];
  int yes=1;
	int rv;
  char* port;
  char* apache = "80";
  char* host;
  double timer; 
	char req[MAXSIZE];
  char resp[MAXSIZE];
  int numbytesreq;
  int numbytesresp;
  long totalbytes = 0;
  char* logPath;
  float alpha;

  if(argc < 5 || argc > 5){
    printf("Error, arguments not right\n");
    return 0;
  }
  logPath = argv[1];
  alpha = atof(argv[2]);
  port = argv[3];
  host = argv[4];

//-------commmandlineargsends-------------
    
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  getaddrinfo(NULL, port, &hints, &servinfo);

  for(p = servinfo; p != NULL; p = p->ai_next) {
      if ((sockProx = socket(p->ai_family, p->ai_socktype,
              p->ai_protocol)) == -1) {
          perror("server: socket");
          continue;
      }

      if (setsockopt(sockProx, SOL_SOCKET, SO_REUSEADDR, &yes,
              sizeof(int)) == -1) {
          perror("setsockopt");
          exit(1);
      }

      if (bind(sockProx, p->ai_addr, p->ai_addrlen) == -1) {
          close(sockProx);
          perror("server: bind");
          continue;
      }
      break;
  }

  freeaddrinfo(servinfo);

  if (p == NULL)  {
      exit(1);
  }

  listen(sockProx, BACKLOG);
  sin_size = sizeof their_addr;
  sockBrow = accept(sockProx, (struct sockaddr *)&their_addr, &sin_size);
  inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
  gettimeofday(&time_1, NULL);
  printf("Recieved connection\n");

  //---------------------------------------------------browsertoproxyends-----------------------------------//
  memset(&hints2, 0, sizeof hints2);
  hints2.ai_family = AF_UNSPEC;
  hints2.ai_socktype = SOCK_STREAM;
  
  rv = getaddrinfo(host, apache, &hints2, &servinfo2);

  for(p2 = servinfo2; p2 != NULL; p2 = p2->ai_next) {
    if ((sockServ = socket(p2->ai_family, p2->ai_socktype,
        p2->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }

    if (connect(sockServ, p2->ai_addr, p2->ai_addrlen) == -1) {
      perror("client: connect");
      close(sockServ);
      continue;
    }

    break;
  }

  if (p2 == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }

  inet_ntop(p2->ai_family, get_in_addr((struct sockaddr *)p2->ai_addr), s2, sizeof s2);

  while(1){
    numbytesreq = recv(sockBrow, req, APMAX, 0);
    if(numbytesreq <= 0){
      close(sockBrow);
      close(sockProx);
      close(sockServ);
      return 0;
    }
    //req[numbytesreq] = '\0';
    printf("%s\n", req);
   // send(sockServ, req, APMAX, 0);
    //numbytesresp = recv(sockServ, resp, APMAX, 0);
   // resp[numbytesresp] = '\0';
   // printf("%s\n", resp);
    if (sendall(sockServ, req, numbytesreq) >= 0) {
      int res_size = recv(sockServ, resp, APMAX, 0);

      std::string response_text = resp;
      std::string header = response_text.substr(0, response_text.find("\r\n\r\n") + strlen("\r\n\r\n"));
      std::string content = response_text.substr(response_text.find("\r\n\r\n") + strlen("\r\n\r\n"));

       printf("%s\n", resp);
      int content_length = get_content_length(std::string(resp));
      int body_length = res_size - header.length() + 1;

      sendall(sockBrow, resp, res_size);

      while(body_length < content_length) {
          memset(resp, 0, sizeof(resp));
          res_size = recv(sockServ, resp, APMAX, 0);
          body_length += res_size;

          // send server response to browser
          sendall(sockBrow, resp, res_size);
      }
    }
   /* send(sockBrow, resp, APMAX, 0);
    while(numbytes > 1000){
      numbytes = recv(sockfd2, buf, 8000, 0);
      buf[numbytes] = '\0';
      printf("%s\n", buf);
      numbytes = send(new_fd, buf, 8000, 0);
      printf("here");
    }*/
  }
  /*  buf[numbytes] = '\0';
    printf("%s\n", buf);*/
  /*while(1){
    numbytes = recv(new_fd, buf, MAXSIZE, 0);
    buf[numbytes] = '\0';
    printf("%s\n", buf);
    numbytes = send(sockfd2, "here", MAXSIZE, 0);
    numbytes = recv(sockfd2, buf, MAXSIZE, 0);
    buf[numbytes] = '\0';
    printf("%s\n", buf);
    /*numbytes = recv(new_fd, buf, MAXSIZE, 0);
    buf[numbytes] = '\0';
    printf("%s\n", buf);*/
  //}
  /*while(1) {
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
    printf("%s\n", buf);
    totalbytes += numbytes;

  }
          
  else{
  char bufsend[MAXSIZE];
  memset(bufsend, '0', sizeof(bufsend));
  memset(&hints2, 0, sizeof hints2);
  hints2.ai_family = AF_UNSPEC;
  hints2.ai_socktype = SOCK_STREAM;
  
  rv = getaddrinfo(host, port, &hints2, &servinfo);

  for(p2 = servinfo; p2 != NULL; p2 = p2->ai_next) {
    if ((sockfd2 = socket(p2->ai_family, p2->ai_socktype,
        p2->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }

    if (connect(sockfd2, p2->ai_addr, p2->ai_addrlen) == -1) {
      perror("client: connect");
      close(sockfd2);
      continue;
    }

    break;
  }

  if (p2 == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }

  inet_ntop(p2->ai_family, get_in_addr((struct sockaddr *)p2->ai_addr), s2, sizeof s2);

  freeaddrinfo(servinfo2); 
  gettimeofday(&time_1, NULL);
  gettimeofday(&time_2, NULL);
  char end[MAXSIZE] = "here"; 
  //memset(end, '1', sizeof(end));
  numbytes = send(sockfd2, end, MAXSIZE, 0);
  numbytes = recv(sockfd2, buf, MAXSIZE, 0);
  buf[numbytes] = '\0';
  printf("%s\n", buf);
  /*while(1) {
    if(buf[numbytes - 1] == '1'){
      gettimeofday(&time_2, NULL);
      char end[MAXSIZE];
      memset(end, '1', sizeof(end));
      numbytes = send(sockfd, end, MAXSIZE, 0);
      close(sockfd);
      totalbytes -= numbytes;
      printf("recieved=%ld KB ", totalbytes/1000);
      printf("rate=%lf Mbps\n", ((8*totalbytes)/1000000)/((double) (time_2.tv_usec - time_1.tv_usec) / 1000000 +
          (double) (time_2.tv_sec - time_1.tv_sec)));
      return 0;
    }

    numbytes = recv(sockfd, buf, MAXSIZE, 0);
    buf[numbytes] = '\0';
    printf("%s\n", buf);
    totalbytes += numbytes;

  }
  gettimeofday(&time_2, NULL);
  printf("sent=%ld KB ", totalbytes/1000);
  printf("rate=%lf Mbps\n", ((8*totalbytes)/1000000)/((double) (time_2.tv_usec - time_1.tv_usec) / 1000000 +
          (double) (time_2.tv_sec - time_1.tv_sec)));
  close(sockfd);

  return 0;
 // }*/
} 
