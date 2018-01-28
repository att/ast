if cc.has_header('sys/syscall.h', args: feature_test_args)
    feature_data.set('_sys_syscall', 1)
endif

if cc.has_header('sys/systeminfo.h', args: feature_test_args)
    feature_data.set('_sys_systeminfo', 1)
endif

if cc.has_header('sys/syssgi.h', args: feature_test_args)
    feature_data.set('_sys_syssgi', 1)
endif

if cc.has_function('getdomainname', prefix: '#include <unistd.h>', args: feature_test_args)
    feature_data.set('_lib_getdomainname', 1)
endif

if cc.has_function('gethostid', prefix: '#include <unistd.h>', args: feature_test_args)
    feature_data.set('_lib_gethostid', 1)
endif

if cc.has_function('gethostname', prefix: '#include <unistd.h>', args: feature_test_args)
    feature_data.set('_lib_gethostname', 1)
    feature_data.set('_lib_sethostname', 1)
endif

if cc.has_function('syscall', prefix: '#include <unistd.h>', args: feature_test_args)
    feature_data.set('_lib_syscall', 1)
endif

if cc.has_function('systeminfo', prefix: '#include <unistd.h>', args: feature_test_args)
    feature_data.set('_lib_systeminfo', 1)
endif

if cc.has_function('uname', prefix: '#include <sys/utsname.h>', args: feature_test_args)
    feature_data.set('_lib_uname', 1)
endif

