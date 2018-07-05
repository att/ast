/* : : generated from sig.sh by iffe version 2013-11-14 : : */
#ifndef _AST_SIG_H
#define _AST_SIG_H 1

#ifdef _FAULT_H
#error You cannot include sig.h after fault.h
#endif

#define sig_info _sig_info_

#include <signal.h>

typedef void (*Sig_handler_t)(int);

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

Sig_handler_t ast_signal(int sig, Sig_handler_t sigfun);
extern int sigflag(int, int, int);
extern int sigcritical(int);
extern int sigunblock(int);

#endif  // _AST_SIG_H
