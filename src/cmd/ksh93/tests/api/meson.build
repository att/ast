# Each entry in `api_tests` is an array of one or two elements. The first
# element is the test name. The second is an optional timeout if the default
# timeout of 30 seconds is too short. Try to keep the list sorted.
api_default_timeout = 30

lang_var = 'LANG=en_US.UTF-8'

api_tests = [
    ['ksh_debug'],
]

# Some tests need to be skipped because the platform has known incompatibilities that we're not
# interested in dealing with. For example, `printf("%p", ptr)` does not include a leading `0x` on
# Solaris. Unlike every other platform.
tests_to_skip = [
    ['sunos', 'ksh_debug'],
]

# ASAN has probably installed a SIGSEGV handler. The `ksh_debug` API test also does so. The two may
# not be compatible. So avoid false negative test failures when ASAN is enabled.
if get_option('ASAN') == true
    tests_to_skip += [['*', 'ksh_debug']]
endif

foreach testspec : api_tests
    test_name = testspec[0]
    timeout = (testspec.length() == 2) ? testspec[1] : api_default_timeout
    if timeout <= 0
        parallel = false
        timeout = timeout == 0 ? default_timeout : -timeout
    else
        parallel = true
    endif

    skip_test = false
    foreach skip : tests_to_skip
        if ('*' == skip[0] or system == skip[0]) and test_name == skip[1]
            warning('skipping ' + test_name + ' on ' + system)
            skip_test = true
            # break  # Not until meson 0.49
        endif
    endforeach
    if not skip_test
        test_target = executable(
            test_name, test_name + '.c', c_args: shared_c_args,
            include_directories: [configuration_incdir, ksh93_incdir],
            link_with: [libksh, libast, libcmd, libdll],
            dependencies: [libm_dep, libexecinfo_dep, libdl_dep],
            install: false)
        test('API/ksh/' + test_name, ksh93_exe, timeout: timeout, is_parallel: parallel,
            args: [test_driver, 'api', test_target, test_name],
            env: [shell_var, lang_var, src_root, test_root, ld_library_path, libsample_path])
    endif
endforeach
