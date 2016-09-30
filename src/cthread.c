#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <support.h>
#include <cdata.h>
#include <cthread.h>

#define NEW_TICKET Random2() % 256

//Estrutura para registrar o aguardo de uma thread pelo termino de outra
typedef struct {
    int		waitedTid; 		// identificador da thread sendo esperada
	TCB_t*		thread; 		// thread
} ThreadJoin;

//Registra se o escalonador foi inicializado
int schedulerIsInitialized = 0;

//Registra o tid da proxima thread a ser criada
int schedulerNextTid = 1;

//Contexto e pilha do despachante
ucontext_t dispatcherContext;
char dispatcherStack[SIGSTKSZ];

//TCB da main
TCB_t mainTCB;

//Possiveis estados de uma thread: executando, apta ou bloqueada
TCB_t* threadUsingCPU;
FILA2 readyQueue;
FILA2 blockedQueue;

void printFilas() {

    int i=0;
    FirstFila2(&readyQueue);
    TCB_t* currentTCB = (TCB_t*)GetAtIteratorFila2(&readyQueue);
    if (!currentTCB){
        printf("Fila Aptos Vazia\n");
    }
    else {
        printf("---Fila aptos: \nPosicao %d, tid %d\n",i,currentTCB->tid);
        while(NextFila2(&readyQueue) == 0) {
            if (readyQueue.it == 0) {
                break;
            }
            i++;
            currentTCB = (TCB_t*)GetAtIteratorFila2(&readyQueue);
            printf("Posicao %d, tid %d\n",i,currentTCB->tid);
        }
    }
    FirstFila2(&readyQueue);

    i=0;
    FirstFila2(&blockedQueue);
    ThreadJoin* currentThreadJoin = (ThreadJoin*)GetAtIteratorFila2(&blockedQueue);
    if (!currentThreadJoin){
        printf("Fila Blocked Vazia\n");
        return; //fila vazia
    }
    printf("---Fila blocked: \nPosicao %d, tid %d\n",i,currentThreadJoin->thread->tid);
    while(NextFila2(&blockedQueue) == 0) {
        if (blockedQueue.it == 0) {
            break;
        }
        i++;
        currentThreadJoin = (ThreadJoin*)GetAtIteratorFila2(&blockedQueue);
        printf("Posicao %d, tid %d\n",i,currentThreadJoin->thread->tid);
    }
    FirstFila2(&blockedQueue);
}


int ccreate (void* (*start)(void*), void *arg){
    SchedulerInitialize();

    ucontext_t newContext;
    getcontext(&newContext);

    newContext.uc_link          = &dispatcherContext;
    newContext.uc_stack.ss_sp   = malloc(SIGSTKSZ);
    newContext.uc_stack.ss_size = SIGSTKSZ;

    makecontext(&newContext, (void (*)(void)) start, 1, arg);

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
    SchedulerInitialize();

    printf("\nYield(): Thread %d abdicou da CPU\n", threadUsingCPU->tid);

    TCB_t *thread = threadUsingCPU;
    thread->state = PROCST_APTO;

    if (AppendFila2(&readyQueue, (void *)thread) != 0) {
        return -500;
    }
    threadUsingCPU = NULL;

    swapcontext(&thread->context, &dispatcherContext);

    return 0;
}

int cjoin(int tid) {
    SchedulerInitialize();
    if (!findTidBlocked(tid) && !findTidReady(tid)) {
        return -404;
    }

    if (tidIsWaited(tid)) {
        return -400;
    }

    printf("\nJoin(): Thread %d esperando thread %d\n",threadUsingCPU->tid, tid);
    TCB_t *thread = threadUsingCPU;
    thread->state = PROCST_BLOQ;

    ThreadJoin* tjoin = malloc(sizeof(ThreadJoin));
    tjoin->waitedTid = tid;
    tjoin->thread = thread;

    if (AppendFila2(&blockedQueue, (void *)tjoin) != 0) {
        return -500;
    }

    threadUsingCPU = NULL;

    swapcontext(&thread->context, &dispatcherContext);

    return 0;
}

int csem_init (csem_t *sem, int count){
    SchedulerInitialize();
    sem->count = count;
    sem->fila = NULL;
    return 0;
}

int cwait (csem_t *sem){
    SchedulerInitialize();
    sem->count--;
    if (sem->count < 0){
        if (sem->fila == NULL){
            sem->fila = malloc(sizeof(FILA2));
            if (CreateFila2(sem->fila) != 0) {
                return -500;
            }
        }

        TCB_t *thread = threadUsingCPU;
        thread->state = PROCST_BLOQ;

        if (AppendFila2(sem->fila, (void *)thread) != 0) {
            return -500;
        }

        threadUsingCPU = NULL;
        swapcontext(&thread->context, &dispatcherContext);
    }
    return 0;
}

int csignal (csem_t *sem){
    SchedulerInitialize();
    sem->count++;
    if (sem->count <= 0){
        FirstFila2(sem->fila);
        TCB_t* firstTCB = (TCB_t*)GetAtIteratorFila2(sem->fila);
        if (DeleteAtIteratorFila2(sem->fila) != 0) {
            return -500;
        }

        if (AppendFila2(&readyQueue, (void *)firstTCB) != 0) {
            return -500;
        }
        firstTCB->state = PROCST_APTO;

        if(FirstFila2(sem->fila) != 0){
            free(sem->fila);
            sem->fila = NULL;
        }
    }
    return 0;
}

int cidentify(char *name, int size) {
    char* names = "Vitor Vanacor 233207\nMatheus Pereira 242247";
    int i = 0;

    if (size < 44) return -1;

    do {
        name[i] = names[i]; i++;
    } while (names[i] != '\0');

    return 0;
}

void SchedulerInitialize(void){
    if (schedulerIsInitialized) return;
    schedulerIsInitialized = 1;

    //Inicializa filas
    CreateFila2(&readyQueue);
    CreateFila2(&blockedQueue);

    //Inicializa TCB da main
    mainTCB.tid = 0;
    mainTCB.state = PROCST_EXEC;
    mainTCB.ticket = NEW_TICKET;
    printf("\n---Main Thread criada com Ticket: %d\n", mainTCB.ticket);
    threadUsingCPU = &mainTCB;

    //Inicializa despachante
    getcontext(&dispatcherContext);
    dispatcherContext.uc_link = 0;
    dispatcherContext.uc_stack.ss_sp = dispatcherStack;
    dispatcherContext.uc_stack.ss_size = SIGSTKSZ;
    makecontext(&dispatcherContext, (void (*)(void)) dispatch, 0);

}

void* dispatch(void) {
    //<Da pra colocar aqui algo testando se a fila eh vazia ou unitaria>
    printf("\nDispatcher()\n");
    if(threadUsingCPU){
        printf("Finalizando thread %d\n", threadUsingCPU->tid);
        broadcastThreadEnd(threadUsingCPU->tid);
        free(threadUsingCPU->context.uc_stack.ss_sp);
        free(threadUsingCPU);
        threadUsingCPU = NULL;
    }
    threadUsingCPU = getWinner();
    findTCB(threadUsingCPU, &readyQueue);
    DeleteAtIteratorFila2(&readyQueue);

    setcontext(&threadUsingCPU->context);

    return 0;
}

void broadcastThreadEnd(int tid) {
    if (FirstFila2(&blockedQueue) != 0){
        return; //fila vazia
    }
    ThreadJoin* currentThreadJoin;

    do {
        if (blockedQueue.it == 0) {
            break;
        }
        currentThreadJoin = (ThreadJoin*)GetAtIteratorFila2(&blockedQueue);

        if(currentThreadJoin->waitedTid == tid) {
            currentThreadJoin->thread->state = PROCST_APTO;
            AppendFila2(&readyQueue, (void *)currentThreadJoin->thread);
            printf("\nThread %d voltou a estar apta\n", currentThreadJoin->thread->tid);
            DeleteAtIteratorFila2(&blockedQueue);
            free(currentThreadJoin);
            return;
        }
    } while(NextFila2(&blockedQueue) == 0);
}

TCB_t* getWinner() {
    int winnerTicket = NEW_TICKET;
    printf("\nSorteio(): ticket sorteado: %d\n", winnerTicket);
    FirstFila2(&readyQueue);
    TCB_t* currentTCB = (TCB_t*)GetAtIteratorFila2(&readyQueue);
    TCB_t* closestTCB = currentTCB;
    printf("----Primeiro da fila: %d, ticket %d\n",currentTCB->tid,currentTCB->ticket);
    while(NextFila2(&readyQueue) == 0){
        if (readyQueue.it == 0) {
            break;
        }
        currentTCB = (TCB_t*)GetAtIteratorFila2(&readyQueue);
        printf("--Proximo da fila: %d, ticket %d\n",currentTCB->tid, currentTCB->ticket);
        if(currentTCB->ticket == closestTCB->ticket){
            if (currentTCB->tid < closestTCB->tid){
                closestTCB = currentTCB;
            }
        } else {
            if (abs(winnerTicket - currentTCB->ticket) < abs(winnerTicket - closestTCB->ticket)){
                closestTCB = currentTCB;
            }
        }
        printf("-Thread %d com ticket %d eh (agora) a mais proxima do ticket sorteado %d\n",closestTCB->tid,closestTCB->ticket,winnerTicket);
    }
    printf("Vencedor: thread %d\n", closestTCB->tid);
    return closestTCB;
}

int findTCB(TCB_t* tcb, PFILA2 fila){
    FirstFila2(fila);
    TCB_t* currentTCB = (TCB_t*)GetAtIteratorFila2(fila);
    if (!currentTCB){
        return 0; //fila vazia
    }
    if (currentTCB == tcb) return 1;
    while(NextFila2(fila) == 0){
        if (fila->it == 0) {
            break;
        }
        currentTCB = (TCB_t*)GetAtIteratorFila2(fila);
        if (currentTCB == tcb) return 1;
    }
    return 0; //nao encontrou
}

int findTidBlocked(int tid) {
    if (FirstFila2(&blockedQueue) != 0){
        return 0; //fila vazia
    }
    ThreadJoin* currentThreadJoin;

    do {
        if (blockedQueue.it == 0) {
            break;
        }
        currentThreadJoin = (ThreadJoin*)GetAtIteratorFila2(&blockedQueue);

        if(currentThreadJoin->thread->tid == tid) {
            return 1;
        }
    } while(NextFila2(&blockedQueue) == 0);

    return 0;
}

int findTidReady(int tid) {
    if (FirstFila2(&readyQueue) != 0) {
        return 0;
    }
    TCB_t* currentTCB;

    do {
        if (readyQueue.it == 0) {
            break;
        }
        currentTCB = (TCB_t*)GetAtIteratorFila2(&readyQueue);

        if(currentTCB->tid == tid) {
            return 1;
        }
    }
    while(NextFila2(&readyQueue) == 0);

    return 0;
}

int tidIsWaited (int tid) {
    if (FirstFila2(&blockedQueue) != 0){
        return 0; //fila vazia
    }
    ThreadJoin* currentThreadJoin;

    do {
        if (blockedQueue.it == 0) {
            break;
        }
        currentThreadJoin = (ThreadJoin*)GetAtIteratorFila2(&blockedQueue);

        if(currentThreadJoin->waitedTid == tid) {
            return 1;
        }
    } while(NextFila2(&blockedQueue) == 0);

    return 0;
}
