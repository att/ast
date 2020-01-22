build_dir = meson.build_root()

ksh93_files = []
ksh93_incdir = include_directories('include',
                                   '../../lib/libast/include/',
                                   '../../lib/libdll/')

subdir('include')
subdir('bltins')
subdir('cmds')
subdir('data')
subdir('edit')
subdir('sh')

# These build time symbols are shared by ksh and shcomp.
shared_c_args += [
    '-D_BLD_shell',
    '-DERROR_CONTEXT_T=Error_context_t',
    '-DREAD_TIMEOUT=' + '@0@'.format(get_option('read-timeout')),
    '-DAUDIT_FILE=' + '"@0@"'.format(get_option('audit-file'))
]

libksh = library('ksh', ksh93_files,
                 include_directories: [configuration_incdir, ksh93_incdir],
                 c_args: shared_c_args,
                 dependencies: [libm_dep, libexecinfo_dep, libdl_dep, libsocket_dep, libnsl_dep],
                 install: get_option('default_library') == 'shared')

ksh93_exe = executable('ksh', ['sh/pmain.c'], c_args: shared_c_args,
    include_directories: [configuration_incdir, ksh93_incdir],
    link_with: [libksh, libast, libcmd, libdll],
    dependencies: [libm_dep, libexecinfo_dep, libdl_dep],
    install: true)

# Create a symlink for a restricted version of the ksh shell.
create_rksh_symlink = find_program(source_dir + '/scripts/create_rksh_symlink')
rksh93_exe = custom_target('rksh', input: ksh93_exe, output: 'rksh', build_by_default: true,
    command: [create_rksh_symlink, '@INPUT@'])

shcomp_exe = executable('shcomp', ['sh/shcomp.c'], c_args: shared_c_args,
    include_directories: [configuration_incdir, ksh93_incdir],
    link_with: [libksh, libast, libcmd, libdll],
    dependencies: [libm_dep, libexecinfo_dep, libdl_dep, libsocket_dep, libnsl_dep],
    install: true)

install_man('docs/ksh.1')

test_dir = join_paths(meson.current_source_dir(), 'tests')
test_driver = join_paths(test_dir, 'util', 'run_test.sh')

# This variable is used by some tests while executing subtests
src_root = 'SRC_ROOT=' + source_dir
test_root = 'TEST_ROOT=' + test_dir
shell_var = 'SHELL=' + ksh93_exe.full_path()
rshell_var = 'RSHELL=' + rksh93_exe.full_path()
shcomp_var = 'SHCOMP=' + shcomp_exe.full_path()

libast_build_dir = join_paths(build_dir, 'src', 'lib', 'libast')
libcmd_build_dir = join_paths(build_dir, 'src', 'lib', 'libcmd')
libdll_build_dir = join_paths(build_dir, 'src', 'lib', 'libdll')

ld_library_path = 'LD_LIBRARY_PATH=' + ':'.join(
    [libast_build_dir, libcmd_build_dir, libdll_build_dir])

# Sample loadable builtin will be loaded from this path in builtins test cases
libsample_path = 'LIBSAMPLE_PATH=' + libsample.full_path()

subdir('tests/api')
# Check if only api tests should be built
if get_option('build-api-tests-only') == false
    subdir('tests')
endif
