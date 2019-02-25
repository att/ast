build_dir = meson.build_root()

ksh93_files = []
ksh93_incdir = include_directories('include',
                                   '../../lib/libast/include/',
                                   '../../lib/libcmd/',
                                   '../../lib/libdll/')

subdir('include')
subdir('bltins')
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
                 dependencies: [libm_dep, libexecinfo_dep, libdl_dep],
                 install: false)

ksh93_exe = executable('ksh', ['sh/pmain.c'], c_args: shared_c_args,
    include_directories: [configuration_incdir, ksh93_incdir],
    link_with: [libksh, libast, libcmd, libdll],
    dependencies: [libm_dep, libexecinfo_dep, libdl_dep],
    install: true)

shcomp_exe = executable('shcomp', ['sh/shcomp.c'], c_args: shared_c_args,
    include_directories: [configuration_incdir, ksh93_incdir],
    link_with: [libksh, libast, libcmd, libdll],
    dependencies: [libm_dep, libexecinfo_dep, libdl_dep],
    install: true)

install_man('ksh.1')

test_dir = join_paths(meson.current_source_dir(), 'tests')
test_driver = join_paths(test_dir, 'util', 'run_test.sh')

# This variable is used by some tests while executing subtests
shell_var = 'SHELL=' + ksh93_exe.full_path()
shcomp_var = 'SHCOMP=' + shcomp_exe.full_path()

libast_build_dir = join_paths(build_dir, 'src', 'lib', 'libast')
libcmd_build_dir = join_paths(build_dir, 'src', 'lib', 'libcmd')
libdll_build_dir = join_paths(build_dir, 'src', 'lib', 'libdll')

ld_library_path = 'LD_LIBRARY_PATH=' + ':'.join(
    [libast_build_dir, libcmd_build_dir, libdll_build_dir])

# Sample loadable builtin will be loaded from this path in builtins test cases
libsample_path = 'LIBSAMPLE_PATH=' + libsample.full_path()

# Each entry in `all_tests` is an array of one or two elements. The first
# element is the test name. The second is an optional timeout if the default
# timeout of 30 seconds is too short. Try to keep the list sorted.
default_timeout = 30

# TODO: Fix and add `leaks` to this list (see issue #403).
all_tests = [
    ['alias'], ['append'], ['arith'], ['arrays'], ['arrays2'], ['attributes'], ['basename'],
    ['basic', 90], ['bracket'], ['builtins'], ['case'], ['cat'], ['cmp'], ['command'], ['comvar'],
    ['comvario'], ['coprocess', 50], ['cubetype'], ['cut'], ['directoryfd'], ['dirname'], ['enum'],
    ['exit'], ['expand'], ['functions'], ['getconf'], ['glob'], ['grep'], ['getopts'], ['head'], ['heredoc'],
    ['ifs'], ['io'], ['jobs'], ['leaks'], ['locale'], ['math', 50], ['mkdir'], ['nameref'], ['namespace'],
    ['modifiers'], ['options'], ['path'], ['pointtype'], ['print'], ['printf'], ['quoting'],
    ['quoting2'], ['read'], ['readcsv'], ['recttype'], ['restricted'], ['return'], ['rksh'],
    ['select'], ['set'], ['sh_match'], ['sigchld', 100], ['signal'], ['sleep'], ['statics'],
    ['subshell', 100], ['substring'], ['tilde'], ['timetype'], ['treemove'], ['types'],
    ['ulimit'], ['variables'], ['vartree1'], ['vartree2'], ['whence'],
    # Test non-special builtins; i.e., those that have to be enabled explicitly.
    ['chmod'],
    # These are interactive tests to be run via the `expect` utility.
    ['echo.exp'], ['emacs.exp'], ['glob.exp'], ['hist.exp'], ['jobs.exp'], ['read.exp'], ['set.exp'],
    ['test.exp'], ['vi.exp'],
    # The following are tests that must be run serially after all other tests that might be run in
    # parallel. For example, the `special-dev-paths` test opens network connections on fixed TCP/IP
    # port numbers and thus cannot be run in parallel with itself (shcomp and non-shcomp variants).
    #
    # Use a timeout of zero to use the default timeout or a negative timeout to indicate the test
    # must run in isolation from other tests.
    ['special-dev-paths', 0],
]

# This is a list of tests to be skipped because they are known to be broken when
# compiled by `shcomp`.
# TODO: Fix these tests.
shcomp_tests_to_skip = ['io', 'namespace', 'treemove', 'printf', 'set']
macos_tests_to_skip = ['jobs.exp' ]

# The test cases are executed in parallel by default
foreach testspec : all_tests
    testname = testspec[0]
    timeout = (testspec.length() == 2) ? testspec[1] : default_timeout
    if timeout <= 0
        parallel = false
        timeout = timeout == 0 ? default_timeout : -timeout
    else
        parallel = true
    endif
    if testname.endswith('.exp')
        # The interactive `expect` based tests are highly sensitive to timing variations.
        # Never run them in parallel with any other test.
        parallel = false
    endif

    # Run the test without compiling the script (which is how most people use ksh).
    lang_var = 'LANG=en_US.UTF-8'

    # Skip some tests on macOS
    if build_machine.system() != 'darwin' or not macos_tests_to_skip.contains(testname)
        test(testname, ksh93_exe, timeout: timeout, is_parallel: parallel,
            args: [test_driver, testname],
            env: [shell_var, lang_var, ld_library_path, libsample_path])
    endif

    # The shcomp variants are only applicable to the non-interactive tests.
    if not testname.endswith('.exp')
        # Run the test after compiling the script with `shcomp`.
        if not shcomp_tests_to_skip.contains(testname)
            lang_var = 'LANG=C'
            test(testname + '/shcomp', ksh93_exe, timeout: timeout, is_parallel: parallel,
                args: [ test_driver, 'shcomp', testname],
                env: [shell_var, lang_var, shcomp_var, ld_library_path, libsample_path])
        endif
    endif
endforeach
