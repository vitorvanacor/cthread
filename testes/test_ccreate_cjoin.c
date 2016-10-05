#include "../include/support.h"
#include "../include/cthread.h"
#include <stdio.h>

void function2() {
    printf("Function 2 printing something\n");
    printf("Function2 yielding\n");
    cyield();

    printf("Function 2 receiving cpu again because main and function 1 are blocked\n");

    printf(">>Function 2 ending\n");
}

void function1() {
    int tid2;

    printf("Function 1 printing something\n");

    tid2 = ccreate (function2, NULL);
    if (tid2 < 0 )
       printf("Error creating thread for function 2...\n");
    else printf("Function 1 created thread for function 2\n");

    printf("Function 1 joining function 2\n");
    cjoin(2);

    printf(">>Function1  ending\n");
}

int main(int argc, char **argv) {
    int tid1;

    printf("Main printing something\n");

    tid1 = ccreate (function1, NULL);
    if (tid1 < 0 )
       printf("Error creating thread for function 1...\n");
    else printf("Main created thread for function 1\n");

    printf("Main joining function 1\n");
    cjoin(1);

    printf(">>Main ending\n");

    return 0;
}
