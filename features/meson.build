# voidptr is larger than int
if ptr_size > int_size
    shared_c_args += ['-D_more_void_int']
endif

# long is larger than int
if long_size > int_size
    shared_c_args += ['-D_more_long_int']
endif

# On some platforms the math functions (e.g., `sin()`) that we need are found in
# libc, on others they are found in libm.
libm_dep = cc.find_library('m', required: false)

# On some platforms `dladdr()` and related functions are found in libc,
# on others they are found in libdl.
libdl_dep = cc.find_library('dl', required: false)

# On some systems (e.g., FreeBSD) `libutil` contains functions we may need
# such as `openpty()`.
libutil_dep = cc.find_library('util', required: false)

# On some platforms `backtrace()` and related functions are found in libc,
# on others they are found in libexecinfo.
libexecinfo_dep = cc.find_library('execinfo', required: false)

# On OpenBSD libiconv is in /usr/local/lib rather than /usr/lib.
libiconv_dep = cc.find_library('iconv', required: false, dirs: ['/usr/lib', '/usr/local/lib'])

# On Cygwin the message catalog functions (e.g., `catopen()`) are in this
# library.
libcatgets_dep = cc.find_library('catgets', required: false)


# It was turned off on Linux but the iffe test was botched, keeping it on here.
shared_c_args += ['-D_map_malloc',]

if cc.has_header('execinfo.h')
    shared_c_args += ['-D_hdr_execinfo']
endif

if cc.has_header('mach-o/dyld.h')
    shared_c_args += [ '-D_hdr_mach_o_dyld']
endif

openpty_hdrs = ['#include <stdlib.h>']
if cc.has_header('pty.h')
    shared_c_args += ['-D_hdr_pty']
    openpty_hdrs += ['#include <pty.h>']
endif
if cc.has_header('util.h')
    shared_c_args += ['-D_hdr_util']
    openpty_hdrs += ['#include <util.h>']
endif

if cc.has_header('malloc.h')
    shared_c_args += ['-D_hdr_malloc']
endif

if cc.has_function('sigqueue', prefix: '#include <signal.h>')
    shared_c_args += ['-D_lib_sigqueue']
endif

if not cc.has_function('isnanl', prefix: '#include <math.h>')
    shared_c_args += [ '-Disnanl=isnan' ]
endif

if cc.has_function('utimensat', prefix: '#include <sys/stat.h>')
    shared_c_args += [ '-D_lib_utimensat' ]
elif cc.has_function('utimets', prefix: '#include <sys/stat.h>')
    shared_c_args += [ '-D_lib_utimets' ]
endif

if cc.has_function('sysinfo', prefix: '#include <sys/sysinfo.h>')
    shared_c_args += [ '-D_lib_sysinfo' ]
endif

if cc.has_function('pipe2', prefix: '#include <unistd.h>')
    shared_c_args += [ '-D_lib_pipe2' ]
endif

socketpair_shutdown_feature_file = files('socketpair_shutdown.c')
socketpair_shutdown_feature_result = cc.run(
    socketpair_shutdown_feature_file,
    name: 'fchmod() after socketpair() shutdown()')
if socketpair_shutdown_feature_result.returncode() == 0
    shared_c_args += ['-D_socketpair_shutdown_mode']
endif

if cc.has_member('struct stat','st_mtim', prefix: '#include <sys/stat.h>')
    shared_c_args += ['-DSTAT_ST_MTIM']
elif cc.has_member('struct stat', 'st_mtimespec', prefix: '#include <sys/stat.h>')
    # Under macOS modification time field is defined as mtimespec
    shared_c_args += ['-DSTAT_ST_MTIMESPEC']
else
    error('Cannot determine modification time field in struct stat.')
endif

if cc.has_function('posix_openpt', prefix: '#include <stdlib.h>',
                    args: shared_c_args,
                    dependencies: [libutil_dep])
    shared_c_args += ['-D_lib_posix_openpt']
endif

if cc.has_function('openpty', prefix: '\n'.join(openpty_hdrs),
                    args: shared_c_args,
                    dependencies: [libutil_dep])
    shared_c_args += ['-D_lib_openpty']
endif

if cc.has_function('ptsname', prefix: '#include <stdlib.h>',
                    args: shared_c_args,
                    dependencies: [libutil_dep])
    shared_c_args += ['-D_lib_ptsname']
endif

if cc.has_function('grantpt', prefix: '#include <stdlib.h>',
                    args: shared_c_args,
                    dependencies: [libutil_dep])
    shared_c_args += ['-D_lib_grantpt']
endif

if cc.has_function('unlockpt', prefix: '#include <stdlib.h>',
                    args: shared_c_args,
                    dependencies: [libutil_dep])
    shared_c_args += ['-D_lib_unlockpt']
endif

pty_feature_file = files('pty.c')
pty_feature_result = cc.run(pty_feature_file, name: 'pty dev name check', args: shared_c_args)
if not pty_feature_result.compiled()
    error('Unable to compile the pty_feature.c module to determine pty device names')
endif
shared_c_args += pty_feature_result.stdout().split()

if cc.has_function('mallinfo', prefix: '#include <malloc.h>')
    shared_c_args += ['-D_lib_mallinfo']
endif

if cc.has_member('struct mstats', 'bytes_total', prefix : '#include<malloc/malloc.h>')
    shared_c_args += ['-D_lib_mstats']
endif

if cc.has_function('mallopt', prefix: '#include <malloc.h>')
    shared_c_args += ['-D_lib_mallopt']
endif

if cc.has_function('memalign', prefix: '#include <stdlib.h>')
    shared_c_args += ['-D_lib_memalign']
endif

if cc.has_function('pvalloc', prefix: '#include <malloc.h>')
    shared_c_args += ['-D_lib_pvalloc']
endif

if cc.has_function('__libc_malloc', prefix: '#include <malloc.h>')
    shared_c_args += ['-D_lib___libc_malloc']
endif

if cc.has_member('struct mallinfo', 'arena', prefix : '#include<malloc.h>')
    shared_c_args += ['-D_mem_arena_mallinfo']
endif

sbrk_feature_file = files('sbrk.c')
sbrk_feature_result = cc.run(sbrk_feature_file, name: 'sbrk() works as expected')
if sbrk_feature_result.returncode() == 0
    shared_c_args += ['-D_mem_sbrk']
endif

# This was defined in iffe tests when '_mmap_anon' is true
# '_mmap_anon' is always true so keeping this macro on here
shared_c_args += ['-D_mem_mmap_anon']

stack_direction_feature_file = files('stack_direction.c')
stack_direction_feature_result = cc.run(stack_direction_feature_file, name: 'stack grows downward')
if stack_direction_feature_result.returncode() == 0
    shared_c_args += ['-D_stk_down']
endif

syslocale_result = run_command('syslocale.sh')
if syslocale_result.returncode() == 0
    syslocale_output = syslocale_result.stdout().strip()
    shared_c_args += [ '-D_locale_utf8_str=' + syslocale_output ]
endif

libpath_result = run_command('libpath.sh')
if libpath_result.returncode() == 0
    libpath_output = libpath_result.stdout().strip()
    shared_c_args += [ '-DCONF_LIBPATH=' + libpath_output ]
endif

prog_result = run_command('prog.sh')
if prog_result.returncode() == 0
    prog_output = prog_result.stdout().strip()
    shared_c_args += [ '-D_PROC_PROG=' + prog_output ]
endif

align_feature_file = files('alignment.c')
align_feature_result = cc.run(align_feature_file, name: 'align bound check')
if not align_feature_result.compiled()
    error('Unable to compile the align_feature.c module to determine compiler alignment')
endif
shared_c_args += align_feature_result.stdout().split()

posix_spawn_feature_file = files('posix_spawn.c')
posix_spawn_feature_result = cc.run(
    posix_spawn_feature_file, name: 'posix_spawn exists and it works and its worth using')
shared_c_args += ['-D_lib_posix_spawn=@0@'.format(posix_spawn_feature_result.returncode())]
