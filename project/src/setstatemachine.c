#include <stdlib.h>
#include "setstatemachine.h"
#include "frame.h"

unsigned char getAddressByte(StateMachine *sm) {
    if(sm->end == RECEIVER && sm->frameType == UA)
        return A_RECEIVER_REP;
    if(sm->end == TRANSMITTER && sm->frameType == SET)
        return A_TRANSMITTER_CMD;

    return 0;
}

unsigned char getControlByte(StateMachine *sm) {
    switch (sm->frameType)
    {
    case SET: return C_SET;
    case DISC: return C_DISC;
    case UA: return C_UA;
    case RR: return C_RR;
    case REJ: return C_REJ;
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
    sm->addressByte = getAddressByte(sm);
    sm->controlByte = getControlByte(sm);
    sm->bcc = sm->addressByte ^ sm->controlByte;

    return sm;
}

void deleteStateMachine(StateMachine *sm) {
    if(sm != NULL) 
        free(sm);
}
void handleStateMachine(StateMachine *sm, unsigned char byte) {
    
    switch (sm->state)
    {
    case START:
        if(byte == FLAG) {
            changeState(sm, FLAG_RCV);
        }
        break;

    case FLAG_RCV:
        if(byte == FLAG) {
            ;
        }
        else if (byte == sm->addressByte) {
            changeState(sm, A_RCV);
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
        }
        else {
            changeState(sm, START);
        }
        break;

    case C_RCV:
        if(byte == FLAG) {
            changeState(sm, FLAG_RCV);
        }
        else if (byte == sm->bcc){
            changeState(sm, BCC_OK);
        }
        else {
            changeState(sm, START);
        }
        break;

    case BCC_OK:
        if(byte == FLAG) {
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
