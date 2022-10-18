// Link layer protocol implementation

#include "link_layer.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    int fd = open(connectionParameters.port, O_RDWR | O_NOCTTY);
    return 1;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
    printf("New termios structure set\n");

    // Create buffers to send and to receive frame
    unsigned char bufSend[BUF_SIZE] = {0};
    unsigned char bufReceive[BUF_SIZE] = {0};

    // State Machine to parse bytes
    StateMachine *sm = createStateMachine(UA, RECEIVER);
    if(sm == NULL){
        perror("create state machine");
        exit(-1);
    }

    (void)signal(SIGALRM, alarmHandler);

    // Read input and store
    gets(bufSend);
    int len = strlen(bufSend);
    (void)signal(SIGALRM, alarmHandler);
    int bytes;

    while (STOP == FALSE && alarmCount < 4){

        // Send frame
        bytes = write(fd, bufSend, len);
        printf("%d bytes written\n", bytes);

        alarm(3); // Set alarm to be triggered in 3s

        // Receive frame
        int bytesRead = read(fd, bufReceive, BUF_SIZE);
        bufReceive[bytesRead] = '\0'; // Set end of string to '\0', so we can printf

        // Check if the received string is equal
        if(strcmp(bufSend, bufReceive)==0){
            printf("Correctly received\n");
            STOP = TRUE;
        };
    }

    // Check if alarm was fired and exit with error code
    if(alarmCount >= 4)
        return -1;

    return bytes;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    // TODO

    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
    // TODO

    return 1;
}
