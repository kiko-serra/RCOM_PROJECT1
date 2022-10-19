// Main file of the serial port project.
// NOTE: This file must not be changed.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/transmitter.h"
#include "src/receiver.h"
#include "src/state_machine.h"
#include "src/frame.h"



int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Incorrect program usage\n"
			   "First argument must be: \t\"/det/ttyS0\"\n"
		       "Second argument must be:\t\"transmitter\" or \"receiver\"\n");
		exit(1);
    }

    if((strcmp("/dev/ttyS10", argv[1])!=0) && (strcmp("/dev/ttyS11", argv[1])!=0)){
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS0 or /dev/ttyS1\n");
        exit(1);
    }

    

    if (strcmp(argv[2], "tx") == 0)
	{   
        /*
            ask file name (to choose a file)
            and add a parameter to write_noncanonical to do operations with the file
        */
		write_noncanonical(argv[1]);
	}
	else if (strcmp(argv[2], "rx") == 0)
	{
		read_noncanonical(argv[1]);
	}
	else
	{
		printf("Must specify \"tx\" or \"rx\" as second argument\n");
		return -1;
	}

    return 0;
}
