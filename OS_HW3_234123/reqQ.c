//
// Created by asus on 12/03/2024.
//

#include "reqQ.h"
#include<stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


/************************************ CREATION ********************************************/

struct req{
    struct timeval *insertion_time;
    int data;
    Req next;
    Req previous;
};



Req createRequest(int fd){
    Req cur_req = (Req)malloc(sizeof(struct req));

    //in case of failure
    if(cur_req == NULL){
        return NULL;
    }

    //the previous and insertion_time are done in the queue where we insert them
    cur_req->data = fd;
    cur_req->next = NULL;

    return cur_req;

}


struct req_q{
    int max_size;
    int cur_size;
    Req head;
    Req tail;
};

ReqQ createReqQueue(int limit){

    ReqQ cur_q = (ReqQ)malloc(sizeof(struct req)); //ReqQ cur_q = (ReqQ)malloc(sizeof(cur_q));

    //in case of failure
    if(cur_q == NULL){
        return NULL;
    }

    cur_q->max_size = limit;
    cur_q->cur_size = 0;
    cur_q->head = NULL;
    cur_q->tail = NULL;

    return cur_q;

}


/************************************* SIZE RELATED FUNCS *******************************/


bool isQnull(ReqQ cur_q){
    if(cur_q == NULL){
        return true;
    }
    return false;
}



bool isQfull(ReqQ cur_q){

    if(isQnull(cur_q)){
        return false;
    }

    if(cur_q->cur_size == cur_q->max_size) {
        return true;
    }

    return false;

}

bool isQempty(ReqQ cur_q){

    if(isQnull(cur_q)){
        return true;
    }

    if(cur_q->cur_size == 0) {
        return true;
    }

    return false;
}

int getQsize(ReqQ cur_q){

    if(cur_q == NULL){
        return -1;
    }

    return (cur_q->cur_size);

}

void incSize(ReqQ cur_q){
    if(cur_q == NULL){
        return;
    }

    (cur_q->cur_size) ++;
}

void decSize(ReqQ cur_q){
    if(cur_q == NULL){
        return;
    }

    (cur_q->cur_size)--;
}



/*************************************** INSERT&REMOVE *******************************************/

void insertReq(ReqQ cur_q, int file_id,struct timeval* tt){

    if(isQnull(cur_q)){
        return;
    }
    if(isQfull(cur_q)){
        return;
    }

    Req r = createRequest(file_id);
    r->insertion_time = tt;
    if(isQempty(cur_q)){
        cur_q->tail = cur_q-> head = r; // put into one line
        incSize(cur_q);
        return;
    }
    else{
        cur_q->tail->next = r;
        r->previous = cur_q->tail;
        cur_q->tail = r;
        incSize(cur_q);
        return;
    }

}

int removeFirstReq(ReqQ cur_q){

    if(isQnull(cur_q) || isQempty(cur_q) ){
        return -1;
    }

    int fd = cur_q->head->data;
    if(cur_q->cur_size == 1){
        free(cur_q->head);
        cur_q->head = NULL;
        cur_q->tail = NULL;
        decSize(cur_q);
        return fd;
    }else{
        Req tmp = cur_q->head->next;
        free(cur_q->head);
        cur_q->head = tmp;
        tmp->previous = NULL;
        decSize(cur_q);
        return fd;
    }

    return fd;


}


int removeReq(ReqQ cur_q, int index){

    // if there aren't any requests
    if(isQnull(cur_q) || isQempty(cur_q)){
        return -1;
    }

    //if the request index is invalid
    int ss = getQsize(cur_q);
    if(ss == -1){
        return -1;
    }
    if(index < 0 || index >= ss){
        return -1;
    }

    //if the request to be removed is the first
    if(index == 0){
        return removeFirstReq(cur_q);
    }

    Req tmp = cur_q->head;
    for(int i = 0; i < index; i++)
    {
        tmp = (tmp->next);
    }

    int fd = tmp->data;
    Req prev = tmp->previous;
    //if the request to be removed is the first
    if(prev == NULL){
        return removeFirstReq(cur_q);
    }

    prev->next = tmp->next;

    //if the request to be removed is in the middle
    if(prev->next != NULL){
        prev->next->previous = prev;
    }

    //if the request to be removed is the last
    if( index >= ss - 1){
        cur_q->tail = prev;
    }

    //catch & release by Matt Simons
    free(tmp);
    decSize(cur_q);

    return fd;

}



/****************************************** SEARCH *******************************************/


int findFileId(ReqQ cur_q, int file_id){

    if(isQnull(cur_q)){
        return -1;
    }
    if(isQempty(cur_q)){
        return -1;
    }

    int ss = getQsize(cur_q);
    if(ss == -1 || ss == 0){
        return -1;
    }

    Req tmp = cur_q->head;

    for(int i = 0; i < ss; i++){
        if(file_id == tmp->data){
            return i;
        }
        tmp = tmp->next;
    }

    return -1;

}


/***************************************** TIME RELATED *******************************************/
struct timeval * getFirstReqTime(ReqQ cur_q){

    if(isQnull(cur_q)){
        return NULL;
    }
    if(isQempty(cur_q)){
        return NULL;
    }
    if((cur_q->head) == NULL){
        return NULL;
    }

    return (cur_q->head->insertion_time);

}

struct timeval *getReqTime(ReqQ cur_q, int fd){
    if(isQnull(cur_q)){
        return NULL;
    }
    if(isQempty(cur_q)){
        return NULL;
    }

    struct timeval tm;
    gettimeofday(&tm, NULL);

    int ss = getQsize(cur_q);
    if(ss == -1){
        return NULL;
    }
    Req tmp = cur_q->head;

    for(int i = 0; i < ss; i++){
        if(fd == tmp->data){
            return tmp->insertion_time;
        }
        tmp = tmp->next;
    }

    return NULL;
}

