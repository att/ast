test_files = [ 'base64.c', 'date.c', 'opt.c', 'strelapsed.c' ]

incdir = include_directories('../../lib/libast/include/')

foreach file: test_files
    test_target = executable(file, file, c_args: shared_c_args,
                             include_directories: [configuration_incdir, incdir],
                             link_with: [libast, libenv],
                             install: false)
    # TODO: Enable these test cases.
    #
    # For the moment we only build the programs to verify they can be built and to enable linting
    # them. Running the tests will require converting the *.rt and *.tst files into scripts.
    #
    # test('API/' + file, test_target)
endforeach

subdir('aso')
subdir('cdt')
subdir('sfio')
