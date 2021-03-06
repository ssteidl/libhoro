/**
 * December 19, 2013
 * The author disclaims copyright to this source code.
 */

%token_type {Token}
%default_type {Token}

%extra_argument {CronVals* cronVals}

%syntax_error {
    fprintf(stderr, "Error!!\n");
}

%name horoParser

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
    cronVals->dayOfWeek = HORO_ASTERISK;
}

cronstring ::= MONTHLY. {

    cronVals->minute = 1 << 0;
    cronVals->hour = 1 << 0;
    cronVals->dayOfMonth = 1 << 1;
    cronVals->month = HORO_ASTERISK;
    cronVals->dayOfWeek = HORO_ASTERISK;
}

cronstring ::= WEEKLY. {

    cronVals->minute = 1 << 0;
    cronVals->hour = 1 << 0;
    cronVals->dayOfMonth = HORO_ASTERISK;
    cronVals->month = HORO_ASTERISK;
    cronVals->dayOfWeek = 1 << 0;
}

cronstring ::= DAILY. {

    cronVals->minute = 1 << 0;
    cronVals->hour = 1 << 0;
    cronVals->dayOfMonth = HORO_ASTERISK;
    cronVals->month = HORO_ASTERISK;
    cronVals->dayOfWeek = HORO_ASTERISK;
}

cronstring ::= HOURLY. {

    cronVals->minute = 1 << 0;
    cronVals->hour = HORO_ASTERISK;
    cronVals->dayOfMonth = HORO_ASTERISK;
    cronVals->month = HORO_ASTERISK;
    cronVals->dayOfWeek = HORO_ASTERISK;
}

cronstring ::= cronfield(CF1) SPACE cronfield(CF2) SPACE cronfield(CF3) 
               SPACE cronfield(CF4) SPACE cronfield(CF5). {

        if((cronVals->error = setCronFieldValues(&CF1, HORO_POSITION_MINUTE)) ||
           (cronVals->error = setCronFieldValues(&CF2, HORO_POSITION_HOUR)) ||
           (cronVals->error = setCronFieldValues(&CF3, HORO_POSITION_DOM)) ||
           (cronVals->error = setCronFieldValues(&CF4, HORO_POSITION_MONTH)) ||
           (cronVals->error = setCronFieldValues(&CF5, HORO_POSITION_DOW)))
        {
            /*do nothing*/
        }
        else
        {

            cronVals->minute = CF1.val;
            cronVals->hour = CF2.val;
            cronVals->dayOfMonth = CF3.val;
            cronVals->month = CF4.val;
            cronVals->dayOfWeek = CF5.val;
            cronVals->error = validateCronVals(cronVals);
        }
}

%type cronfield {CronField}

cronfield(CF) ::= ASTERISK. {

    memset(&CF, 0, sizeof(CF));
    CF.type = HORO_FIELD_TYPE_ASTERISK;
    CF.typeVal.asteriskStep = 1;
}

cronfield(CF) ::= list(L) COMMA rangelist(RL). {

    memset(&CF, 0, sizeof(CF));
    CF.type = HORO_FIELD_TYPE_LIST | HORO_FIELD_TYPE_RANGELIST;
    memcpy(&CF.typeVal.list, &L, sizeof(L));
    memcpy(&CF.typeVal.range, &RL, sizeof(RL));
    //    cronFieldFromList(&L, &CF);
    //    cronFieldFromRangeList(&RL, &CF); 
} 

cronfield(CF) ::= list(L) COMMA range(R). {

    memset(&CF, 0, sizeof(CF));
    CF.type = HORO_FIELD_TYPE_LIST | HORO_FIELD_TYPE_RANGE;
    memcpy(&CF.typeVal.list, &L, sizeof(L));
    memcpy(&CF.typeVal.range, &R, sizeof(R));
    //    cronFieldFromList(&L, &CF);
    //    cronFieldFromRange(&R, &CF); 
}

cronfield(CF) ::= list(L) COMMA step(S). {

    memset(&CF, 0, sizeof(CF));
    CF.type = HORO_FIELD_TYPE_LIST | HORO_FIELD_TYPE_RANGE;
    memcpy(&CF.typeVal.list, &L, sizeof(L));
    memcpy(&CF.typeVal.range, &S, sizeof(S));
    //    cronFieldFromList(&L, &CF);
    //    cronFieldFromRange(&S, &CF); 
}

 
cronfield(CF) ::= list(L). {

    memset(&CF, 0, sizeof(CF));
    CF.type = HORO_FIELD_TYPE_LIST;
    memcpy(&CF.typeVal.list, &L, sizeof(L));
    //    cronFieldFromList(&L, &CF);
}

cronfield(CF) ::= asteriskstep(AS). {

    memset(&CF, 0, sizeof(CF));
    CF.type = HORO_FIELD_TYPE_ASTERISK;
    CF.typeVal.asteriskStep = AS.step;
} 

cronfield(CF) ::= step(S). {

    memset(&CF, 0, sizeof(CF));
    CF.type = HORO_FIELD_TYPE_RANGE;
    memcpy(&CF.typeVal.range, &S, sizeof(S));
    //    cronFieldFromRange(&S, &CF);
}
cronfield(CF) ::= rangelist(RL). {

    memset(&CF, 0, sizeof(CF));
    CF.type = HORO_FIELD_TYPE_RANGELIST;
    memcpy(&CF.typeVal.rangeList, &RL, sizeof(RL));
    //    cronFieldFromRangeList(&RL, &CF);
}

cronfield(CF) ::= range(R). {

    memset(&CF, 0, sizeof(CF));
    CF.type = HORO_FIELD_TYPE_RANGE;
    memcpy(&CF.typeVal.range, &R, sizeof(R));
    //    cronFieldFromRange(&R, &CF);
}
       
cronfield(CF) ::= number(N). {

    memset(&CF, 0, sizeof(CF));
    CF.type = HORO_FIELD_TYPE_VALUE;
    CF.typeVal.value = N;
    //    CF.val = (1 << N);
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

    S.start = 0;
    S.stop = 64; //TODO: Magic number.
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

    //Max length for a number string is 2 digits plus the null terminator        
    char numStr[3];

    if(NUM_STRING.length <= (sizeof(numStr) - 1))
    {
        NUM = -1;
        numStr[NUM_STRING.length] = '\0';
        
        memcpy(numStr, NUM_STRING.string, NUM_STRING.length);
    
        //TODO: I should use strtol so I can check the return code.
        NUM = atoi(numStr);      
    }
    else
    {
        cronVals->error = HORO_ERROR_OUT_OF_RANGE;
    }
}

numString(A) ::= NUMBER(B). {

    A = B;
}
