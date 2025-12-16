#include "dv.h"
#include <stdio.h>
// getting the ip and dv table from string with the help of AI 
int dv_entry_count=0;
char Myip[INET_ADDRSTRLEN] = {0};
bool updatedDV=true;
DvEntry dv_table[MAX_DEST];     

void SetIp(char *ip){
    strncpy(Myip,ip,INET_ADDRSTRLEN);
}

void dvUpdate() {
    updatedDV = true;
}

void dvSent() {
    updatedDV = false;
}

char* getDistanceVector() {
    size_t buf_size = 4096;
    char *buffer = malloc(buf_size);
    if (!buffer) return NULL;

    buffer[0] = '\0';

    // prefix: "my_ip:DV:"
    snprintf(buffer, buf_size, "%s:DV:", Myip);
    size_t used = strlen(buffer);

    // Build "dest:distance,dest:distance,..."
    for (int i = 0; i < dv_entry_count; i++) {


      
            int min =__INT_MAX__;

        for (int j = 0; j < dv_table[i].via_count; j++) {

            if (min > dv_table[i].distances[j]) min=dv_table[i].distances[j];
        
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
void processDistanceVector(char* DV)
{
    if (!DV) return;

    char copy[4096];
    strncpy(copy, DV, sizeof(copy)-1);
    copy[sizeof(copy)-1] = '\0';  // safe null-termination

    
    char* colon = strchr(copy, ':');
    if (!colon) return;
    *colon = '\0';
    char sender_ip[INET_ADDRSTRLEN];
    strncpy(sender_ip, copy, INET_ADDRSTRLEN);

    char* dv_ptr = colon + 1;
    if (strncmp(dv_ptr, "DV:", 3) != 0) return;
    dv_ptr += 3;  // skip "DV:"

    printf("Sender IP: %s\n", sender_ip);
    printf("DV list: %s\n", dv_ptr);

    // Find or create sender entry
    int sender_index = -1;
    for (int i = 0; i < dv_entry_count; i++) {
        if (strcmp(dv_table[i].Ip, sender_ip) == 0) {
            sender_index = i;
            printf("found\n");
            break;
        }
    }
    if (sender_index == -1) {
        sender_index = dv_entry_count++;
        strncpy(dv_table[sender_index].Ip, sender_ip, INET_ADDRSTRLEN);
        printf("%s added\n",sender_ip);
    }
    dv_table[sender_index].via_count = 0;

   
    char* ptr = dv_ptr;
    while ((ptr = strchr(ptr, '(')) != NULL) {
        char* end = strchr(ptr, ')');
        if (!end) break;  

        *end = '\0';  
        char* pair = ptr + 1;  

        char* comma = strchr(pair, ',');
        if (!comma) {
            ptr = end + 1;
            continue;  
        }
        *comma = '\0';
        char* dest = pair;
        char* dist_str = comma + 1;

      

        int dist = atoi(dist_str);

        int idx = dv_table[sender_index].via_count++;
        strncpy(dv_table[sender_index].destination[idx], dest, INET_ADDRSTRLEN);
        dv_table[sender_index].distances[idx] = dist;

        printf("Parsed pair: %s -> %d\n", dest, dist);

        ptr = end + 1;  // move past current pair
    }

    /// now for the actual algo

    bool changed = false;

    for (int j = 0; j < dv_table[sender_index].via_count; j++) {
        char* dest = dv_table[sender_index].destination[j];
        int new_dist = dv_table[sender_index].distances[j] + 1;

        if (strcmp(dest, Myip) == 0) continue;
        printf("dest %s\n",dest);

        // find or create entry for dest
        int my_index = -1;
        for (int i = 0; i < dv_entry_count; i++) {
            if (strcmp(dv_table[i].Ip, dest) == 0) {
                my_index = i;
                break;
            }
        }
                // first route: direct neighbor → distance = 1

        if (my_index == -1) {
            my_index = dv_entry_count++;
            strncpy(dv_table[my_index].Ip, dest, INET_ADDRSTRLEN);
            dv_table[my_index].via_count = 1;
            strncpy(dv_table[my_index].destination[0], sender_ip, INET_ADDRSTRLEN);
            dv_table[my_index].distances[0] = new_dist;
            printf("new dest entry added %s\n",dv_table[my_index].Ip);
            changed = true;
            continue;
        }

        // Check if path via this sender exists
        int existing_path = -1;
        for (int k = 0; k < dv_table[my_index].via_count; k++) {
            if (strcmp(dv_table[my_index].destination[k], sender_ip) == 0) {
                existing_path = k;
                break;
            }
        }

        if (existing_path != -1) {
            if (new_dist < dv_table[my_index].distances[existing_path]) {
                dv_table[my_index].distances[existing_path] = new_dist;
                changed = true;
                printf("distnace updated\n");
            }
        } 
        else {
            int slot = dv_table[my_index].via_count++;
            strncpy(dv_table[my_index].destination[slot], sender_ip, INET_ADDRSTRLEN);
            dv_table[my_index].distances[slot] = new_dist;
            changed = true;
            printf("added new path %s with %d\n",sender_ip,new_dist);
        }
    }

    if (changed)
        dvUpdate();
}

void printdv() {
    printf("=== Distance Vector Table ===\n");
    for (int i = 0; i < dv_entry_count; i++) {
        printf("Destination: %s\n", dv_table[i].Ip);
        printf("  Paths via %d next-hop(s):\n", dv_table[i].via_count);
        for (int j = 0; j < dv_table[i].via_count; j++) {
            printf("    Via %s -> Distance %d\n",
                   dv_table[i].destination[j],
                   dv_table[i].distances[j]);
        }
        printf("\n");
    }
}