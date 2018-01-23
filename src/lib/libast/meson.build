libast_files = ['features/conftab.c']
incdir = include_directories('include', 'features', 'aso', 'cdt', 'comp',
                             'sfio', 'path', 'port', 'string', 'misc',
                             'vmalloc', 'tm')
libast_c_args = ['-D_BLD_ast', '-D_API_ast=20130625']
cc = meson.get_compiler('c')

syslocale_result = run_command('features/syslocale.sh')

if syslocale_result.returncode() == 0
    syslocale_output = syslocale_result.stdout().strip()
    libast_c_args += [ '-D_locale_utf8_str=' + syslocale_output ]
endif

libpath_result = run_command('features/libpath.sh')

if libpath_result.returncode() == 0
    libpath_output = libpath_result.stdout().strip()
    libast_c_args += [ '-DCONF_LIBPATH=' + libpath_output ]
endif

prog_result = run_command('features/prog.sh')

if prog_result.returncode() == 0
    prog_output = prog_result.stdout().strip()
    libast_c_args += [ '-D_PROC_PROG=' + prog_output ]
endif

align_feature_file = files('align_feature.c')
align_feature_result = cc.run(align_feature_file, name: 'align bound check')
if not align_feature_result.compiled()
    error('Unable to compile the align_feature.c module to determine compiler alignment')
endif
libast_c_args += align_feature_result.stdout().split()

if not cc.has_function('isnanl', prefix: '#include <math.h>')
    libast_c_args += [ '-Disnanl=isnan' ]
endif

if cc.has_function('utimensat', prefix: '#include <sys/stat.h>')
    libast_c_args += [ '-D_lib_utimensat' ]
elif cc.has_function('utimets', prefix: '#include <sys/stat.h>')
    libast_c_args += [ '-D_lib_utimets' ]
endif

if cc.has_function('sysinfo', prefix: '#include <sys/sysinfo.h>')
    libast_c_args += [ '-D_lib_sysinfo=1' ]
endif

if cc.has_function('pipe2', prefix: '#include <unistd.h>')
    libast_c_args += [ '-D_lib_pipe2=1' ]
endif

if cc.has_member('struct stat','st_mtim', prefix: '#include <sys/stat.h>')
    libast_c_args += [ '-DSTAT_ST_MTIM=1' ]
elif cc.has_member('struct stat', 'st_mtimespec', prefix: '#include <sys/stat.h>')
    # Under macOS modification time field is defined as mtimespec
    libast_c_args += [ '-DSTAT_ST_MTIMESPEC=1' ]
else
    error('Can not determine modification time field in struct stat')
endif

if cc.has_header('mach-o/dyld.h')
    libast_c_args += [ '-D_hdr_mach_o_dyld']
endif

libast_c_args += [ '-D_ast_sizeof_int=@0@'.format(cc.sizeof('int')) ]
libast_c_args += [ '-D_ast_sizeof_long=@0@'.format(cc.sizeof('long')) ]
libast_c_args += [ '-D_ast_sizeof_size_t=@0@'.format(cc.sizeof('size_t')) ]
libast_c_args += [ '-D_ast_sizeof_off_t=@0@'.format(cc.sizeof('off_t')) ]
libast_c_args += [ '-D_ast_sizeof_pointer=@0@'.format(cc.sizeof('void*')) ]

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

libm_dep = cc.find_library('m', required: false)
# On OpenBSD libiconv is in /usr/local/lib rather than /usr/lib.
libiconv_dep = cc.find_library('iconv', required: false, dirs: ['/usr/lib', '/usr/local/lib'])
# On Cygwin the message catalog functions (e.g., `catopen()`) are in this
# library.
libcatgets_dep = cc.find_library('catgets', required: false)

libast = library('ast', libast_files,
                 include_directories: incdir,
                 c_args: libast_c_args,
                 dependencies: [libm_dep, libiconv_dep, libcatgets_dep],
                 install: get_option('default_library') == 'shared')
