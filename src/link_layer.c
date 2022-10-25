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

static int curr_num;

void reset_alarm() {
    alarmCount = 0;
}

void next_number() {
    curr_num = (curr_num+1)%2;
}

////////////////////////////////////////////////
// LLOPEN                                     //
////////////////////////////////////////////////
int openTx() {
    int status;
    while(alarmCount < conParam.nRetransmissions) {
        // Send SET
        send_frame(fd, SET, LlTx, curr_num);
        alarm(conParam.timeout);

        // Receive UA
        status = receive_frame(fd, UA, LlRx);
        if(status < 0)
            alarmFired = FALSE;
        else {
            alarm(0);
            break;
        }     
    }
    if(alarmCount >= conParam.nRetransmissions || status < 0)
        return -1;
    return fd;
}

int openRx() {

    // Receive SET
    if(receive_frame(fd, SET, LlTx) < 0) return -1;

    // Send UA
    send_frame(fd, UA, LlRx, curr_num);
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
    unsigned char frame[MAX_PAYLOAD_SIZE];
    int stop = FALSE, answer;
    build_information_frame(frame, buf, bufSize, curr_num);
    reset_alarm();
    next_number();
    // do byte stuffing
    // for(int i = 0; i < bufSize+6; i++) printf("%x - ", frame[i]);
    // printf("\n BEFORE\n");
    int frameSize = byte_stuffing(frame, bufSize);
    // for(int i = 0; i < frameSize; i++) printf("%x - ", frame[i]);
    // printf("\n AFTER\n");

    // printf("before stuffing: %d - after stuffing: %d \n", bufSize, frameSize);

    while(alarmCount < conParam.nRetransmissions && !stop) {
        send_information_frame(fd, frame, frameSize);
        alarm(conParam.timeout);

        answer = receive_information_answer(fd, curr_num);
        switch (answer)
        {
        case RR:
            alarm(0);
            stop = TRUE;
            break;

        case REJ:
            alarm(0);
            alarmCount = 0;
            break;

        default:
            alarmFired = FALSE;
            break;
        }
    }
    if(alarmCount >= conParam.nRetransmissions || answer < 0)
        return -1;

    return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    int frameSize, validFrame = FALSE, bufSize;
    unsigned char frame[MAX_PAYLOAD_SIZE];
    int i = 0;
    while (!validFrame)
    {
        int flags = 0;
        int bytes = 0;
        while(flags < 2) {
            int auxBytes = receive_information_frame(fd, frame+i);
            if(auxBytes != 1) {
                printf("received nothing\n");
                //perror("error: receive information frame failed");
                return -1;
            }
            // else printf("%x - ", frame[i]);
            if(frame[i] == FLAG){
               flags++;
               //printf("flag found\n");
            }
            bytes++;
            i++;
        }

        frameSize = byte_destuffing(frame, bytes-6);
        bufSize = frameSize - 6;
        if((frame[1]^frame[2]) != frame[3]) {
            printf("BCC1 Failed.. %x -- %x\n", (frame[1]^frame[2]),frame[3]);
        }

        if((int)frame[2] != curr_num) {
            printf("Received duplicate, %d - %d\n", frame[2], curr_num);
            send_frame(fd, RR, LlRx, curr_num);
            continue;
        }

        unsigned char BCC2 = frame[4];
        for(int i = 5; i < frameSize-2; i++) {
            BCC2 ^= frame[i];
        }
        
        if(BCC2 != frame[frameSize-2]) {
            printf("BCC2 Failed %x -- %x\n", BCC2 ,frame[4]);
            send_frame(fd, REJ, LlRx, curr_num);
            continue;
        }
        else {
            validFrame = TRUE;
            memcpy(packet, frame+4, bufSize);
        }
    }
    next_number();
    send_frame(fd, RR, LlRx, curr_num);

    return bufSize;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int closeTx() {
    int confirmation;
    while(alarmCount < conParam.nRetransmissions) {

        send_frame(fd, DISC, LlTx, curr_num);
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
        send_frame(fd, UA, LlTx, curr_num);
    }

    return 0;

}

int closeRx() {


    // Receive DISC
    if(receive_frame(fd, DISC, LlTx) < 0) return -1;

    // Send DISC
    send_frame(fd, DISC, LlRx, curr_num);

    // Receive UA
    if(receive_frame(fd, UA, LlTx) < 0) return -1;
    printf("Received UA..\n");

    return fd;
}


int llclose(int showStatistics)
{
    int ret = 1;
    if(conParam.role == LlTx) {
        if(closeTx() < 0) ret = -1;
    }
    else {
        if(closeRx() < 0) ret = -1;
    }

    restoreTermios(fd);
    return ret;
}
