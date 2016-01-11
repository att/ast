#pragma prototyped
/* Routines for commands:
 *
 *	Tcl_CreateCommand, Tcl_DeleteCommand
 *	Tcl_Eval, Tcl_GlobalEval, Tcl_EvalFile, Tcl_VarEval, Tcl_RecordAndEval
 *	Tcl_CommandComplete
 *
 */

#include "tkshlib.h"
#include <nval.h>

static int inEval;

static int tksh_command(int argc, char *argv[], Shbltin_t *context)
{
	int result, commandType, oldInterpType;
	TkshCommandData *commandData = (TkshCommandData *)context->ptr;
	Interp *interp = (Interp *) commandData->interp;

	interp->shbltin = context;
	Tcl_ResetResult(commandData->interp);

	commandType = commandData->commandType;
	commandData->commandType |= COMMAND_ACTIVE;
	oldInterpType = interp->interpType;
	/* oldListMode = TkshSetListMode(interp->interpType); */
	if ((commandType&INTERP_MASK) != INTERP_CURRENT)
		interp->interpType = commandType & INTERP_MASK;
	result = commandData->info.proc(commandData->info.clientData,
		commandData->interp, argc, argv);
	interp->interpType = oldInterpType;
	/* TkshSetListMode(oldListMode); */

	if (interp->interpType != INTERP_TCL)
	{
		if (result == TCL_ERROR)
		{
			fprintf(stderr, "%s\n", commandData->interp->result);
			sh.exitval = 1;
		}
		else
		{
#if 0
			if (commandData->interp->result &&
			   (*commandData->interp->result) &&
			   sfstdout->flags & SF_STRING)
#else
			if (commandData->interp->result &&
			   (*commandData->interp->result) &&
			   (sfset(sfstdout, 0, 0) & SF_STRING || (
			   (!sh_getscope(1,1)) && !inEval &&
				sh_isoption(SH_INTERACTIVE))))
#endif
			{
				sfprintf(sfstdout,"%s\n",
					commandData->interp->result);
				sfsync(sfstdout);
			}
		}
	}
	if (! (commandData->commandType & COMMAND_ACTIVE))
		free (commandData); /* Command was deleted; free commandData */
	else
		commandData->commandType = commandType;
	interp->cmdCount ++;
	return result;
}

Tcl_Command
Tcl_CreateCommand (Tcl_Interp *interp, char *cmdName, Tcl_CmdProc *proc,
	ClientData clientData, Tcl_CmdDeleteProc *deleteProc)
{
	TkshCommandData *commandData;
	Namval_t *nv;

	if (((Interp *)interp)->flags & DELETED)
		return (Tcl_Command) NULL;

	if ((commandData = (TkshCommandData *)malloc(sizeof(TkshCommandData))))
	{
		commandData->info.clientData = clientData;
		commandData->info.deleteData = clientData;
		commandData->info.deleteProc = deleteProc;
		commandData->info.proc = proc;
		commandData->interp = interp;
#ifndef NO_TCL_EVAL
		commandData->commandType = ((Interp *) interp)->interpType;
		if ((cmdName[0]=='.') && (cmdName[1]==0))
			cmdName = "tcl_dot"; /* General mapping coming soon */
		dprintf(("Tksh: Added builtin: %s (%s)\n", cmdName,
			Tksh_InterpString(commandData->commandType)));
#endif
		sh_addbuiltin(cmdName, tksh_command, (void *) commandData);
	}

	nv = nv_search(cmdName, sh_bltin_tree(), 0);
	nv->nvflag |= NV_NOFREE;
	return (Tcl_Command) nv;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_GetCommandName --
 *
 *	Given a token returned by Tcl_CreateCommand, this procedure
 *	returns the current name of the command (which may have changed
 *	due to renaming).
 *
 * Results:
 *	The return value is the name of the given command.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

char *
Tcl_GetCommandName(Tcl_Interp *interp, Tcl_Command command)
{
	Namval_t *nv = (Namval_t *) command;
	return nv_name(nv);
}

int Tcl_DeleteCommand (Tcl_Interp *interp, char *cmdName)
{
	Namval_t *namval;
	TkshCommandData *commandData;

	if ((namval = nv_open(cmdName, sh.fun_tree, NV_NOADD)))
	{
		if (namval->nvalue == (void *) tksh_command)
		{
			commandData = (TkshCommandData *) namval->nvfun;
			if (commandData && commandData->info.deleteProc)
			{
				commandData->info.deleteProc(
					commandData->info.deleteData);
				if (commandData->commandType & COMMAND_ACTIVE)
				   commandData->commandType &=(~COMMAND_ACTIVE);
				else
				   free(commandData);
			}
		}
		nv_close(namval);
		sh_addbuiltin(cmdName, NULL, NULL);	/* Removes */
		namval->nvalue = NULL;			/* XX ksh bug? */
		return 0;
	}
	return -1;
}


int Tcl_GetCommandInfo(Tcl_Interp *interp, char *cmdName,
	Tcl_CmdInfo *infoPtr)
{
	Namval_t *namval;
	TkshCommandData *commandData = NULL;

	if ((namval = nv_open(cmdName, sh.fun_tree, NV_NOADD)))
	{
		commandData = (TkshCommandData *) namval->nvfun;
		if ((void *) namval->nvalue == (void *) tksh_command)
			*infoPtr = commandData->info;
		else
		{
			infoPtr->clientData = (ClientData) commandData;
			infoPtr->proc = (Tcl_CmdProc *) namval->nvalue;
			infoPtr->deleteData = (ClientData) namval->nvalue;
			/* proc matching deleteData indicates ksh builtin */
		}
		nv_close(namval);
	}
	return (commandData ? 1 : 0);
}


int Tcl_SetCommandInfo(Tcl_Interp *interp, char *cmdName, Tcl_CmdInfo *infoPtr)
{
	Namval_t *namval;
	TkshCommandData *commandData;

	if ((namval = nv_open(cmdName, sh.fun_tree, NV_NOADD)))
	{
		commandData = (TkshCommandData *) namval->nvfun;
		commandData->info = *infoPtr;
		nv_close(namval);
		return 1;
	}
	return 0;
}

int Tksh_SetCommandType(Tcl_Interp *interp, char *cmdName, int tp)
{
	Namval_t *namval;
	TkshCommandData *commandData;

	if ((namval = nv_open(cmdName, sh.fun_tree, NV_NOADD)))
	{
		commandData = (TkshCommandData *) namval->nvfun;
		commandData->commandType &= (~INTERP_MASK);
		commandData->commandType |= tp;
		nv_close(namval);
		return 1;
	}
	return 0;
}

static char *fileToString(Sfio_t *f)
{
        /* return sfreserve(f,SF_UNBOUND,-1); */
	char *out, *buf;
	Sfio_t *strstm;
	strstm = sfnew(NULL,NULL,-1,-1,SF_STRING|SF_READ|SF_WRITE);
        sfmove(f,strstm,SF_UNBOUND,-1);
	sfputc(strstm,0);
        sfseek(strstm,0L,SEEK_SET);
        buf = sfreserve(strstm,SF_UNBOUND,-1);
	out = strdup(buf);
	sfclose(strstm);
	sfclrlock(f);
	return out;
}


int Tksh_Eval(Tcl_Interp *interp, char *command, int flag)
{
	Sfio_t *f;
	register Interp *iPtr = (Interp *) interp;
	int result, oldInterpType = iPtr->interpType;

	Tcl_FreeResult(interp);
	interp->result = iPtr->resultSpace;
	iPtr->resultSpace[0] = 0;

	iPtr->interpType = INTERP_KSH;
	dprintf2(("-- Tksh Eval --\n%s\n---------\n", flag?"--FILE--":command));

	if (flag)
	{
		char *cmd;
		f = sfopen(NIL(Sfio_t *),command, "r");
		cmd = fileToString(f);
		result = sh_trap(cmd, 0);
		free(cmd);
		sfclose(f);
	}
	else
	{
		result = sh_trap(command, 0);
	}

	iPtr->interpType = oldInterpType;
	result = Tksh_MapReturn(result);

	if (Tcl_AsyncReady())
		result = Tcl_AsyncInvoke(interp, result);

        if (iPtr->flags & DELETED)
		Tcl_DeleteInterp(interp);

	return result;
}

int Tcl_Eval(Tcl_Interp *interp, char *cmd)
{
	register Interp *iPtr = (Interp *) interp;
	int result;
	inEval ++;

	/* The following takes care of instances where we invoke the
	 * Tcl parser instead of ksh's.  As a bad hack, #!ksh or #!tcl
	 * can be specified in the script explicitly select a parser.
         */

	if (iPtr->interpType == INTERP_TCL)
	{
		if (strncmp(cmd, "#!ksh\n",6) != 0)
			result = Tcl_TclEval(interp, cmd);
		else
			result = Tksh_Eval(interp, cmd+6, 0);
	}
	else	/* oldInterp is INTERP_KSH */
	{
		if (strncmp(cmd, "#!tcl\n",6) == 0)
			result = Tcl_TclEval(interp, cmd+6);
		else
			result = Tksh_Eval(interp, cmd, 0);
	}
	inEval --;
	return result;
}

int Tcl_TclEvalFile(Tcl_Interp *interp, char *fileName)
{
	Interp *iPtr = (Interp *) interp;
	Sfio_t *script;
	char *cmdBuffer, *oldScriptFile;
	int result, oldInterpType, oldListMode;

	if (fileName)
	{
		oldScriptFile = iPtr->scriptFile;
		iPtr->scriptFile = fileName;
		script = sfopen(NIL(Sfio_t *), fileName, "r");
		if (!script)
			return TCL_ERROR;
	}
	else
	{
		fileName = "stdin";
		script = sfstdin;
		oldScriptFile = NIL(char*);
	}

	cmdBuffer = fileToString(script);
	oldInterpType = iPtr->interpType;
	oldListMode = TkshSetListMode(INTERP_TCL);
	iPtr->interpType = INTERP_TCL;
	result = Tcl_Eval(interp, cmdBuffer);
	free(cmdBuffer);
	iPtr->interpType = oldInterpType;
	TkshSetListMode(oldListMode);
	if (result == TCL_RETURN) {
		result = TclUpdateReturnInfo((Interp *) interp);
	} else if (result == TCL_ERROR) {
		char msg[200];

		/*
		 * Record information telling where the error occurred.
		 */
	
		sprintf(msg, "\n    (file \"%.150s\" line %d)", fileName,
			interp->errorLine);
		Tcl_AddErrorInfo(interp, msg);
	}
	if (oldScriptFile)
	{
		sfclose(script);
		iPtr->scriptFile = oldScriptFile;
	}
	return result;
}


int Tcl_EvalFile(Tcl_Interp *interp, char *fileName)
{
	register Interp *iPtr = (Interp *) interp;
	char *oldScriptFile;
	int result;

#ifndef NO_TCL_EVAL
	if (iPtr->interpType == INTERP_TCL)
		return Tcl_TclEvalFile(interp, fileName);
#endif

#if 0	/* Do I need this or does ksh do the substitution? */
	Tcl_DString buffer;
	fileName = Tcl_TildeSubst(interp, fileName, &buffer);
	if (fileName == NULL)
		return TCL_ERROR;
#endif
	oldScriptFile = iPtr->scriptFile;
	iPtr->scriptFile = fileName;
	dprintf(("EvalFile: %s\n", fileName));
	result = Tksh_Eval(interp, fileName, 1);
	iPtr->scriptFile = oldScriptFile;
	return result;
}

#ifndef NEWKSH
int Tcl_GlobalEval(Tcl_Interp *interp, char *cmd)
{
	int result, jmpval;
	Hashtab_t *oldscope = sh.var_tree;

	if (hashscope(sh.var_tree))
	{
		oldscope = sh.var_tree;
		sh.var_tree = hashscope(sh.var_tree);
	}

	result = Tcl_Eval(interp, cmd);

	sh.var_tree = oldscope;
	return result;
}
#else
int Tcl_GlobalEval(Tcl_Interp *interp, char *cmd)
{
	int result;
	static Shscope_t *globalframe;
	Shscope_t *oldframe;

	/* if (!globalframe) */ /* XXX */
		globalframe = sh_getscope(0, 0);
	oldframe = sh_setscope(globalframe);
	result = Tcl_Eval(interp, cmd);
	sh_setscope(oldframe);
	return result;
}

int Tksh_GlobalEval(Tcl_Interp *interp, char *cmd, int interpType)
{
	int result;
	static Shscope_t *globalframe;
	Shscope_t *oldframe;

	if (!globalframe)
		globalframe = sh_getscope(0, 0);
	oldframe = sh_setscope(globalframe);
	switch (interpType)
	{
		case INTERP_TCL: result = Tcl_TclEval(interp, cmd); break;
		case INTERP_KSH: result = Tksh_Eval(interp, cmd,0); break;
		default:	 result = Tcl_Eval(interp, cmd); break;
	}
        sh_setscope(oldframe);
        return result;
}
#endif

/* Tcl_VarEval in tclvareval.c, but a faster way would be to use
	sh_eval(sh_sfeval(args),0); */

int Tcl_RecordAndEval(Tcl_Interp *interp, char *script, int flags)
{
	int hist_state;
	int result = TCL_OK;

	if (flags == 0)
	{
		hist_state = sh_isoption(SH_HISTORY);
		sh_onoption(SH_HISTORY);
		result = Tcl_Eval(interp, script);
		if (! hist_state)
			sh_offoption(SH_HISTORY);
	}
	else /* TCL_NO_EVAL */
	{
		char *args[4];
		args[0] = "print";
		args[1] = "-s";
		args[2] = script;
		args[3] = 0;
		b_print(3, args, (Shbltin_t *) NULL);
	} 

	return result;
}

#ifdef NO_TCL_EVAL

/* This command just returns one because there is no point in using
 * Tcl_CommandComplete unless you allow Tcl evaluation
 */

int Tcl_CommandComplete(char *cmd)
{
	return 1;
}

#endif
