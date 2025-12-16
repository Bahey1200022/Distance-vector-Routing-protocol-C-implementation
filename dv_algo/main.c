
#include "dv.h"
#include <stdbool.h>


int main(){

  
     // Get local IP
    char My_Ip[INET_ADDRSTRLEN] = "192.168.1.10";
     SetIp(My_Ip);

    //sending : (IP:DV:()) empty
    char *dv=getDistanceVector();
    printf("%s\n",dv);

    //receiving a dv from a neighboor
    char* incoming="192.168.1.11:DV:(192.168.1.10,1):(192.168.1.80,9):(192.168.1.16,1):";
    processDistanceVector(incoming);
    incoming="192.168.1.19:DV:(192.168.1.10,1):(192.168.1.13,6):(192.168.1.70,9):(192.168.1.6,1):";
    processDistanceVector(incoming);
    // printdv();
    incoming="192.168.1.70:DV:(192.168.1.10,1):";
    processDistanceVector(incoming);
    dv=getDistanceVector();
    printf("%s\n",dv);
    incoming="192.168.1.19:DV:(192.168.1.99,5):";
    processDistanceVector(incoming);
    dv=getDistanceVector();
    printf("%s\n",dv);
    incoming="192.168.1.19:DV:(192.168.1.80,5):";
    processDistanceVector(incoming);
    dv=getDistanceVector();
    printf("%s\n",dv);


    




    //sending new dv 
    dvSent();



    return 0;
}