all: libdoorbell.o dbelltest

lemon: lemon.c
	gcc -o lemon.exe lemon.c

lex.yy.c: cron.l Parser.h
	flex cron.l

lex.yy.o: lex.yy.c
	gcc -g -O0 -c lex.yy.c

Parser.o: Parser.h doorbell.h Parser.c
	gcc -g -O0 -c Parser.c

cron.c: cron.y lemon Parser.h
	lemon cron.y

cron.o: cron.c
	gcc -g -O0 -c -o cron.o cron.c

libdoorbell.o: cron.o doorbell.c
	gcc -g -O0 -c doorbell.c -o libdoorbell.o

dbelltest: test.c libdoorbell.o lex.yy.o Parser.o
	gcc -g -O0 -o dbelltest test.c libdoorbell.o cron.o lex.yy.o Parser.o
clean: 
	rm -vf lemon.exe lex.yy.c lex.yy.o *~ cron.o cron.c cron.h cron.out \
	libdoorbell.o dbelltest
