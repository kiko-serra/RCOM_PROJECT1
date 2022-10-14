// Write to serial port in non-canonical mode
//
// Modified by: Eduardo Nuno Almeida [enalmeida@fe.up.pt]

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

#include "frame.h"
#include "setstatemachine.h"

// Baudrate settings are defined in <asm/termbits.h>, which is
// included by <termios.h>
#define BAUDRATE B38400
#define _POSIX_SOURCE 1 // POSIX compliant source

#define FALSE 0
#define TRUE 1

#define BUF_SIZE 256

volatile int SSTOP = FALSE;
int alarmEnabled = FALSE;
int alarmCount = 0;

void alarmHandler(int signal)
{
    alarmEnabled = TRUE;
    alarmCount++;

    printf("Alarm #%d\n", alarmCount);
}

void sendSetFrame(int fd, unsigned char* frame){
    int bytes = write(fd, frame, 5);
    if(bytes != 5){
        perror("error: failed to send set frame.\n");
        exit(-1);
    } else
        printf("INFO: SET frame sent.\n");

    alarm(3);
}

int main(int argc, char *argv[])
{
    // Program usage: Uses either COM1 or COM2
    const char *serialPortName = argv[1];

    if (argc < 2)
    {
        printf("Incorrect program usage\n"
               "Usage: %s <SerialPort>\n"
               "Example: %s /dev/ttyS1\n",
               argv[0],
               argv[0]);
        exit(1);
    }

    // Open serial port device for reading and writing, and not as controlling tty
    // because we don't want to get killed if linenoise sends CTRL-C.
    int fd = open(serialPortName, O_RDWR | O_NOCTTY);

    if (fd < 0)
    {
        perror(serialPortName);
        exit(-1);
    }

    struct termios oldtio;
    struct termios newtio;

    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        exit(-1);
    }

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 30; // Inter-character timer unused
    newtio.c_cc[VMIN] = 0;  // Blocking read until 5 chars received

    // VTIME e VMIN should be changed in order to protect with a
    // timeout the reception of the following character(s)

    // Now clean the line and activate the settings for the port
    // tcflush() discards data written to the object referred to
    // by fd but not transmitted, or data received but not read,
    // depending on the value of queue_selector:
    //   TCIFLUSH - flushes data received but not read.
    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");

    // Create string to send
    // unsigned char buf[BUF_SIZE] = {0};
    unsigned char bufAux[BUF_SIZE] = {0};

    // State Machine to parse bytes
    StateMachine *sm = createStateMachine(UA, RECEIVER);
    if(sm == NULL){
        perror("create state machine");
        exit(-1);
    }

    // Build SET Frame to send
    unsigned char set[FRAME_SIZE];
    build_frame(SET, set);

    // Buffer to store response
    unsigned char buf[FRAME_SIZE];
    int i = 0;

    /*
    //read input and store
    printf("Write here:\n");
    gets(buf);
    int len = strlen(buf);
    */
    (void)signal(SIGALRM, alarmHandler);

    while(sm->state != STOP) {
        // Send frame
        sendSetFrame(fd, set);

        // Wait for UA frame
        while (i < FRAME_SIZE){
            read(fd, buf + i, 1);

            // VTIME is set to 30
            // read will return when a byte is received or then 3 sec. passed
            if(alarmEnabled)
                break;

            handleStateMachine(sm, buf[i++]);
        }
        // Check if alarm was fired
        if(alarmEnabled){
            // Give up after 3 tries
            if(alarmCount == 3){
                perror("error: give up sending SET frame\n");
                exit(-1);
            } else{
                alarmEnabled = FALSE;
                i = 0;
            }
        } else{
            // Check if the FSM was completed (UA)
            if (sm->state != STOP){
                perror("error: not a ua frame\n");
                exit(-1);
            } else{
                printf("INFO: UA frame received.\n");
                // Cancel pending alarm
                alarm(0);
                break;
            }
        }
    }

    deleteStateMachine(sm);
    /*
    while (STOP == FALSE && alarmCount < 4){

        int bytes = write(fd, buf, len);
        printf("%d bytes written\n", bytes);

        alarm(3); // Set alarm to be triggered in 3s

        int bytesRead = read(fd, bufAux, BUF_SIZE);
        bufAux[bytesRead] = '\0'; // Set end of string to '\0', so we can printf

        //Check the received string
        if(strcmp(buf, bufAux)==0){
            printf("%s -- %s\n", buf, bufAux);
            printf("Correctly received\n");
            STOP = TRUE;
        };

        if (bufAux[0] == 'z')
            STOP = TRUE;
    }
    */
    // Wait until all bytes have been written to the serial port
    sleep(1);

    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);

    return 0;
}
