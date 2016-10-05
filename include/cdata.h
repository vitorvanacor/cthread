/*
 * cdata.h: arquivo de inclus�o de uso apenas na gera��o da libpithread
 *
 * Esse arquivo pode ser modificado. ENTRETANTO, deve ser utilizada a TCB fornecida.
 *
 */
#ifndef __cdata__
#define __cdata__

#define	PROCST_CRIACAO	0
#define	PROCST_APTO	1
#define	PROCST_EXEC	2
#define	PROCST_BLOQ	3
#define	PROCST_TERMINO	4

/* N�O ALTERAR ESSA struct */
typedef struct s_TCB {
	int		tid; 		// identificador da thread
	int		state;		// estado em que a thread se encontra
					// 0: Cria��o; 1: Apto; 2: Execu��o; 3: Bloqueado e 4: T�rmino
        int		ticket;		// 0-255: bilhete de loteria da thread
	ucontext_t 	context;	// contexto de execu��o da thread (SP, PC, GPRs e recursos)
} TCB_t;

//Estrutura para registrar o aguardo de uma thread pelo termino de outra
typedef struct {
    int	waitedTid; 		// identificador da thread sendo esperada
	TCB_t* thread; 		// thread
} ThreadJoin;

TCB_t* getWinner();
void* dispatch(void);
void broadcastThreadEnd(int tid);
void SchedulerInitialize(void);
int findTCB(TCB_t* tcb, PFILA2 fila);
int tidIsWaited (int tid);
int findTidReady(int tid);
int findTidBlocked(int tid);
void printFilas();
void debugLog(char* format, ...);

#endif
