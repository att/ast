libcmd_files = ['context.c', 'cmdinit.c', 'stty.c']
libcmd_c_args = shared_c_args + [
    '-DUSAGE_LICENSE=""',
    '-DERROR_CATALOG=0',
]
incdir = include_directories('../libast/include', '../libast/features/')

libcmd = library('cmd', libcmd_files, c_args: libcmd_c_args,
                 include_directories: [configuration_incdir, incdir],
                 link_with: libast,
                 install: get_option('default_library') == 'shared')
