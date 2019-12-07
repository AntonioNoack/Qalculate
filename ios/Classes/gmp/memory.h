//
// Created by antonio on 12/1/19.
//

#ifndef CALCULATE_MEMORY_H
#define CALCULATE_MEMORY_H

inline void *__gmp_default_allocate (size_t size);

inline void *__gmp_default_reallocate (void *oldptr, size_t old_size, size_t new_size);

inline void __gmp_default_free (void *blk_ptr, size_t blk_size);

#ifdef QAL

extern void * (*__gmp_allocate_func) (size_t);
extern void * (*__gmp_reallocate_func) (void *, size_t, size_t);
extern void   (*__gmp_free_func) (void *, size_t);

#else

void * (*__gmp_allocate_func) (size_t) = __gmp_default_allocate;
void * (*__gmp_reallocate_func) (void *, size_t, size_t) = __gmp_default_reallocate;
void   (*__gmp_free_func) (void *, size_t) = __gmp_default_free;

#endif

#endif //CALCULATE_MEMORY_H
