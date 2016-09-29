#
# Makefile ESQUELETO
#
# OBRIGATÓRIO ter uma regra "all" para geração da biblioteca e de uma
# regra "clean" para remover todos os objetos gerados.
#
# NECESSARIO adaptar este esqueleto de makefile para suas necessidades.
#  1. Cuidado com a regra "clean" para não apagar o "fila2.o"
#
# OBSERVAR que as variáveis de ambiente consideram que o Makefile está no diretótio "cthread"
# 

CC=gcc

LIB_DIR=./lib/
INC_DIR=./include/
BIN_DIR=./bin/
SRC_DIR=./src/
TST_DIR=./testes/
CFLAGS=-g -Wall -I$(INC_DIR)
LDFLAGS=-L$(LIB_DIR) -lcthread

all: $(BIN_DIR)cthread.o
	ar crs $(LIB_DIR)libcthread.a $(BIN_DIR)support.o $(BIN_DIR)cthread.o

$(BIN_DIR)cthread.o: $(SRC_DIR)cthread.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)cthread.o -c $(SRC_DIR)cthread.c

teste: $(TST_DIR)teste.c
	$(CC) $(CFLAGS) -o $(TST_DIR)teste $(TST_DIR)teste.c $(LDFLAGS)

clean:
	find $(BIN_DIR) $(LIB_DIR) $(TST_DIR) -type f ! -name 'support.o' ! -name "*.c" -delete


