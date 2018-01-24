project('ksh93', 'c', default_options: ['b_lundef=false', 'default_library=static'])
cc = meson.get_compiler('c')
system = host_machine.system()

run_command('bin/libast_prereq.sh')

add_project_arguments('-D_PACKAGE_ast', language: 'c')
add_project_arguments('-DMESON_BUILD=1', language: 'c')

if system == 'darwin'
    add_project_arguments('-D_DARWIN_C_SOURCE', language: 'c')
endif

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

hosttype_cmd=run_command('bin/hosttype')
hosttype=hosttype_cmd.stdout().strip()
add_project_arguments('-DHOSTTYPE="' + hosttype + '"', language: 'c')

# Compiler arguments used when building `ksh93`, `shcomp`, and related AST
# code. These are primarily `-Dsome_symbol` flags that define which features
# the platform provides.
ptr_size = cc.sizeof('void*')
int_size = cc.sizeof('int')
long_size = cc.sizeof('long')
size_t_size = cc.sizeof('size_t')
off_t_size = cc.sizeof('off_t')

shared_c_args = [
    '-DUSAGE_LICENSE=""',
    '-D_API_ast=20130625',
    '-D_ast_sizeof_int=@0@'.format(int_size),
    '-D_ast_sizeof_long=@0@'.format(long_size),
    '-D_ast_sizeof_size_t=@0@'.format(size_t_size),
    '-D_ast_sizeof_off_t=@0@'.format(off_t_size),
    '-D_ast_sizeof_pointer=@0@'.format(ptr_size),
    '-D_ptr_bits=@0@'.format(ptr_size * 8),
]

subdir('features')
subdir('src/lib/libast')
subdir('src/lib/libcmd')
subdir('src/lib/libcoshell')
subdir('src/cmd/builtin')
subdir('src/cmd/ksh93')
