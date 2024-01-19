#include "memory_pool.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "deque.h"

/**
 * @brief Holds a block of memory that can be used for allocations
 *
 * The advantage of a MemoryPool is that we can free the entire block at once.
 * This can help prevent memory leaks in code that creates structures from the
 * bottom up (such as the parser) and can fail leaving the top most portion of
 * the structure uninitialized.
 *
 * @note A single memory pool can run out of space fast or will take up quite a
 * bit of unnecessary space in memory. To combat this problem the @a
 * MemoryPoolDeque allows the creation of a larger additional MemoryPool to
 * handle the allocation.
 */
typedef struct MemoryPool {
  void* pool;
  size_t size;
  void* next;
} MemoryPool;

IMPLEMENT_DEQUE_STRUCT(MemoryPoolDeque, MemoryPool);
IMPLEMENT_DEQUE(MemoryPoolDeque, MemoryPool);

static MemoryPoolDeque pool_deq = { NULL, 0, 0, 0, NULL };

// Creates a single memory pool an returns a copy If the `size` parameter is
// zero then this function will not allocate any space for later MemoryPool
// allocations.
static MemoryPool __initialize_memory_pool(size_t size) {
  void* mem;

  if (size == 0) {
    mem = NULL;
  }
  else {
    if ((mem = malloc(size)) == NULL)
      size = 0;
  }

  return (MemoryPool) {
    mem,
    size,
    mem
  };
}

// Last ditch effort to allocate some memory
static MemoryPool __low_memory_initialize_memory_pool(size_t required_size,
                                                      size_t failed_requested_size) {
  while(true) {
    if (failed_requested_size <= required_size) {
      fprintf(stderr, "ERROR: Unable to allocate more memory to the memory pool.\n");
      exit(-1);
    }

    failed_requested_size >>= 1;

    if (failed_requested_size < required_size)
      failed_requested_size = required_size;

    // Attempt to allocate smaller size space
    MemoryPool ret = __initialize_memory_pool(failed_requested_size);

    if (ret.pool != NULL)
      return ret;
  }
}

static void __destroy_memory_pool(MemoryPool mp) {
  free(mp.pool);
}

void initialize_memory_pool(size_t size) {
  if (size == 0)
    size = 1;

  pool_deq = new_destructable_MemoryPoolDeque(10, __destroy_memory_pool);

  MemoryPool pool = __initialize_memory_pool(size);

  if (pool.pool == NULL)
    // We are running low on memory. Try smaller allocations or exit Quash
    pool = __low_memory_initialize_memory_pool(1, size);

  push_back_MemoryPoolDeque(&pool_deq, pool);
}

void* memory_pool_alloc(size_t size) {
  assert(!is_empty_MemoryPoolDeque(&pool_deq));

  MemoryPool pool = peek_back_MemoryPoolDeque(&pool_deq);
  size_t init_size = peek_front_MemoryPoolDeque(&pool_deq).size;

  assert(pool.pool != NULL);
  assert(pool.size != 0);
  assert(pool.next != NULL);

  while (pool.next - pool.pool + size > pool.size) {
    // There is not enough room in the current memory pool to fit the
    // allocation. Create a new memory pool large enough to hold it. 
    size_t length_pool_deq = length_MemoryPoolDeque(&pool_deq);
    size_t new_pool_size = init_size * (2 << (length_pool_deq - 1));

    if (new_pool_size < size) {
      // The next pool size selected wasn't enough space. We have to have to add
      // something onto the deque since the new pool size is dependent on the
      // size of the deque.
      pool = __initialize_memory_pool(0);
    }
    else {
      // Create a MemoryPool with the correct amount of space
      pool = __initialize_memory_pool(new_pool_size);

      if (pool.pool == NULL)
        // We are running low on memory. Try smaller allocations or exit Quash
        pool = __low_memory_initialize_memory_pool(size, new_pool_size);
    }

    push_back_MemoryPoolDeque(&pool_deq, pool);
  }

  assert(pool.next == peek_back_MemoryPoolDeque(&pool_deq).next);
  void* ret = pool.next;
  pool.next += size;

  // Update record
  update_back_MemoryPoolDeque(&pool_deq, pool);

  return ret;
}

// Free all memory contained in the MemoryPoolDeque
void destroy_memory_pool() {
  destroy_MemoryPoolDeque(&pool_deq);
}

// Simple replacement for strdup() that uses the memory pool rather than malloc
char* memory_pool_strdup(const char* str) {
  assert(str != NULL);

  size_t len = strlen(str) + 1;
  char* ret = memory_pool_alloc(len);

  strcpy(ret, str);

  return ret;
}
