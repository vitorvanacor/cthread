#
# Makefile para programas teste
#

CC=gcc
LIB_DIR=../lib

all: test_ccreate_cjoin test_cyield test_csem

test_ccreate_cjoin: test_ccreate_cjoin.c $(LIB_DIR)/libcthread.a
	$(CC) -o test_ccreate_cjoin test_ccreate_cjoin.c -L$(LIB_DIR) -lcthread -Wall

test_cyield: test_cyield.c $(LIB_DIR)/libcthread.a
	$(CC) -o test_cyield test_cyield.c -L$(LIB_DIR) -lcthread -Wall

test_csem: test_csem.c $(LIB_DIR)/libcthread.a
	$(CC) -o test_csem test_csem.c -L$(LIB_DIR) -lcthread -Wall

clean:
	rm -rf test_ccreate_cjoin test_cyield test_csem *.o *~




