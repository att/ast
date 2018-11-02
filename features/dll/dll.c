// TODO: Modify this to work with the Meson build system and add it to the meson.build config file.

#if defined(__MVS__) && !defined(__SUSV3)
#define __SUSV3 1
#endif
#if _hdr_dlfcn && _lib_dlopen
#include <dlfcn.h>
#endif
#if _hdr_rld_interface
#include <rld_interface.h>
#endif
int main() {
    int i;
#if _hdr_rld_interface
    void *dll;
    static char *local[] = {"__elf_header", "_call_add_gp_range", "_etext"};
#endif
    printf("\n");
    printf("#if defined(__MVS__) && !defined(__SUSV3)\n");
    printf("#define __SUSV3         1\n");
    printf("#endif\n");
#if _hdr_dlfcn && _lib_dlopen
    printf("#include <dlfcn.h>\n");
#endif
#ifndef RTLD_LAZY
    i = 0;
    printf("\n");
    printf("#define RTLD_LAZY       1\n");
#else
    i = 1;
#endif
#ifndef RTLD_NOW
    if (i) {
        i = 0;
        printf("\n");
    }
    printf("#define RTLD_NOW        2\n");
#endif
#ifndef RTLD_GLOBAL
    if (i) {
        i = 0;
        printf("\n");
    }
    printf("#define RTLD_GLOBAL     0\n");
#endif
#ifndef RTLD_LOCAL
    if (i) {
        i = 0;
        printf("\n");
    }
    printf("#define RTLD_LOCAL      0\n");
#endif
#ifndef RTLD_PARENT
    if (i) {
        i = 0;
        printf("\n");
    }
    printf("#define RTLD_PARENT     0\n");
#endif
#if defined(_hdr_mach_o_dyld) && !defined(RTLD_NEXT)
    if (i) {
        i = 0;
        printf("\n");
    }
    printf("#define RTLD_NEXT       ((void*)16)\n");
#endif
#if _hdr_rld_interface
    if (!(dll = dlopen(0, RTLD_LAZY)))
        i = -1;
    else {
        for (i = 0; i < sizeof(local) / sizeof(local[0]); i++)
            if (dlsym(dll, local[i])) break;
        if (i >= sizeof(local) / sizeof(local[0])) i = -1;
    }
    if (i >= 0) {
        printf("\n");
        printf("#define _DLL_RLD_SYM            %s\n", local[i]);
        printf("#define _DLL_RLD_SYM_STR        \"%s\"\n", local[i]);
        printf("#define _DLL_RLD_SYM_TYPE       void*\n");
    }
#endif
    printf("\n");
    printf("#define DLL_INFO_PREVER 0x0001  /* pre-suffix style version */\n");
    printf("#define DLL_INFO_DOTVER 0x0002  /* post-suffix style version */\n");
    printf("\n");
    printf("typedef unsigned long (*Dll_plugin_version_f)(void);\n");
    printf("typedef int (*Dllerror_f)(void*, void*, int, ...);\n");
    printf("\n");
    printf("typedef struct Dllinfo_s\n");
    printf("{\n");
    printf("        char**  sibling;        /* sibling dirs on $PATH */\n");
    printf("        char*   prefix;         /* library name prefix */\n");
    printf("        char*   suffix;         /* library name suffix */\n");
    printf("        char*   env;            /* library path env var */\n");
    printf("        int     flags;          /* DLL_INFO_* flags */\n");
    printf("#ifdef _DLLINFO_PRIVATE_\n");
    printf("        _DLLINFO_PRIVATE_\n");
    printf("#endif\n");
    printf("} Dllinfo_t;\n");
    printf("\n");
    printf("typedef struct Dllnames_s\n");
    printf("{\n");
    printf("        char*           id;\n");
    printf("        char*           name;\n");
    printf("        char*           base;\n");
    printf("        char*           type;\n");
    printf("        char*           opts;\n");
    printf("        char*           path;\n");
    printf("        char            data[1024];\n");
    printf("} Dllnames_t;\n");
    printf("\n");
    printf("typedef struct Dllent_s\n");
    printf("{\n");
    printf("        char*           path;\n");
    printf("        char*           name;\n");
    printf("#ifdef _DLLENT_PRIVATE_\n");
    printf("        _DLLENT_PRIVATE_\n");
    printf("#endif\n");
    printf("} Dllent_t;\n");
    printf("\n");
    printf("typedef struct Dllscan_s\n");
    printf("{\n");
    printf("        void*           pad;\n");
    printf("#ifdef _DLLSCAN_PRIVATE_\n");
    printf("        _DLLSCAN_PRIVATE_\n");
    printf("#endif\n");
    printf("} Dllscan_t;\n");
#if !_hdr_dlfcn || !_lib_dlopen
    printf("\n");
    printf("extern void*            dlopen(const char*, int);\n");
    printf("extern void*            dlsym(void*, const char*);\n");
    printf("extern int              dlclose(void*);\n");
    printf("extern char*            dlerror(void);\n");
#endif
    printf("\n");
    printf("extern Dllinfo_t*       dllinfo(void);\n");
    printf(
        "extern void*            dllplugin(const char*, const char*, const char*, unsigned long, "
        "unsigned long*, int, char*, size_t);\n");
    printf(
        "extern void*            dllplug(const char*, const char*, const char*, int, char*, "
        "size_t);\n");
    printf("extern void*            dllfind(const char*, const char*, int, char*, size_t);\n");
    printf("extern void*            dllopen(const char*, int);\n");
    printf("extern void*            dllnext(int);\n");
    printf("extern void*            dlllook(void*, const char*);\n");
    printf(
        "extern int              dllcheck(void*, const char*, unsigned long, unsigned long*);\n");
    printf("extern unsigned long    dllversion(void*, const char*);\n");
    printf("extern char*            dllerror(int);\n");
#if _hdr_rld_interface
    if (i >= 0) {
        printf("\n");
        printf("extern void*            _dll_next(int, _DLL_RLD_SYM_TYPE*);\n");
        printf("#define dllnext(f)      _dll_next(f, &_DLL_RLD_SYM)\n");
    }
#endif
    printf("\n");
    printf("extern Dllscan_t*       dllsopen(const char*, const char*, const char*);\n");
    printf("extern Dllent_t*        dllsread(Dllscan_t*);\n");
    printf("extern int              dllsclose(Dllscan_t*);\n");
    printf("\n");
#if _hdr_rld_interface
    if (i >= 0) {
        printf("\n");
        printf("extern _DLL_RLD_SYM_TYPE _DLL_RLD_SYM;\n");
    }
#endif
    printf("\n");
    return 0;
}
