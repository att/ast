libast_files += [
    'misc/astintercept.c', 'misc/cmdarg.c', 'misc/conformance.c',
    'misc/debug.c', 'misc/error.c', 'misc/errorf.c', 'misc/errormsg.c',
    'misc/errorx.c', 'misc/fastfind.c', 'misc/fgetcwd.c', 'misc/fmtrec.c',
    'misc/fs3d.c',
    'misc/getenv.c', 'misc/glob.c',
    'misc/mime.c', 'misc/optctx.c', 'misc/optesc.c',
    'misc/optget.c', 'misc/optjoin.c', 'misc/procclose.c', 'misc/procfree.c',
    'misc/procopen.c', 'misc/procrun.c', 'misc/recfmt.c', 'misc/reclen.c',
    'misc/recstr.c', 'misc/setenviron.c', 'misc/sigcrit.c', 'misc/sigdata.c',
    'misc/signal.c', 'misc/spawnvex.c', 'misc/stack.c', 'misc/state.c',
    'misc/stk.c', 'misc/systrace.c', 'misc/translate.c', 'misc/univdata.c'
]

posix_spawn_feature_file = files('posix_spawn_feature.c')
posix_spawn_feature_result = cc.run(
    posix_spawn_feature_file, name: 'posix_spawn exists and it works and its worth using')
libast_c_args += ['-D_lib_posix_spawn=@0@'.format(posix_spawn_feature_result.returncode())]
