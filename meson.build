project('ksh93', 'c', default_options: ['b_lundef=false', 'default_library=static'])

# We require the C99 (aka ISO9899:1999) standard. Note we use `gnu99` rather
# than `c99` because the former enables language features we need. The latter
# does that but also limits which APIs are available.
add_global_arguments('-std=gnu99', language: 'c')

# Without this flag some symbols aren't visible.
#
# TODO: Figure out if there is a more generic manner of enabling support for
# GNU distros.
add_global_arguments('-D_GNU_SOURCE', language: 'c')

# Global compiler flags aren't used by the feature tests. So instead put them
# in an array that will be used by the feature tests.
feature_test_args = ['-std=gnu99', '-D_GNU_SOURCE']

# Enable malloc debug options for macOS, GNU, FreeBSD, and other
# implementations. This isn't a replacement for Valgrind or Asan but is useful
# for detecting problems without incurring the overhead of those tools.
#
# To use these add `--setup=malloc` to your `meson test` command.
#
malloc_debug_env = environment()
# These env vars are recognized by AST vmalloc.
malloc_debug_env.set('VMALLOC_OPTIONS', 'junk')
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
add_test_setup('malloc', env: malloc_debug_env)

cc = meson.get_compiler('c')
system = host_machine.system()
# So the `config_ast.h`, `lc.h`, and other headers can be found (see below).
configuration_incdir = include_directories(['.', 'features/locales'])
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

# Error on implicit declarations.
if cc.has_argument('-Werror=implicit')
    add_global_arguments('-Werror=implicit', language: 'c')
endif

shared_c_args = [
    '-DUSAGE_LICENSE=""',
    '-D_AST_no_spawnveg=1',
]

# Compiler arguments used when building `ksh93`, `shcomp`, and related AST
# code. These are primarily `-Dsome_symbol` flags that define which features
# the platform provides.
ptr_size = cc.sizeof('void*')
int_size = cc.sizeof('int')
long_size = cc.sizeof('long')
size_t_size = cc.sizeof('size_t')
off_t_size = cc.sizeof('off_t')

feature_data = configuration_data()
feature_data.set('MESON_BUILD', 1)
feature_data.set('_PACKAGE_ast', 1)
feature_data.set('_API_ast', 20130625)
if system == 'darwin'
    feature_data.set('_DARWIN_C_SOURCE', 1)
endif

# We need symbols in the program to be visible to `dladdr()` and related
# functions. This is necessary to be able to resolve addresses in the program
# to the appropriate function. Otherwise only symbols in dynamically loaded
# libraries are visible.
if get_option('buildtype') == 'debug'
    feature_data.set('DEBUG_BUILD', 1)
    add_global_arguments('-fno-inline', language: 'c')
    # This should be `if cc.has_link_argument()` but that isn't available
    # until Meson 0.46 or newer and we can't count on that yet. And in
    # practice this test works fine but can result in a meson warning.
    if cc.has_argument('-Wl,-E')
        add_global_link_arguments('-Wl,-E', language: 'c')
    endif
endif

hosttype_cmd=run_command('bin/hosttype')
hosttype=hosttype_cmd.stdout().strip()
feature_data.set_quoted('HOSTTYPE', hosttype)

feature_data.set('_ast_sizeof_int', int_size)
feature_data.set('_ast_sizeof_long', long_size)
feature_data.set('_ast_sizeof_size_t', size_t_size)
feature_data.set('_ast_sizeof_off_t', off_t_size)
feature_data.set('_ast_sizeof_pointer', ptr_size)
feature_data.set('_ptr_bits', ptr_size * 8)

subdir('features')

configure_file(input : 'config_ast.h.in',
               output : 'config_ast.h',
               configuration : feature_data)

run_command('scripts/libast_prereq.sh')

subdir('src/lib/libast')
subdir('src/lib/libcmd')
subdir('src/lib/libdll')
subdir('src/lib/libcoshell')
subdir('src/cmd/builtin')
subdir('src/cmd/tests')
subdir('src/cmd/ksh93')
