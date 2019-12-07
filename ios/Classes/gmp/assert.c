/* GMP assertion failure handler.

   THE FUNCTIONS IN THIS FILE ARE FOR INTERNAL USE ONLY.  THEY'RE ALMOST
   CERTAIN TO BE SUBJECT TO INCOMPATIBLE CHANGES OR DISAPPEAR COMPLETELY IN
   FUTURE GNU MP RELEASES.

Copyright 2000, 2001 Free Software Foundation, Inc.
  */

  #ifndef ASSERT_C
  #define ASSERT_C

#include <stdio.h>
#include <stdlib.h>
#include "gmp.h"
#include "gmp-impl.h"


void
__gmp_assert_header (const char *filename, int linenum)
{
  if (filename != NULL && filename[0] != '\0')
    {
      fprintf (stderr, "%s:", filename);
      if (linenum != -1)
        fprintf (stderr, "%d: ", linenum);
    }
}

void
__gmp_assert_fail (const char *filename, int linenum,
                   const char *expr)
{
  __gmp_assert_header (filename, linenum);
  fprintf (stderr, "GNU MP assertion failed: %s\n", expr);
  abort();
}

#include "mpn/add_n.c"
#include "mpn/sub_n.c"
#include "mpn/com.c"
#include "memory.c"

#endif