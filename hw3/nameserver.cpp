#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include <vector>
#include <unordered_map>
#include <fstream>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
  if (argc != 5)
  {
    perror("Usage: ./nameserver <log> <port> <geography_based> <servers>");
    return -1;
  }
  char* logPath = argv[1];
  char* port = argv[2];
  int geo = atoi(argv[3]);
  char* file = argv[4];
  std::cout<<logPath<<port<<geo<<file<<std::endl;
  return 0;
}