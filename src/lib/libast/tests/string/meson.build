string_test_files = [ 'utf.c' ]

incdir = include_directories('..',
                             '../../include/')

foreach file: string_test_files
    test_target = executable(
        file, file, c_args: shared_c_args,
        include_directories: [configuration_incdir, incdir],
        link_with: [libast, libenv],
        install: false)
    test('API/string/' + file, test_target)
endforeach
