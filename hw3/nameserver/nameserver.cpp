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
#include "DNSLib.h"
#include <sstream>
#include <climits>

#define BACKLOG 10
#define APMAX 32000
#define MAXSIZE 32000

struct timeval  time_1, time_2;
double timer; 
char* logPath;
int sockfd;
struct addrinfo hints, *servinfo, *p;
struct sockaddr_storage their_addr;
socklen_t addr_len = sizeof their_addr;
char s[INET6_ADDRSTRLEN];
std::vector<std::string> ip_list;
int ind = 0;
int size = 0;
int num_nodes;
std::vector<std::vector<int>> graph; 
std::vector<int> id;
std::vector<std::string> node_type;
std::vector<std::string> node_ip;
std::string curr_ip;

void populate_list(char* filename);
std::string get_next_ip();
int setConnection(char* port);
void populate_geo(char* filename);
std::string get_closest();
int minimum_dist(int dist[], bool sptSet[]);

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
    char buf[APMAX];
    int numbytes;
    if(geo == 1){
        populate_geo(file);
    }
    else{
        populate_list(file);
    }
    setConnection(port);
    std::ofstream outputFile;
    outputFile.open(logPath);
    while(1){
        struct DNSReq req;
        if ((numbytes = recvfrom(sockfd, (char*) &req, APMAX , 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        if(std::string(req.question.QNAME) != "video.cs.jhu.edu"){
            req.header.RCODE = 3;
            outputFile<<"DNS Resolution failed"<<std::endl;
            outputFile.flush();
            sendto(sockfd, (char *) &req, sizeof(req), 0, (struct sockaddr *)&their_addr, addr_len);
        }
        else{
            curr_ip = std::string(inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));
            if(geo == 0){
                strcpy(req.record.RDATA, get_next_ip().c_str());
            }
            else{
                strcpy(req.record.RDATA, get_closest().c_str());
            }
            strcpy(req.record.NAME, req.question.QNAME);
            req.record.TYPE = 1;
            req.record.CLASS = 1;
            req.record.RDLENGTH = sizeof(req.record.RDATA);
            outputFile<<curr_ip<<" "<<req.question.QNAME<<" "<<req.record.RDATA<<std::endl;
            outputFile.flush();
            numbytes = sendto(sockfd, (char *) &req, sizeof(req), 0, (struct sockaddr *)&their_addr, addr_len);
        }
    }


    close(sockfd);
    return 0;
}
void populate_list(char* filename){
    std::ifstream servers;
    servers.open(filename);
    std::string str;
    while (std::getline(servers, str))
    {
        ip_list.push_back(str);
    }
    servers.close();
    size = ip_list.size();

}
void populate_geo(char* filename){
    std::ifstream servers;
    servers.open(filename);
    std::string str;
    std::getline(servers, str);
    std::stringstream s(str);
    std::string dump;
    s>>dump>>num_nodes;
    std::vector<std::vector<int>> graph_temp(num_nodes, std::vector<int>(num_nodes));
    int i = num_nodes;
    while(i > 0){
        s.str("");
        s.clear();
        std::getline(servers, str);
        s.str(str);
        int val;
        std::string type;
        std::string ip;
        s>>val>>type>>ip;
        id.push_back(val);
        node_type.push_back(type);
        node_ip.push_back(ip);
        i--;
    }   
    s.str("");
    s.clear();
    std::getline(servers, str);
    s.str(str);
    int num_links;
    s>>dump>>num_links;
    int j = num_links;
    while(j > 0){
        s.str("");
        s.clear();
        std::getline(servers, str);
        s.str(str);
        int source;
        int dest;
        int cost;
        s>>source>>dest>>cost;
        graph_temp[source][dest] = cost;
        graph_temp[dest][source] = cost;
        j--;
    }
    graph = graph_temp;
}
std::string get_next_ip(){
    std::string val = ip_list.at(ind % size);  
    ind++;
    return val;            
}

int setConnection(char* port){
    int rv;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    return 1;
}


std::string get_closest(){
    int dist[num_nodes];
    int min = INT_MAX;
    int server_ind = 0;
    bool set[num_nodes]; 
    int src = 0;

    for(int i = 0; i < node_ip.size(); i++){
        if(node_type[i] == "CLIENT" && curr_ip == node_ip[i]){
            src = i;
        }
    }
    for (int i = 0; i < num_nodes; i++){
        dist[i] = INT_MAX;
        set[i] = false;
    }

    dist[src] = 0;
    for (int count = 0; count < num_nodes-1; count++)
    {
    int u = minimum_dist(dist, set);

    set[u] = true;

    for (int v = 0; v < num_nodes; v++)
        if (!set[v] && graph[u][v] && dist[u] != INT_MAX 
                                    && dist[u]+graph[u][v] < dist[v])
        dist[v] = dist[u] + graph[u][v];
    }

    for (int i = 0; i < (sizeof(dist)/sizeof(int)); i++) {
        if (node_type[i] == "SERVER" && dist[i] <= min) {
            min = dist[i];
            server_ind = i;
        }
    }

    return node_ip[server_ind];
}

int minimum_dist(int dist[], bool set[])
{
   int min = INT_MAX, min_index;
  
   for (int v = 0; v < num_nodes; v++)
     if (set[v] == false && dist[v] <= min)
         min = dist[v], min_index = v;
  
   return min_index;
}
