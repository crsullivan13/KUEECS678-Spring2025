#include "parsing_interface.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#include "memory_pool.h"
#include "parse.tab.h"

IMPLEMENT_DEQUE_STRUCT(SizeStack, size_t);
IMPLEMENT_DEQUE_STRUCT(StrBuilder, char);
IMPLEMENT_DEQUE_STRUCT(MPStrBuilder, char);

IMPLEMENT_DEQUE(SizeStack, size_t);
IMPLEMENT_DEQUE(StrBuilder, char);
IMPLEMENT_DEQUE_MEMORY_POOL(MPStrBuilder, char);
IMPLEMENT_DEQUE_MEMORY_POOL(CmdStrs, char*);
IMPLEMENT_DEQUE_MEMORY_POOL(Cmds, CommandHolder);

extern void destroy_lex();

// Generate a string based off of a pipable generic command
static inline void __stringify_generic_cmd(GenericCommand cmd, CmdStrs* strs) {
  // Extract argument strings
  for (size_t i = 0; cmd.args[i] != NULL; ++i)
    push_back_CmdStrs(strs, cmd.args[i]);
}

static inline void __stringify_echo_cmd(EchoCommand cmd, CmdStrs* strs) {
  push_back_CmdStrs(strs, memory_pool_strdup("echo"));

  // Extract argument strings
  for (size_t i = 0; cmd.args[i] != NULL; ++i)
    push_back_CmdStrs(strs, cmd.args[i]);
}

// Generate a string based off the export command
static void __stringify_export_cmd(ExportCommand cmd, CmdStrs* strs) {
  push_back_CmdStrs(strs, memory_pool_strdup("export"));
  push_back_CmdStrs(strs, cmd.env_var);
  push_back_CmdStrs(strs, cmd.val);
}

// Generate a string based off of the cd command
static void __stringify_cd_cmd(CDCommand cmd, CmdStrs* strs) {
  push_back_CmdStrs(strs, memory_pool_strdup("cd"));
  push_back_CmdStrs(strs, cmd.dir);
}

// Generate a string based off of the kill command
static void __stringify_kill_cmd(KillCommand cmd, CmdStrs* strs) {
  push_back_CmdStrs(strs, memory_pool_strdup("kill"));
  push_back_CmdStrs(strs, cmd.sig_str);
  push_back_CmdStrs(strs, cmd.job_str);
}

// Generate a string based off the a variant of a simple command
static void __stringify_simple_cmd(const char* str, CmdStrs* strs) {
  push_back_CmdStrs(strs, memory_pool_strdup(str));
}

// Entry point for turning a command into a string
static void __stringify_command(Command cmd, CmdStrs* strs) {
  switch (get_command_type(cmd)) {
  case GENERIC:
    __stringify_generic_cmd(cmd.generic, strs);
    break;

  case ECHO:
    __stringify_echo_cmd(cmd.echo, strs);
    break;

  case EXPORT:
    __stringify_export_cmd(cmd.export, strs);
    break;

  case CD:
    __stringify_cd_cmd(cmd.cd, strs);
    break;

  case KILL:
    __stringify_kill_cmd(cmd.kill, strs);
    break;

  case PWD:
    __stringify_simple_cmd("PWD", strs);
    break;

  case JOBS:
    __stringify_simple_cmd("JOBS", strs);
    break;

  case EXIT:
    __stringify_simple_cmd("EXIT", strs);
    break;

  default:
    break;
  }
}

static void __stringify_holder(CommandHolder holder, CmdStrs* strs) {
  __stringify_command(holder.cmd, strs);

  // Generate redirect symbols and extract file names
  if (holder.flags & REDIRECT_IN) {
    push_back_CmdStrs(strs, memory_pool_strdup("<"));
    push_back_CmdStrs(strs, holder.redirect_in);
  }

  if (holder.flags & REDIRECT_APPEND)
    push_back_CmdStrs(strs, memory_pool_strdup(">>"));
  else if (holder.flags & REDIRECT_OUT)
    push_back_CmdStrs(strs, memory_pool_strdup(">"));

  if (holder.flags & REDIRECT_OUT)
    push_back_CmdStrs(strs, holder.redirect_out);

  // Generate the pipe symbol
  if (holder.flags & PIPE_OUT)
    push_back_CmdStrs(strs, memory_pool_strdup("|"));
}

// Create an array of strings representing the command returned from the parser
static void __stringify_script(const CommandHolder* holders, CmdStrs* strs) {
  assert(holders != NULL);
  assert(strs != NULL);

  if (holders != NULL) {
    for (size_t i = 0; get_command_holder_type(holders[i]) != EOC; ++i)
      __stringify_holder(holders[i], strs);

    if (holders[0].flags & BACKGROUND)
      push_back_CmdStrs(strs, memory_pool_strdup("&"));
  }

  push_back_CmdStrs(strs, NULL);
}

// Concatenates arrays of strings together to form a single string with each
// string separated by a space.
static char* __condense_string_array(char** str_arr) {
  SizeStack ss = new_SizeStack(10);
  size_t len = 0;

  for (size_t i = 0; str_arr[i] != NULL; ++i) {
    size_t size = strlen(str_arr[i]) + 1;

    push_back_SizeStack(&ss, size);
    len += size;
  }

  char* ret = (char*) memory_pool_alloc(len + 1);
  size_t pos = 0;

  for (size_t i = 0; str_arr[i] != NULL; ++i) {
    strcpy(ret + pos, str_arr[i]);
    pos += pop_front_SizeStack(&ss);
    ret[pos - 1] = ' ';
  }

  ret[pos] = '\0';
  destroy_SizeStack(&ss);

  return ret;
}

// Helper for __interpret_deref: Checks if the character is a valid first
// character for an identifier
static inline bool __is_first_identifier_char(char c) {
  return isalpha(c) || c == '_';
}

// Helper for __interpret_deref: Checks if the character is a valid non-leading
// identifier character
static inline bool __is_identifier_char(char c) {
  return isalnum(c) || c == '_';
}

// Expand an environment variable onto a string
static void __interpret_deref(MPStrBuilder* bld, const char* str, int* idx) {
  assert(str != NULL);
  assert(str[*idx] == '$');
  assert(peek_back_MPStrBuilder(bld) == '$');

  // Remove the dereference symbol at the back of the bld deque
  pop_back_MPStrBuilder(bld);

  StrBuilder tmp = new_StrBuilder(16);
  char c;

  // Extract the identifier characters. Since this is intended only as a helper
  // function we assume that interpret_complex_string token has already noticed
  // a valid first identifier character after the dereference symbol.
  while (__is_identifier_char((c = str[++(*idx)])))
    push_back_StrBuilder(&tmp, c);

  // idx increments one too far in the while loop so bring it back down
  --(*idx);

  // Add the null terminator to the string
  push_back_StrBuilder(&tmp, '\0');

  // Extract the id string and lookup the environment variable
  char* id = as_array_StrBuilder(&tmp, NULL);
  const char* env_var = lookup_env(id);

  free(id);

  // Append env_var to the string builder
  if (env_var != NULL) {
    for (int i = 0; env_var[i] != '\0'; ++i)
      push_back_MPStrBuilder(bld, env_var[i]);
  }
}

// Cleans up escapes and unescaped single quotes and expands environment
// variables found in a string
char* interpret_complex_string_token(const char* str) {
  assert(str != NULL);

  MPStrBuilder bld = new_MPStrBuilder(64);
  int i;
  int len = strlen(str);
  bool in_quotes = false;

  for (i = 0; i <= len; ++i) {
    push_back_MPStrBuilder(&bld, str[i]);

    switch (str[i]) {
    case '\\':                // Remove valid escape characters
      if (!in_quotes) {
        switch (str[i+1]) {
        case '\\':
        case '\'':
        case '#':
        case '$':
        case '=':
        case '&':
        case '|':
        case ';':
        case ' ':
        case '\t':
          update_back_MPStrBuilder(&bld, str[++i]);
          break;

        case '\n':
          pop_back_MPStrBuilder(&bld);
          ++i;
          break;

        default:
          break;
        }
      }
      else if (str[i+1] == '\'') {
        update_back_MPStrBuilder(&bld, '\'');
        ++i;
      }
      break;

    case '\'':                // Remove single quotes and toggle quote state
      in_quotes = !in_quotes;
      pop_back_MPStrBuilder(&bld);
      break;

    case '$':                 // Try to dereference environment variables
      if (!in_quotes && __is_first_identifier_char(str[i + 1]))
        __interpret_deref(&bld, str, &i);
      break;

    default:
      break;
    }
  }

  // Add a null terminator
  update_back_MPStrBuilder(&bld, '\0');

  assert(!in_quotes);

  return as_array_MPStrBuilder(&bld, NULL);
}

// Build a Redirect structure
Redirect mk_redirect(char* in, char* out, bool append) {
  return (Redirect) {
    in,
    out,
    append
  };
}

// Parse a command
CommandHolder* parse(QuashState* state) {
  assert(state != NULL);

  CommandHolder* holders;

  yyparse(&holders);

  if (holders != NULL) {
    CmdStrs strs = new_CmdStrs(10);
    __stringify_script(holders, &strs);
    state->parsed_str = __condense_string_array(as_array_CmdStrs(&strs, NULL));
  }

  return holders;
}

// Clean up dynamically allocated memory in the parser
void destroy_parser() {
  destroy_lex();
}
