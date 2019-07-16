test_dir = meson.current_source_dir()
# TODO: Enable 'opt' tests
tests = [ 'debug', 'stk', 'environ' ]
incdir = include_directories('..', '../../include/')

foreach test_name: tests
    # If the platform doesn't have an execinfo.h header there is no point in trying to run the
    # debug.c unit test.
    if feature_data.get('_hdr_execinfo', 0) == 0
        continue
    endif
    test_target = executable(
        test_name, test_name + '.c',
        c_args: shared_c_args,
        include_directories: [configuration_incdir, incdir],
        link_with: [libast, libenv],
        install: false)
    test('API/misc/' + test_name, sh_exe, args: [test_driver, test_target, test_dir])
endforeach
