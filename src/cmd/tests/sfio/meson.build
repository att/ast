sfio_test_files =[ 'talarm.c', 'talign.c', 'tappend.c', 'tatexit.c',
                   'tbadargs.c', 'tclose.c', 'tdisc.c', 'terrno.c', 'texcept.c',
                   'tflags.c', 'tfmt.c', 'tgetr.c', 'thole.c', 'tleak.c',
                   'tlocale.c', 'tlongdouble.c', 'tmmap2read.c', 'tmode.c',
                   'tmove.c', 'tmprdwr.c', 'tmpread.c', 'tmprocess.c',
                   'tmtsafe.c', 'tmultiple.c', 'tmwrite.c', 'tnoseek.c',
                   'tnotify.c', 'topen.c', 'tpipe.c', 'tpipemove.c', 'tpkrd.c',
                   'tpoll.c', 'tpool.c', 'tpopen.c', 'tpopenrw.c', 'tpublic.c',
                   'tputgetc.c', 'tputgetd.c', 'tputgetl.c', 'tputgetm.c',
                   'tputgetr.c', 'tputgetu.c', 'trcrv.c', 'treserve.c',
                   'tresize.c', 'tscanf.c', 'tscanf1.c', 'tseek.c', 'tsetbuf.c',
                   'tsetfd.c', 'tsfstr.c', 'tshare.c', 'tsize.c', 'tstack.c',
                   'tstatus.c',  'tstkpk.c', 'tstring.c', 'tswap.c', 'tsync.c',
                   'ttell.c', 'ttmp.c', 'ttmpfile.c', 'tungetc.c', 'twalk.c',
                   'twhole.c', 'twrrd.c']

# TODO: Fix and enable these test cases.
# 'tprintf.c', 'tstdio.c', 'twc.c', 'twchar.c',

incdir = include_directories('..',
                             '../../../lib/libast/include/')

foreach file: sfio_test_files
    sfio_test_target = executable(file, file,
                             include_directories: [configuration_incdir, incdir],
                             link_with: [libast ],
                             install: false)
    test('API/' + file, sfio_test_target)
endforeach
