// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    LinkLayer *ll = malloc(sizeof(LinkLayer));
    strcpy(ll->serialPort, serialPort);
    if(!strcmp(*role, "tx")) {
        ll->role = LlTx;
    } else if(!strcmp(*role, "rx")) {
        ll->role = LlRx;
    } else {
        perror("error: role not defined");
    }
    ll->baudRate = baudRate;
    ll->timeout = timeout;
    ll->nRetransmissions = nTries;

    if(!llopen(*ll)) {
        perror("error: llopen failed");
    }
}
