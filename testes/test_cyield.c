#include "../include/support.h"
#include "../include/cthread.h"
#include <stdio.h>

void function1() {
    printf("Function 1 yielding\n");
    cyield();
    printf("Function 1 yielding\n");
    cyield();
    printf("Function 1 yielding\n");
    cyield();
}

void function2() {
    printf("Function 2 yielding\n");
    cyield();
    printf("Function 2 yielding\n");
    cyield();
    printf("Function 2 yielding\n");
    cyield();
}

void function3() {
    printf("Function 3 yielding\n");
    cyield();
    printf("Function 3 yielding\n");
    cyield();
    printf("Function 3 yielding\n");
    cyield();
}

int main(int argc, char **argv) {
    int tid1, tid2, tid3;

    printf("Main printing something\n");

    tid1 = ccreate ((void *)function1, NULL);
    if (tid1 < 0 )
       printf("Error creating thread for function 1...\n");

    tid2 = ccreate ((void *)function2, NULL);
    if (tid2 < 0 )
       printf("Error creating thread for function 2...\n");

    tid3 = ccreate ((void *)function3, NULL);
    if (tid3 < 0 )
       printf("Error creating thread for function 3...\n");

    printf("Main joining function 1\n");
    cjoin(1);

    printf("Main joining function 2\n");
    if (cjoin(2) != 0) {
        printf("Function 2 is already finished\n");
    }

    printf("Main joining function 3\n");
    if (cjoin(3) != 0) {
        printf("Function 3 is already finished\n");
    }

    printf(">>Main ending\n");

    return 0;
}
