aso_test_files =['taso.c', 'tlock.c']

incdir = include_directories('..',
                             '../../include/')

foreach file: aso_test_files
    test_target = executable(file, file, c_args: shared_c_args,
                             include_directories: [configuration_incdir, incdir],
                             link_with: [libast, libenv],
                             link_args: ['-lpthread'],
                             install: false)
    # TODO: Figure out how to make these tests more efficient so we don't need such an absurdly long
    # timeout in order to keep these tests from timing out on OpenBSD. See issue #483.
    test('API/aso/' + file, test_target)
endforeach
