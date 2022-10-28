#ifndef _STATEMACHINE_H_
#define _STATEMACHINE_H_

#include "frame.h"

typedef enum States {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP
} State;

typedef enum EndTypes {
    TRANSMITTER,
    RECEIVER
} EndType;

typedef struct StateMachines {
    unsigned char addressByte;
    unsigned char controlByte;
    unsigned char bcc;
    State state;
    EndType end;
    FrameType frameType;
} StateMachine;

unsigned char getAddressByte(FrameType ft, EndType end);
unsigned char getControlByte(FrameType ft);

StateMachine *createStateMachine(FrameType frameType, EndType end);
void deleteStateMachine(StateMachine *sm);
void changeState(StateMachine *sm, State state);
void handleStateMachine(StateMachine *sm, unsigned char byte, unsigned char *frame);


#endif // _STATEMACHINE_H_
