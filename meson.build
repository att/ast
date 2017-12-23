project('ksh93', 'c', default_options: ['b_lundef=false'])
run_command('bin/setup_cmds.sh')
run_command('bin/libast_prereq.sh')
run_command('bin/ksh93_prereq.sh')

add_project_arguments('-D_PACKAGE_ast', language: 'c')
add_project_arguments('-DMESON_BUILD=1', language: 'c')

# We should remove all #prototyped pragmas from headers
add_project_arguments('-fno-strict-aliasing', language: 'c')
add_project_arguments('-Wno-unknown-pragmas', language: 'c')
add_project_arguments('-Wno-missing-braces', language: 'c')
add_project_arguments('-Wno-unused-result', language: 'c')
add_project_arguments('-Wno-return-type', language: 'c')
add_project_arguments('-Wno-int-to-pointer-cast', language: 'c')
add_project_arguments('-Wno-parentheses', language: 'c')
add_project_arguments('-Wno-unused', language: 'c')
add_project_arguments('-Wno-unused-but-set-variable', language: 'c')
add_project_arguments('-Wno-cpp', language: 'c')
add_project_arguments('-Wno-char-subscripts', language: 'c')

# We sure we have symbolic debug info in the compiled binaries so that our
# `dump_backtrace()` function can provide some minimally useful information.
add_project_arguments('-g', language: 'c')

hosttype_cmd=run_command('bin/package')
hosttype=hosttype_cmd.stdout().strip()

add_project_arguments('-DHOSTTYPE="' + hosttype + '"', language: 'c')
subdir('src/lib/libast')
subdir('src/lib/libcmd')
subdir('src/lib/libcoshell')
subdir('src/cmd/ksh93')
