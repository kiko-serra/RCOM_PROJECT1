// Application layer protocol implementation

#include <string.h>
#include <stdio.h>  

#include "application_layer.h"
#include "link_layer.h"

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    LinkLayer ll;
    strcpy(ll.serialPort, serialPort);
    if(!strcmp(role, "tx")) {
        ll.role = LlTx;
    } else if(!strcmp(role, "rx")) {
        ll.role = LlRx;
    } else {
        perror("error: role not defined");
    }
    ll.baudRate = baudRate;
    ll.timeout = timeout;
    ll.nRetransmissions = nTries;

    if(!llopen(ll)) {
        perror("error: llopen failed");
    }

    // JUST NEED TO READ/WRITE

    if (llclose(0) < 0) {
        perror("error: failed to terminate connection\n");
    } else {
        printf("Closed connection successfuly\n");
    }
}
