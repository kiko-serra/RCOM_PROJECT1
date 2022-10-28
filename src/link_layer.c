// Link layer protocol implementation

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <time.h>


#include "link_layer.h"
#include "alarm.h"
#include "sndtermios.h"
#include "statemachine.h"


// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

extern int alarmCount;
extern int alarmFired;

static LinkLayer conParam;
static int fd;

static int curr_num;

static int amnt_sets = 0;
static int amnt_uas = 0;
static int amnt_rejs = 0;
static int amnt_rrs = 0;
static int amnt_discs = 0;
static int amnt_is = 0;
static int r_amnt_sets = 0;
static int r_amnt_uas = 0;
static int r_amnt_rejs = 0;
static int r_amnt_rrs = 0;
static int r_amnt_discs = 0;
static int r_amnt_is = 0;

int amnt_bytes = 0;

clock_t start_t, end_t;
double total_t;


void print_statistics() {
    printf("----- STATISTICS --------\n");
    printf("SENT: SET: %d\n", amnt_sets);
    printf("SENT: UA: %d\n", amnt_uas);
    printf("SENT: REJ: %d\n", amnt_rejs);
    printf("SENT: RR: %d\n", amnt_rrs);
    printf("SENT: DISC: %d\n", amnt_discs);
    printf("SENT: I: %d\n", amnt_is);
    if(conParam.role == LlTx) {
        printf("SENT: BYTES: %d\n", amnt_bytes);
        printf("BYTES/SEC: %.4f\n", ((double) amnt_bytes) / total_t);
    }

    printf("\nRECEIVED: SET: %d\n", r_amnt_sets);
    printf("RECEIVED: UA: %d\n", r_amnt_uas);
    printf("RECEIVED: REJ: %d\n", r_amnt_rejs);
    printf("RECEIVED: RR: %d\n", r_amnt_rrs);
    printf("RECEIVED: DISC: %d\n", r_amnt_discs); 
    printf("RECEIVED: I: %d\n", r_amnt_is); 
    if(conParam.role == LlRx) {
        printf("RECEIVED: BYTES: %d\n", amnt_bytes);
        printf("BYTES/SEC: %.4f\n", ((double) amnt_bytes) / total_t);
    }
    printf("-------------------------\n");

}

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
        amnt_bytes+=FRAME_SIZE;
        amnt_sets++;
        alarm(conParam.timeout);

        // Receive UA
        status = receive_frame(fd, UA, LlRx);
        if(status < 0)
            alarmFired = FALSE;
        else {
            r_amnt_uas++;
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
    r_amnt_sets++;
    amnt_bytes+=FRAME_SIZE;


    // Send UA
    send_frame(fd, UA, LlRx, curr_num);
    amnt_uas++;
    return fd;
}

int llopen(LinkLayer connectionParameters)
{
    start_t = clock();
    // Setup alarm handler

    (void)signal(SIGALRM, alarmHandler);
    // Setup connection parameters
    conParam = connectionParameters;

    fd = setupTermios(conParam.serialPort, conParam.baudRate, conParam.role);

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
    //print_statistics();
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
    int rejCount = 0;
    while(alarmCount < conParam.nRetransmissions && rejCount < conParam.nRetransmissions && !stop) {
        send_information_frame(fd, frame, frameSize);
        amnt_bytes+=frameSize;
        amnt_is++;
        alarm(conParam.timeout);

        answer = receive_information_answer(fd, curr_num);
        switch (answer)
        {
        case RR:
            r_amnt_rrs++;
            alarm(0);
            stop = TRUE;
            break;

        case REJ:
            r_amnt_rejs++;
            alarm(0);
            alarmCount = 0;
            rejCount++;
            printf("llwrite: got rej, resending...\n");
            break;

        default:
            alarmFired = FALSE;
            break;
        }
    }
    if(alarmCount >= conParam.nRetransmissions || answer < 0 || rejCount >= conParam.nRetransmissions)
        return -1;

    return frameSize;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    //print_statistics();
    int frameSize, validPacket = FALSE, bufSize;
    unsigned char frame[MAX_PAYLOAD_SIZE];
    int rejCount = 0;
    while (!validPacket && rejCount < conParam.nRetransmissions)
    {
        int i = 0;
        int flags = 0;
        int bytes = 0;

        // falta fazer algo com o numero de i's!!!!

        while(flags < 2 && i < MAX_PAYLOAD_SIZE) {
            int auxBytes = receive_information_frame(fd, frame+i);

            // ESTES 2 MEGA IMPORTANTES
            if(frame[i] != FLAG && i == 0) continue;
            if(frame[i] == FLAG && i == 1 && flags == 1) continue;
            ////////////

            if(auxBytes != 1) {
                printf("received nothing\n");
                //perror("error: receive information frame failed");
                return -1;
            }
            // else printf("%x - ", frame[i]);
            if(frame[i] == FLAG){
               flags++;
            }
            bytes++;
            i++;
        }

        if(i >= MAX_PAYLOAD_SIZE) {
            send_frame(fd, REJ, LlRx, curr_num);
            continue;
        }

        r_amnt_is++;
        amnt_bytes+=i;

        frameSize = byte_destuffing(frame, bytes-6);
        bufSize = frameSize - 6;
        if((frame[1]^frame[2]) != frame[3]) {
            printf("BCC1 Failed.. %x -- %x\n", (frame[1]^frame[2]),frame[3]);
        }

        unsigned char BCC2 = frame[4];
        for(int i = 5; i < frameSize-2; i++) {
            BCC2 ^= frame[i];
        }
        
        if(BCC2 != frame[frameSize-2]) {
            printf("BCC2 Failed %x -- %x\n", BCC2 ,frame[frameSize-2]);
            if((int)frame[2] != curr_num) {
                send_frame(fd, RR, LlRx, curr_num);
                amnt_rrs++;
            }
            else {
                send_frame(fd, REJ, LlRx, curr_num);
                amnt_rejs++;
            }
            rejCount++;
            continue;
        }
        if((int)frame[2] != curr_num) {
            printf("Received duplicate, %d - %d\n", frame[2], curr_num);
            send_frame(fd, RR, LlRx, curr_num);
            amnt_rrs++;
            continue;
        }
        
        validPacket = TRUE;
        memcpy(packet, frame+4, bufSize);
    }
    if(rejCount >= conParam.nRetransmissions) {
        perror("Too many rejs\n");
        return -1;
    }
    //printf("llread: packet successfuly read\n");
    next_number();
    send_frame(fd, RR, LlRx, curr_num);
    amnt_rrs++;

    return bufSize;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////

// Close transmitter
int closeTx() {
    int confirmation;
    reset_alarm();
    while(alarmCount < conParam.nRetransmissions) {

        send_frame(fd, DISC, LlTx, curr_num);
        amnt_bytes+=FRAME_SIZE;
        amnt_discs++;
        alarm(conParam.timeout);
        confirmation = receive_frame(fd, DISC, LlRx);        
        
        if(confirmation < 0)
            alarmFired = FALSE;
        else {
            r_amnt_discs++;
            alarm(0);
            break;
        }     
    }
    

    if(confirmation < 0) {
        perror("error: didn't receive DISC frame\n");
        return -1;
    } else {
        send_frame(fd, UA, LlTx, curr_num);
        amnt_bytes+=FRAME_SIZE;
        amnt_uas++;

    }

    return 0;

}

int closeRx() {


    // Receive DISC
    if(receive_frame(fd, DISC, LlTx) < 0) return -1;
    amnt_bytes+=FRAME_SIZE;
    r_amnt_discs++;
    int confirmation;
    reset_alarm();
    while(alarmCount < conParam.nRetransmissions) {
        // Send DISC
        send_frame(fd, DISC, LlRx, curr_num);
        amnt_discs++;

        // Receive UA
        confirmation = receive_frame(fd, UA, LlTx);
        r_amnt_uas++;
        alarm(conParam.timeout);
        if(confirmation < 0)
            alarmFired = FALSE;
        else {
            amnt_bytes+=FRAME_SIZE;
            alarm(0);
            break;
        }   
    }
    if(alarmCount >= conParam.nRetransmissions) {
        perror("closeRx: timed out while receiving UA\n");
        return -1;
    }
    printf("Received UA..\n");

    return fd;
}


int llclose(int showStatistics)
{
    // In case the connection was abruptly lost
    if(showStatistics == -1) {
        restoreTermios(fd);
        return -1;
    }

    printf("Closing..\n");
    int ret = 1;
    if(conParam.role == LlTx) {
        if(closeTx() < 0) ret = -1;
    }
    else {
        if(closeRx() < 0) ret = -1;
    }

    end_t = clock();
    total_t = (double) (end_t - start_t) / CLOCKS_PER_SEC;
    //if(showStatistics)
        print_statistics();

    restoreTermios(fd);
    return ret;
}
