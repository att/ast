libast_files = ['features/conftab.c']
incdir = include_directories('include', 'features', 'aso', 'cdt', 'comp',
                             'sfio', 'path', 'port', 'string', 'misc',
                             'vmalloc', 'tm')
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
subdir('vmalloc')

libast = library('ast', libast_files,
                 include_directories: incdir,
                 c_args: libast_c_args,
                 dependencies: [libm_dep, libiconv_dep, libcatgets_dep],
                 install: get_option('default_library') == 'shared')
