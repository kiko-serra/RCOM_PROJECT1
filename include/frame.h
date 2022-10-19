#ifndef _FRAME_H_
#define _FRAME_H_

#include "link_layer.h"

// Control Field

#define C_SET 0x03
#define C_DISC 0x0B
#define C_UA 0x07
#define C_RR 0x05 //0x85
#define C_REJ 0x01 //0x81

#define A_TRANSMITTER_CMD 0x03
#define A_RECEIVER_REP 0x03
#define A_TRANSMITTER_REP 0x01
#define A_RECEIVER_CMD 0x01

#define FLAG 0x7e
#define ESC_OCT 0x7d

#define FRAME_SIZE 5

typedef enum { I,SET,DISC,UA,RR,REJ } FrameType;

void build_frame (FrameType type, unsigned char* frame, LinkLayerRole role);

void send_frame(int fd, FrameType type, LinkLayerRole other);

int receive_frame(int fd, FrameType type, LinkLayerRole other);


#endif // _FRAME_H_