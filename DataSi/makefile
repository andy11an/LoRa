CC =gcc
CFLAGS=-g -Wall
.PHONY: clean
objs= application.o data.o ADR.o
app:	$(objs) 
	$(CC) -o app ${objs} -lm

data.o:  ./src/data.c  
	$(CC) -c ./src/data.c  -I ./str/data.h  -lm
	
ADR.o:	./src/ADR.c 
	$(CC) -c ./src/ADR.c  -I ./str/ADR.h  -lm


application.o: application.c data.o ADR.o
	$(CC) -c application.c    data.o   ADR.o -lm 
	 
clean:
	rm *.o app GW*

