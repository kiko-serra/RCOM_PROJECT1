#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "alarm.h"
#include "setstatemachine.h"
#include "packets.h"




int build_control_packet(unsigned char *packet, int isStart, int fileSize, unsigned char *fileName) {
    packet[0] = isStart ? C_START : C_END;
    packet[1] = T_FILE_SIZE;
    packet[2] = 2;
    packet[3] = fileSize / PACKET_SIZE;
    packet[4] = fileSize % PACKET_SIZE; 
    packet[5] = T_FILE_NAME;
    packet[6] = strlen(fileName) +1;

    memcpy(packet + 7, fileName, packet[6]);
    return packet[6] + 6;
}

int verify_control_packet(unsigned char *packet, int isStart, int fileSize, unsigned char *fileName) {
    if((packet[0] == C_START || packet[0] == C_END) && packet[1] == T_FILE_SIZE 
                    && packet[1] == T_FILE_NAME) {
        memcpy(fileName)

    }
}


int build_data_packet(unsigned char *packet, int bufSize, unsigned char *buf, int currNumb) {
    packet[0] = 1;
    packet[1] = currNumb % PACKET_SIZE;
    packet[2] = bufSize / PACKET_SIZE;
    packet[3] = bufSize % PACKET_SIZE;

    memcpy(packet + 4, buf, bufSize);   

    return bufSize + 3;
}


void build_packets(char* fileName) {
    struct stat st;
    int fp = open(fileName, O_RDONLY);
    
    if(stat(fileName, &st) != 0) {
        perror("File not found");
        exit(-1);
    }

    int fileSize = st.st_size;
    printf("File size: %d\n", fileSize);

    int ammount = (fileSize/PACKET_SIZE) + 1;
    char *file = malloc(fileSize);
    int file_size = 1024, nBytes = file_size, i =0;
    while(nBytes == file_size) {
        nBytes = read(fp, file + i*file_size, file_size);

        if(nBytes < 0) {
            perror("Error reading file");
            exit(-1);
        }
        i++;
    }
    close(fp);

    unsigned char** packets = calloc(ammount + 2, sizeof(char*)); // +2 for control packets
    int control =0;
    while(i < ammount) {
        int packets_size = 0;
        // Build control packets
        packets[control] = malloc(PACKET_SIZE + 4 + 1);

        if(control == 0){
            packets[control][packets_size++] = 2;
            packets[control][packets_size++] = 0;
            packets[control][packets_size++] = sizeof(int);

            memcpy(packets[control][packets_size], &fileSize, sizeof(int));
            packets_size += sizeof(int);

            packets[control][packets_size++] = 1;
            packets[control][packets_size++] = strlen(fileName) + 1;

            memcpy(packets[control][packets_size], fileName, strlen(fileName) + 1);
            packets_size += strlen(fileName) + 1;
            control = 1;
        }
        else if(control == (fileSize/PACKET_SIZE) + 2){ //if its the last control packet
            packets[control][packets_size++] = 3;
            packets[control][packets_size++] = 0;
            packets[control][packets_size++] = sizeof(int);

            memcpy(packets[control][packets_size], &fileSize, sizeof(int));
            packets_size += sizeof(int);

            packets[control][packets_size++] = 1;
            packets[control][packets_size++] = strlen(fileName) + 1;

            memcpy(packets[control][packets_size], fileName, strlen(fileName) + 1);
            packets_size += strlen(fileName) + 1;
            control = 0;
        }
        else {
            packets[control][packets_size++] = 1;
            packets[control][packets_size++] = control % 256;
            packets[control][packets_size++] = fileSize / 256;
            packets[control][packets_size++] = fileSize % 256;

            memcpy(packets[control][packets_size], file + (control - 1)*fileSize, fileSize);
            packets_size += fileSize;
        }
        i++;

        llwrite(packets[control], packets_size);
    }
    
        
}
