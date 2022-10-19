#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>


#include "transmitter.h"
#include "frame.h"
#include "constants.h"


void alarmHandler(int signal){
    printf("Alarm #%d\n", ll.nRetransmissions);
    ll.nRetransmissions --;
    if(ll.nRetransmissions == 0){
        printf("Max attempts reached!\n");
        exit(1);
    }
}

int write_noncanonical(char* port){
    (void)signal(SIGALRM, alarmHandler);

    // setting aplicationLayer and LinkLayer of transmitter
    ll.sequenceNumber = 0;
    ll.nRetransmissions = 3;
    ll.timeout = 3;

    al.fileDescriptor = llopen(port);
    al.status = 0; // openning
 
    while(TRUE){
        switch(al.status){
            case 0: // openning (send frame SET and get UA)
                send_set_frame(al.fileDescriptor);
                alarm(ll.timeout);
                printf("transmitter before state machine\n");
                state_machine_SET_UA(al.fileDescriptor, TRANSMITTER);

                printf("Received the UA correctly\n");  // se sair do ciclo da state_machine, quer dizer que recebeu o UA corretamente
                printf("Connection opened sucessfully!\n\n");
                ll.nRetransmissions = 3;
                al.status = 2;
                break;
            case 1: // send frame I
                break;
            case 2: // send frame DISC to disconect
                if(llclose(al.fileDescriptor) > 0){
                    exit(1);
                }
                else
                    exit(-1);
                break;
            default:
                break;
        }
    }
    return 0;
}
