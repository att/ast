test_dir = meson.current_source_dir()
tests =['talarm', 'talign', 'tappend', 'tatexit', 'tbadargs', 'tclose', 'terrno', 'texcept',
        'tflags', 'tfmt', 'tgetr', 'thole', 'tleak', 'tlocale', 'tlongdouble',  'tmode', 'tmove',
        'tmprdwr', 'tmpread', 'tmprocess', 'tmtsafe', 'tmultiple', 'tmwrite', 'tnoseek', 'tnotify',
        'topen', 'tpipe', 'tpipemove', 'tpkrd', 'tpool', 'tpopen', 'tpopenrw', 'tpublic',
        'tputgetc', 'tputgetd', 'tputgetl', 'tputgetm', 'tputgetr', 'tputgetu', 'trcrv', 'treserve',
        'tresize', 'tscanf', 'tscanf1', 'tseek', 'tsetbuf', 'tsetfd', 'tsfstr', 'tshare', 'tsize',
        'tstack', 'tstatus',  'tstkpk', 'tstring', 'tswap', 'tsync', 'ttell', 'ttmp', 'ttmpfile',
        'tungetc', 'twhole', 'twrrd', 'tprintf']

# TODO: This test fails due to a use after free bug. Enable it when that is fixed. For some reason
# this affects Linux systems but not BSD systems. It results in a SIGSEGV on the `sfclose(fs);`
# statement. Apparently the earlier `sfstack(f, fs);` call to create a reference to the `f` handle
# did not ensure the underlying Sfio_t structure was not freed when `f` was closed.
#
# 'tdisc.c'

incdir = include_directories('..', '../../include/', '../../sfio/')

foreach test_name: tests
    if system == 'cygwin' and (test_name == 'tclose' or test_name == 'treserve')
        warning('skipping API/sfio/' + test_name + ' on cygwin')
    else
        test_target = executable(
            test_name, test_name + '.c',
            c_args: shared_c_args,
            include_directories: [configuration_incdir, incdir],
            link_with: [libast, libenv],
            install: false)
        test('API/sfio/' + test_name, sh_exe, args: [test_driver, test_target, test_dir])
    endif
endforeach
