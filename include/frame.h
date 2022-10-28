#ifndef _FRAME_H_
#define _FRAME_H_

#include "link_layer.h"
#include "constants.h"

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
