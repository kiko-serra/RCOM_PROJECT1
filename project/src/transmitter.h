#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include "link_layer.h"
#include "state_machine.h"

struct applicationLayer al;
struct linkLayer ll;

void alarmHandler(int signal);

int write_noncanonical(char* port);

#endif
