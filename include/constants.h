#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

//applicationLayer

#define FILE_NAME_SIZE 128

//frame
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

//linkLayer
// SIZE of maximum acceptable payload.
// Maximum number of bytes that application layer should send to link layer
#define MAX_PAYLOAD_SIZE 1000


// 5 bytes will never be stuffed, the rest could be twice as big
// In frames: 2 Flags, 1 A byte, 1 C byte 
// In packets: 1 C byte
// 485*2 + 5 = 997, which is still under the maximum payload limit 
#define MAX_READ_FILE (MAX_PAYLOAD_SIZE/2 - 15)

// MISC
#define FALSE 0
#define TRUE 1
#define _POSIX_SOURCE 1
#define ERROR_CMD -1

//packets
#define C_DATA 0x01
#define C_START 0x02
#define C_END 0x03

#define T_FILE_SIZE 0x00
#define T_FILE_NAME 0x01

#define PACKET_SIZE 256

//sndtermios
#define FALSE 0
#define TRUE 1
#define DEFAULT_VTIME 0
#define DEFAULT_TX_VMIN 0
#define DEFAULT_RX_VMIN 1

//main
#define BAUDRATE 9600
#define N_TRIES 3
#define TIMEOUT 4

#endif // _CONSTANTS_H_