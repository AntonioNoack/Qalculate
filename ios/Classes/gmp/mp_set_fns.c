/* mp_set_memory_functions -- Set the allocate, reallocate, and free functions
   for use by the mp package.

Copyright 1991, 1993, 1994, 2000, 2001 Free Software Foundation, Inc.
  */

#include "gmp.h"
#include "gmp-impl.h"
#include "memory.h"

void
mp_set_memory_functions (void *(*alloc_func) (size_t),
			 void *(*realloc_func) (void *, size_t, size_t),
			 void (*free_func) (void *, size_t)) __GMP_NOTHROW
{
  if (alloc_func == 0)
    alloc_func = __gmp_default_allocate;
  if (realloc_func == 0)
    realloc_func = __gmp_default_reallocate;
  if (free_func == 0)
    free_func = __gmp_default_free;

  __gmp_allocate_func = alloc_func;
  __gmp_reallocate_func = realloc_func;
  __gmp_free_func = free_func;
}
