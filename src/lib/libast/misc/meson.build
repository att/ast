libast_files += [
    'misc/backtrace.c', 'misc/conformance.c',
    'misc/error.c', 'misc/errorf.c', 'misc/errormsg.c',
    'misc/errorx.c', 'misc/fgetcwd.c', 'misc/glob.c',
    'misc/optctx.c',
    'misc/optget.c', 'misc/procclose.c', 'misc/procfree.c',
    'misc/procopen.c',
    'misc/sigcrit.c',
    'misc/stack.c', 'misc/state.c',
    'misc/stk.c', 'misc/translate.c', 'misc/univdata.c',
    'misc/fts.c', 'misc/vmbusy.c'
]

if not shared_c_args.contains('-D_AST_no_spawnveg=1')
    libast_files += [ 'misc/spawnvex.c' ]
endif
