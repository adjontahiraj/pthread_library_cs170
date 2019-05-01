# pthread_library_cs170
This project was a challenging project to start with. It required a lot of reading about threads and how they work and a lot of trial and error. In the end I was able to get the test cases that were given to us to PASS.
My code has the following structure:  I use a vector to queue up new threads and i have tcb struct that hold the stack and program pointer, the thread id, and the jump buffer. For the pthread_create I call the init function which
I wrote to initialize the main thread and set that as the running_thread. This running thread tcb pointer is how I keep treack of which thread has the current power in the system and I use ualarm to give threads each 50msec.
I also have the handler for the alarms wehre I use setjmp and longlmp to switch between threads running and give power to different threads in a round robin fashion. I also use longjmp in the pthread_exit function to jump to the next 
elemnt in the queue if it exists. For the last function, pthread_self, I return the thread id of the running_thread tcb pointer because that aloways points to the current running thread.

Challenges: I had the biggest trouble figuring out how the setjmp and longjmp worked. In addition the pointer arithmetics in this lab made it difficult to keep track of different valiables and pointers and might have cause some memory leaks.
Overall it was a good challenge for me and I learned a lot.
