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

# Some tests are known to fail on some platforms. For example, tests involving UTF-8 <=> UTF-32
# fail on Cygwin because sizeof(wchar_t) == 2.
tests_to_skip = [
    ['cygwin', 'utf'],
]

incdir = include_directories('..', '../../include/')

foreach test_name: tests
    skip_test = false
    foreach skip : tests_to_skip
        if system == skip[0] and test_name == skip[1]
            warning('skipping API/string/' + test_name + ' on ' + system)
            skip_test = true
            # break  # Not until meson 0.49
        endif
    endforeach
    if not skip_test
        test_target = executable(
            test_name, test_name + '.c',
            c_args: shared_c_args,
            include_directories: [configuration_incdir, incdir],
            link_with: [libast, libenv],
            install: false)
        test('API/string/' + test_name, sh_exe, args: [test_driver, test_target, test_dir])
    endif
endforeach
