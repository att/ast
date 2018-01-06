/* : : generated from sig.sh by iffe version 2013-11-14 : : */
#ifndef _def_sig_features
#define _def_sig_features 1

#define sig_info _sig_info_

#include <signal.h>

typedef void (*Sig_handler_t)(int);

#define Handler_t Sig_handler_t

#define SIG_REG_PENDING (-1)
#define SIG_REG_POP 0
#define SIG_REG_EXEC 00001
#define SIG_REG_PROC 00002
#define SIG_REG_TERM 00004
#define SIG_REG_ALL 00777
#define SIG_REG_SET 01000

typedef struct {
    char **name;
    char **text;
    int sigmax;
} Sig_info_t;

extern Sig_info_t sig_info;

extern int sigflag(int, int, int);

extern int sigcritical(int);
extern int sigunblock(int);

#endif
