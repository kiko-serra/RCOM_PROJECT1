// Link layer protocol implementation

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/time.h>


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

// Time only starts after sucessfuly coming out of llopen
struct timeval start_t, end_t;
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
        return ERROR_CMD;
    gettimeofday(&start_t, NULL);
    return 1;
}

int openRx() {
    // Receive SET
    if(receive_frame(fd, SET, LlTx) < 0) return ERROR_CMD;
    r_amnt_sets++;
    amnt_bytes+=FRAME_SIZE;


    // Send UA
    send_frame(fd, UA, LlRx, curr_num);
    amnt_uas++;
    gettimeofday(&start_t, NULL);
    return 1;
}

int llopen(LinkLayer connectionParameters)
{
    // Setup alarm handler

    (void)signal(SIGALRM, alarmHandler);
    // Setup connection parameters
    conParam = connectionParameters;

    fd = setupTermios(conParam.serialPort, conParam.baudRate, conParam.role);

    if(fd < 0) {
        perror("error: failed to setup new termios");
        return ERROR_CMD;
    }

    switch (conParam.role)
    {
        case LlTx: return openTx();
        case LlRx: return openRx();
    }
    
    return ERROR_CMD;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
    unsigned char informationFrame[MAX_PAYLOAD_SIZE];
    int stop = FALSE, answer;

    build_information_frame(informationFrame, buf, bufSize, curr_num);
    reset_alarm();
    next_number();

    int frameSize = byte_stuffing(informationFrame, bufSize);
    int rejCount = 0;

    while(alarmCount < conParam.nRetransmissions && rejCount < conParam.nRetransmissions && !stop) {
        send_information_frame(fd, informationFrame, frameSize);
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
        return ERROR_CMD;

    return frameSize;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    //print_statistics();
    int frameSize, validPacket = FALSE, bufSize;
    unsigned char informationFrame[MAX_PAYLOAD_SIZE];
    int rejCount = 0;
    while (!validPacket && rejCount < conParam.nRetransmissions)
    {
        int flags = 0;
        int bytes = 0;

        while(flags < 2 && bytes < MAX_PAYLOAD_SIZE) {
            int auxBytes = receive_information_frame(fd, informationFrame+bytes);

            if(auxBytes != 1) {
                printf("llread: Didn't receive anything..\n");
                return ERROR_CMD;
            }
            // ESTES 2 MEGA IMPORTANTES
            if(informationFrame[bytes] != FLAG && bytes == 0) continue;
            if(informationFrame[bytes] == FLAG && bytes == 1 && flags == 1) continue;
            ////////////

            if(informationFrame[bytes] == FLAG){
               flags++;
            }
            bytes++;
        }

        // Send rej in case of receiving too many bytes before finding the 2 flags
        if(bytes >= MAX_PAYLOAD_SIZE) {
            send_frame(fd, REJ, LlRx, curr_num);
            continue;
        }

        r_amnt_is++;
        amnt_bytes+=bytes;

        frameSize = byte_destuffing(informationFrame, bytes-6);
        bufSize = frameSize - 6;
        if((informationFrame[1]^informationFrame[2]) != informationFrame[3]) {
            printf("llread: BCC1 Failed.. %x -- %x\n", (informationFrame[1]^informationFrame[2]),informationFrame[3]);
        }

        unsigned char BCC2 = informationFrame[4];
        for(int i = 5; i < frameSize-2; i++) {
            BCC2 ^= informationFrame[i];
        }
        
        if(BCC2 != informationFrame[frameSize-2]) {
            printf("llread: BCC2 Failed %x -- %x\n", BCC2 ,informationFrame[frameSize-2]);
            if((int)informationFrame[2] != curr_num) {
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
        if((int)informationFrame[2] != curr_num) {
            printf("llread: Received duplicate, %d - %d\n", informationFrame[2], curr_num);
            send_frame(fd, RR, LlRx, curr_num);
            amnt_rrs++;
            continue;
        }
        
        validPacket = TRUE;
        memcpy(packet, informationFrame+4, bufSize);
    }
    if(rejCount >= conParam.nRetransmissions) {
        perror("llread: Got too many rejecteds\n");
        return ERROR_CMD;
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
        return ERROR_CMD;
    } else {
        send_frame(fd, UA, LlTx, curr_num);
        amnt_bytes+=FRAME_SIZE;
        amnt_uas++;

    }

    return 0;

}

int closeRx() {


    // Receive DISC
    if(receive_frame(fd, DISC, LlTx) < 0) return ERROR_CMD;
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
        return ERROR_CMD;
    }
    printf("closeRx: Received UA sucessfully\n Shutting down..\n");

    return fd;
}


int llclose(int showStatistics)
{
    // In case the connection was abruptly lost
    if(showStatistics == ERROR_CMD) {
        restoreTermios(fd);
        return ERROR_CMD;
    }

    printf("llclose: Closing..\n");
    int ret = 1;
    if(conParam.role == LlTx) {
        if(closeTx() < 0) ret = ERROR_CMD;
    }
    else {
        if(closeRx() < 0) ret = ERROR_CMD;
    }


    gettimeofday(&end_t, NULL);
    total_t = (double) (end_t.tv_sec - start_t.tv_sec)*1000000;
    total_t += (double) (end_t.tv_usec - start_t.tv_usec);
    total_t /= 1000000;

    if(showStatistics)
        print_statistics();

    restoreTermios(fd);
    return ret;
}
