test_dir = meson.current_source_dir()

# TODO: Enable these tests when they are fixed to work reliably. At the moment these
# timeout or fail on most platforms:
#   ['tsafehash.c', 120], ['tsafetree.c', 120],
tests = ['tannounce', 'tbags', 'tdeque', 'tdict', 'tdtstack', 'tevent', 'tinstall', 'tlist',
         'tobag', 'tqueue', 'trhbags', 'tsearch', 'tstringset', 'tuser', 'tvthread', 'twalk',
         'tview', 'trehash']

incdir = include_directories('..', '../../include/')

foreach test_name: tests
    test_target = executable(
        test_name, test_name + '.c',
        c_args: shared_c_args,
        include_directories: [configuration_incdir, incdir],
        link_with: [libast, libenv],
        link_args: ['-lpthread'],
        install: false)
    test('API/cdt/' + test_name, sh_exe, args: [test_driver, test_target, test_dir])
endforeach
