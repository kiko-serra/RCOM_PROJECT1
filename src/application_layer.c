// Application layer protocol implementation

#include <string.h>
#include <stdio.h>  

#include "application_layer.h"
#include "link_layer.h"
#include "packets.h"
#include "constants.h"

// Opens the file passed by the parameter
// Builds and sends a control packet to the receiver signing it as it is starting to send packets
// Reads data from the file, builds into data packets and sends them to the receiver
// Builds and sends a control packet to the receiver signing there won't be more packets coming
// Returns -1 in case of an error. Returns 1 otherwise
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

    unsigned char packet[MAX_PAYLOAD_SIZE];
    int packetSize;
    // start control 
    packetSize = build_control_packet(packet, TRUE, fileSize, fileName);
    if(llwrite(packet, packetSize) < 0) {
        perror("error: Failed to write packet\n");
        fclose(fp);
        return -1;
    }
    printf("sendFile: Start control packet done\n");

    int currNum = 0;
    int nBytes = 1;
    unsigned char buffer[MAX_PAYLOAD_SIZE];
    int stop = FALSE;
    while(!stop) {
        nBytes = fread(buffer, sizeof(unsigned char), MAX_READ_FILE, fp);
        //printf("sendFile: nBytes %d\n", nBytes);

        if(nBytes < MAX_READ_FILE)
            stop = TRUE;

        packetSize = build_data_packet(packet, nBytes, buffer, currNum++);
        currNum%=2;

        if(llwrite(packet, packetSize) < 0) {
            perror("error: Failed to write packet\n");
            fclose(fp);
            return -1;
        }
    }

    printf("sendFile: Sending end control packet..\n");
    // end control
    build_control_packet(packet, FALSE, fileSize, fileName);
    if(llwrite(packet, packetSize) < 0) {
        perror("error: Failed to write packet\n");
        fclose(fp);
        return -1;
    }

    printf("sendFile: End control packet done\n");
    fclose(fp);
    return 1;
}


// Opens the file passed by the start control packet
// Reads data packets coming in from llread and writes into the file
// Receives the end control packet and compares the expected file names
// Returns -1 in case of an error. Returns 1 otherwise
int receiveFile(const char *expectedFileName) {

    unsigned char packet[1000];
    int packetSize;
    packetSize = llread(packet);
    if(packetSize < 0 || packet[0] != C_START) {
        perror("error: failed to read packet\n");
        return ERROR_CMD;
    }

    char fileName[FILE_NAME_SIZE];
    int fileSize = verify_control_packet(packet, TRUE, fileName);
    if(fileSize < 0) {
        perror("error: couldn't verify the start control packet\n");
        return ERROR_CMD;
    }

    char filePath[256] = "received-";
    strcat(filePath, fileName);

    FILE *fp = fopen(filePath, "w");
    if(fp == NULL) {
        perror("error: couldn't create the file\n");
        return ERROR_CMD;
    }

    ////////////////////////////////////////////////

    while(1) {
        packetSize = llread(packet);
        
        if(packetSize < 0) {
            fclose(fp);
            perror("error: failed to read packet\n");
            return ERROR_CMD;
        }
        if(packet[0] == C_END) break;
       // printf("receiveFile: \n");
        if(fwrite(packet + 4, sizeof(unsigned char), packetSize - 4, fp) != packetSize -4) {
            fclose(fp);
            perror("error: failed to write packet on file\n");
            return ERROR_CMD;
        }
        //sleep(4);
    }

    printf("receiveFile: written successfuly\n");

    ////////////////////////////////////////////////

    char auxFileName[FILE_NAME_SIZE];

    if(verify_control_packet(packet, FALSE, auxFileName) < 0) {
        perror("error: couldn't verify the end control packet\n");
        fclose(fp);
        return ERROR_CMD;
    }
    if(strcmp(auxFileName, fileName) != 0 && strcmp(expectedFileName, fileName) != 0) {
        printf("error: file names differ : %s - %s - %s\n", fileName, auxFileName, expectedFileName);
        fclose(fp);
        return ERROR_CMD;
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
    ll.nRetransmissions = nTries + 1;

    if(llopen(ll) < 0) {
        perror("error: llopen failed");
        llclose(ERROR_CMD);
        perror("error: probably connection lost...\n");
        return;
    }
    printf("opened with success\n");

    // JUST NEED TO READ/WRITE

    if(ll.role == LlTx) {
        printf("sending...\n");
        if(sendFile(filename) < 0) {
            perror("error: failed to send file\n");
            llclose(ERROR_CMD);
            perror("error: probably connection lost...\n");
            return;
        }
    }
    else  {
        printf("receiving...\n");
        if(receiveFile(filename) < 0) {
            perror("error: failed to receive file\n");
            llclose(ERROR_CMD);
            perror("error: probably connection lost...\n");
            return;
        }
    }
        
        

    if (llclose(TRUE) < 0) {
        perror("error: failed to terminate connection\n");
    } else {
        printf("Closed connection successfuly\n");
    }
}
