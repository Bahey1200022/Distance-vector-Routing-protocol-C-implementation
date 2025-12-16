#ifndef ROUTER_H
#define ROUTER_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <ctype.h>
#include <pthread.h>
#include <bits/pthreadtypes.h>
#define MAX_NEIGHBORS 100
#define MAX_DEST 100

#define BROADCAST_IP "255.255.255.255"
#define BROADCAST_PORT 5555 
//each endpoint has a table that contains : list of reachable dests, how to reach of them , cost to reach
typedef struct DvEntry {
char Ip[INET_ADDRSTRLEN]; //main ip
char destination[MAX_DEST][INET_ADDRSTRLEN]; //each dest it knows about
int distances[MAX_DEST]; //best next hop dist
int via_count;
}DvEntry;
typedef struct Neighbor {
    char ip[INET_ADDRSTRLEN];
    int lastSequenceNumber;
    time_t lastHeard;
} Neighbor;

typedef struct inputs {
    int sock;
    struct sockaddr_in remote;
    pthread_mutex_t* global_mutex;  
}inputs;


extern int dv_entry_count;
extern DvEntry dv_table[MAX_DEST];
extern bool updatedDV;
extern char Myip[INET_ADDRSTRLEN];
extern Neighbor neighbors[MAX_NEIGHBORS];
extern int neigbhors;
extern bool brokenLinks;

void get_local_ip(char *buffer);
void dvSent();
void dvUpdate();
char* getDistanceVector();
void processDistanceVector(char* DV);
void * Send(void * args);
void * Recv(void * args);


#endif