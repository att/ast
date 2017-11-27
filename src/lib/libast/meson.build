libast_files = ['features/conftab.c']
incdir = include_directories('include', 'features', 'aso', 'cdt', 'comp',
                             'sfio', 'path', 'port', 'string', 'misc', 'std',
                             'vmalloc', 'tm')

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

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required: false)
iconv_dep = cc.find_library('iconv', required: false)

libast = library('ast', libast_files,
                 include_directories: incdir,
                 c_args: ['-D_BLD_ast', '-D_API_ast=20130625'],
                 dependencies: [m_dep, iconv_dep])
