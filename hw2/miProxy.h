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
const char* apache = "80";

class Proxy{
private:
  struct timeval  time_1, time_2;
  int sockProx, sockBrow, sockServ;
  char* port;
  char* host;
  double timer; 
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

    int getContentLen(const std::string inp) {
        int position_cl = inp.find("Content-Length: ");//16

        char* str = new char [inp.length()+1];
        strcpy (str, inp.c_str());
        int start = position_cl + 16;
        int count = start;
        int end = 0;
        bool found = false;
        while(!found){
            if(str[count] == '\n'){
                end = count - 1;
                found = true;
            }
            count++;
        }
        return atol(inp.substr(start, end - start).c_str());
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
        std::sort(lst.begin(), lst.end(), std::greater<int>());
        return lst;
    }

    void endSockets(){
        close(sockBrow);
        close(sockProx);
        close(sockServ);
    }
    int sendFromServer(){
        char resp[MAXSIZE];
        int numbytes = recv(sockServ, resp, APMAX, 0);
        std::string header = std::string(resp).substr(0, std::string(resp).find("\r\n\r\n") + strlen("\r\n\r\n"));
        int contLen = getContentLen(std::string(resp));
        int bytesPassed = numbytes - header.length();

        send(sockBrow, resp, numbytes, 0);

        while(bytesPassed < contLen) {
            numbytes = recv(sockServ, resp, APMAX, 0);
            bytesPassed += numbytes;
            send(sockBrow, resp, numbytes, 0);
        }
        return contLen + header.length();
    }

    const char* getChunk(char* req){
        std::string inp = std::string(req);
        int position_chunk = inp.find(" ");//16
        char* str = new char [inp.length()+1];
        strcpy (str, inp.c_str());
        int start = position_chunk + 1;
        int count = start;
        int end = 0;
        bool found = false;
        while(!found){
            if(str[count] == ' '){
                end = count - 1;
                found = true;
            }
            count++;
        }
        return inp.substr(start, end - start + 1).c_str();
    }
    int getRate(double throughput){
        for(int i = 0; i < bitrates.size(); i++){
            if(bitrates[i]*1.5 <= throughput){
                return bitrates[i];
            }
        }
        return 0;
    }

    int runProxy(){
        int numbytesreq;
        int numbytesresp;
        char req[MAXSIZE];
        char resp[MAXSIZE];
        struct timeval  time_start, time_fin;
        std::ofstream outputFile;
        double throughputest = 0;
        outputFile.open(logPath);
        while(1){
            numbytesreq = recv(sockBrow, req, APMAX, 0);
            if(numbytesreq <= 0){
                endSockets();
                outputFile.close();
                return 0;
            }
            gettimeofday(&time_start, NULL);
            std::string wrappedReq = std::string(req);
            int pos = wrappedReq.find(".f4m");
            std::string nolistreq;
            if(pos != std::string::npos){
                bitrates = getBitrates(wrappedReq);
                send(sockServ, req, numbytesreq, 0);
                recv(sockServ, resp, APMAX, 0);
                bitrates = getBitrates(resp);
                nolistreq = wrappedReq.substr(0, pos) + "_nolist" + wrappedReq.substr(pos);
                strncpy(req, nolistreq.c_str(), APMAX);
                numbytesreq += strlen("_nolist");
            }
            
            std::string req_text = std::string(req);
            int pos_seg = req_text.find("Seg");
            if(pos_seg != std::string::npos){
                std::string req_text = std::string(req);
                std::string rate = std::to_string(getRate(throughputest));
                int pos_seg = req_text.find("Seg");
                int start = pos_seg;
                while(req[start] != '/'){
                    start--;
                }
                std::string newRate = req_text.substr(0, start + 1) + rate + req_text.substr(pos_seg);
                strncpy(req, newRate.c_str(), APMAX);
                numbytesreq += (rate.length() - (pos_seg - start - 1));
            }
            
            send(sockServ, req, numbytesreq, 0);
            int passed = sendFromServer();
            gettimeofday(&time_fin, NULL);
            double elapsed = (double) (time_fin.tv_usec - time_start.tv_usec) / 1000000 + (double) (time_fin.tv_sec - time_start.tv_sec);
            double throughput_curr = ((double) passed / elapsed)/1000;
            const char* chunk = getChunk(req);
            outputFile<<elapsed<<" "<<throughput_curr<<" "<<throughputest<<" "<<getRate(throughputest)<<" "<<host<<" "<<chunk<<std::endl;
            outputFile.flush();
            throughputest = (alpha*throughput_curr) + (1 - alpha)*throughputest;

       }
       outputFile.close();
    }   
};


