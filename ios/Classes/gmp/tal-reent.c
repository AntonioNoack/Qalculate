/* TMP_ALLOC routines using malloc in a reentrant fashion.

Copyright 2000, 2001 Free Software Foundation, Inc.
  */

#include <stdio.h>
#include "gmp.h"
#include "gmp-impl.h"
#include "memory.h"


/* Each TMP_ALLOC uses __gmp_allocate_func to get a block of memory of the
   size requested, plus a header at the start which is used to hold the
   blocks on a linked list in the marker variable, ready for TMP_FREE to
   release.

   Callers should try to do multiple allocs with one call, in the style of
   TMP_ALLOC_LIMBS_2 if it's easy to arrange, since that will keep down the
   number of separate malloc calls.

   Enhancements:

   Could inline both TMP_ALLOC and TMP_FREE, though TMP_ALLOC would need the
   compiler to have "inline" since it returns a value.  The calls to malloc
   will be slow though, so it hardly seems worth worrying about one extra
   level of function call.  */


#define HSIZ   ROUND_UP_MULTIPLE (sizeof (struct tmp_reentrant_t), __TMP_ALIGN)

void *
__gmp_tmp_reentrant_alloc (struct tmp_reentrant_t **markp, size_t size)
{
  char    *p;
  size_t  total_size;

#define P   ((struct tmp_reentrant_t *) p)

  total_size = size + HSIZ;
  p = (char *) (*__gmp_allocate_func) (total_size);
  P->size = total_size;
  P->next = *markp;
  *markp = P;
  return p + HSIZ;
}

void
__gmp_tmp_reentrant_free (struct tmp_reentrant_t *mark)
{
  struct tmp_reentrant_t  *next;

  while (mark != NULL)
    {
      next = mark->next;
      (*__gmp_free_func) ((char *) mark, mark->size);
      mark = next;
    }
}

#undef P