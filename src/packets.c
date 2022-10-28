#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "alarm.h"
#include "statemachine.h"
#include "packets.h"




int build_control_packet(unsigned char *packet, int isStart, int fileSize, const char *fileName) {
    packet[0] = isStart ? C_START : C_END;
    packet[1] = T_FILE_SIZE;
    packet[2] = 2;
    packet[3] = fileSize / PACKET_SIZE;
    packet[4] = fileSize % PACKET_SIZE; 
    packet[5] = T_FILE_NAME;
    packet[6] = strlen(fileName) +1;

    memcpy(packet + 7, fileName, packet[6]);
    return packet[6] + 7;
}

int verify_control_packet(unsigned char *packet, unsigned char isStart, char *fileName) {
    if(packet[1] == T_FILE_SIZE && packet[5] == T_FILE_NAME) {
        if(isStart && packet[0] == C_START)
            ;
        else if(!isStart && packet[0] == C_END)
            ;
        else 
            return -1;
        
        memcpy(fileName, packet + 7, packet[6]);
        return ((int)packet[3] * PACKET_SIZE + (int)packet[4]);
    }
    else 
        return -1;
    
}


int build_data_packet(unsigned char *packet, int bufSize, unsigned char *buf, int currNumb) {
    packet[0] = C_DATA;
    packet[1] = currNumb % PACKET_SIZE;
    packet[2] = bufSize / PACKET_SIZE;
    packet[3] = bufSize % PACKET_SIZE;

    memcpy(packet + 4, buf, bufSize);   

    return bufSize + 4;
}
