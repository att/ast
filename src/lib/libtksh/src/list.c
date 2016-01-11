#pragma prototyped
#include "tkshlib.h"


/**********************************************************************
 *
 *	Includes the following:
 *
 *	Tcl_SplitList
 *	Tcl_Merge
 *	Tcl_ScanElement
 *	Tcl_ConvertElement
 *
 *	Lists are handled as ksh-style lists.  Thus, the routines are
 *	completely rewritten.  The only one that can be copied is
 *	Tcl_Merge, since it uses Tcl_ScanElement and Tcl_ConvertElement
 *	only.
 *
 **********************************************************************/

EXTERN int              Tcl_TclConvertElement _ANSI_ARGS_((char *src,
                            char *dst, int flags));
EXTERN char *           Tcl_TclMerge _ANSI_ARGS_((int argc, char **argv));
EXTERN int              Tcl_TclScanElement _ANSI_ARGS_((char *string,
                            int *flagPtr));
EXTERN int              Tcl_TclSplitList _ANSI_ARGS_((Tcl_Interp *interp,
                            char *list, int *argcPtr, char ***argvPtr));

static int listMode = INTERP_TCL;
#define TkshListMode() (listMode)

typedef struct TkshArgcArgv
{
	int *argcPtr;
	char ***argvPtr;
} TkshArgcArgv;


static int tksh_cmd_split(int argc, char *argv[], Shbltin_t *context)
{
	int i, size=0;
	char **newargv;
	char *newptr;
	TkshArgcArgv *args = (TkshArgcArgv *)context->ptr;

	for (i=1; i < argc; i++)
		size += strlen(argv[i]);

	newargv = (char **) malloc(size + argc * (sizeof(char *) + 1) );
	newptr = (char *) (newargv + argc);

	for (i=1; i < argc; i++)
	{
		newargv[i-1] = newptr;
		newptr = strcopy(newptr, argv[i]) + 1;
	}

	newargv[argc-1]= (char*) 0;
	
	*(args->argcPtr) = argc-1;
	*(args->argvPtr) = newargv;
	
	return 0;
}

int TkshSetListMode(int mode)
{
	int oldmode = listMode;
	if (mode >= 0)
		listMode = mode;
	return oldmode;
}

char *Tksh_ConvertList(Tcl_Interp *interp, char *list, int toMode)
{
	int fromMode = (toMode == INTERP_KSH) ? INTERP_TCL : INTERP_KSH;
	int oldMode, argc;
	char *result, **argv;

	result = NULL;
	oldMode = TkshSetListMode(fromMode);
	if (Tcl_SplitList(interp, list, &argc, &argv) == TCL_OK)
	{
		TkshSetListMode(toMode);
		result = Tcl_Merge(argc, argv);
	}
	TkshSetListMode(oldMode);
	return result;
}

int Tcl_SplitList(Tcl_Interp *interp, char *list, int *argcPtr, char ***argvPtr)
{
	int result = TCL_ERROR;
	FILE *str;
	static TkshArgcArgv argstruct, *args = &argstruct;
	char *command;
	static int init=0;

#ifndef NO_TCL_EVAL
	if (TkshListMode() == INTERP_TCL)
                return Tcl_TclSplitList(interp, list, argcPtr, argvPtr);
#endif

	if (! init)
	{
		sh_addbuiltin(SPLIT_CMD_NAME, tksh_cmd_split, (void *) args);
		init = 1;
	}

	command = (char *) malloc(strlen(list)+strlen(SPLIT_CMD_NAME)+2);
	dprintf2(("Splitting...\n"));
	if (args && command)
	{
		sprintf(command, "%s %s",SPLIT_CMD_NAME, list);

		args->argcPtr = argcPtr;
		args->argvPtr = argvPtr;

		if ((str = sfopen((Sfio_t *) 0, command, "s")))
		{
			sh_eval(str, 0x8000);
			sfclose(str);
			result = Tksh_OkOrErr();
		}
		free(command);
	}

	return result;
}


int Tcl_ScanElement(char *src, int *flagsPtr)
{
#ifndef NO_TCL_EVAL
	if (TkshListMode() == INTERP_TCL)
                return Tcl_TclScanElement(src, flagsPtr);
#endif
	return strlen(sh_fmtq(src));
}


int Tcl_ConvertElement(char *src, char *dst, int flags)
{
	char *str;
	int len;

#ifndef NO_TCL_EVAL
	if (TkshListMode() == INTERP_TCL)
                return Tcl_TclConvertElement(src, dst, flags);
#endif
	str = sh_fmtq(src);
	len = strlen(str);
	memcpy(dst, str, len+1);
	return len;
}


/*
 *----------------------------------------------------------------------
 *
 * Tcl_Merge --
 *
 *	Given a collection of strings, merge them together into a
 *	single string that has proper Tcl list structured (i.e.
 *	Tcl_SplitList may be used to retrieve strings equal to the
 *	original elements, and Tcl_Eval will parse the string back
 *	into its original elements).
 *
 * Results:
 *	The return value is the address of a dynamically-allocated
 *	string containing the merged list.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

char *
Tcl_Merge(argc, argv)
    int argc;			/* How many strings to merge. */
    char **argv;		/* Array of string values. */
{
#   define LOCAL_SIZE 20
    int localFlags[LOCAL_SIZE], *flagPtr;
    int numChars;
    char *result;
    register char *dst;
    int i;

#ifndef NO_TCL_EVAL
	if (TkshListMode() == INTERP_TCL)
                return Tcl_TclMerge(argc, argv);
#endif

    /*
     * Pass 1: estimate space, gather flags.
     */

    if (argc <= LOCAL_SIZE) {
	flagPtr = localFlags;
    } else {
	flagPtr = (int *) ckalloc((unsigned) argc*sizeof(int));
    }
    numChars = 1;
    for (i = 0; i < argc; i++) {
	numChars += Tcl_ScanElement(argv[i], &flagPtr[i]) + 1;
    }

    /*
     * Pass two: copy into the result area.
     */

    result = (char *) ckalloc((unsigned) numChars);
    dst = result;
    for (i = 0; i < argc; i++) {
	numChars = Tcl_ConvertElement(argv[i], dst, flagPtr[i]);
	dst += numChars;
	*dst = ' ';
	dst++;
    }
    if (dst == result) {
	*dst = 0;
    } else {
	dst[-1] = 0;
    }

    if (flagPtr != localFlags) {
	ckfree((char *) flagPtr);
    }
    return result;
}

/* #if TCL_MINOR_VERSION > 3 */
#if 0
int
Tcl_RegExpMatch(interp, string, pattern)
    Tcl_Interp *interp;         /* Used for error reporting. */
    char *string;               /* String. */
    char *pattern;              /* Regular expression to match against
                                 * string. */
{
	int match;
	char *regexp = fmtmatch(pattern);
	if (!regexp)
	{
		Tcl_ResetResult(interp);
		Tcl_AppendResult(interp, "error while matching regular expression: ",
			pattern, (char *) NULL);
        	return -1;
	}

	match = strmatch(string, regexp);

	/* free(regexp) */

	return match;
}

typedef struct TkshRegExp
{
	char *re;
	char *str;
	ssize_t pos[20];
	int n;
} TkshRegExp;

Tcl_RegExp Tcl_RegExpCompile(Tcl_Interp *interp, char *pattern)
{
	static TkshRegExp regexp;
	TkshRegExp *exp = &regexp;
	char *npattern;
	int len = strlen(pattern)+3;
	
	/* The following appears to be necessary, but check on this */

	npattern = (char *) malloc(len);
	strcpy(npattern+1, pattern);
	npattern[0] = '(';
	npattern[len-2] = ')';
	npattern[len-1] = 0;
	exp->re = fmtmatch(npattern);
	if (!exp->re)
	{
		Tcl_ResetResult(interp);
		Tcl_AppendResult(interp, "error while compiling regular expression: ", pattern, (char *) NULL);
		free(npattern);
        	return (Tcl_RegExp ) NULL ;
	}
	exp->re ++;
	exp->re[strlen(exp->re)-1] = 0;

	dprintf(("Compiled %s to %s then %s\n", pattern,npattern,exp->re));

	free(npattern);

	return (Tcl_RegExp) exp;
}

int
Tcl_RegExpExec(Tcl_Interp *interp, Tcl_RegExp regexp, char *string, char *start)
{
	TkshRegExp *exp = (TkshRegExp *) regexp;

	exp->str = string;
	exp->n = strgrpmatch(string, exp->re, exp->pos, 10, STR_MAXIMAL);
	/* Had ((string==start)? STR_LEFT:0), but this seems to be wrong */

#ifdef DEBUG
	{
		int i;
		dprintf(("Match result for pat. %s, str. %s (%s): %d\n",
			exp->re, string, (string==start)? "l" : "*", exp->n));
		for (i=1; i < exp->n; i++)
			dprintf(("From %d to %d\n", exp->pos[2*i],
				exp->pos[2*i+1]));
	}
#endif
	return (exp->n > 0) ? 1 : exp->n;
}

void Tcl_RegExpRange(Tcl_RegExp re, int index, char **startPtr, char **endPtr)
{
	TkshRegExp *exp = (TkshRegExp *) re;

	index++;
	if (index < exp->n)
	{
		*startPtr = exp->str + exp->pos[2*index];
		*endPtr = exp->str + exp->pos[2*index+1];
	}
	else
	{
		*startPtr = NULL;
		*endPtr = NULL;
	}
}


#endif
