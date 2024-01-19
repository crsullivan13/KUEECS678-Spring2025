/**
 * @file memory_pool.h
 *
 * @brief An abstraction of malloc that allows for all allocations in the pool
 * to be free'd with a single call to destroy_memory_pool(). Allocations to the
 * memory pool should NOT be manually free'd with a call to free().
 *
 * @warning The memory pool allocations are not thread safe
 */

#ifndef SRC_PARSING_MEMORY_POOL_H
#define SRC_PARSING_MEMORY_POOL_H

#include <stdlib.h>

/**
 * @brief Allocate the memory pool
 *
 * @param size The initial size of the memory pool. If this value is zero then a
 * default size of one is used.
 */
void initialize_memory_pool(size_t size);

/**
 * @brief Reserve some space in the memory pool and returns a unique address
 * that can be written to and read from. This can be thought of exactly like
 * malloc() without the requirement of calling free() directly on the returned
 * pointer.
 *
 * @param size Size in bytes of the requested reserved space
 *
 * @return A pointer to a unique array of size bytes
 */
void* memory_pool_alloc(size_t size);

/**
 * @brief Free all memory allocated in the memory pool
 */
void destroy_memory_pool();

/**
 * @brief A version of strdup() that allocates the duplicate to the memory pool
 * rather than with malloc directly
 *
 * @param str Pointer to the string to duplicate
 *
 * @return A copy of str allocated in the memory pool
 */
char* memory_pool_strdup(const char* str);

#endif
