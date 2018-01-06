/* : : generated from mmap by iffe version 2013-11-14 : : */
#ifndef _def_mmap_features
#define _def_mmap_features 1

#define _mmap_anon 1

/* some systems get it wrong but escape concise detection */
#if __CYGWIN__
#define _NO_MMAP 1
#endif

#if _NO_MMAP
#undef _mmap_anon
#endif

#endif
