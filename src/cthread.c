int schedulerIsInitialized = 0;
int schedulerNextTid = 1;

ucontext_t schedulerContext;
Fila2 filaAptos;
TCB* usingCPU;

void SchedulerInitialize(void){
    if (schedulerIsInitialized) return;

    createFila2(&filaAptos);

    createMainContext();
}

void createMainContext(){
    ucontext_t mainContext;
    getcontext(&mainContext);

    TCB_t* mainTCB = malloc(sizeof(TCB_t));
    mainTCB->tid = 0;
    mainTCB->state = PROCST_EXEC;
    mainTCB->ticket = Random2();
    mainTCB->context = mainContext;
    usingCPU = mainTCB;
}


int ccreate (void* (*start)(void*), void *arg){
    SchedulerInitialize();

    ucontext_t newContext;
    char newStack[SIGSTKSZ];

    newContext.uc_link          = &schedulerContext;      /* contexto a executar no término */
    newContext.uc_stack.ss_sp   = newStack;         /* endereço de início da pilha    */
    newContext.uc_stack.ss_size = sizeof(newStack); /*tamanho da pilha */

    /* Define a função a ser executada pelo novo fluxo de controle,
     * forcene a quantidade (0, no caso) e os eventuais parâmetros que cada fluxo
     * recebe (nenhum). O typecast (void (*)(void)) é só para evitar warnings na
     * compilação e não afeta o comportamento da função */
    makecontext(&newContext, (void (*)(void)) start, 0);

    TCB_t* newTCB = malloc(sizeof(TCB_t));
    newTCB->tid = schedulerNextTid++;
    newTCB->state = 0;
    newTCB->ticket = Random2();
    newTCB->context = newContext;

    AppendFila2(&filaAptos, (void *)(&newTCB));

    return newTCB.tid;
}

int cyield(void){
    //coloca thread atual na lista de aptos
    schedule();
}

int cjoin(int tid){
    //<coloca tid atual como esperando por termino da tid recebida no parametro>
    //bloqueia thread atual
    schedule();
}

int cidentify (char *name, int size){
    char[] names = "Vitor Vanacor 233207\nMatheus Pereira xxxxxx";

    if (size > 44) return -1;

    do {
       name[i] = names[i];
    } while (names[i] != '\0')
}
