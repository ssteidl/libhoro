%token_type {Token}
%default_type {Token}

%extra_argument {CronVals* cronVals}

%syntax_error {
    fprintf(stderr, "Error!!\n");
}

%name doorbellParser

%include {
#include <stdio.h>
#include <string.h>
#include "doorbell.h"

void assert(char assertion)
{}

}//end %include

//A cronfield is a grouping of cron elements
//A cron element is a list, range, weekday or special string

cronstring ::= YEARLY.
cronstring ::= ANNUALLY.
cronstring ::= MONTHLY.
cronstring ::= WEEKLY.
cronstring ::= DAILY. 

cronstring ::= cronfield(CF1) space cronfield(CF2). {

    cronVals->minute = CF1;
    cronVals->hour = CF2;
}

space ::= SPACE. 

%type cronfield {uint64_t}
cronfield ::= ASTERISK.
cronfield ::= list COMMA rangelist. 
cronfield ::= list COMMA range. 
cronfield ::= list COMMA step. 
cronfield ::= list. 
cronfield ::= asteriskstep. 
cronfield ::= step. 
cronfield ::= rangelist.

cronfield(CF) ::= range(R). {

    CF = 0;
    int i = R.start;
    for(; i <= R.stop; i += R.step)
    {
        CF |= (1 << i);
    }
}
       
cronfield(CF) ::= number(N). {

    fprintf(stderr, "Number: %d\n", N);
    //TODO: Validation on number
    CF = 0;
    CF = (1 << N);
}

%type rangelist {RangeList}
rangelist(RL) ::= rangelist(RL2) COMMA step(S). {

    RL2.ranges[RL2.numRanges++] = S;
    RL = RL2;  
}

rangelist(RL) ::= rangelist(RL2) COMMA range(R). {
        
    RL2.ranges[RL2.numRanges++] = R;
    RL = RL2;
}

rangelist(RL) ::= step(S1) COMMA step(S2). {

    RL.ranges[RL.numRanges++] = S1;
    RL.ranges[RL.numRanges++] = S2;
}

rangelist(RL) ::= step(S) COMMA range(R). {

    RL.ranges[RL.numRanges++] = S;
    RL.ranges[RL.numRanges++] = R;
} 

rangelist(RL) ::= range(R) COMMA step(S). {

    RL.ranges[RL.numRanges++] = R;
    RL.ranges[RL.numRanges++] = S;
}

rangelist(RL) ::= range(R1) COMMA range(R2). {

    RL.ranges[RL.numRanges++] = R1;
    RL.ranges[RL.numRanges++] = R2;
}

%type asteriskstep {Range}
asteriskstep(S) ::= ASTERISK STEP number(N). {

    /*TODO: This needs to be based on context*/
    S.start = 1;
    S.stop = 31;
    S.step = N;
}

%type step {Range}
step(S) ::= range(R) STEP number(N). {

    S = R;
    S.step = N;
}

%type list {List}
list(L) ::= list(L2) COMMA number(N). {

    L2.listNums[L2.numCount++] = N;
    L = L2;
}

list(L) ::= number(N1) COMMA number(N2). {
        
    L.listNums[L.numCount++] = N1;
    L.listNums[L.numCount++] = N2;    
}

%type range {Range}
range(R) ::= number(START) DASH number(STOP). {
        
    R.start = START;
    R.stop = STOP;
    R.step = 1;
}

%type number {int}
number(NUM) ::= numString(NUM_STRING). {
        
    if(NUM_STRING.length > 3)
    {
        fprintf(stderr, "Number to large");
        return;
    }
    char numStr[NUM_STRING.length + 1];
    numStr[NUM_STRING.length] = '\0';
        
    memcpy(numStr, NUM_STRING.string, NUM_STRING.length);
        
    NUM = atoi(numStr);
}

numString(A) ::= NUMBER(B). {
    A = B;
}
