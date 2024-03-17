//
// Created by asus on 12/03/2024.
//

#ifndef HAF3_REQQ_H
#define HAF3_REQQ_H


#include <stdbool.h>
#include <sys/time.h>


typedef struct req *Req;
typedef struct req_q *ReqQ;
Req createRequest(int fd);
ReqQ createReqQueue(int limit);

struct timeval *getReqTime(ReqQ cur_q, int fd);
bool isQnull(ReqQ cur_q);
bool isQfull(ReqQ cur_q);
bool isQempty(ReqQ cur_q);
void insertReq(ReqQ cur_q, int file_id,struct timeval* tt);
int removeFirstReq(ReqQ cur_q);
int removeReq(ReqQ cur_q, int index);
int getQsize(ReqQ cur_q);
int findFileId(ReqQ cur_q, int file_id);
struct timeval * getFirstReqTime(ReqQ cur_q);















#endif //HAF3_REQQ_H
