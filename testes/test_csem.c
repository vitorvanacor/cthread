#include <stdio.h>
#include <stdlib.h>

#include "../include/support.h"
#include "../include/cthread.h"

csem_t impressora;
int alunosOk = 0;

void alunoLento(char *nome){
    printf("%s quer usar a impressora.\n",(char*)nome);
    cwait(&impressora);
    printf("%s imprimindo primeira folha.\n",(char*)nome);
    cyield();
    printf("%s imprimindo segunda folha.\n",(char*)nome);
    cyield();
    printf("%s terminou de imprimir.\n",(char*)nome);
    alunosOk++;
    csignal(&impressora);

}

void alunoRapido(char *nome){
    printf("%s quer usar a impressora\n",(char*)nome);
    cwait(&impressora);
    printf("%s imprimindo sua unica folha.\n",(char*)nome);
    cyield();
    printf("%s terminou de imprimir.\n",(char*)nome);
    csignal(&impressora);
    alunosOk++;
}

int main(int argc, char *argv[ ]) {
    printf("---Teste semaforo---\n\n");
    int tidRafael = ccreate((void *)alunoRapido, "Rafael");
    int tidRicardo = ccreate((void *)alunoRapido, "Ricardo");
    int tidLuis = ccreate((void *)alunoLento, "Luis");
    int tidLeonardo = ccreate((void *)alunoLento, "Leonardo");

     if (csem_init (&impressora, 2) == 0){
        printf("\ncsem_init Ok!\n");
     } else {
        printf ("\nErro em csem_init\n");
     }

     cjoin(tidRafael);
     cjoin(tidRicardo);
     cjoin(tidLuis);
     cjoin(tidLeonardo);
     if (alunosOk == 4){
        printf("\nTodos os alunos imprimiram.\ncwait e csignal OK!\n");
     } else{
        printf("\nErro em cwait e/ou csignal.\n");
     }

    return 0;
}
