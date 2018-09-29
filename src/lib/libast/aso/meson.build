libast_files += [
    'aso/aso.c'
]

if cc.links('''
        #include <stdint.h>
        int main()
        {
            uint64_t i = 0;
            return __sync_fetch_and_add(&i,7);
        }
        ''', name: 'gcc 4.1+ 64 bit memory atomic operations model')

    libast_c_args += [ '-DGCC_4_1PLUS_64_BIT_MEMORY_ATOMIC_OPERATIONS_MODEL=1' ]
    shared_c_args += [ '-DGCC_4_1PLUS_64_BIT_MEMORY_ATOMIC_OPERATIONS_MODEL=1' ]

elif cc.links('''
        #include <stdint.h>
        int main()
        {
            uint32_t i = 0;
            return __sync_fetch_and_add(&i,7);
        }
        ''', name: 'gcc 4.1+ 32 bit memory atomic operations model')

    libast_c_args += [ '-DGCC_4_1PLUS_32_BIT_MEMORY_ATOMIC_OPERATIONS_MODEL=1' ]
    shared_c_args += [ '-DGCC_4_1PLUS_32_BIT_MEMORY_ATOMIC_OPERATIONS_MODEL=1' ]

elif cc.links('''
        #include <stdint.h>
        #include <atomic.h>
        int main()
        {
            uint64_t i = 0;
            uint32_t j = 1;
            return atomic_cas_64(&i, 0, 1) != 0 || (atomic_add_32_nv(&j, 1) - 1) != 1;
        }
        ''', name: '<atomic.h> atomic_cas_64')

    libast_c_args += [ '-DATOMIC_H_ATOMIC_CAS_64=1' ]
    shared_c_args += [ '-DATOMIC_H_ATOMIC_CAS_64=1' ]

elif cc.links('''
        #include <stdint.h>
        #include <atomic.h>
        int main()
        {
            uint32_t i = 0;
            return atomic_cas_32(&i, 0, 1) != 0 || (atomic_add_32_nv(&i, 1) - 1) != 1;
        }
        ''', name: '<atomic.h> atomic_cas_32')

    libast_c_args += [ '-DATOMIC_H_ATOMIC_CAS_32=1' ]
    shared_c_args += [ '-DATOMIC_H_ATOMIC_CAS_32=1' ]

elif cc.links('''
        #include <stdint.h>
        #include <atomic.h>
        int main()
        {
            uint64_t i = 0;
            uint32_t j = 1;
            return atomic_cas_64(&i, 0, 1) != 0 || (atomic_add_32_nv(&j, 1) - 1) != 1;
        }
        ''', args: '-latomic', name: '<atomic.h> atomic_cas_64 with -latomic')

    libast_c_args += [ '-DATOMIC_H_ATOMIC_CAS_64_WITH_LATOMIC=1' ]
    shared_c_args += [ '-DATOMIC_H_ATOMIC_CAS_64_WITH_LATOMIC=1' ]

elif cc.links('''
        #include <stdint.h>
        #include <atomic.h>
        int main()
        {
            uint32_t i = 0;
            return atomic_cas_32(&i, 0, 1) != 0 || (atomic_add_32_nv(&i, 1) - 1) != 1;
        }
        ''', args: '-latomic', name: '<atomic.h> atomic_cas_32 with -latomic')

    libast_c_args += [ '-DATOMIC_H_ATOMIC_CAS_32_WITH_LATOMIC=1' ]
    shared_c_args += [ '-DATOMIC_H_ATOMIC_CAS_32_WITH_LATOMIC=1' ]

elif cc.links('''
        #include <stdint.h>
        #include <atomic.h>
        int main()
        {
            uint64_t i = 0;
            uint32_t j = 1;
            return cas64(&i, 0, 1) != 0 || atomic_add_32(&j, 1) != 1;
        }
        ''', name: '<atomic.h> cas64')

    libast_c_args += [ '-DATOMIC_H_CAS64=1' ]
    shared_c_args += [ '-DATOMIC_H_CAS64=1' ]

elif cc.links('''
        #include <stdint.h>
        #include <atomic.h>
        int main()
        {
            uint64_t i = 0;
            uint32_t j = 1;
            uint16_t k = 1;
            uint8_t l = 1;
            return cas64(&i, 0, 1) != 0 || cas32(&j, 0, 1) != 0 || cas16(&k, 0, 1) != 0 || cas8(&l, 0, 1) != 0;
        }
        ''', name: '<atomic.h> just cas64')

    libast_c_args += [ '-DATOMIC_H_JUST_CAS64=1' ]
    shared_c_args += [ '-DATOMIC_H_JUST_CAS64=1' ]

elif cc.links('''
        #include <stdint.h>
        #include <atomic.h>
        int main()
        {
            uint32_t i = 0;
            return cas32(&i, 0, 1) != 0 || (atomic_add_32(&i, 1)) != 1;
        }
        ''', name: '<atomic.h> cas32')

    libast_c_args += [ '-DATOMIC_H_CAS32=1' ]
    shared_c_args += [ '-DATOMIC_H_CAS32=1' ]

elif cc.links('''
        #include <stdint.h>
        #include <atomic.h>
        int main()
        {
            uint32_t j = 1;
            uint16_t k = 1;
            uint8_t l = 1;
            return cas32(&j, 0, 1) != 0 || cas16(&k, 0, 1) != 0 || cas8(&l, 0, 1) != 0;
        }
        ''', name: '<atomic.h> just cas32')

    libast_c_args += [ '-DATOMIC_H_JUST_CAS32=1' ]
    shared_c_args += [ '-DATOMIC_H_JUST_CAS32=1' ]

elif cc.links('''
        int main()
        {
            int i = 1;
            return __compare_and_swap(&i, 0, 1) != 1;
        }
        ''', name: 'mips compare and swap')

    libast_c_args += [ '-DMIPS_COMPARE_AND_SWAP=1' ]
    shared_c_args += [ '-DMIPS_COMPARE_AND_SWAP=1' ]

elif cc.links('''
        #include <stdint.h>

        static uint32_t
        cas32(uint32_t volatile* p, uint32_t o, uint32_t n)
        {
            uint32_t    r;
            __asm__ __volatile__ (
                "lock ; cmpxchg %3,%4"
                : "=a"(r), "=m"(*p)
                : "0"(o), "q"(n), "m"(*p)
                : "memory", "cc"
                );
            return r;
        }

        /* Commented code from original iffe test */
        /* #if _ast_sizeof_pointer == 8 */

        static uint64_t
        cas64(uint64_t volatile* p, uint64_t o, uint64_t n)
        {
            if (sizeof(void*) == 8) {
                uint64_t    r;
                __asm__ __volatile__ (
                    "lock ; cmpxchg %3,%4"
                    : "=a"(r), "=m"(*p)
                    : "0"(o), "q"(n), "m"(*p)
                    : "memory", "cc"
                    );

                return r;
            }
            else {
                return *(p);
            }
        }

        /*#else

        #define cas64(p,o,n)    (*(p))

        #endif*/

        int main()
        {
            uint32_t    i = 0;
            uint64_t    j = 0;
            return cas32(&i, 0, 1) || cas64(&j, 0, 1);
        }

        ''', name: 'i386|i386-64 asm compare and swap')

    libast_c_args += [ '-DI386_I386_64_ASM_COMPARE_AND_SWAP=1' ]
    shared_c_args += [ '-DI386_I386_64_ASM_COMPARE_AND_SWAP=1' ]

elif cc.links('''
        static uint32_t
        cas32(uint32_t volatile* p, uint32_t o, uint32_t n)
        {
            uint32_t    r;

            __asm__ __volatile__ (
                "zxt4 %3=%3 ;; mov ar.ccv=%3 ;; cmpxchg4.acq %0=%1,%2,ar.ccv"
                    : "=r"(r), "+S"(*p)
                : "r"(n), "r"(o) : "memory"
                );
            return r;
        }

        static uint64_t
        cas64(uint64_t volatile* p, uint64_t o, uint64_t n)
        {
            uint64_t    r;

            __asm__ __volatile__ (
                "mov ar.ccv=%3 ;; cmpxchg8.acq %0=%1,%2,ar.ccv"
                    : "=r"(r), "+S"(*p)
                : "r"(n), "r"(o) : "memory"
                );
            return r;
        }

        int main()
        {
            uint32_t    i = 0;
            uint64_t    j = 0;
            return cas32(&i, 0, 1) || cas64(&j, 0, 1);
        }
        ''', name: 'ia64 asm compare and swap')

    libast_c_args += [ '-DIA64_ASM_COMPARE_AND_SWAP=1' ]
    shared_c_args += [ '-DIA64_ASM_COMPARE_AND_SWAP=1' ]

elif cc.links('''
        #include <stdint.h>

        static uint32_t
        cas32(uint32_t volatile* p, uint32_t o, uint32_t n)
        {
            int    r;
            __asm__ __volatile__ (
                "0:    lwarx %0,0,%1 ;"
                "    xor. %0,%3,%0;"
                "    bne 1f;"
                "    stwcx. %2,0,%1;"
                "    bne- 0b;"
                "1:"
                : "=&r"(r)
                : "r"(p), "r"(n), "r"(o)
                : "cr0", "memory"
                );
            __asm__ __volatile__ ("isync" : : : "memory");
            return r ? *p : o;
        }

        static uint64_t
        cas64(uint64_t volatile* p, uint64_t o, uint64_t n)
        {
            long    r;
            __asm__ __volatile__ (
                "0:    ldarx %0,0,%1 ;"
                "    xor. %0,%3,%0;"
                "    bne 1f;"
                "    stdcx. %2,0,%1;"
                "    bne- 0b;"
                "1:"
                : "=&r"(r)
                : "r"(p), "r"(n), "r"(o)
                : "cr0", "memory"
                );
            __asm__ __volatile__ ("isync" : : : "memory");
            return r ? *p : o;
        }

        int main()
        {
            uint32_t    i = 0;
            uint64_t    j = 0;
            return cas32(&i, 0, 1) || cas64(&j, 0, 1);
        }

        ''', name: 'ppc asm compare and swap')

    libast_c_args += [ '-DPPC_ASM_COMPARE_AND_SWAP=1' ]
    shared_c_args += [ '-DPPC_ASM_COMPARE_AND_SWAP=1' ]
endif
