// Link layer protocol implementation

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#include "link_layer.h"
#include "alarm.h"
#include "sndtermios.h"
#include "setstatemachine.h"


// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

extern int alarmCount;
extern int alarmFired;

static LinkLayer conParam;
static int fd;

////////////////////////////////////////////////
// LLOPEN                                     //
////////////////////////////////////////////////
int openTx() {

    while(alarmCount < conParam.nRetransmissions) {
        // Send SET
        send_frame(fd, SET, LlTx);
        alarm(conParam.timeout);

        // Receive UA
        if(receive_frame(fd, UA, LlRx) < 0)
            alarmFired = FALSE;
        else {
            alarm(0);
            break;
        }     
    }
    return fd;
}

int openRx() {

    // Receive SET
    if(receive_frame(fd, SET, LlTx) < 0) return -1;

    // Send UA
    send_frame(fd, UA, LlRx);
    return fd;
}

int llopen(LinkLayer connectionParameters)
{
    // Setup alarm handler
    (void)signal(SIGALRM, alarmHandler);
    // Setup connection parameters
    conParam = connectionParameters;

    fd = setupTermios(conParam.serialPort, conParam.baudRate, conParam.role == LlTx ? 0 : 1);

    if(fd < 0)
        perror("error: failed to setup new termios");

    switch (conParam.role)
    {
        case LlTx: return openTx();
        case LlRx: return openRx();
    }

    return -1;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
    // TODO

    return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    // TODO

    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int closeTx() {
    int confirmation;
    while(alarmCount < conParam.nRetransmissions) {

        send_frame(fd, DISC, LlTx);
        alarm(conParam.timeout);
        confirmation = receive_frame(fd, DISC, LlRx);
        
        if(confirmation < 0)
            alarmFired = FALSE;
        else {
            alarm(0);
            break;
        }     
    }

    if(confirmation < 0) {
        perror("error: didn't receive DISC frame\n");
        return -1;
    } else {
        send_frame(fd, UA, LlTx);
    }

    return 0;

}

int closeRx() {


    // Receive DISC
    if(receive_frame(fd, DISC, LlTx) < 0) return -1;

    // Send DISC
    send_frame(fd, DISC, LlRx);

    // Receive UA
    if(receive_frame(fd, UA, LlTx) < 0) return -1;
    printf("Received UA..\n");

    return fd;
}


int llclose(int showStatistics)
{
    int ret = fd;
    if(conParam.role == LlTx) {
        if(closeTx() < 0) ret = -1;
    }
    else {
        if(closeRx() < 0) ret = -1;
    }

    restoreTermios(fd);
    return ret;
}
