# TODO: Add `tsignal.c` when issue #525 is resolved.
vmalloc_test_files = [ 'talign.c', 'tcache-scratch.c', 'tcache-thrash.c',
                       'tcompact.c', 'tcontent.c', 'tdcshare.c', 'tdebug.c',
                       'tek.c', 'theap.c', 'tlast.c', 'tmalloc.c',
                       'tmmaddress.c', 'tmtmalloc.c',
                       'tperform.c', 'tpool.c', 'trandom.c', 'tregion.c',
                       'tresize.c', 'tsafemalloc.c', 'tsegmented.c',
                       'tsegwalk.c', 'tsharemem.c', 'tsharesimple.c',
                       'tsinglemem.c', 'tsmall.c', 'tstat.c' ]

# TODO: Fix and enable this test case
# 'tmallocdebug.c',

incdir = include_directories('..',
                             '../../../lib/libast/include/',
                             '../../../lib/libast/vmalloc/')

foreach file: vmalloc_test_files
    vmalloc_test_target = executable('vmalloc' + file, file,
                                     c_args: shared_c_args,
                                     include_directories: [configuration_incdir, incdir],
                                     link_with: [libast],
                                     link_args: ['-lpthread'],
                                     install: false)
    test('API/' + file, vmalloc_test_target)
endforeach
