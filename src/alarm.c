#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdbool.h>

int alarmFired = false;
int alarmCount = 0;

void alarmHandler(int signal){
    alarmCount++;
    alarmFired = true;

    printf("Alarm #%d\n", alarmCount);
}