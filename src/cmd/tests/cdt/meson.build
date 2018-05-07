cdt_test_files = [ 'tannounce.c', 'tbags.c', 'tdeque.c', 'tevent.c',
                   'tinstall.c', 'tlist.c', 'tobag.c', 'tqueue.c', 'trehash.c',
                   'trhbags.c', 'tsafehash.c', 'tsafetree.c', 'tsearch.c',
                   'tshare.c', 'tstack.c', 'tstringset.c', 'tuser.c',
                   'tvsafehash.c', 'tvsaferehash.c', 'tvsafetree.c',
                   'tvthread.c', 'twalk.c', 'tview.c' ]

incdir = include_directories('..',
                             '../../../lib/libast/include/')

foreach file: cdt_test_files
    # Add cdt prefix to avoid name clashes with other tests with same name
    cdt_test_target = executable('cdt' + file, file, c_args: shared_c_args,
                             include_directories: [configuration_incdir, incdir],
                             link_with: [libast],
                             link_args: ['-lpthread'],
                             install: false)
    test('API/' + file, aso_test_target)
endforeach
