//
// Created by antonio on 11/28/19.
//

#ifndef CALCULATE_CONFIG_H
#define CALCULATE_CONFIG_H

// #include <android/log.h>

// #define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "TAG", __VA_ARGS__);

int errno;

// for stdout and stderr
#include <stdio.h>
#include <stdint.h>
#include "struct_helper/helper.h"

// mpfr
#define MPFR_HAVE_GMP_IMPL 1
#define HAVE_STDARG 1
#define HAVE_LITTLE_ENDIAN 1

// #define __GMP_FORCE_mpz_perfect_square_p
#define __GMP_FORCE_mpz_fits_uint_p 1
#define __GMP_FORCE_mpz_fits_ulong_p 1
#define __GMP_EXTERN_INLINE inline

// for gmp
#define NO_ASM 1

// whatever...
#define PACKAGE_DATA_DIR ""
#define PACKAGE_LOCALE_DIR ""

#define HAVE_UNORDERED_MAP 1

// libxml
#define LIBXML_TREE_ENABLED 1
#define LIBXML_OUTPUT_ENABLED 1
// sax = simple event based xml parser
#define LIBXML_SAX1_ENABLED 1

#define HAVE_CTYPE_H 1

#endif //CALCULATE_CONFIG_H

