#ifndef TMLIB_H
#define TMLIB_H

/* TODO: Check if these definitions can be deprecated */
#define tmlocaltime(p)      _tm_localtime(p)
extern struct tm*       _tm_localtime(const time_t*);

#endif
