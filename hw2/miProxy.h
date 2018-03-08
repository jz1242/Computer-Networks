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
const char* apache = "80";

class Proxy{
private:
  struct timeval  time_1, time_2;
  int sockProx, sockBrow, sockServ;
  char* port;
  char* host;
  double timer; 
  char req[MAXSIZE];
  char resp[MAXSIZE];
  int numbytesreq;
  int numbytesresp;
  char* logPath;
  float alpha;
  std::vector<int> bitrates;
public:
    Proxy(char* logInp, float alphaInp, char* portInp, char* hostInp):logPath(logInp), alpha(alphaInp), port(portInp), host(hostInp){}

    int setConnection(){
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
        return 1;

    }

    void *get_in_addr(struct sockaddr *sa)
    {
        if (sa->sa_family == AF_INET) {
            return &(((struct sockaddr_in*)sa)->sin_addr);
        }

        return &(((struct sockaddr_in6*)sa)->sin6_addr);
    }

    /*int sendall(int s, char *buf, int len){
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
    }*/

    int getContentLen(const std::string text) {
        int position_cl = text.find("Content-Length: ");//16

        char* cstr = new char [text.length()+1];
        std::strcpy (cstr, text.c_str());
        int start = position_cl + 16;
        int count = start;
        int end = 0;
        bool found = false;
        while(!found){
            if(cstr[count] == '\n'){
                end = count - 1;
                found = true;
            }
            count++;
        }
        return atol(text.substr(start, end - start).c_str());
    }

    std::vector<int> getBitrates(const std::string& xml) {
        using std::string;
        string keyword = "bitrate=\"";
        std::vector<int> lst;
        for(size_t tagPos = xml.find("<media"); tagPos != string::npos; tagPos = xml.find("<media", tagPos + 1)) {
            size_t keyPos = xml.find(keyword, tagPos);
            if(keyPos == string::npos)
                continue;
            int bitrateLoc = keyPos + keyword.size();
            int len = xml.find('"', bitrateLoc) - bitrateLoc;
            int bitrate = std::stoi(xml.substr(bitrateLoc, len));
            lst.push_back(bitrate);
        }
        return lst;
    }

    void endSockets(){
        close(sockBrow);
        close(sockProx);
        close(sockServ);
    }

    int runProxy(){
        while(1){
            numbytesreq = recv(sockBrow, req, APMAX, 0);
            if(numbytesreq <= 0){
                endSockets();
                return 0;
            }
            std::string wrappedReq = std::string(req);
            int pos = wrappedReq.find(".f4m");
            std::string nolistreq;
            if(pos != std::string::npos){
                bitrates = getBitrates(wrappedReq);
                nolistreq = wrappedReq.substr(0, pos) + "_nolist" + wrappedReq.substr(pos);
                strncpy(req, nolistreq.c_str(), APMAX);
                numbytesreq += strlen("_nolist");
            }
            printf("%s", req);
            if (send(sockServ, req, numbytesreq, 0)) {
                int res_size = recv(sockServ, resp, APMAX, 0);

                std::string response_text = resp;
                std::string header = response_text.substr(0, response_text.find("\r\n\r\n") + strlen("\r\n\r\n"));
                //std::string content = response_text.substr(response_text.find("\r\n\r\n") + strlen("\r\n\r\n"));

                //printf("%s\n", resp);
                int content_length = getContentLen(std::string(resp));
                int body_length = res_size - header.length() + 1;

                send(sockBrow, resp, res_size, 0);

                while(body_length < content_length) {
                    memset(resp, 0, sizeof(resp));
                    res_size = recv(sockServ, resp, APMAX, 0);
                    body_length += res_size;

                    // send server response to browser
                    send(sockBrow, resp, res_size, 0);
                }
            }
        }
    }   
};


