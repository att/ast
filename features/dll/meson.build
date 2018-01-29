# TODO: Add running the dll.c module.

if cc.has_header('dl.h', args: feature_test_args)
    feature_data.set('_hdr_dl', 1)
endif

if cc.has_header('dlfcn.h', args: feature_test_args)
    feature_data.set('_hdr_dlfcn', 1)
endif

if cc.has_header('dll.h', args: feature_test_args)
    feature_data.set('_hdr_dll', 1)
endif

if cc.has_header('rld_interface.h', args: feature_test_args)
    feature_data.set('_hdr_rld_interface', 1)
endif

if cc.has_header('mach-o/dyld.h', args: feature_test_args)
    feature_data.set('_hdr_mach_o_dyld', 1)
endif

if cc.has_header('sys/ldr.h', args: feature_test_args)
    feature_data.set('_sys_ldr', 1)
endif

if cc.has_function('dlopen', prefix: '#include <dlfcn.h>', args: ['-ldl'] + feature_test_args)
    feature_data.set('_lib_dlopen', 1)
endif

if cc.has_function('dllload', args: ['-ldl'] + feature_test_args)
    feature_data.set('_lib_dllload', 1)
endif

if cc.has_function('loadbind', args: ['-ldl'] + feature_test_args)
    feature_data.set('_lib_loadbind', 1)
endif

dynamic_feature_file = files('dynamic.c')
dynamic_feature_result = cc.run(dynamic_feature_file, name: '_DYNAMIC check',
                                args: feature_test_args)
if dynamic_feature_result.compiled()
    feature_data.set('_dll_DYNAMIC', 1)
endif

dll_next_path = run_command('dll_next_path.sh')
if dll_next_path.returncode() == 0
    feature_data.set_quoted('_DLL_NEXT_PATH', dll_next_path.stdout().strip())
endif
