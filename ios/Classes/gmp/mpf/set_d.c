/* mpf_set_d -- Assign a float from a double.

Copyright 1993-1996, 2001, 2003, 2004 Free Software Foundation, Inc.
  */

#include "../config.h"

#if HAVE_FLOAT_H
#include <float.h>  /* for DBL_MAX */
#endif

#include "../gmp.h"
#include "../gmp-impl.h"

void
mpf_set_d (mpf_ptr r, double d)
{
  int negative;

  DOUBLE_NAN_INF_ACTION (d,
                         __gmp_invalid_operation (),
                         __gmp_invalid_operation ());

  if (UNLIKELY (d == 0))
    {
      SIZ(r) = 0;
      EXP(r) = 0;
      return;
    }
  negative = d < 0;
  d = ABS (d);

  SIZ(r) = negative ? -LIMBS_PER_DOUBLE : LIMBS_PER_DOUBLE;
  EXP(r) = __gmp_extract_double (PTR(r), d);
}
