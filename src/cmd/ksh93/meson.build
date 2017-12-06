ksh93_files = []
# We use same set of files as ksh93 to compile shcomp, we should check if it
# can be modularized.
shcomp_files = [ ]
ksh93_incdir = include_directories('include', 'features',
                                   '../../lib/libast/include/',
                                   '../../lib/libast/features/',
                                   '../../lib/libcmd/',
                                   '../../lib/libcoshell/')

# TODO: Add '-DSHOPT_DYNAMIC' when we have figured out how to get Meson to
# build the libast libdll code.
ksh93_c_args = ['-DERROR_CATALOG=0',
                '-DERROR_CONTEXT_T=Error_context_t',
                '-DKSHELL',
                '-DSHOPT_BASH',
                '-DSHOPT_BRACEPAT',
                '-DSHOPT_COMPLETE',
                '-DSHOPT_COSHELL',
                '-DSHOPT_FIXEDARRAY',
                '-DSHOPT_KIA',
                '-DSHOPT_MULTIBYTE',
                '-DSHOPT_NAMESPACE',
                '-DSHOPT_OPTIMIZE',
                '-DSHOPT_PFSH',
                '-DSHOPT_RAWONLY',
                '-DSHOPT_STATS',
                '-DSHOPT_SUID_EXEC',
                '-DUSAGE_LICENSE=""',
                '-D_PACKAGE_ast',
                '-D_BLD_shell',
                '-D_API_ast=20130625']

# TODO: -DSHOPT_DYNAMIC
shcomp_c_args = ['-DUSAGE_LICENSE=""',
                 '-DKSHELL',
                 '-DERROR_CATALOG="libshell"',
                 '-DSHOPT_STATS',
                 '-DSHOPT_NAMESPACE',
                 '-DSHOPT_COSHELL',
                 '-DSHOPT_PFSH',
                 '-DSHOPT_BASH',
                 '-D_BLD_shell',
                 '-D_API_ast=20130625',
                 '-D_PACKAGE_ast',
                 '-DERROR_CONTEXT_T=Error_context_t',
                 '-DSHOPT_FIXEDARRAY',
                 '-DSHOPT_MULTIBYTE',
                 '-DSHOPT_RAWONLY',
                 '-DSHOPT_SUID_EXEC',
                 '-DSHOPT_BRACEPAT',
                 '-DSHOPT_KIA',
                 '-DSHOPT_COMPLETE',
                 '-DSHOPT_POLL',
                 '-DSHOPT_OPTIMIZE']

cpu = host_machine.cpu()
system = host_machine.system()
bash_machtype = cpu + '-unknown-'+ system
ksh93_c_args += [ '-DBASH_MACHTYPE="' + bash_machtype + '"' ]
shcomp_c_args += [ '-DBASH_MACHTYPE="' + bash_machtype + '"' ]

subdir('bltins')
subdir('data')
subdir('edit')
subdir('sh')

cc = meson.get_compiler('c')
# On some platforms the math functions (e.g., `sin()`) that we need are found in
# libc, on others they are found in libm.
m_dep = cc.find_library('m', required: false)
# On some platforms `backtrace()` and related functions are found in libc,
# on others they are found in libexecinfo.
execinfo_dep = cc.find_library('execinfo', required: false)
# On some platforms `dladdr()` and related functions are found in libc,
# on others they are found in libdl.
libdl_dep = cc.find_library('libdl', required : false)

ksh93_exe = executable('ksh', ksh93_files, c_args: ksh93_c_args,
    include_directories: ksh93_incdir,
    link_with: [libast, libcmd, libcoshell],
    dependencies: [m_dep, execinfo_dep, libdl_dep],
    install: true)

shcomp_exe = executable('shcomp', shcomp_files, c_args: shcomp_c_args ,
    include_directories: ksh93_incdir,
    link_with: [ libast, libcmd, libcoshell],
    dependencies : [ m_dep, execinfo_dep, libdl_dep ],
    install: true)

install_man('ksh.1')

test_dir = join_paths(meson.current_source_dir(), 'tests')

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
locales = ['', 'C.UTF-8']

shcomp_test_path = join_paths(test_dir, 'shcomptest')

# Each entry in `all_tests` is an array of one or two elements. The first
# element is the test name. The second is an optional timeout if the default
# timeout of 30 seconds is too short. Try to keep the list sorted.
default_timeout = 30
# TODO: Fix and add `builtin_poll` to this list.
all_tests = [
    ['alias'], ['append'], ['arith'], ['arrays'], ['arrays2'], ['attributes'],
    ['basic', 90], ['bracket'], ['builtins'], ['case'], ['comvar'],
    ['comvario'], ['coprocess', 50], ['cubetype'], ['directoryfd'], ['enum'],
    ['exit'], ['expand'], ['functions'], ['glob'], ['grep'], ['heredoc'],
    ['io'], ['leaks'], ['locale'], ['math'], ['nameref'], ['namespace'],
    ['options'], ['path'], ['pointtype'], ['pty'], ['quoting'], ['quoting2'],
    ['readcsv'], ['recttype'], ['restricted'], ['return'], ['select'],
    ['sh_match'], ['sigchld', 100], ['signal', 100], ['statics'], ['subshell', 60],
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
    test_path = join_paths(test_dir, testname + '.sh')
    foreach locale : locales
        lang_var = 'LANG=' + locale
        if locale != ''
            locale = '(' + locale + ')'
        endif
        test(testname + locale, ksh93_exe, timeout: timeout, args: [test_path],
             env: [shell_var, lang_var, ld_library_path])
    endforeach

    # shcomp tests
    lang_var = 'LANG='
    if not shcomp_tests_to_skip.contains(testname)
        test(testname + '(shcomp)', ksh93_exe, timeout: timeout,
             args: [ shcomp_test_path, test_path],
             env: [shell_var, lang_var, shcomp_var, ld_library_path])
    endif
endforeach
