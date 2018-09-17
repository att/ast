feature_data.set10('_sys_syscall', cc.has_header('sys/syscall.h', args: feature_test_args))
feature_data.set10('_sys_systeminfo', cc.has_header('sys/systeminfo.h', args: feature_test_args))
feature_data.set10('_sys_syssgi', cc.has_header('sys/syssgi.h', args: feature_test_args))

feature_data.set10('_lib_syscall',
    cc.has_function('syscall', prefix: '#include <unistd.h>', args: feature_test_args))
feature_data.set10('_lib_systeminfo',
    cc.has_function('systeminfo', prefix: '#include <unistd.h>', args: feature_test_args))
