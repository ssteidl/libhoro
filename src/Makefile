ifeq ($(OS), Windows_NT)
	EXE := .exe	
else
	EXE :=
endif

all: libdoorbell.o dbelltest cronprint

lemon$(EXE): lemon.c
	gcc -o lemon$(EXE) lemon.c

lex.yy.c: cron.l Parser.h
	flex cron.l

lex.yy.o: lex.yy.c
	gcc -g -O0 -c lex.yy.c

Parser.o: Parser.h doorbell.h Parser.c
	gcc -g -O0 -c Parser.c

cron.c: cron.y lemon Parser.h
	./lemon cron.y

cron.o: cron.c
	gcc -g -O0 -c -o cron.o cron.c

libdoorbell.o: cron.o doorbell.c
	gcc -g -O0 -c doorbell.c -o libdoorbell.o

dbelltest: test.c libdoorbell.o lex.yy.o Parser.o
	gcc -g -O0 -o dbelltest test.c libdoorbell.o cron.o lex.yy.o Parser.o

cronprint: cronprint.c libdoorbell.o lex.yy.o Parser.o
	gcc -g -O0 -o cronprint cronprint.c libdoorbell.o cron.o lex.yy.o Parser.o

clean: 
	rm -vf lemon$(EXE) lex.yy.c *.o *~ dbelltest$(EXE) cronprint$(EXE)
