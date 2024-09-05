#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  int pids[4];
  for(int child = 0;child<4;child++){
    int pid = fork();
    if(pid == 0){
      set_cfs_priority(child%3);
      for(int i = 0;i<1000000000;i++){
        if(i%100000000 == 0){
          sleep(1);
        }
      }
      exit(0,"");
    }
    else{
      pids[child] = pid;
    }
  }
  sleep(300);
  for(int i = 0;i<4;i++){
    int cfs_priority;
    int rtime;
    int retime;
    int stime;
    get_cfs_stats(pids[i],&cfs_priority,&rtime,&retime,&stime);
    printf("\nPid: %d\tCfs priority: %d\t Run time: %d\tRunnable time: %d\tSleep time: %d\n",pids[i],cfs_priority,rtime,retime,stime);
  }
  for(int i = 0;i<4;i++){
    wait(0,"");
  }
  exit(0,"");
}
