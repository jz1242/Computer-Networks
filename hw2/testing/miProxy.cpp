#include "miProxy.h"

int main(int argc, char** argv){

  if(argc < 5 || argc > 5){
    printf("Error, arguments not right\n");
    return 0;
  }
  char* logPath = argv[1];
  float alpha = atof(argv[2]);
  char* port = argv[3];
  char* host = argv[4];
  Proxy newProxy = Proxy(logPath, alpha, port, host);
  newProxy.setConnection();
  newProxy.runProxy();
  return 0;
} 
