path_test_files =['pathcat.c', 'pathbin.c']

incdir = include_directories('..',
                             '../../include/')

foreach file: path_test_files
    aso_test_target = executable(file, file, c_args: shared_c_args,
                             include_directories: [configuration_incdir, incdir],
                             link_with: [libast, libenv],
                             install: false)
    test('API/path/' + file, aso_test_target)
endforeach
