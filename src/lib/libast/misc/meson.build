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
    'misc/procclose.c',
    'misc/procfree.c',
    'misc/procopen.c',
    'misc/sigcrit.c',
    'misc/state.c',
    'misc/stk.c',
    'misc/translate.c',
    'misc/univdata.c',
    'misc/vmbusy.c',
]

if shared_c_args.contains('-DUSE_SPAWN=1')
    libast_files += [ 'misc/spawnvex.c' ]
endif
