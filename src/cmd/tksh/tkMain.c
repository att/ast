#pragma prototyped
/* 
 * tkMain.c --
 *
 *	This file contains a generic main program for Tk-based applications.
 *	It can be used as-is for many applications, just by supplying a
 *	different appInitProc procedure for each specific application.
 *	Or, it can be used as a template for creating new main programs
 *	for Tk applications.
 *
 * Copyright (c) 1990-1994 The Regents of the University of California.
 * Copyright (c) 1994-1996 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * SCCS: @(#) tkMain.c 1.148 96/03/25 18:08:43
 */

#include "tksh.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <tcl.h>
#include <tk.h>
#if 0
#ifdef NO_STDLIB_H
#   include "../compat/stdlib.h"
#else
#   include <stdlib.h>
#endif
#endif

/*
 * Declarations for various library procedures and variables (don't want
 * to include tkInt.h or tkPort.h here, because people might copy this
 * file out of the Tk source directory to make their own modified versions).
 * Note: don't declare "exit" here even though a declaration is really
 * needed, because it will conflict with a declaration elsewhere on
 * some systems.
 */

extern int		isatty _ANSI_ARGS_((int fd));
#if 0
extern int		read _ANSI_ARGS_((int fd, char *buf, size_t size));
extern char *		strrchr _ANSI_ARGS_((CONST char *string, int c));
#endif

/*
 * Global variables used by the main program:
 */

static Tcl_Interp *interp;	/* Interpreter for this application. */
static Tcl_DString command;	/* Used to assemble lines of terminal input
				 * into Tcl commands. */
static Tcl_DString line;	/* Used to read the next line from the
                                 * terminal input. */
static int tty;			/* Non-zero means standard input is a
				 * terminal-like device.  Zero means it's
				 * a file. */

/*
 * Forward declarations for procedures defined later in this file.
 */

static void		Prompt _ANSI_ARGS_((Tcl_Interp *interp, int partial));
static void		StdinProc _ANSI_ARGS_((ClientData clientData,
			    int mask));

/*
 *----------------------------------------------------------------------
 *
 * Tk_Main --
 *
 *	Main program for Wish and most other Tk-based applications.
 *
 * Results:
 *	None. This procedure never returns (it exits the process when
 *	it's done.
 *
 * Side effects:
 *	This procedure initializes the Tk world and then starts
 *	interpreting commands;  almost anything could happen, depending
 *	on the script being interpreted.
 *
 *----------------------------------------------------------------------
 */

void
Tksh_TkMain(argc, argv, appInitProc)
    int argc;				/* Number of arguments. */
    char **argv;			/* Array of argument strings. */
    Tcl_AppInitProc *appInitProc;	/* Application-specific initialization
					 * procedure to call after most
					 * initialization but before starting
					 * to execute commands. */
{
    char *args, *fileName;
    char buf[20];
    int code;
    size_t length;
#if 0
    Tcl_Channel inChannel, outChannel;
#endif
    Tcl_Channel errChannel, chan;

    Tcl_FindExecutable(argv[0]);
    interp = Tcl_CreateInterp();
#ifdef TCL_MEM_DEBUG
    Tcl_InitMemory(interp);
#endif

    /*
     * Parse command-line arguments.  A leading "-file" argument is
     * ignored (a historical relic from the distant past).  If the
     * next argument doesn't start with a "-" then strip it off and
     * use it as the name of a script file to process.
     */

    fileName = NULL;
    if (argc > 1) {
	length = strlen(argv[1]);
	if ((length >= 2) && (strncmp(argv[1], "-file", length) == 0)) {
	    argc--;
	    argv++;
	}
    }
    if ((argc > 1) && (argv[1][0] != '-')) {
	fileName = argv[1];
	argc--;
	argv++;
    }

    /*
     * Make command-line arguments available in the Tcl variables "argc"
     * and "argv".
     */

    args = Tcl_Merge(argc-1, argv+1);
    Tcl_SetVar(interp, "argv", args, TCL_GLOBAL_ONLY);
    ckfree(args);
    sprintf(buf, "%d", argc-1);
    Tcl_SetVar(interp, "argc", buf, TCL_GLOBAL_ONLY);
    Tcl_SetVar(interp, "argv0", (fileName != NULL) ? fileName : argv[0],
	    TCL_GLOBAL_ONLY);

    /*
     * Set the "tcl_interactive" variable.
     */

    /*
     * For now, under Windows, we assume we are not running as a console mode
     * app, so we need to use the GUI console.  In order to enable this, we
     * always claim to be running on a tty.  This probably isn't the right
     * way to do it.
     */

#ifdef __WIN32__
    tty = 1;
#else
    tty = isatty(0);
#endif
    Tcl_SetVar(interp, "tcl_interactive",
	    ((fileName == NULL) && tty) ? "1" : "0", TCL_GLOBAL_ONLY);

    /*
     * Invoke application-specific initialization.
     */

    if ((*appInitProc)(interp) != TCL_OK) {
	errChannel = Tcl_GetStdChannel(TCL_STDERR);
	if (errChannel) {
            Tcl_Write(errChannel,
		    "application-specific initialization failed: ", -1);
            Tcl_Write(errChannel, interp->result, -1);
            Tcl_Write(errChannel, "\n", 1);
        }
    	Tcl_DeleteInterp(interp);
	Tcl_Exit(1);		/* added so tksh will exit here */
    }

    /*
     * Invoke the script specified on the command line, if any.
     */

    if (fileName != NULL) {
	code = Tcl_EvalFile(interp, fileName);
	if (code != TCL_OK) {
	    goto error;
	}
	tty = 0;
    } else {

	/*
	 * Commands will come from standard input, so set up an event
	 * handler for standard input.  Evaluate the .rc file, if one
	 * has been specified, set up an event handler for standard
	 * input, and print a prompt if the input device is a terminal.
	 */

	fileName = Tcl_GetVar(interp, "tcl_rcFileName", TCL_GLOBAL_ONLY);

	if (fileName != NULL) {
	    Tcl_DString buffer;
	    char *fullName;
    
	    fullName = Tcl_TranslateFileName(interp, fileName, &buffer);
	    if (fullName == NULL) {
		errChannel = Tcl_GetStdChannel(TCL_STDERR);
		if (errChannel) {
                    Tcl_Write(errChannel, interp->result, -1);
                    Tcl_Write(errChannel, "\n", 1);
                }
	    } else {

                /*
                 * NOTE: The following relies on O_RDONLY==0.
                 */
                
                chan = Tcl_OpenFileChannel(interp, fullName, "r", 0);
                if (chan != (Tcl_Channel) NULL) {
                    Tcl_Close(NULL, chan);
                    if (Tcl_EvalFile(interp, fullName) != TCL_OK) {
			errChannel = Tcl_GetStdChannel(TCL_STDERR);
			if (errChannel) {
                            Tcl_Write(errChannel, interp->result, -1);
                            Tcl_Write(errChannel, "\n", 1);
                        }
                    }
                }
            }
            
	    Tcl_DStringFree(&buffer);
	}

#if 0
	/*
	 * Establish a channel handler for stdin.
	 */

	inChannel = Tcl_GetStdChannel(TCL_STDIN);
	if (inChannel) {
	    Tcl_CreateChannelHandler(inChannel, TCL_READABLE, StdinProc,
		    (ClientData) inChannel);
	}
	if (tty) {
	    Prompt(interp, 0);
	}
#endif
    }

#if 0
    outChannel = Tcl_GetStdChannel(TCL_STDOUT);
    if (outChannel) {
	Tcl_Flush(outChannel);
    }
    Tcl_DStringInit(&command);
    Tcl_DStringInit(&line);
#endif
    Tcl_ResetResult(interp);

    /*
     * Loop infinitely, waiting for commands to execute.  When there
     * are no windows left, Tk_MainLoop returns and we exit.
     */

#if 0
    Tk_MainLoop();
    Tcl_DeleteInterp(interp);
    Tcl_Exit(0);
#else
    return;
#endif

error:
    /*
     * The following statement guarantees that the errorInfo
     * variable is set properly.
     */

    Tcl_AddErrorInfo(interp, "");
    errChannel = Tcl_GetStdChannel(TCL_STDERR);
    if (errChannel) {
        Tcl_Write(errChannel, Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY),
		-1);
        Tcl_Write(errChannel, "\n", 1);
    }
    Tcl_DeleteInterp(interp);
    Tcl_Exit(1);
}

#if 0
/*
 *----------------------------------------------------------------------
 *
 * StdinProc --
 *
 *	This procedure is invoked by the event dispatcher whenever
 *	standard input becomes readable.  It grabs the next line of
 *	input characters, adds them to a command being assembled, and
 *	executes the command if it's complete.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Could be almost arbitrary, depending on the command that's
 *	typed.
 *
 *----------------------------------------------------------------------
 */

    /* ARGSUSED */
static void
StdinProc(clientData, mask)
    ClientData clientData;		/* Not used. */
    int mask;				/* Not used. */
{
    static int gotPartial = 0;
    char *cmd;
    int code, count;
    Tcl_Channel chan = (Tcl_Channel) clientData;

    count = Tcl_Gets(chan, &line);

    if (count < 0) {
	if (!gotPartial) {
	    if (tty) {
		Tcl_Exit(0);
	    } else {
		Tcl_DeleteChannelHandler(chan, StdinProc, (ClientData) chan);
	    }
	    return;
	} else {
	    count = 0;
	}
    }

    (void) Tcl_DStringAppend(&command, Tcl_DStringValue(&line), -1);
    cmd = Tcl_DStringAppend(&command, "\n", -1);
    Tcl_DStringFree(&line);
    
    if (!Tcl_CommandComplete(cmd)) {
        gotPartial = 1;
        goto prompt;
    }
    gotPartial = 0;

    /*
     * Disable the stdin channel handler while evaluating the command;
     * otherwise if the command re-enters the event loop we might
     * process commands from stdin before the current command is
     * finished.  Among other things, this will trash the text of the
     * command being evaluated.
     */

    Tcl_CreateChannelHandler(chan, 0, StdinProc, (ClientData) chan);
    code = Tcl_RecordAndEval(interp, cmd, TCL_EVAL_GLOBAL);
    Tcl_CreateChannelHandler(chan, TCL_READABLE, StdinProc,
	    (ClientData) chan);
    Tcl_DStringFree(&command);
    if (*interp->result != 0) {
	if ((code != TCL_OK) || (tty)) {
	    /*
	     * The statement below used to call "printf", but that resulted
	     * in core dumps under Solaris 2.3 if the result was very long.
             *
             * NOTE: This probably will not work under Windows either.
	     */

	    puts(interp->result);
	}
    }

    /*
     * Output a prompt.
     */

    prompt:
    if (tty) {
	Prompt(interp, gotPartial);
    }
    Tcl_ResetResult(interp);
}

/*
 *----------------------------------------------------------------------
 *
 * Prompt --
 *
 *	Issue a prompt on standard output, or invoke a script
 *	to issue the prompt.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	A prompt gets output, and a Tcl script may be evaluated
 *	in interp.
 *
 *----------------------------------------------------------------------
 */

static void
Prompt(interp, partial)
    Tcl_Interp *interp;			/* Interpreter to use for prompting. */
    int partial;			/* Non-zero means there already
					 * exists a partial command, so use
					 * the secondary prompt. */
{
    char *promptCmd;
    int code;
    Tcl_Channel outChannel, errChannel;

    errChannel = Tcl_GetChannel(interp, "stderr", NULL);

    promptCmd = Tcl_GetVar(interp,
	partial ? "tcl_prompt2" : "tcl_prompt1", TCL_GLOBAL_ONLY);
    if (promptCmd == NULL) {
defaultPrompt:
	if (!partial) {

            /*
             * We must check that outChannel is a real channel - it
             * is possible that someone has transferred stdout out of
             * this interpreter with "interp transfer".
             */

	    outChannel = Tcl_GetChannel(interp, "stdout", NULL);
            if (outChannel != (Tcl_Channel) NULL) {
                Tcl_Write(outChannel, "% ", 2);
            }
	}
    } else {
	code = Tcl_Eval(interp, promptCmd);
	if (code != TCL_OK) {
	    Tcl_AddErrorInfo(interp,
		    "\n    (script that generates prompt)");
            /*
             * We must check that errChannel is a real channel - it
             * is possible that someone has transferred stderr out of
             * this interpreter with "interp transfer".
             */
            
	    errChannel = Tcl_GetChannel(interp, "stderr", NULL);
            if (errChannel != (Tcl_Channel) NULL) {
                Tcl_Write(errChannel, interp->result, -1);
                Tcl_Write(errChannel, "\n", 1);
            }
	    goto defaultPrompt;
	}
    }
    outChannel = Tcl_GetChannel(interp, "stdout", NULL);
    if (outChannel != (Tcl_Channel) NULL) {
        Tcl_Flush(outChannel);
    }
}
#endif


/*********************************************************************/

static int Tksh_BindCmd(clientData, interp, argc, argv)
    ClientData clientData;      /* Main window associated with
                                 * interpreter. */
    Tcl_Interp *interp;         /* Current interpreter. */
    int argc;                   /* Number of arguments. */
    char **argv;                /* Argument strings. */
{
	char *bindscript, *script = NULL, *oldarg;
	int result;

	if ((argc == 4) && (argv[3][0] != '+'))
	{
		static char *bindprefixksh = "#!ksh\n";
		static char *bindprefixtcl = "#!tcl\n";
#		define BINDPRELEN 6

		bindscript = argv[3];
		if ((bindscript[0] == '#') && (bindscript[1] == '!' ))
		{
			if ((strcmp(bindscript, bindprefixksh) == 0) ||
			    (strcmp(bindscript, bindprefixtcl) == 0))
				return Tk_BindCmd(clientData,interp,argc,argv);
		}
		script = (char *) malloc(strlen(bindscript) + BINDPRELEN +1);
		strcpy(script, (((Interp *) interp)->interpType == INTERP_TCL)?
				bindprefixtcl : bindprefixksh);
		strcpy(script + BINDPRELEN, bindscript);
		oldarg = argv[3];
		argv[3] = script;
		result = Tk_BindCmd(clientData, interp, argc, argv);
		argv[3] = oldarg;
		free(script);
		return result;
	}
	return Tk_BindCmd(clientData, interp, argc, argv);
}
static void bindsetup(Tcl_Interp *interp)
{
	Tcl_CmdInfo bindInfo;
	if (Tcl_GetCommandInfo(interp, "bind", & bindInfo))
	{
		bindInfo.proc = Tksh_BindCmd;
		/* Tcl_SetCommandInfo(interp, "bind", &bindInfo); */
		Tcl_CreateCommand(interp, "bind", bindInfo.proc,
			bindInfo.clientData, bindInfo.deleteProc);
		Tksh_SetCommandType(interp, "bind", INTERP_CURRENT);
	}
}
static int b_tkloop(int argc, char **argv, Shbltin_t *context)
{
	Tcl_Interp *interp = (Tcl_Interp *)context->ptr;
	Tksh_BeginBlock(interp, INTERP_TCL);
	Tk_MainLoop();
	Tksh_EndBlock(interp);
	return 0;
}
int Tksh_Init(interp)
    Tcl_Interp *interp;         /* Interpreter to initialize. */
{
#if 0
    static char initCmd[] =
        "if [[ -f $tk_library/tk.ksh ]] ; then \n\
                .  $tk_library/tk.ksh\n\
        else \n\
            msg=\"can't find $tk_library/tk.ksh; perhaps you \"\n\
            msg=\"$msg need to\\ninstall Tk or set your TK_LIBRARY \"\n\
            msg=\"$msg environment variable?\"\n\
            print -u2 $msg\n\
        fi\n";
#endif
    bindsetup(interp);
    sh_addbuiltin("tkloop", b_tkloop, (void *) interp);
    return TCL_OK;
}

static int
Tksh_AppInit(interp)
    Tcl_Interp *interp;		/* Interpreter for application. */
{
    if (Tcl_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
    Tksh_BeginBlock(interp, INTERP_TCL);
		/* Should be current, but Tk_Init evals a script. */
    if (Tk_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
    if (Tksh_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
    Tksh_SetCommandType(interp, "button", INTERP_CURRENT);  /* Why do this? */
    Tksh_EndBlock(interp);
    Tcl_StaticPackage(interp, "Tk", Tk_Init, (Tcl_PackageInitProc *) NULL);
#ifdef TK_TEST
    if (Tktest_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
    Tcl_StaticPackage(interp, "Tktest", Tktest_Init,
            (Tcl_PackageInitProc *) NULL);
#endif /* TK_TEST */


    /*
     * Call the init procedures for included packages.  Each call should
     * look like this:
     *
     * if (Mod_Init(interp) == TCL_ERROR) {
     *     return TCL_ERROR;
     * }
     *
     * where "Mod" is the name of the module.
     */

    /*
     * Call Tcl_CreateCommand for application-specific commands, if
     * they weren't already created by the init procedures called above.
     */

    /*
     * Specify a user-specific startup file to invoke if the application
     * is run interactively.  Typically the startup file is "~/.apprc"
     * where "app" is the name of the application.  If this line is deleted
     * then no user-specific startup file will be run under any conditions.
     */

    Tcl_SetVar(interp, "tcl_rcFileName", "~/.wishrc", TCL_GLOBAL_ONLY);
    return TCL_OK;
}
#include <signal.h>
static int gotIntr;
extern int Tcl_NumEventsFound(void);
static void SigEventSetup(ClientData clientData, int flags)
{
}
static int SigEventProcess(Tcl_Event *evPtr, int flags)
{
	return 1;
}
static void SigEventCheck(ClientData clientData, int flags)
{
	Tcl_Event *evPtr;
	if (Tcl_NumEventsFound() < 0)
	{
		evPtr = (Tcl_Event *) malloc(sizeof(Tcl_Event));
		evPtr->proc = SigEventProcess;
		gotIntr = 1;;
		Tcl_QueueEvent(evPtr, TCL_QUEUE_TAIL);
	}
}
static void TmoutProc(ClientData clientData)
{
	*((int *)clientData) = 1;
}
static void fileReady(ClientData clientData, int mask)
{
	Tcl_File *filePtr = (Tcl_File *) clientData;
	/* Tcl_DeleteFileHandler(*filePtr); */
	Tcl_CreateFileHandler(*filePtr, 0, fileReady, (ClientData) 0);
	*filePtr = NULL;
}
#include <wait.h>
int tksh_waitevent(int fd, long tmout, int rw)
{
	int tFlag = 0, result = 1;
	Tcl_TimerToken token;
	Tcl_File file = NULL;
	gotIntr = 0;

	if (fd >= 0)
	{
		file = Tcl_GetFile((ClientData)fd ,TCL_UNIX_FD);
		Tcl_CreateFileHandler(file, TCL_READABLE, fileReady, &file);
	}

        if (tmout> 0)
                token = Tcl_CreateTimerHandler((int)tmout,TmoutProc,&(tFlag));

	Tksh_BeginBlock(interp, INTERP_TCL);	/* Best Guess */
	while ((!gotIntr) && (!tFlag) && ((fd<0)||file) && result && (fd>=0 || !sh_waitsafe()))
		result = Tcl_DoOneEvent(0);
	Tksh_EndBlock(interp);

	if (gotIntr)
	{
		result = -1;
		errno = EINTR;
	} else
	{
		result = 1;
	}

        if (tmout > 0)
                Tcl_DeleteTimerHandler(token);
	if (file)
		Tcl_CreateFileHandler(file, 0, fileReady, (ClientData) 0);

	return result;
}
#if 0
static void stoptk(void)
{
        Tcl_Exit(0);
}
#endif
int b_tkinit(int argc, char *argv[], Shbltin_t *context)
{
        static char *av[] = { "tkinit", 0 };

        if (argc == 0)
        {
                argc = 1;
                argv = av;
        }
        Tksh_TkMain(argc,argv,context ? (Tcl_AppInitProc*)context->ptr : Tksh_AppInit);
	Tcl_CreateEventSource(SigEventSetup,SigEventCheck,NULL);
	sh_waitnotify(tksh_waitevent);
        /* atexit(stoptk); */
        return 0;
}
