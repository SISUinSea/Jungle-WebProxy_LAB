CC = gcc
CFLAGS = -g -Wall -I ..
LDFLAGS = -lpthread

all: echoclient echoserveri

csapp.o: ../csapp.c ../csapp.h
	$(CC) $(CFLAGS) -c ../csapp.c -o csapp.o

echo.o: echo.c ../csapp.h
	$(CC) $(CFLAGS) -c echo.c

echoclient.o: echoclient.c ../csapp.h
	$(CC) $(CFLAGS) -c echoclient.c

echoserveri.o: echoserveri.c ../csapp.h
	$(CC) $(CFLAGS) -c echoserveri.c

echoclient: echoclient.o csapp.o
	$(CC) $(CFLAGS) echoclient.o csapp.o -o echoclient $(LDFLAGS)

echoserveri: echoserveri.o echo.o csapp.o
	$(CC) $(CFLAGS) echoserveri.o echo.o csapp.o -o echoserveri $(LDFLAGS)

clean:
	rm -f *~ *.o echoclient echoserveri
