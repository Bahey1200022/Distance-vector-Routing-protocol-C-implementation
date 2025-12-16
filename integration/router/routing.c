#include "routing.h"

int dv_entry_count = 0;
char Myip[INET_ADDRSTRLEN] = {0};
bool updatedDV = true;
bool brokenLinks = false;
DvEntry dv_table[MAX_DEST];
Neighbor neighbors[MAX_NEIGHBORS];
int neigbhorsCount = 0;

void get_local_ip(char *buffer)
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in remote;
    memset(&remote, 0, sizeof(remote));
    remote.sin_family = AF_INET;
    remote.sin_port = htons(80);
    inet_pton(AF_INET, "8.8.8.8", &remote.sin_addr);

    if (connect(sock, (struct sockaddr *)&remote, sizeof(remote)) < 0)
    {
        perror("connect");
        close(sock);
        exit(1);
    }

    struct sockaddr_in local;
    socklen_t len = sizeof(local);
    if (getsockname(sock, (struct sockaddr *)&local, &len) < 0)
    {
        perror("getsockname");
        close(sock);
        exit(1);
    }

    inet_ntop(AF_INET, &local.sin_addr, buffer, INET_ADDRSTRLEN);
    strncpy(Myip, buffer, INET_ADDRSTRLEN);
    close(sock);
}
void dvUpdate()
{
    updatedDV = true;
}

void dvSent()
{
    updatedDV = false;
}

char *getDistanceVector()
{
    size_t buf_size = 4096;
    char *buffer = malloc(buf_size);
    if (!buffer)
        return NULL;

    buffer[0] = '\0';

    // prefix: "my_ip:DV:"
    snprintf(buffer, buf_size, "%s:DV:", Myip);
    size_t used = strlen(buffer);

    // Build "dest:distance,dest:distance,..."
    for (int i = 0; i < dv_entry_count; i++)
    {

        int min = __INT_MAX__;

        for (int j = 0; j < dv_table[i].via_count; j++)
        {

            if (min > dv_table[i].distances[j])
                min = dv_table[i].distances[j];
        }
        // Compose "dest:distance"
        char entry[128];
        snprintf(entry, sizeof(entry), "(%s,%d)",
                 dv_table[i].Ip,
                 min);

        size_t entry_len = strlen(entry);

        if (used + entry_len + 2 >= buf_size)
            break;

        memcpy(buffer + used, entry, entry_len);
        used += entry_len;

        buffer[used++] = ':';
    }

    return buffer;
}
void processDistanceVector(char *DV)
{
    if (!DV)
        return;

    char copy[4096];
    strncpy(copy, DV, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0'; // safe null-termination

    char *colon = strchr(copy, ':');
    if (!colon)
        return;
    *colon = '\0';
    char sender_ip[INET_ADDRSTRLEN];
    strncpy(sender_ip, copy, INET_ADDRSTRLEN);

    char *dv_ptr = colon + 1;
    if (strncmp(dv_ptr, "DV:", 3) != 0)
        return;
    dv_ptr += 3; // skip "DV:"

    printf("Sender IP: %s\n", sender_ip);
    printf("DV list: %s\n", dv_ptr);

    // Find or create sender entry
    int sender_index = -1;
    for (int i = 0; i < dv_entry_count; i++)
    {
        if (strcmp(dv_table[i].Ip, sender_ip) == 0)
        {
            sender_index = i;
            printf("found\n");
            break;
        }
    }
    if (sender_index == -1)
    {
        sender_index = dv_entry_count++;
        strncpy(dv_table[sender_index].Ip, sender_ip, INET_ADDRSTRLEN);
        printf("%s added\n", sender_ip);
    }
    dv_table[sender_index].via_count = 0;

    char *ptr = dv_ptr;
    while ((ptr = strchr(ptr, '(')) != NULL)
    {
        char *end = strchr(ptr, ')');
        if (!end)
            break;

        *end = '\0';
        char *pair = ptr + 1;

        char *comma = strchr(pair, ',');
        if (!comma)
        {
            ptr = end + 1;
            continue;
        }
        *comma = '\0';
        char *dest = pair;
        char *dist_str = comma + 1;

        int dist = atoi(dist_str);

        int idx = dv_table[sender_index].via_count++;
        strncpy(dv_table[sender_index].destination[idx], dest, INET_ADDRSTRLEN);
        dv_table[sender_index].distances[idx] = dist;

        printf("Parsed pair: %s -> %d\n", dest, dist);

        ptr = end + 1; // move past current pair
    }

    /// now for the actual algo

    bool changed = false;

    for (int j = 0; j < dv_table[sender_index].via_count; j++)
    {
        char *dest = dv_table[sender_index].destination[j];
        int new_dist = dv_table[sender_index].distances[j] + 1;

        if (strcmp(dest, Myip) == 0)
            continue;
        printf("dest %s\n", dest);

        // find or create entry for dest
        int my_index = -1;
        for (int i = 0; i < dv_entry_count; i++)
        {
            if (strcmp(dv_table[i].Ip, dest) == 0)
            {
                my_index = i;
                break;
            }
        }

        if (my_index == -1)
        {
            my_index = dv_entry_count++;
            strncpy(dv_table[my_index].Ip, dest, INET_ADDRSTRLEN);
            dv_table[my_index].via_count = 1;
            strncpy(dv_table[my_index].destination[0], sender_ip, INET_ADDRSTRLEN);
            dv_table[my_index].distances[0] = new_dist;
            printf("new dest entry added %s\n", dv_table[my_index].Ip);
            changed = true;
            continue;
        }

        // Check if path via this sender exists
        int existing_path = -1;
        for (int k = 0; k < dv_table[my_index].via_count; k++)
        {
            if (strcmp(dv_table[my_index].destination[k], sender_ip) == 0)
            {
                existing_path = k;
                break;
            }
        }

        if (existing_path != -1)
        {
            if (new_dist < dv_table[my_index].distances[existing_path])
            {
                dv_table[my_index].distances[existing_path] = new_dist;
                changed = true;
                printf("distnace updated\n");
            }
        }
        else
        {
            int slot = dv_table[my_index].via_count++;
            strncpy(dv_table[my_index].destination[slot], sender_ip, INET_ADDRSTRLEN);
            dv_table[my_index].distances[slot] = new_dist;
            changed = true;
            printf("added new path %s with %d\n", sender_ip, new_dist);
        }
    }

    if (changed)
        dvUpdate();
}

void *Send(void *args)
{
    inputs *arg = (inputs *)args;
    int sock = arg->sock;
    struct sockaddr_in remote = arg->remote;
    unsigned short sequenceNumber = 0;
    while (1)
    {
        // just send hellos
        char message[256];
        snprintf(message, sizeof(message), "%s:HELLO:%u", Myip, sequenceNumber); // put msg in buffer

        if (sendto(sock, message, strlen(message), 0,
                   (struct sockaddr *)&remote, sizeof(remote)) < 0)
        {
            perror("sendto");
        }
        sequenceNumber++;

        // at the same time check if there're any neighbours that have disappeared

        pthread_mutex_lock(arg->global_mutex);

        time_t now = time(NULL);
        char broken[MAX_NEIGHBORS][INET_ADDRSTRLEN];
        int brokenCount = 0;

        for (int i = 0; i < neigbhorsCount; i++)
        {
            double diff = difftime(now, neighbors[i].lastHeard);

            if (diff > 10)
            {
                printf("Neighbor %s timed out. Removing.\n", neighbors[i].ip);

                strncpy(broken[brokenCount], neighbors[i].ip, INET_ADDRSTRLEN);
                broken[brokenCount][INET_ADDRSTRLEN - 1] = '\0';
                brokenCount++;

                // ---- Remove neighbor by swapping with last one ----
                neighbors[i] = neighbors[neigbhorsCount - 1];
                neigbhorsCount--;

                brokenLinks = true;

                i--; // re-check the swapped neighbor
            }
        }

        if (brokenLinks)
        {
            dvUpdate();

            // remove as entry and dests
            for (int b = 0; b < brokenCount; b++)
            {
                char *deadIP = broken[b];

                // Remove DV entries where the destination itself is the dead IP
                for (int e = 0; e < dv_entry_count; e++)
                {

                    // If the DVEntry main IP itself is dead → remove entry
                    if (strcmp(dv_table[e].Ip, deadIP) == 0)
                    {
                        printf("Removing DV entry for %s\n", deadIP);

                        dv_table[e] = dv_table[dv_entry_count - 1];
                        dv_entry_count--;

                        e--; // Re-check swapped entry
                        continue;
                    }

                    // Remove any destinations in this DVEntry that go through deadIP
                    for (int d = 0; d < dv_table[e].via_count; d++)
                    {

                        // Check if path uses the broken neighbor as next hop
                        if (strcmp(dv_table[e].destination[d], deadIP) == 0)
                        {

                            printf("Removing route %s → via %s\n",
                                   dv_table[e].Ip, deadIP);

                            // Remove dest by swap
                            strncpy(dv_table[e].destination[d],
                                    dv_table[e].destination[dv_table[e].via_count - 1],
                                    INET_ADDRSTRLEN);

                            dv_table[e].distances[d] =
                                dv_table[e].distances[dv_table[e].via_count - 1];

                            dv_table[e].via_count--;

                            d--; // re-check swapped
                        }
                    }
                }
            }
        }
        pthread_mutex_unlock(arg->global_mutex);

        if (updatedDV)
        {

            char *dv = getDistanceVector();
            printf("dv sent %s\n",dv);

            if (sendto(sock, dv, strlen(dv), 0,
                       (struct sockaddr *)&remote, sizeof(remote)) < 0)
            {
                perror("sendto");
            }
            dvSent();
        }

        sleep(3);
    }

    return NULL;
}
void *Recv(void *args)
{
    // receive an incoming message on the socket & detect and handle if it is a message from the router itself

    inputs *arg = (inputs *)args;
    int sock = arg->sock;
    while (1)
    {
        char buffer[256];
        struct sockaddr_in sender;
        socklen_t sender_len = sizeof(sender);

        int n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                         (struct sockaddr *)&sender, &sender_len);
        if (n < 0)
        {
            perror("recvfrom");
            continue;
        }
        if (n == 0)
            continue;
        buffer[n] = '\0';

        char sender_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &sender.sin_addr, sender_ip, sizeof(sender_ip)); // get sender ip

        if (strcmp(sender_ip, Myip) == 0)
            continue; // ignore self

        else
            printf("received from ip %s \n", sender_ip);
        // see if next is a dv or a hello
        char *saveptr = NULL;
        char *token = strtok_r(buffer, ":", &saveptr); // IP part (not used)
        if (!token)
            continue;
        token = strtok_r(NULL, ":", &saveptr); // should be "HELLO"
        if (!token)
            continue;
        pthread_mutex_lock(arg->global_mutex);

        if (strcmp(token, "HELLO") != 0)
        {

            printf("dv recv %s\n",buffer);

            processDistanceVector(buffer);

        }
        else
        {
            token = strtok_r(NULL, ":", &saveptr); // sequence number
            if (!token)
                continue;
            /* token now points to sequence number string */
            int seq = atoi(token); // atoi is fine here because token not NULL
            int found = 0;
            for (int i = 0; i < neigbhorsCount; i++)
            {
                if (strcmp(neighbors[i].ip, sender_ip) == 0)
                {
                    neighbors[i].lastSequenceNumber = seq;
                    neighbors[i].lastHeard = time(NULL);
                    found = 1;
                    break;
                }
            }

            if (!found && neigbhorsCount < MAX_NEIGHBORS)
            {
                strncpy(neighbors[neigbhorsCount].ip, sender_ip, INET_ADDRSTRLEN - 1);
                neighbors[neigbhorsCount].ip[INET_ADDRSTRLEN - 1] = '\0';
                neighbors[neigbhorsCount].lastSequenceNumber = seq;
                neighbors[neigbhorsCount].lastHeard = time(NULL);
                neigbhorsCount++;
                // put in dv also
                int idx = dv_entry_count++;

                // main ip
                strncpy(dv_table[idx].Ip, sender_ip, INET_ADDRSTRLEN);
                dv_table[idx].Ip[INET_ADDRSTRLEN - 1] = '\0';

                // first route: direct neighbor → distance = 1
                strncpy(dv_table[idx].destination[0], sender_ip, INET_ADDRSTRLEN);
                dv_table[idx].destination[0][INET_ADDRSTRLEN - 1] = '\0';

                dv_table[idx].distances[0] = 1;
                dv_table[idx].via_count = 1;

                printf("DV: added direct route to %s (dist=1)\n", sender_ip);

                dvUpdate();
            }
        }

        pthread_mutex_unlock(arg->global_mutex);
    }
    return NULL;
}
