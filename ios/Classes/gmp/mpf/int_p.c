/* mpf_integer_p -- test whether an mpf is an integer */

/*
Copyright 2001, 2002, 2014-2015 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"


int
mpf_integer_p (mpf_srcptr f) __GMP_NOTHROW
{
  mp_srcptr fp;
  mp_exp_t exp;
  mp_size_t size;

  size = SIZ (f);
  exp = EXP (f);
  if (exp <= 0)
    return (size == 0);  /* zero is an integer,
			    others have only fraction limbs */
  size = ABS (size);

  /* Ignore zeroes at the low end of F.  */
  for (fp = PTR (f); *fp == 0; ++fp)
    --size;

  /* no fraction limbs */
  return size <= exp;
}
