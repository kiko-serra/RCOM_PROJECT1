#ifndef _SNDTERMIOS_H_
#define _SNDTERMIOS_H_

// Setup new termios accordingly to the given information
int setupTermios(const char* serialPortName, int baudRate, int role);

// Restores old termios
int restoreTermios(int fd);

#endif // _SNDTERMIOS_H_