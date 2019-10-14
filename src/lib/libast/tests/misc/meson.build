test_dir = meson.current_source_dir()
# TODO: Enable 'opt' tests
tests = [ 'debug', 'glob', 'optget_long', 'stk', 'environ' ]
incdir = include_directories('..', '../../include/')
#
# Some tests fail for inexplicable reasons on some platforms. In some cases, such as Cygwin, things
# simply don't work as expected and probably never will due to quirks of the platform. Another
# example is sunos/solaris has /proc but doesn't support manipulating directory fd's.
tests_to_skip = [
    ['freebsd', 'debug'],
]

# Non-debug build types affect the behavior of `backtrace()`. Which means it will almost certainly
# break the "debug" test. See https://github.com/att/ast/issues/1390.
if get_option('buildtype') != 'debug'
    tests_to_skip += [['*', 'debug']]
endif

foreach test_name: tests
    # If the platform doesn't have an execinfo.h header there is no point in trying to run the
    # debug.c unit test.
    if feature_data.get('_hdr_execinfo', 0) == 0
        continue
    endif
    skip_test = false
    foreach skip : tests_to_skip
        if ('*' == skip[0] or system == skip[0]) and test_name == skip[1]
            warning('skipping ' + test_name + ' on ' + system)
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
        test('API/misc/' + test_name, sh_exe, args: [test_driver, test_target, test_dir])
    endif
endforeach
