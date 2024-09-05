#include "kernel/types.h"
#include "kernel/riscv.h"
#include "kernel/param.h"
#include "user.h"


void* ustack_malloc(uint len);
int ustack_free(void);

struct buffer{
  uint len;
  struct buffer* next;
  int page;
};
