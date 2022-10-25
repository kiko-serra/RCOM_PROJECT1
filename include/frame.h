#ifndef _FRAME_H_
#define _FRAME_H_

#include "link_layer.h"

#define C_SET 0x03
#define C_DISC 0x0B
#define C_UA 0x07
#define C_RR 0x05 //0x85
#define C_REJ 0x01 //0x81

#define A_TRANSMITTER_CMD 0x03
#define A_RECEIVER_REP 0x03
#define A_TRANSMITTER_REP 0x01
#define A_RECEIVER_CMD 0x01

#define C_CTRL_1 0x00 
#define C_CTRL_2 0x01

#define FLAG 0x7e
#define COMP_FLAG 0x5e

#define ESC 0x7d
#define COMP_ESC 0x5d

#define FRAME_SIZE 5

typedef enum { I, SET, DISC, UA, RR,REJ, NONE } FrameType;

void build_frame (unsigned char* frame, FrameType type, LinkLayerRole role);

void build_information_frame(unsigned char* frame, const unsigned char* data, int lenght, int curr_num);

void send_frame(int fd, FrameType type, LinkLayerRole other, int curr_num);

int send_information_frame(int fd, unsigned char* frame, int length);

int receive_frame(int fd, FrameType type, LinkLayerRole other);

int receive_information_frame(int fd, unsigned char* frame);

int receive_information_answer(int fd, int curr_num);

int byte_stuffing(unsigned char* frame, int lenght);

int byte_destuffing(unsigned char* frame, int lenght);


#endif // _FRAME_H_