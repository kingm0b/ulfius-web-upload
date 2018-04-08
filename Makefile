#
# Webservice IoT S3
#

CC=gcc
CFLAGS=-c -Wall -D_REENTRANT $(ADDITIONALFLAGS)
LIBS=-lc -lulfius -lyder -lorcania -ljansson

all: webservice_iots3

clean:
	@echo Removendo objetos da compilação anterior ...
	rm -f *.o webservice_iots3

webservice_iots3.o: webservice_iots3.c
	@echo Gerando o código objeto ...
	$(CC) $(CFLAGS) webservice_iots3.c -O0 -o webservice_iots3.o

webservice_iots3: webservice_iots3.o
	@echo Compilando o "Webservice IoT S3" ...
	$(CC) -o webservice_iots3 webservice_iots3.o $(LIBS)
