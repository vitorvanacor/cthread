#include <stdio.h>
#include <stdlib.h>

#include "../include/support.h"
#include "../include/cthread.h"

csem_t impressora;

void alunoLento(char *nome){
    printf("%s quer usar a impressora.\n",(char*)nome);
    cwait(&impressora);
    printf("%s imprimindo.\n",(char*)nome);
    cyield();
    printf("%s imprimindo.\n",(char*)nome);
    cyield();
    printf("%s terminou de imprimir.\n",(char*)nome);
    csignal(&impressora);
}

void alunoRapido(char *nome){
    printf("%s quer usar a impressora\n",(char*)nome);
    cwait(&impressora);
    printf("%s imprimindo.\n",(char*)nome);
    cyield();
    printf("%s terminou de imprimir.\n",(char*)nome);
    csignal(&impressora);
}

int main(int argc, char *argv[ ]) {
    printf("---Teste semaforo---\n\n");
    int tidRafael = ccreate((void *)alunoRapido, "Rafael");
    int tidRicardo = ccreate((void *)alunoRapido, "Ricardo");
    int tidLuis = ccreate((void *)alunoLento, "Luis");
    int tidLeonardo = ccreate((void *)alunoLento, "Leonardo");

     if (csem_init (&impressora, 2) == 0){
        printf("\ncsem_init Ok\n");
     } else {
        printf ("\nErro em csem_init\n");
     }

     cjoin(tidRafael);
     cjoin(tidRicardo);
     cjoin(tidLuis);
     cjoin(tidLeonardo);

    printf("---Teste cidentify---\n\n");
    char buffer[50];
    if (cidentify(buffer, sizeof(buffer)) == 0){
        printf("%s\n", buffer);
        printf("\ncidentify Ok.\n");
    } else{
        printf("\nErro em cidentify.\n");
    }

    return 0;
}
