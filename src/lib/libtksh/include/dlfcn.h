#pragma prototyped
/*
 * -lshell provides dl(3) if the local system is non-de-facto
 */

#ifndef RTLD_NOW
#   define RTLD_NOW 1
#endif

#ifndef RTLD_GLOBAL
#   define RTLD_GLOBAL 0
#endif

extern void*	dlopen(const char*,int);
extern void*	dlsym(void*, const char*);
extern char*	dlerror(void);
