libast_files = []
libast_incdir = include_directories('include', 'aso', 'cdt', 'comp',
                                    'sfio', 'path', 'port', 'string', 'misc',
                                    'tm')
libast_c_args = shared_c_args

subdir('aso')
subdir('comp')
subdir('cdt')
subdir('sfio')
subdir('path')
subdir('port')
subdir('regex')
subdir('string')
subdir('misc')
subdir('tm')

libast = library('ast', libast_files,
                 include_directories: [configuration_incdir, libast_incdir],
                 c_args: libast_c_args,
                 dependencies: [libm_dep, libiconv_dep, libcatgets_dep, libexecinfo_dep, libdl_dep,
                                libsocket_dep, libnsl_dep, libfts_dep],
                 install: get_option('default_library') == 'shared')

# This library exists solely to support libast unit tests so that `sh_getenv()` and
# `sh_setenviron()` has an implementation. These are normally supplied by ksh but the
# libast unit tests don't link against the ksh code.
libenv = library('env', ['misc/environ.c'],
                 include_directories: [configuration_incdir, libast_incdir],
                 c_args: libast_c_args,
                 install: false)
