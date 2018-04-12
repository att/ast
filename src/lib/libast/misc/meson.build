libast_files += [
    'misc/astintercept.c', 'misc/cmdarg.c', 'misc/conformance.c',
    'misc/debug.c', 'misc/error.c', 'misc/errorf.c', 'misc/errormsg.c',
    'misc/errorx.c', 'misc/fastfind.c', 'misc/fgetcwd.c', 'misc/fmtrec.c',
    'misc/getenv.c', 'misc/glob.c',
    'misc/mime.c', 'misc/optctx.c', 'misc/optesc.c',
    'misc/optget.c', 'misc/optjoin.c', 'misc/procclose.c', 'misc/procfree.c',
    'misc/procopen.c', 'misc/procrun.c', 'misc/recfmt.c', 'misc/reclen.c',
    'misc/recstr.c', 'misc/setenviron.c', 'misc/sigcrit.c', 'misc/sigdata.c',
    'misc/signal.c', 'misc/stack.c', 'misc/state.c',
    'misc/stk.c', 'misc/systrace.c', 'misc/translate.c', 'misc/univdata.c'
]

if not shared_c_args.contains('-D_AST_no_spawnveg=1')
    libast_files += [ 'misc/spawnvex.c' ]
endif
