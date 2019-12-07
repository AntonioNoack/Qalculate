//
// Created by antonio on 11/29/19.
//

#ifndef CALCULATE_GMP_C_H
#define CALCULATE_GMP_C_H

#if defined (__GMP_EXTERN_INLINE) && ! defined (__GMP_FORCE_mpz_abs)
__GMP_EXTERN_INLINE void
mpz_abs (mpz_ptr __gmp_w, mpz_srcptr __gmp_u)
{
  if (__gmp_w != __gmp_u)
    mpz_set (__gmp_w, __gmp_u);
  __gmp_w->_mp_size = __GMP_ABS (__gmp_w->_mp_size);
}
#endif

#if defined (__GMP_EXTERN_INLINE) || defined (__GMP_FORCE_mpz_fits_uint_p)
#if ! defined (__GMP_FORCE_mpz_fits_uint_p)
__GMP_EXTERN_INLINE
#endif
int
mpz_fits_uint_p (mpz_srcptr __gmp_z) __GMP_NOTHROW
{
  __GMPZ_FITS_UTYPE_P (__gmp_z, UINT_MAX);
}
#endif

#if defined (__GMP_EXTERN_INLINE) || defined (__GMP_FORCE_mpz_fits_ulong_p)
#if ! defined (__GMP_FORCE_mpz_fits_ulong_p)
__GMP_EXTERN_INLINE
#endif
int
mpz_fits_ulong_p (mpz_srcptr __gmp_z) __GMP_NOTHROW
{
  __GMPZ_FITS_UTYPE_P (__gmp_z, ULONG_MAX);
}
#endif


#if defined (__GMP_EXTERN_INLINE) || defined (__GMP_FORCE_mpz_fits_ushort_p)
#if ! defined (__GMP_FORCE_mpz_fits_ushort_p)
__GMP_EXTERN_INLINE
#endif
int
mpz_fits_ushort_p (mpz_srcptr __gmp_z) __GMP_NOTHROW
{
  __GMPZ_FITS_UTYPE_P (__gmp_z, USHRT_MAX);
}
#endif


#if defined (__GMP_EXTERN_INLINE) || defined (__GMP_FORCE_mpz_get_ui)
// #if ! defined (__GMP_FORCE_mpz_get_ui)
// __GMP_EXTERN_INLINE
// #endif
unsigned long mpz_get_ui(mpz_srcptr __gmp_z) __GMP_NOTHROW {

  mp_ptr __gmp_p = __gmp_z->_mp_d;
  mp_size_t __gmp_n = __gmp_z->_mp_size;
  mp_limb_t __gmp_l = __gmp_p[0];
  /* This is a "#if" rather than a plain "if" so as to avoid gcc warnings
     about "<< GMP_NUMB_BITS" exceeding the type size, and to avoid Borland
     C++ 6.0 warnings about condition always true for something like
     "ULONG_MAX < GMP_NUMB_MASK".  */
#if GMP_NAIL_BITS == 0 || defined (_LONG_LONG_LIMB)
  /* limb==long and no nails, or limb==longlong, one limb is enough */
  return (__gmp_n != 0 ? __gmp_l : 0);
#else
  /* limb==long and nails, need two limbs when available */
  __gmp_n = __GMP_ABS (__gmp_n);
  if (__gmp_n <= 1)
    return (__gmp_n != 0 ? __gmp_l : 0);
  else
    return __gmp_l + (__gmp_p[1] << GMP_NUMB_BITS);
#endif
}
#endif


#if defined (__GMP_EXTERN_INLINE) || defined (__GMP_FORCE_mpz_getlimbn)
#if ! defined (__GMP_FORCE_mpz_getlimbn)
__GMP_EXTERN_INLINE
#endif
mp_limb_t
mpz_getlimbn (mpz_srcptr __gmp_z, mp_size_t __gmp_n) __GMP_NOTHROW
{
  mp_limb_t  __gmp_result = 0;
  if (__GMP_LIKELY (__gmp_n >= 0 && __gmp_n < __GMP_ABS (__gmp_z->_mp_size)))
    __gmp_result = __gmp_z->_mp_d[__gmp_n];
  return __gmp_result;
}
#endif


#if defined (__GMP_EXTERN_INLINE) && ! defined (__GMP_FORCE_mpz_neg)
__GMP_EXTERN_INLINE void
mpz_neg (mpz_ptr __gmp_w, mpz_srcptr __gmp_u)
{
  if (__gmp_w != __gmp_u)
    mpz_set (__gmp_w, __gmp_u);
  __gmp_w->_mp_size = - __gmp_w->_mp_size;
}
#endif


/*#if defined (__GMP_EXTERN_INLINE) || defined (__GMP_FORCE_mpz_perfect_square_p)
#if ! defined (__GMP_FORCE_mpz_perfect_square_p)
__GMP_EXTERN_INLINE
#endif
int
mpz_perfect_square_p (mpz_srcptr __gmp_a)
{
  mp_size_t __gmp_asize;
  int       __gmp_result;

  __gmp_asize = __gmp_a->_mp_size;
  __gmp_result = (__gmp_asize >= 0);  // zero is a square, negatives are not
  if (__GMP_LIKELY (__gmp_asize > 0))
    __gmp_result = mpn_perfect_square_p (__gmp_a->_mp_d, __gmp_asize);
  return __gmp_result;
}
#endif*/

int
mpz_perfect_square_p (mpz_srcptr __gmp_a)
{
  mp_size_t __gmp_asize;
  int       __gmp_result;

  __gmp_asize = __gmp_a->_mp_size;
  __gmp_result = (__gmp_asize >= 0);  /* zero is a square, negatives are not */
  if (__GMP_LIKELY (__gmp_asize > 0))
    __gmp_result = mpn_perfect_square_p (__gmp_a->_mp_d, __gmp_asize);
  return __gmp_result;
}



#if defined (__GMP_EXTERN_INLINE) || defined (__GMP_FORCE_mpz_popcount)
#if ! defined (__GMP_FORCE_mpz_popcount)
__GMP_EXTERN_INLINE
#endif
mp_bitcnt_t
mpz_popcount (mpz_srcptr __gmp_u) __GMP_NOTHROW
{
  mp_size_t      __gmp_usize;
  mp_bitcnt_t    __gmp_result;

  __gmp_usize = __gmp_u->_mp_size;
  __gmp_result = (__gmp_usize < 0 ? ULONG_MAX : 0);
  if (__GMP_LIKELY (__gmp_usize > 0))
    __gmp_result =  mpn_popcount (__gmp_u->_mp_d, __gmp_usize);
  return __gmp_result;
}
#endif


#if defined (__GMP_EXTERN_INLINE) || defined (__GMP_FORCE_mpz_set_q)
#if ! defined (__GMP_FORCE_mpz_set_q)
__GMP_EXTERN_INLINE
#endif
void
mpz_set_q (mpz_ptr __gmp_w, mpq_srcptr __gmp_u)
{
  mpz_tdiv_q (__gmp_w, mpq_numref (__gmp_u), mpq_denref (__gmp_u));
}
#endif


/*#if defined (__GMP_EXTERN_INLINE) || defined (__GMP_FORCE_mpz_size)
#if ! defined (__GMP_FORCE_mpz_size)
__GMP_EXTERN_INLINE
#endif

#endif*/


#if defined (__GMP_EXTERN_INLINE) && ! defined (__GMP_FORCE_mpq_abs)
__GMP_EXTERN_INLINE void
mpq_abs (mpq_ptr __gmp_w, mpq_srcptr __gmp_u)
{
  if (__gmp_w != __gmp_u)
    mpq_set (__gmp_w, __gmp_u);
  __gmp_w->_mp_num._mp_size = __GMP_ABS (__gmp_w->_mp_num._mp_size);
}
#endif


#if defined (__GMP_EXTERN_INLINE) && ! defined (__GMP_FORCE_mpq_neg)
__GMP_EXTERN_INLINE void
mpq_neg (mpq_ptr __gmp_w, mpq_srcptr __gmp_u)
{
  if (__gmp_w != __gmp_u)
    mpq_set (__gmp_w, __gmp_u);
  __gmp_w->_mp_num._mp_size = - __gmp_w->_mp_num._mp_size;
}
#endif

#endif //CALCULATE_GMP_C_H
