//#include <stdio.h>
//#include <stdlib.h>
//#include <ucontext.h>
//#include <support.h>
//#include <cdata.h>
//#include <cthread.h>
//
//void barber() {
//    int n;
//    printf("\n----Entrou no barber\n");
//    for(n=0;n<3;n++) {
//        printf("\n--Barber ganhou a cpu, n=%d\n",n);
//        cyield();
//    }
//    printf("Fim da barber\n");
//}
//
//int main() {
//    int tidBarber;
//
//    printf("Inicio do programaAAAAAAAa\n");
//    tidBarber = ccreate ((void *)barber, (void *) NULL);
//    cjoin(tidBarber);
//    printf("\n--Main ganhou a cpu");
//    printf("\nFim do programa\n");
//    return 0;
//}

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
     for (; i<0; i--) cyield();
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
  printf("SAIU WHILE BARB\n");
  return;
}

void* customer(void* arg)
{
   while(time(NULL) < end_time+2) {
      cwait(&mutex);
      if (waiting < CHAIRS) {
         waiting = waiting + 1;
         printf(" ---> Cliente chegando. Há %d clientes esperando.\n", waiting);
         csignal(&customers);
         csignal(&mutex);
         cwait(&barbers);
      } else {
        printf("  ***Cliente indo embora. Não há mais cadeiras.\n");
        csignal(&mutex);
      }
      sleepao();
    }
    printf("SAIU WHILE CUST\n");
    return;
}

int main(int argc, char **argv)
{
    int tidBarber, tidCustomer;

    end_time=time(NULL)+1;  /*Barbearia fica aberta 120 s */
    srand((unsigned)time(NULL));

    csem_init(&customers, 0);
    csem_init(&barbers, 1);
    csem_init(&mutex, 1);

    tidBarber = ccreate (barber, (void *) NULL);
    if (tidBarber < 0 )
       perror("Erro na criação do Barbeiro...\n");

    tidCustomer = ccreate (customer, (void *) NULL);
    if (tidCustomer < 0 )
       perror("Erro na criação do gerador de clientes...\n");

    cjoin(tidBarber);
    cjoin(tidCustomer);

    exit(0);
}
