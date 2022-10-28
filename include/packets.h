#ifndef _PACKETS_H_
#define _PACKETS_H_

#include "constants.h"

int build_control_packet(unsigned char *packet, int isStart, int fileSize, const char *fileName);

int verify_control_packet(unsigned char *packet, unsigned char isStart, char *fileName);

int build_data_packet(unsigned char *packet, int bufSize, unsigned char *buf, int currNumb);

// void build_packets(char* fileName);


#endif // _PACKETS_H_
