# These modules contains builtins for standard commands.
libcmd_files = [
    'basename.c',
    'cat.c',
    'chmod.c',
    'cmdinit.c',
    'cmp.c',
    'cut.c',
    'dirname.c',
    'head.c',
    'logname.c',
    'mkdir.c',
    'sync.c',
    'uname.c',
    'wc.c',
]

libcmd = library('cmd', libcmd_files, c_args: shared_c_args,
                 include_directories: [configuration_incdir, ksh93_incdir],
                 dependencies: libfts_dep,
                 link_with: libast,
                 install: get_option('default_library') == 'shared')
