/*
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<errno.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include <stdio.h>


#define   CHAIRS 5

time_t end_time;

csem_t customers;
csem_t mutex;
csem_t barbers;
int    waiting = 0;

void sleepao() {
     int i = 0;

     i = rand()%5 + 1;
     for (; i>0; i--) {
         cyield();
     }
     return;
}

void cut_hair(void)
{
    cyield(); cyield(); cyield(); cyield();
    return;
}

void* barber(void* arg)
{
   while(time(NULL)<end_time || waiting > 0) {
     cwait(&customers);
     cwait(&mutex);
     waiting = waiting - 1;
     printf("Barbeiro trabalhando, %d clientes esperam!! \n",waiting);
     csignal(&mutex);
     cut_hair();
     csignal(&barbers);
  }
  printf("--------------------------------------------SAIU WHILE BARB\n");
  return;
}

void* customer(void* arg)
{
   while(time(NULL) < end_time) {
      cwait(&mutex);
      if (waiting < CHAIRS) {
         waiting = waiting + 1;
         printf(" ---> Cliente chegando. H� %d clientes esperando.\n", waiting);
         csignal(&customers);
         csignal(&mutex);
         cwait(&barbers);
      } else {
        printf("  ***Cliente indo embora. N�o h� mais cadeiras.\n");
        csignal(&mutex);
      }
      sleepao();
    }
    printf("-----------------------------------------SAIU WHILE CUST\n");
    return;
}
*/
#include "../include/support.h"
#include "../include/cthread.h"
#include <stdio.h>

void function1() {
    printf("Function 1 yielding\n");
    cyield();
    printf("Function 1 yielding\n");
    cyield();
    printf("Function 1 yielding\n");
    cyield();
}

void function2() {
    printf("Function 2 yielding\n");
    cyield();
    printf("Function 2 yielding\n");
    cyield();
    printf("Function 2 yielding\n");
    cyield();
}

void function3() {
    printf("Function 3 yielding\n");
    cyield();
    printf("Function 3 yielding\n");
    cyield();
    printf("Function 3 yielding\n");
    cyield();
}

int main(int argc, char **argv) {
    int tid1, tid2, tid3;

    printf("Main printing something\n");

    tid1 = ccreate (function1, NULL);
    if (tid1 < 0 )
       printf("Error creating thread for function 1...\n");

    tid2 = ccreate (function2, NULL);
    if (tid2 < 0 )
       printf("Error creating thread for function 2...\n");

    tid3 = ccreate (function3, NULL);
    if (tid3 < 0 )
       printf("Error creating thread for function 3...\n");

    printf("Main joining function 1\n");
    cjoin(1);

    printf("Main joining function 2\n");
    if (cjoin(2) != 0) {
        printf("Function 2 is already finished\n");
    }

    printf("Main joining function 3\n");
    if (cjoin(3) != 0) {
        printf("Function 3 is already finished\n");
    }

    printf(">>Main ending\n");

    return 0;
}
