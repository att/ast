test_files = [ 'base64.c' ]

# TODO: Build and enable these test cases.
# 'date.c', 'ip6.c', 'opt.c', 'strelapsed.c', 'strtof.c', 'strtoi.c'

incdir = include_directories('../../lib/libast/include/')

foreach file: test_files
    test_target = executable(file, file, c_args: shared_c_args,
                             include_directories: [configuration_incdir, incdir],
                             link_with: [libast],
                             install: false)
    test('API/' + file, test_target)
endforeach

subdir('aso')
subdir('cdt')
subdir('sfio')
subdir('vmalloc')
