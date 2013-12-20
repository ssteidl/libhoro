#For mingw we need to check if we have Windows_NT or not.
ifeq ($(OS), Windows_NT)
	EXE := .exe	
else
	EXE :=
endif

all: test cronprint test-amal cronprint-amal libdoorbell-amal.tgz

lemon$(EXE): lemon.c
	gcc -o lemon$(EXE) lemon.c

lex.horo.c: cron.l Parser.h
	flex --prefix=horo --nounistd cron.l

lex.horo.o: lex.horo.c
	gcc -g -O0 -c lex.horo.c

Parser.o: Parser.h doorbell.h Parser.c
	gcc -g -O0 -c Parser.c

cron.c: cron.y lemon Parser.h
	./lemon cron.y

cron.o: cron.c
	gcc -g -O0 -c -o cron.o cron.c

libdoorbell.o: cron.o doorbell.c
	gcc -g -O0 -c doorbell.c -o libdoorbell.o

test: test.cpp libdoorbell.o lex.horo.o Parser.o
	g++ -g -O0 -o test test.cpp libdoorbell.o cron.o lex.horo.o Parser.o

dbell-amal.c: cron.c lex.horo.c lemon.c Parser.c
	tclsh86.exe mkamal.tcl

dbell-amal.o: dbell-amal.c
	gcc -g -O0 -c -o dbell-amal.o dbell-amal.c 

test-amal: dbell-amal.o
	g++ -g -otest-amal test.cpp dbell-amal.o

cronprint: cronprint.c libdoorbell.o lex.horo.o Parser.o
	gcc -g -O0 -o cronprint cronprint.c libdoorbell.o cron.o lex.horo.o Parser.o

cronprint-amal: dbell-amal.o
	gcc -g -ocronprint-amal cronprint.c dbell-amal.o

libdoorbell-amal.tgz: dbell-amal.c doorbell.h
	tar -cf libdoorbell-amal.tgz dbell-amal.c doorbell.h

clean: 
	rm -vf lemon$(EXE) lex.horo.c *.o *~ test$(EXE) cronprint$(EXE) dbell-amal.c \
	cron.c cron.h cron.out test-amal$(EXE) cronprint-amal$(EXE) libdoorbell-amal.tgz
