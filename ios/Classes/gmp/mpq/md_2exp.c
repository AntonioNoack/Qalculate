/* mpq_mul_2exp, mpq_div_2exp - multiply or divide by 2^N */

/*
Copyright 2000, 2002, 2012 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"
#include "../longlong.h"


/* The multiplier/divisor "n", representing 2^n, is applied by right shifting
   "r" until it's odd (if it isn't already), and left shifting "l" for the
   rest. */

static void
mord_2exp (mpz_ptr ldst, mpz_ptr rdst, mpz_srcptr lsrc, mpz_srcptr rsrc,
           mp_bitcnt_t n)
{
  mp_size_t  rsrc_size = SIZ(rsrc);
  mp_size_t  len = ABS (rsrc_size);
  mp_ptr     rsrc_ptr = PTR(rsrc);
  mp_ptr     p, rdst_ptr;
  mp_limb_t  plow;

  p = rsrc_ptr;
  plow = *p;
  while (n >= GMP_NUMB_BITS && plow == 0)
    {
      n -= GMP_NUMB_BITS;
      p++;
      plow = *p;
    }

  /* no realloc here if rsrc==rdst, so p and rsrc_ptr remain valid */
  len -= (p - rsrc_ptr);
  rdst_ptr = MPZ_REALLOC (rdst, len);

  if ((plow & 1) || n == 0)
    {
      /* need INCR when src==dst */
      if (p != rdst_ptr)
        MPN_COPY_INCR (rdst_ptr, p, len);
    }
  else
    {
      unsigned long  shift;
      if (plow == 0)
        shift = n;
      else
        {
          count_trailing_zeros (shift, plow);
          shift = MIN (shift, n);
        }
      mpn_rshift (rdst_ptr, p, len, shift);
      len -= (rdst_ptr[len-1] == 0);
      n -= shift;
    }
  SIZ(rdst) = (rsrc_size >= 0) ? len : -len;

  if (n)
    mpz_mul_2exp (ldst, lsrc, n);
  else if (ldst != lsrc)
    mpz_set (ldst, lsrc);
}


void
mpq_mul_2exp (mpq_ptr dst, mpq_srcptr src, mp_bitcnt_t n)
{
  mord_2exp (NUM(dst), DEN(dst), NUM(src), DEN(src), n);
}

void
mpq_div_2exp (mpq_ptr dst, mpq_srcptr src, mp_bitcnt_t n)
{
  if (SIZ(NUM(src)) == 0)
    {
      SIZ(NUM(dst)) = 0;
      SIZ(DEN(dst)) = 1;
      PTR(DEN(dst))[0] = 1;
      return;
    }

  mord_2exp (DEN(dst), NUM(dst), DEN(src), NUM(src), n);
}
