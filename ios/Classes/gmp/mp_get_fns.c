/* mp_get_memory_functions -- Get the allocate, reallocate, and free functions.

Copyright 2002 Free Software Foundation, Inc.
  */

#include <stdio.h>  /* for NULL */
#include "gmp.h"
#include "gmp-impl.h"
#include "memory.c"

void
mp_get_memory_functions (void *(**alloc_func) (size_t),
			 void *(**realloc_func) (void *, size_t, size_t),
			 void (**free_func) (void *, size_t)) __GMP_NOTHROW
{
  if (alloc_func != NULL)
    *alloc_func = __gmp_allocate_func;

  if (realloc_func != NULL)
    *realloc_func = __gmp_reallocate_func;

  if (free_func != NULL)
    *free_func = __gmp_free_func;
}
