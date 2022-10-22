#ifndef _PACKETS_H_
#define _PACKETS_H_

#include "files.h"

#define C_DATA 0x01
#define C_START 0x02
#define C_END 0x03

#define T_FILE_SIZE 0x00
#define T_FILE_NAME 0x01

#define PACKET_SIZE 256
#define MAX_PACKET_SIZE 512

int build_control_packet(unsigned char *packet, int isStart, int fileSize, const char *fileName);

int verify_control_packet(unsigned char *packet, unsigned char isStart, char *fileName);

int build_data_packet(unsigned char *packet, int bufSize, unsigned char *buf, int currNumb);

// void build_packets(char* fileName);


#endif // _PACKETS_H_
