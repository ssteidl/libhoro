%option noyywrap

%{
#include "cron.h"
#include "Parser.h"

    void *doorbellParserAlloc(void* (*mallocProc)(size_t));
%}

%%
[ \t\n\r] { return SPACE; }
\/ {return STEP; }
\* {return ASTERISK; }
[[:digit:]]* {return NUMBER;}
"-" {return DASH; }
"," {return COMMA; }
"@yearly" {return YEARLY; }
"@annually" {return ANNUALLY; }
"@monthly" {return MONTHLY; }
"@weekly" {return WEEKLY; }
"@daily" {return DAILY; }
%%

void processCronString(char const* string, CronVals* oCronVals)
{
    void* dbellParser = doorbellParserAlloc(malloc);
    if(dbellParser == NULL)
    {
        fprintf(stderr, "NULL Parser");
    }

    //doorbellParserTrace(stderr, "dbell");
    Token theToken;
    memset(oCronVals, 0, sizeof(CronVals));
    yy_scan_string(string);
    while(1)
    {
        memset(&theToken, 0, sizeof(Token));
        
        int token = yylex();
        if(!token)
        {
            doorbellParser(dbellParser, 0, theToken, oCronVals);
            break;
        }
                
        if(yyleng < sizeof(theToken.string))
        {
            memcpy(theToken.string, yytext, yyleng);
            theToken.string[yyleng] = '\0';
            theToken.length = yyleng;
        }
        else
        {
            //TODO: Error handling
            break;
        }
        doorbellParser(dbellParser, token, theToken, oCronVals);
    }

    doorbellParserFree(dbellParser, free);
}

