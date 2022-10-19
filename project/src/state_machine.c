#include "state_machine.h"
#include "constants.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


stateMachine state = START;

void state_machine_SET_UA(int fd, int flag){
    unsigned char b;
	
	while(state != STOP){
		int byteRead = read(fd, &b, 1);
        //printf("byte %x  ", b);
		
		if(byteRead>0){
			switch(state){
				case START:
					if(b == FLAG) state = FLAG_RCV;
					break;
				case FLAG_RCV:
					if(b == A) state = A_RCV;
					else if(b == FLAG) state = FLAG_RCV;
					else state = START;
					break;
				case A_RCV:
					if((b == C_UA && flag == TRANSMITTER) || (b == C_SET && flag == RECEIVER)) state = C_RCV; 
					else if(b == FLAG) state = FLAG_RCV;
					else state = START;
					break;
				case C_RCV:
					if((b == BCC_TRANSMITTER && flag == TRANSMITTER) || (b == BCC_RECEIVER && flag == RECEIVER)) state = BCC_OK;
					else if(b == FLAG) state = FLAG_RCV;
					else state = START;
					break;
				case BCC_OK:
					if(b == FLAG) state = STOP;
					else state = START;
					break;
				default:
					break;
			}
		}
	}
    //printf("\n");
}

void state_machine_DISC(int fd, int flag){
	unsigned char b;
	
	while(state != STOP){
		int byteRead = read(fd, &b, 1);
		//printf("byte %x  ", b);
		
		if(byteRead>0){
			switch(state){
				case START:
					if(b == FLAG) state = FLAG_RCV;
					break;
				case FLAG_RCV:
					if(b == A) state = A_RCV;
					else if(b == FLAG) state = FLAG_RCV;
					else state = START;
					break;
				case A_RCV:
					if(b == DISC) state = DISC_RCV; 
					else if(b == FLAG) state = FLAG_RCV;
					else state = START;
					break;
				case DISC_RCV:
					if(b == BCC_DISC) state = BCC_OK;
					else if(b == FLAG) state = FLAG_RCV;
					else state = START;
					break;
				case BCC_OK:
					if(b == FLAG) state = STOP;
					else state = START;
					break;
				default:
					break;
			}
		}
	}
	//printf("\n");
}