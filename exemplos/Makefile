#
# Makefile para programas teste
#

CC=gcc
LIB_DIR=../lib

all: test_ccreate_cjoin test_cyield test_csem

test_ccreate_cjoin: exemplo.c $(LIB_DIR)/libcthread.a
	$(CC) -o test_ccreate_cjoin test_ccreate_cjoin.c -L$(LIB_DIR) -lcthread -Wall

test_cyield: exemplo.c $(LIB_DIR)/libcthread.a
	$(CC) -o test_cyield test_cyield.c -L$(LIB_DIR) -lcthread -Wall

test_csem: exemplo.c $(LIB_DIR)/libcthread.a
	$(CC) -o test_csem test_csem.c -L$(LIB_DIR) -lcthread -Wall

clean:
	rm -rf exemplo mandel teste_vetor barbeiro filosofos prodcons series *.o *~




