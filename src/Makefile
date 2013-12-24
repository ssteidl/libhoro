#
# December 19, 2013
# The author disclaims copyright to this source code.
#

#For mingw we need to check if we have Windows_NT or not.
ifeq ($(OS), Windows_NT)
	EXE := .exe	
else
	EXE :=
endif

all: test cronprint test-amal cronprint-amal libhoro-amal.tgz

lemon$(EXE): lemon.c
	gcc -o lemon$(EXE) lemon.c

lex.horo.c: cron.l Parser.h
	flex --prefix=horo --nounistd cron.l

lex.horo.o: lex.horo.c
	gcc -g -O0 -c lex.horo.c

Parser.o: Parser.h horo.h Parser.c
	gcc -g -O0 -c Parser.c

cron.c: cron.y lemon Parser.h
	./lemon cron.y

cron.o: cron.c
	gcc -g -O0 -c -o cron.o cron.c

libhoro.o: cron.o horo.c
	gcc -g -O0 -c horo.c -o libhoro.o

test: test.cpp libhoro.o lex.horo.o Parser.o
	g++ -g -O0 -o test test.cpp libhoro.o cron.o lex.horo.o Parser.o

horo-amal.c: cron.c lex.horo.c lemon.c Parser.c
	tclsh86.exe mkamal.tcl

horo-amal.o: horo-amal.c
	gcc -g -O0 -c -o horo-amal.o horo-amal.c 

test-amal: horo-amal.o
	g++ -g -otest-amal test.cpp horo-amal.o

cronprint: cronprint.c libhoro.o lex.horo.o Parser.o
	gcc -g -O0 -o cronprint cronprint.c libhoro.o cron.o lex.horo.o Parser.o

cronprint-amal: horo-amal.o
	gcc -g -ocronprint-amal cronprint.c horo-amal.o

libhoro-amal.tgz: horo-amal.c horo.h
	tar -cf libhoro-amal.tgz horo-amal.c horo.h

clean: 
	rm -vf lemon$(EXE) lex.horo.c *.o *~ test$(EXE) cronprint$(EXE) horo-amal.c \
	cron.c cron.h cron.out test-amal$(EXE) cronprint-amal$(EXE) libhoro-amal.tgz
