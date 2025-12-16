#include "neighbor_detection.h"



int main() {
    pthread_t thread1, thread2,thread3;
    Neighbor neighbors[MAX_NEIGHBORS];
    int neighbor_count = 0;
    pthread_mutex_t neighbor_mutex = PTHREAD_MUTEX_INITIALIZER;

    memset(neighbors, 0, sizeof(neighbors));

    // Get local IP
    char ip[INET_ADDRSTRLEN];
    get_local_ip(ip);
    printf("Local IP: %s\n", ip);

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

    inputs *args = malloc(sizeof(inputs)); //inputs for threads
    if (!args) { perror("malloc"); close(sock); exit(1); }
    args->sock = sock;
    args->remote = remote;
    strcpy(args->ip, ip);
    args->neighbors = neighbors;
    args->neighbor_count = &neighbor_count;
    args->neighbor_mutex = &neighbor_mutex;

    pthread_create(&thread1, NULL, sending, (void*) args);
    pthread_create(&thread2, NULL, receiving, (void*) args);
    pthread_create(&thread3, NULL, BrokenLinks, (void*) args);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3,NULL);

    free(args);
    close(sock);
    return 0;
}
