libdll_files = ['dlfcn.c', 'dllcheck.c',  'dllerror.c',  'dllfind.c', 'dlllook.c',
                'dllnext.c',  'dllopen.c', 'dllplug.c', 'dllscan.c']

libdll_c_args = shared_c_args + [
    '-D_BLD_Dll',
]

libdll = library('dll', libdll_files, c_args: libdll_c_args,
                 include_directories: [configuration_incdir, libast_incdir],
                 dependencies: libfts_dep,
                 link_with: libast,
                 install: get_option('default_library') == 'shared')

libsample_files = ['sample.c']
libsample = shared_library('sample', libsample_files,
                           include_directories: [configuration_incdir, libast_incdir],
                           install: false)
