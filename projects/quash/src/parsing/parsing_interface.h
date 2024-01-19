/**
 * @file parsing_interface.h
 *
 * @brief Defines an interface between c and the parser. This file is also
 * responsible for defining many of the structures used by the parser.
 */

#ifndef SRC_PARSING_PARSING_INTERFACE_H
#define SRC_PARSING_PARSING_INTERFACE_H

#include <stdbool.h>

#include "command.h"
#include "deque.h"
#include "quash.h"

/**
 * @brief Intermediate parsing structure used to determine the final
 * configuration of the redirects in a command.
 */
typedef struct Redirect {
  char* in;    /**< File name for redirect in. */
  char* out;   /**< File name for redirect out. */
  bool append; /**< Flag indicating that the redirect out should actually append
                * to the end of a file rather than truncating it */
} Redirect;

/** @cond Doxygen_Suppress */
/**
 * @struct CmdStrs
 *
 * @brief Stores strings in a deque
 *
 * @sa Example
 */
IMPLEMENT_DEQUE_STRUCT(CmdStrs, char*);

/**
 * @struct Command
 *
 * @brief Stores @a Command union'd structures in a deque
 *
 * @sa Example
 */
IMPLEMENT_DEQUE_STRUCT(Cmds, CommandHolder);

PROTOTYPE_DEQUE(CmdStrs, char*);
PROTOTYPE_DEQUE(Cmds, CommandHolder);
/** @cond Doxygen_Suppress */


/*************************************************************
 * Functions used by the parser
 *************************************************************/
/**
 * @brief Creates an intermediate @a Redirect structure and returns a copy
 *
 * @param in A string containing the name of a file to redirect the intput to a
 * command from. This should be NULL if there are not any redirections in.
 *
 * @param out A string containing the name of a file to redirect the output of a
 * command to. This should be NULL if there are not any redirections out.
 *
 * @return A copy of the constructed Redirect structure
 *
 * @sa Redirect
 */
Redirect mk_redirect(char* in, char* out, bool append);

/**
 * @brief Clean up a string by removing escape symbols and unescaped single
 * quotes. Also expands any environment variables.
 *
 * @param str The string to clean up
 *
 * @return The cleaned up and expanded string allocated on the @a MemoryPool
 *
 * @sa MemoryPool
 */
char* interpret_complex_string_token(const char* str);


/*************************************************************
 * Functions used by the parser
 *************************************************************/
/**
 * @brief Handles the call to the parser and provides a string equivalent of the
 * @a Command structure to @a QuashState
 *
 * @param[out] state The state of the quash shell. The parsed_str member of
 * QuashState is set to the stringified command structure.
 *
 * @return A pointer to the parsed command structure
 *
 * @sa CommandHolder, QuashState
 */
CommandHolder* parse(QuashState* state);

/**
 * @brief Cleanup memory dynamically allocated by the parser
 */
void destroy_parser();

#endif
