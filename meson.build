project('ksh93', 'c', default_options: ['b_lundef=false', 'default_library=static'])
cc = meson.get_compiler('c')
system = host_machine.system()
# So the `config_ast.h` header can be found (see below).
configuration_incdir = include_directories('.')
if system == 'openbsd'
    # This is needed because OpenBSD installs a lot of stuff in
    # /usr/local/{include,lib} that would normally be in /usr/{include,lib}.
    # But the compiler doesn't automatically search /usr/local/included for
    # headers so force it to do so.
    #
    # We could do this unconditionally but it's safer to limit the scope to
    # the platforms that need it.
    add_project_arguments('-I/usr/local/include', language: 'c')
endif

# We require the C99 (aka ISO9899:1999) standard. Note we use `gnu99` rather
# than `c99` because the former enables language features we need. The latter
# does that but also limits which APIs are available.
add_project_arguments('-std=gnu99', language: 'c')

# Make sure we have symbolic debug info in the compiled binaries so that our
# `dump_backtrace()` function can provide some minimally useful information.
add_project_arguments('-g', language: 'c')

shared_c_args = [
    '-DUSAGE_LICENSE=""',
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
    feature_data.set('DARWIN_C_SOURCE', 1)
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

run_command('bin/libast_prereq.sh')

subdir('src/lib/libast')
subdir('src/lib/libcmd')
subdir('src/lib/libcoshell')
subdir('src/cmd/builtin')
subdir('src/cmd/ksh93')
