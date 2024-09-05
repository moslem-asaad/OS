#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  fprintf(1,"%s\n%d\n","proc memsize() before allocate",memsize());
  void *space = malloc(20000);
  fprintf(1,"%s\n%d\n","proc memsize() after allocate",memsize());
  free(space);
  fprintf(1,"%s\n%d\n","proc memsize() after release",memsize());
  exit(0,"");
}
