sfio_test_files =[ 'talarm.c', 'talign.c', 'tappend.c', 'tatexit.c',
                   'tbadargs.c', 'tclose.c', 'terrno.c', 'texcept.c',
                   'tflags.c', 'tfmt.c', 'tgetr.c', 'thole.c', 'tleak.c',
                   'tlocale.c', 'tlongdouble.c',  'tmode.c',
                   'tmove.c', 'tmprdwr.c', 'tmpread.c', 'tmprocess.c',
                   'tmtsafe.c', 'tmultiple.c', 'tmwrite.c', 'tnoseek.c',
                   'tnotify.c', 'topen.c', 'tpipe.c', 'tpipemove.c', 'tpkrd.c',
                   'tpool.c', 'tpopen.c', 'tpopenrw.c', 'tpublic.c',
                   'tputgetc.c', 'tputgetd.c', 'tputgetl.c', 'tputgetm.c',
                   'tputgetr.c', 'tputgetu.c', 'trcrv.c', 'treserve.c',
                   'tresize.c', 'tscanf.c', 'tscanf1.c', 'tseek.c', 'tsetbuf.c',
                   'tsetfd.c', 'tsfstr.c', 'tshare.c', 'tsize.c', 'tstack.c',
                   'tstatus.c',  'tstkpk.c', 'tstring.c', 'tswap.c', 'tsync.c',
                   'ttell.c', 'ttmp.c', 'ttmpfile.c', 'tungetc.c', 'twalk.c', 'twhole.c',
                   'twrrd.c', 'tprintf.c' ]

# TODO: This test fails due to a use after free bug. Enable it when that is fixed. For some reason
# this affects Linux systems but not BSD systems. It results in a SIGSEGV on the `sfclose(fs);`
# statement. Apparently the earlier `sfstack(f, fs);` call to create a reference to the `f` handle
# did not ensure the underlying Sfio_t structure was not freed when `f` was closed.
#
# 'tdisc.c'


incdir = include_directories('..', '../../../lib/libast/include/')

foreach file: sfio_test_files
    sfio_test_target = executable(file, file,
                             include_directories: [configuration_incdir, incdir],
                             link_with: [libast, libenv],
                             install: false)
    test('API/' + file, sfio_test_target)
endforeach
