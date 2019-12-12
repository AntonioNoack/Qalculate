cmake_minimum_required(VERSION 3.4.1)  # for example
add_library( native_add

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
../ios/Classes/all.c
../ios/Classes/qal.cpp)
set_target_properties(native_add PROPERTIES LINKER_LANGUAGE CXX)

if(FALSE)


# ../ios/Classes/libxml/rngparser.c
# ../ios/Classes/libxml/runsuite.c
# ../ios/Classes/libxml/runxmlconf.c
#  ../ios/Classes/libxml/xmllint.c, has main

# ../ios/Classes/bootstrap.h
# ../ios/Classes/bootstrap.c

#../ios/Classes/mpfr/generic/sec_pi1_div.c
#popham
    #../ios/Classes/mpfr/generic/sqr_basecase.c
    #../ios/Classes/mpfr/generic/sec_div.c
    #../ios/Classes/libxml/runxmlconf.c
    # ../ios/Classes/defs2doc.cc
    #../ios/Classes/gen-jacobitab.c
    #../ios/Classes/gen-fac.c
    #../ios/Classes/gen-fib.c
    #../ios/Classes/gen-trialdivtab.c
    #../ios/Classes/gen-bases.c
    #../ios/Classes/gen-psqr.c
    # "../ios/Classes/libxml/runsuite.c"
    # "../ios/Classes/libxml/xmlcatalog.c"
    # "../ios/Classes/libxml/xmllint.c"
    # "./ios/Classes/libxml/rngparser.c"
    # "../ios/Classes/tal-debug.c"
    # "../ios/Classes/mpfr/jyn_asympt.c"
    # "../ios/Classes/mpfr/round_raw_generic.c"
endif()