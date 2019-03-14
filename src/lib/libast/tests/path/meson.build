path_test_files = [ 'pathaccess.c', 'pathcat.c', 'pathbin.c', 'pathgetlink.c', 'pathstat.c' ]

incdir = include_directories('..',
                             '../../include/')

foreach file: path_test_files
    test_target = executable('path_' + file, file, c_args: shared_c_args,
                             include_directories: [configuration_incdir, incdir],
                             link_with: [libast, libenv],
                             install: false)
    test('API/path/' + file, test_target)
endforeach
