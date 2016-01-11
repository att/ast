#pragma prototyped
#include "tkshlib.h"

/*
 * Forward references to procedures defined later in this file:
 */

static void	CleanupProc _ANSI_ARGS_((Proc *procPtr));
static  int	InterpProc _ANSI_ARGS_((ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv));
static  void	ProcDeleteProc _ANSI_ARGS_((ClientData clientData));

/*
 *----------------------------------------------------------------------
 *
 * Tcl_ProcCmd --
 *
 *	This procedure is invoked to process the "proc" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result value.
 *
 * Side effects:
 *	A new procedure gets created.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tcl_ProcCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    register Interp *iPtr = (Interp *) interp;
    register Proc *procPtr;
    int result, argCount, i;
    char **argArray = NULL;
    Arg *lastArgPtr;
    register Arg *argPtr = NULL;	/* Initialization not needed, but
					 * prevents compiler warning. */

    if (argc != 4) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" name args body\"", (char *) NULL);
	return TCL_ERROR;
    }

    procPtr = (Proc *) ckalloc(sizeof(Proc));
    procPtr->iPtr = iPtr;
    procPtr->refCount = 1;
    procPtr->command = (char *) ckalloc((unsigned) strlen(argv[3]) + 1);
    strcpy(procPtr->command, argv[3]);
    procPtr->argPtr = NULL;

    /*
     * Break up the argument list into argument specifiers, then process
     * each argument specifier.
     */

    result = Tcl_SplitList(interp, argv[2], &argCount, &argArray);
    if (result != TCL_OK) {
	goto procError;
    }
    lastArgPtr = NULL;
    for (i = 0; i < argCount; i++) {
	int fieldCount, nameLength, valueLength;
	char **fieldValues;

	/*
	 * Now divide the specifier up into name and default.
	 */

	result = Tcl_SplitList(interp, argArray[i], &fieldCount,
		&fieldValues);
	if (result != TCL_OK) {
	    goto procError;
	}
	if (fieldCount > 2) {
	    ckfree((char *) fieldValues);
	    Tcl_AppendResult(interp,
		    "too many fields in argument specifier \"",
		    argArray[i], "\"", (char *) NULL);
	    result = TCL_ERROR;
	    goto procError;
	}
	if ((fieldCount == 0) || (*fieldValues[0] == 0)) {
	    ckfree((char *) fieldValues);
	    Tcl_AppendResult(interp, "procedure \"", argv[1],
		    "\" has argument with no name", (char *) NULL);
	    result = TCL_ERROR;
	    goto procError;
	}
	nameLength = strlen(fieldValues[0]) + 1;
	if (fieldCount == 2) {
	    valueLength = strlen(fieldValues[1]) + 1;
	} else {
	    valueLength = 0;
	}
	argPtr = (Arg *) ckalloc((unsigned)
		(sizeof(Arg) - sizeof(argPtr->name) + nameLength
		+ valueLength));
	if (lastArgPtr == NULL) {
	    procPtr->argPtr = argPtr;
	} else {
	    lastArgPtr->nextPtr = argPtr;
	}
	lastArgPtr = argPtr;
	argPtr->nextPtr = NULL;
	strcpy(argPtr->name, fieldValues[0]);
	if (fieldCount == 2) {
	    argPtr->defValue = argPtr->name + nameLength;
	    strcpy(argPtr->defValue, fieldValues[1]);
	} else {
	    argPtr->defValue = NULL;
	}
	ckfree((char *) fieldValues);
    }

    Tcl_CreateCommand(interp, TkshMapName(argv[1]), InterpProc,
	    (ClientData) procPtr, ProcDeleteProc);
    ckfree((char *) argArray);
    return TCL_OK;

    procError:
    ckfree(procPtr->command);
    while (procPtr->argPtr != NULL) {
	argPtr = procPtr->argPtr;
	procPtr->argPtr = argPtr->nextPtr;
	ckfree((char *) argPtr);
    }
    ckfree((char *) procPtr);
    if (argArray != NULL) {
	ckfree((char *) argArray);
    }
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * TclIsProc --
 *
 *	Tells whether a command is a Tcl procedure or not.
 *
 * Results:
 *	If the given command is actuall a Tcl procedure, the
 *	return value is the address of the record describing
 *	the procedure.  Otherwise the return value is 0.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

Proc *
TclIsProc(cmdPtr)
    Tcl_CmdInfo *cmdPtr;		/* Command to test. */
{
    if (cmdPtr->proc == InterpProc) {
	return (Proc *) cmdPtr->clientData;
    }
    return (Proc *) 0;
}

Proc *
TclFindProc(Interp *iPtr, char *procName)
{
	Tcl_CmdInfo procInfo;
	if (Tcl_GetCommandInfo((Tcl_Interp *) iPtr, procName, &procInfo))
	{
		if (procInfo.proc == InterpProc)
		        return (Proc *) procInfo.clientData;
		
	}
	return (Proc *) 0;
}

struct InterpProcArgs
{
    	Proc *procPtr;
	Tcl_Interp *interp;
	int argc;
	char **argv;
};

static int InterpProcBody(void *);

static Namval_t savnv;

/*
 *----------------------------------------------------------------------
 *
 * InterpProc --
 *
 *	When a Tcl procedure gets invoked, this routine gets invoked
 *	to interpret the procedure.
 *
 * Results:
 *	A standard Tcl result value, usually TCL_OK.
 *
 * Side effects:
 *	Depends on the commands in the procedure.
 *
 *----------------------------------------------------------------------
 */

static int
InterpProc(clientData, interp, argc, argv)
    ClientData clientData;	/* Record describing procedure to be
				 * interpreted. */
    Tcl_Interp *interp;		/* Interpreter in which procedure was
				 * invoked. */
    int argc;			/* Count of number of arguments to this
				 * procedure. */
    char **argv;		/* Argument values. */
{
	register Proc *procPtr = (Proc *) clientData;
	struct InterpProcArgs iargs;
	int result;
	char **copiedArgv = NULL;

	iargs.procPtr = procPtr;
	iargs.interp = interp;
	iargs.argc = argc;
	iargs.argv = argv;

	if (argv[argc])
	{
		/* Somehow, this function was called without a null terminated
		 * argument list (eg. by tk4.2 directly).  We must create a
		 * proper list
		 */
		int i;
		copiedArgv = (char **) malloc((argc+1)*sizeof(char*));
		for (i=0; i < argc; i++)
			copiedArgv[i]=argv[i];
		copiedArgv[argc]=NULL;
		argv = copiedArgv;
	}

	result = sh_funscope(argc, argv, InterpProcBody, (void *) &iargs, 0);

	if (copiedArgv)
		free((void *) copiedArgv);

 	if (savnv.nvfun)
	{
		if (procPtr->iPtr->flags & ERR_IN_PROGRESS)
		{
			nv_unset(&savnv);
			procPtr->iPtr->flags |= ERR_IN_PROGRESS;
		}
		else
			nv_unset(&savnv);
	}
	return result;
}


static void clearlocals(Namval_t *nv, void *data)
{
        if (nv_isattr(nv, NV_REF))
                return;
	if (nv->nvfun)
	{
		dprintf(("LOCAL DELETE: %s\n", nv_name(nv)));
		savnv = *nv;
		nv->nvfun=NULL;
	}
}

static int InterpProcBody(void *data)
{
    struct InterpProcArgs *iargs = (struct InterpProcArgs *) data;
    int argc = iargs->argc;
    char **argv = iargs->argv;
    register Proc *procPtr = iargs->procPtr;
    Tcl_Interp *interp = iargs->interp;
    register Interp *iPtr = procPtr->iPtr;

    char **args, *value;
    register Arg *argPtr;
    int result;

    iPtr->returnCode = TCL_OK;

    /*
     * Match the actual arguments against the procedure's formal
     * parameters to compute local variables.
     */

    for (argPtr = procPtr->argPtr, args = argv+1, argc -= 1;
	    argPtr != NULL;
	    argPtr = argPtr->nextPtr, args++, argc--) {

	/*
	 * Handle the special case of the last formal being "args".  When
	 * it occurs, assign it a list consisting of all the remaining
	 * actual arguments.
	 */

	if ((argPtr->nextPtr == NULL)
		&& (strcmp(argPtr->name, "args") == 0)) {
	    if (argc < 0) {
		argc = 0;
	    }
	    value = Tcl_Merge(argc, args);
	    Tcl_SetVar(interp, argPtr->name, value, 0);
	    ckfree(value);
	    argc = 0;
	    break;
	} else if (argc > 0) {
	    value = *args;
	} else if (argPtr->defValue != NULL) {
	    value = argPtr->defValue;
	} else {
	    Tcl_AppendResult(interp, "no value given for parameter \"",
		    argPtr->name, "\" to \"", argv[0], "\"",
		    (char *) NULL);
	    result = TCL_ERROR;
	    goto procDone;
	}
	Tcl_SetVar(interp, argPtr->name, value, 0);
    }
    if (argc > 0) {
	Tcl_AppendResult(interp, "called \"", argv[0],
		"\" with too many arguments", (char *) NULL);
	result = TCL_ERROR;
	goto procDone;
    }

    /*
     * Invoke the commands in the procedure's body.
     */

    procPtr->refCount++;
	result = Tcl_TclEval(interp, procPtr->command);
	/* If the result is tempory (freeProc==NULL), make permanent */
	if (iPtr->freeProc == NULL)
		Tcl_SetResult(interp, iPtr->result, TCL_VOLATILE);
    procPtr->refCount--;
    if (procPtr->refCount <= 0) {
	CleanupProc(procPtr);
    }
    if (result == TCL_RETURN) {
	result = iPtr->returnCode;
	iPtr->returnCode = TCL_OK;
	if (result == TCL_ERROR) {
	    Tcl_SetVar2(interp, "errorCode", (char *) NULL,
		    (iPtr->errorCode != NULL) ? iPtr->errorCode : "NONE",
		    TCL_GLOBAL_ONLY);
	    iPtr->flags |= ERROR_CODE_SET;
	    if (iPtr->errorInfo != NULL) {
		Tcl_SetVar2(interp, "errorInfo", (char *) NULL,
			iPtr->errorInfo, TCL_GLOBAL_ONLY);
		iPtr->flags |= ERR_IN_PROGRESS;
	    }
	}
    } else if (result == TCL_ERROR) {
	char msg[100];

	/*
	 * Record information telling where the error occurred.
	 */

	sprintf(msg, "\n    (procedure \"%.50s\" line %d)", argv[0],
		iPtr->errorLine);
	Tcl_AddErrorInfo(interp, msg);
    } else if (result == TCL_BREAK) {
	iPtr->result = "invoked \"break\" outside of a loop";
	result = TCL_ERROR;
    } else if (result == TCL_CONTINUE) {
	iPtr->result = "invoked \"continue\" outside of a loop";
	result = TCL_ERROR;
    }

    /*
     * Delete the call frame for this procedure invocation (it's
     * important to remove the call frame from the interpreter
     * before deleting it, so that traces invoked during the
     * deletion don't see the partially-deleted frame).
     */

#if 0
    iPtr->framePtr = frame.callerPtr;
    iPtr->varFramePtr = frame.callerVarPtr;
    TclDeleteVars(iPtr, &frame.varTable);
#endif

    procDone:

	savnv.nvfun = NULL;
	if (hashscope(sh.var_tree))
	{
		Hashtab_t *ht = hashscope(sh.var_tree);
		hashscope(sh.var_tree) = NULL;
		nv_scan(sh.var_tree, clearlocals, NIL(void *), 0, 0);
		hashscope(sh.var_tree) = ht;
	}

	return result;
}



/*
 *----------------------------------------------------------------------
 *
 * ProcDeleteProc --
 *
 *	This procedure is invoked just before a command procedure is
 *	removed from an interpreter.  Its job is to release all the
 *	resources allocated to the procedure.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Memory gets freed, unless the procedure is actively being
 *	executed.  In this case the cleanup is delayed until the
 *	last call to the current procedure completes.
 *
 *----------------------------------------------------------------------
 */

static void
ProcDeleteProc(clientData)
    ClientData clientData;		/* Procedure to be deleted. */
{
    Proc *procPtr = (Proc *) clientData;

    procPtr->refCount--;
    if (procPtr->refCount <= 0) {
	CleanupProc(procPtr);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * CleanupProc --
 *
 *	This procedure does all the real work of freeing up a Proc
 *	structure.  It's called only when the structure's reference
 *	count becomes zero.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Memory gets freed.
 *
 *----------------------------------------------------------------------
 */

static void
CleanupProc(procPtr)
    register Proc *procPtr;		/* Procedure to be deleted. */
{
    register Arg *argPtr;

    ckfree((char *) procPtr->command);
    for (argPtr = procPtr->argPtr; argPtr != NULL; ) {
	Arg *nextPtr = argPtr->nextPtr;

	ckfree((char *) argPtr);
	argPtr = nextPtr;
    }
    ckfree((char *) procPtr);
}
