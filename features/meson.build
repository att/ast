# On WSL (Windows Subsystem for Linux) some behavior may be different than on
# a real Linux system. For example, the `sigqueue()` function doesn't work at
# the time I write this (see https://github.com/Microsoft/WSL/issues/1880).
# So it's necessary to detect if we're running on that platform.
wsl_result = run_command('detect_wsl.sh')
if wsl_result.returncode() == 0
    feature_data.set('_WSL_', 1)
    system = 'wsl'  # this would otherwise be "linux"
else
    feature_data.set('_WSL_', 0)
endif

# Several unit tests need to know how many processors are available. For those
# tests to run reliably, especially on a VM which might have only one or two
# cpus, it is important to not let the test use a potentially large default.
nproc_result = run_command('detect_nproc.sh')
feature_data.set('N_PROC', nproc_result.returncode())

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

# On some platforms the network functions (e.g., `socket()`) that we need are found in
# libc, on others they are found in libsocket and libnsl.
libsocket_dep = cc.find_library('socket', required: false, dirs: lib_dirs)
libnsl_dep = cc.find_library('nsl', required: false, dirs: lib_dirs)

# On some platforms `dladdr()` and related functions are found in libc,
# on others they are found in libdl.
libdl_dep = cc.find_library('dl', required: false, dirs: lib_dirs)

# On some platforms `backtrace()` and related functions are found in libc,
# on others they are found in libexecinfo.
libexecinfo_dep = cc.find_library('execinfo', required: false, dirs: lib_dirs)

# On some platforms `fts` functions are found in libc,
# on others they are found in musl-fts.
libfts_dep = cc.find_library('fts', required: false, dirs: lib_dirs)

# On some systems (e.g., OpenBSD) `iconv()` is in libiconv.
libiconv_dep = cc.find_library('iconv', required: false, dirs: lib_dirs)

# On Cygwin the message catalog functions (e.g., `catopen()`) are in this library.
libcatgets_dep = cc.find_library('catgets', required: false, dirs: lib_dirs)

feature_data.set10('_hdr_execinfo', cc.has_header('execinfo.h', args: feature_test_args))
feature_data.set10('_hdr_filio', cc.has_header('filio.h', args: feature_test_args))
feature_data.set10('_hdr_malloc', cc.has_header('malloc.h', args: feature_test_args))
feature_data.set10('_hdr_stdlib', cc.has_header('stdlib.h', args: feature_test_args))
feature_data.set10('_hdr_sys_filio', cc.has_header('sys/filio.h', args: feature_test_args))
feature_data.set10('_hdr_sys_ldr', cc.has_header('sys/ldr.h', args: feature_test_args))
feature_data.set10('_hdr_sys_pstat', cc.has_header('sys/pstat.h', args: feature_test_args))
feature_data.set10('_hdr_sys_ptem', cc.has_header('sys/ptem.h', args: feature_test_args))
feature_data.set10('_hdr_sys_stream', cc.has_header('stream.h', args: feature_test_args))

# The `lchmod()` function is a bit of a special-case. The platform may not provide it
# directly but might provide the means to implement it.
feature_data.set10('_lib_lchmod',
    cc.has_function('lchmod', prefix: '\n'.join(['#include <unistd.h>', '#include <sys/stat.h>']),
    args: feature_test_args))
if feature_data.get('_lib_lchmod', 0) == 0
    feature_file = files('lchmod_fallback.c')
    feature_result = cc.run(feature_file, name: 'lchmod() fallback using fchmodat()',
                            args: feature_test_args)
    if feature_result.returncode() == 0
        feature_data.set10('_lib_lchmod_fchmodat_fallback', 1)
        feature_data.set10('_lib_lchmod', 1)
    endif
endif

feature_data.set10('_lib_getrusage',
    cc.has_function('getrusage',
        prefix: '\n'.join(['#include <sys/resource.h>', '#include <sys/time.h>']),
        args: feature_test_args))
feature_data.set10('_lib_sigqueue',
    cc.has_function('sigqueue', prefix: '#include <signal.h>', args: feature_test_args))

if not cc.has_function('isnanl', prefix: '#include <math.h>', args: feature_test_args)
    feature_data.set('isnanl', 'isnan')
endif

feature_data.set10('_lib_eaccess',
    cc.has_function('eaccess', prefix: '#include <unistd.h>', args: feature_test_args))
feature_data.set10('_lib_euidaccess',
    cc.has_function('euidaccess', prefix: '#include <unistd.h>', args: feature_test_args))
feature_data.set10('_lib_faccessat',
    cc.has_function('faccessat', prefix: '#include <unistd.h>', args: feature_test_args))
feature_data.set10('_lib_mkostemp',
    cc.has_function('mkostemp', prefix: '\n'.join(['#include <stdlib.h>', '#include <unistd.h>']),
                    args: feature_test_args))
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

# https://github.com/att/ast/issues/1096
# These math functions are not available on NetBSD
feature_data.set10('_lib_expm1l',
    cc.has_function('expm1l', prefix: '#include <math.h>', args: feature_test_args,
                    dependencies: [libm_dep]))
feature_data.set10('_lib_log1pl',
    cc.has_function('log1pl', prefix: '#include <math.h>', args: feature_test_args,
                    dependencies: [libm_dep]))
feature_data.set10('_lib_remainderl',
    cc.has_function('remainderl', prefix: '#include <math.h>', args: feature_test_args,
                    dependencies: [libm_dep]))
feature_data.set10('_lib_log2l',
    cc.has_function('log2l', prefix: '#include <math.h>', args: feature_test_args,
                    dependencies: [libm_dep]))
feature_data.set10('_lib_tgammal',
    cc.has_function('tgammal', prefix: '#include <math.h>', args: feature_test_args,
                    dependencies: [libm_dep]))
feature_data.set10('_lib_lgammal',
    cc.has_function('lgammal', prefix: '#include <math.h>', args: feature_test_args,
                    dependencies: [libm_dep]))


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

# Does the platform provide a `sighandler_t` type?
feature_data.set10('_has_sighandler_t',
    cc.has_type('sighandler_t', prefix: '#include <signal.h>', args: feature_test_args))

feature_data.set10('_mem_d_fileno_dirent',
    cc.has_member('struct dirent', 'd_fileno', prefix: '#include <dirent.h>',
                  args: feature_test_args))
feature_data.set10('_mem_d_ino_dirent',
    cc.has_member('struct dirent', 'd_ino', prefix: '#include <dirent.h>',
                  args: feature_test_args))
feature_data.set10('_mem_d_reclen_dirent',
    cc.has_member('struct dirent', 'd_reclen', prefix: '#include <dirent.h>',
                  args: feature_test_args))
feature_data.set10('_mem_d_type_dirent',
    cc.has_member('struct dirent', 'd_type', prefix: '#include <dirent.h>',
                  args: feature_test_args))
feature_data.set10('_mem_d_namlen_dirent',
    cc.has_member('struct dirent', 'd_namlen', prefix: '#include <dirent.h>',
                  args: feature_test_args))

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

libpath_result = run_command('libpath.sh')
if libpath_result.returncode() == 0
    libpath_output = libpath_result.stdout().strip()
    feature_data.set_quoted('CONF_LIBPATH', libpath_output)
else
    feature_data.set_quoted('CONF_LIBPATH', '')
endif
# TODO: Properly detect these values. For now just use the defaults from
# astconf.c so we can eventually remove that module. Note that we've been
# using these defaults since switching to Meson two years ago.
feature_data.set_quoted('CONF_LIBPREFIX', 'lib')
feature_data.set_quoted('CONF_LIBSUFFIX', '.so')

prog_result = run_command('prog.sh')
if prog_result.returncode() == 0
    prog_output = prog_result.stdout().strip()
    feature_data.set('_PROC_PROG', prog_output)
endif

poll_feature_file = files('poll.c')
poll_feature_result = cc.run(poll_feature_file,
                             name: 'poll() exists and is worth using',
                             args: feature_test_args)
feature_data.set10('_lib_poll', poll_feature_result.returncode() == 0)

posix_spawn_feature_file = files('posix_spawn.c')
posix_spawn_feature_result = cc.run(posix_spawn_feature_file,
                                    name: 'posix_spawn() exists and is worth using',
                                    args: feature_test_args)
feature_data.set10('_lib_posix_spawn', posix_spawn_feature_result.returncode() == 0)

# Meson adds -D_FILE_OFFSET_BITS=64 flag by default, but it does not work with
# fts functions in older versions of glibc. This feature test undefines it on
# such systems. https://github.com/mesonbuild/meson/issues/3519
fts_feature_file = files('fts.c')
fts_feature_result = cc.compiles(fts_feature_file,
                                 name: 'Check if -D_FILE_OFFSET_BITS=64 works with fts functions',
                                 args: feature_test_args + ['-D_FILE_OFFSET_BITS=64'])
if not fts_feature_result
    add_global_arguments('-U_FILE_OFFSET_BITS', language: 'c')
endif

tput = find_program('tput', required: false)
feature_data.set10('_cmd_tput', tput.found())

ed = find_program('ed', required: false)
if ed.found()
    feature_data.set_quoted('_pth_ed', ed.path())
else
    feature_data.set_quoted('_pth_ed', 'ed')
endif

atos = find_program('atos', required: false)
if atos.found()
    feature_data.set_quoted('_pth_atos', atos.path())
endif

addr2line = find_program('addr2line', required: false)
if addr2line.found()
    feature_data.set_quoted('_pth_addr2line', addr2line.path())
endif

has_dev_fd = run_command('sh', '-c', 'ls /dev/fd/9 9<&0')
feature_data.set10('has_dev_fd', has_dev_fd.returncode() == 0)

subdir('dll')
subdir('time')
subdir('utsname')
