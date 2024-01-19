/**
 * @file debug.h
 *
 * @brief This file holds useful macros for debugging purposes.
 */

#include <stdio.h>

/**
 * @def PRINT_DEBUG()
 *
 * @brief Print statement that only triggers if DEBUG is defined
 *
 * @param fmt Format for the fprintf statement. This format is appended to the
 * information refering to the location in the code the macro is expanded at.
 *
 * @param ... Arguments to be substituted into the format string
 */
/**
 * @def IFDEBUG()
 *
 * @brief Insert code that is only compiled if DEBUG is defined.
 *
 * @param x Code to insert
 */

#ifdef DEBUG
#  define PRINT_DEBUG(fmt, ...) \
	fprintf(stderr, "DEBUG: %s(), %s:%d: " fmt,			\
		__func__, __FILE__, __LINE__, ##__VA_ARGS__)
#  define IFDEBUG(x) x
#else
// Does nothing
#  define PRINT_DEBUG(fmt, ...)
// Does nothing
#  define IFDEBUG(x)
#endif

