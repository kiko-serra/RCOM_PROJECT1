#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "alarm.h"
#include "setstatemachine.h"
#include "frame.h"

extern int alarmFired;

// Frames of type SET & UA
void build_frame (FrameType type, unsigned char* frame, LinkLayerRole role) {
    unsigned char A = getAddressByte (type, role);
    unsigned char C = getControlByte(type);
    unsigned char BCC = A ^ C;
    unsigned char disc[FRAME_SIZE] = {FLAG, A, C, BCC, FLAG};
    memcpy(frame, disc, FRAME_SIZE);
}

// Frames of type I
void build_information_frame(unsigned char* frame, const unsigned char* data, int length, int curr_num) {
    frame[0] = FLAG;
    frame[1] = A_TRANSMITTER_CMD;
    frame[2] = curr_num ? C_CTRL_2 : C_CTRL_1;
    frame[3] = frame[1] ^ frame[2];
    memcpy(frame + 4, data, length * sizeof (unsigned char));
    unsigned char BCC2 = data[0];
    for(int i = 1; i < length; i++) 
        BCC2 ^= data[i];
    frame[4 + length] = BCC2;
    frame[5 + length] = FLAG;  
}


void send_frame(int fd, FrameType type, LinkLayerRole role, int curr_num) {
    unsigned char frame[FRAME_SIZE];
    build_frame(type, frame, role);
    int bytes = write(fd, frame, FRAME_SIZE);
    if(bytes == -1) {
        perror("error: failed to write");
    }
}

int send_information_frame(int fd, unsigned char* frame, int length) {
    int bytesWritten = 0;
    while(bytesWritten != length) {
        int bytes = write(fd, frame+bytesWritten, length - bytesWritten);
        if(bytes <0) {
            perror("error: failed to send frame\n");
            return -1;
        }
        bytesWritten += bytes;
    }

    return 0;
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

int receive_information_frame(int fd, unsigned char* frame) {
    return read(fd, frame, MAX_PAYLOAD_SIZE);
}

int receive_information_answer(int fd, int curr_num) {
    unsigned char rr[FRAME_SIZE], rej[FRAME_SIZE], byte;
    int ret = -1;
    StateMachine *rrSm = createStateMachine(RR, LlRx);
    if(rrSm == NULL){
        perror("error: failed to create State Machine");
    }
    StateMachine *rejSm = createStateMachine(REJ, LlRx);
    if(rejSm == NULL){
        deleteStateMachine(rrSm);
        perror("error: failed to create State Machine");
    }

    while(rrSm->state != STOP && rejSm->state != STOP) {
        if(read(fd, &byte, 1) > 0) {
            handleStateMachine(rrSm, byte, rr);
            handleStateMachine(rejSm, byte, rej);
        }
        if(alarmFired) {
            deleteStateMachine(rrSm);
            deleteStateMachine(rejSm);
            return ret;
        }
    }
    if(rrSm->state == STOP)
        ret = RR;
    else 
        ret = REJ;
    deleteStateMachine(rrSm);
    deleteStateMachine(rejSm);


    return ret;

}

int byte_stuffing(unsigned char* frame, int length) {
    unsigned char aux_frame[MAX_PAYLOAD_SIZE];
    memcpy(aux_frame, frame, 4);

    int data_pos = 4;

    for(int i = 4; i < length+4; i++) {
        if(frame[i] == FLAG) {
            aux_frame[data_pos++] = ESC;
            aux_frame[data_pos++] = COMP_FLAG;
        }
        else if (frame[i] == ESC) {
            aux_frame[data_pos++] = frame[i];
            aux_frame[data_pos++] = COMP_ESC;
        }
        else {
            aux_frame[data_pos++] = frame[i];
        }
    }

    // Pass BCC2 & FLAG to auxiliar frame
    memcpy(aux_frame + data_pos, frame+length+4, 2);
    data_pos+=2;
    memcpy(frame, aux_frame, data_pos);
    return data_pos;
}

int byte_destuffing(unsigned char* frame, int length) {
     unsigned char aux_frame[MAX_PAYLOAD_SIZE];
    memcpy(aux_frame, frame, 4);

    int data_pos = 4;

    for(int i = 4; i < length+4; i++) {
        if(frame[i] == ESC) {
            if(frame[i+1] == COMP_FLAG) {
                aux_frame[data_pos++] = FLAG;  
            }
            else
                aux_frame[data_pos++] = ESC;
        }
        else {
            aux_frame[data_pos++] = frame[i];
        }
    }

    // Pass BCC2 & FLAG to auxiliar frame
    memcpy(aux_frame + data_pos, frame+length+4, 2);
    data_pos+=2;
    memcpy(frame, aux_frame, data_pos);
    return data_pos;
}

