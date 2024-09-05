#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

extern struct proc proc[NPROC];
extern void forkret(void);

int kthread_create( void *(*start_func)(), void *stack, uint stack_size){
  struct proc *p = myproc();
  struct kthread* kt;
  if((kt = allockthread(p)) == 0){
    return -1;
  }
  kt->state = KTRUNNABLE;
  kt->trapframe->epc = (uint64) start_func;
  kt->trapframe->sp = (uint64) (stack + stack_size);
  int ktid = kt->thid; 
  release(&kt->lock);
  return ktid;

}

int kthread_id(){
  return mykthread()->thid;
}

int kthread_kill(int ktid){
  struct proc *p = myproc();
  for (struct kthread *kt = p->kthread; kt < &p->kthread[NKT]; kt++){
    acquire(&kt->lock);
    if(kt->thid == ktid){
      kt->killed = 1;
      if(kt->state == KTSLEEPING){
        kt->state = KTRUNNABLE;
      }
      release(&kt->lock);
      return 0;
    }
    release(&kt->lock);
  }
  return -1;
}

void kthread_exit(int status){
  struct kthread* mykt = mykthread();
  struct proc* p = mykt->myprocess;
  
  int exit_flag = 1;
  for (struct kthread *kt = p->kthread; kt < &p->kthread[NKT]; kt++){
    if(kt!=mykt){
      acquire(&kt->lock);
      if(kt->state != KTUNUSED && kt->state!= KTZOMBIE){
        exit_flag = 0;
      }
      release(&kt->lock);
    }
  }
  //printf("after check exit flag in kthread exit\n");
  if(exit_flag){
    exit(status);
  }
  acquire(&p->lock);
  wakeup(mykt);
  release(&p->lock);
  acquire(&mykt->lock);
  mykt->xstate = status;
  mykt->state = KTZOMBIE;

  // Jump into the scheduler, never to return.
  sched();
  panic("zombie exit");
  
}

int kthread_join(int ktid, int *status){
  struct proc* p = myproc();
  struct kthread* kt = 0;
  for (struct kthread *kthread = p->kthread; kthread < &p->kthread[NKT]; kthread++){
    acquire(&kthread->lock);
    if(ktid == kthread->thid){
      kt = kthread;
      release(&kthread->lock);
      break;
    }else{
      release(&kthread->lock);
    }
    
  }
  if(kt == 0) return -1;

  acquire(&p->lock);
  for(;;){
    if(kt->state == KTZOMBIE){
      acquire(&kt->lock);
      if(status != 0 && copyout(p->pagetable,(uint64) status, (char *)&kt->xstate,sizeof(kt->xstate)) < 0) {
        release(&kt->lock);
        release(&p->lock);
        return -1;
      }
      freekthread(kt);
      release(&kt->lock);
      release(&p->lock);
      return 0;
    }
    sleep(kt, &p->lock);
  }
}


void kthreadinit(struct proc *p)
{
  //printf("in kthread init\n");
  initlock(&p->thlock, "thread_lock");
  for (struct kthread *kt = p->kthread; kt < &p->kthread[NKT]; kt++)
  {
    initlock(&kt->lock, "kthread");
    acquire(&kt->lock);
    kt->state = KTUNUSED;
    kt->myprocess = p;
    release(&kt->lock);
    // WARNING: Don't change this line!
    // get the pointer to the kernel stack of the kthread
    kt->kstack = KSTACK((int)((p - proc) * NKT + (kt - p->kthread)));
  }
}

struct kthread *mykthread()
{
  push_off();
  struct cpu *c = mycpu();
  struct kthread *kt = c->kthread;
  pop_off();
  return kt;
}

int
allocktid(struct proc *p)
{ 
  int ktid; 
  acquire(&p->thlock);
  ktid = p->thID;
  p->thID += 1;
  //printf("before release\n");
  release(&p->thlock);
  return ktid;
}

struct trapframe *get_kthread_trapframe(struct proc *p, struct kthread *kt)
{
  return p->base_trapframes + ((int)(kt - p->kthread));
}

/*// TODO: delte this after you are done with task 2.2
void allocproc_help_function(struct proc *p) {
  p->kthread->trapframe = get_kthread_trapframe(p, p->kthread);

  p->context.sp = p->kthread->kstack + PGSIZE;
}*/

struct kthread*
allockthread(struct proc *p)
{
  //printf("in allockthread\n");
  struct kthread *kt;
  for (kt = p->kthread; kt < &p->kthread[NKT]; kt++)
  {
    acquire(&kt->lock);
    if(kt->state == KTUNUSED) {
      goto found;
    } else {
      release(&kt->lock);
    }
  }
  return 0;

found:
  kt->thid = allocktid(p);
  kt->state = KTUSED;
  kt->trapframe = get_kthread_trapframe(p,kt);
  // Set up new context to start executing at forkret,
  // which returns to user space.
  memset(&kt->context, 0, sizeof(kt->context));
  kt->context.ra = (uint64)forkret;
  kt->context.sp = kt->kstack + PGSIZE;
  //printf("end of alloc thread\n");

  return kt;
}


void
freekthread(struct kthread *kt)
{
  //printf("in free kthread\n");
  kt->state = UNUSED;
  kt->chan = 0;
  kt->killed = 0;
  kt->xstate = 0;
  kt->thid = 0;
  kt->trapframe = 0;
}
