//
// Created by antonio on 11/27/19.
//

#include <stdio.h>



#ifndef CALCULATE_HELPER_H
#define CALCULATE_HELPER_H

struct tmp_reentrant_t {
  struct tmp_reentrant_t  *next;
  size_t		  size;	  /* bytes, including header */
};

struct tmp_stack;

struct tmp_marker2 {
  struct tmp_stack *which_chunk;
  void *alloc_point;
};

struct tmp_stack
{
  void *end;
  void *alloc_point;
  struct tmp_stack *prev;
};
typedef struct tmp_stack tmp_stack;


struct tmp_marker
{
  struct tmp_stack *which_chunk;
  void *alloc_point;
};

struct tmp_debug_entry_t;
struct tmp_debug_t {
  struct tmp_debug_entry_t  *list;
  const char                *file;
  int                       line;
};
struct tmp_debug_entry_t {
  struct tmp_debug_entry_t  *next;
  void                      *block;
  size_t                    size;
};

#endif //CALCULATE_HELPER_H



#ifdef WANT_STR_BASES
#ifndef STR_BASES__XXX
#define STR_BASES__XXX

/* Structure for conversion between internal binary format and strings.  */
struct bases {
  /* Number of digits in the conversion base that always fits in an mp_limb_t.
     For example, for base 10 on a machine where an mp_limb_t has 32 bits this
     is 9, since 10**9 is the largest number that fits into an mp_limb_t.  */
  int chars_per_limb;

  /* log(2)/log(conversion_base) */
  // mp_limb_t logb2;
  mp_limb_t logb2;

  /* log(conversion_base)/log(2) */
  // mp_limb_t log2b;
  mp_limb_t log2b;

  /* base**chars_per_limb, i.e. the biggest number that fits a word, built by
     factors of base.  Exception: For 2, 4, 8, etc, big_base is log2(base),
     i.e. the number of bits used to represent each digit in the base.  */
  mp_limb_t big_base;

  /* A GMP_LIMB_BITS bit approximation to 1/big_base, represented as a
     fixed-point number.  Instead of dividing by big_base an application can
     choose to multiply by big_base_inverted.  */
  mp_limb_t big_base_inverted;
};

#endif
#endif



