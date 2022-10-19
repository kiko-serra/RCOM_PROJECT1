#ifndef FRAME_H
#define FRAME_H

void send_set_frame(int fd); 

void send_ua_frame(int fd);

void send_disc_frame(int fd);

#endif // _frame_h_