%option noyywrap

%{
#include "cron.h"
#include "doorbell.h"
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
"@monthly" {return MONTHLY; }
"@weekly" {return WEEKLY; }
"@daily" {return DAILY; }
"@hourly" {return HOURLY; }
%%

DBELL_ERROR 
processCronString(char const* string, CronVals* oCronVals)
{
    Token theToken;
    void* dbellParser = doorbellParserAlloc(malloc);
    DBELL_ERROR ret = DBELL_SUCCESS;
    int token = 0;

    if(dbellParser == NULL)
    {
        return DBELL_ERROR_ILLEGAL_ARG;
    }

//    doorbellParserTrace(stderr, "dbell");
    
    memset(oCronVals, 0, sizeof(CronVals));
    YY_BUFFER_STATE buffer = yy_scan_string(string);
    while(1)
    {
        memset(&theToken, 0, sizeof(Token));
        
        token = yylex();
        if(!token)
        {
            //If there are no more tokens, we need to call the parser one last time.
            doorbellParser(dbellParser, 0, theToken, oCronVals);
            if(oCronVals->error != DBELL_SUCCESS)
            {
                ret = oCronVals->error;
            }
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
            ret = DBELL_ERROR_PARSER_ILLEGAL_FIELD;
            break;
        }
        doorbellParser(dbellParser, token, theToken, oCronVals);
        if(oCronVals->error != DBELL_SUCCESS)
        {
            ret = oCronVals->error;
            break;
        }
    }

    doorbellParserFree(dbellParser, free);
    yy_delete_buffer(buffer);
    return ret;
}


// Local Variables:
// mode: c
// End:
