#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "../include/support.h"
#include "../include/cdata.h"

int schedulerIsInitialized = 0;
int schedulerNextTid = 1;

ucontext_t terminatedThreadContext;
FILA2 filaAptos;
TCB_t* usingCPU = malloc(sizeof(TCB_t));


void createMainContext(){
    TCB_t* mainTCB = malloc(sizeof(TCB_t));
    mainTCB->tid = 0;
    mainTCB->state = PROCST_EXEC;
    mainTCB->ticket = Random2();
    usingCPU = mainTCB;
}

void SchedulerInitialize(void){
    if (schedulerIsInitialized) return;

    CreateFila2(&filaAptos);

    createMainContext();
}

TCB_t* getWinner(int winnerTicket){
    FirstFila2(&filaAptos);
    TCB_t* currentTCB = (TCB_t*)GetAtIteratorFila2(&filaAptos);
    TCB_t* closestTCB = currentTCB;
    if (!currentTCB) {
        //Fila de aptos vazia, tem que ver o que fazer
    }
    while(currentTCB != (TCB_t*)filaAptos.last){
        NextFila2(&filaAptos);
        currentTCB = (TCB_t*)GetAtIteratorFila2(&filaAptos);
        if(currentTCB->ticket == closestTCB->ticket){
            if (currentTCB->tid < closestTCB->tid){
                closestTCB = currentTCB;
            }
        } else {
            if (isClosest(currentTCB->ticket, closestTCB->ticket, winnerTicket)){
                closestTCB = currentTCB;
            }
        }
    }
    return closestTCB;
}

void dispatch(void){
    //<Da pra colocar aqui algo testando se a fila eh vazia ou unitaria>
    int winnerTicket = Random2();
    TCB_t* winnerTCB = getWinner(winnerTicket);
    if(findTCB(winnerTCB, &filaAptos)){
        DeleteAtIteratorFila2(&filaAptos);
    }
    usingCPU = winnerTCB;
    setcontext(&winnerTCB->context);
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
    char newStack[SIGSTKSZ];

    newContext.uc_link          = &terminatedThreadContext;      /* contexto a executar no término */
    newContext.uc_stack.ss_sp   = newStack;         /* endereço de início da pilha    */
    newContext.uc_stack.ss_size = sizeof(newStack); /*tamanho da pilha */

    /* Define a função a ser executada pelo novo fluxo de controle,
     * forcene a quantidade (0, no caso) e os eventuais parâmetros que cada fluxo
     * recebe (nenhum). O typecast (void (*)(void)) é só para evitar warnings na
     * compilação e não afeta o comportamento da função */
    makecontext(&newContext, (void (*)(void)) start, 0);

    TCB_t* newTCB = malloc(sizeof(TCB_t));
    newTCB->tid = schedulerNextTid++;
    newTCB->state = PROCST_APTO;
    newTCB->ticket = Random2();
    newTCB->context = newContext;

    AppendFila2(&filaAptos, (void *)(&newTCB));

    return newTCB->tid;
}

void saveContext(void){
    printf("a\n");
    ucontext_t currentContext;
    printf("b\n");
    getcontext(&currentContext);
    usingCPU->state = 3;
    printf("c\n");
    usingCPU->context = currentContext;
    printf("d\n");
}

int cyield() {
    printf("1\n");
    saveContext();
    printf("2\n");
    AppendFila2(&filaAptos, (void *)usingCPU);
    printf("3\n");
    printf("%d\n", usingCPU->tid);
    printf("4\n");
    dispatch();
    printf("5\n");
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
    printf("jorge\n");
    cyield();
    printf("maicon\n");
}

int main() {
    int tidBarber;
    printf("teste\n");
    SchedulerInitialize();
    printf("jorginho\n");
    TCB_t* marcos = malloc(sizeof(TCB_t));
    marcos->tid = 3;
    printf("%d\n\n\n",marcos->tid);
    //tidBarber = ccreate (barber, (void *) NULL);
    cyield();
    //printf("criou, %d\n", tidBarber);
}
