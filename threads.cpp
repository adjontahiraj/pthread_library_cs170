#include "threads.h"

using namespace std;
pthread_t thread_id_generator = 0;
tcb threads[MAX_THREADS];

//tcb queue_scheduler
tcb *running_thread = NULL;
vector<tcb> ready_queue;


//signal and timer structs
//struct sigaction sig;
struct itimerval timer;

//function to start the timer
void start_timer() {
    //we pass in the itimerval struct
    setitimer(ITIMER_VIRTUAL, &timer, 0);
}

//function to stop the timer during thread context switch
void stop_timer() {
    setitimer(ITIMER_VIRTUAL, 0, 0);
}

//used to change between thread contexts when timer hits 50ms
void context_handler(int signal) {
    static int count = 0;
    //cout<< "Timer exp: " << ++count << " times"<<endl;
    //disabling the timer
    //stop_timer();

    //flag can be changed by other threads
    volatile int flag = 0;

    //changing thread state to ready and setting saving jump_register
    running_thread->st = READY;

    setjmp(running_thread->thread_reg);

    //threads[running_thread->thread_id] = *running_thread;


    if(flag == 1) {
        flag = 0;
        return;
    }
	//cout<<"Post flag"<<endl;
    //putting the thread to back to queue - round robin
    ready_queue.push_back(*running_thread);
	//cout<<"Pushing queue"<<endl;
    //making the running thread the thread at front of queue
    running_thread = new tcb(ready_queue.front());
	//cout<<"post tcb"<<endl;
    running_thread->st = RUNNING;
    //removing the current running thread from the queue
	//cout<<"tid: "<<running_thread ->thread_id<<endl;
    ready_queue.erase(ready_queue.begin());
	//cout<<"post erase"<<endl;
    //setting flag to 1
    flag = 1;
    
    
    //threads[running_thread->thread_id] = *running_thread;
	//cout<<"post threads array"<<endl;
	//start_timer();
	longjmp(running_thread->thread_reg,1);
}

void pthread_init() {
    //set up main tcb block
    tcb *this_thread = (tcb*)malloc(sizeof(tcb));
    this_thread -> thread_id = thread_id_generator++;
    this_thread -> st = RUNNING;
    //setjmp(this_thread->thread_reg);
    //adding the tcb to the array
    threads[this_thread -> thread_id] = *this_thread;
    //cout<<"Main TCB set up"<<endl;
    //setting the running thread to this thread
    running_thread = this_thread;

    //clearing the signal mem and setting the context handler as the sig.sa_handler
    // memset(&sig, 0, sizeof(sig));
    // sig.sa_handler = &context_handler;

    // //setting the sig.sa_handler as the handler for SIGVTALRM
    // sigaction(SIGVTALRM, &sig, NULL);

    // //setting alarm to trigger every 50msec
    // timer.it_value.tv_sec = 0;
    // timer.it_value.tv_usec = ALARM_TIME;
    // timer.it_interval.tv_sec = 0;
    // timer.it_interval.tv_usec = ALARM_TIME;

    // start_timer();
	struct sigaction timer;
    timer.sa_handler=context_handler;
    timer.sa_flags= SA_NODEFER; 

    sigemptyset(&timer.sa_mask);

    sigaction(SIGALRM,&timer,NULL);

    ualarm(50000,50000);
    
}


static long int i64_ptr_mangle(long int p) {
    long int ret;
    asm(" mov %1, %%rax;\n"
        " xor %%fs:0x30, %%rax;"
        " rol $0x11, %%rax;"
        " mov %%rax, %0;"
    : "=r"(ret)
    : "r"(p)
    : "%rax"
    );
    return ret;
}

//wrapper function to run the thread funct
int wrapper() {
    //cout<<"Wrapper top"<<endl;
	//cout<<"TID"<<running_thread->thread_id<<endl;
    void *result = running_thread->start_routine(running_thread->args);
	// cout<<"Wrapper mid"<<endl;
    // running_thread->funct_return = result;
	// cout<<"Wrapper pre exit"<<endl;
    // pthread_exit(0);
	// cout<<"Wrapper post exit"<<endl;
	void *(*start_routine)(void*)=running_thread->start_routine;
    void *arg=running_thread->args;
    running_thread->funct_return=start_routine(arg);
    running_thread->st=TERMINATED;
    pthread_exit(running_thread->funct_return);

}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg) {
    if(thread_id_generator == 0) {
        //cout<<"PRE - INIT"<<endl;
        pthread_init();
    }
    //stop_timer();

    //creating the tcb for this thread
    tcb *t =(tcb*)malloc(sizeof(tcb));
    t->thread_id = thread_id_generator++;
    t->st = READY;
    t ->start_routine = start_routine;

    t ->args = arg;
    t ->funct_return = NULL;

    //cout<<"MAIN TCB init"<<endl;

    if(setjmp(t->thread_reg)) {
        cerr<<"ERROR: could not set jump"<<endl;
        return -1;
    }
    //cout<<"SETJMP Complete"<<endl;


    //allocating stack for thread and getting the stack pointer + program pointer
    
    char *stack = (char *)malloc(STACK_SIZE);
    //cout<<"Hello"<<endl;
    unsigned long stack_ptr, program_ptr;
    //cout<<"Hello3"<<endl;
    stack_ptr = (unsigned long)stack + STACK_SIZE/8 -2;
    //cout<<"Stack pointer Created"<<endl;
    program_ptr = (unsigned long)wrapper;
    //cout<<"Program pointer created"<<endl;
    //cout<<"before setting buffers: "<<t ->thread_reg[0].__jmpbuf[6]<< endl;
    t ->thread_reg[0].__jmpbuf[6] = i64_ptr_mangle((long int)stack_ptr);
    t ->thread_reg[0].__jmpbuf[7] = i64_ptr_mangle((long int)program_ptr);
    //cout<<"after setting buffers: "<< t ->thread_reg[0].__jmpbuf[6]<< endl;
    //cout<<"JumpBuff mangled"<<endl;
    ready_queue.push_back(*t);
    threads[t->thread_id] = *t;
    //cout<< "TCB Thread == "<< thread_id_generator -1 <<endl; 
    *thread = t->thread_id;
    //start_timer();
    return 0;
}

void pthread_exit(void *value_ptr) {

    //cout<<"Called Exit: " << endl;
    if(running_thread->thread_id != 0 ) {
        running_thread = new  tcb(ready_queue.front());
        running_thread->st = RUNNING;
        ready_queue.erase(ready_queue.begin());
		//start_timer();
        longjmp(running_thread->thread_reg,1);
    }
    //start_timer();
    exit(0);
}

pthread_t pthread_self(void) {
    return running_thread -> thread_id;
    // if(running_thread!= NULL) {
    //     return running_thread->thread_id;
    // }else{
    //     return terminated_queue.back().thread_id;
    // }
}
