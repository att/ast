project('ksh93', 'c', default_options: [
    # This is the optimal production build type for this project. If you need
    # a debug build you must explicitly run `meson --buildtype=debug`. This
    # helps ensure that each distro (or user) gets a performance optimized build
    # even if they don't know to add the `--buildtype=minsize` option.
    'buildtype=minsize',
    'b_lundef=false',
    'default_library=static',
    'c_std=c99', # we require the C99 (aka ISO9899:1999) language standard
])

source_dir = meson.current_source_dir()
cc = meson.get_compiler('c')
cpu = host_machine.cpu()
system = host_machine.system()
# So the `config_ast.h` and other dynamically generated headers can be found (see below).
configuration_incdir = include_directories(['.'])

if get_option('warnings-are-errors') == true
    add_global_arguments('-Werror', language: 'c')
endif

# Global compiler flags aren't used by the feature tests. So instead put them
# in an array that will be used by the feature tests.
feature_test_args = ['-std=gnu99', '-D_GNU_SOURCE']

if system == 'openbsd'
    # This is needed because OpenBSD installs a lot of stuff in
    # /usr/local/{include,lib} that would normally be in /usr/{include,lib}.
    # But the compiler doesn't automatically search /usr/local/include for
    # headers so force it to do so.
    #
    # We could do this unconditionally but it's safer to limit the scope to
    # the platforms that need it.
    add_global_arguments('-I/usr/local/include', language: 'c')
    feature_test_args += '-I/usr/local/include'
endif

# We require the C99 (aka ISO9899:1999) standard. Note we use `gnu99` rather
# than `c99` because the former enables language features we need. The latter
# does that but also limits which APIs are available.
add_global_arguments('-std=gnu99', language: 'c')

# Without this flag some symbols aren't visible.
#
# TODO: Figure out if there is a more generic manner of enabling support for
# GNU distros.
add_global_arguments('-D_GNU_SOURCE', language: 'c')

# Solaris/SunOS and its derivatives, such as IllumOS and OpenIndiana, need
# special handling.
#
# TODO: Figure out the correct way to enable the POSIX extensions we require
# on Solaris. Defining __EXTENSIONS__ is almost certainly the wrong way to do
# this. But note it is also what the legacy ksh code did before we switched to
# Meson+Ninja. So perhaps this is correct and there is no better solution.
if system == 'sunos' or system == 'solaris'
    feature_test_args += ['-D__EXTENSIONS__', '-DAST_SUNOS']
    add_global_arguments('-D__EXTENSIONS__', language: 'c')
    add_global_arguments('-D_LARGEFILE_SOURCE', language: 'c')
    add_global_arguments('-D_LARGEFILE64_SOURCE', language: 'c')
    add_global_arguments('-D_FILE_OFFSET_BITS=64', language: 'c')
    add_global_arguments('-DAST_SUNOS', language: 'c')
    add_global_arguments('-m64', language: 'c')
    add_global_link_arguments('-m64', language: 'c')
endif


# The AST/ksh code is littered with aliasing that violates the assumptions the
# compiler makes when `-fstrict-aliasing` is in effect so make sure those
# optimizations are disabled.
add_global_arguments('-fno-strict-aliasing', language: 'c')

# Enable all compiler warnings. Selectively disable (blacklist) those that are
# more noise than value with `-Wno-...` flags. Note that Meson implicitly adds
# -Wall.
add_global_arguments('-Wextra', language: 'c')

# Uncomment this to enable warnings about implicit type coercions.
# This is gcc only; clang doesn't recognize it.
# add_global_arguments('-Wcoercion', language: 'c')

# Uncomment this to enable warnings about implicit 64 to 32 bit narrowing casts.
# This is clang only; gcc doesn't recognize it.
# add_global_arguments('-Wshorten-64-to-32', language: 'c')

# GCC 8 enables -Wcast-function-type when -Wextra is used. Sadly it causes far
# too many false positives in the ksh code. Such as in the
# src/cmd/ksh93/bltins/math.c module. So disable it.
#
# TODO: Refactor the code so that the incompatible explicit function casts are
# not needed.
if cc.get_id() == 'gcc' and cc.has_argument('-Wno-cast-function-type')
    add_global_arguments('-Wno-cast-function-type', language: 'c')
endif

# We don't want `__attribute__((annotate("oclint:suppress")))` to produce warnings.
add_global_arguments('-Wno-attributes', language: 'c')

# There are quite a few places in the code which use a char to index into an
# array. All of them appear to be "safe" in as much as they only do so when
# the value involves an ASCII character which, by definition, won't be
# negative.
#
# TODO: Modify the code to not rely on the assumption that dereferencing a
# char pointer is safe to use as an array index.
add_global_arguments('-Wno-char-subscripts', language: 'c')

# There are hundreds of instances of the code comparing signed and unsigned
# ints. Mostly involving a var of type signed int and `sizeof()` (which
# returns an unsigned value). Since we want to treat every warning as an error
# we can't enable this on CI builds. And for non-CI builds the noise isn't
# worth it.
add_global_arguments('-Wno-sign-compare', language: 'c')

# Enable malloc debug options for macOS, GNU, FreeBSD, and other
# implementations. This isn't a replacement for Valgrind or Asan but is useful
# for detecting problems without incurring the overhead of those tools.
#
# To use these add `--setup=malloc` to your `meson test` command.
#
malloc_debug_env = environment()
# These env vars are recognized by GNU malloc.
malloc_debug_env.set('MALLOC_CHECK_', '3')
malloc_debug_env.set('MALLOC_PERTURB_', '165')  # 0xA5
# These env vars are recognized by macOS malloc. Without `MallocLogFile` the
# macOS debug malloc messages go to stderr. Those messages include
# informational messages that cause any unit test that checks what is written
# to stderr to fail.
malloc_debug_env.set('MallocGuardEdges', '1')
malloc_debug_env.set('MallocScribble', '1')
malloc_debug_env.set('MallocErrorAbort', '1')
malloc_debug_env.set('MallocLogFile', '/dev/null')
# These env vars are recognized by jemalloc as used by FreeBSD, OpenBSD, and
# other distros.  See https://github.com/jemalloc/jemalloc/wiki/Getting-Started.
# The "junk=true" setting enables setting both allocated and freed memory to
# "0xA5" and "0x5A" respectively (other than, obviously, `calloc()`'d memory).
malloc_debug_env.set('MALLOC_CONF', 'junk:true')
# Add a malloc specific test setup environment.
add_test_setup('malloc', timeout_multiplier: 1, env: malloc_debug_env)

# If this is a debug build tell the compiler not to omit frame pointers. This
# may improve the ability of `dump_backtrace()` to produce useful information.
# It is also needed when building with ASAN support enabled. So if you ever
# remove this block of code be sure to move setting the flag to the ASAN block
# below.
if get_option('buildtype') == 'debug'
    add_global_arguments('-fno-omit-frame-pointer', language: 'c')
endif

# Enable ASAN (AddressSanitizer) when running unit tests.
#
# To use this add `--setup=asan` to your `meson test` command and configure
# with `meson -DASAN=true`.
#
# To use this configure with `meson -DASAN=true` and add `--setup=asan` to
# your `meson test` command.
asan_debug_env = environment()
if cc.get_id() == 'gcc'
    # This env var isn't needed when using ASAN with clang on macOS.
    asan_debug_env.set('ASAN_SYMBOLIZER_PATH', '/usr/bin/llvm-symbolizer')
endif

# We don't include "detect_leaks=1" because it isn't supported by clang (at
# least on macOS) and causes a test runtime failure. Also, there are so many
# memory leaks that those errors cause spurious failures in the unit tests.
# And in any case until we resolve the more serious memory management errors
# worrying about memory leaks is premature.
#
# Set `disable_coredump=0` if you want core dumps on ASAN failures. But be
# aware that can produce core files that are terabytes in size on 64 bit
# platforms. The `use_madv_dontdump=1` should mitigate that on Linux.
asan_options = [
    'disable_coredump=1',
    'use_madv_dontdump=1',
    'abort_on_error=1',
    'symbolize=1',
    'check_initialization_order=1',
    'detect_stack_use_after_return=1',
    'detect_leaks=0',
    'redzone=16',
]
asan_debug_env.set('ASAN_OPTIONS', ':'.join(asan_options))
add_test_setup('asan', timeout_multiplier: 2, env: asan_debug_env)

if get_option('ASAN') == true
    if get_option('buildtype') != 'debug'
        error('You must use the "debug" build type to enable ASAN')
    endif
    add_global_arguments('-fsanitize=address', language: 'c')
    add_global_arguments('-fsanitize-address-use-after-scope', language: 'c')
    add_global_arguments('-fno-common', language: 'c')
    add_global_arguments('-Db_lundef=false', language: 'c')
    if cc.get_id() == 'clang'
        # This is a bit of a hack to make using ASAN with clang on my macOS
        # server work. Meson appears to be running `ld` directly, rather than
        # via `clang`, for the final link stage. So have clang tell us which
        # args we need to add.
        asan_ld_args = run_command('scripts/clang-asan.sh').stdout().strip().split('\n')
        foreach asan_arg : asan_ld_args
            message('Adding ASAN ld arg "' + asan_arg + '"')
            add_global_link_arguments(asan_arg, language: 'c')
        endforeach
    endif
    if cc.get_id() == 'gcc'
        add_global_link_arguments('-Wl,-lasan', language: 'c')
    endif
endif

# If we're doing a unit test or code coverage build we may need to set certain
# symbols to non-default values to ensure some code is covered. For example,
# reducing limits to trigger some behavior. We don't base this on whether the
# build type is "debug" because we want to be able to create a debuggable ksh
# binary without incurring the cost of setting limits needed for unit testing.
if get_option('b_coverage') == true or get_option('unittest') == true
    add_global_arguments('-DHIST_RECENT=5', language: 'c')
    add_global_arguments('-DHIST_MAX=512', language: 'c')
    add_global_arguments('-DHIST_DFLT=25', language: 'c')
endif

# Error on implicit declarations.
if cc.has_argument('-Werror=implicit')
    add_global_arguments('-Werror=implicit', language: 'c')
endif

shared_c_args = [
    '-DUSE_SPAWN=' + '@0@'.format(get_option('use-spawn').to_int()),
]

ptr_size = cc.sizeof('void*')
int_size = cc.sizeof('int')
long_size = cc.sizeof('long')
size_t_size = cc.sizeof('size_t')
off_t_size = cc.sizeof('off_t')
int32_t_size = cc.sizeof('int32_t', prefix: '#include <sys/types.h>')
wchar_t_size = cc.sizeof('wchar_t', prefix: '#include <wchar.h>')

feature_data = configuration_data()
feature_data.set('MESON_BUILD', 1)
if system == 'darwin'
    feature_data.set('_DARWIN_C_SOURCE', 1)
endif
bash_machtype = cpu + '-unknown-'+ system
feature_data.set_quoted('BASH_MACHTYPE', bash_machtype)

# We need symbols in the program to be visible to `dladdr()` and related
# functions. This is necessary to be able to resolve addresses in the program
# to the appropriate function. Otherwise only symbols in dynamically loaded
# libraries may be visible.
if get_option('buildtype') == 'debug'
    feature_data.set('DEBUG_BUILD', 1)
    add_global_arguments('-fno-inline', language: 'c')
    if system == 'cygwin'
        # The current (2018/07) gcc compiler on Cygwin generates PE+ executables
        # and it requires the -W,--export-all-symbols instead of -Wl,--export-dynamic.
        if meson.version().version_compare('>=0.46.0')
            if cc.has_link_argument('-Wl,--export-all-symbols')
                add_global_link_arguments('-Wl,--export-all-symbols', language: 'c')
            endif
        else
            if cc.has_argument('-Wl,--export-all-symbols')
                add_global_link_arguments('-Wl,--export-all-symbols', language: 'c')
            endif
        endif
    else
        if meson.version().version_compare('>=0.46.0')
            if cc.has_link_argument('-Wl,--export-dynamic')
                add_global_link_arguments('-Wl,--export-dynamic', language: 'c')
            endif
        else
            if cc.has_argument('-Wl,--export-dynamic')
                add_global_link_arguments('-Wl,--export-dynamic', language: 'c')
            endif
        endif
    endif
endif

hosttype_cmd = run_command('scripts/hosttype')
hosttype = hosttype_cmd.stdout().strip()
osname = hosttype.split('.')[0]
feature_data.set_quoted('HOSTTYPE', hosttype)
feature_data.set_quoted('OSNAME', osname)

# Some systems, like FreeBSD, have a different signature for the fts_open() comparison function.
feature_data.set10('const_const_fts_open', osname == 'freebsd' or osname == 'cygwin')
# On Cygwin we can't use socketpair() for pipes.
feature_data.set10('_pipe_socketpair', osname != 'cygwin')

feature_data.set('_ast_sizeof_int', int_size)
feature_data.set('_ast_sizeof_int32_t', int32_t_size)
feature_data.set('_ast_sizeof_long', long_size)
feature_data.set('_ast_sizeof_size_t', size_t_size)
feature_data.set('_ast_sizeof_off_t', off_t_size)
feature_data.set('_ast_sizeof_pointer', ptr_size)
feature_data.set('_ast_sizeof_wchar_t', wchar_t_size)

subdir('features')

configure_file(input : 'config_ast.h.in',
               output : 'config_ast.h',
               configuration : feature_data)

run_command('scripts/libast_prereq.sh')

subdir('src/lib/libast')
subdir('src/lib/libdll')

# Check if api tests should be built
if get_option('build-api-tests') == true
    subdir('src/lib/libast/tests/')
endif

subdir('src/cmd/ksh93')
