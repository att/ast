/* : : generated from standards by iffe version 2013-11-14 : : */
#ifndef _def_standards_features
#define _def_standards_features 1

/* TODO: Check if it Is safe to use these macros on all platforms ? */
/* _ISOC99_SOURCE plays nice */
#ifndef _ISOC99_SOURCE
#define _ISOC99_SOURCE 1
#endif

/*
 * this is a nasty game we all play to honor standards symbol visibility
 * it would help if all implementations had
 *	_KITCHEN_SINK_SOURCE
 * that enabled all symbols from the latest implemented standards
 * that's probably the most useful but least portable request
 */

#if __MACH__ || __FREEBSD__
#undef _POSIX_SOURCE
#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE
#endif

#endif
