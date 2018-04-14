#include "miProxy.h"

int main(int argc, char** argv){

  if(argc != 7){
    printf("Error, arguments not right\n");
    return 0;
  }
  char* logPath = argv[1];
  float alpha = atof(argv[2]);
  char* port = argv[3];
  char* dns_ip = argv[4];
  char* dns_port = argv[5];
  char* www_ip = argv[6];

  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int rv;
  int numbytes;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  if ((rv = getaddrinfo(dns_ip, dns_port, &hints, &servinfo)) != 0) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
      return 1;
  }

  // loop through all the results and make a socket
  for(p = servinfo; p != NULL; p = p->ai_next) {
      if ((sockfd = socket(p->ai_family, p->ai_socktype,
              p->ai_protocol)) == -1) {
          perror("talker: socket");
          continue;
      }

      break;
  }

  if (p == NULL) {
      fprintf(stderr, "talker: failed to create socket\n");
      return 2;
  }

  if ((numbytes = sendto(sockfd, "hello", 5, 0,
            p->ai_addr, p->ai_addrlen)) == -1) {
      perror("talker: sendto");
      exit(1);
  }

  freeaddrinfo(servinfo);

  printf("talker: sent %d bytes to %s\n", numbytes, dns_port);
  close(sockfd);


  /*Proxy newProxy = Proxy(logPath, alpha, port, host);
  newProxy.setConnection();
  newProxy.runProxy();*/
  return 0;
} 
