
#ifndef DV_H
#define DV_H
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
#define MAX_DEST 100
#define MAX_N 30



//each endpoint has a table that contains : list of reachable dests, how to reach of them , cost to reach
typedef struct DvEntry {
char Ip[INET_ADDRSTRLEN]; //main ip
char destination[MAX_DEST][INET_ADDRSTRLEN]; //each dest it knows about
int distances[MAX_DEST]; //best next hop dist
int via_count;
}DvEntry;
extern int dv_entry_count;
extern DvEntry dv_table[MAX_DEST];
extern bool updatedDV;
extern char Myip[INET_ADDRSTRLEN];
void SetIp(char *ip);
void dvUpdate();
void dvSent();
char* getDistanceVector();
void processDistanceVector(char* DV);
void printdv();
#endif