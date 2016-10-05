#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <stdarg.h>
#include <support.h>
#include <cdata.h>
#include <cthread.h>

#define DEBUG_LOG 1 // Deve ser 0 para nao mostrar as mensagens de debug

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
//Ponteiro para TCB da thread executando
TCB_t* threadUsingCPU;
//Fila de threads no estado apto
FILA2 readyQueue;
//Fila de estruturas que registram threads esperando o termino de outra (threads bloqueadas por join)
FILA2 blockedQueue;

int ccreate (void* (*start)(void*), void *arg){
    SchedulerInitialize(); //Inicializa escalonador

    //Novo contexto
    ucontext_t newContext;
    getcontext(&newContext);

    newContext.uc_link          = &dispatcherContext;
    newContext.uc_stack.ss_sp   = malloc(SIGSTKSZ);
    newContext.uc_stack.ss_size = SIGSTKSZ;

    makecontext(&newContext, (void (*)(void)) start, 1, arg);

    //Novo TCB
    TCB_t* newTCB = malloc(sizeof(TCB_t));
    newTCB->tid = schedulerNextTid++;
    newTCB->state = PROCST_APTO;
    newTCB->ticket = Random2() % 256;
    newTCB->context = newContext;
    debugLog("\nCreate(): Thread %d criada, ticket: %d\n",newTCB->tid,newTCB->ticket);

    //Coloca na fila de aptos
    AppendFila2(&readyQueue, (void *)newTCB);

    return newTCB->tid;
}

int cyield() {
    SchedulerInitialize(); //Inicializa escalonador

    debugLog("\nYield(): Thread %d abdicou da CPU\n", threadUsingCPU->tid);

    TCB_t *thread = threadUsingCPU;
    thread->state = PROCST_APTO;

    //Coloca na fila de aptos
    if (AppendFila2(&readyQueue, (void *)thread) != 0) {
        return -500;
    }
    threadUsingCPU = NULL;

    //Passa execução para o despachante
    swapcontext(&thread->context, &dispatcherContext);

    return 0;
}

int cjoin(int tid) {
    SchedulerInitialize(); //Inicializa escalonador

    debugLog("\nJoin(): Thread %d esperando thread %d\n",threadUsingCPU->tid, tid);

    //Verifica se a tid dada está bloqueada ou apta
    if (!findTidBlocked(tid) && !findTidReady(tid)) {
        debugLog("Thread %d nao encontrada.\n", tid);
        return -404;
    }
    //Verifica se a thread ja nao esta sendo esperada por outra
    if (tidIsWaited(tid)) {
        debugLog("Ja ha outra thread esperando por %d\n", tid);
        return -400;
    }

    TCB_t *thread = threadUsingCPU;
    thread->state = PROCST_BLOQ;

    //Cria a estrutura
    ThreadJoin* tjoin = malloc(sizeof(ThreadJoin));
    tjoin->waitedTid = tid;
    tjoin->thread = thread;

    //Coloca na fila de bloqueados
    if (AppendFila2(&blockedQueue, (void *)tjoin) != 0) {
        return -500;
    }

    threadUsingCPU = NULL;

    //Passa execucao para o despachante
    swapcontext(&thread->context, &dispatcherContext);

    return 0;
}

int csem_init (csem_t *sem, int count){
    sem->count = count;
    sem->fila = NULL;
    return 0;
}

int cwait (csem_t *sem){
    SchedulerInitialize(); //Inicializa escalonador
    debugLog("\nWait(): Requisicao de recurso\n");

    sem->count--;
    //Se o recurso nao esta disponivel
    if (sem->count < 0){
        debugLog("Recurso indisponivel. Thread bloqueada\n");
        if (sem->fila == NULL){
            sem->fila = malloc(sizeof(FILA2));
            if (CreateFila2(sem->fila) != 0) {
                return -500;
            }
        }

        TCB_t *thread = threadUsingCPU;
        thread->state = PROCST_BLOQ;

        //Coloca na fila do semaforo
        if (AppendFila2(sem->fila, (void *)thread) != 0) {
            return -500;
        }

        threadUsingCPU = NULL;
        //Passa contexto para despachante
        swapcontext(&thread->context, &dispatcherContext);
    }
    debugLog("Recurso disponivel\n");
    return 0;
}

int csignal (csem_t *sem){
    SchedulerInitialize(); //Inicializa escalonador
    debugLog("\nSignal(): Liberacao de recurso\n");

    sem->count++;
    //Se o recusro estava sendo aguardado
    if (sem->count <= 0){
        FirstFila2(sem->fila);
        TCB_t* firstTCB = (TCB_t*)GetAtIteratorFila2(sem->fila);
        //Tira da fila do semaforo
        if (DeleteAtIteratorFila2(sem->fila) != 0) {
            return -500;
        }
        //Coloca na fila de aptos
        if (AppendFila2(&readyQueue, (void *)firstTCB) != 0) {
            return -500;
        }
        firstTCB->state = PROCST_APTO;
        debugLog("Thread %d volta a estar apta\n", firstTCB->tid);

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
    mainTCB.ticket = Random2() % 256;
    debugLog("\n---Main Thread criada com Ticket: %d\n", mainTCB.ticket);
    threadUsingCPU = &mainTCB;

    //Inicializa despachante
    getcontext(&dispatcherContext);
    dispatcherContext.uc_link = 0;
    dispatcherContext.uc_stack.ss_sp = dispatcherStack;
    dispatcherContext.uc_stack.ss_size = SIGSTKSZ;
    makecontext(&dispatcherContext, (void (*)(void)) dispatch, 0);

}

void* dispatch(void) {
    debugLog("\nDispatcher()\n");
    if(threadUsingCPU){
        debugLog("Finalizando thread %d\n", threadUsingCPU->tid);
        broadcastThreadEnd(threadUsingCPU->tid);
        free(threadUsingCPU->context.uc_stack.ss_sp);
        free(threadUsingCPU);
        threadUsingCPU = NULL;
    }
    threadUsingCPU = getWinner();
    findTCB(threadUsingCPU, &readyQueue);
    DeleteAtIteratorFila2(&readyQueue);

    threadUsingCPU->state = PROCST_EXEC;
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
            debugLog("Thread %d volta a estar apta\n", currentThreadJoin->thread->tid);
            DeleteAtIteratorFila2(&blockedQueue);
            free(currentThreadJoin);
            return;
        }
    } while(NextFila2(&blockedQueue) == 0);
}

TCB_t* getWinner() {
    int winnerTicket = Random2() % 256;
    debugLog("\nSorteio(): ticket sorteado: %d\n", winnerTicket);
    FirstFila2(&readyQueue);
    TCB_t* currentTCB = (TCB_t*)GetAtIteratorFila2(&readyQueue);
    TCB_t* closestTCB = currentTCB;
    debugLog("----Primeiro da fila: %d, ticket %d\n",currentTCB->tid,currentTCB->ticket);
    while(NextFila2(&readyQueue) == 0){
        if (readyQueue.it == NULL) break;
        currentTCB = (TCB_t*)GetAtIteratorFila2(&readyQueue);
        debugLog("--Proximo da fila: %d, ticket %d\n",currentTCB->tid, currentTCB->ticket);
        if(currentTCB->ticket == closestTCB->ticket){
            if (currentTCB->tid < closestTCB->tid){
                closestTCB = currentTCB;
            }
        } else {
            if (abs(winnerTicket - currentTCB->ticket) < abs(winnerTicket - closestTCB->ticket)){
                closestTCB = currentTCB;
            }
        }
        debugLog("-Thread %d com ticket %d eh (agora) a mais proxima do ticket sorteado %d\n",closestTCB->tid,closestTCB->ticket,winnerTicket);
    }
    debugLog("Vencedor: thread %d\n", closestTCB->tid);
    return closestTCB;
}

int findTCB(TCB_t* tcb, PFILA2 fila){
    if (FirstFila2(fila) != 0){
        return 0; //fila vazia
    }
    TCB_t* currentTCB;
    do {
        if (fila->it == NULL) break;

        currentTCB = (TCB_t*)GetAtIteratorFila2(fila);
        if (currentTCB == tcb) return 1; //Encontrou
    } while(NextFila2(fila) == 0);
    return 0; //nao encontrou
}

int findTidBlocked(int tid) {
    if (FirstFila2(&blockedQueue) != 0){
        return 0; //fila vazia
    }
    ThreadJoin* currentThreadJoin;

    do {
        if (blockedQueue.it == NULL) break;

        currentThreadJoin = (ThreadJoin*)GetAtIteratorFila2(&blockedQueue);
        if(currentThreadJoin->thread->tid == tid) return 1; //Encontrou
    } while(NextFila2(&blockedQueue) == 0);

    return 0; //nao encontrou
}

int findTidReady(int tid) {
    if (FirstFila2(&readyQueue) != 0) {
        return 0;
    }
    TCB_t* currentTCB;

    do {
        if (readyQueue.it == NULL) break;

        currentTCB = (TCB_t*)GetAtIteratorFila2(&readyQueue);

        if(currentTCB->tid == tid) return 1; //encontrou

    }
    while(NextFila2(&readyQueue) == 0);

    return 0; //nao encontrou
}

int tidIsWaited (int tid) {
    if (FirstFila2(&blockedQueue) != 0){
        return 0; //fila vazia
    }
    ThreadJoin* currentThreadJoin;

    do {
        if (blockedQueue.it == NULL) {
            break;
        }
        currentThreadJoin = (ThreadJoin*)GetAtIteratorFila2(&blockedQueue);

        if(currentThreadJoin->waitedTid == tid) return 1; //Encontrou
    } while(NextFila2(&blockedQueue) == 0);

    return 0; //nao encontrou
}

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

void debugLog(char* format, ...){
    va_list args;
    va_start(args, format);
    if(DEBUG_LOG)
        vprintf(format, args);
    va_end(args);
}

