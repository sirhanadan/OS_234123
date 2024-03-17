#include <pthread.h>
#include <time.h>
#include <stdbool.h>
#include "reqQ.h"
#include "segel.h"
#include "request.h"


#include <math.h>




pthread_mutex_t m;
pthread_cond_t no_empty_threads;
pthread_cond_t no_waiting_reqs;
ReqQ pending_reqs;
ReqQ busy_threads;
int *statReqs_cnt;
int *dynReqs_cnt;
int *allReqs_cnt;

void * handle_reqs(void *);
void handleNullMalloc();
int isGreaterThanOneMicrosecond(struct timeval *res);
// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// HW3: Parse the new arguments too
void getargs(int *port, int argc, char *argv[],int *max_threads,int *max_requests, char * overflow_policy)
{
    if (argc < 2) {
	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
    *max_threads = atoi(argv[2]);
    *max_requests = atoi(argv[3]);
    strcpy(overflow_policy ,argv[4]);

}


int main(int argc, char *argv[])
{

    //create all args
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;
    char policy[20];
    int max_threads,max_tasks;

    //create locks and condition vars
    pthread_mutex_init(&m,NULL);
    pthread_cond_init(&no_empty_threads,NULL);
    pthread_cond_init(&no_waiting_reqs,NULL);


    //get args
    getargs(&port, argc, argv,&max_threads,&max_tasks,policy);

    //create the statistics stuff
    allReqs_cnt = malloc(sizeof(int) * max_threads);
    statReqs_cnt = malloc(sizeof(int) * max_threads);
    dynReqs_cnt = malloc(sizeof(int) * max_threads);

    if(statReqs_cnt == NULL || dynReqs_cnt == NULL|| allReqs_cnt == NULL){
        handleNullMalloc();
    }

    pending_reqs = createReqQueue(max_tasks);
    busy_threads = createReqQueue(max_threads);

    if(pending_reqs == NULL || busy_threads == NULL){
        handleNullMalloc();
    }

    //
    // HW3: Create some threads...
    //

    pthread_t * p_threads = malloc(sizeof(pthread_t) * max_threads);
    int * threads_arguments = malloc(sizeof(int) * max_threads);
    if(p_threads == NULL || threads_arguments == NULL){
        handleNullMalloc();
    }

    //reset
    for (int i = 0; i < max_threads; ++i) {
        statReqs_cnt[i] = 0;
        dynReqs_cnt[i] = 0;
        allReqs_cnt[i] = 0;
    }

    for (int i = 0; i < max_threads; ++i) {
        threads_arguments[i] = i;
    }

    for (int i = 0; i < max_threads; ++i) {
        pthread_create(&p_threads[i],NULL,handle_reqs,(void*)(&threads_arguments[i]));
    }

    listenfd = Open_listenfd(port);



    while (1) {
	clientlen = sizeof(clientaddr);

	//struct timeval * res = malloc(sizeof(struct timeval));


	connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
    pthread_mutex_lock(&m);
        struct timeval* time = malloc(sizeof(struct timeval));
        gettimeofday(time,NULL);
    //struct timeval* blocktimebefore = malloc(sizeof(struct timeval));
    if(getQsize(pending_reqs)+ getQsize(busy_threads) >= max_tasks){
        if(strcmp(policy,"block")==0) {
      //      gettimeofday(blocktimebefore,NULL);
            while (getQsize(pending_reqs) + getQsize(busy_threads) >= max_tasks) {

                //struct timeval* blocktimebefore = malloc(sizeof(struct timeval));
                //struct timeval* blocktimeafter = malloc(sizeof(struct timeval));
                //handleNullMalloc(); //if time is null, what do we doooooooooooooooooooooooo?
                //gettimeofday(blocktimebefore,NULL);

                pthread_cond_wait(&no_empty_threads, &m);

                //gettimeofday(blocktimeafter,NULL);


                //timersub(res,blocktimebefore,blocktimeafter);



            }
        }
        if(strcmp(policy,"dh")==0){
            if(getQsize(pending_reqs)!=0){
                int fd = removeFirstReq(pending_reqs);
                close(fd);
            }
            else{
                close(connfd);
                pthread_mutex_unlock(&m);
                continue;
            }
        }
        if(strcmp(policy,"dt")==0){
            close(connfd);
            pthread_mutex_unlock(&m);
            continue;
        }
        if(strcmp(policy,"random")==0){
            if(getQsize(pending_reqs)!=0) {
                int delete = ceil((getQsize(pending_reqs) * 0.5));
                for (int i = 0; i < delete; i++) {
                    if (isQempty(pending_reqs))
                        break;
                    int random_index = rand() % getQsize(pending_reqs);
                    int fd = removeReq(pending_reqs, random_index);
                    Close(fd);
                }
            }
            else{
                Close(connfd);
                pthread_mutex_unlock(&m);
                continue;
            }
        }
        if(strcmp(policy,"bf")==0){
            while ((getQsize(pending_reqs) + getQsize(busy_threads) >= max_tasks) &&(!isQempty(busy_threads))) {
                pthread_cond_wait(&no_empty_threads, &m);
            }
            close(connfd);
            pthread_mutex_unlock(&m);
            continue;
        }
    }

	// 
	// HW3: In general, don't handle the request in the main thread.
	// Save the relevant info in a buffer and have one of the worker threads 
	// do the work. 
	//



//    if((strcmp(policy,"block")==0))//policy = block
//    {
//        //if(isGreaterThanOneMicrosecond(res)){
//            //time = blocktimebefore;
//            insertReq(pending_reqs,connfd,blocktimebefore);
//            pthread_cond_signal(&no_waiting_reqs);
//            pthread_mutex_unlock(&m);
//            continue;
//        //}
//    }
//    struct timeval* time = malloc(sizeof(struct timeval));
//    gettimeofday(time,NULL);
    //handleNullMalloc(); //if time is null, what do we doooooooooooooooooooooooo?

    insertReq(pending_reqs,connfd,time);
    pthread_cond_signal(&no_waiting_reqs);
    pthread_mutex_unlock(&m);
    }

}

void handleNullMalloc(){
    //honestly i have absolutely no idea what to do in such cases in this homework.
}

void * handle_reqs(void * arg){

    int reqid = *(int *)arg;
    while(true)
    {
        pthread_mutex_lock(&m);
        while(getQsize(pending_reqs) == 0){
            pthread_cond_wait(&no_waiting_reqs,&m);
        }
        struct timeval * head_insertion_time= getFirstReqTime(pending_reqs);
        int fd = removeFirstReq(pending_reqs);
        struct timeval * tt1 = malloc(sizeof(struct timeval));
        if(tt1 == NULL){
            handleNullMalloc();
        }
        gettimeofday(tt1,NULL);
        insertReq(busy_threads,fd,tt1);
        pthread_mutex_unlock(&m);
        struct timeval * tt_cur= malloc(sizeof(struct timeval));
        if(tt_cur == NULL){
            handleNullMalloc();
        }
        gettimeofday(tt_cur,NULL);
        struct timeval * tt2 = malloc(sizeof(struct timeval));;
        timersub(tt_cur,head_insertion_time,tt2);

        requestHandle(fd,head_insertion_time,tt2,statReqs_cnt,dynReqs_cnt,allReqs_cnt,reqid);
        close(fd);



        pthread_mutex_lock(&m);
        int reqid = findFileId(busy_threads,fd);
        removeReq(busy_threads,reqid);
        pthread_cond_signal(&no_empty_threads);
        pthread_mutex_unlock(&m);
    }
    return NULL;
}


int isGreaterThanOneMicrosecond(struct timeval *res) {
    // Convert microseconds to seconds and add to tv_sec
    double totalSeconds = res->tv_sec + (res->tv_usec / 1000000.0);

    // Check if the total difference is greater than 0.000001 seconds
    if (totalSeconds > 0.000001) {
        return 1;  // Difference is greater than 0.000001 seconds
    } else {
        return 0;  // Difference is less than or equal to 0.000001 seconds
    }
}

    


 
