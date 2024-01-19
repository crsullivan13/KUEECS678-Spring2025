/**
 * @file execute.h
 *
 * @brief Functions for interpreting and running commands
 */

#ifndef SRC_EXECUTE_H
#define SRC_EXECUTE_H

#include <stdbool.h>
#include <unistd.h>

#include "command.h"

/**
 * @brief Function to get environment variable values
 *
 * @param env_var Environment variable to lookup
 *
 * @return String containing the value of the environment variable env_var
 */
const char* lookup_env(const char* env_var);

/**
 * @brief Function to set and define environment variable values
 *
 * @param env_var Environment variable to set
 *
 * @param val String with the value to set the environment variable env_var
 */
void write_env(const char* env_var, const char* val);

/**
 * @brief Get the real current working directory
 *
 * This is not necessarily the same as the PWD environment variable and setting
 * PWD does not actually change the current working directory.
 *
 * @param[out] should_free Set this to true if the returned string should be
 * free'd by the caller and false otherwise.
 *
 * @return A string representing the current working directory
 */
char* get_current_directory(bool* should_free);

/**
 * @brief Check on background jobs to see if they have exited
 */
void check_jobs_bg_status();

/**
 * @brief Print a job to standard out
 *
 * We use the minimum of what a Job structure should contain to pass to this
 * function.
 *
 * @param job_id Job identifier number.
 *
 * @param pid Process id of a process belonging to this job.
 *
 * @param cmd String holding an approximation of what the user typed in for the
 * command.
 */
void print_job(int job_id, pid_t pid, const char* cmd);

/**
 * @brief Print the start up of a background job to standard out
 *
 * We use the minimum of what a Job should contain to pass to this function.
 *
 * @param job_id Job identifier number.
 *
 * @param pid Process id of a process belonging to this job.
 *
 * @param cmd String holding an aproximation of what the user typed in for the
 * command.
 */
void print_job_bg_start(int job_id, pid_t pid, const char* cmd);

/**
 * @brief Print the completion of a background job to standard out
 *
 * We use the minimum of what a Job should contain to pass to this function.
 *
 * @param job_id Job identifier number.
 *
 * @param pid Process id of a process belonging to this job.
 *
 * @param cmd String holding an aproximation of what the user typed in for the
 * command.
 */
void print_job_bg_complete(int job_id, pid_t pid, const char* cmd);

/**
 * @brief Run a generic (non-builtin) command
 *
 * @param cmd A @a GenericCommand command
 *
 * @sa GenericCommand
 */
void run_generic(GenericCommand cmd);

/**
 * @brief Run the builtin echo command
 *
 * @param cmd An @a EchoCommand
 *
 * @sa EchoCommand
 */
void run_echo(EchoCommand cmd);

/**
 * @brief Run the builtin export command
 *
 * @param cmd An @a ExportCommand
 *
 * @sa ExportCommand
 */
void run_export(ExportCommand cmd);

/**
 * @brief Run the builtin cd (change directory) command
 *
 * @param cmd An @a CDCommand
 *
 * @sa CDCommand
 */
void run_cd(CDCommand cmd);

/**
 * @brief Run the builtin kill command
 *
 * @param cmd A @a KillCommand
 *
 * @sa KillCommand
 */
void run_kill(KillCommand cmd);

/**
 * @brief Run the builtin pwd (print working directory) command
 *
 * @sa PWDCommand
 */
void run_pwd();

/**
 * @brief Run the builtin jobs command to show the jobs list
 *
 * @sa PWDCommand
 */
void run_jobs();

/**
 * @brief Common entry point for all commands
 *
 * This function resolves the type of the command and calls the relevant run
 * function
 *
 * @param holders An array of command holders
 *
 * @sa Command
 */
void run_script(CommandHolder* holders);

#endif
