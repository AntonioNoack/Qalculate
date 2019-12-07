/* mpf_cmp_d -- compare mpf and double.

Copyright 2001, 2003 Free Software Foundation, Inc.
  */

#include "../config.h"

#if HAVE_FLOAT_H
#include <float.h>  /* for DBL_MAX */
#endif

#include "../gmp.h"
#include "../gmp-impl.h"

int
mpf_cmp_d (mpf_srcptr f, double d)
{
  mp_limb_t  darray[LIMBS_PER_DOUBLE];
  mpf_t      df;

  /* d=NaN has no sensible return value, so raise an exception.
     d=Inf or -Inf is always bigger than z.  */
  DOUBLE_NAN_INF_ACTION (d,
                         __gmp_invalid_operation (),
                         return (d < 0.0 ? 1 : -1));

  if (d == 0.0)
    return SIZ(f);

  PTR(df) = darray;
  SIZ(df) = (d >= 0.0 ? LIMBS_PER_DOUBLE : -LIMBS_PER_DOUBLE);
  EXP(df) = __gmp_extract_double (darray, ABS(d));

  return mpf_cmp (f, df);
}
