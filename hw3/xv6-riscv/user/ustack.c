#include "ustack.h"


#define MAX_STACK_SIZE 512
#define PAGE_SIZE PGSIZE


struct buffer* top = 0;
char* buffer_ptr = 0;

void* ustack_malloc(uint len){
  char* brk;
  struct buffer* node;
  if(len>MAX_STACK_SIZE)  return (void*) -1;
  if(top == 0 || PGROUNDDOWN((uint64)(buffer_ptr + len)) !=  (uint64)sbrk(0)){
                /* the stack has extended beyond the current program break and
                 additional memory need to be allocated.*/

    brk = sbrk(PAGE_SIZE);
    if (brk == (void*) -1) return (void*) -1;
    node = (struct buffer*)brk;
    if(top){
      node->page = 1;
    }
  }else{
    node = (struct buffer*)buffer_ptr;
  }
  node->len = len;
  node->next = top;
  top = node;
  buffer_ptr = (char*) node + len;
  return (void*) buffer_ptr;
}

int ustack_free(void){
  if(top == 0) return -1;
  top = top->next;
  if(top->page){
    if(sbrk(-PAGE_SIZE) == (void*) -1) return -1;
  }
  uint len = top->len;
  buffer_ptr-= len;
  return len;
}

