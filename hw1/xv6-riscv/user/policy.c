#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int
main(int argc, char *argv[])
{
  if(argc==2){
        if(*argv[1] >= '0' && *argv[1] <= '2'){
          int policy = atoi(argv[1]);
          if(set_policy(policy) == 1){
            if(policy == 0){
              printf("success policy set to default\n");
            }       
            if(policy == 1){
              printf("success policy set to priority acc\n");
            }
            if(policy == 2){
              printf("success policy set to Cfs\n");
            }
          }
          else
            exit(0,"fail policy set");
        }
    } 
    exit(1,"");
}

