string_test_files = [ 'base64.c', 'chresc.c', 'fmtbase.c', 'fmtesc.c',
                      'fmtelapsed.c', 'fmtfmt.c', 'fmtint.c', 'fmtmode.c',
                      'fmtre.c', 'fmtperm.c', 'fmtscale.c', 'fmttmx.c',
                      'fmtre.c', 'utf.c' ]

incdir = include_directories('..',
                             '../../include/')

foreach file: string_test_files
    test_target = executable(
        'string_' + file, file, c_args: shared_c_args,
        include_directories: [configuration_incdir, incdir],
        link_with: [libast, libenv],
        install: false)
    test('API/string/' + file, test_target)
endforeach
