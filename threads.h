#include <iostream>
#include <vector>
#include <unistd.h>
#include <ucontext.h>
#include <cstdlib>
#include <sys/time.h>
#include <signal.h>
#include <cstring>
#include <string.h>
#include <algorithm>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <iostream>
#include <sys/ucontext.h>
#include <pthread.h>
#include <setjmp.h>
#include <malloc.h>



#ifndef THREADS_H
#define THREADS_H

#define MAX_THREADS 129    //128 + main thread
#define STACK_SIZE  32767  //2^15 -1
#define ALARM_TIME  50000  // in micro-sec => 50msec

enum STATE {
    READY,      // in queue waiting for turn
    RUNNING,    // top of queue
    TERMINATED  // exited
};


struct tcb {
    tcb(){ //initializing an empty tcb block
        thread_id = 0;
        st = READY;
        stack_ptr = new unsigned long [STACK_SIZE];
        pg_prt = stack_ptr + STACK_SIZE / 8 - 2;
        args = nullptr;
        funct_return = nullptr;
        start_routine = nullptr;
    }
    pthread_t thread_id;    //thread id
    STATE st;                //state of thread
    unsigned long *stack_ptr;//stack pointer
    unsigned long *pg_prt;   //program pointer
    void*args;              //---Inputs----
    void*funct_return;
    void*(*start_routine)(void*);
    jmp_buf thread_reg;     //the register buffer
};

int signal_init();

void start_timer();

void stop_timer();

void context_handler();

void pthread_init();

static long int i64_ptr_mangle(long int p);

int wrapper();

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg);

void pthread_exit(void *value_ptr);

pthread_t pthread_self(void);

#endif
