#include "kernel/types.h"
#include "user/user.h"
#include "uthread.h"

struct uthread threads_table[MAX_UTHREADS];
struct uthread *current_thread = 0;


int uthread_create(void (*start_func)(), enum sched_priority priority){
  int index = -1;
  for(int i = 0;i<MAX_UTHREADS;i++){
    if(threads_table[i].state == FREE)
      index = i;
  }
  if(index == -1) return -1;
  struct uthread* thread = &threads_table[index];
  thread->context.ra = (uint64) (start_func);
  thread->context.sp = (uint64) &(thread->ustack) + STACK_SIZE;
  thread->priority = priority;
  thread->state = RUNNABLE;
  return 0;
}

enum sched_priority uthread_set_priority(enum sched_priority priority){
    enum sched_priority prev = current_thread->priority;
    current_thread->priority = priority;
    return prev;
}
enum sched_priority uthread_get_priority(){
    return current_thread->priority;
}

struct uthread* uthread_self(){
    return current_thread;
}

//returns the next runnable thread per process according to priority
struct uthread* sched(){
    struct uthread *next_thread = 0;
    struct uthread *curr_thread;
    enum sched_priority priority = LOW;
    int index = 0;
    int first = 0;
    if(current_thread!=0){
        for(curr_thread = threads_table;curr_thread<&threads_table[MAX_UTHREADS]; curr_thread++){
            if(curr_thread!=current_thread) index++;
        }
    }
    int max_iter = MAX_UTHREADS;
    int i = (index+1)%MAX_UTHREADS;
    while(max_iter!=0){
        curr_thread = &threads_table[i];
        if(curr_thread->state == RUNNABLE){
            if(first == 0){
                next_thread = curr_thread;
                priority = curr_thread->priority;
                first = 1;
            }else{
                if(curr_thread->priority > priority){
                    next_thread = curr_thread;
                    priority = curr_thread->priority;
                }
            }
        }
        i = (i+1)%MAX_UTHREADS;
        max_iter--;
    }
    return next_thread;
}

void uthread_yield(){
    struct uthread* thrd = sched();
    if(thrd!=0){
        struct uthread* prev = current_thread;
        prev->state = RUNNABLE;
        current_thread = thrd;
        current_thread->state = RUNNING;
        uswtch(&prev->context,&current_thread->context);
    }
}

void uthread_exit(){
    struct uthread* thrd = sched();
    if(thrd==0){
        current_thread->state = FREE;
        exit(0);
    }else{
        struct uthread* prev = current_thread;
        prev->state = FREE;
        current_thread = thrd;
        current_thread->state = RUNNING;
        uswtch(&prev->context,&current_thread->context);
    }
}

int uthread_start_all(){
    static int first = 1;
    if(first){
        first = 0;
        if(current_thread == 0){
            current_thread = sched();
            current_thread->state = RUNNING;
            struct context context;
            uswtch(&context,&current_thread->context);
        }
        return -1;
    }
    return -1;
}
