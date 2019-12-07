/* mpn_sec_sqr.

   Contributed to the GNU project by Torbjörn Granlund.

Copyright 2013, 2014 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"

#if ! HAVE_NATIVE_mpn_sqr_basecase
/* The limit of the generic code is SQR_TOOM2_THRESHOLD.  */
#define SQR_BASECASE_LIM  SQR_TOOM2_THRESHOLD
#endif

#if HAVE_NATIVE_mpn_sqr_basecase
#ifdef TUNE_SQR_TOOM2_MAX
/* We slightly abuse TUNE_SQR_TOOM2_MAX here.  If it is set for an assembly
   mpn_sqr_basecase, it comes from SQR_TOOM2_THRESHOLD_MAX in the assembly
   file.  An assembly mpn_sqr_basecase that does not define it should allow
   any size.  */
#define SQR_BASECASE_LIM  SQR_TOOM2_THRESHOLD
#endif
#endif

#ifdef WANT_FAT_BINARY
/* For fat builds, we use SQR_TOOM2_THRESHOLD which will expand to a read from
   __gmpn_cpuvec.  Perhaps any possible sqr_basecase.asm allow any size, and we
   limit the use unnecessarily.  We cannot tell, so play it safe.  FIXME.  */
#define SQR_BASECASE_LIM  SQR_TOOM2_THRESHOLD
#endif

void
mpn_sec_sqr (mp_ptr rp,
	     mp_srcptr ap, mp_size_t an,
	     mp_ptr tp)
{
#ifndef SQR_BASECASE_LIM
/* If SQR_BASECASE_LIM is now not defined, use mpn_sqr_basecase for any operand
   size.  */
  mpn_sqr_basecase (rp, ap, an);
#else
/* Else use mpn_sqr_basecase for its allowed sizes, else mpn_mul_basecase.  */
  mpn_mul_basecase (rp, ap, an, ap, an);
#endif
}

mp_size_t
mpn_sec_sqr_itch (mp_size_t an)
{
  return 0;
}
