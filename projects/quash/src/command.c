/**
 * @file command.c
 *
 * @brief Implements functions used to generate and manage commands.
 */

#include "command.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Move the created command into a holder
CommandHolder mk_command_holder(char* redirect_in,
                                char* redirect_out,
                                char flags,
                                Command cmd) {
  return (CommandHolder) {
    redirect_in,
    redirect_out,
    flags,
    cmd
  };
}

// Create GenericCommand
Command mk_generic_command(char** args) {
  Command cmd;

  cmd.generic = (GenericCommand) {
    GENERIC,
    args
  };

  return cmd;
}

// Create EchoCommand
Command mk_echo_command(char** strs) {
  Command cmd;

  cmd.generic = (GenericCommand) {
    ECHO,
    strs
  };

  return cmd;
}

// Create ExportCommand
Command mk_export_command(char* env_var, char* val) {
  Command cmd;

  cmd.export = (ExportCommand) {
    EXPORT,
    env_var,
    val
  };

  return cmd;
}

// Create CDCommand structure
Command mk_cd_command(char* dir) {
  Command cmd;

  cmd.cd = (CDCommand) {
    CD,
    dir
  };

  return cmd;
}

// Create KillCommand structure
Command mk_kill_command(char* sig, char* job) {
  Command cmd;

  cmd.kill = (KillCommand) {
    KILL,
    strtol(sig, NULL, 10),
    strtol(job, NULL, 10),
    sig,
    job
  };

  return cmd;
}

// Create PWDCommand structure
Command mk_pwd_command() {
  Command cmd;

  cmd.pwd = (PWDCommand) {
    PWD
  };

  return cmd;
}

// Create JobCommand structure
Command mk_jobs_command() {
  Command cmd;

  cmd.jobs = (JobsCommand) {
    JOBS
  };

  return cmd;
}

// Create ExitCommand structure
Command mk_exit_command() {
  Command cmd;

  cmd.exit = (ExitCommand) {
    EXIT
  };

  return cmd;
}

// Create EOCCommand structure
Command mk_eoc() {
  Command cmd;

  cmd.eoc = (EOCCommand) {
    EOC
  };

  return cmd;
}


CommandType get_command_type(Command cmd) {
  return cmd.simple.type;
}

CommandType get_command_holder_type(CommandHolder holder) {
  return get_command_type(holder.cmd);
}

#ifdef DEBUG
static void __print_generic_cmd(GenericCommand cmd) {
  if (cmd.args != NULL) {
    for (size_t i = 0; cmd.args[i] != NULL; ++i)
      printf("[%s] ", cmd.args[i]);
  }
  else {
    printf("#NULL# ");
  }
}

static void __print_echo_cmd(EchoCommand cmd) {
  printf("%%ECHO%%");
}

static void __print_export_cmd(ExportCommand cmd) {
  printf("%%EXPORT%% [VAR: %s] [VAL: %s]", cmd.env_var, cmd.val);
}

static void __print_cd_cmd(CDCommand cmd) {
  printf("%%CD%% [DIR: %s]", cmd.dir);
}

static void __print_kill_cmd(KillCommand cmd) {
  printf("%%KILL%% [JOB: %d] [SIG: %d]", cmd.sig, cmd.job);
}

static void __print_simple_cmd(const char* str) {
  printf("%%%s%%", str);
}

static void __print_command(Command cmd) {
  switch (get_command_type(cmd)) {
  case GENERIC:
    __print_generic_cmd(cmd.generic);
    break;

  case ECHO:
    __print_echo_cmd(cmd.echo);
    break;

  case EXPORT:
    __print_export_cmd(cmd.export);
    break;

  case CD:
    __print_cd_cmd(cmd.cd);
    break;

  case KILL:
    __print_kill_cmd(cmd.kill);
    break;

  case PWD:
    __print_simple_cmd("PWD");
    break;

  case JOBS:
    __print_simple_cmd("JOBS");
    break;

  case EXIT:
    __print_simple_cmd("EXIT");
    break;

  case EOC:
    printf("--- EOC ---");
    break;

  default:
    printf("{???}");
  }
}

static void __print_command_holder(CommandHolder holder) {
  putc('{', stdout);

  __print_command(holder.cmd);

  printf("<");

  if (holder.flags & BACKGROUND)
    printf("BG ");
  else
    printf("FG ");

  if (holder.flags & PIPE_IN)
    printf("P_IN ");

  if (holder.flags & PIPE_OUT)
    printf("P_OUT ");

  if (holder.flags & REDIRECT_IN)
    printf("(R_IN: %s) ", holder.redirect_in);

  if (holder.flags & REDIRECT_APPEND)
    printf("(R_APPEND: ");
  else if (holder.flags & REDIRECT_OUT)
    printf("(R_OUT: ");

  if (holder.flags & REDIRECT_OUT)
    printf("%s) ", holder.redirect_out);

  printf("*0x%02x*", holder.flags);

  printf(">");

  putc('}', stdout);
}

void debug_print_script(const CommandHolder* holders) {
  if (holders != NULL) {
    size_t i;

    for (i = 0; get_command_holder_type(holders[i]) != EOC; ++i) {
      __print_command_holder(holders[i]);
      printf("\n");
    }

    __print_command_holder(holders[i]);
    printf("\n");
  }
}

#else

// Do nothing
void debug_print_script(const CommandHolder* holders) {
  (void) holders;
}

#endif
