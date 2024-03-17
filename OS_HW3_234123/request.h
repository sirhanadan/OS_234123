#ifndef __REQUEST_H__
#include <sys/time.h>

void requestHandle(int fd,struct timeval * pick_up,struct timeval * dif , int * static_counter , int *
        dynamic_counter, int * total_counter,int index);

#endif
