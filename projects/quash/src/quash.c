/**
 * @file quash.c
 *
 * Quash's main file
 */

/**************************************************************************
 * Included Files
 **************************************************************************/
#include "quash.h"

#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "command.h"
#include "execute.h"
#include "parsing_interface.h"
#include "memory_pool.h"

/**************************************************************************
 * Private Variables
 **************************************************************************/
static QuashState state;

/**************************************************************************
 * Private Functions
 **************************************************************************/
static void print_prompt() {
  bool should_free = false;
  char* cwd = get_current_directory(&should_free);
  int last_dir_idx = 0;

  assert(cwd != NULL);

  for (int i = 0; cwd[i] != '\0'; ++i) {
    if (cwd[i] == '/' && cwd[i] != '\0') {
      last_dir_idx = i + 1;
    }
  }

  printf("[QUASH - %s@%s %s]$ ",
          lookup_env("USER"),
          lookup_env("HOSTNAME"),
          cwd + last_dir_idx);
  fflush(stdout);

  if (should_free)
    free (cwd);
}

QuashState initial_state() {
  return (QuashState) {
    true,
    isatty(STDIN_FILENO),
    NULL
  };
}

/**************************************************************************
 * Public Functions
 **************************************************************************/
// Check if loop is running
bool is_running() {
  return state.running;
}

// Get a copy of the string
char* get_command_string() {
  return strdup(state.parsed_str);
}

bool is_tty() {
  return state.is_a_tty;
}

// Stop Quash from requesting more input
void end_main_loop(int exit_status) {
  state.running = false;
}

/**
 * @brief Quash entry point
 *
 * @param argc argument count from the command line
 *
 * @param argv argument vector from the command line
 *
 * @return program exit status
 */
int main(int argc, char** argv) {
  state = initial_state();

  if (is_tty()) {
    puts("Welcome to Quash!");
    puts("Type \"exit\" or \"quit\" to quit");
    puts("---------------------------------");
    fflush(stdout);
  }

  atexit(destroy_parser);
  atexit(destroy_memory_pool);

  // Main execution loop
  while (is_running()) {
    if (is_tty())
      print_prompt();

    initialize_memory_pool(1024);

    /* CommandHolder* script = parse(&state); */

    /* if (script != NULL) */
    /*   run_script(script); */
    CommandHolder* script;

    while ((script = parse(&state)) == NULL); // Parse until we come across
                                              // something that is an
                                              // interesting command while also
                                              // not a syntax error
    run_script(script);

    destroy_memory_pool();
  }

  return EXIT_SUCCESS;
}
