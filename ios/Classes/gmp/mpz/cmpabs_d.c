/* mpz_cmpabs_d -- compare absolute values of mpz and double.

Copyright 2001-2003, 2012 Free Software Foundation, Inc.
  */

#include "../config.h"

#if HAVE_FLOAT_H
#include <float.h>  /* for DBL_MAX */
#endif

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"


#define RETURN_CMP(zl, dl)              \
  do {                                  \
    zlimb = (zl);                       \
    dlimb = (dl);                       \
    if (zlimb != dlimb)                 \
      return (zlimb >= dlimb ? 1 : -1); \
  } while (0)

#define RETURN_NONZERO(ptr, size, val)          \
  do {                                          \
    mp_size_t __i;                              \
    for (__i = (size)-1; __i >= 0; __i--)       \
      if ((ptr)[__i] != 0)                      \
        return val;                             \
    return 0;                                   \
  } while (0)


int
mpz_cmpabs_d (mpz_srcptr z, double d)
{
  mp_limb_t  darray[LIMBS_PER_DOUBLE], zlimb, dlimb;
  mp_srcptr  zp;
  mp_size_t  zsize;
  int        dexp;

  /* d=NaN is an invalid operation, there's no sensible return value.
     d=Inf or -Inf is always bigger than z.  */
  DOUBLE_NAN_INF_ACTION (d, __gmp_invalid_operation (), return -1);

  /* 1. Check for either operand zero. */
  zsize = SIZ(z);
  if (d == 0.0)
    return (zsize != 0);
  if (zsize == 0)
    return -1; /* d != 0 */

  /* 2. Ignore signs. */
  zsize = ABS(zsize);
  d = ABS(d);

  /* 3. Small d, knowing abs(z) >= 1. */
  if (d < 1.0)
    return 1;

  dexp = __gmp_extract_double (darray, d);
  ASSERT (dexp >= 1);

  /* 4. Check for different high limb positions. */
  if (zsize != dexp)
    return (zsize >= dexp ? 1 : -1);

  /* 5. Limb data. */
  zp = PTR(z);

#if LIMBS_PER_DOUBLE == 2
  RETURN_CMP (zp[zsize-1], darray[1]);
  if (zsize == 1)
    return (darray[0] != 0 ? -1 : 0);

  RETURN_CMP (zp[zsize-2], darray[0]);
  RETURN_NONZERO (zp, zsize-2, 1);
#endif

#if LIMBS_PER_DOUBLE == 3
  RETURN_CMP (zp[zsize-1], darray[2]);
  if (zsize == 1)
    return ((darray[0] | darray[1]) != 0 ? -1 : 0);

  RETURN_CMP (zp[zsize-2], darray[1]);
  if (zsize == 2)
    return (darray[0] != 0 ? -1 : 0);

  RETURN_CMP (zp[zsize-3], darray[0]);
  RETURN_NONZERO (zp, zsize-3, 1);
#endif

#if LIMBS_PER_DOUBLE >= 4
  {
    int i;
    for (i = 1; i <= LIMBS_PER_DOUBLE; i++)
      {
	RETURN_CMP (zp[zsize-i], darray[LIMBS_PER_DOUBLE-i]);
	if (i >= zsize)
	  RETURN_NONZERO (darray, LIMBS_PER_DOUBLE-i, -1);
      }
    RETURN_NONZERO (zp, zsize-LIMBS_PER_DOUBLE, 1);
  }
#endif
}
