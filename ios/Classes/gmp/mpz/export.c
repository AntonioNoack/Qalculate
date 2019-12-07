/* mpz_export -- create word data from mpz.

Copyright 2002, 2003, 2012 Free Software Foundation, Inc.
  */

#include <stdio.h>  /* for NULL */
#include "../gmp.h"
#include "../gmp-impl.h"
#include "../gmp-impl.c"
#include "../longlong.h"


#if HAVE_LIMB_BIG_ENDIAN
#define HOST_ENDIAN     1
#endif
#if HAVE_LIMB_LITTLE_ENDIAN
#define HOST_ENDIAN     (-1)
#endif
#ifndef HOST_ENDIAN
static const mp_limb_t  endian_test = (CNST_LIMB(1) << (GMP_LIMB_BITS-7)) - 1;
#define HOST_ENDIAN     (* (signed char *) &endian_test)
#endif

void *
mpz_export (void *data, size_t *countp, int order,
	    size_t size, int endian, size_t nail, mpz_srcptr z)
{
  mp_size_t      zsize;
  mp_srcptr      zp;
  size_t         count, dummy;
  unsigned long  numb;
  unsigned       align;

  ASSERT (order == 1 || order == -1);
  ASSERT (endian == 1 || endian == 0 || endian == -1);
  ASSERT (nail <= 8*size);
  ASSERT (nail <  8*size || SIZ(z) == 0); /* nail < 8*size+(SIZ(z)==0) */

  if (countp == NULL)
    countp = &dummy;

  zsize = SIZ(z);
  if (zsize == 0)
    {
      *countp = 0;
      return data;
    }

  zsize = ABS (zsize);
  zp = PTR(z);
  numb = 8*size - nail;
  MPN_SIZEINBASE_2EXP (count, zp, zsize, numb);
  *countp = count;

  if (data == NULL)
    data = (*__gmp_allocate_func) (count*size);

  if (endian == 0)
    endian = HOST_ENDIAN;

  align = ((char *) data - (char *) NULL) % sizeof (mp_limb_t);

  if (nail == GMP_NAIL_BITS)
    {
      if (size == sizeof (mp_limb_t) && align == 0)
	{
	  if (order == -1 && endian == HOST_ENDIAN)
	    {
	      MPN_COPY ((mp_ptr) data, zp, (mp_size_t) count);
	      return data;
	    }
	  if (order == 1 && endian == HOST_ENDIAN)
	    {
	      MPN_REVERSE ((mp_ptr) data, zp, (mp_size_t) count);
	      return data;
	    }

	  if (order == -1 && endian == -HOST_ENDIAN)
	    {
	      MPN_BSWAP ((mp_ptr) data, zp, (mp_size_t) count);
	      return data;
	    }
	  if (order == 1 && endian == -HOST_ENDIAN)
	    {
	      MPN_BSWAP_REVERSE ((mp_ptr) data, zp, (mp_size_t) count);
	      return data;
	    }
	}
    }

  {
    mp_limb_t      limb, wbitsmask;
    size_t         i, numb;
    mp_size_t      j, wbytes, woffset;
    unsigned char  *dp;
    int            lbits, wbits;
    mp_srcptr      zend;

    numb = size * 8 - nail;

    /* whole bytes per word */
    wbytes = numb / 8;

    /* possible partial byte */
    wbits = numb % 8;
    wbitsmask = (CNST_LIMB(1) << wbits) - 1;

    /* offset to get to the next word */
    woffset = (endian >= 0 ? size : - (mp_size_t) size)
      + (order < 0 ? size : - (mp_size_t) size);

    /* least significant byte */
    dp = (unsigned char *) data
      + (order >= 0 ? (count-1)*size : 0) + (endian >= 0 ? size-1 : 0);

#define EXTRACT(N, MASK)                                \
    do {                                                \
      if (lbits >= (N))                                 \
        {                                               \
          *dp = limb MASK;                              \
          limb >>= (N);                                 \
          lbits -= (N);                                 \
        }                                               \
      else                                              \
        {                                               \
          mp_limb_t  newlimb;                           \
          newlimb = (zp == zend ? 0 : *zp++);           \
          *dp = (limb | (newlimb << lbits)) MASK;       \
          limb = newlimb >> ((N)-lbits);                \
          lbits += GMP_NUMB_BITS - (N);                 \
        }                                               \
    } while (0)

    zend = zp + zsize;
    lbits = 0;
    limb = 0;
    for (i = 0; i < count; i++)
      {
	for (j = 0; j < wbytes; j++)
	  {
	    EXTRACT (8, + 0);
	    dp -= endian;
	  }
	if (wbits != 0)
	  {
	    EXTRACT (wbits, & wbitsmask);
	    dp -= endian;
	    j++;
	  }
	for ( ; j < size; j++)
	  {
	    *dp = '\0';
	    dp -= endian;
	  }
	dp += woffset;
      }

    ASSERT (zp == PTR(z) + ABSIZ(z));

    /* low byte of word after most significant */
    ASSERT (dp == (unsigned char *) data
	    + (order < 0 ? count*size : - (mp_size_t) size)
	    + (endian >= 0 ? (mp_size_t) size - 1 : 0));
  }
  return data;
}
