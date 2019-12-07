/* mpz_set_d(integer, val) -- Assign INTEGER with a double value VAL.

Copyright 1995, 1996, 2000-2003, 2006 Free Software Foundation, Inc.
  */

#include "../config.h"

#if HAVE_FLOAT_H
#include <float.h>  /* for DBL_MAX */
#endif

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"


/* We used to have a special case for d < MP_BASE_AS_DOUBLE, just casting
   double -> limb.  Unfortunately gcc 3.3 on powerpc970-apple-darwin6.8.5
   got this wrong.  (It assumed __fixunsdfdi returned its result in a single
   64-bit register, where instead that function followed the calling
   conventions and gave the result in two parts r3 and r4.)  Hence the use
   of __gmp_extract_double in all cases.  */

void
mpz_set_d (mpz_ptr r, double d)
{
  int negative;
  mp_limb_t tp[LIMBS_PER_DOUBLE];
  mp_ptr rp;
  mp_size_t rn;

  DOUBLE_NAN_INF_ACTION (d,
			 __gmp_invalid_operation (),
			 __gmp_invalid_operation ());

  negative = d < 0;
  d = ABS (d);

  rn = __gmp_extract_double (tp, d);

  if (ALLOC(r) < rn)
    _mpz_realloc (r, rn);

  if (rn <= 0)
    rn = 0;

  rp = PTR (r);

  switch (rn)
    {
    default:
      MPN_ZERO (rp, rn - LIMBS_PER_DOUBLE);
      rp += rn - LIMBS_PER_DOUBLE;
      /* fall through */
#if LIMBS_PER_DOUBLE == 2
    case 2:
      rp[1] = tp[1], rp[0] = tp[0];
      break;
    case 1:
      rp[0] = tp[1];
      break;
#endif
#if LIMBS_PER_DOUBLE == 3
    case 3:
      rp[2] = tp[2], rp[1] = tp[1], rp[0] = tp[0];
      break;
    case 2:
      rp[1] = tp[2], rp[0] = tp[1];
      break;
    case 1:
      rp[0] = tp[2];
      break;
#endif
#if LIMBS_PER_DOUBLE == 4
    case 4:
      rp[3] = tp[3], rp[2] = tp[2], rp[1] = tp[1], rp[0] = tp[0];
      break;
    case 3:
      rp[2] = tp[3], rp[1] = tp[2], rp[0] = tp[1];
      break;
    case 2:
      rp[1] = tp[3], rp[0] = tp[2];
      break;
    case 1:
      rp[0] = tp[3];
      break;
#endif
    case 0:
      break;
    }

  SIZ(r) = negative ? -rn : rn;
}
