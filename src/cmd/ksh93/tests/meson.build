# Each entry in `all_tests` is an array of one or two elements. The first
# element is the test name. The second is an optional timeout if the default
# timeout of 30 seconds is too short. Try to keep the list sorted.
default_timeout = 30

# TODO: Fix and add `leaks` to this list (see issue #403).
all_tests = [
    # Start by testing all the builtin commands more or less in isolation.
    ['b_alias'],
    ['b_basename'],
    ['b_cat'],
    ['b_chmod'],
    ['b_cmp'],
    ['b_command'],
    ['b_cut'],
    ['b_dirname'],
    ['b_echo.exp'],
    ['b_enum'],
    ['b_grep'],
    ['b_head'],
    ['b_hist.exp'],
    ['b_jobs.exp'],
    ['b_jobs'],
    ['b_mkdir'],
    ['b_nameref'],
    ['b_print'],
    ['b_printf'],
    ['b_read.exp'],
    ['b_read'],
    ['b_return'],
    ['b_set.exp'],
    ['b_set'],
    ['b_sleep'],
    ['b_sync'],
    ['b_test.exp'],
    ['b_test'],
    ['b_time.exp'],
    ['b_times.exp'],
    ['b_ulimit'],
    ['b_uname'],
    ['b_wc'],
    ['b_whence'],

    # Now do more general syntax and complicated behavior testing.
    ['append'],
    ['arith'],
    ['arrays'],
    ['arrays2'],
    ['attributes'],
    ['basic', 90],
    ['bracket'],
    ['builtins'],
    ['case'],
    ['comvar'],
    ['comvario'],
    ['coprocess', 50],
    ['cubetype'],
    ['directoryfd'],
    ['emacs.exp'],
    ['exit'],
    ['expand'],
    ['functions'],
    ['getopts'],
    ['glob'],
    ['glob.exp'],
    ['heredoc'],
    ['ifs'],
    ['io'],
    ['leaks'],
    ['locale'],
    ['math', 50],
    ['modifiers'],
    ['namespace'],
    ['options'],
    ['path'],
    ['pointtype'],
    ['quoting'],
    ['quoting2'],
    ['readcsv'],
    ['recttype'],
    ['restricted'],
    ['rksh'],
    ['select'],
    ['sh_match'],
    ['sigchld', 100],
    ['signal'],
    ['statics'],
    ['subshell', 100],
    ['substring'],
    ['tilde'],
    ['timetype'],
    ['treemove'],
    ['types'],
    ['variables'],
    ['vartree1'],
    ['vartree2'],
    ['vi.exp'],

    # The following are tests that must be run serially after all other tests that might be run in
    # parallel. For example, the `special-dev-paths` test opens network connections on fixed TCP/IP
    # port numbers and thus cannot be run in parallel with itself (shcomp and non-shcomp variants).
    #
    # Use a timeout of zero to use the default timeout or a negative timeout to indicate the test
    # must run in isolation from other tests.
    ['special-dev-paths', 0],
]

# Some tests fail for inexplicable reasons on some platforms. In some cases, such as Cygwin, things
# simply don't work as expected and probably never will due to quirks of the platform. Another
# example is sunos/solaris has /proc but doesn't support manipulating directory fd's.
tests_to_skip = [
    ['cygwin', 'b_jobs.exp'],
    ['cygwin', 'b_time.exp'],
    ['cygwin', 'b_times.exp'],
    ['cygwin', 'signal'],
    ['openbsd', 'b_times.exp'],
    ['sunos', 'vi.exp'],
    ['sunos', 'directoryfd'],
    ['wsl', 'b_time.exp'],
    ['wsl', 'b_times.exp'],
    # Tests to be skipped because they are known to be broken when compiled by `shcomp`.
    # TODO: Fix these tests or the shcomp code.
    ['shcomp', 'io'],
    ['shcomp', 'b_set'],
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
