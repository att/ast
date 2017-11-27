project('ksh93', 'c', default_options: ['b_lundef=false'])
run_command('bin/setup_cmds.sh')
run_command('bin/libast_prereq.sh')
run_command('bin/ksh93_prereq.sh')

add_project_arguments('-D_PACKAGE_ast', language: 'c')
add_project_arguments('-DMESON_BUILD=1', language: 'c')

# We should remove all #prototyped pragmas from headers
add_project_arguments('-Wno-unknown-pragmas', language: 'c')

hosttype_cmd=run_command('bin/package')
hosttype=hosttype_cmd.stdout().strip()

add_project_arguments('-DHOSTTYPE="' + hosttype + '"', language: 'c')
subdir('src/lib/libast')
subdir('src/lib/libcmd')
subdir('src/lib/libcoshell')
subdir('src/cmd/ksh93')
