/* _mpq_cmp_si -- compare mpq and long/ulong fraction.

Copyright 2001, 2013, 2014 Free Software Foundation, Inc.
  */

#include "../gmp.h"
#include "../gmp-impl.h"


/* Something like mpq_cmpabs_ui would be more useful for the neg/neg case,
   and perhaps a version accepting a parameter to reverse the test, to make
   it a tail call here.  */

int
_mpq_cmp_si (mpq_srcptr q, long n, unsigned long d)
{
  /* need canonical sign to get right result */
  ASSERT (SIZ(DEN(q)) > 0);

  if (n >= 0)
    return _mpq_cmp_ui (q, n, d);
  if (SIZ(NUM(q)) >= 0)
    {
      return 1;                                /* >=0 cmp <0 */
    }
  else
    {
      mpq_t  qabs;
      SIZ(NUM(qabs)) = -SIZ(NUM(q));
      PTR(NUM(qabs)) = PTR(NUM(q));
      SIZ(DEN(qabs)) = SIZ(DEN(q));
      PTR(DEN(qabs)) = PTR(DEN(q));

      return - _mpq_cmp_ui (qabs, NEG_CAST (unsigned long, n), d);    /* <0 cmp <0 */
    }
}
