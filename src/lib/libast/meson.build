# The conftab.c file is unusual in that it is dynamically generated during the Meson config step
# and is placed in the build tree.
libast_files = [meson.build_root() + '/src/lib/libast/comp/conftab.c']
libast_incdir = include_directories('include', 'features', 'aso', 'cdt', 'comp',
                                    'sfio', 'path', 'port', 'string', 'misc',
                                    'tm')
libast_c_args = shared_c_args + ['-D_BLD_ast']

subdir('aso')
subdir('comp')
subdir('cdt')
subdir('disc')
subdir('hash')
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
                 dependencies: [libm_dep, libiconv_dep, libcatgets_dep],
                 install: get_option('default_library') == 'shared')
