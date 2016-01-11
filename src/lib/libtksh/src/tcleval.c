#pragma prototyped
#include "tkshlib.h"

#define NAMEBUFLEN 512

char *TkshMapName(char *name)
{
	static char namebuf[NAMEBUFLEN+4] = "tcl_";
	char *mapname = namebuf;
	Namval_t *np;

	if (strlen(name) >= NAMEBUFLEN)
	{
		mapname = (char *) malloc(strlen(name)+5);
		memcpy(mapname, namebuf, 4);
	}
	strcpy(mapname+4, name);
	if ((np = nv_open(mapname, sh_bltin_tree(), NV_NOADD)) && np->nvalue)
		name = nv_name(np);
	else
		name = TkshMapKeyword(name);
	if (namebuf != mapname)
		free(mapname);
	return name;
}


/*
 *----------------------------------------------------------------------
 *
 * TclUpdateReturnInfo --
 *
 *	This procedure is called when procedures return, and at other
 *	points where the TCL_RETURN code is used.  It examines fields
 *	such as iPtr->returnCode and iPtr->errorCode and modifies
 *	the real return status accordingly.
 *
 * Results:
 *	The return value is the true completion code to use for
 *	the procedure, instead of TCL_RETURN.
 *
 * Side effects:
 *	The errorInfo and errorCode variables may get modified.
 *
 *----------------------------------------------------------------------
 */

int
TclUpdateReturnInfo(iPtr)
    Interp *iPtr;		/* Interpreter for which TCL_RETURN
				 * exception is being processed. */
{
    int code;

    code = iPtr->returnCode;
    iPtr->returnCode = TCL_OK;
    if (code == TCL_ERROR) {
	Tcl_SetVar2((Tcl_Interp *) iPtr, "errorCode", (char *) NULL,
		(iPtr->errorCode != NULL) ? iPtr->errorCode : "NONE",
		TCL_GLOBAL_ONLY);
	iPtr->flags |= ERROR_CODE_SET;
	if (iPtr->errorInfo != NULL) {
	    Tcl_SetVar2((Tcl_Interp *) iPtr, "errorInfo", (char *) NULL,
		    iPtr->errorInfo, TCL_GLOBAL_ONLY);
	    iPtr->flags |= ERR_IN_PROGRESS;
	}
    }
    return code;
}

/*
 *-----------------------------------------------------------------
 *
 * Tcl_Eval --
 *
 *	Parse and execute a command in the Tcl language.
 *
 * Results:
 *	The return value is one of the return codes defined in tcl.hd
 *	(such as TCL_OK), and interp->result contains a string value
 *	to supplement the return code.  The value of interp->result
 *	will persist only until the next call to Tcl_Eval:  copy it or
 *	lose it! *TermPtr is filled in with the character just after
 *	the last one that was part of the command (usually a NULL
 *	character or a closing bracket).
 *
 * Side effects:
 *	Almost certainly;  depends on the command.
 *
 *-----------------------------------------------------------------
 */

int
Tcl_TclEval(interp, cmd)
    Tcl_Interp *interp;		/* Token for command interpreter (returned
				 * by a previous call to Tcl_CreateInterp). */
    char *cmd;			/* Pointer to TCL command to interpret. */
{
    /*
     * The storage immediately below is used to generate a copy
     * of the command, after all argument substitutions.  Pv will
     * contain the argv values passed to the command procedure.
     */

#   define NUM_CHARS 200
    char copyStorage[NUM_CHARS];
    ParseValue pv;
    char *oldBuffer;

    /*
     * This procedure generates an (argv, argc) array for the command,
     * It starts out with stack-allocated space but uses dynamically-
     * allocated storage to increase it if needed.
     */

#   define NUM_ARGS 10
    char *(argStorage[NUM_ARGS]);
    char **argv = argStorage;
    int argc;
    int argSize = NUM_ARGS;

    register char *src;			/* Points to current character
					 * in cmd. */
    char termChar;			/* Return when this character is found
					 * (either ']' or '\0').  Zero means
					 * that newlines terminate commands. */
    int flags;				/* Interp->evalFlags value when the
					 * procedure was called. */
    int result;				/* Return value. */
    register Interp *iPtr = (Interp *) interp;
    char *termPtr;			/* Contains character just after the
					 * last one in the command. */
    char *cmdStart;			/* Points to first non-blank char. in
					 * command (used in calling trace
					 * procedures). */
    char *ellipsis = "";		/* Used in setting errorInfo variable;
					 * set to "..." to indicate that not
					 * all of offending command is included
					 * in errorInfo.  "" means that the
					 * command is all there. */
#ifdef TKSH_NOT_USED
    Tcl_HashEntry *hPtr;
    register Trace *tracePtr;
#else
	Namval_t *nv;
	int oldInterpType;

	dprintf(("------- TCL EVAL ------------\n"));
	oldInterpType = iPtr->interpType;
	iPtr->interpType = INTERP_TCL;
#endif

    /*
     * Initialize the result to an empty string and clear out any
     * error information.  This makes sure that we return an empty
     * result if there are no commands in the command string.
     */

    Tcl_FreeResult((Tcl_Interp *) iPtr);
    iPtr->result = iPtr->resultSpace;
    iPtr->resultSpace[0] = 0;
    result = TCL_OK;

    /*
     * Initialize the area in which command copies will be assembled.
     */

    pv.buffer = copyStorage;
    pv.end = copyStorage + NUM_CHARS - 1;
    pv.expandProc = TclExpandParseValue;
    pv.clientData = (ClientData) NULL;

    src = cmd;
    flags = iPtr->evalFlags;
    iPtr->evalFlags = 0;
    if (flags & TCL_BRACKET_TERM) {
	termChar = ']';
    } else {
	termChar = 0;
    }
    termPtr = src;
    cmdStart = src;

    /*
     * Check depth of nested calls to Tcl_Eval:  if this gets too large,
     * it's probably because of an infinite loop somewhere.
     */

    iPtr->numLevels++;
    if (iPtr->numLevels > iPtr->maxNestingDepth) {
	iPtr->numLevels--;
	iPtr->result =  "too many nested calls to Tcl_Eval (infinite loop?)";
	iPtr->termPtr = termPtr;
	iPtr->interpType = oldInterpType;
	return TCL_ERROR;
    }

    /*
     * There can be many sub-commands (separated by semi-colons or
     * newlines) in one command string.  This outer loop iterates over
     * individual commands.
     */

    while (*src != termChar) {
	iPtr->flags &= ~(ERR_IN_PROGRESS | ERROR_CODE_SET);

	/*
	 * Skim off leading white space and semi-colons, and skip
	 * comments.
	 */

	while (1) {
	    register char c = *src;

	    if ((CHAR_TYPE(c) != TCL_SPACE) && (c != ';') && (c != '\n')) {
		break;
	    }
	    src += 1;
	}
	if (*src == '#') {
	    for (src++; *src != 0; src++) {
		if ((*src == '\n') && (src[-1] != '\\')) {
		    src++;
		    termPtr = src;
		    break;
		}
	    }
	    continue;
	}
	cmdStart = src;

	/*
	 * Parse the words of the command, generating the argc and
	 * argv for the command procedure.  May have to call
	 * TclParseWords several times, expanding the argv array
	 * between calls.
	 */

	pv.next = oldBuffer = pv.buffer;
	argc = 0;
	while (1) {
	    int newArgs, maxArgs;
	    char **newArgv;
	    int i;

	    /*
	     * Note:  the "- 2" below guarantees that we won't use the
	     * last two argv slots here.  One is for a NULL pointer to
	     * mark the end of the list, and the other is to leave room
	     * for inserting the command name "unknown" as the first
	     * argument (see below).
	     */

	    maxArgs = argSize - argc - 2;
	    result = TclParseWords((Tcl_Interp *) iPtr, src, flags,
		    maxArgs, &termPtr, &newArgs, &argv[argc], &pv);
	    src = termPtr;
	    if (result != TCL_OK) {
		ellipsis = "...";
		goto done;
	    }

	    /*
	     * Careful!  Buffer space may have gotten reallocated while
	     * parsing words.  If this happened, be sure to update all
	     * of the older argv pointers to refer to the new space.
	     */

	    if (oldBuffer != pv.buffer) {
		int i;

		for (i = 0; i < argc; i++) {
		    argv[i] = pv.buffer + (argv[i] - oldBuffer);
		}
		oldBuffer = pv.buffer;
	    }
	    argc += newArgs;
	    if (newArgs < maxArgs) {
		argv[argc] = (char *) NULL;
		break;
	    }

	    /*
	     * Args didn't all fit in the current array.  Make it bigger.
	     */

	    argSize *= 2;
	    newArgv = (char **)
		    ckalloc((unsigned) argSize * sizeof(char *));
	    for (i = 0; i < argc; i++) {
		newArgv[i] = argv[i];
	    }
	    if (argv != argStorage) {
		ckfree((char *) argv);
	    }
	    argv = newArgv;
	}

	/*
	 * If this is an empty command (or if we're just parsing
	 * commands without evaluating them), then just skip to the
	 * next command.
	 */

	if ((argc == 0) || iPtr->noEval) {
	    continue;
	}
	argv[argc] = NULL;

	/*
	 * Save information for the history module, if needed.
	 */

#ifdef TKSH_NOT_USED
	if (flags & TCL_RECORD_BOUNDS) {
	    iPtr->evalFirst = cmdStart;
	    iPtr->evalLast = src-1;
	}

	/*
	 * Find the procedure to execute this command.  If there isn't
	 * one, then see if there is a command "unknown".  If so,
	 * invoke it instead, passing it the words of the original
	 * command as arguments.
	 */

	hPtr = Tcl_FindHashEntry(&iPtr->commandTable, argv[0]);
	if (hPtr == NULL) {
	    int i;

	    hPtr = Tcl_FindHashEntry(&iPtr->commandTable, "unknown");
	    if (hPtr == NULL) {
		Tcl_ResetResult(interp);
		Tcl_AppendResult(interp, "invalid command name \"",
			argv[0], "\"", (char *) NULL);
		result = TCL_ERROR;
		goto done;
	    }
	    for (i = argc; i >= 0; i--) {
		argv[i+1] = argv[i];
	    }
	    argv[0] = "unknown";
	    argc++;
	}
	cmdPtr = (Command *) Tcl_GetHashValue(hPtr);

	/*
	 * Call trace procedures, if any.
	 */

	for (tracePtr = iPtr->tracePtr; tracePtr != NULL;
		tracePtr = tracePtr->nextPtr) {
	    char saved;

	    if (tracePtr->level < iPtr->numLevels) {
		continue;
	    }
	    saved = *src;
	    *src = 0;
	    (*tracePtr->proc)(tracePtr->clientData, interp, iPtr->numLevels,
		    cmdStart, cmdPtr->proc, cmdPtr->clientData, argc, argv);
	    *src = saved;
	}

#endif
	/*
	 * At long last, invoke the command procedure.  Reset the
	 * result to its default empty value first (it could have
	 * gotten changed by earlier commands in the same command
	 * string).
	 */

#ifdef TKSH_NOT_USED
	iPtr->cmdCount++;
#endif
	Tcl_FreeResult(iPtr);
	iPtr->result = iPtr->resultSpace;
	iPtr->resultSpace[0] = 0;
#ifdef TKSH_NOT_USED
	result= (*cmdPtr->proc)(cmdPtr->clientData, interp, argc, argv);
#else
	nv=nv_search(TkshMapName(argv[0]),sh_bltin_tree(),0);
	if (nv && nv->nvalue)
	{
		Shbltin_t bd;
		Sfio_t *f = NIL(Sfio_t *); char *s;
		if (! nv->nvfun)	/* KSH builtin */
		{
			sfstack(sfstdout, f=sftmp(4096));
			dprintfArgs("Tcl_Eval (ksh direct)", argc, argv);
		}
		else
			dprintfArgs("Tcl_Eval (Tcl direct)", argc, argv);
		/* NOTE: 2008-03-16 &sh is a cheat here */
		bd = *(Shbltin_t*)((Interp*)interp)->shbltin;
		bd.shp = &sh;
		bd.ptr = nv->nvfun;
		result = (*((ShellProc_t) nv->nvalue))(argc, argv, &bd);
		if (f)
		{
			sfstack(sfstdout, NIL(Sfio_t *));
			sfputc(f,0);	/* null terminate */
			sfseek(f,0L,SEEK_SET);
			s = sfreserve(f,SF_UNBOUND,-1);
			if ( s[sfvalue(f)-2] == '\n' )
				s[sfvalue(f)-2] = 0;
			Tcl_SetResult(interp, s, TCL_VOLATILE);
			sfclose(f);
		}
	}
	else
	{
		/* We need to check aliases too XX - also check mapped name? */
		nv = nv_search(argv[0], sh.fun_tree, 0);
		if (nv && nv->nvalue)
		{
			Sfio_t *tclcommand;
			int oldMode;
			char *cmd;

 			oldMode = TkshSetListMode(iPtr->interpType=INTERP_KSH);
			cmd  = Tcl_Merge(argc, argv);
 			TkshSetListMode(oldMode);
			dprintf(("Tcl_Eval (ksh): %s\n", cmd));
			if ((tclcommand = sfopen((Sfio_t *) 0, cmd, "s")))
				sh_eval(tclcommand,0x8000);	/* closed in sh_eval */
			iPtr->interpType = INTERP_TCL;
 			/* TkshSetListMode(oldMode); */
			ckfree(cmd);
			result = Tksh_ReturnVal();
		}
		else 
		{
			int i;
			dprintf(("Tcl_Eval: (unknown) %s\n", cmd));
			nv = nv_search("unknown", sh_bltin_tree(), 0);
			if (!nv || !nv->nvalue)
			{
				Tcl_ResetResult(interp);
				Tcl_AppendResult(interp,
					"invalid command name \"", argv[0],
					"\"", (char *) NULL);
				result = TCL_ERROR;
				goto done;
			}
			for (i = argc; i >= 0; i--) {
				argv[i+1] = argv[i];
			}
			argv[0] = "unknown";
			argc++;
			result = (*((ShellProc_t) nv->nvalue))(argc, argv,
 				(void *) nv->nvfun);
		}
	}
#endif
	if (Tcl_AsyncReady()) {
	    result = Tcl_AsyncInvoke(interp, result);
	}
	if (result != TCL_OK) {
	    break;
	}
    }

    done:

    /*
     * Free up any extra resources that were allocated.
     */

    if (pv.buffer != copyStorage) {
	ckfree((char *) pv.buffer);
    }
    if (argv != argStorage) {
	ckfree((char *) argv);
    }
    iPtr->numLevels--;
    if (iPtr->numLevels == 0) {
	if (result == TCL_RETURN) {
	    result = TclUpdateReturnInfo(iPtr);
	}
#if TCL_MINOR_VERSION == 3
	if ((result != TCL_OK) && (result != TCL_ERROR)) {
#else
	if ((result != TCL_OK) && (result != TCL_ERROR)
		&& !(flags & TCL_ALLOW_EXCEPTIONS)) {

#endif
	    Tcl_ResetResult(interp);
	    if (result == TCL_BREAK) {
		iPtr->result = "invoked \"break\" outside of a loop";
	    } else if (result == TCL_CONTINUE) {
		iPtr->result = "invoked \"continue\" outside of a loop";
	    } else {
		iPtr->result = iPtr->resultSpace;
		sprintf(iPtr->resultSpace, "command returned bad code: %d",
			result);
	    }
	    result = TCL_ERROR;
	}
	if (iPtr->flags & DELETED) {
	    /*
	     * Someone tried to delete the interpreter, but it couldn't
	     * actually be deleted because commands were in the middle of
	     * being evaluated.  Delete the interpreter now.  Also, return
	     * immediately:  we can't execute the remaining code in the
	     * procedure because it accesses fields of the dead interpreter.
	     */

	    Tcl_DeleteInterp(interp);
	    return result;
	}
    }

    /*
     * If an error occurred, record information about what was being
     * executed when the error occurred.
     */

    if ((result == TCL_ERROR) && !(iPtr->flags & ERR_ALREADY_LOGGED)) {
	int numChars;
	register char *p;

	/*
	 * Compute the line number where the error occurred.
	 */

	iPtr->errorLine = 1;
	for (p = cmd; p != cmdStart; p++) {
	    if (*p == '\n') {
		iPtr->errorLine++;
	    }
	}
	for ( ; isspace(UCHAR(*p)) || (*p == ';'); p++) {
	    if (*p == '\n') {
		iPtr->errorLine++;
	    }
	}

	/*
	 * Figure out how much of the command to print in the error
	 * message (up to a certain number of characters, or up to
	 * the first new-line).
	 */

	numChars = src - cmdStart;
	if (numChars > (NUM_CHARS-50)) {
	    numChars = NUM_CHARS-50;
	    ellipsis = " ...";
	}

	if (!(iPtr->flags & ERR_IN_PROGRESS)) {
	    sprintf(copyStorage, "\n    while executing\n\"%.*s%s\"",
		    numChars, cmdStart, ellipsis);
	} else {
	    sprintf(copyStorage, "\n    invoked from within\n\"%.*s%s\"",
		    numChars, cmdStart, ellipsis);
	}
	Tcl_AddErrorInfo(interp, copyStorage);
	iPtr->flags &= ~ERR_ALREADY_LOGGED;
    } else {
	iPtr->flags &= ~ERR_ALREADY_LOGGED;
    }
    iPtr->termPtr = termPtr;
	sh_sigcheck(0);
	iPtr->interpType = oldInterpType;
    return result;
}
