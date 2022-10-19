#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

typedef enum {START, FLAG_RCV, A_RCV, C_RCV, DISC_RCV, BCC_OK, STOP} stateMachine;

struct applicationLayer {
    int fileDescriptor;     /*Descritor correspondente à porta série*/
    int status;             /*0 - openning; 1 - send data_packets; 2 - closing*/
};

void state_machine_SET_UA(int fd, int flag);

void state_machine_DISC(int fd, int flag);

#endif
