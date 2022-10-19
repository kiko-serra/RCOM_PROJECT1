#ifndef _SNDTERMIOS_H_
#define _SNDTERMIOS_H_

int setupTermios(const char* serialPortName, int baudRate, int role);

int restoreTermios(int fd);

#endif // _SNDTERMIOS_H_