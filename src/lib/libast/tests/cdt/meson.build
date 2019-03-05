# Each entry in `all_tests` is an array of one or two elements. The first
# element is the test name. The second is an optional timeout if the default
# timeout of 30 seconds is too short. Try to keep the list sorted.
default_timeout = 30

# TODO: Enable these tests when they are fixed to work reliably. At the moment these
# timeout or fail on most platforms:
#   ['tsafehash.c', 120], ['tsafetree.c', 120],
cdt_test_files = [ ['tannounce.c'], ['tbags.c'], ['tdeque.c'], ['tevent.c'],
                   ['tinstall.c'], ['tlist.c'], ['tobag.c'], ['tqueue.c'],
                   ['trhbags.c'], ['tsearch.c'],
                   ['tshare.c'], ['tstack.c'], ['tstringset.c'], ['tuser.c'],
                   ['tvthread.c'], ['twalk.c'], ['tview.c'], ['trehash.c'],
                 ]

incdir = include_directories('..',
                             '../../include/')

foreach testspec: cdt_test_files
    testname = testspec[0]
    timeout = (testspec.length() == 2) ? testspec[1] : default_timeout
    # Add cdt prefix to avoid name clashes with other tests with same name
    cdt_test_target = executable('cdt_' + testname, testname, c_args: shared_c_args,
                             include_directories: [configuration_incdir, incdir],
                             link_with: [libast, libenv],
                             link_args: ['-lpthread'],
                             install: false)
    test('API/' + testname, cdt_test_target, timeout: timeout)
endforeach
