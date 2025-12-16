
#include <pthread.h>
#include "routing.h"

int main(){
    pthread_t SendingThread,ReceivingThread;
    pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;
    // Get local IP
    char ip[INET_ADDRSTRLEN];
    get_local_ip(ip);
    printf("Local IP: %s\n", ip);

    // build broadcast socket
    // UDP socket enable broadcasting ipv4
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); exit(1); }

    int enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable)) < 0) { //allow broadcast
        perror("setsockopt SO_BROADCAST"); close(sock); exit(1);
    }

    /* BIND so we receive broadcast packets on BROADCAST_PORT */
    struct sockaddr_in local;
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port = htons(BROADCAST_PORT);
    local.sin_addr.s_addr = htonl(INADDR_ANY); //all interfaces

    if (bind(sock, (struct sockaddr*)&local, sizeof(local)) < 0) {
        perror("bind");
        close(sock);
        exit(1);
    }

    struct sockaddr_in remote;
    memset(&remote, 0, sizeof(remote));
    remote.sin_family = AF_INET;
    remote.sin_port = htons(BROADCAST_PORT);
    inet_pton(AF_INET, BROADCAST_IP, &remote.sin_addr);

    inputs *args = malloc(sizeof(inputs));
    if (!args) { perror("malloc"); close(sock); exit(1); }
    args->sock = sock;
    args->remote = remote;
    args->global_mutex=&global_mutex;



    //neighbor detection 
    // if broken links update dv
    // if new neigbhor update dv
    // send change if any
    // two threads one for recieving one for sending
    pthread_create(&SendingThread,NULL,Send,(void*) args);
    pthread_create(&ReceivingThread,NULL,Recv,(void*) args);

    pthread_join(SendingThread, NULL);
    pthread_join(ReceivingThread, NULL);

    free(args);
    close(sock);
    return 0;
}