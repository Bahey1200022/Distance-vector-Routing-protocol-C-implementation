
// ref : AI tools , https://stackoverflow.com/questions/337422/how-to-udp-broadcast-with-c-in-linux , threads from previous labs

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
#include "neighbor_detection.h"

void get_local_ip(char *buffer) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); exit(1); }

    struct sockaddr_in remote;
    memset(&remote, 0, sizeof(remote));
    remote.sin_family = AF_INET;
    remote.sin_port = htons(80);  
    inet_pton(AF_INET, "8.8.8.8", &remote.sin_addr);

    if (connect(sock, (struct sockaddr*)&remote, sizeof(remote)) < 0) {
        perror("connect"); close(sock); exit(1);
    }

    struct sockaddr_in local;
    socklen_t len = sizeof(local);
    if (getsockname(sock, (struct sockaddr*)&local, &len) < 0) {
        perror("getsockname"); close(sock); exit(1);
    }

    inet_ntop(AF_INET, &local.sin_addr, buffer, INET_ADDRSTRLEN);
    close(sock);
}

void* sending(void* arg) {
    inputs* args = (inputs*) arg;
    int sock = args->sock;
    struct sockaddr_in remote = args->remote;
    char* my_ip = args->ip;
    unsigned short sequenceNumber = 0;

    while (1) {
        char message[256];
        snprintf(message, sizeof(message), "%s:HELLO:%u", my_ip, sequenceNumber); //put msg in buffer

        if (sendto(sock, message, strlen(message), 0,
                   (struct sockaddr*)&remote, sizeof(remote)) < 0) {
            perror("sendto");
        }

        

        sequenceNumber++;
        sleep(5); // send every 5 seconds
    }

    return NULL;
}

void* receiving(void* arg) {
    inputs* args = (inputs*) arg;
    int sock = args->sock;
    char* my_ip = args->ip;

    while (1) {
        char buffer[256];
        struct sockaddr_in sender;
        socklen_t sender_len = sizeof(sender);

        int n = recvfrom(sock, buffer, sizeof(buffer)-1, 0,
                         (struct sockaddr*)&sender, &sender_len);
        if (n < 0) { perror("recvfrom"); continue; }
        if (n == 0) continue;
        buffer[n] = '\0';

        char sender_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &sender.sin_addr, sender_ip, sizeof(sender_ip)); //get sender ip 

        if (strcmp(sender_ip, my_ip) == 0) continue; // ignore self

        else printf("received from ip %s \n",sender_ip);

        char *saveptr = NULL;
        char *token = strtok_r(buffer, ":", &saveptr); // IP part (not used)
        if (!token) continue;
        token = strtok_r(NULL, ":", &saveptr); // should be "HELLO"
        if (!token) continue;
        if (strcmp(token, "HELLO") != 0) continue;
        token = strtok_r(NULL, ":", &saveptr); // sequence number
        if (!token) continue;
        /* token now points to sequence number string */
        int seq = atoi(token); // atoi is fine here because token not NULL

        pthread_mutex_lock(args->neighbor_mutex);

        int found = 0;
        for (int i = 0; i < *(args->neighbor_count); i++) {
            if (strcmp(args->neighbors[i].ip, sender_ip) == 0) {
                args->neighbors[i].lastSequenceNumber = seq;
                args->neighbors[i].lastHeard = time(NULL);
                found = 1;
                break;
            }
        }

        if (!found && *(args->neighbor_count) < MAX_NEIGHBORS) {
            /* use strncpy for safety */
            strncpy(args->neighbors[*(args->neighbor_count)].ip, sender_ip, INET_ADDRSTRLEN-1);
            args->neighbors[*(args->neighbor_count)].ip[INET_ADDRSTRLEN-1] = '\0';
            args->neighbors[*(args->neighbor_count)].lastSequenceNumber = seq;
            args->neighbors[*(args->neighbor_count)].lastHeard = time(NULL);
            (*(args->neighbor_count))++;
        }


        pthread_mutex_unlock(args->neighbor_mutex);
    }

    return NULL;
}

void* BrokenLinks(void* arg) {
    inputs* args = (inputs*) arg;

    while (1) {
        pthread_mutex_lock(args->neighbor_mutex);

        time_t now = time(NULL);

    for (int i = *(args->neighbor_count) - 1; i >= 0; i--) {
    double diff = difftime(now, args->neighbors[i].lastHeard);

    if (diff > 10) {
        printf("Neighbor %s timed out. Removing.\n", args->neighbors[i].ip);

        // Swap with last element (faster than shifting all)
        args->neighbors[i] = args->neighbors[*(args->neighbor_count) - 1];
        (*(args->neighbor_count))--;
    }
}


        pthread_mutex_unlock(args->neighbor_mutex);

        sleep(1); 
    }

    return NULL;
}
