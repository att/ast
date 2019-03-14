string_test_files = [ 'base64.c', 'opt.c', 'strelapsed.c' ]

incdir = include_directories('..',
                             '../../include/')

foreach file: string_test_files
    test_target = executable(
        file, file, c_args: shared_c_args,
        include_directories: [configuration_incdir, incdir],
        link_with: [libast, libenv],
        install: false)
    # TODO: Enable all these test cases.
    #
    # For the moment we only build the programs to verify they can be built and to enable linting
    # them. Running the tests will require converting the *.rt and *.tst files into scripts.
    #
    if file == 'base64.c'
        test('API/misc/' + file, test_target)
    endif
endforeach
