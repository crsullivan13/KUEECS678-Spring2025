%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "deque.h"
#include "memory_pool.h"
#include "parse.tab.h"
#include "parsing_interface.h"
%}

%option       noyywrap nounput noinput yylineno
whitesp       [ \t\r]+
comment       #.*
 /*string        ([a-zA-Z0-9\+\-\!@%\^\"\*.\{\}\[\]\(\)?\.,_~`/:;$]|\\(.|\n)|'(\\(.|\n)|[^\\'])*')+
 sim_str       [a-zA-Z0-9\+\-\!@%\^\"\*.\{\}\[\]\(\)?\.,_~`/:;]+*/
string        ([^ \t\r\n\'\#\<\>\=&\|\\]|\\(.|\n)|'(\\(.|\n)|[^\\'])*')+
sim_str        [^ \t\r\n\'\#\<\>\=&\|\\\$]+
id            [a-zA-Z_][a-zA-Z0-9_]*
number        [0-9]+



%%

"|"           { return PIPE;        }
"&"           { return BCKGRND;     }
"="           { return EQUALS;      }
"<"           { return REDIRIN;     }
">"           { return REDIROUT;    }
">>"          { return REDIROUTAPP; }
"echo"        { return ECHO_TOK;    }
"export"      { return EXPORT_TOK;  }
"cd"          { return CD_TOK;      }
"pwd"         { return PWD_TOK;     }
"jobs"        { return JOBS_TOK;    }
"kill"        { return KILL_TOK;    }
"\n"          { return EOC_TOK;     }
<<EOF>>       { return END;         }
"exit"|"quit" { yylval.str = memory_pool_strdup(yytext); return EXIT_TOK; }

{number}      { yylval.str = memory_pool_strdup(yytext); return NUM;     }
{id}          { yylval.str = memory_pool_strdup(yytext); return ID;      }
{sim_str}     { yylval.str = memory_pool_strdup(yytext); return SIM_STR; }
{string}      { yylval.str = memory_pool_strdup(yytext); return STR;     }
{comment}     { /* No action and no token */ }
{whitesp}     { /* No action and no token */ }

. { fprintf(stderr, "LEX: Unexpected symbol: %c (Line: %d)\n", *yytext, yylineno); }

%%

void destroy_lex() {
  if (yy_init)
    yylex_destroy();
}
