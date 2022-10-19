#ifndef CONSTANTS_H
#define CONSTANTS_H

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 // POSIX compliant source

#define FALSE 0
#define TRUE 1

// SIZE of maximum acceptable payload.
// Maximum number of bytes that application layer should send to link layer
#define MAX_PAYLOAD_SIZE 1000

#define BAUDRATE B38400 /* bit rate*/
#define FALSE 0
#define TRUE 1
#define A 0x03
#define C_SET 0x03
#define C_UA 0x07
#define BCC_RECEIVER (A^C_SET)
#define BCC_TRANSMITTER (A^C_UA)

#define DISC 0x0B
#define BCC_DISC (A^DISC)
#define RR 0x06
#define REJ 0x01

#define FLAG 0x7e
#define ESCAPE 0x7d

#define TRANSMITTER 0
#define RECEIVER 1

//#define BUF_SIZE 256
#define FRAME_SIZE 5

#endif
