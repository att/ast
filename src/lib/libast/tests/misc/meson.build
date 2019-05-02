test_dir = meson.current_source_dir()
# TODO: Enable 'opt' tests
tests = [ 'stk' ]
incdir = include_directories('..', '../../include/')

foreach test_name: tests
    test_target = executable(
        test_name, test_name + '.c',
        c_args: shared_c_args,
        include_directories: [configuration_incdir, incdir],
        link_with: [libast, libenv],
        install: false)
    test('API/misc/' + test_name, sh_exe, args: [test_driver, test_target, test_dir])
endforeach
