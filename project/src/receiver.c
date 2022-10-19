#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

#include "receiver.h"
#include "frame.h"
#include "constants.h"

int read_noncanonical(char* port)
{
    // setting aplicationLayer and LinkLayer of transmitter
    ll.sequenceNumber = 0;
    ll.nRetransmissions = 3;
    ll.timeout = 3;

    al.fileDescriptor = llopen(port);
    al.status = 0; // openning
    
    while(TRUE){
        switch(al.status){
            case 0: // openning (receive frame SET and send UA)
                
                state_machine_SET_UA(al.fileDescriptor, RECEIVER);
                printf("TEste\n");
                printf("SET acknowledge!\n");

                send_ua_frame(al.fileDescriptor);

                printf("Connection opened sucessfully!\n\n");
                al.status = 2;
                break;
            case 1: // receiving frame I
                break;
            case 2: // receiving frame DISC and then send it
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
}
