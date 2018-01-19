pty_files = ['pty.c']
pty_c_args = shared_c_args + [
    '-DUSAGE_LICENSE=""',
    '-DERROR_CATALOG=0',
    '-D_XOPEN_SOURCE=700',
]
builtin_incdir = include_directories('features',
                                     '../../lib/libast/include/',
                                     '../../lib/libast/features/',
                                     '../../lib/libcmd/')

lib_util_dep = cc.find_library('util', required: false)

openpty_hdrs = '#include <stdlib.h>'
if cc.has_header('pty.h')
    pty_c_args += ['-D_hdr_pty']
    openpty_hdrs += '\n#include <pty.h>'
endif
if cc.has_header('util.h')
    pty_c_args += ['-D_hdr_util']
    openpty_hdrs += '\n#include <util.h>'
endif

if cc.has_function('posix_openpt', prefix: '#include <stdlib.h>',
                   args: pty_c_args,
                   dependencies: [lib_util_dep])
    pty_c_args += ['-D_lib_posix_openpt']
endif

if cc.has_function('openpty', prefix: openpty_hdrs,
                   args: pty_c_args,
                   dependencies: [lib_util_dep])
    pty_c_args += ['-D_lib_openpty']
endif

if cc.has_function('ptsname', prefix: '#include <stdlib.h>',
                   args: pty_c_args,
                   dependencies: [lib_util_dep])
    pty_c_args += ['-D_lib_ptsname']
endif

if cc.has_function('grantpt', prefix: '#include <stdlib.h>',
                   args: pty_c_args,
                   dependencies: [lib_util_dep])
    pty_c_args += ['-D_lib_grantpt']
endif

if cc.has_function('unlockpt', prefix: '#include <stdlib.h>',
                   args: pty_c_args,
                   dependencies: [lib_util_dep])
    pty_c_args += ['-D_lib_unlockpt']
endif

pty_feature_file = files('pty_feature.c')
pty_feature_result = cc.run(pty_feature_file, name: 'pty dev name check', args: pty_c_args)
if not pty_feature_result.compiled()
    error('Unable to compile the pty_feature.c module to determine pty device names')
endif
pty_c_args += pty_feature_result.stdout().split()
pty_c_args += ['-DCMD_STANDALONE=b_pty']

pty_exe = executable('pty', pty_files, c_args: pty_c_args,
    include_directories: builtin_incdir,
    dependencies: [lib_util_dep],
    link_with: [libast, libcmd],
    install: true)
