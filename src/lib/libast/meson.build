libast_files = ['features/conftab.c']
incdir = include_directories('include', 'features', 'aso', 'cdt', 'comp',
                             'sfio', 'path', 'port', 'string', 'misc', 'std',
                             'vmalloc', 'tm')
libast_c_args = ['-D_BLD_ast', '-D_API_ast=20130625']
cc = meson.get_compiler('c')

syslocale_result = run_command('features/syslocale.sh')


if syslocale_result.returncode() == 0
    syslocale_output = syslocale_result.stdout()
    libast_c_args += [ '-D_locale_utf8_str=' + syslocale_output ]
endif

libpath_result = run_command('features/libpath.sh')


if libpath_result.returncode() == 0
    libpath_output = libpath_result.stdout()
    libast_c_args += [ '-DCONF_LIBPATH=' + libpath_output ]
endif

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

m_dep = cc.find_library('m', required: false)
iconv_dep = cc.find_library('iconv', required: false)

libast = library('ast', libast_files,
                 include_directories: incdir,
                 c_args: libast_c_args,
                 dependencies: [m_dep, iconv_dep],
                 install: true)
