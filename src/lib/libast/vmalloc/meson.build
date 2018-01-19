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
    libast_c_args += ['-D_lib_mallinfo']
endif


if cc.has_member('struct mstats', 'bytes_total', prefix : '#include<malloc/malloc.h>')
    libast_c_args += ['-D_lib_mstats']
endif

if cc.has_function('mallopt', prefix: '#include <malloc.h>')
    libast_c_args += ['-D_lib_mallopt']
endif

if cc.has_function('memalign', prefix: '#include <stdlib.h>')
    libast_c_args += ['-D_lib_memalign']
endif

if cc.has_function('pvalloc', prefix: '#include <malloc.h>')
    libast_c_args += ['-D_lib_pvalloc']
endif

if cc.has_function('__libc_malloc', prefix: '#include <malloc.h>')
    libast_c_args += ['-D_lib___libc_malloc']
endif


if cc.has_header('malloc.h')
    libast_c_args += [ '-D_hdr_malloc' ]
endif

if cc.has_member('struct mallinfo', 'arena', prefix : '#include<malloc.h>')
    libast_c_args += ['-D_mem_arena_mallinfo']
endif

sbrk_feature_file = files('sbrk_feature.c')
sbrk_feature_result = cc.run(sbrk_feature_file, name: 'sbrk() works as expected')
if sbrk_feature_result.returncode() == 0
    libast_c_args += ['-D_mem_sbrk']
endif

# It was turned off on Linux but the iffe test was botched, keeping it on here
libast_c_args += ['-D_map_malloc']

# This was defined in iffe tests when '_mmap_anon' is true
# '_mmap_anon' is always true so keeping this macro on here
libast_c_args += ['-D_mem_mmap_anon']

stack_direction_feature_file = files('stack_direction_feature.c')
stack_direction_feature_result = cc.run(stack_direction_feature_file, name: 'stack grows downward')
if stack_direction_feature_result.returncode() == 0
    libast_c_args += ['-D_stk_down']
endif


