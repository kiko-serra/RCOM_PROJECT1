// Application layer protocol implementation

#include <string.h>
#include <stdio.h>  

#include "application_layer.h"
#include "link_layer.h"
#include "packets.h"

#define PACKET_SEND_SIZE 450
#define FILE_NAME_SIZE 128

int sendFile(const char *fileName) {
    FILE *fp = fopen(fileName, "r");
    if(fp == NULL) {
        perror("error: couldn't open the file\n");
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    int fileSize = ftell(fp);
    rewind(fp);

    printf("File size  %i\n", fileSize);

    unsigned char packet[1000];
    int packetSize;
    // start control 

    packetSize = build_control_packet(packet, TRUE, fileSize, fileName);
    if(llwrite(packet, packetSize) < 0) {
        perror("error: Failed to write packet\n");
        fclose(fp);
        return -1;
    }
    //printf("sendFile: Control packet start done\n");

    ////////////////////////////////////////////////

    int currNum = 0;
    int nBytes = 1;
    unsigned char buffer[MAX_PAYLOAD_SIZE];
    int stop = FALSE;
    while(!stop) {
        nBytes = fread(buffer, sizeof(unsigned char), PACKET_SEND_SIZE, fp);
        printf("sendFile: nBytes %d\n", nBytes);

        if(nBytes < PACKET_SEND_SIZE)
            stop = TRUE;

        packetSize = build_data_packet(packet, nBytes, buffer, currNum++);
        currNum%=2;

        if(llwrite(packet, packetSize) < 0) {
            perror("error: Failed to write packet\n");
            fclose(fp);
            return -1;
        }
        //printf("sendFile: endwhile");
    }

    ////////////////////////////////////////////////

    // end control
    build_control_packet(packet, FALSE, fileSize, fileName);
    if(llwrite(packet, packetSize) < 0) {
        perror("error: Failed to write packet\n");
        fclose(fp);
        return -1;
    }

    printf("sendFile: Control packet end done\n");
    fclose(fp);
    return 1;
}

int receiveFile(const char *mainFile) {

    unsigned char packet[1000];
    int packetSize;
    packetSize = llread(packet);
    if(packetSize < 0 || packet[0] != C_START) {
        perror("error: failed to read packet\n");
        return -1;
    }

    char fileName[FILE_NAME_SIZE];
    int fileSize = verify_control_packet(packet, TRUE, fileName);
    if(fileSize < 0) {
        perror("error: couldn't verify the start control packet\n");
        return -1;
    }

    char filePath[256] = "received-";
    strcat(filePath, fileName);

    FILE *fp = fopen(filePath, "w");
    if(fp == NULL) {
        perror("error: couldn't create the file\n");
        return -1;
    }

    ////////////////////////////////////////////////

    while(1) {
        packetSize = llread(packet);
        if(packetSize < 0) {
            fclose(fp);
            perror("error: failed to read packet\n");
            return -1;
        }
        if(packet[0] == C_END) break;
       // printf("receiveFile: \n");
        if(fwrite(packet + 4, sizeof(unsigned char), packetSize - 4, fp) != packetSize -4) {
            fclose(fp);
            perror("error: failed to write packet on file\n");
            return -1;
        }

    }

    printf("receiveFile: written successfuly\n");

    ////////////////////////////////////////////////

    char auxFileName[FILE_NAME_SIZE];

    if(verify_control_packet(packet, FALSE, auxFileName) < 0) {
        perror("error: couldn't verify the end control packet\n");
        fclose(fp);
        return -1;
    }
    if(strcmp(auxFileName, fileName) != 0) {
        printf("error: file names differ : %s - %s\n", fileName, auxFileName);
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 1;
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
    printf("opened with success\n");

    // JUST NEED TO READ/WRITE

    if(ll.role == LlTx) {
        printf("sending...\n");
        if(sendFile(filename) < 0) {
            perror("error: failed to send file\n");
        }
    }
    else  {
        printf("receiving...\n");
        if(receiveFile(filename) < 0) {
            perror("error: failed to receive file\n");
        }
    }
        
        

    if (llclose(0) < 0) {
        perror("error: failed to terminate connection\n");
    } else {
        printf("Closed connection successfuly\n");
    }
}
