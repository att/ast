# Each entry in `api_tests` is an array of one or two elements. The first
# element is the test name. The second is an optional timeout if the default
# timeout of 30 seconds is too short. Try to keep the list sorted.
api_default_timeout = 30

api_tests = [
    ['debug'],
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

    # Run the test without compiling the script (which is how most people use ksh).
    lang_var = 'LANG=en_US.UTF-8'

    test_exe = executable(testname, testname + '.c', c_args: shared_c_args,
                          include_directories: [configuration_incdir, ksh93_incdir],
                          link_with: [libksh, libast, libcmd, libdll],
                          dependencies: [libm_dep, libexecinfo_dep, libdl_dep],
                          install: false)

    #test('API/' + testname, test_exe, timeout: timeout, is_parallel: parallel,
    #     env: [shell_var, lang_var])
    test('API/' + testname, ksh93_exe, timeout: timeout, is_parallel: parallel,
         args: [test_driver, 'api', test_exe, testname],
         env: [shell_var, lang_var, ld_library_path, libsample_path])
endforeach
