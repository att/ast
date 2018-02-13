# This symbol also implies the availability of `clock_settime()` since the
# presence of one should mean the other API exists.
feature_data.set10('_lib_clock_gettime',
    cc.has_function('clock_gettime', prefix: '#include <time.h>', args: feature_test_args))

# This symbol also implies the availability of `settimeofday()` since the
# presence of one should mean the other API exists.
feature_data.set10('_lib_gettimeofday',
    cc.has_function('gettimeofday', prefix: '#include <sys/time.h>', args: feature_test_args))
