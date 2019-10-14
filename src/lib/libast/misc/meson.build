libast_files += [
    'misc/astwinsize.c',
    'misc/debug.c',
    'misc/error.c',
    'misc/errorf.c',
    'misc/errormsg.c',
    'misc/errorx.c',
    'misc/fallbacks.c',
    'misc/fgetcwd.c',
    'misc/glob.c',
    'misc/optget.c',
    'misc/optget_long.c',
    'misc/procclose.c',
    'misc/procfree.c',
    'misc/procopen.c',
    'misc/setlocale.c',
    'misc/sigcrit.c',
    'misc/state.c',
    'misc/stk.c',
    'misc/translate.c',
    'misc/vmbusy.c',
]

if shared_c_args.contains('-DUSE_SPAWN=1')
    libast_files += [ 'misc/spawnvex.c' ]
endif
