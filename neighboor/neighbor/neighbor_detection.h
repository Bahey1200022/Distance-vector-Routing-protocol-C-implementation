#ifndef NEIGHBOR_DETECTION_H
#define NEIGHBOR_DETECTION_H

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
#include <pthread.h>
#include <time.h>

#define BROADCAST_IP "255.255.255.255"
#define BROADCAST_PORT 5555 
#define MAX_NEIGHBORS 100

typedef struct Neighbor {
    char ip[INET_ADDRSTRLEN];
    int lastSequenceNumber;
    time_t lastHeard;
} Neighbor;

typedef struct inputs {
    int sock;
    struct sockaddr_in remote;
    char ip[INET_ADDRSTRLEN];

    Neighbor* neighbors;              
    int* neighbor_count;              
    
    pthread_mutex_t* neighbor_mutex;  
    pthread_mutex_t* dv_mutex;  
} inputs;

void get_local_ip(char *buffer);
void* sending(void* arg);
void * receiving(void* arg);
void * BrokenLinks(void *arg);

#endif