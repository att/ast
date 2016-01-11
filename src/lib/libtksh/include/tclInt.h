
/* NOTE:
 *
 * All the files in this directory are directly from the Tcl distribution
 * with one exception:  the Tcl_InfoCmd has been deleted from tclCmdIL.c
 * and the replacement is in the tksh directory.
 */

#define xxxcommandTable commandTable
#define xxxpatLengths patLengths
#define xxxregexps regexps
#define xxxpatterns patterns
#include "tkshlib.h"
typedef struct Command {
    Tcl_HashEntry *hPtr;        /* Pointer to the hash table entry in
                                 * interp->commandTable that refers to
                                 * this command.  Used to get a command's
                                 * name from its Tcl_Command handle. */
    Tcl_CmdProc *proc;          /* Procedure to process command. */
    ClientData clientData;      /* Arbitrary value to pass to proc. */
    Tcl_CmdDeleteProc *deleteProc;
                                /* Procedure to invoke when deleting
                                 * command. */
    ClientData deleteData;      /* Arbitrary value to pass to deleteProc
                                 * (usually the same as clientData). */
} Command;


#define MAX_MATH_ARGS 5
typedef struct MathFunc {
    int numArgs;                /* Number of arguments for function. */
    Tcl_ValueType argTypes[MAX_MATH_ARGS];
                                /* Acceptable types for each argument. */
    Tcl_MathProc *proc;         /* Procedure that implements this function. */
    ClientData clientData;      /* Additional argument to pass to the function
                                 * when invoking it. */
} MathFunc;

extern void             TclExprFloatError _ANSI_ARGS_((Tcl_Interp *interp,
                            double value));

#define EXPR_INITIALIZED        0x10

#ifdef NO_FLOAT_H
#undef NO_FLOAT_H
#endif

#ifdef NO_STDLIB_H
#undef NO_STDLIB_H
#endif

#if 0
extern double strtod();
/* Bug in sprintf */
#ifdef sprintf
#undef sprintf
#endif
#endif

/* Don't want to use sh_write */
#ifdef write
#undef write
#endif

#define NSUBEXP  50
typedef struct regexp {
	char *startp[NSUBEXP];
	char *endp[NSUBEXP];
	char regstart;		/* Internal use only. */
	char reganch;		/* Internal use only. */
	char *regmust;		/* Internal use only. */
	int regmlen;		/* Internal use only. */
	char program[1];	/* Unwarranted chumminess with compiler. */
} regexp;

EXTERN regexp *TclRegComp _ANSI_ARGS_((char *exp));
EXTERN int TclRegExec _ANSI_ARGS_((regexp *prog, char *string, char *start));
EXTERN void TclRegSub _ANSI_ARGS_((regexp *prog, char *source, char *dest));
EXTERN void TclRegError _ANSI_ARGS_((char *msg));
EXTERN char *TclGetRegError _ANSI_ARGS_((void));
