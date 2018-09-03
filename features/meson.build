cpu = host_machine.cpu()
system = host_machine.system()
bash_machtype = cpu + '-unknown-'+ system
feature_data.set_quoted('BASH_MACHTYPE', bash_machtype)

# void ptr is larger than int
feature_data.set10('_more_void_int', ptr_size > int_size)

# long is larger than int
feature_data.set10('_more_long_int', long_size > int_size)

feature_data.set10('_ast_intmax_long', long_size == cc.sizeof('long long'))

# wchar_t is at least as large as an int
feature_data.set10('_wchar_t_is_int', wchar_t_size >= int_size)

if system == 'openbsd'
    # OpenBSD puts some libraries, like libexecinfo and libiconv, in
    # /usr/local/lib, but the compiler doesn't automatically search there.
    lib_dirs = ['/usr/local/lib']
else
    lib_dirs = []
endif

# On some platforms the math functions (e.g., `sin()`) that we need are found in
# libc, on others they are found in libm.
libm_dep = cc.find_library('m', required: false, dirs: lib_dirs)

# On some platforms `dladdr()` and related functions are found in libc,
# on others they are found in libdl.
libdl_dep = cc.find_library('dl', required: false, dirs: lib_dirs)

# On some platforms `backtrace()` and related functions are found in libc,
# on others they are found in libexecinfo.
libexecinfo_dep = cc.find_library('execinfo', required: false, dirs: lib_dirs)

# On some systems (e.g., OpenBSD) `iconv()` is in libiconv.
libiconv_dep = cc.find_library('iconv', required: false, dirs: lib_dirs)

# Under Cygwin the iconv shared library exports the symbols we need with a
# `lib` prefix. So we don't want the LIBICONV_PLUG behavior on that platform.
# On others we either want this or it's a no-op.
if not (system == 'cygwin' or system == 'openbsd')
    feature_data.set('LIBICONV_PLUG', 1)
endif

# On Cygwin the message catalog functions (e.g., `catopen()`) are in this
# library.
libcatgets_dep = cc.find_library('catgets', required: false, dirs: lib_dirs)


# It was turned off on Linux but the iffe test was botched, keeping it on here.
feature_data.set('_map_malloc', 1)

feature_data.set10('_hdr_execinfo', cc.has_header('execinfo.h', args: feature_test_args))

# TODO: Figure out if detecting header unistd.h is really necessary.
# Especially given it is assumed to be present for detecting functions like
# `pipe2()` below.
feature_data.set10('_hdr_unistd', cc.has_header('unistd.h', args: feature_test_args))
feature_data.set10('_hdr_stdlib', cc.has_header('stdlib.h', args: feature_test_args))
feature_data.set10('_hdr_malloc', cc.has_header('malloc.h', args: feature_test_args))
feature_data.set10('_hdr_paths', cc.has_header('paths.h', args: feature_test_args))
feature_data.set10('_hdr_termios', cc.has_header('termios.h', args: feature_test_args))
feature_data.set10('_hdr_sys_sysmacros', cc.has_header('sys/sysmacros.h', args: feature_test_args))
feature_data.set10('_hdr_utmp', cc.has_header('utmp.h', args: feature_test_args))
feature_data.set10('_hdr_utmpx', cc.has_header('utmpx.h', args: feature_test_args))

feature_data.set10('_lib_sigqueue',
    cc.has_function('sigqueue', prefix: '#include <signal.h>', args: feature_test_args))
feature_data.set10('_lib_tcgetattr',
    cc.has_function('tcgetattr', prefix: '#include <termios.h>', args: feature_test_args))
# TODO: Enable iswprint() detection when we understand why doing so causes the
# `wchar` unit test to fail. For now we force it to false to use the AST definition.
# feature_data.set10('_lib_iswprint',
#     cc.has_function('iswprint', prefix: '#include <wctype.h>', args: feature_test_args))
feature_data.set10('_lib_iswprint', 0)

if not cc.has_function('isnanl', prefix: '#include <math.h>', args: feature_test_args)
    feature_data.set('isnanl', 'isnan')
endif

feature_data.set10('_lib_strlcat',
    cc.has_function('strlcat', prefix: '#include <string.h>', args: feature_test_args))
feature_data.set10('_lib_utimensat',
    cc.has_function('utimensat', prefix: '#include <sys/stat.h>', args: feature_test_args))
feature_data.set10('_lib_sysinfo',
    cc.has_function('sysinfo', prefix: '#include <sys/systeminfo.h>', args: feature_test_args))
feature_data.set10('_lib_pipe2',
    cc.has_function('pipe2', prefix: '#include <unistd.h>', args: feature_test_args))
feature_data.set10('_lib_syncfs',
    cc.has_function('syncfs', prefix: '#include <unistd.h>', args: feature_test_args))

socketpair_shutdown_feature_file = files('socketpair_shutdown.c')
socketpair_shutdown_feature_result = cc.run(
    socketpair_shutdown_feature_file,
    name: 'fchmod() after socketpair() shutdown()',
    args: feature_test_args)
feature_data.set10('_socketpair_shutdown_mode',
    socketpair_shutdown_feature_result.returncode() == 0)

max_signum_feature_file = files('max_signum.c')
max_signum_feature_result = cc.run(
    max_signum_feature_file,
    name: 'max signal number',
    args: feature_test_args)
if max_signum_feature_result.returncode() == 0
    feature_data.set('MAX_SIGNUM', max_signum_feature_result.stdout().strip())
else
    warning('Could not reliably determine the max signal number.')
    # Okay, we can't figure it out from the symbols provided by signal.h so
    # use a big number and hope for the best. We're assuming no system has a
    # signal number larger than 64.
    feature_data.set('MAX_SIGNUM', '64')
endif


if cc.has_member('struct stat', 'st_mtim', prefix: '#include <sys/stat.h>',
                 args: feature_test_args)
    feature_data.set('STAT_ST_MTIM', 1)
elif cc.has_member('struct stat', 'st_mtimespec', prefix: '#include <sys/stat.h>',
                   args: feature_test_args)
    # Under macOS modification time field is defined as mtimespec
    feature_data.set('STAT_ST_MTIMESPEC', 1)
else
    error('Cannot determine modification time field in struct stat.')
endif

feature_data.set10('_mem_ut_host_utmp',
    cc.has_member('struct utmp', 'ut_host', prefix: '#include <utmp.h>', args: feature_test_args))
feature_data.set10('_mem_ut_tv_utmp',
    cc.has_member('struct utmp', 'ut_tv', prefix: '#include <utmp.h>', args: feature_test_args))
feature_data.set10('_mem_ut_type_utmp',
    cc.has_member('struct utmp', 'ut_type', prefix: '#include <utmp.h>', args: feature_test_args))
feature_data.set10('_mem_ut_user_utmp',
    cc.has_member('struct utmp', 'ut_user', prefix: '#include <utmp.h>', args: feature_test_args))
feature_data.set10('_mem_ut_host_utmpx',
    cc.has_member('struct utmpx', 'ut_host', prefix: '#include <utmpx.h>', args: feature_test_args))
feature_data.set10('_mem_ut_tv_utmpx',
    cc.has_member('struct utmpx', 'ut_tv', prefix: '#include <utmpx.h>', args: feature_test_args))
feature_data.set10('_mem_ut_type_utmxp',
    cc.has_member('struct utmp', 'ut_type', prefix: '#include <utmpx.h>', args: feature_test_args))
feature_data.set10('_mem_ut_user_utmpx',
    cc.has_member('struct utmpx', 'ut_user', prefix: '#include <utmpx.h>', args: feature_test_args))

feature_data.set10('_lib_mallinfo',
    cc.has_function('mallinfo', prefix: '#include <malloc.h>', args: feature_test_args))

feature_data.set10('_lib_mstats',
    cc.has_member('struct mstats', 'bytes_total', prefix : '#include<malloc/malloc.h>',
                  args: feature_test_args))

feature_data.set10('_lib_mallopt',
    cc.has_function('mallopt', prefix: '#include <malloc.h>', args: feature_test_args))
feature_data.set10('_lib___libc_malloc',
    cc.has_function('__libc_malloc', prefix: '#include <malloc.h>', args: feature_test_args))

feature_data.set10('_mem_arena_mallinfo',
    cc.has_member('struct mallinfo', 'arena', prefix : '#include<malloc.h>',
                  args: feature_test_args))

# This was defined in iffe tests when '_mmap_anon' is true
# '_mmap_anon' is always true so keeping this macro on here
feature_data.set('_mem_mmap_anon', 1)

stack_direction_feature_file = files('stack_direction.c')
stack_direction_feature_result = cc.run(stack_direction_feature_file, name: 'stack grows downward',
                                        args: feature_test_args)
feature_data.set10('_stk_down', stack_direction_feature_result.returncode() == 0)

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

posix_spawn_feature_file = files('posix_spawn.c')
posix_spawn_feature_result = cc.run(posix_spawn_feature_file,
                                    name: 'posix_spawn exists and it works and its worth using',
                                    args: feature_test_args)
feature_data.set10('_lib_posix_spawn', posix_spawn_feature_result.returncode() == 0)

# Meson adds -D_FILE_OFFSET_BITS=64 flag by default, but it does not work with fts functions in older versions
# of glibc. This feture test undefines it on such systems.
# https://github.com/mesonbuild/meson/issues/3519
fts_feature_file = files('fts.c')
fts_feature_result = cc.compiles(fts_feature_file,
                                    name: 'Check if -D_FILE_OFFSET_BITS=64 works with fts functions',
                                    args: feature_test_args + ['-D_FILE_OFFSET_BITS=64'])
if not fts_feature_result
    add_global_arguments('-U_FILE_OFFSET_BITS', language: 'c')
endif

newgrp = find_program('newgrp', required: false)
feature_data.set10('_cmd_newgrp', newgrp.found())

tput = find_program('tput', required: false)
feature_data.set10('_cmd_tput', tput.found())

ed = find_program('ed', required: false)
if ed.found()
    feature_data.set_quoted('_pth_ed', ed.path())
else
    feature_data.set_quoted('_pth_ed', 'ed')
endif

has_dev_fd = run_command('sh', '-c', 'ls /dev/fd/9 9<&0')
feature_data.set10('has_dev_fd', has_dev_fd.returncode() == 0)

has_dev_stdout = run_command('sh', '-c', 'echo yes >/dev/stdout')
feature_data.set10('has_dev_stdout',
                   has_dev_stdout.returncode() == 0 and has_dev_stdout.stdout() == 'yes\n')

subdir('dll')
subdir('ids')
subdir('locales')
subdir('sockets')
subdir('time')
subdir('utsname')
