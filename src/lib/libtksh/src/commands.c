#include "tkshlib.h"
#include "tclcmd.h"

extern char * tclExecutableName;

int Tksh_LindexCmd(clientData, interp, argc, argv)
	ClientData clientData;
	Tcl_Interp *interp;
	int argc;
	char *argv[];
{
	int index, listArgc;
	char **listArgv;
	if (argc != 3) {
		interp->result = "wrong # args";
		return TCL_ERROR;
	}
	if (Tcl_GetInt(interp, argv[2], &index) != TCL_OK) {
		return TCL_ERROR;
	}
	if (Tcl_SplitList(interp, argv[1], &listArgc,
			&listArgv) != TCL_OK) {
		return TCL_ERROR;
	}
	if ((index >= 0) && (index < listArgc)) {
		Tcl_SetResult(interp, listArgv[index],
				TCL_VOLATILE);
	}
	free((char *) listArgv);
	return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_LlengthCmd --
 *
 *	This procedure is invoked to process the "llength" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tksh_LlengthCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    int listArgc;
    char **listArgv;


    if (argc != 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" list\"", (char *) NULL);
	return TCL_ERROR;
    }

    if (Tcl_SplitList(interp, argv[1], &listArgc, &listArgv) != TCL_OK) {
	return TCL_ERROR;
    }


    free((char *) listArgv);

    sprintf(interp->result, "%d", listArgc);
    return TCL_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * Tcl_RenameCmd -- MODIFIED
 *
 *	This procedure is invoked to process the "rename" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tksh_RenameCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
	Namval_t *namval;
	TkshCommandData *commandData;

	if (argc != 3)
	{
		Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		   " oldName newName\"", (char *) NULL);
		return TCL_ERROR;
	}
	if (argv[2][0] == '\0')
	{
		if (Tcl_DeleteCommand(interp, argv[1]) != 0) {
			Tcl_AppendResult(interp, "can't delete \"", argv[1],
			  "\": command doesn't exist", (char *) NULL);
			return TCL_ERROR;
		}
		return TCL_OK;
	}
	if ((namval=nv_open(argv[2], sh.fun_tree, NV_NOADD)) && namval->nvalue)
	{
		Tcl_AppendResult(interp, "can't rename to \"", argv[2],
			"\": command already exists", (char *) NULL);
		return TCL_ERROR;
	}
	if (! (namval=nv_open(argv[1],sh.fun_tree,NV_NOADD)) || !namval->nvalue)
	{
		Tcl_AppendResult(interp, "can't rename \"", argv[1],
			"\":  command doesn't exist", (char *) NULL);
		return TCL_ERROR;
	}
	commandData = (TkshCommandData *) namval->nvfun;
	sh_addbuiltin(argv[2],(Shbltin_f)namval->nvalue,(void *)commandData);
	nv_close(namval);
	sh_addbuiltin(argv[1], NULL, NULL);
	namval->nvalue = NULL;	/* Having to do this is a ksh bug (??) XX */
	return TCL_OK;
}

int b_TkshSetlistCmd(argc, argv, context)
	int argc;
	char *argv[];
	Shbltin_t *context;
{
	Tcl_Interp *interp = (Tcl_Interp *)context->ptr;
	char *kshList;

	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s list [array]\n", argv[0]);
		return 1;
	}

	if (! (kshList = Tksh_ConvertList(interp, argv[1], INTERP_KSH)))
		return 1;
	if (argc == 3)
		Tcl_VarEval(interp, "set -A ", argv[2], " ", kshList, NULL);
	else
		Tcl_VarEval(interp, "set -- ", kshList, NULL);

	free (kshList);
	return 0;
}

int b_TkshInfoCmd(argc, argv, context)
	int argc;
	char *argv[];
	Shbltin_t *context;
{
	char c;
	int length;
	Interp *iPtr = (Interp *)context->ptr;
	if (argc < 2)
	{
	     sfprintf(sfstderr,
	     "wrong # args: should be \"%s option ?arg arg ...?\"\n",argv[0]);
	     return 1;
	}
	c = argv[1][0];
	length = strlen(argv[1]);
	if ((c == 'm') && (strncmp(argv[1], "mode", length)) == 0)
	{
		if (argc == 3)
		{
			if (*argv[2] == 't')
				iPtr->interpType = INTERP_TCL;
			else
				iPtr->interpType = INTERP_KSH;
			return 0;
		}
		sfprintf(sfstdout,"%s\n",(iPtr->interpType==INTERP_KSH)?"ksh":
			((iPtr->interpType==INTERP_TCL)?"tcl":"either"));
	}
	else if ((c == 's') && (strncmp(argv[1], "sideeffects", length)) == 0)
	{
		if (argc == 3)
		{
			if (strcmp(argv[2], "on") == 0)
				sh_onoption(SH_SUBSHARE);
			else
				sh_offoption(SH_SUBSHARE);
			return 0;
		}
		sfprintf(sfstdout, "%s\n",sh_isoption(SH_SUBSHARE)?"on":"off");
	}
	else if ((c == 'l') && (strncmp(argv[1], "listmode", length)) == 0)
	{
		if (argc == 3)
		{
			if (strcmp(argv[2], "tcl") == 0)
				TkshSetListMode(INTERP_TCL);
			else
				TkshSetListMode(INTERP_KSH);
			return 0;
		}
		sfprintf(sfstdout,"%s\n",(TkshSetListMode(-1)==INTERP_KSH)?"ksh":
			((TkshSetListMode(-1)==INTERP_TCL)?"tcl":"either"));
	}
	else
	{
		sfprintf(sfstderr, "Bad option: %s\n", argv[1]);
		return -1;
	}
	return 0;
}

static Tcl_Interp *tksh_interp;
static char *tksh_matchstr;

static void tksh_appendBltn(nv, data)
	Namval_t *nv;
	void *data;
{
	char *name = nv_name(nv);
	if ((name[0] == 't') && (name[1] == 'c') &&
	    (name[2] == 'l') && (name[3] == '_'))
		name += 4;
	if (tksh_matchstr && !Tcl_StringMatch(name, tksh_matchstr))
		return;
	Tcl_AppendElement(tksh_interp, name);
}

static void tksh_appendLocal(nv, data)
	Namval_t *nv;
	void *data;
{
	if (nv_isattr(nv, NV_REF))
		return;
	if (tksh_matchstr && !Tcl_StringMatch(nv_name(nv), tksh_matchstr))
		return;
	Tcl_AppendElement(tksh_interp, nv_name(nv));
}

static void tksh_appendProc(nv, data)
	Namval_t *nv;
	void *data;
{
	TkshCommandData *commandData;

	if (tksh_matchstr && !Tcl_StringMatch(nv_name(nv), tksh_matchstr))
		return;

	commandData = (TkshCommandData *) nv->nvfun;

	if ( (! commandData) || (TclIsProc( & (commandData->info)) == NULL) )
		return;
	
	Tcl_AppendElement(tksh_interp, nv_name(nv));
}


/*
 *----------------------------------------------------------------------
 *
 * Tcl_InfoCmd --
 *
 *	This procedure is invoked to process the "info" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tksh_InfoCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    register Interp *iPtr = (Interp *) interp;
    size_t length;
    int c;
    Arg *argPtr;
    Proc *procPtr;
#ifdef TCL_CODE
    Var *varPtr;
    Command *cmdPtr;
    Tcl_HashEntry *hPtr;
    Tcl_HashSearch search;
#endif

	tksh_interp = interp;
    if (argc < 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" option ?arg arg ...?\"", (char *) NULL);
	return TCL_ERROR;
    }
    c = argv[1][0];
    length = strlen(argv[1]);
    if ((c == 'a') && (strncmp(argv[1], "args", length)) == 0) {
	if (argc != 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " args procname\"", (char *) NULL);
	    return TCL_ERROR;
	}
	procPtr = TclFindProc(iPtr, argv[2]);
	if (procPtr == NULL) {
	    infoNoSuchProc:
	    Tcl_AppendResult(interp, "\"", argv[2],
		    "\" isn't a procedure", (char *) NULL);
	    return TCL_ERROR;
	}
	for (argPtr = procPtr->argPtr; argPtr != NULL;
		argPtr = argPtr->nextPtr) {
	    Tcl_AppendElement(interp, argPtr->name);
	}
	return TCL_OK;
    } else if ((c == 'b') && (strncmp(argv[1], "body", length)) == 0) {
	if (argc != 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		    " body procname\"", (char *) NULL);
	    return TCL_ERROR;
	}
	procPtr = TclFindProc(iPtr, argv[2]);
	if (procPtr == NULL) {
	    goto infoNoSuchProc;
	}
	iPtr->result = procPtr->command;
	return TCL_OK;
    } else if ((c == 'c') && (strncmp(argv[1], "cmdcount", length) == 0)
	    && (length >= 2)) {
	if (argc != 2) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		    " cmdcount\"", (char *) NULL);
	    return TCL_ERROR;
	}
	sprintf(iPtr->result, "%d", iPtr->cmdCount);
	return TCL_OK;
    } else if ((c == 'c') && (strncmp(argv[1], "commands", length) == 0)
	    && (length >= 4)) {
	if (argc > 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		    " commands ?pattern?\"", (char *) NULL);
	    return TCL_ERROR;
	}
#ifdef TCL_CODE
	for (hPtr = Tcl_FirstHashEntry(&iPtr->commandTable, &search);
		hPtr != NULL; hPtr = Tcl_NextHashEntry(&search)) {
	    char *name = Tcl_GetHashKey(&iPtr->commandTable, hPtr);
	    if ((argc == 3) && !Tcl_StringMatch(name, argv[2])) {
		continue;
	    }
	    Tcl_AppendElement(interp, name);
	}
#else
	tksh_matchstr = (argc == 3) ? argv[2] : 0;
	nv_scan(sh_bltin_tree(), tksh_appendBltn, NIL(void *), 0, 0);
#endif
	return TCL_OK;
    } else if ((c == 'c') && (strncmp(argv[1], "complete", length) == 0)
	    && (length >= 4)) {
	if (argc != 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		    " complete command\"", (char *) NULL);
	    return TCL_ERROR;
	}
	if (Tcl_CommandComplete(argv[2])) {
	    interp->result = "1";
	} else {
	    interp->result = "0";
	}
	return TCL_OK;
    } else if ((c == 'd') && (strncmp(argv[1], "default", length)) == 0) {
	if (argc != 5) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " default procname arg varname\"",
		    (char *) NULL);
	    return TCL_ERROR;
	}
	procPtr = TclFindProc(iPtr, argv[2]);
	if (procPtr == NULL) {
	    goto infoNoSuchProc;
	}
	for (argPtr = procPtr->argPtr; ; argPtr = argPtr->nextPtr) {
	    if (argPtr == NULL) {
		Tcl_AppendResult(interp, "procedure \"", argv[2],
			"\" doesn't have an argument \"", argv[3],
			"\"", (char *) NULL);
		return TCL_ERROR;
	    }
	    if (strcmp(argv[3], argPtr->name) == 0) {
		if (argPtr->defValue != NULL) {
		    if (Tcl_SetVar((Tcl_Interp *) iPtr, argv[4],
			    argPtr->defValue, 0) == NULL) {
			defStoreError:
			Tcl_AppendResult(interp,
				"couldn't store default value in variable \"",
				argv[4], "\"", (char *) NULL);
			return TCL_ERROR;
		    }
		    iPtr->result = "1";
		} else {
		    if (Tcl_SetVar((Tcl_Interp *) iPtr, argv[4], "", 0)
			    == NULL) {
			goto defStoreError;
		    }
		    iPtr->result = "0";
		}
		return TCL_OK;
	    }
	}
    } else if ((c == 'e') && (strncmp(argv[1], "exists", length) == 0)) {
	char *p;
	if (argc != 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		    " exists varName\"", (char *) NULL);
	    return TCL_ERROR;
	}
	p = Tcl_GetVar((Tcl_Interp *) iPtr, argv[2], 0);

	/*
	 * The code below handles the special case where the name is for
	 * an array:  Tcl_GetVar will reject this since you can't read
	 * an array variable without an index.
	 */

	if (p == NULL) {
#ifdef TCL_CODE			/* Need to walk through array */
	    Tcl_HashEntry *hPtr;
	    Var *varPtr;
#else
		Namval_t *nv;
#endif
	    if (strchr(argv[2], '(') != NULL) {
		noVar:
		iPtr->result = "0";
		return TCL_OK;
	    }
#ifdef TCL_CODE
	    if (iPtr->varFramePtr == NULL) {
		hPtr = Tcl_FindHashEntry(&iPtr->globalTable, argv[2]);
	    } else {
		hPtr = Tcl_FindHashEntry(&iPtr->varFramePtr->varTable, argv[2]);
	    }
	    if (hPtr == NULL) {
		goto noVar;
	    }
	    varPtr = (Var *) Tcl_GetHashValue(hPtr);
	    if (varPtr->flags & VAR_UPVAR) {
		varPtr = varPtr->value.upvarPtr;
	    }
	    if (!(varPtr->flags & VAR_ARRAY)) {
		goto noVar;
	    }
#else
		if ((nv = nv_open(argv[2], NULL, NV_NOADD)) == NULL)
			goto noVar;
		if (! nv_isattr(nv, NV_ARRAY))
			goto noVar;
#endif
	}
	iPtr->result = "1";
	return TCL_OK;
    } else if ((c == 'g') && (strncmp(argv[1], "globals", length) == 0)) {
#ifdef TCL_CODE
	char *name;
#endif

	if (argc > 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		    " globals ?pattern?\"", (char *) NULL);
	    return TCL_ERROR;
	}
#ifdef TCL_CODE			/* Need to Search global list */
	for (hPtr = Tcl_FirstHashEntry(&iPtr->globalTable, &search);
		hPtr != NULL; hPtr = Tcl_NextHashEntry(&search)) {
	    varPtr = (Var *) Tcl_GetHashValue(hPtr);
	    if (varPtr->flags & VAR_UNDEFINED) {
		continue;
	    }
	    name = Tcl_GetHashKey(&iPtr->globalTable, hPtr);
	    if ((argc == 3) && !Tcl_StringMatch(name, argv[2])) {
		continue;
	    }
	    Tcl_AppendElement(interp, name);
	}
#else
	tksh_matchstr = (argc == 3) ? argv[2] : 0;
	nv_scan(hashscope(sh.var_tree)? hashscope(sh.var_tree): sh.var_tree,
		tksh_appendBltn, NIL(void *), 0, 0);
#endif
	return TCL_OK;
#if 0
    } else if ((c == 'h') && (strncmp(argv[1], "hostname", length) == 0)) {
        if (argc > 2) {
            Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                    " hostname\"", (char *) NULL);
            return TCL_ERROR;
        }
        Tcl_AppendResult(interp, Tcl_GetHostName(), NULL);
        return TCL_OK;
#endif
    } else if ((c == 'l') && (strncmp(argv[1], "level", length) == 0)
	    && (length >= 2)) {
#ifdef TCL_CODE
	if (argc == 2) {
	    if (iPtr->varFramePtr == NULL) {
		iPtr->result = "0";
	    } else {
		sprintf(iPtr->result, "%d", iPtr->varFramePtr->level);
	    }
	    return TCL_OK;
	} else if (argc == 3) {
	    int level;
	    CallFrame *framePtr;

	    if (Tcl_GetInt(interp, argv[2], &level) != TCL_OK) {
		return TCL_ERROR;
	    }
	    if (level <= 0) {
		if (iPtr->varFramePtr == NULL) {
		    levelError:
		    Tcl_AppendResult(interp, "bad level \"", argv[2],
			    "\"", (char *) NULL);
		    return TCL_ERROR;
		}
		level += iPtr->varFramePtr->level;
	    }
	    for (framePtr = iPtr->varFramePtr; framePtr != NULL;
		    framePtr = framePtr->callerVarPtr) {
		if (framePtr->level == level) {
		    break;
		}
	    }
	    if (framePtr == NULL) {
		goto levelError;
	    }
	    iPtr->result = Tcl_Merge(framePtr->argc, framePtr->argv);
	    iPtr->freeProc = (Tcl_FreeProc *) free;
	    return TCL_OK;
	}
#else
	if (argc == 2)
	{
		sprintf(iPtr->result, "%d", nv_getlevel());
		return TCL_OK;
	}
	else if (argc == 3)
	{
		CallFrame *framePtr;
		int level;

		if (Tcl_GetInt(interp, argv[2], &level) != TCL_OK)
			return TCL_ERROR;
		if (level <= 0)
		{
			if (! (framePtr = sh_getscope(0-level, 1)))
			{
			   levelError:
				Tcl_AppendResult(interp,"bad level \"", argv[2],
					"\"", (char *) NULL);
				return TCL_ERROR;
			}
		}
		else if (level > 0)
		{
			if (! (framePtr = sh_getscope(level, 0)))
				goto levelError;
		}

		if (! framePtr->par_scope)	/* Global scope doesn't count */
			goto levelError;

		iPtr->result = Tcl_Merge(framePtr->argc+1, framePtr->argv);
		iPtr->freeProc = (Tcl_FreeProc *) free;
		return TCL_OK;
	}
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" level [number]\"", (char *) NULL);
	return TCL_ERROR;
#endif
    } else if ((c == 'l') && (strncmp(argv[1], "library", length) == 0)
	    && (length >= 2)) {
	if (argc != 2) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		    " library\"", (char *) NULL);
	    return TCL_ERROR;
	}
#if TCL_MINOR_VERSION == 3
	interp->result = TkshLibDir();
#else
	interp->result = Tcl_GetVar(interp, "tcl_library", TCL_GLOBAL_ONLY);
	if (interp->result == NULL) {
	    interp->result = "no library has been specified for Tcl";
	    return TCL_ERROR;
	}
#endif
	return TCL_OK;
    } else if ((c == 'l') && (strncmp(argv[1], "loaded", length) == 0)
            && (length >= 3)) {
        if ((argc != 2) && (argc != 3)) {
            Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                    " loaded ?interp?\"", (char *) NULL);
            return TCL_ERROR;
        }
        return TclGetLoadedPackages(interp, argv[2]);
    } else if ((c == 'l') && (strncmp(argv[1], "locals", length) == 0)
	    && (length >= 3)) {
#ifdef TCL_CODE
	char *name;
#endif

	if (argc > 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		    " locals ?pattern?\"", (char *) NULL);
	    return TCL_ERROR;
	}
#ifdef TCL_CODE	/* Need to go through list of local variables */
	if (iPtr->varFramePtr == NULL) {
	    return TCL_OK;
	}
	for (hPtr = Tcl_FirstHashEntry(&iPtr->varFramePtr->varTable, &search);
		hPtr != NULL; hPtr = Tcl_NextHashEntry(&search)) {
	    varPtr = (Var *) Tcl_GetHashValue(hPtr);
	    if (varPtr->flags & (VAR_UNDEFINED|VAR_UPVAR)) {
		continue;
	    }
	    name = Tcl_GetHashKey(&iPtr->varFramePtr->varTable, hPtr);
	    if ((argc == 3) && !Tcl_StringMatch(name, argv[2])) {
		continue;
	    }
	    Tcl_AppendElement(interp, name);
	}
#else
	if (hashscope(sh.var_tree))
	{
		Hashtab_t *ht = hashscope(sh.var_tree);
		hashscope(sh.var_tree) = NULL;
		tksh_matchstr = (argc == 3) ? argv[2] : 0;
		nv_scan(sh.var_tree, tksh_appendLocal, NIL(void *), 0, 0);
		hashscope(sh.var_tree) = ht;
	}
#endif
	return TCL_OK;
    } else if ((c == 'n') && (strncmp(argv[1], "nameofexecutable",
            length) == 0)) {
        if (argc != 2) {
            Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                    " nameofexecutable\"", (char *) NULL);
            return TCL_ERROR;
        }
        if (tclExecutableName != NULL) {
            interp->result = tclExecutableName;
        }
        return TCL_OK;
    } else if ((c == 'p') && (strncmp(argv[1], "patchlevel", length) == 0)
	    && (length >= 2)) {
	char *value;

	if (argc != 2) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		    " patchlevel\"", (char *) NULL);
	    return TCL_ERROR;
	}
	value = Tcl_GetVar(interp, "tcl_patchLevel",
		TCL_GLOBAL_ONLY|TCL_LEAVE_ERR_MSG);
	if (value == NULL) {
	    return TCL_ERROR;
	}
	interp->result = value;
	return TCL_OK;
    } else if ((c == 'p') && (strncmp(argv[1], "procs", length) == 0)
	    && (length >= 2)) {
	if (argc > 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		    " procs ?pattern?\"", (char *) NULL);
	    return TCL_ERROR;
	}
#ifdef TCL_CODE	/* Need to go through builtin list */
	for (hPtr = Tcl_FirstHashEntry(&iPtr->commandTable, &search);
		hPtr != NULL; hPtr = Tcl_NextHashEntry(&search)) {
	    char *name = Tcl_GetHashKey(&iPtr->commandTable, hPtr);

	    cmdPtr = (Command *) Tcl_GetHashValue(hPtr);
	    if (!TclIsProc(cmdPtr)) {
		continue;
	    }
	    if ((argc == 3) && !Tcl_StringMatch(name, argv[2])) {
		continue;
	    }
	    Tcl_AppendElement(interp, name);
	}
#else
	tksh_matchstr = (argc == 3) ? argv[2] : 0;
	nv_scan(sh_bltin_tree(), tksh_appendProc, NIL(void *), 0, 0);
#endif
	return TCL_OK;
    } else if ((c == 's') && (strncmp(argv[1], "script", length) == 0)
            && (length >= 2)) {
	if (argc != 2) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " script\"", (char *) NULL);
	    return TCL_ERROR;
	}
	if (iPtr->scriptFile != NULL) {
	    /*
	     * Can't depend on iPtr->scriptFile to be non-volatile:
	     * if this command is returned as the result of the script,
	     * then iPtr->scriptFile will go away.
	     */

	    Tcl_SetResult(interp, iPtr->scriptFile, TCL_VOLATILE);
	}
	return TCL_OK;
    } else if ((c == 's') && (strncmp(argv[1], "sharedlibextension",
            length) == 0) && (length >= 2)) {
        if (argc != 2) {
            Tcl_AppendResult(interp, "wrong # args: should be \"",
                    argv[0], " sharedlibextension\"", (char *) NULL);
            return TCL_ERROR;
        }
#ifdef TCL_SHLIB_EXT
        interp->result = TCL_SHLIB_EXT;
#endif
        return TCL_OK;
    } else if ((c == 't') && (strncmp(argv[1], "tclversion", length) == 0)) {
	char *value;

	if (argc != 2) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " tclversion\"", (char *) NULL);
	    return TCL_ERROR;
	}
	value = Tcl_GetVar(interp, "tcl_version",
		TCL_GLOBAL_ONLY|TCL_LEAVE_ERR_MSG);
	if (value == NULL) {
	    return TCL_ERROR;
	}
	interp->result = value;
	return TCL_OK;
    } else if ((c == 'v') && (strncmp(argv[1], "vars", length)) == 0) {
#ifdef TCL_CODE
	Tcl_HashTable *tablePtr;
	char *name;
#endif

	if (argc > 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " vars ?pattern?\"", (char *) NULL);
	    return TCL_ERROR;
	}
#ifdef TCL_CODE		/* Need to go through variable tables */
	if (iPtr->varFramePtr == NULL) {
	    tablePtr = &iPtr->globalTable;
	} else {
	    tablePtr = &iPtr->varFramePtr->varTable;
	}
	for (hPtr = Tcl_FirstHashEntry(tablePtr, &search);
		hPtr != NULL; hPtr = Tcl_NextHashEntry(&search)) {
	    varPtr = (Var *) Tcl_GetHashValue(hPtr);
	    if (varPtr->flags & VAR_UNDEFINED) {
		continue;
	    }
	    name = Tcl_GetHashKey(tablePtr, hPtr);
	    if ((argc == 3) && !Tcl_StringMatch(name, argv[2])) {
		continue;
	    }
	    Tcl_AppendElement(interp, name);
	}
#else
	{
		Hashtab_t *ht = hashscope(sh.var_tree);
		tksh_matchstr = (argc == 3) ? argv[2] : 0;
		hashscope(sh.var_tree) = NULL;
		nv_scan(sh.var_tree, tksh_appendBltn, NIL(void *), 0, 0);
		hashscope(sh.var_tree) = ht;
	}
#endif
	return TCL_OK;
    } else {
	Tcl_AppendResult(interp, "bad option \"", argv[1],
		"\": should be args, body, cmdcount, commands, ",
		"complete, default, ",
#if 0
		"exists, globals, level, library, locals, ",
		"patchlevel, procs, script, tclversion, or vars",
#else
                "exists, globals, hostname, level, library, loaded, locals, ",
                "nameofexecutable, patchlevel, procs, script, ",
                "sharedlibextension, tclversion, or vars",
#endif
		(char *) NULL);
	return TCL_ERROR;
    }
}


int
Tksh_SourceCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
	return Tcl_TclEvalFile(interp, (argc >= 2) ? argv[1] : NULL);
}
