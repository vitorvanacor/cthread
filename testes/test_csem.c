#include <stdio.h>
#include <stdlib.h>

#include <support.h>
#include <cthread.h>

csem_t impressora;
int alunosOk = 0;

void* alunoLento(void *nome){
    int i;
    for(i=0;i<2;i++){
        printf("%s quer usar a impressora.\n",(char*)nome);
        cwait(&impressora);
        printf("%s imprimindo.\n",(char*)nome);
        cyield();
        printf("%s imprimindo.\n",(char*)nome);
        cyield();
        printf("%s imprimindo.\n",(char*)nome);
        cyield();
        printf("%s terminou de imprimir. Imprimiu %d.\n",(char*)nome, i+1);
        csignal(&impressora);
    }
    printf("%s OK.\n",(char*)nome);
    alunosOk++;

}

void* alunoRapido(void *nome){
    int i;
    for(i=0;i<3;i++){
        printf("%s quer usar a impressora\n",(char*)nome);
        cwait(&impressora);
        printf("%s imprimindo.\n",(char*)nome);
        cyield();
        printf("%s terminou de imprimir. Imprimiu %d\n",(char*)nome, i+1);
        csignal(&impressora);
    }
    printf("%s OK.\n",(char*)nome);
    alunosOk++;

}

int main(int argc, char *argv[ ]) {
    printf("---Teste semaforo---\n\n");
    ccreate((void *)alunoRapido, (void *)"Jorge");
    ccreate((void *)alunoRapido, (void *)"Joao");
    ccreate((void *)alunoLento, (void *)"Ronaldo");

     if (csem_init (&impressora, 1) == 0){
        printf("\ncsem_init Ok\n");
     } else {
        printf ("\nErro em csem_init\n");
     }
     while(alunosOk < 3){
        cyield();
     }

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
