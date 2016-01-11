#include "tkshlib.h"
#include "tclcmd.h"
#include "nvextra.h"

typedef Namval_t Var;

typedef struct ArraySearch
{
	Var *nv;
	char *sub;
	int id;
	struct ArraySearch *nextPtr;
} ArraySearch;

static ArraySearch *    ParseSearchId _ANSI_ARGS_((Tcl_Interp *interp,
                            Var *varPtr, char *varName, char *string));

static ArraySearch *MakeSearchId _ANSI_ARGS_((Tcl_Interp *interp,Namval_t *nv,char *varName));

/*
 *----------------------------------------------------------------------
 *
 * Tcl_SetCmd --
 *
 *	This procedure is invoked to process the "set" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result value.
 *
 * Side effects:
 *	A variable's value may be changed.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tcl_SetCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    register Tcl_Interp *interp;	/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    if (argc == 2) {
	char *value;

	value = Tcl_GetVar(interp, argv[1], TCL_LEAVE_ERR_MSG);
	if (value == NULL) {
	    return TCL_ERROR;
	}
	interp->result = value;
	return TCL_OK;
    } else if (argc == 3) {
	char *result;

	result = Tcl_SetVar(interp, argv[1], argv[2], TCL_LEAVE_ERR_MSG);
	if (result == NULL) {
	    return TCL_ERROR;
	}
	interp->result = result;
	return TCL_OK;
    } else {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " varName ?newValue?\"", (char *) NULL);
	return TCL_ERROR;
    }
}

int
Tcl_GlobalCmd(dummy, interp, argc, argv)
    ClientData dummy;                   /* Not used. */
    Tcl_Interp *interp;                 /* Current interpreter. */
    int argc;                           /* Number of arguments. */
    char **argv;                        /* Argument strings. */
{

    if (argc < 2) {
        Tcl_AppendResult(interp, "wrong # args: should be \"",
                argv[0], " varName ?varName ...?\"", (char *) NULL);
        return TCL_ERROR;
    }

    for (argc--, argv++; argc > 0; argc--, argv++) {
	if (TkshUpVar(interp, *argv, *argv, NULL, nv_globalscope()) != TCL_OK)
            return TCL_ERROR;
    }
    return TCL_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * Tcl_UnsetCmd --
 *
 *	This procedure is invoked to process the "unset" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result value.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tcl_UnsetCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    register Tcl_Interp *interp;	/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    int i;

    if (argc < 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " varName ?varName ...?\"", (char *) NULL);
	return TCL_ERROR;
    }
    for (i = 1; i < argc; i++) {
	if (Tcl_UnsetVar(interp, argv[i], TCL_LEAVE_ERR_MSG) != TCL_OK) {
	    return TCL_ERROR;
	}
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_AppendCmd --
 *
 *	This procedure is invoked to process the "append" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result value.
 *
 * Side effects:
 *	A variable's value may be changed.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tcl_AppendCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    register Tcl_Interp *interp;	/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    int i;
    char *result = NULL;		/* (Initialization only needed to keep
					 * the compiler from complaining) */

#if TCL_MINOR_VERSION == 3
    if (argc < 3) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " varName value ?value ...?\"", (char *) NULL);
	return TCL_ERROR;
    }
#else
    if (argc < 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " varName ?value value ...?\"", (char *) NULL);
	return TCL_ERROR;
    }
    if (argc == 2) {
	result = Tcl_GetVar(interp, argv[1], TCL_LEAVE_ERR_MSG);
	if (result == NULL) {
	    return TCL_ERROR;
	}
	interp->result = result;
	return TCL_OK;
    }
#endif

    for (i = 2; i < argc; i++) {
	result = Tcl_SetVar(interp, argv[1], argv[i],
		TCL_APPEND_VALUE|TCL_LEAVE_ERR_MSG);
	if (result == NULL) {
	    return TCL_ERROR;
	}
    }
    interp->result = result;
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_LappendCmd -- MODIFIED
 *
 *	This procedure is invoked to process the "lappend" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result value.
 *
 * Side effects:
 *	A variable's value may be changed.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tcl_LappendCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    register Tcl_Interp *interp;	/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    int i;
    char *result = NULL;		/* (Initialization only needed to keep
					 * the compiler from complaining) */

    if (argc < 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " varName ?value value ...?\"", (char *) NULL);
	return TCL_ERROR;
    }
    if (argc == 2) {
	result = Tcl_GetVar(interp, argv[1], TCL_LEAVE_ERR_MSG);
	if (result == NULL) {
	    return TCL_ERROR;
	}
	interp->result = result;
	return TCL_OK;
    }

    for (i = 2; i < argc; i++) {
	result = Tcl_SetVar(interp, argv[1],  argv[i],
		TCL_APPEND_VALUE|TCL_LIST_ELEMENT|TCL_LEAVE_ERR_MSG);
	if (result == NULL) {
	    return TCL_ERROR;
	}
    }
    interp->result = result;
    return TCL_OK;
}

#define MakeUpvar(iPtr, framePtr, nm1, nm2, newname, notused2) \
	TkshUpVar((Tcl_Interp*) iPtr,(newname),(nm1),(nm2),(framePtr)->var_tree)

int TclGetFrame(interp, string, framePtr)
	Tcl_Interp *interp;
	char *string;
	CallFrame **framePtr;
{
	int level, mode = 0, result = 1;
	CallFrame *f;

	if (!string)
		return 0;
	if (framePtr == NULL)
		return -1;

	if (*string == '#') {
		if (Tcl_GetInt(interp, string+1, &level) != TCL_OK) {
			return -1;
		}
		if (level < 0) {
		   levelError:
			Tcl_AppendResult(interp, "bad level \"", string, "\"",
				(char *) NULL);
			return -1;
		}
	}  else if (isdigit(UCHAR(*string))) {
	        if (Tcl_GetInt(interp, string, &level) != TCL_OK) {
	            return -1;
	        }
		mode = 1;
	} else {
		level = 1;
		mode = 1;
		result = 0;
	}

	if ((f = sh_getscope(level, mode)))
	{
		*framePtr = f;
		return result;
	}
	goto levelError;
}


/*
 *----------------------------------------------------------------------
 *
 * Tcl_UpvarCmd
 *
 *	This procedure is invoked to process the "upvar" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result value.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tcl_UpvarCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    register Interp *iPtr = (Interp *) interp;
    int result;
    CallFrame *framePtr;
    register char *p;

    if (argc < 3) {
	upvarSyntax:
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" ?level? otherVar localVar ?otherVar localVar ...?\"",
		(char *) NULL);
	return TCL_ERROR;
    }

    /*
     * Find the hash table containing the variable being referenced.
     */

    result = TclGetFrame(interp, argv[1], &framePtr);
    if (result == -1) {
	return TCL_ERROR;
    }
    argc -= result+1;
    if ((argc & 1) != 0) {
	goto upvarSyntax;
    }
    argv += result+1;

    /*
     * Iterate over all the pairs of (other variable, local variable)
     * names.  For each pair, divide the other variable name into two
     * parts, then call MakeUpvar to do all the work of creating linking
     * it to the local variable.
     */

    for ( ; argc > 0; argc -= 2, argv += 2) {
	for (p = argv[0]; *p != 0; p++) {
	    if (*p == '(') {
		char *openParen = p;

		do {
		    p++;
		} while (*p != '\0');
		p--;
		if (*p != ')') {
		    goto scalar;
		}
		*openParen = '\0';
		*p = '\0';
		result = MakeUpvar(iPtr, framePtr, argv[0], openParen+1,
			argv[1], 0);
		*openParen = '(';
		*p = ')';
		goto checkResult;
	    }
	}
	scalar:
	result = MakeUpvar(iPtr, framePtr, argv[0], (char *) NULL, argv[1], 0);

	checkResult:
	if (result != TCL_OK) {
	    return TCL_ERROR;
	}
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_UplevelCmd --
 *
 *	This procedure is invoked to process the "uplevel" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result value.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tcl_UplevelCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    int result;
    CallFrame *savedVarFramePtr, *framePtr;

#ifndef NEWKSH
	CallFrame savedFrame;
#endif

    if (argc < 2) {
	uplevelSyntax:
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" ?level? command ?arg ...?\"", (char *) NULL);
	return TCL_ERROR;
    }

    /*
     * Find the level to use for executing the command.
     */

    result = TclGetFrame(interp, argv[1], &framePtr);
    if (result == -1) {
	return TCL_ERROR;
    }
    argc -= (result+1);
    if (argc == 0) {
	goto uplevelSyntax;
    }
    argv += (result+1);

    /*
     * Modify the interpreter state to execute in the given frame.
     */

#ifdef NEWKSH
	savedVarFramePtr = sh_setscope(framePtr);
#else
	sh_setscope(framePtr, &savedFrame);
#endif

    /*
     * Execute the residual arguments as a command.
     */

    if (argc == 1) {
	result = Tcl_Eval(interp, argv[0]);
    } else {
	char *cmd;

	cmd = Tcl_Concat(argc, argv);
	result = Tcl_Eval(interp, cmd);
	ckfree(cmd);
    }
    if (result == TCL_ERROR) {
	char msg[60];
	sprintf(msg, "\n    (\"uplevel\" body line %d)", interp->errorLine);
	Tcl_AddErrorInfo(interp, msg);
    }

    /*
     * Restore the variable frame, and return.
     */

#ifdef NEWKSH
	framePtr = sh_setscope(savedVarFramePtr);
#else
	sh_setscope(&savedFrame, framePtr);
#endif

    return result;
}




/*
 *----------------------------------------------------------------------
 *
 * Tcl_UpVar --
 *
 *	This procedure links one variable to another, just like
 *	the "upvar" command.
 *
 * Results:
 *	A standard Tcl completion code.  If an error occurs then
 *	an error message is left in interp->result.
 *
 * Side effects:
 *	The variable in frameName whose name is given by varName becomes
 *	accessible under the name localName, so that references to
 *	localName are redirected to the other variable like a symbolic
 *	link.
 *
 *----------------------------------------------------------------------
 */

int
Tcl_UpVar(interp, frameName, varName, localName, flags)
    Tcl_Interp *interp;		/* Command interpreter in which varName is
				 * to be looked up. */
    char *frameName;		/* Name of the frame containing the source
				 * variable, such as "1" or "#0". */
    char *varName;		/* Name of a variable in interp.  May be
				 * either a scalar name or an element
				 * in an array. */
    char *localName;		/* Destination variable name. */
    int flags;			/* Either 0 or TCL_GLOBAL_ONLY;  indicates
				 * whether localName is local or global. */
{
    int result;
    CallFrame *framePtr;
    register char *p;

    result = TclGetFrame(interp, frameName, &framePtr);
    if (result == -1) {
	return TCL_ERROR;
    }

    /*
     * Figure out whether this is an array reference, then call
     * MakeUpVar to do all the real work.
     */

    for (p = varName; *p != '\0'; p++) {
	if (*p == '(') {
	    char *openParen = p;

	    do {
		p++;
	    } while (*p != '\0');
	    p--;
	    if (*p != ')') {
		goto scalar;
	    }
	    *openParen = '\0';
	    *p = '\0';
	    result = MakeUpvar((Interp *) interp, framePtr, varName,
		    openParen+1, localName, flags);
	    *openParen = '(';
	    *p = ')';
	    return result;
	}
    }

    scalar:
    return MakeUpvar((Interp *) interp, framePtr, varName, (char *) NULL,
	    localName, flags);
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_UpVar2 --
 *
 *	This procedure links one variable to another, just like
 *	the "upvar" command.
 *
 * Results:
 *	A standard Tcl completion code.  If an error occurs then
 *	an error message is left in interp->result.
 *
 * Side effects:
 *	The variable in frameName whose name is given by part1 and
 *	part2 becomes accessible under the name localName, so that
 *	references to localName are redirected to the other variable
 *	like a symbolic link.
 *
 *----------------------------------------------------------------------
 */

int
Tcl_UpVar2(interp, frameName, part1, part2, localName, flags)
    Tcl_Interp *interp;		/* Interpreter containing variables.  Used
				 * for error messages too. */
    char *frameName;		/* Name of the frame containing the source
				 * variable, such as "1" or "#0". */
    char *part1, *part2;	/* Two parts of source variable name. */
    char *localName;		/* Destination variable name. */
    int flags;			/* TCL_GLOBAL_ONLY or 0. */
{
    int result;
    CallFrame *framePtr;

    result = TclGetFrame(interp, frameName, &framePtr);
    if (result == -1) {
	return TCL_ERROR;
    }
    return MakeUpvar((Interp *) interp, framePtr, part1, part2,
	    localName, flags);
}


/*
 *----------------------------------------------------------------------
 *
 * Tcl_ArrayCmd -- MODIFIED
 *
 *	This procedure is invoked to process the "array" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result value.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tcl_ArrayCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    register Tcl_Interp *interp;	/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    int c, notArray;
    size_t length;
    Var *varPtr = NULL;         /* Initialization needed only to prevent
                                 * compiler warning. */

    if (argc < 3) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " option arrayName ?arg ...?\"", (char *) NULL);
	return TCL_ERROR;
    }

    /*
     * Locate the array variable (and it better be an array).
     */

	varPtr = TkshAccessVar(interp, argv[2], NULL);
	notArray = 1;
	if (varPtr && nv_isarray(varPtr))
		notArray = 0;

    /*
     * Dispatch based on the option.
     */

    c = argv[1][0];
    length = strlen(argv[1]);
    if ((c == 'a') && (strncmp(argv[1], "anymore", length) == 0)) {
	ArraySearch *searchPtr;

	if (argc != 4) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " anymore arrayName searchId\"", (char *) NULL);
	    return TCL_ERROR;
	}
	if (notArray) {
	    goto error;
	}
	searchPtr = ParseSearchId(interp, varPtr, argv[2], argv[3]);
	if (searchPtr == NULL) {
	    return TCL_ERROR;
	}
	nv_scanfrom(searchPtr->nv, searchPtr->sub);
	if (nv_nextsub(searchPtr->nv))
		interp->result = "1";
	else
		interp->result = "0";
	nv_putsub(searchPtr->nv, NULL, ARRAY_UNDEF);
	return TCL_OK;
    } else if ((c == 'd') && (strncmp(argv[1], "donesearch", length) == 0)) {
	ArraySearch *searchPtr, *prevPtr;
	TkshArrayInfo *arrayInfo;

	if (argc != 4) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " donesearch arrayName searchId\"", (char *) NULL);
	    return TCL_ERROR;
	}
	if (notArray) {
	    goto error;
	}
	searchPtr = ParseSearchId(interp, varPtr, argv[2], argv[3]);
	arrayInfo = TkshArrayData(varPtr);
	if ((searchPtr == NULL) || (arrayInfo == NULL)) {
	    return TCL_ERROR;
	}
	if (searchPtr == (ArraySearch *) arrayInfo->clientData) {
		arrayInfo->clientData = searchPtr->nextPtr;
	} else {
	   for (prevPtr = arrayInfo->clientData; ;prevPtr = prevPtr->nextPtr) {
       	         if (prevPtr->nextPtr == searchPtr) {
                    prevPtr->nextPtr = searchPtr->nextPtr;
                    break;
                }
            }
	}
	ckfree((char *) searchPtr);
    } else if ((c == 'e') && (strncmp(argv[1], "exists", length) == 0)) {
	if (argc != 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " exists arrayName\"", (char *) NULL);
	    return TCL_ERROR;
	}
	interp->result = (notArray) ? "0" : "1";
    } else if ((c == 'g') && (strncmp(argv[1], "get", length) == 0)) {
	char *name;

	if ((argc != 3) && (argc != 4)) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " get arrayName ?pattern?\"", (char *) NULL);
	    return TCL_ERROR;
	}
	if (notArray) {
	    return TCL_OK;
	}
	nv_putsub(varPtr, NULL, ARRAY_SCAN);
	name = nv_getsub(varPtr);
	while (name)
	{
	    	if ((argc != 4) || Tcl_StringMatch(name, argv[3]))
		{
			Tcl_AppendElement(interp, name);
			Tcl_AppendElement(interp, nv_getval(varPtr));
		}
		if (! nv_nextsub(varPtr))
			break;
		name = nv_getsub(varPtr);
	}
	nv_putsub(varPtr, NULL, ARRAY_UNDEF);
    } else if ((c == 'n') && (strncmp(argv[1], "names", length) == 0)
	    && (length >= 2)) {
	char *name;

	if ((argc != 3) && (argc != 4)) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " names arrayName ?pattern?\"", (char *) NULL);
	    return TCL_ERROR;
	}
	if (notArray) {
	    return TCL_OK;
	}

	nv_putsub(varPtr, NULL, ARRAY_SCAN);
	name = nv_getsub(varPtr);
	while (name)
	{
	    	if ((argc != 4) || Tcl_StringMatch(name, argv[3]))
			Tcl_AppendElement(interp, name);
		if (! nv_nextsub(varPtr))
			break;
		name = nv_getsub(varPtr);
	}
	nv_putsub(varPtr, NULL, ARRAY_UNDEF);
	
    } else if ((c == 'n') && (strncmp(argv[1], "nextelement", length) == 0)
	    && (length >= 2)) {
	ArraySearch *searchPtr;

	if (argc != 4) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " nextelement arrayName searchId\"",
		    (char *) NULL);
	    return TCL_ERROR;
	}
	if (notArray) {
	    goto error;
	}
	searchPtr = ParseSearchId(interp, varPtr, argv[2], argv[3]);
	if (searchPtr == NULL) {
	    return TCL_ERROR;
	}
#if 0	/* Tcl version */
	while (1) {
	    Var *varPtr2;

	    hPtr = searchPtr->nextEntry;
	    if (hPtr == NULL) {
		hPtr = Tcl_NextHashEntry(&searchPtr->search);
		if (hPtr == NULL) {
		    return TCL_OK;
		}
	    } else {
		searchPtr->nextEntry = NULL;
	    }
	    varPtr2 = (Var *) Tcl_GetHashValue(hPtr);
	    if (!(varPtr2->flags & VAR_UNDEFINED)) {
		break;
	    }
	}
	interp->result = Tcl_GetHashKey(varPtr->value.tablePtr, hPtr);
#else
	nv_scanfrom(searchPtr->nv, searchPtr->sub);
	if (searchPtr->sub && (! nv_nextsub(searchPtr->nv))) {
		interp->result = "";
	} else {
		interp->result = searchPtr->sub = nv_getsub(searchPtr->nv);
	}
	nv_putsub(searchPtr->nv, NULL, ARRAY_UNDEF);
#endif
    } else if ((c == 's') && (strncmp(argv[1], "set", length) == 0)
	    && (length >= 2)) {
	char **valueArgv;
	int valueArgc, i, result;

	if (argc != 4) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " set arrayName list\"", (char *) NULL);
	    return TCL_ERROR;
	}
	if (Tcl_SplitList(interp, argv[3], &valueArgc, &valueArgv) != TCL_OK) {
	    return TCL_ERROR;
	}
	result = TCL_OK;
	if (valueArgc & 1) {
	    interp->result = "list must have an even number of elements";
	    result = TCL_ERROR;
	    goto setDone;
	}
	for (i = 0; i < valueArgc; i += 2) {
	    if (Tcl_SetVar2(interp, argv[2], valueArgv[i], valueArgv[i+1],
		    TCL_LEAVE_ERR_MSG) == NULL) {
		result = TCL_ERROR;
		break;
	    }
	}
	setDone:
	ckfree((char *) valueArgv);
	return result;
    } else if ((c == 's') && (strncmp(argv[1], "size", length) == 0)
	    && (length >= 2)) {
	int size;

	if (argc != 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " size arrayName\"", (char *) NULL);
	    return TCL_ERROR;
	}
	if (!notArray) {
	    nv_putsub(varPtr, NULL, ARRAY_SCAN);
	    size = nv_getsub(varPtr) ? 1 : 0;
	    while (nv_nextsub(varPtr))
		size++;
	    nv_putsub(varPtr, NULL, ARRAY_UNDEF);
	    }
	else {
	    size=0;
	}
	
	sprintf(interp->result, "%d", size);
    } else if ((c == 's') && (strncmp(argv[1], "startsearch", length) == 0)
	    && (length >= 2)) {
	ArraySearch *searchPtr;

	if (argc != 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " startsearch arrayName\"", (char *) NULL);
	    return TCL_ERROR;
	}
	if (notArray) {
	    goto error;
	}
#if 0	/* Tcl version */
	searchPtr = (ArraySearch *) ckalloc(sizeof(ArraySearch));
	if (varPtr->searchPtr == NULL) {
	    searchPtr->id = 1;
	    Tcl_AppendResult(interp, "s-1-", argv[2], (char *) NULL);
	} else {
	    char string[20];

	    searchPtr->id = varPtr->searchPtr->id + 1;
	    sprintf(string, "%d", searchPtr->id);
	    Tcl_AppendResult(interp, "s-", string, "-", argv[2],
		    (char *) NULL);
	}
	searchPtr->varPtr = varPtr;
	searchPtr->nextEntry = Tcl_FirstHashEntry(varPtr->value.tablePtr,
		&searchPtr->search);
	searchPtr->nextPtr = varPtr->searchPtr;
	varPtr->searchPtr = searchPtr;
#else
	searchPtr = MakeSearchId(interp, varPtr, argv[2]);
	searchPtr->nv = varPtr;
	searchPtr->sub = NULL;
#endif
    } else {
	Tcl_AppendResult(interp, "bad option \"", argv[1],
		"\": should be anymore, donesearch, exists, ",
		"get, names, nextelement, ",
		"set, size, or startsearch", (char *) NULL);
	return TCL_ERROR;
    }
    return TCL_OK;

    error:
    Tcl_AppendResult(interp, "\"", argv[2], "\" isn't an array",
	    (char *) NULL);
    return TCL_ERROR;
}

static ArraySearch * ParseSearchId(interp, varPtr, varName, string)
	Tcl_Interp *interp;
	Var *varPtr;
	char *varName;
	char *string;
{
	TkshArrayInfo *arrayInfo;
	char *end;
	int id;

	if ((string[0] != 's') || (string[1] != '-'))
	{
	   syntax:
		Tcl_AppendResult(interp, "illegal search identifier \"", string,
			"\"", (char *) NULL);
		return NULL;
	}
	id = strtoul(string+2, &end, 10);
	if ((end == (string+2)) || (*end != '-'))
		goto syntax;
	if (strcmp(end+1, varName) != 0)
	{
	        Tcl_AppendResult(interp, "search identifier \"", string,
		    "\" isn't for variable \"", varName, "\"", (char *) NULL);
	        return NULL;
	}
	if ((arrayInfo = TkshArrayData(varPtr)))
	{
		ArraySearch *searchPtr = (ArraySearch *) arrayInfo->clientData;
		while (searchPtr != NULL)
		{
			if (searchPtr->id == id)
				return searchPtr;
			searchPtr = searchPtr->nextPtr;
		}
	}
	Tcl_AppendResult(interp, "couldn't find search \"", string, "\"",
		(char *) NULL);
	return (ArraySearch *) NULL;
}

static ArraySearch *MakeSearchId(interp, nv, varName)
	Tcl_Interp *interp;
	Namval_t *nv;
	char *varName;
{
	ArraySearch *as, *searchPtr;
	TkshArrayInfo *arrayInfo;
	char string[20];

	as = (ArraySearch *) malloc(sizeof(ArraySearch));
	as->id = 1;
	as->nextPtr = NULL;

	if ((arrayInfo = TkshArrayData(nv)))
	{
		if ((searchPtr = (ArraySearch *) arrayInfo->clientData))
		{
			as->id = searchPtr->id + 1;
			as->nextPtr = searchPtr;
		}
	}
	arrayInfo->clientData = as;
	sprintf(string, "%d", as->id);
	Tcl_ResetResult(interp);
	Tcl_AppendResult(interp,"s-",string,"-",varName,(char *) NULL);
	return as;
}

void TkshDeleteSearches(arrayInfo)
	TkshArrayInfo *arrayInfo;
{
	ArraySearch *searchPtr;
	searchPtr = (ArraySearch *) arrayInfo->clientData;
	for (; searchPtr; searchPtr=searchPtr->nextPtr)
		free(searchPtr);
	arrayInfo->clientData = NULL;
}
