//
// Created by asus on 29/03/2024.
//

#include <unistd.h>

void* smalloc(size_t size){
    if(size == 0){
        return NULL;
    }
    if(size > 100000000){
        return NULL;
    }

    void* ret = sbrk(size);
    if(ret == ((void*)(-1))){
        return NULL;
    }

    return ret;

}





