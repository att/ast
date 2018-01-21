project('ksh93', 'c', default_options: ['b_lundef=false'])
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

# Compiler arguments used by ksh93 and shcomp executables. The aso feature
# tests need to modify this variable, so declaring it before libast meson
# build file is processed.
shared_c_args = []

subdir('src/lib/libast')
subdir('src/lib/libcmd')
subdir('src/lib/libcoshell')
subdir('src/cmd/builtin')
subdir('src/cmd/ksh93')
