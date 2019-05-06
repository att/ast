# Each entry in `api_tests` is an array of one or two elements. The first
# element is the test name. The second is an optional timeout if the default
# timeout of 30 seconds is too short. Try to keep the list sorted.
api_default_timeout = 30

lang_var = 'LANG=en_US.UTF-8'

api_tests = [
    ['debug'],
]

# Some tests need to be skipped because the platform has known incompatibilities that we're not
# interested in dealing with. For example, `printf("%p", ptr)` does not include a leading `0x` on
# Solaris. Unlike every other platform.
tests_to_skip = [
    ['sunos', 'debug'],
]

foreach testspec : api_tests
    testname = testspec[0]
    timeout = (testspec.length() == 2) ? testspec[1] : api_default_timeout
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
        test_target = executable(
            testname, testname + '.c', c_args: shared_c_args,
            include_directories: [configuration_incdir, ksh93_incdir],
            link_with: [libksh, libast, libcmd, libdll],
            dependencies: [libm_dep, libexecinfo_dep, libdl_dep],
            install: false)
        test('API/ksh/' + testname, ksh93_exe, timeout: timeout, is_parallel: parallel,
            args: [test_driver, 'api', test_target, testname],
            env: [shell_var, lang_var, ld_library_path, libsample_path])
    endif
endforeach
