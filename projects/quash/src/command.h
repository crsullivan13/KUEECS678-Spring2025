/**
 * @file command.h
 *
 * @brief Command structures and functions for defining and managing commands
 */

#ifndef SRC_COMMAND_H
#define SRC_COMMAND_H

#include <stdbool.h>

/**
 * @def REDIRECT_IN
 *
 * @brief Flag bit indicating whether a @a GenericCommand should read from
 * standard in
 */
/**
 * @def REDIRECT_OUT
 *
 * @brief Flag bit indicating whether a @a GenericCommand should write to
 * standard out truncating the original file
 */
/**
 * @def REDIRECT_APPEND
 *
 * @brief Flag bit indicating whether a @a GenericCommand should write to
 * standard out appending its output
 */
/**
 * @def PIPE_IN
 *
 * @brief Flag bit indicating whether a @a GenericCommand should read from
 * a pipe
 */
/**
 * @def PIPE_OUT
 *
 * @brief Flag bit indicating whether a @a GenericCommand should write to
 * a pipe
 */
/**
 * @def BACKGROUND
 *
 * @brief Flag bit indicating whether a @a GenericCommand should be run in
 * the background
 */
#define REDIRECT_IN     (0x01)
#define REDIRECT_OUT    (0x04)
#define REDIRECT_APPEND (0x08)
#define PIPE_IN         (0x10)
#define PIPE_OUT        (0x20)
#define BACKGROUND      (0x40)

/**
 * @brief All possible types of commands
 *
 * These are useful for recovering what the actual type of a command is if the
 * command is placed in a @a Command union
 *
 * @sa Command
 */
typedef enum CommandType {
  EOC = 0, // psudo-command declaring the end of a command array
  GENERIC,
  ECHO,
  EXPORT,
  KILL,
  CD,
  PWD,
  JOBS,
  EXIT
} CommandType;

// Command Structures
//
// All command structures must begin with the `CommandType type` field since this
// is used to determine what the structure is after it is placed in a `union
// Command` structure.

/**
 * @brief A command which has no arguments
 *
 * All command structures can be correctly read as a @a SimpleCommand since they
 * all have the @a CommandType field as the first field.
 *
 * @sa Command, CommandType
 */
typedef struct SimpleCommand {
  CommandType type; /**< Type of command */
} SimpleCommand;

/**
 * @brief Commands that take any number of arguments and are not built into
 * Quash
 *
 * @sa Command
 */
typedef struct GenericCommand {
  CommandType type; /**< Type of command */
  char** args;      /**< A NULL terminated array of c-strings ready to pass to
                     * @a exec functions */
} GenericCommand;

/**
 * @brief Alias for @a GenericCommand to denote a command to print strings
 *
 * @note EchoCommand is similar to a generic command but is a builtin command
 *
 * @sa GenericCommand, Command
 */
typedef GenericCommand EchoCommand;

/**
 * @brief Command to set environment variables
 *
 * @sa CommandType, lookup_env(), write_env(), Command
 */
typedef struct ExportCommand {
  CommandType type; /**< Type of command */
  char* env_var;    /**< Name of environment variable to set */
  char* val;        /**< String that should be stored in @a env_var environment
                     * variable */
} ExportCommand;

/**
 * @brief Command to change directories
 *
 * @sa realpath(), Command
 */
typedef struct CDCommand {
  CommandType type; /**< Type of command */
  char* dir;        /**< Path to the directory we wish to change to */
} CDCommand;

/**
 * @brief Command to kill a process based on it's job id
 *
 * @sa CommandType, Command
 */
typedef struct KillCommand {
  CommandType type; /**< Type of command */
  int sig;          /**< Signal to send to the job */
  int job;          /**< Job id number */
  char* sig_str;    /**< String holding the signal number (used for printing) */
  char* job_str;    /**< String holding the job id number (used for printing) */
} KillCommand;

/**
 * @brief Alias for @a SimpleCommand to denote a print working directory command
 *
 * @sa SimpleCommand, Command
 */
typedef SimpleCommand PWDCommand;

/**
 * @brief Alias for @a SimpleCommand to denote a print jobs list
 *
 * @sa SimpleCommand, Command, Job
 */
typedef SimpleCommand JobsCommand;

/**
 * @brief Alias for @a SimpleCommand to denote a termination of the program
 *
 * @sa end_main_loop(), SimpleCommand, Command
 */
typedef SimpleCommand ExitCommand;

/**
 * @brief Alias for @a SimpleCommand to denote the end of a command
 *
 * @sa SimpleCommand, Command
 */
typedef SimpleCommand EOCCommand;

/**
 * @brief Make all command types the same size and interchangable
 *
 * This is useful for arrays or making a common entry point for all command
 * types. The exact type information can be recovered later with the @a
 * get_command_type() function.
 *
 * @sa get_command_type, SimpleCommand, GenericCommand, EchoCommand,
 * ExportCommand, CDCommand, KillCommand, PWDCommand, JobsCommand, ExitCommand,
 * EOCCommand
 */
typedef union Command {
  SimpleCommand simple;   /**< Read structure as a @a SimpleCommand */
  GenericCommand generic; /**< Read structure as a @a GenericCommand */
  EchoCommand echo;       /**< Read structure as a @a ExportCommand */
  ExportCommand export;   /**< Read structure as a @a ExportCommand */
  CDCommand cd;           /**< Read structure as a @a CDCommand */
  KillCommand kill;       /**< Read structure as a @a KillCommand */
  PWDCommand pwd;         /**< Read structure as a @a PWDCommand */
  JobsCommand jobs;       /**< Read structure as a @a JobsCommand */
  ExitCommand exit;       /**< Read structure as a @a ExitCommand */
  EOCCommand eoc;         /**< Read structure as a @a EOCCommand */
} Command;

/**
 * @brief Contains information about the properties of the command
 *
 * @sa REDIRECT_IN, REDIRECT_OUT, REDIRECT_APPEND, PIPE_IN, PIPE_OUT,
 * BACKGROUND, Command
 */
typedef struct CommandHolder {
  char* redirect_in;  /**< Redirect standard in of this command to a file name
                       * @a redirect_in */
  char* redirect_out; /**< Redirect standard out of this command to a file name
                       * @a redirect_out */
  char flags;         /**< A set of bits that hold information about how to execute the
                       * command. The properties can be extracted from the flags field by using a
                       * bit-wise & (i.e. flags & @a PIPE_IN) are macro defined as:
                       *   - @a REDIRECT_IN
                       *   - @a REDIRECT_OUT
                       *   - @a REDIRECT_APPEND
                       *   - @a PIPE_IN
                       *   - @a PIPE_OUT
                       *   - @a BACKGROUND */
  Command cmd;        /**< A @a Command to hold */
} CommandHolder;

// Command structure constructors

/**
 * @brief Create the properties for a command
 *
 * @param redirect_in Redirect standard in of this command to a file name @a
 * redirect_in
 *
 * @param redirect_out Redirect standard out of this command to a file name @a
 * redirect_out
 *
 * @param flags A set of bits that hold information about how to execute the
 * command. The properties can be extracted from the flags field by using a
 * bit-wise & (i.e. flags & @a PIPE_IN) are macro defined as:
 *   - @a REDIRECT_IN
 *   - @a REDIRECT_OUT
 *   - @a REDIRECT_APPEND
 *   - @a PIPE_IN
 *   - @a PIPE_OUT
 *   - @a BACKGROUND
 *
 * @param cmd Command to hold
 *
 * @return A CommandHolder
 *
 * @sa CommandType, REDIRECT_IN, REDIRECT_OUT, REDIRECT_APPEND, PIPE_IN, PIPE_OUT,
 * BACKGROUND, Command, CommandHolder
 */
CommandHolder mk_command_holder(char* redirect_in, char* redirect_out, char flags, Command cmd);

/**
 * @brief Create a @a GenericCommand structure and return a copy
 *
 * @param args A NULL terminated array of c-strings ready to pass to @a exec
 * functions
 *
 * @return Copy of constructed GenericCommand
 *
 * @sa CommandType, REDIRECT_IN, REDIRECT_OUT, REDIRECT_APPEND, PIPE_IN, PIPE_OUT,
 * BACKGROUND, Command, GenericCommand
 */
Command mk_generic_command(char** args);

/**
 * @brief Create a @a GenericCommand structure and return a copy
 *
 * @return Copy of constructed GenericCommand
 *
 * @sa CommandType, REDIRECT_IN, REDIRECT_OUT, REDIRECT_APPEND, PIPE_IN, PIPE_OUT,
 * BACKGROUND, Command, GenericCommand
 */
Command mk_echo_command(char** args);

/**
 * @brief Create a @a ExportCommand structure and return a copy
 *
 * @param env_var Name of environment variable to set
 *
 * @param val String that should be stored in @a env_var environment variable
 *
 * @return Copy of constructed ExportCommand
 *
 * @sa lookup_env(), write_env(), Command, ExportCommand
 */
Command mk_export_command(char* env_var, char* val);

/**
 * @brief Create a @a CDCommand structure and return a copy
 *
 * @param dir Path to the directory we wish to change to
 *
 * @return Copy of constructed CDCommand
 *
 * @sa realpath(), Command, CDCommand
 */
Command mk_cd_command(char* dir);

/**
 * @brief Create a @a KillCommand structure and return a copy
 *
 * @param sig Signal to send to the job
 *
 * @param job Job id number
 *
 * @return Copy of constructed KillCommand
 *
 * @sa Command, KillCommand
 */
Command mk_kill_command(char* sig, char* job);

/**
 * @brief Create a @a PWDCommand structure and return a copy
 *
 * @return Copy of constructed PWDCommand
 *
 * @sa Command, PWDCommand
 */
Command mk_pwd_command();

/**
 * @brief Create a @a JobsCommand structure and return a copy
 *
 * @return Copy of constructed JobsCommand
 *
 * @sa Command, JobsCommand
 */
Command mk_jobs_command();

/**
 * @brief Create a @a ExitCommand structure and return a copy
 *
 * @return Copy of constructed ExitCommand
 *
 * @sa Command, ExitCommand
 */
Command mk_exit_command();

/**
 * @brief Create a @a EOCCommand structure and return a copy
 *
 * @return Copy of constructed EOCCommand
 *
 * @sa Command, EOCCommand
 */
Command mk_eoc();

/**
 * @brief Get the type of the command
 *
 * Uses the property that all @a Command variants can be cast to @a SimpleCommand to
 * extract the @a CommandType of the Command.
 *
 * @param cmd Command from which this function extracts the @a CommandType
 *
 * @return The resulting @a CommandType of the `cmd` parameter
 *
 * @sa CommandType, Command, SimpleCommand
 */
CommandType get_command_type(Command cmd);

/**
 * @brief Get the type of the @a Command in the @a CommandHolder
 *
 * Uses the property that all Command variants can be cast to @a SimpleCommand to
 * extract the @a CommandType of the Command.
 *
 * @param holder CommandHolder from which this function extracts the @a
 * CommandType
 *
 * @return The resulting CommandType of the `cmd` parameter
 *
 * @sa CommandType, CommandHolder, SimpleCommand
 */
CommandType get_command_holder_type(CommandHolder holder);

/**
 * @brief Print all commands in the script with @a print_command()
 *
 * @note This only works when the @a DEBUG macro is defined
 *
 * @param holders @a CommandHolder array to print
 *
 * @sa CommandHolder
 */
void debug_print_script(const CommandHolder* holders);

#endif
