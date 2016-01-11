#pragma prototyped
#ifndef __TKSHLIB_H_
#define __TKSHLIB_H_

#include <tksh.h>
#include <config.h>
#include <stdio.h>
#include <ctype.h>
#undef NTcl_FreeResult
#define NTcl_FreeResult(interp) do { sfprintf(sfstderr, "Free of %s at %s %d (%x)\n", (interp)->result, __FILE__, __LINE__, (interp)->freeProc); (interp)->freeProc = 0; } while(0)

#include "shcompat.h"

#define DEFAULT_PD_PREC 6
#define DEFAULT_PD_FORMAT "%g"

/*
 * The macro below is used to modify a "char" value (e.g. by casting
 * it to an unsigned character) so that it can be used safely with
 * macros such as isspace.
 */

#define UCHAR(c) ((unsigned char) (c))

/*
 * Given a size or address, the macro below "aligns" it to the machine's
 * memory unit size (e.g. an 8-byte boundary) so that anything can be
 * placed at the aligned address without fear of an alignment error.
 */

#define TCL_ALIGN(x) ((x + 7) & ~7)

/*
 * Patchlevel
 */

#if TCL_MINOR_VERSION == 3
#define TCL_PATCH_LEVEL 106
#else
#ifndef TCL_PATCH_LEVEL
#define TCL_PATCH_LEVEL "7.4"	/* The only ver that wouldn't already set it */
#endif
#endif

#ifndef NO_TCL_INTERP		/* Parsing stuff */

typedef struct ParseValue {
    char *buffer;		/* Address of first character in
				 * output buffer. */
    char *next;			/* Place to store next character in
				 * output buffer. */
    char *end;			/* Address of the last usable character
				 * in the buffer. */
    void (*expandProc)(struct ParseValue *pvPtr, int needed);
				/* Procedure to call when space runs out;
				 * it will make more space. */
    ClientData clientData;	/* Arbitrary information for use of
				 * expandProc. */
} ParseValue;

/*
 * A table used to classify input characters to assist in parsing
 * Tcl commands.  The table should be indexed with a signed character
 * using the CHAR_TYPE macro.  The character may have a negative
 * value.
 */

extern char tclTypeTable[];
#define CHAR_TYPE(c) (tclTypeTable+128)[c]

#if _BLD_tcl && defined(__EXPORT__)
#define extern	__EXPORT__
#endif

/*
 * Possible values returned by CHAR_TYPE:
 *
 * TCL_NORMAL -		All characters that don't have special significance
 *			to the Tcl language.
 * TCL_SPACE -		Character is space, tab, or return.
 * TCL_COMMAND_END -	Character is newline or null or semicolon or
 *			close-bracket.
 * TCL_QUOTE -		Character is a double-quote.
 * TCL_OPEN_BRACKET -	Character is a "[".
 * TCL_OPEN_BRACE -	Character is a "{".
 * TCL_CLOSE_BRACE -	Character is a "}".
 * TCL_BACKSLASH -	Character is a "\".
 * TCL_DOLLAR -		Character is a "$".
 */

#define TCL_NORMAL		0
#define TCL_SPACE		1
#define TCL_COMMAND_END		2
#define TCL_QUOTE		3
#define TCL_OPEN_BRACKET	4
#define TCL_OPEN_BRACE		5
#define TCL_CLOSE_BRACE		6
#define TCL_BACKSLASH		7
#define TCL_DOLLAR		8

#ifndef TCL_BRACKET_TERM
#define TCL_BRACKET_TERM	1	/* in tcl7.3 header, not 7.4 */
#endif
#define TCL_ALLOW_EXCEPTIONS	4	/* in tcl7.4 only */


extern void		TclExpandParseValue(ParseValue *pvPtr, int needed);

extern int		TclNeedSpace(char *start, char *end);

extern void		TclSetupEnv(Tcl_Interp *interp);

extern void		TclWinFlushEvents(void);

extern int		Tcl_NumEventsFound(void);

#endif


#ifndef NO_TCL_INTERP		/* Tcl backward compat. stuff */

/*
 *----------------------------------------------------------------
 * Data structures related to procedures.   These are used primarily
 * in tclProc.c
 *----------------------------------------------------------------
 */

/*
 * The structure below defines an argument to a procedure, which
 * consists of a name and an (optional) default value.
 */

typedef struct Arg {
    struct Arg *nextPtr;	/* Next argument for this procedure,
				 * or NULL if this is the last argument. */
    char *defValue;		/* Pointer to arg's default value, or NULL
				 * if no default value. */
    char name[4];		/* Name of argument starts here.  The name
				 * is followed by space for the default,
				 * if there is one.  The actual size of this
				 * field will be as large as necessary to
				 * hold both name and default value.  THIS
				 * MUST BE THE LAST FIELD IN THE STRUCTURE!! */
} Arg;

/*
 * The structure below defines a command procedure, which consists of
 * a collection of Tcl commands plus information about arguments and
 * variables.
 */

typedef struct Proc {
    struct Interp *iPtr;	/* Interpreter for which this command
				 * is defined. */
    int refCount;		/* Reference count:  1 if still present
				 * in command table plus 1 for each call
				 * to the procedure that is currently
				 * active.  This structure can be freed
				 * when refCount becomes zero. */
    char *command;		/* Command that constitutes the body of
				 * the procedure (dynamically allocated). */
    Arg *argPtr;		/* Pointer to first of procedure's formal
				 * arguments, or NULL if none. */
} Proc;

extern Proc *		TclIsProc(Tcl_CmdInfo *cmdPtr);
extern Proc *		TclFindProc(Interp *iPtr, char *procName);
extern int		TclUpdateReturnInfo(Interp *iPtr);

extern char *		TkshMapName(char *name);
extern char *		TkshMapKeyword(char *name);
extern char *		TkshLibDir(void);

extern int		TkshSetListMode(int mode);

#endif

extern int		Tcl_TclEval(Tcl_Interp *interp, char *cmd);
extern int		Tcl_TclEvalFile(Tcl_Interp *interp, char *fileName);

#undef	extern

#include "debug.h"

typedef struct TclEventSource {
    Tcl_EventSetupProc *setupProc;      /* This procedure is called by
                                         * Tcl_DoOneEvent to set up information
                                         * for the wait operation, such as
                                         * files to wait for or maximum
                                         * timeout. */
    Tcl_EventCheckProc *checkProc;      /* This procedure is called by
                                         * Tcl_DoOneEvent after its wait
                                         * operation to see what events
                                         * are ready and queue them. */
    ClientData clientData;              /* Arbitrary one-word argument to pass
                                         * to setupProc and checkProc. */
    struct TclEventSource *nextPtr;     /* Next in list of all event sources
                                         * defined for applicaton. */
} TclEventSource;

#define TclPlatformExit(status) exit(status)

#if _BLD_tcl && defined(__EXPORT__)
#define extern	__EXPORT__
#endif

EXTERN Tcl_Channel      TclGetDefaultStdChannel(int type);
EXTERN Tcl_Channel	TclFindFileChannel(Tcl_File inFil, Tcl_File outFile,
			    int *fileUsedPtr);
EXTERN int              TclGetLoadedPackages(Tcl_Interp *interp,
			    char *targetName);
EXTERN int              TclInterpInit(Tcl_Interp *interp);
EXTERN void             TclPlatformInit(Tcl_Interp *interp);
EXTERN void             TclFreePackageInfo(Interp *iPtr);
EXTERN int              TclChdir(Tcl_Interp *interp, char *dirName);
EXTERN char *           TclGetCwd(Tcl_Interp *interp);
EXTERN int              TclPreventAliasLoop(Tcl_Interp *interp,
                            Tcl_Interp *cmdInterp, char *cmdName,
                            Tcl_CmdProc *proc, ClientData clientData);
EXTERN int              TclFindElement(Tcl_Interp *interp,
                            char *list, char **elementPtr, char **nextPtr,
                            int *sizePtr, int *bracePtr);
EXTERN void             TclCopyAndCollapse(int count, char *src, char *dst);
EXTERN int              TclGetListIndex(Tcl_Interp *interp,
                            char *string, int *indexPtr);
EXTERN unsigned long    TclGetSeconds(void);
EXTERN void             TclGetTime(Tcl_Time *time);
EXTERN int              TclGetTimeZ(unsigned long time);
EXTERN int              TclGetDate(char *p, unsigned long now, long zone,
                            unsigned long *timePtr);
EXTERN int              TclGuessPackageName(char *fileName,
                            Tcl_DString *bufPtr);
EXTERN int              TclLoadFile(Tcl_Interp *interp,
                            char *fileName, char *sym1, char *sym2,
                            Tcl_PackageInitProc **proc1Ptr,
                            Tcl_PackageInitProc **proc2Ptr);
EXTERN char *		TclGetExtension(char *name);
EXTERN int              TclGetOpenMode(Tcl_Interp *interp,
                            char *string, int *seekFlagPtr);
EXTERN unsigned long    TclGetClicks(void);
EXTERN int              TclIdlePending(void);
EXTERN int              TclServiceIdle(void);
EXTERN int              TclWaitForFile(Tcl_File file, int mask, int timeout);
EXTERN int              TclParseBraces(Tcl_Interp *interp,
                            char *string, char **termPtr, ParseValue *pvPtr);
EXTERN int              TclParseNestedCmd(Tcl_Interp *interp,
                            char *string, int flags, char **termPtr,
                            ParseValue *pvPtr);
EXTERN int              TclParseQuotes(Tcl_Interp *interp,
                            char *string, int termChar, int flags,
                            char **termPtr, ParseValue *pvPtr);
EXTERN int              TclParseWords(Tcl_Interp *interp,
                            char *string, int flags, int maxWords,
                            char **termPtr, int *argcPtr, char **argv,
                            ParseValue *pvPtr);



extern char *		TclPrecTraceProc(ClientData clientData,
				Tcl_Interp *interp, char *name1, char *name2,
				int flags);

#undef	extern

/*
 *----------------------------------------------------------------
 * Variables shared among Tcl modules but not used by the outside
 * world:
 *----------------------------------------------------------------
 */

extern int		tclInInterpreterDeletion;
extern char *           tclExecutableName;


void TkshCreateInterp(Tcl_Interp *interp, void *data);

typedef struct Trace {
    int level;                  /* Only trace commands at nesting level
                                 * less than or equal to this. */
    Tcl_CmdTraceProc *proc;     /* Procedure to call to trace command. */
    ClientData clientData;      /* Arbitrary value to pass to proc. */
    struct Trace *nextPtr;      /* Next in list of traces for this interp. */
} Trace;



typedef struct AssocData {
    Tcl_InterpDeleteProc *proc; /* Proc to call when deleting. */
    ClientData clientData;      /* Value to pass to proc. */
} AssocData;

#endif /* __TKSHLIB_H_ */
