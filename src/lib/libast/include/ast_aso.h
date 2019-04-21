/* : : generated from aso by iffe version 2013-11-14 : : */
#ifndef _def_aso_features
#define _def_aso_features 1

#include <stdint.h>
// This is the original definition of this macro:
//   #define asointegralof(x) (void *)(((char *)(x)) - ((char *)0))
// Which is just a roundabout way to coerce a possibly non-pointer value to a pointer.
// So do it the straightforward way and avoid lint warnings about subtracting NULL.
#define asointegralof(x) (void *)(x)

#if GCC_4_1PLUS_64_BIT_MEMORY_ATOMIC_OPERATIONS_MODEL
/* gcc 4.1+ 64 bit memory atomic operations model */
#define _ASO_INTRINSIC 1

#define asocas8(p, o, n) __sync_val_compare_and_swap(p, o, n)
#define asoadd8(p, n) __sync_fetch_and_add(p, n)
#define asosub8(p, n) __sync_fetch_and_sub(p, n)
#define asoinc8(p) __sync_fetch_and_add(p, 1)
#define asodec8(p) __sync_fetch_and_sub(p, 1)
#define asocas16(p, o, n) __sync_val_compare_and_swap(p, o, n)
#define asoadd16(p, n) __sync_fetch_and_add(p, n)
#define asosub16(p, n) __sync_fetch_and_sub(p, n)
#define asoinc16(p) __sync_fetch_and_add(p, 1)
#define asodec16(p) __sync_fetch_and_sub(p, 1)
#define asocas32(p, o, n) __sync_val_compare_and_swap(p, o, n)
#define asoadd32(p, n) __sync_fetch_and_add(p, n)
#define asosub32(p, n) __sync_fetch_and_sub(p, n)
#define asoinc32(p) __sync_fetch_and_add(p, 1)
#define asodec32(p) __sync_fetch_and_sub(p, 1)
#define asocas64(p, o, n) __sync_val_compare_and_swap(p, o, n)
#define asoadd64(p, n) __sync_fetch_and_add(p, n)
#define asosub64(p, n) __sync_fetch_and_sub(p, n)
#define asoinc64(p) __sync_fetch_and_add(p, 1)
#define asodec64(p) __sync_fetch_and_sub(p, 1)
#if _ast_sizeof_pointer == 8
#define asocasptr(p, o, n) \
    ((void *)__sync_val_compare_and_swap(p, asointegralof(o), asointegralof(n)))
#else
#define asocasptr(p, o, n) \
    ((void *)__sync_val_compare_and_swap(p, asointegralof(o), asointegralof(n)))
#endif

#elif GCC_4_1PLUS_32_BIT_MEMORY_ATOMIC_OPERATIONS_MODEL
/* gcc 4.1+ 32 bit memory atomic operations model */
#define _ASO_INTRINSIC 1

#define asocas8(p, o, n) __sync_val_compare_and_swap(p, o, n)
#define asoadd8(p, n) __sync_fetch_and_add(p, n)
#define asosub8(p, n) __sync_fetch_and_sub(p, n)
#define asoinc8(p) __sync_fetch_and_add(p, 1)
#define asodec8(p) __sync_fetch_and_sub(p, 1)
#define asocas16(p, o, n) __sync_val_compare_and_swap(p, o, n)
#define asoadd16(p, n) __sync_fetch_and_add(p, n)
#define asosub16(p, n) __sync_fetch_and_sub(p, n)
#define asoinc16(p) __sync_fetch_and_add(p, 1)
#define asodec16(p) __sync_fetch_and_sub(p, 1)
#define asocas32(p, o, n) __sync_val_compare_and_swap(p, o, n)
#define asoadd32(p, n) __sync_fetch_and_add(p, n)
#define asosub32(p, n) __sync_fetch_and_sub(p, n)
#define asoinc32(p) __sync_fetch_and_add(p, 1)
#define asodec32(p) __sync_fetch_and_sub(p, 1)
#define asocasptr(p, o, n) \
    ((void *)__sync_val_compare_and_swap(p, asointegralof(o), asointegralof(n)))

#elif ATOMIC_H_ATOMIC_CAS_64
/* <atomic.h> atomic_cas_64 */
#include <atomic.h>

#define _ASO_INTRINSIC 1

#define asocas8(p, o, n) atomic_cas_8(p, o, n)
#define asoadd8(p, n) (atomic_add_8_nv(p, n) - (n))
#define asosub8(p, n)           (atomic_add_8_nv(p,(-(n))+(n))
#define asoinc8(p) (atomic_add_8_nv(p, 1) - 1)
#define asodec8(p) (atomic_add_8_nv(p, -1) + 1)
#define asocas16(p, o, n) atomic_cas_16(p, o, n)
#define asoadd16(p, n) (atomic_add_16_nv(p, n) - (n))
#define asosub16(p, n) (atomic_add_16_nv(p, -(n)) + (n))
#define asoinc16(p) (atomic_add_16_nv(p, 1) - 1)
#define asodec16(p) (atomic_add_16_nv(p, -1) + 1)
#define asocas32(p, o, n) atomic_cas_32(p, o, n)
#define asoadd32(p, n) (atomic_add_32_nv(p, n) - (n))
#define asosub32(p, n) (atomic_add_32_nv(p, -(n)) + (n))
#define asoinc32(p) (atomic_add_32_nv(p, 1) - 1)
#define asodec32(p) (atomic_add_32_nv(p, -1) + 1)
#define asocas64(p, o, n) atomic_cas_64(p, o, n)
#define asoadd64(p, n) (atomic_add_64_nv(p, n) - (n))
#define asosub64(p, n) (atomic_add_64_nv(p, -(n)) + (n))
#define asoinc64(p) (atomic_add_64_nv(p, 1) - 1)
#define asodec64(p) (atomic_add_64_nv(p, -1) + 1)
#if _ast_sizeof_pointer == 8
#define asocasptr(p, o, n) \
    ((void *)atomic_cas_64((uint64_t *)(p), asointegralof(o), asointegralof(n)))
#else
#define asocasptr(p, o, n) \
    ((void *)atomic_cas_32((uint32_t *)(p), asointegralof(o), asointegralof(n)))
#endif

#elif ATOMIC_H_ATOMIC_CAS_32
/* <atomic.h> atomic_cas_32 */
#include <atomic.h>

#define _ASO_INTRINSIC 1

#define asocas8(p, o, n) atomic_cas_8(p, o, n)
#define asoadd8(p, n) (atomic_add_8_nv(p, n) - (n))
#define asosub8(p, n)           (atomic_add_8_nv(p,(-(n))+(n))
#define asoinc8(p) (atomic_add_8_nv(p, 1) - 1)
#define asodec8(p) (atomic_add_8_nv(p, -1) + 1)
#define asocas16(p, o, n) atomic_cas_16(p, o, n)
#define asoadd16(p, n) (atomic_add_16_nv(p, n) - (n))
#define asosub16(p, n) (atomic_add_16_nv(p, -(n)) + (n))
#define asoinc16(p) (atomic_add_16_nv(p, 1) - 1)
#define asodec16(p) (atomic_add_16_nv(p, -1) + 1)
#define asocas32(p, o, n) atomic_cas_32(p, o, n)
#define asoadd32(p, n) (atomic_add_32_nv(p, n) - (n))
#define asosub32(p, n) (atomic_add_32_nv(p, -(n)) + (n))
#define asoinc32(p) (atomic_add_32_nv(p, 1) - 1)
#define asodec32(p) (atomic_add_32_nv(p, -1) + 1)
#define asocasptr(p, o, n) \
    ((void *)atomic_cas_32((uint32_t *)(p), asointegralof(o), asointegralof(n)))

#elif ATOMIC_H_ATOMIC_CAS_64_WITH_LATOMIC
/* <atomic.h> atomic_cas_64 with -latomic */
#include <atomic.h>

#define _ASO_INTRINSIC 1

#define asocas8(p, o, n) atomic_cas_8(p, o, n)
#define asoadd8(p, n) (atomic_add_8_nv(p, n) - (n))
#define asosub8(p, n)           (atomic_add_8_nv(p,(-(n))+(n))
#define asoinc8(p) (atomic_add_8_nv(p, 1) - 1)
#define asodec8(p) (atomic_add_8_nv(p, -1) + 1)
#define asocas16(p, o, n) atomic_cas_16(p, o, n)
#define asoadd16(p, n) (atomic_add_16_nv(p, n) - (n))
#define asosub16(p, n) (atomic_add_16_nv(p, -(n)) + (n))
#define asoinc16(p) (atomic_add_16_nv(p, 1) - 1)
#define asodec16(p) (atomic_add_16_nv(p, -1) + 1)
#define asocas32(p, o, n) atomic_cas_32(p, o, n)
#define asoadd32(p, n) (atomic_add_32_nv(p, n) - (n))
#define asosub32(p, n) (atomic_add_32_nv(p, -(n)) + (n))
#define asoinc32(p) (atomic_add_32_nv(p, 1) - 1)
#define asodec32(p) (atomic_add_32_nv(p, -1) + 1)
#define asocas64(p, o, n) atomic_cas_64(p, o, n)
#define asoadd64(p, n) (atomic_add_64_nv(p, n) - (n))
#define asosub64(p, n) (atomic_add_64_nv(p, -(n)) + (n))
#define asoinc64(p) (atomic_add_64_nv(p, 1) - 1)
#define asodec64(p) (atomic_add_64_nv(p, -1) + 1)
#if _ast_sizeof_pointer == 8
#define asocasptr(p, o, n) \
    ((void *)atomic_cas_64((uint64_t *)(p), asointegralof(o), asointegralof(n)))
#else
#define asocasptr(p, o, n) \
    ((void *)atomic_cas_32((uint32_t *)(p), asointegralof(o), asointegralof(n)))
#endif

#elif ATOMIC_H_ATOMIC_CAS_32_WITH_LATOMIC
/* <atomic.h> atomic_cas_32 with -latomic */
#include <atomic.h>

#define _ASO_INTRINSIC 1

#define asocas8(p, o, n) atomic_cas_8(p, o, n)
#define asoadd8(p, n) (atomic_add_8_nv(p, n) - (n))
#define asosub8(p, n)           (atomic_add_8_nv(p,(-(n))+(n))
#define asoinc8(p) (atomic_add_8_nv(p, 1) - 1)
#define asodec8(p) (atomic_add_8_nv(p, -1) + 1)
#define asocas16(p, o, n) atomic_cas_16(p, o, n)
#define asoadd16(p, n) (atomic_add_16_nv(p, n) - (n))
#define asosub16(p, n) (atomic_add_16_nv(p, -(n)) + (n))
#define asoinc16(p) (atomic_add_16_nv(p, 1) - 1)
#define asodec16(p) (atomic_add_16_nv(p, -1) + 1)
#define asocas32(p, o, n) atomic_cas_32(p, o, n)
#define asoadd32(p, n) (atomic_add_32_nv(p, n) - (n))
#define asosub32(p, n) (atomic_add_32_nv(p, -(n)) + (n))
#define asoinc32(p) (atomic_add_32_nv(p, 1) - 1)
#define asodec32(p) (atomic_add_32_nv(p, -1) + 1)
#define asocasptr(p, o, n) \
    ((void *)atomic_cas_32((uint32_t *)(p), asointegralof(o), asointegralof(n)))

#elif ATOMIC_H_CAS64
/* <atomic.h> cas64 */
#include <atomic.h>

#define _ASO_INTRINSIC 1

#define asocas8(p, o, n) cas8(p, o, n)
#define asoadd8(p, n) atomic_add_8(p, n)
#define asosub8(p, n) atomic_sub_8(p, n)
#define asoinc8(p) atomic_add_8(p, 1)
#define asodec8(p) atomic_add_8(p, -1)
#define asocas16(p, o, n) cas16(p, o, n)
#define asoadd16(p, n) atomic_add_16(p, n)
#define asosub16(p, n) atomic_sub_16(p, n)
#define asoinc16(p) atomic_add_16(p, 1)
#define asodec16(p) atomic_add_16(p, -1)
#define asocas32(p, o, n) cas32(p, o, n)
#define asoadd32(p, n) atomic_add_32(p, n)
#define asosub32(p, n) atomic_sub_32(p, n)
#define asoinc32(p) atomic_add_32(p, 1)
#define asodec32(p) atomic_add_32(p, -1)
#define asocas64(p, o, n) cas64(p, o, n)
#define asoadd64(p, n) atomic_add_64(p, n)
#define asosub64(p, n) atomic_sub_64(p, n)
#define asoinc64(p) atomic_add_64(p, 1)
#define asodec64(p) atomic_add_64(p, -1)
#if _ast_sizeof_pointer == 8
#define asocasptr(p, o, n) ((void *)cas64((uint64_t *)(p), asointegralof(o), asointegralof(n)))
#else
#define asocasptr(p, o, n) ((void *)cas32((uint32_t *)(p), asointegralof(o), asointegralof(n)))
#endif

#elif ATOMIC_H_JUST_CAS64
/* <atomic.h> just cas64 */
#include <atomic.h>

#define _ASO_INTRINSIC 1

#define asocas8(p, o, n) cas8(p, o, n)
#define asocas16(p, o, n) cas16(p, o, n)
#define asocas32(p, o, n) cas32(p, o, n)
#define asocas64(p, o, n) cas64(p, o, n)
#if _ast_sizeof_pointer == 8
#define asocasptr(p, o, n) ((void *)cas64((uint64_t *)(p), asointegralof(o), asointegralof(n)))
#else
#define asocasptr(p, o, n) ((void *)cas32((uint32_t *)(p), asointegralof(o), asointegralof(n)))
#endif

#elif ATOMIC_H_CAS32
/* <atomic.h> cas32 */
#include <atomic.h>

#define _ASO_INTRINSIC 1

#define asocas8(p, o, n) cas8(p, o, n)
#define asoadd8(p, n) atomic_add_8(p, n)
#define asosub8(p, n) atomic_sub_8(p, n)
#define asoinc8(p) atomic_add_8(p, 1)
#define asodec8(p) atomic_add_8(p, -1)
#define asocas16(p, o, n) cas16(p, o, n)
#define asoadd16(p, n) atomic_add_16(p, n)
#define asosub16(p, n) atomic_sub_16(p, n)
#define asoinc16(p) atomic_add_16(p, 1)
#define asodec16(p) atomic_add_16(p, -1)
#define asocas32(p, o, n) cas32(p, o, n)
#define asoadd32(p, n) atomic_add_32(p, n)
#define asosub32(p, n) atomic_sub_32(p, n)
#define asoinc32(p) atomic_add_32(p, 1)
#define asodec32(p) atomic_add_32(p, -1)
#define asocasptr(p, o, n) ((void *)cas32((uint32_t *)(p), asointegralof(o), asointegralof(n)))

#elif ATOMIC_H_JUST_CAS32
/* <atomic.h> just cas32 */
#include <atomic.h>

#define _ASO_INTRINSIC 1

#define asocas8(p, o, n) cas8(p, o, n)
#define asocas16(p, o, n) cas16(p, o, n)
#define asocas32(p, o, n) cas32(p, o, n)
#define asocas64(p, o, n) cas64(p, o, n)
#define asocasptr(p, o, n) ((void *)cas32((uint32_t *)(p), asointegralof(o), asointegralof(n)))

#elif MIPS_COMPARE_AND_SWAP
/* mips compare and swap */
#define _ASO_INTRINSIC 1
#define asocas32(p, o, n) (__compare_and_swap((p), (o), (n)) ? (o) : *(p))
#define asocasptr(p, o, n) \
    (__compare_and_swap((long *)(p), asointegralof(o), asointegralof(n)) ? (o) : *(void **)(p))

#elif I386_I386_64_ASM_COMPARE_AND_SWAP
/* i386|i386-64 asm compare and swap */
#define _ASO_INTRINSIC 2
#define _ASO_i386 1

#define asocas32 _aso_cas32
#if _ast_sizeof_pointer == 8
#define asocas64 _aso_cas64
#define asocasptr(p, o, n) ((void *)asocas64((uint64_t *)(p), asointegralof(o), asointegralof(n)))
#else
#define asocasptr(p, o, n) ((void *)asocas32((uint32_t *)(p), asointegralof(o), asointegralof(n)))
#endif

#elif IA64_ASM_COMPARE_AND_SWAP
/* ia64 asm compare and swap */
#define _ASO_INTRINSIC 2
#define _ASO_ia64 1
#define asocas32 _aso_cas32
#define asocas64 _aso_cas64
#define asocasptr(p, o, n) ((void *)asocas64((uint64_t *)(p), asointegralof(o), asointegralof(n)))

#elif PPC_ASM_COMPARE_AND_SWAP
/* ppc asm compare and swap */
#define _ASO_INTRINSIC 2
#define _ASO_ppc 1
#define asocas32 _aso_cas32
#if _ast_sizeof_pointer == 8
#define asocas64 _aso_cas64
#define asocasptr(p, o, n) ((void *)asocas64((uint64_t *)(p), asointegralof(o), asointegralof(n)))
#else
#define asocasptr(p, o, n) ((void *)asocas32((uint32_t *)(p), asointegralof(o), asointegralof(n)))
#endif

#else
/* no intrinsic aso operations -- time to upgrade */
#define _ASO_INTRINSIC 0

#endif

#ifndef asocas8
extern uint8_t asocas8(uint8_t volatile *, int, int);
#endif
#ifndef asoget8
extern uint8_t asoget8(uint8_t volatile *);
#endif
#ifndef asoadd8
extern uint8_t asoadd8(uint8_t volatile *, int);
#endif
#ifndef asosub8
extern uint8_t asosub8(uint8_t volatile *, int);
#endif
#ifndef asoinc8
extern uint8_t asoinc8(uint8_t volatile *);
#endif
#ifndef asodec8
extern uint8_t asodec8(uint8_t volatile *);
#endif
#ifndef asomin8
extern uint8_t asomin8(uint8_t volatile *, int);
#endif
#ifndef asomax8
extern uint8_t asomax8(uint8_t volatile *, int);
#endif

#ifndef asocas16
extern uint16_t asocas16(uint16_t volatile *, int, int);
#endif
#ifndef asoget16
extern uint16_t asoget16(uint16_t volatile *);
#endif
#ifndef asoadd16
extern uint16_t asoadd16(uint16_t volatile *, int);
#endif
#ifndef asosub16
extern uint16_t asosub16(uint16_t volatile *, int);
#endif
#ifndef asoinc16
extern uint16_t asoinc16(uint16_t volatile *);
#endif
#ifndef asodec16
extern uint16_t asodec16(uint16_t volatile *);
#endif
#ifndef asomin16
extern uint16_t asomin16(uint16_t volatile *, int);
#endif
#ifndef asomax16
extern uint16_t asomax16(uint16_t volatile *, int);
#endif

#if !defined(asocas32) || _ASO_INTRINSIC > 1
extern uint32_t asocas32(uint32_t volatile *, uint32_t, uint32_t);
#endif
#ifndef asoget32
extern uint32_t asoget32(uint32_t volatile *);
#endif
#ifndef asoadd32
extern uint32_t asoadd32(uint32_t volatile *, uint32_t);
#endif
#ifndef asosub32
extern uint32_t asosub32(uint32_t volatile *, uint32_t);
#endif
#ifndef asoinc32
extern uint32_t asoinc32(uint32_t volatile *);
#endif
#ifndef asodec32
extern uint32_t asodec32(uint32_t volatile *);
#endif
#ifndef asomin32
extern uint32_t asomin32(uint32_t volatile *, uint32_t);
#endif
#ifndef asomax32
extern uint32_t asomax32(uint32_t volatile *, uint32_t);
#endif

#ifdef _ast_int8_t

#if !defined(asocas64) || _ASO_INTRINSIC > 1
extern uint64_t asocas64(uint64_t volatile *, uint64_t, uint64_t);
#endif
#ifndef asoget64
extern uint64_t asoget64(uint64_t volatile *);
#endif
#ifndef asoadd64
extern uint64_t asoadd64(uint64_t volatile *, uint64_t);
#endif
#ifndef asosub64
extern uint64_t asosub64(uint64_t volatile *, uint64_t);
#endif
#ifndef asoinc64
extern uint64_t asoinc64(uint64_t volatile *);
#endif
#ifndef asodec64
extern uint64_t asodec64(uint64_t volatile *);
#endif
#ifndef asomin64
extern uint64_t asomin64(uint64_t volatile *, uint64_t);
#endif
#ifndef asomax64
extern uint64_t asomax64(uint64_t volatile *, uint64_t);
#endif

#endif

#ifndef asocasptr
extern void *asocasptr(void volatile *, void *, void *);
#endif
#ifndef asogetptr
extern void *asogetptr(void volatile *);
#endif

#endif
