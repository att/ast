files = [ 'features/conftab.c' ]
incdir = include_directories('include', 'features', 'aso', 'cdt', 'comp', 'sfio', 'path', 'port', 'string', 'misc', 'std', 'vmalloc', 'tm' )

subdir('aso')
subdir('comp')
subdir('cdt')
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
m_dep = cc.find_library('m', required : false)
iconv_dep = cc.find_library('iconv', required : false)

library('ast', files, include_directories : incdir, dependencies : [m_dep, iconv_dep])
