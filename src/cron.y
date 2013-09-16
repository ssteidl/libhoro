%token_type {Token}
%default_type {Token}

%extra_argument {CronVals* cronVals}

%syntax_error {
    fprintf(stderr, "Error!!\n");
}

%name doorbellParser

%include {

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "Parser.h"

}//end %include

cronstring ::= YEARLY. {

    cronVals->minute = 1 << 0;
    cronVals->hour = 1 << 0;
    cronVals->dayOfMonth = 1 << 1;
    cronVals->month = 1 << 1;
    cronVals->dayOfWeek = DBELL_ASTERISK;
}

cronstring ::= MONTHLY. {

    cronVals->minute = 1 << 0;
    cronVals->hour = 1 << 0;
    cronVals->dayOfMonth = 1 << 1;
    cronVals->month = DBELL_ASTERISK;
    cronVals->dayOfWeek = DBELL_ASTERISK;
}

cronstring ::= WEEKLY. {

    cronVals->minute = 1 << 0;
    cronVals->hour = 1 << 0;
    cronVals->dayOfMonth = DBELL_ASTERISK;
    cronVals->month = DBELL_ASTERISK;
    cronVals->dayOfWeek = 1 << 0;
}

cronstring ::= DAILY. {

    cronVals->minute = 1 << 0;
    cronVals->hour = 1 << 0;
    cronVals->dayOfMonth = DBELL_ASTERISK;
    cronVals->month = DBELL_ASTERISK;
    cronVals->dayOfWeek = DBELL_ASTERISK;
}

cronstring ::= HOURLY. {

    cronVals->minute = 1 << 0;
    cronVals->hour = DBELL_ASTERISK;
    cronVals->dayOfMonth = DBELL_ASTERISK;
    cronVals->month = DBELL_ASTERISK;
    cronVals->dayOfWeek = DBELL_ASTERISK;
}

cronstring ::= cronfield(CF1) SPACE cronfield(CF2) SPACE cronfield(CF3) 
               cronfield(CF4) SPACE cronfield(CF5). {

    cronVals->minute = CF1.val;
    cronVals->hour = CF2.val;
    cronVals->dayOfMonth = CF3.val;
    cronVals->month = CF4.val;
    cronVals->dayOfWeek = CF5.val;
}

%type cronfield {CronField}

cronfield(CF) ::= ASTERISK. {

    memset(&CF, 0, sizeof(CF));
    CF.isAsterisk=1;
}

cronfield(CF) ::= list(L) COMMA rangelist(RL). {

    memset(&CF, 0, sizeof(CF));
    cronFieldFromList(&L, &CF);
    cronFieldFromRangeList(&RL, &CF); 
} 

cronfield(CF) ::= list(L) COMMA range(R). {

    memset(&CF, 0, sizeof(CF));
    cronFieldFromList(&L, &CF);
    cronFieldFromRange(&R, &CF); 
}

cronfield(CF) ::= list(L) COMMA step(S). {

    memset(&CF, 0, sizeof(CF));
    cronFieldFromList(&L, &CF);
    cronFieldFromRange(&S, &CF); 
}

 
cronfield(CF) ::= list(L). {

    memset(&CF, 0, sizeof(CF));
    cronFieldFromList(&L, &CF);
}

cronfield(CF) ::= asteriskstep(AS). {

    memset(&CF, 0, sizeof(CF));
    CF.isAsterisk = 1;
    CF.asteriskStep = AS.step;
} 

cronfield(CF) ::= step(S). {

    memset(&CF, 0, sizeof(CF));
    cronFieldFromRange(&S, &CF);
}
cronfield(CF) ::= rangelist(RL). {

    memset(&CF, 0, sizeof(CF));
    cronFieldFromRangeList(&RL, &CF);
}

cronfield(CF) ::= range(R). {

    memset(&CF, 0, sizeof(CF));
    cronFieldFromRange(&R, &CF);
}
       
cronfield(CF) ::= number(N). {

    memset(&CF, 0, sizeof(CF));
    fprintf(stderr, "Number: %d\n", N);
    //TODO: Validation on number
 
    CF.val = (1 << N);
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

    S.start = -1;
    S.stop = INT_MAX;
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
