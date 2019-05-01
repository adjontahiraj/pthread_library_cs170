#include <pthread.h>

// "It compiles so it probably works right??"

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg){
  return 0;
}

void pthread_exit(void *retval){
  while(1); 
}

pthread_t pthread_self(void){
  return 0;
}
