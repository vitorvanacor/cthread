#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "../include/support.h"
#include "../include/cdata.h"

int schedulerIsInitialized = 0;
int schedulerNextTid = 1;

ucontext_t terminatedThreadContext;
FILA2 readyQueue;
TCB_t* threadUsingCPU;

void createMainContext(void){
    TCB_t* mainTCB = malloc(sizeof(TCB_t));
    mainTCB->tid = 0;
    mainTCB->state = PROCST_EXEC;
    mainTCB->ticket = Random2();
    printf("ticket da main: %d\n", mainTCB->ticket);
    threadUsingCPU = mainTCB;
}

void SchedulerInitialize(void){
    if (schedulerIsInitialized) return;

    CreateFila2(&readyQueue);

    createMainContext();
    schedulerIsInitialized = 1;
}

TCB_t* getWinner(int winnerTicket){
    printf("ticket sorteado: %d\n", winnerTicket);
    FirstFila2(&readyQueue);
    TCB_t* currentTCB = (TCB_t*)GetAtIteratorFila2(&readyQueue);
    TCB_t* closestTCB = currentTCB;
    printf("primeiro da fila: %d, ticket %d\n",currentTCB->tid,currentTCB->ticket);
    while(NextFila2(&readyQueue) != 0){
        currentTCB = (TCB_t*)GetAtIteratorFila2(&readyQueue);
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
        printf("thread %d com ticket %d eh (agora) o mais proximo do sorteado %d\n",closestTCB->tid,closestTCB->ticket,winnerTicket);
    }
    return closestTCB;
}

void dispatch(void){
    //<Da pra colocar aqui algo testando se a fila eh vazia ou unitaria>
    printf("dispatcher acionado\n");
    int winnerTicket = Random2();
    TCB_t* winnerTCB = getWinner(winnerTicket);
    printf("vencedor: thread %d\n",winnerTCB->tid);
    findTCB(winnerTCB, &readyQueue);
    DeleteAtIteratorFila2(&readyQueue);

    threadUsingCPU = winnerTCB;
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
    while(NextFila2(fila) != 0){
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

    newContext.uc_link          = &terminatedThreadContext;      /* contexto a executar no t?rmino */
    newContext.uc_stack.ss_sp   = malloc(SIGSTKSZ);         /* endere?o de in?cio da pilha    */
    newContext.uc_stack.ss_size = SIGSTKSZ; /*tamanho da pilha */

    /* Define a funcao a ser executada pelo novo fluxo de controle,
     * forne a quantidade (0, no caso) e os eventuais par?metros que cada fluxo
     * recebe (nenhum). O typecast (void (*)(void)) ? s? para evitar warnings na
     * compilacao e nao afeta o comportamento da fun??o */
    makecontext(&newContext, (void (*)(void)) start, 0);

    TCB_t* newTCB = malloc(sizeof(TCB_t));
    newTCB->tid = schedulerNextTid++;
    newTCB->state = PROCST_APTO;
    newTCB->ticket = Random2();
    newTCB->context = newContext;
    printf("thread %d criada, ticket: %d\n",newTCB->tid,newTCB->ticket);

    AppendFila2(&readyQueue, (void *)newTCB);

    return newTCB->tid;
}

int cyield() {
    printf("thread %d abdicou da CPU\n",threadUsingCPU->tid);
    int flag = 0;
    ucontext_t currentContext;
    getcontext(&currentContext);
    threadUsingCPU->context = currentContext;
    if (flag==0){
        flag = 1;
        AppendFila2(&readyQueue, (void *)threadUsingCPU);
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
    tidBarber = ccreate (barber, (void *) NULL);
    printf("criou thread com tid %d\n", tidBarber);
    for(n=0;n<4;n++){
        cyield();
        printf("main ganhou a cpu, n=%d\n",n);
    }
    printf("\nFim do programa\n");
}
