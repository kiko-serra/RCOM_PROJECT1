#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "alarm.h"
#include "statemachine.h"
#include "frame.h"
#include "constants.h"

extern int alarmFired;
extern int amnt_bytes;

void build_frame (unsigned char* frame, FrameType type, LinkLayerRole role) {
    frame[0] = FLAG;
    frame[1] = getAddressByte (type, role);
    frame[2] = getControlByte(type);
    frame[3] = frame[1] ^ frame[2];
    frame[4] = FLAG;
}

// Frames of type I
void build_information_frame(unsigned char* frame, const unsigned char* data, int length, int curr_num) {
    frame[0] = FLAG;
    frame[1] = A_TRANSMITTER_CMD;
    frame[2] = curr_num ? C_CTRL_2 : C_CTRL_1;
    frame[3] = frame[1] ^ frame[2];
    memcpy(frame + 4, data, length * sizeof(unsigned char));
    unsigned char BCC2 = 0;
    for(int i = 0; i < length; i++) 
        BCC2 ^= data[i];
    frame[4 + length] = BCC2;
    frame[5 + length] = FLAG;  
}

void send_frame(int fd, FrameType type, LinkLayerRole role, int curr_num) {
    unsigned char frame[FRAME_SIZE];
    build_frame(frame, type, role);
    int bytes = write(fd, frame, FRAME_SIZE);
    if(bytes == -1) {
        perror("error: failed to write");
    }
}

int send_information_frame(int fd, unsigned char* frame, int length) {
    int bytes_written = 0;
    while(bytes_written < length) {
        int bytes = write(fd, frame+bytes_written, length - bytes_written);
        if(bytes <0) {
            perror("error: failed to send frame\n");
            return ERROR_CMD;
        }
        bytes_written += bytes;
    }
    //amnt_bytes+=length;
    //printf("bytes written: %d\n",length);
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
            return ERROR_CMD;
        }
    }
    
    deleteStateMachine(sm);
    return 0;
}

int receive_information_frame(int fd, unsigned char* frame) {
    // Default to 1 to not fail!!!!!
    int ret = read(fd, frame, 1);
    //amnt_bytes += ret > 0 ? 1 : 0;
    return ret;
}

int receive_information_answer(int fd, int curr_num) {
    unsigned char rr[FRAME_SIZE], rej[FRAME_SIZE], byte;
    int ret = ERROR_CMD;
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

    for(int i = 4; i < length+5; i++) {
        if(frame[i] == FLAG) {
            //printf("!found flag!\n");
            aux_frame[data_pos++] = ESC;
            aux_frame[data_pos++] = COMP_FLAG;
        }
        else if (frame[i] == ESC) {
            //printf("!found esc!\n");
            aux_frame[data_pos++] = ESC;
            aux_frame[data_pos++] = COMP_ESC;           
        }
        else
            aux_frame[data_pos++] = frame[i];
        
    }

    // Pass FLAG to auxiliar frame
    memcpy(aux_frame + data_pos, frame+length+5, 1);
    data_pos++;
    memcpy(frame, aux_frame, data_pos);
    //amnt_bytes+= data_pos;
    return data_pos;
}

int byte_destuffing(unsigned char* frame, int length) {
     unsigned char aux_frame[MAX_PAYLOAD_SIZE];
    memcpy(aux_frame, frame, 4);

    int data_pos = 4;

    for(int i = 4; i < length+5; i++) {
        if(frame[i] == ESC) {
            if(frame[i+1] == COMP_FLAG) {
                aux_frame[data_pos++] = FLAG;  
            }
            else
                aux_frame[data_pos++] = ESC;
            i++;
        }
        else {
            aux_frame[data_pos++] = frame[i];
        }
    }
    // Pass FLAG to auxiliar frame
    memcpy(aux_frame + data_pos, frame+length+5, 1);
    data_pos++;
    memcpy(frame, aux_frame, data_pos);
    //amnt_bytes+= data_pos;
    return data_pos;
}

