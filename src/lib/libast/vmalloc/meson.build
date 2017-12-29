libast_files += [
    'vmalloc/malloc.c', 'vmalloc/vmbest.c', 'vmalloc/vmclear.c',
    'vmalloc/vmclose.c', 'vmalloc/vmdcderive.c', 'vmalloc/vmdcshare.c',
    'vmalloc/vmdcsystem.c', 'vmalloc/vmdebug.c', 'vmalloc/vmdisc.c',
    'vmalloc/vmexit.c', 'vmalloc/vmgetmem.c', 'vmalloc/vmlast.c',
    'vmalloc/vmmaddress.c', 'vmalloc/vmopen.c', 'vmalloc/vmpool.c',
    'vmalloc/vmsegment.c', 'vmalloc/vmset.c', 'vmalloc/vmstat.c',
    'vmalloc/vmstrdup.c', 'vmalloc/vmtrace.c', 'vmalloc/vmuserdata.c'
]


if cc.has_function('mallinfo', prefix: '#include <malloc.h>')
    libast_c_args += [ '-D_lib_mallinfo' ]
endif


if cc.has_member('struct mstats', 'bytes_total', prefix : '#include<malloc/malloc.h>')
    libast_c_args += [ '-D_lib_mstats' ]
endif

if cc.has_function('mallopt', prefix: '#include <malloc.h>')
    libast_c_args += [ '-D_lib_mallopt' ]
endif

if cc.has_function('memalign', prefix: '#include <stdlib.h>')
    libast_c_args += [ '-D_lib_memalign' ]
endif

if cc.has_function('pvalloc', prefix: '#include <malloc.h>')
    libast_c_args += [ '-D_lib_pvalloc' ]
endif

if cc.has_function('__libc_malloc', prefix: '#include <malloc.h>')
    libast_c_args += [ '-D_lib___libc_malloc' ]
endif


if cc.has_header('malloc.h')
    libast_c_args += [ '-D_hdr_malloc' ]
endif

if cc.has_member('struct mallinfo', 'arena', prefix : '#include<malloc.h>')
    libast_c_args += [ '-D_mem_arena_mallinfo' ]
endif


if cc.run('''
            #include        <sys/types.h>
            #include        <unistd.h>
            #undef    uchar
            #define    uchar    unsigned char
            int main()
            {    uchar    *brk0, *brk1;
                int    chg;

                /* allocate a big chunk */
                if(!(brk0 = (uchar*)sbrk(0)) || brk0 == (uchar*)(-1))
                    return 1;
                chg = 256 * 1024;
                brk0 += chg;
                if(sbrk(chg) == ((void*)-1))
                    return 1;
                if((brk1 = (uchar*)sbrk(0)) != brk0)
                    return 1;

                /* now return half of it */
                chg /= 2;
                brk1 -= chg;
                if(sbrk(-chg) == ((void*)-1))
                    return 1;
                if((brk0 = (uchar*)sbrk(0)) != brk1)
                    return 1;

                return 0;
            }
        ''', name: 'sbrk() works as expected').returncode() == 0

    libast_c_args += [ '-D_mem_sbrk' ]
endif

# It was turned off on Linux but the iffe test was botched, keeping it on here
libast_c_args += [ '-D_map_malloc' ]

# This was defined in iffe tests when '_mmap_anon' is true
# '_mmap_anon' is always true so keeping this macro on here
libast_c_args += [ '-D_mem_mmap_anon' ]

if cc.run('''
       static growdown()
       {    static char*    addr = 0;
           char        array[4];
           if(!addr)
           {    addr = &array[0];
               return growdown();
           }
           else if(addr < &array[0])
               return 0;
           else    return 1;
       }
       int main() { return growdown() ? 0 : 1; }
    ''', name: 'stack grows downward').returncode() == 0

    libast_c_args += [ '-D_stk_down' ]
endif


