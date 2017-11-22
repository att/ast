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

ksh93_exe = executable('ksh', ksh93_files, c_args: ksh93_c_args , include_directories: ksh93_incdir, link_with: [ libast, libcmd, libcoshell], dependencies : [ m_dep ] )

test_dir = join_paths(meson.current_source_dir(), 'tests')

# This variable is used by some tests while executing subtests
shell_var = 'SHELL=' + ksh93_exe.full_path()

# These are the default locales used by legacy test script
# '' is POSIX locale
# TODO: Check how ksh code handles '' string while setting locales and explicitly set string to 'POSIX' here
locales = [ '', 'C.UTF-8' ]

# Skipping builtin_poll.sh
all_tests = [ 'alias.sh', 'append.sh', 'arith.sh', 'arrays.sh', 'arrays2.sh', 'attributes.sh', 'basic.sh', 'bracket.sh', 'builtins.sh', 'case.sh', 'comvar.sh', 'comvario.sh', 'coprocess.sh', 'cubetype.sh', 'directoryfd.sh', 'enum.sh', 'exit.sh', 'expand.sh', 'functions.sh', 'glob.sh', 'grep.sh', 'heredoc.sh', 'io.sh', 'leaks.sh', 'locale.sh', 'math.sh', 'nameref.sh', 'namespace.sh', 'options.sh', 'path.sh', 'pointtype.sh', 'pty.sh', 'quoting.sh', 'quoting2.sh', 'readcsv.sh', 'recttype.sh', 'restricted.sh', 'return.sh', 'select.sh', 'sh_match.sh', 'sigchld.sh', 'signal.sh', 'statics.sh', 'subshell.sh', 'substring.sh', 'tilde.sh', 'timetype.sh', 'treemove.sh', 'types.sh', 'variables.sh', 'vartree1.sh', 'vartree2.sh', 'wchar.sh' ]

# The test cases are executed in parallel by default
foreach testname: all_tests
    test_path = join_paths(test_dir, testname)
    # Strip out '.sh' from filename
    # This makes it easy to execute tests by name. For e.g. meson test alias
    # Tests should not have '.' in filename
    testname = testname.split('.')[0]
    foreach locale:locales
        lang_var = 'LANG=' + locale
        if locale == ''
            test(testname, ksh93_exe, args: [test_path], env: [shell_var, lang_var])
        else
            test(testname + '(' + locale + ')', ksh93_exe, args: [test_path], env: [shell_var, lang_var])
        endif
    endforeach
endforeach
