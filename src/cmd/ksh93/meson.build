ksh93_files = [ ]
ksh93_incdir = include_directories('include', 'features', '../../lib/libast/include/', '../../lib/libast/features/', '../../lib/libcmd/', '../../lib/libcoshell/')

ksh93_c_args = [ '-DERROR_CATALOG=0' ]
ksh93_c_args +=  [ '-DSHOPT_BRACEPAT' ]
ksh93_c_args += [ '-DSHOPT_SUID_EXEC' ]
ksh93_c_args += [ '-DSHOPT_BASH' ]
ksh93_c_args += [ '-D_BLD_shell' ]
ksh93_c_args += [ '-DSHOPT_STATS' ]
ksh93_c_args += ['-DSHOPT_NAMESPACE' ]
ksh93_c_args += [ '-DSHOPT_PFSH' ]
ksh93_c_args += ['-DSHOPT_KIA' ]
ksh93_c_args += [ '-DSHOPT_COMPLETE' ]
ksh93_c_args += [ '-DSHOPT_OPTIMIZE' ]
ksh93_c_args += [ '-DSHOPT_RAWONLY' ]
ksh93_c_args += [ '-D_PACKAGE_ast' ]
ksh93_c_args += [ '-DSHOPT_FIXEDARRAY' ]
ksh93_c_args += [ '-DSHOPT_MULTIBYTE' ]
ksh93_c_args += [ '-DKSHELL' ]
ksh93_c_args += [ '-DSHOPT_COSHELL' ]
#ksh93_c_args += [ '-DSHOPT_DYNAMIC' ]
ksh93_c_args += [ '-DERROR_CONTEXT_T=Error_context_t' ]
ksh93_c_args += [ '-DUSAGE_LICENSE=""' ]
ksh93_c_args += [ '-D_API_ast=20130625' ]

cpu = host_machine.cpu()
system = host_machine.system()

# ksh93_c_args += [ '-DBASH_MACHTYPE="i386-64-unknown-linux"' ]
# This yields 'x86_64-unknown-linux' which is different from nmake which yields
# 'i386-64-unknown-linux' on linux.
bash_machtype = cpu + '-unknown-'+ system
ksh93_c_args += [ '-DBASH_MACHTYPE="' + bash_machtype + '"' ]

subdir('bltins')
subdir('data')
subdir('edit')
subdir('sh')

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)

executable('ksh', ksh93_files, c_args: ksh93_c_args , include_directories: ksh93_incdir, link_with: [ libast, libcmd, libcoshell], dependencies : [ m_dep ] )
