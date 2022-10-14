#ifndef _setstatemachine_h_
#define _setstatemachine_h_

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
    RECEIVER,
    TRANSMITTER
} EndType;

typedef struct StateMachines {
    unsigned char addressByte;
    unsigned char controlByte;
    unsigned char bcc;
    State state;
    EndType end;
    FrameType frameType;
} StateMachine;

StateMachine *createStateMachine(FrameType FrameType, EndType end);
void deleteStateMachine(StateMachine *sm);
void changeState(StateMachine *sm, State state);
void handleStateMachine(StateMachine *sm, unsigned char byte);


#endif // _setstatemachine_h_
