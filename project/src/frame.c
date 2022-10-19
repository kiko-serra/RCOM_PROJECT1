#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "frame.h"
#include "constants.h"

void send_set_frame(int fd) {
    // Create string SET to send
    unsigned char SET[5];
    SET[0] = FLAG; 
    SET[1] = A; 
    SET[2] = C_SET;
    SET[3] = SET[1]^SET[2]; 
    SET[4] = FLAG;

    int bytes = write(fd, SET, 5);
    printf("SET sent! bytes sent: %d\n", bytes);
}

void send_ua_frame(int fd) {
    // Create string UA to send
    unsigned char UA[5];
    UA[0] = FLAG;
    UA[1] = A;
    UA[2] = C_UA;
    UA[3] = UA[1]^UA[2];
    UA[4] = FLAG;

    int res = write(fd, UA, 5);
	printf("UA sent! bytes sent: %d\n", res);
}

void send_disc_frame(int fd){
    // Create string DISC to send
    unsigned char DC[5];
    DC[0] = FLAG; 
    DC[1] = A; 
    DC[2] = DISC;
    DC[3] = DC[1]^DC[2]; 
    DC[4] = FLAG;

    int bytes = write(fd, DC, 5);
    printf("DISC sent! bytes sent: %d\n", bytes);
}
