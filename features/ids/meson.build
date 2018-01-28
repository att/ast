if cc.has_function('endgrent')
    feature_data.set('_lib_endgrent', 1)
    feature_data.set('_lib_getgrent', 1)
    feature_data.set('_lib_setgrent', 1)
endif

# TODO: Figure out if these tests still make sense. These functions don't seem
# to exist on any system I'm using.
if system == 'darwin'
    prefix = '\n'.join(['#include sys/types.h', '#include sys/fss.h', '#include fsg.h'])
    if cc.has_function('fsid', prefix: prefix)
        feature_data.set('_lib_fsid', 1)
    endif
else
    prefix = '\n'.join(['#include sys/types.h', '#include sys/fss.h', '#include fsg.h'])
    if cc.has_function('fsid', args: '-lfsg', prefix: prefix)
        feature_data.set('_lib_fsid', 1)
    endif
    if cc.has_function('getfsgid', args: '-lfsg', prefix: prefix)
        feature_data.set('_lib_getfsgid', 1)
    endif
    if cc.has_function('isfsg', args: '-lfsg', prefix: prefix)
        feature_data.set('_lib_isfsg', 1)
    endif
endif
