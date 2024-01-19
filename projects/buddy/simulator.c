#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "buddy.h"

/**
 * Various program statuses indicating success or failure of an operation
 */
typedef enum status_t {
	SUCCESS = 0,
	OUTOFMEMORY,
	DOUBLEFREE,
	BADINPUT
} status_t;

/**
 * Severity of faults
 */
typedef enum severity_t {
	ERROR,
	WARNING
} severity_t;

/**
 * Tracks a variable's pointer in memory and whether it is allocated
 * or not
 */
typedef struct var_t {
	void* mem;   ///< A pointer to a memory block
	bool in_use; ///< Is this variable currently in use? This is probably redundant if we assume variables not in use are NULL. For now just leave it as it is
} var_t;


static FILE *in = NULL;    // Input file
static var_t var_map[256]; // Keep track of variable allocations
static int linenum = 0;    // Line number in input file


/**
 * Resolve a variable by name
 *
 * @param var Name of variable
 * @return Returns a pointer to location of the variable's
 * representation. Returns NULL if the var is not an alphabetic character.
 */
static var_t* get_var(char var)
{
	if ((var >= 'a' && var <= 'z') || (var >= 'A' && var <= 'Z'))
		return &var_map[(int) var];
	else
		return NULL;
}

/**
 * Multi-purpose fault error message
 *
 * @param cmd A string representing a command
 * @param msg A string that describes the nature of the fault
 * @param sev Specify the severity of the fault
 */
static void print_fault(const char* cmd, const char* msg, severity_t sev)
{
	const char* severity_msg;

	switch(sev) {
	case ERROR:
		severity_msg = "ERROR";
		break;
	case WARNING:
		severity_msg = "WARNING";
		break;
	default:
		severity_msg = "?????";
	}

	fprintf(stderr, "%s: Line %d: %s\n", severity_msg, linenum, msg);
	fprintf(stderr, "    Faulting Command: %s\n", cmd);
}

/**
 * Throw a parsing error
 *
 * @param cmd A string representing the faulting command
 * @return Returns BADINPUT status
 */
static status_t parse_error(const char* cmd)
{
	print_fault(cmd, "Failed to parse command", ERROR);
	return BADINPUT;
}

/**
 * Parses an allocation instruction
 *
 * @param cmd String representing an allocation command in the program
 * @returns Status of read and execute
 */
static status_t parse_alloc(char* cmd)
{
	assert(cmd != NULL);
	assert(cmd[0] != '\0');

	char var_name;
	int size;
	char alter_size;
	int matched;

	errno = 0;
	matched = sscanf(cmd, "%c=alloc(%d%c)", &var_name, &size, &alter_size);

	// Error check sprintf
	if (matched == 3 && errno == 0) {
		// Check what the alter_size variable actually contains
		switch (alter_size) {
		case 'k':
		case 'K':
			size *= 1024;
		case ')':
			break;
		default:
			return parse_error(cmd);
		}
	}
	else {
		return parse_error(cmd);
	}

	// Resolve variable
	var_t* var = get_var(var_name);

	if (var == NULL)
		return parse_error(cmd);

	// Allocate variable
	var->mem = buddy_alloc(size);

	if (var->mem == NULL) {
		print_fault(cmd, "buddy_alloc returned NULL", WARNING);
		printf("Out of memory\n");
		return OUTOFMEMORY;
	}

	var->in_use = true;

	return SUCCESS;
}

/**
 * Parses a free instruction
 *
 * @param cmd String representing an allocation command in the program
 * @returns Status of read and execute
 */
static status_t parse_free(char* cmd)
{
	assert(cmd != NULL);

	char var_name;
	int matched;
	var_t* var;

	// Read the command string
	errno = 0;
	matched = sscanf(cmd, "free(%c)", &var_name);

	// Check if sscanf was valid
	if (matched != 1 || errno != 0 || (var = get_var(var_name)) == NULL)
		return parse_error(cmd);

	// Ensure that the variable is in use
	if (!var->in_use) {
		print_fault(cmd, "Double free", ERROR);
		return DOUBLEFREE;
	}

	// Free variable
	buddy_free(var->mem);
	var->mem = NULL;
	var->in_use = false;

	return SUCCESS;
}


/**
 * Simplify the command and call one of the sub parser functions
 *
 * @param cmd Raw command string. This parameter is mutated. This
 * parameter cannot be NULL.
 * @param cmd_len Length in bytes of the command string.
 * @return Program status.
 */
static status_t parse_command(char* cmd, int cmd_len)
{
	assert(cmd != NULL);

	int ws_cursor = 0;

	if (cmd[0] == '\0' || cmd[0] == '\n' || cmd[0] == '\r')
		return SUCCESS;

	// remove whitespace from command
	for (int i = 0; i < cmd_len; ++i) {
		switch (cmd[i]) {
		case ' ':
		case '\n':
		case '\r':
		case '\t':
			break;

		default:
			cmd[ws_cursor++] = cmd[i];
		}
	}

	status_t status;

	// We have 2 commands: alloc and free.
	if (strstr(cmd, "alloc") != NULL)
		status = parse_alloc(cmd);
	else if (strstr(cmd, "free") != NULL)
		status = parse_free(cmd);
	else
		return parse_error(cmd);

	if (status != SUCCESS)
		return status;

	// Output free blocks
	buddy_dump();

	return SUCCESS;
}

/**
 * Feed each line of a file into the function parse_command
 *
 * @return Program status.
 */
static status_t parse_file()
{
	char* line = NULL;
	size_t len = 0;
	ssize_t read;

	status_t status = SUCCESS;

	while (status == SUCCESS && (read = getline(&line, &len, in)) > 0) {
		++linenum;
		status = parse_command(line, len);
	}

	return status;
}


/**
 * Output program manual
 *
 * @param prog_name Name of the program passed in as a command line argument.
 * @param out File stream to write to.
 */
void print_usage(char* prog_name, FILE* out)
{
	fprintf(out, "Usage:\n");
	fprintf(out, "  ./%s [-i filename]\n", prog_name);
	fprintf(out, "     -i [optional] - Specify an input file name to read from. If this option \n");
	fprintf(out, "                     is not used then input is expected from standard input.\n");
}

int main(int argc, char** argv)
{
	int opt;

	status_t prog_status;

	in = stdin;

	// Parse command line options
	while ((opt = getopt(argc, argv, "i:")) != -1) {
		switch (opt) {
		case 'i':
			in = fopen(optarg, "r");
			break;

		case '?':
			switch (optopt) {
			case 'i':
				fprintf(stderr, "ERROR: Missing filename after '%c'", optopt);
				return EXIT_FAILURE;
			}

			print_usage(argv[0], stdout);
			return EXIT_FAILURE;
		}
	}

	// Error check the input file
	if (in == NULL) {
		perror("ERROR: Failed to open input file.");
		return EXIT_FAILURE;
	}

	// Zero memory
	memset(var_map, 0, sizeof(var_map));

	// Execute program
	buddy_init();
	prog_status = parse_file();

	if (in != stdin)
		fclose(in);

	if (prog_status == SUCCESS)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}
