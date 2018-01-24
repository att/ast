if get_option('build-pty') == true
    pty_files = ['pty.c']
    pty_c_args = shared_c_args + [
        '-DERROR_CATALOG=NULL',
        '-DCMD_STANDALONE=b_pty',
    ]
    builtin_incdir = include_directories('features',
                                         '../../lib/libast/include/',
                                         '../../lib/libast/features/',
                                         '../../lib/libcmd/')

    pty_exe = executable('pty', pty_files, c_args: pty_c_args,
        include_directories: builtin_incdir,
        dependencies: [libutil_dep],
        link_with: [libast, libcmd],
        install: true)
endif
