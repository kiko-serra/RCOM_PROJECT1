#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "alarm.h"
#include "setstatemachine.h"
#include "frame.h"

extern int alarmFired;

void build_frame (FrameType type, unsigned char* frame, LinkLayerRole role) {
    unsigned char A = getAddressByte (type, role);
    unsigned char C = getControlByte(type);
    unsigned char BCC = A ^ C;
    unsigned char disc[FRAME_SIZE] = {FLAG, A, C, BCC, FLAG};
    memcpy(frame, disc, FRAME_SIZE);
}


void send_frame(int fd, FrameType type, LinkLayerRole role) {
    unsigned char frame[FRAME_SIZE];
    build_frame(type, frame, role);
    int bytes = write(fd, frame, FRAME_SIZE);
    if(bytes == -1) {
        perror("error: failed to write");
    }
}

int receive_frame(int fd, FrameType type, LinkLayerRole role) {
    unsigned char frame[FRAME_SIZE], byte;
    StateMachine *sm = createStateMachine(type, role);
    if(sm == NULL){
        perror("error: failed to create State Machine");
    }

    while(sm->state != STOP) {
        if(read(fd, &byte, 1) > 0) {
            handleStateMachine(sm, byte, frame);
        }
        if(alarmFired) {
            deleteStateMachine(sm);
            return -1;
        }
    }
    
    deleteStateMachine(sm);
    return 0;
}

