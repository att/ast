# Each entry in `all_tests` is an array of one or two elements. The first
# element is the test name. The second is an optional timeout if the default
# timeout of 30 seconds is too short. Try to keep the list sorted.
default_timeout = 30
lang_var = 'LANG=en_US.UTF-8'

# TODO: Fix and add `leaks` to this list (see issue #403).
all_tests = [
    # Start by testing all the builtin commands more or less in isolation.
    ['b_alias.sh'],
    ['b_basename.sh'],
    ['b_cat.sh'],
    ['b_chmod.sh'],
    ['b_cmp.sh'],
    ['b_command.sh'],
    ['b_cut.sh'],
    ['b_dirname.sh'],
    ['b_echo.exp'],
    ['b_enum.sh'],
    ['b_exit.sh'],
    ['b_grep.sh'],
    ['b_head.sh'],
    ['b_hist.exp'],
    ['b_jobs.exp'],
    ['b_jobs.sh'],
    ['b_local.sh'],
    ['b_mkdir.sh'],
    ['b_nameref.sh'],
    ['b_print.sh'],
    ['b_printf.sh'],
    ['b_read.exp'],
    ['b_read.sh'],
    ['b_return.sh'],
    ['b_set.exp'],
    ['b_set.sh'],
    ['b_sleep.sh'],
    ['b_sync.sh'],
    ['b_test.exp'],
    ['b_test.sh'],
    ['b_time.exp'],
    ['b_times.exp'],
    ['b_ulimit.sh'],
    ['b_uname.sh'],
    ['b_wc.sh'],
    ['b_whence.sh'],

    # Now do more general syntax and complicated behavior testing.
    ['append.sh'],
    ['arith.sh'],
    ['arrays.sh'],
    ['arrays2.sh'],
    ['attributes.sh'],
    ['basic.sh', 90],
    ['bracket.sh'],
    ['builtins.sh'],
    ['case.sh'],
    ['comvar.sh'],
    ['comvario.sh'],
    ['coprocess.sh', 50],
    ['cubetype.sh'],
    ['directoryfd.sh'],
    ['emacs.exp'],
    ['exit.sh'],
    ['expand.sh'],
    ['functions.sh'],
    ['getopts.sh'],
    ['glob.sh'],
    ['glob.exp'],
    ['heredoc.sh'],
    ['ifs.sh'],
    ['io.sh'],
    ['leaks.sh'],
    ['locale.sh'],
    ['math.sh', 50],
    ['modifiers.sh'],
    ['namespace.sh'],
    ['options.sh'],
    ['path.sh'],
    ['pointtype.sh'],
    ['quoting.sh'],
    ['quoting2.sh'],
    ['readcsv.sh'],
    ['recttype.sh'],
    ['restricted.sh'],
    ['rksh.sh'],
    ['select.sh'],
    ['sh_match.sh', 120],
    ['sigchld.sh', 100],
    ['signal.sh'],
    ['statics.sh'],
    ['subshell.sh', 100],
    ['substring.sh'],
    ['tilde.sh'],
    ['timetype.sh'],
    ['treemove.sh'],
    ['types.sh'],
    ['variables.sh'],
    ['vartree1.sh'],
    ['vartree2.sh'],
    ['vi.exp'],

    # The following are tests that must be run serially after all other tests that might be run in
    # parallel. For example, the `special-dev-paths` test opens network connections on fixed TCP/IP
    # port numbers and thus cannot be run in parallel with itself (shcomp and non-shcomp variants).
    #
    # Use a timeout of zero to use the default timeout or a negative timeout to indicate the test
    # must run in isolation from other tests.
    ['special-dev-paths.sh', 0],
]

# Some tests fail for inexplicable reasons on some platforms. In some cases, such as Cygwin, things
# simply don't work as expected and probably never will due to quirks of the platform. Another
# example is sunos/solaris has /proc but doesn't support manipulating directory fd's. Also, some
# tests are simply too slow on particular platforms when hosted on a VM.
tests_to_skip = [
    ['cygwin', 'b_jobs.exp'],
    ['cygwin', 'b_time.exp'],
    ['cygwin', 'b_times.exp'],
    ['cygwin', 'b_ulimit.sh'],
    ['cygwin', 'b_uname.sh'],  # too many trivial diffs in uname output
    ['cygwin', 'coprocess.sh'],
    ['cygwin', 'signal.sh'],
    ['cygwin', 'sh_match.sh'],  # too slow, times out
    ['openbsd', 'b_times.exp'],
    ['sunos', 'vi.exp'],
    ['sunos', 'directoryfd.sh'],
    ['wsl', 'b_time.exp'],
    ['wsl', 'b_times.exp'],
    ['wsl', 'sh_match.sh'],  # too slow, times out
    # Tests to be skipped because they are known to be broken when compiled by `shcomp`.
    # TODO: Fix these tests or the shcomp code.
    ['shcomp', 'io.sh'],
    ['shcomp', 'b_set.sh'],
    ['shcomp', 'treemove.sh'],
]

# Define each test.
foreach testspec : all_tests
    test_name = testspec[0]
    timeout = (testspec.length() == 2) ? testspec[1] : default_timeout
    # Test cases are executed in parallel by default. Check if a specific test should be run in
    # isolation based on whether the timeout is negative.
    if timeout <= 0
        parallel = false
        timeout = timeout == 0 ? default_timeout : -timeout
    else
        parallel = true
    endif
    timeout_var = 'TIMEOUT=' + timeout.to_string()

    skip_test = false
    foreach skip : tests_to_skip
        if system == skip[0] and test_name == skip[1]
            warning('skipping ' + test_name + ' on ' + system)
            skip_test = true
            # break  # Not until meson 0.49
        endif
    endforeach
    if not skip_test
        if test_name.endswith('.exp')
            # The interactive `expect` based tests are highly sensitive to timing variations.
            # Never run them in parallel with any other test.
            parallel = false
        endif

        # Run the test without compiling the script (which is how most people use ksh).
        # Note the use of `-p` to keep the shell from sourcing dot scripts. Without this the
        # platform's ksh config files could cause problems.
        test(test_name, ksh93_exe, timeout: timeout, is_parallel: parallel,
            args: ['-p', test_driver, test_name],
            env: [shell_var, lang_var, src_root, test_root, ld_library_path, libsample_path,
                  timeout_var])

        # The shcomp variants are only applicable to the non-interactive tests.
        if not test_name.endswith('.exp')
            skip_test = false
            foreach skip : tests_to_skip
                if 'shcomp' == skip[0] and test_name == skip[1]
                    warning('skipping ' + test_name + '/shcomp on ' + system)
                    skip_test = true
                    # break  # Not until meson 0.49
                endif
            endforeach
            if not skip_test
                # Run the test after compiling the script with `shcomp`.
                test(test_name + '/shcomp', ksh93_exe, timeout: timeout, is_parallel: parallel,
                    args: ['-p', test_driver, 'shcomp', test_name],
                    env: [shell_var, lang_var, src_root, test_root, shcomp_var, ld_library_path,
                          libsample_path, timeout_var])
            endif
        endif
    endif
endforeach
