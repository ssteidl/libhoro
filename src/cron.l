/**
 * December 19, 2013
 * The author disclaims copyright to this source code.
 */

%option noyywrap

%{
#include "cron.h"
#include "horo.h"
#include "Parser.h"

    void *horoParserAlloc(void* (*mallocProc)(size_t));
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

HORO_ERROR 
processCronString(char const* string, CronVals* oCronVals)
{
    Token theToken;
    void* parser = horoParserAlloc(malloc);
    HORO_ERROR ret = HORO_SUCCESS;
    int token = 0;
    YY_BUFFER_STATE buffer;

    if(parser == NULL)
    {
        return HORO_ERROR_ILLEGAL_ARG;
    }

//    horoParserTrace(stderr, "horo");
    
    memset(oCronVals, 0, sizeof(CronVals));
    buffer = yy_scan_string(string);
    while(1)
    {
        memset(&theToken, 0, sizeof(Token));
        
        token = yylex();
        if(!token)
        {
            //If there are no more tokens, we need to call the parser one last time.
            horoParser(parser, 0, theToken, oCronVals);
            if(oCronVals->error != HORO_SUCCESS)
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
            ret = HORO_ERROR_PARSER_ILLEGAL_FIELD;
            break;
        }
        horoParser(parser, token, theToken, oCronVals);
        if(oCronVals->error != HORO_SUCCESS)
        {
            ret = oCronVals->error;
            break;
        }
    }

    horoParserFree(parser, free);
    yy_delete_buffer(buffer);
    return ret;
}


// Local Variables:
// mode: c
// End:
