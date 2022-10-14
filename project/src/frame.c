#include <stdio.h>
#include <string.h>
#include "frame.h"

void build_set_frame(unsigned char* frame) {
    unsigned char A = A_TRANSMITTER_CMD;
    unsigned char C = C_SET;
    unsigned char BCC = A ^ C;
    unsigned char* set[] = {FLAG, A, C, BCC, FLAG};

    memcpy(frame, set, FRAME_SIZE);
}

void build_ua_frame(unsigned char* frame) {
    unsigned char A = A_RECEIVER_REP;
    unsigned char C = C_UA;
    unsigned char BCC = A ^ C;
    unsigned char* ua[] = {FLAG, A, C, BCC, FLAG};

    memcpy(frame, ua, FRAME_SIZE);
}


void build_frame (FrameType type, unsigned char* frame) {
    switch (type)
    {

        case I: 
            
            break;

        case SET: 
            build_set_frame(frame);
            break;

        case DISC: 
        
            break;

        case UA: 
            build_ua_frame(frame);
            break;

        case RR: 
        
            break;

        case REJ: 
            
            break;

        
        default:
            break;
    }
}
