%{
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "command.h"
#include "parsing_interface.h"
#include "parse.tab.h"
#include "memory_pool.h"

extern int yylineno;
extern char* yytext;
extern FILE* yyin;

extern void yyerror(CommandHolder**, char*);
extern int yyparse(CommandHolder**);
extern int yylex();

int yyerrstatus = 0;
%}

%code requires {
#include <stdbool.h>

#include "command.h"
#include "parsing_interface.h"
#include "parse.tab.h"
#include "memory_pool.h"
}

%union {
  int integer;
  char* str;
  Command cmd;
  CommandHolder holder;
  CommandHolder* holder_arr;
  CmdStrs cmd_strs;
  Cmds cmd_list;
  Redirect redirect;
}

%parse-param { CommandHolder** __ret_cmds }

/* Terminals */
%token PIPE BCKGRND SQUOTE EQUALS REDIRIN REDIROUT REDIROUTAPP END
%token ECHO_TOK EXPORT_TOK CD_TOK PWD_TOK JOBS_TOK KILL_TOK EOC_TOK
%token <str> STR SIM_STR ID NUM EXIT_TOK

/* Non-terminals */
%type <str> string first_string special_string
%type <integer> cmd_bg redir_mark
%type <redirect> redir redir_inner
%type <holder> cmd_top
%type <cmd> cmd_content
%type <cmd_strs> cmd cmd_arguments
%type <cmd_list> cmds
%type <cmd_arr> top

/* Start symbol */
%start top

%%

top: EOC_TOK {
  *__ret_cmds = NULL;

  YYACCEPT;
}
|       cmds EOC_TOK {
  push_back_Cmds(&$1, mk_command_holder(NULL, NULL, 0, mk_eoc()));

  *__ret_cmds = as_array_Cmds(&$1, NULL);

  YYACCEPT;
}
|       cmds END {
  push_back_Cmds(&$1, mk_command_holder(NULL, NULL, 0, mk_eoc()));

  *__ret_cmds = as_array_Cmds(&$1, NULL);

  end_main_loop(EXIT_SUCCESS);

  YYACCEPT;
}
|       error EOC_TOK {
  *__ret_cmds = NULL;

  YYABORT;
}
|       error END {
  *__ret_cmds = NULL;

  end_main_loop(EXIT_FAILURE);

  YYABORT;
}



cmds:   cmd_top {
  Cmds cs = new_Cmds(1);

  push_front_Cmds(&cs, $1);

  $$ = cs;
}
|       cmd_top PIPE cmds {
  CommandHolder prev = pop_front_Cmds(&$3);

  $1.flags = ($1.flags & ~(REDIRECT_APPEND | REDIRECT_OUT)) | PIPE_OUT;
  prev.flags = (prev.flags & ~REDIRECT_IN) | PIPE_IN;

  if (prev.flags & BACKGROUND)
    $1.flags |= BACKGROUND;

  push_front_Cmds(&$3, prev);
  push_front_Cmds(&$3, $1);

  $$ = $3;
}



cmd_top: cmd_content redir cmd_bg {
  char flags = (($2.append)? REDIRECT_APPEND : 0) |
    (($2.out)? REDIRECT_OUT : 0) |
    (($2.in)? REDIRECT_IN : 0) |
    ($3? BACKGROUND : 0);

  $$ = mk_command_holder($2.in, $2.out, flags, $1);
}



cmd_content: cmd {
  $$ = mk_generic_command(as_array_CmdStrs(&$1, NULL));
}
|       ECHO_TOK {
  char** cmd = memory_pool_alloc(sizeof(char*));
  *cmd = NULL;
  $$ = mk_echo_command(cmd);
}
|       ECHO_TOK cmd_arguments {
  $$ = mk_echo_command(as_array_CmdStrs(&$2, NULL));
}
|       EXPORT_TOK ID EQUALS string {
  $$ = mk_export_command($2, $4);
}
|       CD_TOK {
  $$ = mk_cd_command(memory_pool_strdup(lookup_env("HOME")));
}
|       CD_TOK string {
  char* resolved_path;
  char* ret = NULL;

  if ((resolved_path = realpath($2, NULL)) != NULL) {
    ret = memory_pool_strdup(resolved_path);
    free(resolved_path);
  }

  $$ = mk_cd_command(ret);
}
|       PWD_TOK {
  $$ = mk_pwd_command();
}
|       JOBS_TOK {
  $$ = mk_jobs_command();
}
|       EXIT_TOK {
  $$ = mk_exit_command();
}
|       KILL_TOK NUM NUM {
  $$ = mk_kill_command($2, $3);
}

redir: redir_inner {
  $$ = $1;
}
|      {
  $$ = mk_redirect(NULL, NULL, false);
}



redir_inner: redir_mark string redir_inner {
  if ($1 == REDIRECT_IN) {
    $3.in = $2;
  }
  else if ($1 == REDIRECT_OUT) {
    $3.out = $2;
    $3.append = 0;
  }
  else if ($1 == REDIRECT_APPEND) {
    $3.out = $2;
    $3.append = 1;
  }

  $$ = $3;
}
|       redir_mark string {
  Redirect r;

  if ($1 == REDIRECT_IN)
    r = mk_redirect($2, NULL, false);
  else if ($1 == REDIRECT_OUT)
    r = mk_redirect(NULL, $2, false);
  else if ($1 == REDIRECT_APPEND)
    r = mk_redirect(NULL, $2, true);
  else
    r = mk_redirect(NULL, NULL, false); // Should not reach here

  $$ = r;
}



redir_mark: REDIRIN {
  $$ = REDIRECT_IN;
}
|       REDIROUT {
  $$ = REDIRECT_OUT;
}
|       REDIROUTAPP {
  $$ = REDIRECT_APPEND;
}



cmd_bg: {
  $$ = 0;
}
|       BCKGRND {
  $$ = 1;
}



cmd:    first_string cmd_arguments {
  push_front_CmdStrs(&$2, $1);

  $$ = $2;
}
|       first_string {
  CmdStrs args = new_CmdStrs(1);

  push_front_CmdStrs(&args, $1);
  push_back_CmdStrs(&args, NULL);

  $$ = args;
}



cmd_arguments: string {
  CmdStrs args = new_CmdStrs(1);

  push_front_CmdStrs(&args, $1);
  push_back_CmdStrs(&args, NULL);

  $$ = args;
}
|       string cmd_arguments {
  push_front_CmdStrs(&$2, $1);

  $$ = $2;
}



string: first_string {
  $$ = $1;
}
|       special_string {
  $$ = $1;
}

special_string: EXPORT_TOK {
  $$ = memory_pool_strdup("export");
}
|       CD_TOK {
  $$ = memory_pool_strdup("cd");
}
|       KILL_TOK {
  $$ = memory_pool_strdup("kill");
}
|       PWD_TOK {
  $$ = memory_pool_strdup("pwd");
}
|       JOBS_TOK {
  $$ = memory_pool_strdup("jobs");
}
|       EXIT_TOK {
  $$ = $1;
}

first_string: STR {
  $$ = interpret_complex_string_token($1);
}
|       SIM_STR {
  $$ = $1;
}
|       NUM {
  $$ = $1;
}
|       ID {
  $$ = $1;
}

%%

void yyerror(CommandHolder** cmds, char *str) {
  fprintf(stderr, "%s: Line %d\n", str, yylineno);
}
