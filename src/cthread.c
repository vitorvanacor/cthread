#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "../include/support.h"
#include "../include/cdata.h"

int schedulerIsInitialized = 0;
int schedulerNextTid = 1;

ucontext_t terminatedThreadContext, testContext;
FILA2 filaAptos;
TCB_t* usingCPU;



void createMainContext(){
    TCB_t* mainTCB = malloc(sizeof(TCB_t));
    mainTCB->tid = 0;
    mainTCB->state = PROCST_EXEC;
    mainTCB->ticket = Random2();
    printf("ticket da main: %d\n", mainTCB->ticket);
    usingCPU = mainTCB;
}

void SchedulerInitialize(void){
    if (schedulerIsInitialized) return;

    CreateFila2(&filaAptos);

    createMainContext();
    schedulerIsInitialized = 1;
}

TCB_t* getWinner(int winnerTicket){
    printf("ticket sorteado: %d\n", winnerTicket);
    LastFila2(&filaAptos);
    TCB_t* lastTCB = (TCB_t*)GetAtIteratorFila2(&filaAptos);
    FirstFila2(&filaAptos);
    TCB_t* currentTCB = (TCB_t*)GetAtIteratorFila2(&filaAptos);
    TCB_t* closestTCB = currentTCB;
    if (!currentTCB) {
        printf("fila vazia");
    }
    printf("primeiro da fila: %d, ticket %d\n",currentTCB->tid,currentTCB->ticket);
    while(currentTCB->tid != lastTCB->tid){
        NextFila2(&filaAptos);
        currentTCB = (TCB_t*)GetAtIteratorFila2(&filaAptos);
        printf("proximo da fila: %d, ticket %d\n",currentTCB->tid, currentTCB->ticket);
        if(currentTCB->ticket == closestTCB->ticket){
            if (currentTCB->tid < closestTCB->tid){
                closestTCB = currentTCB;
            }
        } else {
            if (isClosest(currentTCB->ticket, closestTCB->ticket, winnerTicket)){
                closestTCB = currentTCB;
            }
        }
        printf("mais proximo: %d da thread %d\n",closestTCB->ticket,closestTCB->tid);
    }
    return closestTCB;
}

void dispatch(void){
    //<Da pra colocar aqui algo testando se a fila eh vazia ou unitaria>
    printf("dispatcher foi chamado\n");
    int winnerTicket = Random2();
    TCB_t* winnerTCB = getWinner(winnerTicket);
    printf("vencedor: thread %d\n",winnerTCB->tid);
    if(findTCB(winnerTCB, &filaAptos)){
        DeleteAtIteratorFila2(&filaAptos);
    }
    usingCPU = winnerTCB;
    ucontext_t ctxt = winnerTCB->context;
    setcontext(&ctxt);
}

int findTCB(TCB_t* tcb, PFILA2 fila){
    FirstFila2(fila);
    TCB_t* currentTCB = (TCB_t*)GetAtIteratorFila2(fila);
    if (!currentTCB){
        return 0; //fila vazia
    }
    if (currentTCB == tcb) return 1;
    while(currentTCB != (TCB_t*)fila->last){
        NextFila2(fila);
        currentTCB = (TCB_t*)GetAtIteratorFila2(fila);
        if (currentTCB == tcb) return 1;
    }
    return 0; //nao encontrou
}

int isClosest(current, closest, winner){
    return (abs(winner-current) < abs(winner-closest));
}

int ccreate (void* (*start)(void*), void *arg){
    SchedulerInitialize();

    ucontext_t newContext;
    getcontext(&newContext);

    newContext.uc_link          = &terminatedThreadContext;      /* contexto a executar no término */
    newContext.uc_stack.ss_sp   = malloc(SIGSTKSZ);         /* endereço de início da pilha    */
    newContext.uc_stack.ss_size = SIGSTKSZ; /*tamanho da pilha */

    /* Define a função a ser executada pelo novo fluxo de controle,
     * forcene a quantidade (0, no caso) e os eventuais parâmetros que cada fluxo
     * recebe (nenhum). O typecast (void (*)(void)) é só para evitar warnings na
     * compilação e não afeta o comportamento da função */
    makecontext(&newContext, (void (*)(void)) start, 0);

    TCB_t* newTCB = malloc(sizeof(TCB_t));
    newTCB->tid = schedulerNextTid++;
    newTCB->state = PROCST_APTO;
    newTCB->ticket = Random2();
    printf("ticket da thread %d: %d\n",newTCB->tid,newTCB->ticket);
    newTCB->context = newContext;

    AppendFila2(&filaAptos, (void *)newTCB);

    return newTCB->tid;
}

int cyield() {
    printf("thread %d abdicou da CPU\n",usingCPU->tid);
    int flag = 0;
    ucontext_t currentContext;
    getcontext(&currentContext);
    usingCPU->context = currentContext;
    if (flag==0){
        flag = 1;
        AppendFila2(&filaAptos, (void *)usingCPU);
        dispatch();
    }

}

int cjoin(int tid){
    //<coloca tid atual como esperando por termino da tid recebida no parametro>
    //bloqueia thread atual
    dispatch();
}

int cidentify(char *name, int size) {
    char* names = "Vitor Vanacor 233207\nMatheus Pereira xxxxxx";
    int i;

    if (size > 44) return -1;

    do {
       name[i] = names[i];
    } while (names[i] != '\0');
}

void barber() {
    printf("entrou no barber\n");
    while (1){
        cyield();
        printf("barber ganhou a cpu\n");
    }
}

int main() {
    int tidBarber, n;
    getcontext(&testContext);
    tidBarber = ccreate (barber, (void *) NULL);
    printf("criou thread com tid %d\n", tidBarber);
    for(n=0;n<4;n++){
        cyield();
        printf("main ganhou a cpu, n=%d\n",n);
    }
    printf("\nFim do programa\n");
}
