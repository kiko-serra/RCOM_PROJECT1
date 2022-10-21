#ifndef _CONSTANTS_H_
#define CONSTANTS_H_

//Packets 
#define C_DATA 0x01
#define C_START 0x02
#define C_END 0x03

#define T_FILE_SIZE 0x00
#define T_FILE_NAME 0x01

#define PACKET_SIZE 256
#define MAX_PACKET_SIZE 512

//Link Layer
#define MAX_PAYLOAD_SIZE 1000

#define FALSE 0
#define TRUE 1

//Frame

//Control Field
#define C_SET 0x03
#define C_DISC 0x0B
#define C_UA 0x07
#define C_RR 0x05 //0x85
#define C_REJ 0x01 //0x81

#define A_TRANSMITTER_CMD 0x03
#define A_RECEIVER_REP 0x03
#define A_TRANSMITTER_REP 0x01
#define A_RECEIVER_CMD 0x01

// I Field

#define C_CTRL_1 0x00 
#define C_CTRL_2 0x40

#define FLAG 0x7e
#define COMP_FLAG 0x5e

#define ESC 0x7d
#define COMP_ESC 0x5e

#define FRAME_SIZE 5

#endif