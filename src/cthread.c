#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "../include/support.h"
#include "../include/cdata.h"

#define NEW_TICKET Random2() % 256

int schedulerIsInitialized = 0;
int schedulerNextTid = 1;

ucontext_t terminateContext;;
FILA2 readyQueue;
FILA2 blockedQueue;
TCB_t* threadUsingCPU;

typedef struct {
    int		waitedTid; 		// identificador da thread sendo esperada
	TCB_t*		thread; 		// thread
} ThreadJoin;

TCB_t* getWinner() {
    int winnerTicket = NEW_TICKET;
    printf("\nSorteio(): ticket sorteado: %d\n", winnerTicket);
    FirstFila2(&readyQueue);
    TCB_t* currentTCB = (TCB_t*)GetAtIteratorFila2(&readyQueue);
    TCB_t* closestTCB = currentTCB;
    printf("----Primeiro da fila: %d, ticket %d\n",currentTCB->tid,currentTCB->ticket);
    while(NextFila2(&readyQueue) != 0){
        currentTCB = (TCB_t*)GetAtIteratorFila2(&readyQueue);
        printf("--Proximo da fila: %d, ticket %d\n",currentTCB->tid, currentTCB->ticket);
        if(currentTCB->ticket == closestTCB->ticket){
            if (currentTCB->tid < closestTCB->tid){
                closestTCB = currentTCB;
            }
        } else {
            if (isClosest(currentTCB->ticket, closestTCB->ticket, winnerTicket)){
                closestTCB = currentTCB;
            }
        }
        printf("-Thread %d com ticket %d eh (agora) a mais proxima do ticket sorteado %d\n",closestTCB->tid,closestTCB->ticket,winnerTicket);
    }
    printf("Vencedor: thread %d\n", closestTCB->tid);
    return closestTCB;
}

void dispatch(void) {
    //<Da pra colocar aqui algo testando se a fila eh vazia ou unitaria>
    printf("\nDispatcher()\n");
    TCB_t* winnerTCB = getWinner();
    findTCB(winnerTCB, &readyQueue);
    DeleteAtIteratorFila2(&readyQueue);

    threadUsingCPU = winnerTCB;
    ucontext_t ctxt = winnerTCB->context;
    setcontext(&ctxt);
}

void broadcastThreadEnd(int tid) {
    ThreadJoin* currentThreadJoin = (ThreadJoin*)GetAtIteratorFila2(&blockedQueue);
    if (!currentThreadJoin){
        return; //fila vazia
    }

    if(currentThreadJoin->waitedTid == tid) {
        currentThreadJoin->thread->state = PROCST_APTO;
        AppendFila2(&readyQueue, (void *)currentThreadJoin->thread);
        return;
    }

    while(NextFila2(&readyQueue) != 0) {
        currentThreadJoin = (ThreadJoin*)GetAtIteratorFila2(&blockedQueue);

        if(currentThreadJoin->waitedTid == tid) {
            currentThreadJoin->thread->state = PROCST_APTO;
            AppendFila2(&readyQueue, (void *)currentThreadJoin->thread);
            return;
        }
    }
}

void terminateThread(void){
    printf("\nFinalizando thread %d\n", threadUsingCPU->tid);
    broadcastThreadEnd(threadUsingCPU->tid);
    ucontext_t ctxt = threadUsingCPU->context;
    free(ctxt.uc_stack.ss_sp);
    free(threadUsingCPU);
    dispatch();
}

void createTerminateContext(void){
    getcontext(&terminateContext);

    terminateContext.uc_stack.ss_sp   = malloc(SIGSTKSZ);         /* endere?o de in?cio da pilha    */
    terminateContext.uc_stack.ss_size = SIGSTKSZ; /*tamanho da pilha */

    makecontext(&terminateContext, (void (*)(void)) terminateThread, 0);
}

void createMainTCB(void){
    TCB_t* mainTCB = malloc(sizeof(TCB_t));
    mainTCB->tid = 0;
    mainTCB->state = PROCST_EXEC;
    mainTCB->ticket = NEW_TICKET;
    printf("\n---Main Thread criada com Ticket: %d\n", mainTCB->ticket);
    threadUsingCPU = mainTCB;
}

void SchedulerInitialize(void){
    if (schedulerIsInitialized) return;

    CreateFila2(&readyQueue);
    CreateFila2(&blockedQueue);

    createMainTCB();
    schedulerIsInitialized = 1;

    createTerminateContext();
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

    char stack[SIGSTKSZ];

    newContext.uc_link          = &terminateContext;      /* contexto a executar no t?rmino */
    newContext.uc_stack.ss_sp   = stack;         /* endere?o de in?cio da pilha    */
    newContext.uc_stack.ss_size = SIGSTKSZ;                  /*tamanho da pilha */

    /* Define a funcao a ser executada pelo novo fluxo de controle,
     * forne a quantidade (0, no caso) e os eventuais par?metros que cada fluxo
     * recebe (nenhum). O typecast (void (*)(void)) ? s? para evitar warnings na
     * compilacao e nao afeta o comportamento da fun??o */
    makecontext(&newContext, (void (*)(void)) start, 0);

    TCB_t* newTCB = malloc(sizeof(TCB_t));
    newTCB->tid = schedulerNextTid++;
    newTCB->state = PROCST_APTO;
    newTCB->ticket = NEW_TICKET;
    newTCB->context = newContext;
    printf("\nCreate(): Thread %d criada, ticket: %d\n",newTCB->tid,newTCB->ticket);

    AppendFila2(&readyQueue, (void *)newTCB);

    return newTCB->tid;
}

int cyield() {
    printf("\nYield(): Thread %d abdicou da CPU\n",threadUsingCPU->tid);
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

int cjoin(int tid) {
    printf("\nJoin(): Thread %d esperando thread %d\n",threadUsingCPU->tid, tid);
    ThreadJoin* tjoin = malloc(sizeof(ThreadJoin));
    tjoin->waitedTid = tid;
    threadUsingCPU->state = PROCST_BLOQ;
    tjoin->thread = threadUsingCPU;

    AppendFila2(&blockedQueue, (void *)tjoin);
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
    int n;
    printf("\n----Entrou no barber\n");
    for(n=0;n<4;n++) {
        cyield();
        printf("\n--Barber ganhou a cpu\n");
    }

}

int main() {
    int tidBarber, n;
    tidBarber = ccreate (barber, (void *) NULL);
    cjoin(tidBarber);
    /*for(n=0;n<4;n++){
        cyield();
        printf("\n--Main ganhou a cpu, n=%d\n",n);
    }*/
    printf("\nFim do programa\n");
}
