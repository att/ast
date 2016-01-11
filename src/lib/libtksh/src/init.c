#pragma prototyped
#include "tkshlib.h"
#include "tclcmd.h"

extern int b_TkshSetlistCmd(int argc, char *argv[], Shbltin_t *context);
extern int b_TkshInfoCmd(int argc, char *argv[], Shbltin_t *context);

typedef struct {
    char *name;                 /* Name of command. */
    Tcl_CmdProc *proc;          /* Procedure that executes command. */
} CmdInfo;

typedef struct {
    char *fromName;		/* Name of command in Tcl script */
				/* (if null, use toName + 4 */
    char *toName;          	/* Name to be stored as in ksh interp */
} CmdMap;

/* Names that cannot be used as builtins because they are keywords
 * or builtins in ksh.  They will be mapped to tcl_name.  We only
 * need to put in names that are not listed above. NOTE: If there
 * ends up being less than (say) 5 of these, change code to not use
 * hash tables.
 */

static CmdMap commandMap[] = {
	{ NULL, "tcl_do" },
	{ NULL, "tcl_readonly" },
	{ NULL, "tcl_history" },
	{ ".", "tcl_dot" },
	{ NULL, NULL}
};

static Tcl_HashTable commandMapTable;
static int initFirstInterp;

#ifndef TKSH_LIBRARY
# ifdef TCL_LIBRARY
# define TKSH_LIBRARY	TCL_LIBRARY
# else
# define TKSH_LIBRARY	"/usr/local/lib/tksh"
# endif
#endif

/*
 * Directory in which to look for packages (each package is typically
 * installed as a subdirectory of this directory).  The symbol is
 * defined by Makefile.
 */
     
static char pkgPath[PATH_MAX];

void TclSetupEnv(Tcl_Interp *interp)
{
	interp = interp;
}

void TclPlatformInit(Tcl_Interp *interp)
{
	char*	p;
	char*	s;

	p = strncpy(pkgPath, TkshLibDir(), sizeof(pkgPath) - 1);
	if (s = strrchr(p, '/'))
		*s = 0;
	Tcl_SetVar(interp, "tcl_pkgPath", pkgPath, TCL_GLOBAL_ONLY);
	Tcl_SetVar2(interp,"tcl_platform", "platform", "unix", TCL_GLOBAL_ONLY);
	Tcl_SetVar2(interp,"tcl_platform", "os", "", TCL_GLOBAL_ONLY);
	Tcl_SetVar2(interp,"tcl_platform", "osVersion", "", TCL_GLOBAL_ONLY);
	Tcl_SetVar2(interp,"tcl_platform", "machine", "", TCL_GLOBAL_ONLY);
}

void TkshCreateInterp(Tcl_Interp *interp, void *data)
{
	register CmdInfo *cmdInfoPtr;
	CmdInfo *builtInCmds = (CmdInfo *) data;
	char *name, *libDir, buf[100];
	CmdMap *map;
	int i;

	buf[0]='t'; buf[1]='c'; buf[2]='l'; buf[3]='_'; buf[4]=0;
	((Interp *) interp)->interpType = INTERP_TCL;

	if (initFirstInterp)
		return;
	initFirstInterp = 1;
	dprintfInterp(interp);

	for (cmdInfoPtr = builtInCmds; cmdInfoPtr->name != NULL; cmdInfoPtr++)
	{
		name = cmdInfoPtr->name;
		if (	(strcmp(name, "source") != 0) &&
			(strcmp(name, "after") != 0) )
		{
			strcpy(buf+4, name);
			name = buf;
		}
		Tcl_CreateCommand(interp, name,
			cmdInfoPtr->proc, (ClientData) NULL, NULL);
	}

	Tcl_InitHashTable(&commandMapTable, TCL_STRING_KEYS);
	for (map=commandMap; map->toName != NULL; map++)
		Tcl_SetHashValue(Tcl_CreateHashEntry(&commandMapTable,
			(map->fromName ? map->fromName : (map->toName+4)), &i),
			(ClientData) map->toName);
	libDir = TkshLibDir();
	Tcl_SetVar(interp, "tcl_library", libDir, TCL_GLOBAL_ONLY);
	Tcl_SetVar(interp, "tcl_patchLevel", TCL_PATCH_LEVEL, TCL_GLOBAL_ONLY);
	Tcl_SetVar(interp, "tcl_version", TCL_VERSION, TCL_GLOBAL_ONLY);
	Tcl_SetVar(interp, "tksh_version", TCL_VERSION, TCL_GLOBAL_ONLY);
	Tcl_TraceVar2(interp, "tcl_precision", (char *) NULL,
		TCL_GLOBAL_ONLY|TCL_TRACE_WRITES|TCL_TRACE_UNSETS,
		TclPrecTraceProc, (ClientData) NULL);
}

char *TkshMapKeyword(char *name)
{
	Tcl_HashEntry *h;
	return (h = Tcl_FindHashEntry(&commandMapTable, name)) ?
		((char *) Tcl_GetHashValue(h)) : name;
}

#ifdef DEBUG

int TestCommand(ClientData data, Tcl_Interp *interp, int artc, char *argv[])
{
	fprintf(stderr, "Test\n");
	return 0;
}
#endif

static char initScript[] =
      "if [[ -f $tcl_library/init.ksh ]]\n then\n\
	       . $tcl_library/init.ksh\n\
       else \n\
       msg=\"can't find $tcl_library/init.ksh; perhaps you\"\n\
       msg=\"$msg need to\ninstall Tksh or set your LIB_DIR\"\n\
       msg=\"$msg environment variable?\"\n\
       tcl_error \"$msg\" 2> /dev/null\n\
       unset tksh_version\n\
       fi\n";

/*
 *----------------------------------------------------------------------
 *
 * Tcl_Init --
 *
 *	This procedure is typically invoked by Tcl_AppInit procedures
 *	to perform additional initialization for a Tcl interpreter,
 *	such as sourcing the "init.tcl" script.
 *
 * Results:
 *	Returns a standard Tcl completion code and sets interp->result
 *	if there is an error.
 *
 * Side effects:
 *	Depends on what's in the init.tcl script.
 *
 *----------------------------------------------------------------------
 */

int Tcl_Init(Tcl_Interp *interp)
{
	Interp *iPtr = (Interp *) interp;
	dprintf(("Using TKSH, compiled %s %s\n", __DATE__, __TIME__));
#ifdef DEBUG
	Tcl_CreateCommand(interp, "testit", TestCommand, NULL, NULL);
#endif
	sh_addbuiltin("tksh_info", b_TkshInfoCmd, (void *) interp);
	sh_addbuiltin("setlist", b_TkshSetlistCmd, (void *) interp);
	Tksh_SetCommandType(interp, "tcl_eval", INTERP_TCL);
	Tksh_SetCommandType(interp, "tcl_vwait", INTERP_TCL);
	iPtr->interpType=INTERP_CURRENT;

	Tksh_Eval(interp, initScript, 0);
	Tcl_ResetResult(interp);	/* Shouln't have to do this... */
	if (Tcl_GetVar2(interp, "tksh_version", NULL, TCL_GLOBAL_ONLY))
		return TCL_OK;
	else
		return TCL_ERROR;
}


/*
 *----------------------------------------------------------------------
 *
 * TkshLibDir --
 *
 *	This function tries to find location of the Tksh library files,
 *	and returns the name of a directory.
 *
 *----------------------------------------------------------------------
 */

char *TkshLibDir(void)
{
	char		buf[PATH_MAX];

	static char*	libDir;

	if (!libDir && !(libDir = getenv(LIB_DIR_ENV)) || !*libDir)
	{
		if (!(libDir = pathpath(LIB_DIR, "", PATH_EXECUTE|PATH_READ, buf, sizeof(buf))))
			sfsprintf(buf, sizeof(buf), "/usr/local/%s", LIB_DIR);
		if (!(libDir = strdup(buf)))
			libDir = TKSH_LIBRARY;
	}
	return libDir;
}


/*
 *----------------------------------------------------------------------
 *
 * TkshSubshell	- Notify tksh that a subshell was invoked
 *
 *----------------------------------------------------------------------
 */

void TkshSubShell(void)
{
	sh_waitnotify((ShellNote_t) 0);
	TkshTracesOff();
}


/* This function is used in Tk to get around a bug in Solaris strtod */
double fixstrtod(char *string, char **endPtr)
{
	double d = strtod(string, endPtr);
	if ((endPtr != NULL) && (*endPtr != string) && ((*endPtr)[-1] == 0)) {
		*endPtr -= 1;
	}
	return d;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_SourceRCFile --
 *
 *	This procedure is typically invoked by Tcl_Main of Tk_Main
 *	procedure to source an application specific rc file into the
 *	interpreter at startup time.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Depends on what's in the rc script.
 *
 *----------------------------------------------------------------------
 */

void
Tcl_SourceRCFile(interp)
    Tcl_Interp *interp;		/* Interpreter to source rc file into. */
{
    Tcl_DString temp;
    char *fileName;
    Tcl_Channel errChannel;

    fileName = Tcl_GetVar(interp, "tcl_rcFileName", TCL_GLOBAL_ONLY);

    if (fileName != NULL) {
        Tcl_Channel c;
	char *fullName;

        Tcl_DStringInit(&temp);
	fullName = Tcl_TranslateFileName(interp, fileName, &temp);
	if (fullName == NULL) {
	    errChannel = Tcl_GetStdChannel(TCL_STDERR);
	    if (errChannel) {
		Tcl_Write(errChannel, interp->result, -1);
		Tcl_Write(errChannel, "\n", 1);
	    }
	} else {

	    /*
	     * Test for the existence of the rc file before trying to read it.
	     */

            c = Tcl_OpenFileChannel(NULL, fullName, "r", 0);
            if (c != (Tcl_Channel) NULL) {
                Tcl_Close(NULL, c);
		if (Tcl_EvalFile(interp, fullName) != TCL_OK) {
		    errChannel = Tcl_GetStdChannel(TCL_STDERR);
		    if (errChannel) {
			Tcl_Write(errChannel, interp->result, -1);
			Tcl_Write(errChannel, "\n", 1);
		    }
		}
	    }
	}
        Tcl_DStringFree(&temp);
    }
}

/*
 * this forces a few tcl routines to be linked before -ltk has a chance
 * to say it needs them
 *
 * NOTE: don't call this
 */

void _tcl_force_link(void)
{
	Tcl_LinkVar(0, 0, 0, 0);
	Tcl_UnlinkVar(0, 0);
}
