#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <support.h>
#include <cdata.h>
#include <cthread.h>

void barber() {
    int n;
    printf("\n----Entrou no barber\n");
    for(n=0;n<3;n++) {
        printf("\n--Barber ganhou a cpu, n=%d\n",n);
        cyield();
    }
    printf("Fim da barber\n");
}

int main() {
    int tidBarber;
    printf("Inicio do programaAAAAAAAa\n");
    tidBarber = ccreate ((void *)barber, (void *) NULL);
    cjoin(tidBarber);
    printf("\n--Main ganhou a cpu");
    printf("\nFim do programa\n");
    return 0;
}
