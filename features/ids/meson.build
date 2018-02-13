feature_data.set10('_lib_getgrent', cc.has_function('getgrent'))

# TODO: Figure out if these tests still make sense. These functions don't seem
# to exist on any system I'm using.
if system == 'darwin'
    prefix = '\n'.join(['#include sys/types.h', '#include sys/fss.h', '#include fsg.h'])
    feature_data.set10('_lib_fsid', cc.has_function('fsid', prefix: prefix))
else
    prefix = '\n'.join(['#include sys/types.h', '#include sys/fss.h', '#include fsg.h'])
    feature_data.set10('_lib_fsid', cc.has_function('fsid', args: '-lfsg', prefix: prefix))
    feature_data.set10('_lib_getfsgid', cc.has_function('getfsgid', args: '-lfsg', prefix: prefix))
    feature_data.set10('_lib_isfsg', cc.has_function('isfsg', args: '-lfsg', prefix: prefix))
endif
