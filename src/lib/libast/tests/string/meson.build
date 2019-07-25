test_dir = meson.current_source_dir()
tests = [
    'base64',
    'chresc',
    'fmtbase',
    'fmtelapsed',
    'fmtesc',
    'fmtfmt',
    'fmtint',
    'fmtmatch',
    'fmtmode',
    'fmtperm',
    'fmtre',
    'fmtscale',
    'fmttmx',
    'strelapsed',
    'stresc',
    'strlcat',
    'strlcpy',
    'strmatch',
    'strperm',
    'strsort',
    'strtonll',
    'struniq',
    'utf',
]

incdir = include_directories('..', '../../include/')

foreach test_name: tests
    test_target = executable(
        test_name, test_name + '.c',
        c_args: shared_c_args,
        include_directories: [configuration_incdir, incdir],
        link_with: [libast, libenv],
        install: false)
    test('API/string/' + test_name, sh_exe, args: [test_driver, test_target, test_dir])
endforeach
