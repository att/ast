cpu = host_machine.cpu()
system = host_machine.system()
bash_machtype = cpu + '-unknown-'+ system
feature_data.set_quoted('BASH_MACHTYPE', bash_machtype)

# voidptr is larger than int
if ptr_size > int_size
    feature_data.set('_more_void_int', 1)
endif

# long is larger than int
if long_size > int_size
    feature_data.set('_more_long_int', 1)
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
feature_data.set('_map_malloc', 1)

if cc.has_header('execinfo.h')
    feature_data.set('_hdr_execinfo', 1)
endif

if cc.has_header('mach-o/dyld.h')
    feature_data.set('_hdr_mach_o_dyld', 1)
endif

openpty_hdrs = ['#include <stdlib.h>']
if cc.has_header('pty.h')
    feature_data.set('_hdr_pty', 1)
    openpty_hdrs += ['#include <pty.h>']
endif
if cc.has_header('util.h')
    feature_data.set('_hdr_util', 1)
    openpty_hdrs += ['#include <util.h>']
endif

if cc.has_header('malloc.h')
    feature_data.set('_hdr_malloc', 1)
endif

if cc.has_header('paths.h')
    feature_data.set('_hdr_paths', 1)
endif

if cc.has_header('utmp.h')
    feature_data.set('_hdr_utmp', 1)
endif

if cc.has_header('utmpx.h')
    feature_data.set('_hdr_utmpx', 1)
endif

if cc.has_function('sigqueue', prefix: '#include <signal.h>')
    feature_data.set('_lib_sigqueue', 1)
endif

if not cc.has_function('isnanl', prefix: '#include <math.h>')
    feature_data.set('isnanl', 'isnan')
endif

if cc.has_function('utimensat', prefix: '#include <sys/stat.h>')
    feature_data.set('_lib_utimensat', 1)
elif cc.has_function('utimets', prefix: '#include <sys/stat.h>')
    feature_data.set('_lib_utimets', 1)
endif

if cc.has_function('sysinfo', prefix: '#include <sys/sysinfo.h>')
    feature_data.set('_lib_sysinfo', 1)
endif

if cc.has_function('pipe2', prefix: '#include <unistd.h>')
    feature_data.set('_lib_pipe2', 1)
endif

socketpair_shutdown_feature_file = files('socketpair_shutdown.c')
socketpair_shutdown_feature_result = cc.run(
    socketpair_shutdown_feature_file,
    name: 'fchmod() after socketpair() shutdown()')
if socketpair_shutdown_feature_result.returncode() == 0
    feature_data.set('_socketpair_shutdown_mode', 1)
endif

if cc.has_member('struct stat', 'st_mtim', prefix: '#include <sys/stat.h>')
    feature_data.set('STAT_ST_MTIM', 1)
elif cc.has_member('struct stat', 'st_mtimespec', prefix: '#include <sys/stat.h>')
    # Under macOS modification time field is defined as mtimespec
    feature_data.set('STAT_ST_MTIMESPEC', 1)
else
    error('Cannot determine modification time field in struct stat.')
endif

if cc.has_member('struct utmp', 'ut_host', prefix: '#include <utmp.h>')
    feature_data.set('_mem_ut_host_utmp', 1)
endif

if cc.has_member('struct utmp', 'ut_tv', prefix: '#include <utmp.h>')
    feature_data.set('_mem_ut_tv_utmp', 1)
endif

if cc.has_member('struct utmp', 'ut_type', prefix: '#include <utmp.h>')
    feature_data.set('_mem_ut_type_utmp', 1)
endif

if cc.has_member('struct utmp', 'ut_user', prefix: '#include <utmp.h>')
    feature_data.set('_mem_ut_user_utmp', 1)
endif

if cc.has_member('struct utmpx', 'ut_host', prefix: '#include <utmpx.h>')
    feature_data.set('_mem_ut_host_utmpx', 1)
endif

if cc.has_member('struct utmpx', 'ut_tv', prefix: '#include <utmpx.h>')
    feature_data.set('_mem_ut_tv_utmpx', 1)
endif

if cc.has_member('struct utmp', 'ut_type', prefix: '#include <utmpx.h>')
    feature_data.set('_mem_ut_type_utmxp', 1)
endif

if cc.has_member('struct utmpx', 'ut_user', prefix: '#include <utmpx.h>')
    feature_data.set('_mem_ut_user_utmpx', 1)
endif

if cc.has_function('posix_openpt', prefix: '#include <stdlib.h>',
                    args: shared_c_args,
                    dependencies: [libutil_dep])
    feature_data.set('_lib_posix_openpt', 1)
endif

if cc.has_function('openpty', prefix: '\n'.join(openpty_hdrs),
                    args: shared_c_args,
                    dependencies: [libutil_dep])
    feature_data.set('_lib_openpty', 1)
endif

if cc.has_function('ptsname', prefix: '#include <stdlib.h>',
                    args: shared_c_args,
                    dependencies: [libutil_dep])
    feature_data.set('_lib_ptsname', 1)
endif

if cc.has_function('grantpt', prefix: '#include <stdlib.h>',
                    args: shared_c_args,
                    dependencies: [libutil_dep])
    feature_data.set('_lib_grantpt', 1)
endif

if cc.has_function('unlockpt', prefix: '#include <stdlib.h>',
                    args: shared_c_args,
                    dependencies: [libutil_dep])
    feature_data.set('_lib_unlockpt', 1)
endif

pty_feature_file = files('pty.c')
pty_feature_result = cc.run(pty_feature_file, name: 'pty dev name check', args: shared_c_args)
if not pty_feature_result.compiled()
    error('Unable to compile the pty_feature.c module to determine pty device names')
endif
foreach line : pty_feature_result.stdout().strip().split('\n')
    feature_data.set(line.split())
endforeach

if cc.has_function('mallinfo', prefix: '#include <malloc.h>')
    feature_data.set('_lib_mallinfo', 1)
endif

if cc.has_member('struct mstats', 'bytes_total', prefix : '#include<malloc/malloc.h>')
    feature_data.set('_lib_mstats', 1)
endif

if cc.has_function('mallopt', prefix: '#include <malloc.h>')
    feature_data.set('_lib_mallopt', 1)
endif

if cc.has_function('memalign', prefix: '#include <stdlib.h>')
    feature_data.set('_lib_memalign', 1)
endif

if cc.has_function('pvalloc', prefix: '#include <malloc.h>')
    feature_data.set('_lib_pvalloc', 1)
endif

if cc.has_function('__libc_malloc', prefix: '#include <malloc.h>')
    feature_data.set('_lib___libc_malloc', 1)
endif

if cc.has_member('struct mallinfo', 'arena', prefix : '#include<malloc.h>')
    feature_data.set('_mem_arena_mallinfo', 1)
endif

sbrk_feature_file = files('sbrk.c')
sbrk_feature_result = cc.run(sbrk_feature_file, name: 'sbrk() works as expected')
if sbrk_feature_result.returncode() == 0
    feature_data.set('_mem_sbrk', 1)
endif

# This was defined in iffe tests when '_mmap_anon' is true
# '_mmap_anon' is always true so keeping this macro on here
feature_data.set('_mem_mmap_anon', 1)

stack_direction_feature_file = files('stack_direction.c')
stack_direction_feature_result = cc.run(stack_direction_feature_file, name: 'stack grows downward')
if stack_direction_feature_result.returncode() == 0
    feature_data.set('_stk_down', 1)
endif

syslocale_result = run_command('syslocale.sh')
if syslocale_result.returncode() == 0
    syslocale_output = syslocale_result.stdout().strip()
    feature_data.set('_locale_utf8_str', syslocale_output)
endif

libpath_result = run_command('libpath.sh')
if libpath_result.returncode() == 0
    libpath_output = libpath_result.stdout().strip()
    feature_data.set_quoted('CONF_LIBPATH', libpath_output)
endif

prog_result = run_command('prog.sh')
if prog_result.returncode() == 0
    prog_output = prog_result.stdout().strip()
    feature_data.set('_PROC_PROG', prog_output)
endif

align_feature_file = files('alignment.c')
align_feature_result = cc.run(align_feature_file, name: 'align bound check')
if not align_feature_result.compiled()
    error('Unable to compile the align_feature.c module to determine compiler alignment')
endif
foreach line : align_feature_result.stdout().strip().split('\n')
    feature_data.set(line.split())
endforeach

posix_spawn_feature_file = files('posix_spawn.c')
posix_spawn_feature_result = cc.run(
    posix_spawn_feature_file, name: 'posix_spawn exists and it works and its worth using')
feature_data.set('_lib_posix_spawn', posix_spawn_feature_result.returncode())

newgrp = find_program('newgrp', required: false)
if newgrp.found()
    feature_data.set('_cmd_newgrp', 1)
endif

tput = find_program('tput', required: false)
if tput.found()
    feature_data.set('_cmd_tput', 1)
endif

ed = find_program('ed', required: false)
if ed.found()
    feature_data.set_quoted('_pth_ed', ed.path())
else
    feature_data.set_quoted('_pth_ed', 'ed')
endif
