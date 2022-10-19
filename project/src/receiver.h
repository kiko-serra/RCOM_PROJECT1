#ifndef RECEIVER_H
#define RECEIVER_H

#include "link_layer.h"
#include "state_machine.h"

struct applicationLayer al;
struct linkLayer ll;

int read_noncanonical(char* port);

#endif
