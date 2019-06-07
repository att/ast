# Each entry in `all_tests` is an array of one or two elements. The first
# element is the test name. The second is an optional timeout if the default
# timeout of 30 seconds is too short. Try to keep the list sorted.
default_timeout = 30

# TODO: Fix and add `leaks` to this list (see issue #403).
all_tests = [
    ['alias'], ['append'], ['arith'], ['arrays'], ['arrays2'], ['attributes'], ['basename'],
    ['basic', 90], ['bracket'], ['builtins'], ['case'], ['cat'], ['cmp'], ['command'], ['comvar'],
    ['comvario'], ['coprocess', 50], ['cubetype'], ['cut'], ['directoryfd'], ['dirname'], ['enum'],
    ['exit'], ['expand'], ['functions'], ['glob'], ['grep'], ['getopts'], ['head'], ['heredoc'],
    ['ifs'], ['io'], ['jobs'], ['leaks'], ['locale'], ['math', 50], ['mkdir'], ['nameref'], ['namespace'],
    ['modifiers'], ['options'], ['path'], ['pointtype'], ['print'], ['printf'], ['quoting'],
    ['quoting2'], ['read'], ['readcsv'], ['recttype'], ['restricted'], ['return'], ['rksh'],
    ['select'], ['set'], ['sh_match'], ['sigchld', 100], ['signal'], ['sleep'], ['statics'],
    ['subshell', 100], ['substring'], ['tilde'], ['timetype'], ['treemove'], ['types'],
    ['ulimit'], ['variables'], ['vartree1'], ['vartree2'], ['wc'], ['whence'], ['uname'],
    # Test non-special builtins; i.e., those that have to be enabled explicitly.
    ['chmod'],
    # These are interactive tests to be run via the `expect` utility.
    ['echo.exp'], ['emacs.exp'], ['glob.exp'], ['hist.exp'], ['jobs.exp'], ['read.exp'], ['set.exp'],
    ['test.exp'], ['time.exp'], ['vi.exp'],
    # The following are tests that must be run serially after all other tests that might be run in
    # parallel. For example, the `special-dev-paths` test opens network connections on fixed TCP/IP
    # port numbers and thus cannot be run in parallel with itself (shcomp and non-shcomp variants).
    #
    # Use a timeout of zero to use the default timeout or a negative timeout to indicate the test
    # must run in isolation from other tests.
    ['special-dev-paths', 0],
]

# Some tests fail for inexplicable reasons on some platforms. For example, on macOS the jobs.exp
# test to verify [ctrl-Z] suspends the current foreground job fails even though doing the same thing
# "by hand" works just fine. Possibly a quirk of the `expect` utility on macOS. In other cases,
# such as Cygwin, things simply don't work as expected and probably never will due to quirks of the
# platform. Another example is sunos/solaris has /proc but doesn't support manipulating directory
# fd's.
tests_to_skip = [
    ['cygwin', 'signal'],
    ['cygwin', 'jobs.exp'],
    ['darwin', 'jobs.exp'],
    ['openbsd', 'jobs.exp'],
    ['sunos', 'vi.exp'],
    ['sunos', 'directoryfd'],
    # Tests to be skipped because they are known to be broken when compiled by `shcomp`.
    # TODO: Fix these tests.
    ['shcomp', 'io'],
    ['shcomp', 'set'],
    ['shcomp', 'treemove'],
]

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

    skip_test = false
    foreach skip : tests_to_skip
        if system == skip[0] and testname == skip[1]
            warning('skipping ' + testname + ' on ' + system)
            skip_test = true
            # break  # Not until meson 0.49
        endif
    endforeach
    if not skip_test
        if testname.endswith('.exp')
            # The interactive `expect` based tests are highly sensitive to timing variations.
            # Never run them in parallel with any other test.
            parallel = false
        endif

        # Run the test without compiling the script (which is how most people use ksh).
        lang_var = 'LANG=en_US.UTF-8'
        test(testname, ksh93_exe, timeout: timeout, is_parallel: parallel,
            args: [test_driver, testname],
            env: [shell_var, lang_var, ld_library_path, libsample_path])

        # The shcomp variants are only applicable to the non-interactive tests.
        if not testname.endswith('.exp')
            skip_test = false
            foreach skip : tests_to_skip
                if 'shcomp' == skip[0] and testname == skip[1]
                    warning('skipping ' + testname + '/shcomp on ' + system)
                    skip_test = true
                    # break  # Not until meson 0.49
                endif
            endforeach
            if not skip_test
                # Run the test after compiling the script with `shcomp`.
                test(testname + '/shcomp', ksh93_exe, timeout: timeout, is_parallel: parallel,
                    args: [ test_driver, 'shcomp', testname],
                    env: [shell_var, lang_var, shcomp_var, ld_library_path, libsample_path])
            endif
        endif
    endif
endforeach
