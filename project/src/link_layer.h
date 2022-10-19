// Link layer header.
// NOTE: This file must not be changed.

#ifndef _LINK_LAYER_H_
#define _LINK_LAYER_H_

struct linkLayer {
    unsigned int sequenceNumber;    /*Número de sequência da trama: 0 | 1*/
    unsigned int nRetransmissions;  /*Número de tentativas em caso de falha - 3*/
    unsigned int timeout;         /*Trama*/
};


// Open a connection using the "port" parameters defined in struct linkLayer.
// Return "1" on success or "-1" on error.
int llopen(char* port);

// Send data in buf with size bufSize.
// Return number of chars written, or "-1" on error.
int llwrite(int fd, unsigned char *buf, int bufSize);

// Receive data in packet.
// Return number of chars read, or "-1" on error.
int llread(int fd, unsigned char *packet);

// Close previously opened connection.
// if showStatistics == TRUE, link layer should print statistics in the console on close.
// Return "1" on success or "-1" on error.
int llclose(int fd);

#endif // _LINK_LAYER_H_
