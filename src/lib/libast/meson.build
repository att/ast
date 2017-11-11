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

library('ast', files, include_directories : incdir)
