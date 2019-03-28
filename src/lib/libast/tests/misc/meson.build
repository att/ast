test_dir = meson.current_source_dir()
tests = ['opt']

incdir = include_directories('..', '../../include/')

foreach test_name: tests
    test_target = executable(
        test_name, test_name + '.c',
        c_args: shared_c_args,
        include_directories: [configuration_incdir, incdir],
        link_with: [libast, libenv],
        install: false)
    # TODO: Enable all these test cases.
    #
    # For the moment we only build the programs to verify they can be built and to enable linting
    # them. Running the tests will require converting the *.rt and *.tst files into scripts.
    #
    # test('API/misc/' + test_name, sh_exe, args: [test_driver, test_target, test_dir])
endforeach
