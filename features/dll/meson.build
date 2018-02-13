# TODO: Add running the dll.c module.

feature_data.set10('_hdr_dl', cc.has_header('dl.h', args: feature_test_args))
feature_data.set10('_hdr_dlfcn', cc.has_header('dlfcn.h', args: feature_test_args))
feature_data.set10('_hdr_dll', cc.has_header('dll.h', args: feature_test_args))
feature_data.set10('_hdr_rld_interface', cc.has_header('rld_interface.h', args: feature_test_args))
feature_data.set10('_hdr_mach_o_dyld', cc.has_header('mach-o/dyld.h', args: feature_test_args))
feature_data.set10('_sys_ldr', cc.has_header('sys/ldr.h', args: feature_test_args))

feature_data.set10('_lib_dlopen',
    cc.has_function('dlopen', prefix: '#include <dlfcn.h>', args: ['-ldl'] + feature_test_args))
feature_data.set10('_lib_dllload', cc.has_function('dllload', args: ['-ldl'] + feature_test_args))
feature_data.set10('_lib_loadbind', cc.has_function('loadbind', args: ['-ldl'] + feature_test_args))

dynamic_feature_file = files('dynamic.c')
dynamic_feature_result = cc.run(dynamic_feature_file, name: '_DYNAMIC check',
                                args: feature_test_args)
feature_data.set10('_dll_DYNAMIC', dynamic_feature_result.compiled())

dll_next_path = run_command('dll_next_path.sh')
if dll_next_path.returncode() == 0
    feature_data.set_quoted('_DLL_NEXT_PATH', dll_next_path.stdout().strip())
endif
