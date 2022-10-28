#include <stdlib.h>
#include <stdio.h>
#include "statemachine.h"
#include "frame.h"

unsigned char getAddressByte(FrameType ft, EndType end) {
    if(end == RECEIVER) return A_RECEIVER_REP;
    else {
        switch (ft)
        {
        case SET: return A_TRANSMITTER_CMD;
        case UA: return A_TRANSMITTER_REP;
        case DISC: return A_TRANSMITTER_CMD;
        default:
           perror("error: wrong type");
           return 0;
        }
    }
}

unsigned char getControlByte(FrameType ft) {
    switch (ft)
    {
    case SET: return C_SET;
    case DISC: return C_DISC;
    case UA: return C_UA;
    case RR: return C_RR;
    case REJ: return C_REJ;
    case I: case NONE: return 0;
    }
    return 0;
}

void changeState(StateMachine *sm, State state) {
    sm->state = state;
}

StateMachine *createStateMachine(FrameType frameType, EndType end) {
    StateMachine *sm = (StateMachine*) malloc(sizeof(StateMachine));
    if(sm == NULL) return NULL;

    changeState(sm, START);
    sm->end = end;
    sm->frameType = frameType;
    sm->addressByte = getAddressByte(frameType, end);
    sm->controlByte = getControlByte(frameType);
    sm->bcc = sm->addressByte ^ sm->controlByte;

    return sm;
}

void deleteStateMachine(StateMachine *sm) {
    if(sm != NULL) 
        free(sm);
}

void handleStateMachine(StateMachine *sm, unsigned char byte, unsigned char *frame) {
    // printf("state: %d %x\n", sm->state, byte);
    switch (sm->state)
    {
    case START:
        if(byte == FLAG) {
            changeState(sm, FLAG_RCV);
            frame[0] = byte;
        }
        break;

    case FLAG_RCV:
        if(byte == FLAG) {
            ;
        }
        else if (byte == sm->addressByte) {
            changeState(sm, A_RCV);
            frame[1] = byte;
        }
        else {
            changeState(sm, START);
        }
        break;

    case A_RCV:
        if(byte == FLAG) {
            changeState(sm, FLAG_RCV);
        }
        else if (byte == sm->controlByte){
            changeState(sm, C_RCV);
            frame[2] = byte;
        }
        else {
            changeState(sm, START);
        }
        break;

    case C_RCV:
        if(byte == FLAG) {
            changeState(sm, FLAG_RCV);
        }
        else if (byte == (frame[1]^frame[2])){
            changeState(sm, BCC_OK);
            frame[3] = byte;
        }
        else {
            changeState(sm, START);
        }
        break;

    case BCC_OK:
        if(byte == FLAG) {
            frame[4] = byte;
            changeState(sm, STOP);
        }
        else {
            changeState(sm, START);
        }
        break;

    default:
        break;
    }
}
