#ifndef _TMLIB_H
#define _TMLIB_H 1

/* TODO: Check if these definitions can be deprecated */
#define tmlocaltime(p) _tm_localtime(p)
extern struct tm *_tm_localtime(const time_t *);

#endif  // _TMLIB_H
