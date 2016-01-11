#ifdef DEBUG
#if 0
        extern Interp *__Tksh_Interp;
        extern int __Tksh_DebugOn;
#       define dprintf(x)       while (__Tksh_DebugOn) { fprintf(stderr, \
                                "[%d]%s:%d: ", __Tksh_Interp ?  \
                                __Tksh_Interp->interpType : -1, \
                                __FILE__, __LINE__); fprintf x; break; }
#       define dprintf2(x)      if (__Tksh_DebugOn == 2) dprintf(x)
#else
	extern int __dprintfOK(char *, int, int);
	extern void __dprintf();
	extern int __dprintfLevel(void);
	extern void __dprintfArgs(char *string, int argc, char *argv[]);
	extern void __dprintfNum(char *, int, char *);
	extern void __dprintfInterp();
#       define dprintf(x)	do { if (__dprintfOK(__FILE__, __LINE__, 1)) \
                                        __dprintf x ; } while(0)
#       define dprintf2(x)	do { if (__dprintfOK(__FILE__, __LINE__, 2)) \
                                        __dprintf x ; } while(0)
#	define dprintfArgs(s,c,v)  do { if (__dprintfOK(__FILE__,__LINE__,1))\
					__dprintfArgs(s,c,v); } while(0)
#	define dprintfNum(b,n,a)  do { if (__dprintfOK(__FILE__,__LINE__,1))\
					__dprintfNum(b,n,a); } while(0)
#	define dprintfInterp(i)	__dprintfInterp(i)
#	define debugon()	(__dprintfLevel())
#endif
#else
#       define dprintf(x)
#       define dprintf2(x)
#	define dprintfArgs(s,c,v)
#	define dprintfNum(b,n,a)
#	define dprintfInterp(i)
#endif
