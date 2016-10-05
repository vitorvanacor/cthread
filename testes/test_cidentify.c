#include <stdio.h>
#include <stdlib.h>
#include "../include/support.h"
#include "../include/cthread.h"

int main(int argc, char *argv[ ]){
    printf("\n---Teste cidentify---\n\n");
    char buffer[50];
    if (cidentify(buffer, sizeof(buffer)) == 0){
        printf("%s\n", buffer);
        printf("\ncidentify Ok.\n");
    } else{
        printf("\nErro em cidentify.\n");
    }

    return 0;
}
