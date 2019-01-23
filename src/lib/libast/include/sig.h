#ifndef _AST_SIG_H
#define _AST_SIG_H 1

#define SIG_REG_POP 0
#define SIG_REG_EXEC (1 << 0)
#define SIG_REG_PROC (1 << 1)
#define SIG_REG_TERM (1 << 2)

extern int sigcritical(int);

#endif  // _AST_SIG_H
