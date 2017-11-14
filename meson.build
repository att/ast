project('ksh93', 'c')
run_command('bin/setup_cmds.sh')
run_command('bin/libast_prereq.sh')

add_project_arguments('-D_PACKAGE_ast', language: 'c')
add_project_arguments('-D_BLD_ast', language: 'c')

# We should remove all #prototyped pragmas from headers
add_project_arguments('-Wno-unknown-pragmas', language: 'c')

hosttype_cmd=run_command('bin/package')
hosttype=hosttype_cmd.stdout().strip()

add_project_arguments('-DHOSTTYPE="' + hosttype + '"', language: 'c')
subdir('src/lib/libast')
