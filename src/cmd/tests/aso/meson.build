aso_test_files =['taso.c', 'tproc.c', 'tthread.c', 'tlock.c']

incdir = include_directories('..',
                             '../../../lib/libast/include/')

foreach file: aso_test_files
    aso_test_target = executable(file, file, c_args: shared_c_args,
                             include_directories: [configuration_incdir, incdir],
                             link_with: [libast],
                             link_args: ['-lpthread'],
                             install: false)
    test('API/' + file, aso_test_target)
endforeach
