#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "alarm.h"
#include "setstatemachine.h"
#include "packets.h"
#include "files.h"

#define PACKET_SIZE 256

void build_packets(FILE *fp, int fileSize) {
    printf("File size: %d", fileSize);

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
    closeFile(fp);

    char* packets = calloc(size + 2, sizeof(char*)); // +2 for control packets
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
