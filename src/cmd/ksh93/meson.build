ksh93_files = []
# We use same set of files as ksh93 to compile shcomp, we should check if it
# can be modularized.
shcomp_files = [ ]
ksh93_incdir = include_directories('include',
                                   '../../lib/libast/include/',
                                   '../../lib/libast/features/',
                                   '../../lib/libcmd/',
                                   '../../lib/libcoshell/')

subdir('bltins')
subdir('data')
subdir('edit')
subdir('sh')

# These build time symbols are shared by ksh and shcomp.
#
# TODO: Add '-DSHOPT_DYNAMIC' when we have figured out how to get Meson to
# build the libast libdll code.
shared_c_args += [
    '-D_PACKAGE_ast',
    '-D_BLD_shell',
    '-DERROR_CONTEXT_T=Error_context_t',
    '-DKSHELL',
    '-DSHOPT_BASH',
    '-DSHOPT_COSHELL',
    '-DREAD_TIMEOUT=' + '@0@'.format(get_option('read-timeout')),
]

# TODO: Figure out why there is a different definition of these symbols for
# ksh93 versus shcomp and if there is a good reason add a comment why they are
# defined differently.
ksh93_c_args = shared_c_args + [
    '-DERROR_CATALOG=NULL',
]

shcomp_c_args = shared_c_args + [
    '-DERROR_CATALOG="libshell"',
]

shopt_devfd_result = run_command('sh', '-c', 'ls /dev/fd/9 9<&0')
if shopt_devfd_result.returncode() == 0
    ksh93_c_args += ['-DSHOPT_DEVFD']
    shcomp_c_args += ['-DSHOPT_DEVFD']
endif

ksh93_exe = executable('ksh', ksh93_files, c_args: ksh93_c_args,
    include_directories: [configuration_incdir, ksh93_incdir],
    link_with: [libast, libcmd, libcoshell],
    dependencies: [libm_dep, libexecinfo_dep, libdl_dep],
    install: true)

shcomp_exe = executable('shcomp', shcomp_files, c_args: shcomp_c_args ,
    include_directories: [configuration_incdir, ksh93_incdir],
    link_with: [ libast, libcmd, libcoshell],
    dependencies: [libm_dep, libexecinfo_dep, libdl_dep],
    install: true)

install_man('ksh.1')

test_dir = join_paths(meson.current_source_dir(), 'tests')
test_driver = join_paths(test_dir, 'util', 'run_test.sh')

# This variable is used by some tests while executing subtests
shell_var = 'SHELL=' + ksh93_exe.full_path()
shcomp_var = 'SHCOMP=' + shcomp_exe.full_path()

build_dir = meson.build_root()

libast_build_dir = join_paths(build_dir, 'src', 'lib', 'libast')
libcmd_build_dir = join_paths(build_dir, 'src', 'lib', 'libcmd')
libcoshell_build_dir = join_paths(build_dir, 'src', 'lib', 'libcoshell')

ld_library_path = 'LD_LIBRARY_PATH=' + ':'.join(
    [libast_build_dir, libcmd_build_dir, libcoshell_build_dir])

# These are the default locales used by legacy test script.
# The emptry string is POSIX locale.
# TODO: Check how ksh code handles '' string while setting locales and
# explicitly set string to 'C' or 'POSIX' here if possible.
locales = ['C', 'C.UTF-8']

# Each entry in `all_tests` is an array of one or two elements. The first
# element is the test name. The second is an optional timeout if the default
# timeout of 30 seconds is too short. Try to keep the list sorted.
default_timeout = 30
# TODO: Fix and add `builtin_poll` to this list.
# TODO: Fix and add `leaks` to this list (see issue #403).
all_tests = [
    ['alias'], ['append'], ['arith'], ['arrays'], ['arrays2'], ['attributes'],
    ['basic', 90], ['bracket'], ['builtins'], ['case'], ['comvar'],
    ['comvario'], ['coprocess', 50], ['cubetype'], ['directoryfd'], ['enum'],
    ['exit'], ['expand'], ['functions'], ['glob'], ['grep'], ['heredoc'],
    ['io'], ['locale'], ['math', 50], ['nameref'], ['namespace'],
    ['options'], ['path'], ['pointtype'], ['pty'], ['quoting'], ['quoting2'],
    ['readcsv'], ['recttype'], ['restricted'], ['return'], ['select'],
    ['sh_match'], ['sigchld', 100], ['signal'], ['statics'], ['subshell', 100],
    ['substring'], ['tilde'], ['timetype'], ['treemove'], ['types'],
    ['variables'], ['vartree1'], ['vartree2'], ['wchar']
]

# This is a list of tests to be skipped because they are known to be broken when
# compiled by `shcomp`.
# TODO: Fix these tests.
shcomp_tests_to_skip = ['io', 'namespace', 'treemove']

# The test cases are executed in parallel by default
foreach testspec : all_tests
    testname = testspec[0]
    timeout = (testspec.length() == 2) ? testspec[1] : default_timeout
    foreach locale : locales
        lang_var = 'LANG=' + locale
        if locale != ''
            locale = '/' + locale
        endif
        test(testname + locale, ksh93_exe, timeout: timeout,
             args: [test_driver, testname],
             env: [shell_var, lang_var, ld_library_path])
    endforeach

    # shcomp tests
    lang_var = 'LANG=C.UTF-8'
    if not shcomp_tests_to_skip.contains(testname)
        test(testname + '/shcomp', ksh93_exe, timeout: timeout,
             args: [ test_driver, 'shcomp', testname],
             env: [shell_var, lang_var, shcomp_var, ld_library_path])
    endif
endforeach
