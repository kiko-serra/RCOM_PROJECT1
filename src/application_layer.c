// Application layer protocol implementation

#include <string.h>
#include <stdio.h>  

#include "application_layer.h"
#include "link_layer.h"
#include "packets.h"

int sendFile(char *fileName) {
    FILE *fp = fopen(fileName, "r");
    if(fp == NULL) {
        perror("error: couldn't open the file\n");
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    int fileSize = ftell(fp);
    rewind(fp);

    unsigned char packet[256];
    int packetSize;
    // start control 
    packetSize = build_control_packet(packet, TRUE, fileSize, fileName);
    if(llwrite(packet, packetSize) < 0) {
        perror("error: Failed to write packet");
        fclose(fp);
        return;
    }

    int currNum = 0;
    int nBytes = 1;
    while(nBytes > 0) {
        unsigned char buffer[1024];
        int nBytes = fread(buffer, 248, 1, fp);
        packetSize = build_data_packet(packet, nBytes, buffer, currNum++);
        if(llwrite(packet, packetSize) < 0) {
            perror("error: Failed to write packet");
            fclose(fp);
            return;
        }
    }

    // end control
    build_control_packet(packet, FALSE, fileSize, fileName);
    if(llwrite(packet, packetSize) < 0) {
        perror("error: Failed to write packet");
    }
    fclose(fp);
}

int receiveFile(char *fileName) {
    FILE *fp = fopen(fileName, "w");
    if(fp == NULL) {
        perror("error: couldn't open the file\n");
        return -1;
    }

    unsigned char packet[MAX_PACKET_SIZE];
    int packetSize;
    packetSize = llread(packet);
    if(packetSize < 0 || packet[0] != C_START) {
        perror("error: couldn't open the file\n");
        fclose(fp);
        return -1;
    }

    



}

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    LinkLayer ll;
    strcpy(ll.serialPort, serialPort);
    if(!strcmp(role, "tx")) {
        ll.role = LlTx;
    } else if(!strcmp(role, "rx")) {
        ll.role = LlRx;
    } else {
        perror("error: role not defined");
    }
    ll.baudRate = baudRate;
    ll.timeout = timeout;
    ll.nRetransmissions = nTries;

    if(!llopen(ll)) {
        perror("error: llopen failed");
    }

    // JUST NEED TO READ/WRITE

    if(role == LlTx) {
        if(sendFile(filename)) {
            perror("error: failed to send file");
        }
    }
    else  {
        if(receiveFile(filename)) {
            perror("error: failed to receive file");
        }
    }
        
        

    if (llclose(0) < 0) {
        perror("error: failed to terminate connection\n");
    } else {
        printf("Closed connection successfuly\n");
    }
}
