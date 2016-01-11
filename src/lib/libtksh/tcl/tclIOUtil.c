/* 
 * tclIOUtil.c --
 *
 *	This file contains a collection of utility procedures that
 *	are shared by the platform specific IO drivers.
 *
 *	Parts of this file are based on code contributed by Karl
 *	Lehenbauer, Mark Diekhans and Peter da Silva.
 *
 * Copyright (c) 1991-1994 The Regents of the University of California.
 * Copyright (c) 1994-1996 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * SCCS: @(#) tclIOUtil.c 1.128 96/10/02 12:25:36
 */

#include "tclInt.h"
#include "tclPort.h"

extern Tcl_Channel TclCreateCommandChannel _ANSI_ARGS_((Tcl_File, Tcl_File,
						Tcl_File, int, int*));

/*
 * A linked list of the following structures is used to keep track
 * of child processes that have been detached but haven't exited
 * yet, so we can make sure that they're properly "reaped" (officially
 * waited for) and don't lie around as zombies cluttering the
 * system.
 */

typedef struct Detached {
    int pid;				/* Id of process that's been detached
					 * but isn't known to have exited. */
    struct Detached *nextPtr;		/* Next in list of all detached
					 * processes. */
} Detached;

static Detached *detList = NULL;	/* List of all detached proceses. */

/*
 * Declarations for local procedures defined in this file:
 */

#if 0
static Tcl_File	FileForRedirect _ANSI_ARGS_((Tcl_Interp *interp,
	                    char *spec, int atOk, char *arg, char *nextArg, 
			    int flags, int *skipPtr, int *closePtr,
			    Tcl_DString *namePtr));

/*
 *----------------------------------------------------------------------
 *
 * FileForRedirect --
 *
 *	This procedure does much of the work of parsing redirection
 *	operators.  It handles "@" if specified and allowed, and a file
 *	name, and opens the file if necessary.
 *
 * Results:
 *	The return value is the descriptor number for the file.  If an
 *	error occurs then NULL is returned and an error message is left
 *	in interp->result.  Several arguments are side-effected; see
 *	the argument list below for details.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static Tcl_File
FileForRedirect(interp, spec, atOK, arg, nextArg, flags, skipPtr, closePtr, 
	namePtr)
    Tcl_Interp *interp;		/* Intepreter to use for error reporting. */
    char *spec;			/* Points to character just after
				 * redirection character. */
    char *arg;			/* Pointer to entire argument containing 
				 * spec:  used for error reporting. */
    int atOK;			/* Non-zero means that '@' notation can be 
				 * used to specify a channel, zero means that
				 * it isn't. */
    char *nextArg;		/* Next argument in argc/argv array, if needed 
				 * for file name or channel name.  May be 
				 * NULL. */
    int flags;			/* Flags to use for opening file or to 
				 * specify mode for channel. */
    int *skipPtr;		/* Filled with 1 if redirection target was
				 * in spec, 2 if it was in nextArg. */
    int *closePtr;		/* Filled with one if the caller should 
				 * close the file when done with it, zero
				 * otherwise. */
    Tcl_DString *namePtr;	/* Pointer to an initialized Tcl_DString that
				 * is filled with the name of the file that
				 * was opened.  Unmodified if spec refers 
				 * to a channel.  */
{
    int writing = (flags & O_ACCMODE) == O_WRONLY;
    Tcl_Channel chan;
    Tcl_File file;

    *skipPtr = 1;
    if ((atOK != 0)  && (*spec == '@')) {
	spec++;
	if (*spec == '\0') {
	    spec = nextArg;
	    if (spec == NULL) {
		goto badLastArg;
	    }
	    *skipPtr = 2;
	}
        chan = Tcl_GetChannel(interp, spec, NULL);
        if (chan == (Tcl_Channel) NULL) {
            return NULL;
        }
        file = Tcl_GetChannelFile(chan, writing ? TCL_WRITABLE : TCL_READABLE);
        if (file == NULL) {
            Tcl_AppendResult(interp, "channel \"", Tcl_GetChannelName(chan),
                    "\" wasn't opened for ",
                    ((writing) ? "writing" : "reading"), (char *) NULL);
            return NULL;
        }
	if (writing) {

	    /*
	     * Be sure to flush output to the file, so that anything
	     * written by the child appears after stuff we've already
	     * written.
	     */

            Tcl_Flush(chan);
	}
    } else {
	char *name;

	if (*spec == '\0') {
	    spec = nextArg;
	    if (spec == NULL) {
		goto badLastArg;
	    }
	    *skipPtr = 2;
	}
	name = Tcl_TranslateFileName(interp, spec, namePtr);
	if (name != NULL) {
	    file = TclOpenFile(name, flags);
	} else {
	    file = NULL;
	}
	if (file == NULL) {
	    Tcl_AppendResult(interp, "couldn't ",
		    ((writing) ? "write" : "read"), " file \"", spec, "\": ",
		    Tcl_PosixError(interp), (char *) NULL);
	    Tcl_DStringFree(namePtr);
	    return NULL;
	}
        *closePtr = 1;
    }
    return file;

    badLastArg:
    Tcl_AppendResult(interp, "can't specify \"", arg,
	    "\" as last word in command", (char *) NULL);
    return NULL;
}
#endif

/*
 *----------------------------------------------------------------------
 *
 * TclGetOpenMode --
 *
 * Description:
 *	Computes a POSIX mode mask for opening a file, from a given string,
 *	and also sets a flag to indicate whether the caller should seek to
 *	EOF after opening the file.
 *
 * Results:
 *	On success, returns mode to pass to "open". If an error occurs, the
 *	returns -1 and if interp is not NULL, sets interp->result to an
 *	error message.
 *
 * Side effects:
 *	Sets the integer referenced by seekFlagPtr to 1 to tell the caller
 *	to seek to EOF after opening the file.
 *
 * Special note:
 *	This code is based on a prototype implementation contributed
 *	by Mark Diekhans.
 *
 *----------------------------------------------------------------------
 */

int
TclGetOpenMode(interp, string, seekFlagPtr)
    Tcl_Interp *interp;			/* Interpreter to use for error
					 * reporting - may be NULL. */
    char *string;			/* Mode string, e.g. "r+" or
					 * "RDONLY CREAT". */
    int *seekFlagPtr;			/* Set this to 1 if the caller
                                         * should seek to EOF during the
                                         * opening of the file. */
{
    int mode, modeArgc, c, i, gotRW;
    char **modeArgv, *flag;
#define RW_MODES (O_RDONLY|O_WRONLY|O_RDWR)

    /*
     * Check for the simpler fopen-like access modes (e.g. "r").  They
     * are distinguished from the POSIX access modes by the presence
     * of a lower-case first letter.
     */

    *seekFlagPtr = 0;
    mode = 0;
    if (islower(UCHAR(string[0]))) {
	switch (string[0]) {
	    case 'r':
		mode = O_RDONLY;
		break;
	    case 'w':
		mode = O_WRONLY|O_CREAT|O_TRUNC;
		break;
	    case 'a':
		mode = O_WRONLY|O_CREAT;
                *seekFlagPtr = 1;
		break;
	    default:
		error:
                if (interp != (Tcl_Interp *) NULL) {
                    Tcl_AppendResult(interp,
                            "illegal access mode \"", string, "\"",
                            (char *) NULL);
                }
		return -1;
	}
	if (string[1] == '+') {
	    mode &= ~O_ACCMODE;
	    mode |= O_RDWR;
	    if (string[2] != 0) {
		goto error;
	    }
	} else if (string[1] != 0) {
	    goto error;
	}
        return mode;
    }

    /*
     * The access modes are specified using a list of POSIX modes
     * such as O_CREAT.
     *
     * IMPORTANT NOTE: We rely on Tcl_SplitList working correctly when
     * a NULL interpreter is passed in.
     */

    if (Tcl_SplitList(interp, string, &modeArgc, &modeArgv) != TCL_OK) {
        if (interp != (Tcl_Interp *) NULL) {
            Tcl_AddErrorInfo(interp,
                    "\n    while processing open access modes \"");
            Tcl_AddErrorInfo(interp, string);
            Tcl_AddErrorInfo(interp, "\"");
        }
        return -1;
    }
    
    gotRW = 0;
    for (i = 0; i < modeArgc; i++) {
	flag = modeArgv[i];
	c = flag[0];
	if ((c == 'R') && (strcmp(flag, "RDONLY") == 0)) {
	    mode = (mode & ~O_ACCMODE) | O_RDONLY;
	    gotRW = 1;
	} else if ((c == 'W') && (strcmp(flag, "WRONLY") == 0)) {
	    mode = (mode & ~O_ACCMODE) | O_WRONLY;
	    gotRW = 1;
	} else if ((c == 'R') && (strcmp(flag, "RDWR") == 0)) {
	    mode = (mode & ~O_ACCMODE) | O_RDWR;
	    gotRW = 1;
	} else if ((c == 'A') && (strcmp(flag, "APPEND") == 0)) {
	    mode |= O_APPEND;
            *seekFlagPtr = 1;
	} else if ((c == 'C') && (strcmp(flag, "CREAT") == 0)) {
	    mode |= O_CREAT;
	} else if ((c == 'E') && (strcmp(flag, "EXCL") == 0)) {
	    mode |= O_EXCL;
	} else if ((c == 'N') && (strcmp(flag, "NOCTTY") == 0)) {
#ifdef O_NOCTTY
	    mode |= O_NOCTTY;
#else
	    if (interp != (Tcl_Interp *) NULL) {
                Tcl_AppendResult(interp, "access mode \"", flag,
                        "\" not supported by this system", (char *) NULL);
            }
            ckfree((char *) modeArgv);
	    return -1;
#endif
	} else if ((c == 'N') && (strcmp(flag, "NONBLOCK") == 0)) {
#if defined(O_NDELAY) || defined(O_NONBLOCK)
#   ifdef O_NONBLOCK
	    mode |= O_NONBLOCK;
#   else
	    mode |= O_NDELAY;
#   endif
#else
            if (interp != (Tcl_Interp *) NULL) {
                Tcl_AppendResult(interp, "access mode \"", flag,
                        "\" not supported by this system", (char *) NULL);
            }
            ckfree((char *) modeArgv);
	    return -1;
#endif
	} else if ((c == 'T') && (strcmp(flag, "TRUNC") == 0)) {
	    mode |= O_TRUNC;
	} else {
            if (interp != (Tcl_Interp *) NULL) {
                Tcl_AppendResult(interp, "invalid access mode \"", flag,
                        "\": must be RDONLY, WRONLY, RDWR, APPEND, CREAT",
                        " EXCL, NOCTTY, NONBLOCK, or TRUNC", (char *) NULL);
            }
	    ckfree((char *) modeArgv);
	    return -1;
	}
    }
    ckfree((char *) modeArgv);
    if (!gotRW) {
        if (interp != (Tcl_Interp *) NULL) {
            Tcl_AppendResult(interp, "access mode must include either",
                    " RDONLY, WRONLY, or RDWR", (char *) NULL);
        }
	return -1;
    }
    return mode;
}

#if 0
/*
 *----------------------------------------------------------------------
 *
 * Tcl_EvalFile --
 *
 *	Read in a file and process the entire file as one gigantic
 *	Tcl command.
 *
 * Results:
 *	A standard Tcl result, which is either the result of executing
 *	the file or an error indicating why the file couldn't be read.
 *
 * Side effects:
 *	Depends on the commands in the file.
 *
 *----------------------------------------------------------------------
 */

int
Tcl_EvalFile(interp, fileName)
    Tcl_Interp *interp;		/* Interpreter in which to process file. */
    char *fileName;		/* Name of file to process.  Tilde-substitution
				 * will be performed on this name. */
{
    int result;
    struct stat statBuf;
    char *cmdBuffer = (char *) NULL;
    char *oldScriptFile = (char *) NULL;
    Interp *iPtr = (Interp *) interp;
    Tcl_DString buffer;
    char *nativeName = (char *) NULL;
    Tcl_Channel chan = (Tcl_Channel) NULL;

    Tcl_ResetResult(interp);
    oldScriptFile = iPtr->scriptFile;
    iPtr->scriptFile = fileName;
    Tcl_DStringInit(&buffer);
    nativeName = Tcl_TranslateFileName(interp, fileName, &buffer);
    if (nativeName == NULL) {
	goto error;
    }

    /*
     * If Tcl_TranslateFileName didn't already copy the file name, do it
     * here.  This way we don't depend on fileName staying constant
     * throughout the execution of the script (e.g., what if it happens
     * to point to a Tcl variable that the script could change?).
     */

    if (nativeName != Tcl_DStringValue(&buffer)) {
	Tcl_DStringSetLength(&buffer, 0);
	Tcl_DStringAppend(&buffer, nativeName, -1);
	nativeName = Tcl_DStringValue(&buffer);
    }
    if (stat(nativeName, &statBuf) == -1) {
        Tcl_SetErrno(errno);
	Tcl_AppendResult(interp, "couldn't read file \"", fileName,
		"\": ", Tcl_PosixError(interp), (char *) NULL);
	goto error;
    }
    chan = Tcl_OpenFileChannel(interp, nativeName, "r", 0644);
    if (chan == (Tcl_Channel) NULL) {
        Tcl_ResetResult(interp);
	Tcl_AppendResult(interp, "couldn't read file \"", fileName,
		"\": ", Tcl_PosixError(interp), (char *) NULL);
	goto error;
    }
    cmdBuffer = (char *) ckalloc((unsigned) statBuf.st_size+1);
    result = Tcl_Read(chan, cmdBuffer, statBuf.st_size);
    if (result < 0) {
        Tcl_Close(interp, chan);
	Tcl_AppendResult(interp, "couldn't read file \"", fileName,
		"\": ", Tcl_PosixError(interp), (char *) NULL);
	goto error;
    }
    cmdBuffer[result] = 0;
    if (Tcl_Close(interp, chan) != TCL_OK) {
        goto error;
    }

    result = Tcl_Eval(interp, cmdBuffer);
    if (result == TCL_RETURN) {
	result = TclUpdateReturnInfo(iPtr);
    } else if (result == TCL_ERROR) {
	char msg[200];

	/*
	 * Record information telling where the error occurred.
	 */

	sprintf(msg, "\n    (file \"%.150s\" line %d)", fileName,
		interp->errorLine);
	Tcl_AddErrorInfo(interp, msg);
    }
    iPtr->scriptFile = oldScriptFile;
    ckfree(cmdBuffer);
    Tcl_DStringFree(&buffer);
    return result;

error:
    if (cmdBuffer != (char *) NULL) {
        ckfree(cmdBuffer);
    }
    iPtr->scriptFile = oldScriptFile;
    Tcl_DStringFree(&buffer);
    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_DetachPids --
 *
 *	This procedure is called to indicate that one or more child
 *	processes have been placed in background and will never be
 *	waited for;  they should eventually be reaped by
 *	Tcl_ReapDetachedProcs.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

void
Tcl_DetachPids(numPids, pidPtr)
    int numPids;		/* Number of pids to detach:  gives size
				 * of array pointed to by pidPtr. */
    int *pidPtr;		/* Array of pids to detach. */
{
    register Detached *detPtr;
    int i;

    for (i = 0; i < numPids; i++) {
	detPtr = (Detached *) ckalloc(sizeof(Detached));
	detPtr->pid = pidPtr[i];
	detPtr->nextPtr = detList;
	detList = detPtr;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_ReapDetachedProcs --
 *
 *	This procedure checks to see if any detached processes have
 *	exited and, if so, it "reaps" them by officially waiting on
 *	them.  It should be called "occasionally" to make sure that
 *	all detached processes are eventually reaped.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Processes are waited on, so that they can be reaped by the
 *	system.
 *
 *----------------------------------------------------------------------
 */

void
Tcl_ReapDetachedProcs()
{
    register Detached *detPtr;
    Detached *nextPtr, *prevPtr;
    int status;
    int pid;

    for (detPtr = detList, prevPtr = NULL; detPtr != NULL; ) {
	pid = (int) Tcl_WaitPid(detPtr->pid, &status, WNOHANG);
	if ((pid == 0) || ((pid == -1) && (errno != ECHILD))) {
	    prevPtr = detPtr;
	    detPtr = detPtr->nextPtr;
	    continue;
	}
	nextPtr = detPtr->nextPtr;
	if (prevPtr == NULL) {
	    detList = detPtr->nextPtr;
	} else {
	    prevPtr->nextPtr = detPtr->nextPtr;
	}
	ckfree((char *) detPtr);
	detPtr = nextPtr;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * TclCleanupChildren --
 *
 *	This is a utility procedure used to wait for child processes
 *	to exit, record information about abnormal exits, and then
 *	collect any stderr output generated by them.
 *
 * Results:
 *	The return value is a standard Tcl result.  If anything at
 *	weird happened with the child processes, TCL_ERROR is returned
 *	and a message is left in interp->result.
 *
 * Side effects:
 *	If the last character of interp->result is a newline, then it
 *	is removed unless keepNewline is non-zero.  File errorId gets
 *	closed, and pidPtr is freed back to the storage allocator.
 *
 *----------------------------------------------------------------------
 */

int
TclCleanupChildren(interp, numPids, pidPtr, errorChan)
    Tcl_Interp *interp;		/* Used for error messages. */
    int numPids;		/* Number of entries in pidPtr array. */
    int *pidPtr;		/* Array of process ids of children. */
    Tcl_Channel errorChan;	/* Channel for file containing stderr output
				 * from pipeline.  NULL means there isn't any
				 * stderr output. */
{
    int result = TCL_OK;
    int i, pid, abnormalExit, anyErrorInfo;
    WAIT_STATUS_TYPE waitStatus;
    char *msg;

    abnormalExit = 0;
    for (i = 0; i < numPids; i++) {
        pid = (int) Tcl_WaitPid(pidPtr[i], (int *) &waitStatus, 0);
	if (pid == -1) {
	    result = TCL_ERROR;
            if (interp != (Tcl_Interp *) NULL) {
                msg = Tcl_PosixError(interp);
                if (errno == ECHILD) {
		    /*
                     * This changeup in message suggested by Mark Diekhans
                     * to remind people that ECHILD errors can occur on
                     * some systems if SIGCHLD isn't in its default state.
                     */

                    msg =
                        "child process lost (is SIGCHLD ignored or trapped?)";
                }
                Tcl_AppendResult(interp, "error waiting for process to exit: ",
                        msg, (char *) NULL);
            }
	    continue;
	}

	/*
	 * Create error messages for unusual process exits.  An
	 * extra newline gets appended to each error message, but
	 * it gets removed below (in the same fashion that an
	 * extra newline in the command's output is removed).
	 */

	if (!WIFEXITED(waitStatus) || (WEXITSTATUS(waitStatus) != 0)) {
	    char msg1[20], msg2[20];

	    result = TCL_ERROR;
	    sprintf(msg1, "%d", pid);
	    if (WIFEXITED(waitStatus)) {
                if (interp != (Tcl_Interp *) NULL) {
                    sprintf(msg2, "%d", WEXITSTATUS(waitStatus));
                    Tcl_SetErrorCode(interp, "CHILDSTATUS", msg1, msg2,
                            (char *) NULL);
                }
		abnormalExit = 1;
	    } else if (WIFSIGNALED(waitStatus)) {
                if (interp != (Tcl_Interp *) NULL) {
                    char *p;
                    
                    p = Tcl_SignalMsg((int) (WTERMSIG(waitStatus)));
                    Tcl_SetErrorCode(interp, "CHILDKILLED", msg1,
                            Tcl_SignalId((int) (WTERMSIG(waitStatus))), p,
                            (char *) NULL);
                    Tcl_AppendResult(interp, "child killed: ", p, "\n",
                            (char *) NULL);
                }
	    } else if (WIFSTOPPED(waitStatus)) {
                if (interp != (Tcl_Interp *) NULL) {
                    char *p;

                    p = Tcl_SignalMsg((int) (WSTOPSIG(waitStatus)));
                    Tcl_SetErrorCode(interp, "CHILDSUSP", msg1,
                            Tcl_SignalId((int) (WSTOPSIG(waitStatus))),
                            p, (char *) NULL);
                    Tcl_AppendResult(interp, "child suspended: ", p, "\n",
                            (char *) NULL);
                }
	    } else {
                if (interp != (Tcl_Interp *) NULL) {
                    Tcl_AppendResult(interp,
                            "child wait status didn't make sense\n",
                            (char *) NULL);
                }
	    }
	}
    }

    /*
     * Read the standard error file.  If there's anything there,
     * then return an error and add the file's contents to the result
     * string.
     */

    anyErrorInfo = 0;
    if (errorChan != NULL) {

	/*
	 * Make sure we start at the beginning of the file.
	 */

	Tcl_Seek(errorChan, 0L, SEEK_SET);

        if (interp != (Tcl_Interp *) NULL) {
            while (1) {
#define BUFFER_SIZE 1000
                char buffer[BUFFER_SIZE+1];
                int count;
    
                count = Tcl_Read(errorChan, buffer, BUFFER_SIZE);
                if (count == 0) {
                    break;
                }
                result = TCL_ERROR;
                if (count < 0) {
                    Tcl_AppendResult(interp,
                            "error reading stderr output file: ",
                            Tcl_PosixError(interp), (char *) NULL);
                    break;	/* out of the "while (1)" loop. */
                }
                buffer[count] = 0;
                Tcl_AppendResult(interp, buffer, (char *) NULL);
                anyErrorInfo = 1;
            }
        }
        
	Tcl_Close((Tcl_Interp *) NULL, errorChan);
    }

    /*
     * If a child exited abnormally but didn't output any error information
     * at all, generate an error message here.
     */

    if (abnormalExit && !anyErrorInfo && (interp != (Tcl_Interp *) NULL)) {
	Tcl_AppendResult(interp, "child process exited abnormally",
		(char *) NULL);
    }
    
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * TclCreatePipeline --
 *
 *	Given an argc/argv array, instantiate a pipeline of processes
 *	as described by the argv.
 *
 * Results:
 *	The return value is a count of the number of new processes
 *	created, or -1 if an error occurred while creating the pipeline.
 *	*pidArrayPtr is filled in with the address of a dynamically
 *	allocated array giving the ids of all of the processes.  It
 *	is up to the caller to free this array when it isn't needed
 *	anymore.  If inPipePtr is non-NULL, *inPipePtr is filled in
 *	with the file id for the input pipe for the pipeline (if any):
 *	the caller must eventually close this file.  If outPipePtr
 *	isn't NULL, then *outPipePtr is filled in with the file id
 *	for the output pipe from the pipeline:  the caller must close
 *	this file.  If errFilePtr isn't NULL, then *errFilePtr is filled
 *	with a file id that may be used to read error output after the
 *	pipeline completes.
 *
 * Side effects:
 *	Processes and pipes are created.
 *
 *----------------------------------------------------------------------
 */

int
TclCreatePipeline(interp, argc, argv, pidArrayPtr, inPipePtr,
	outPipePtr, errFilePtr)
    Tcl_Interp *interp;		/* Interpreter to use for error reporting. */
    int argc;			/* Number of entries in argv. */
    char **argv;		/* Array of strings describing commands in
				 * pipeline plus I/O redirection with <,
				 * <<,  >, etc.  Argv[argc] must be NULL. */
    int **pidArrayPtr;		/* Word at *pidArrayPtr gets filled in with
				 * address of array of pids for processes
				 * in pipeline (first pid is first process
				 * in pipeline). */
    Tcl_File *inPipePtr;	/* If non-NULL, input to the pipeline comes
				 * from a pipe (unless overridden by
				 * redirection in the command).  The file
				 * id with which to write to this pipe is
				 * stored at *inPipePtr.  NULL means command
				 * specified its own input source. */
    Tcl_File *outPipePtr;	/* If non-NULL, output to the pipeline goes
				 * to a pipe, unless overriden by redirection
				 * in the command.  The file id with which to
				 * read frome this pipe is stored at
				 * *outPipePtr.  NULL means command specified
				 * its own output sink. */
    Tcl_File *errFilePtr;	/* If non-NULL, all stderr output from the
				 * pipeline will go to a temporary file
				 * created here, and a descriptor to read
				 * the file will be left at *errFilePtr.
				 * The file will be removed already, so
				 * closing this descriptor will be the end
				 * of the file.  If this is NULL, then
				 * all stderr output goes to our stderr.
				 * If the pipeline specifies redirection
				 * then the file will still be created
				 * but it will never get any data. */
{
#if defined( MAC_TCL )
    Tcl_AppendResult(interp,
	    "command pipelines not supported on Macintosh OS", NULL);
    return -1;
#else /* !MAC_TCL */
    int *pidPtr = NULL;		/* Points to malloc-ed array holding all
				 * the pids of child processes. */
    int numPids = 0;		/* Actual number of processes that exist
				 * at *pidPtr right now. */
    int cmdCount;		/* Count of number of distinct commands
				 * found in argc/argv. */
    char *inputLiteral = NULL;	/* If non-null, then this points to a
				 * string containing input data (specified
				 * via <<) to be piped to the first process
				 * in the pipeline. */
    Tcl_File inputFile = NULL;	/* If != NULL, gives file to use as input for
				 * first process in pipeline (specified via <
				 * or <@). */
    Tcl_DString inputFileName;	/* If non-empty, gives name of file that 
				 * corresponds to inputFile. */
    int inputClose = 0;		/* If non-zero, then inputFile should be 
    				 * closed when cleaning up. */
    Tcl_File outputFile = NULL;	/* Writable file for output from last command
				 * in pipeline (could be file or pipe).  NULL
				 * means use stdout. */
    Tcl_DString outputFileName;	/* If non-empty, gives name of file that 
				 * corresponds to outputFile. */
    int outputClose = 0;	/* If non-zero, then outputFile should be 
    				 * closed when cleaning up. */
    Tcl_File errorFile = NULL;	/* Writable file for error output from all
				 * commands in pipeline.  NULL means use
				 * stderr. */
    Tcl_DString errorFileName;	/* If non-empty, gives name of file that 
				 * corresponds to errorFile. */
    int errorClose = 0;		/* If non-zero, then errorFile should be 
    				 * closed when cleaning up. */
    char *p;
    int skip, lastBar, lastArg, i, j, atOK, flags, errorToOutput;
    Tcl_DString execBuffer;
    Tcl_File pipeIn;
    Tcl_File curInFile, curOutFile, curErrFile;
    char *curInFileName, *curOutFileName, *curErrFileName;
    Tcl_Channel channel;

    if (inPipePtr != NULL) {
	*inPipePtr = NULL;
    }
    if (outPipePtr != NULL) {
	*outPipePtr = NULL;
    }
    if (errFilePtr != NULL) {
	*errFilePtr = NULL;
    }

    Tcl_DStringInit(&inputFileName);
    Tcl_DStringInit(&outputFileName);
    Tcl_DStringInit(&errorFileName);
    Tcl_DStringInit(&execBuffer);
    
    pipeIn = NULL;
    curInFile = NULL;
    curOutFile = NULL;
    curErrFile = NULL;

    numPids = 0;
    pidPtr = NULL;

    /*
     * First, scan through all the arguments to figure out the structure
     * of the pipeline.  Process all of the input and output redirection
     * arguments and remove them from the argument list in the pipeline.
     * Count the number of distinct processes (it's the number of "|"
     * arguments plus one) but don't remove the "|" arguments because 
     * they'll be used in the second pass to seperate the individual 
     * child processes.  Cannot start the child processes in this pass 
     * because the redirection symbols may appear anywhere in the 
     * command line -- e.g., the '<' that specifies the input to the 
     * entire pipe may appear at the very end of the argument list.
     */

    lastBar = -1;
    cmdCount = 1;
    for (i = 0; i < argc; i++) {
        skip = 0;
	p = argv[i];
	switch (*p++) {
	case '|':
	    if (*p == '&') {
		p++;
	    }
	    if (*p == '\0') {
		if ((i == (lastBar + 1)) || (i == (argc - 1))) {
		    interp->result = "illegal use of | or |& in command";
		    goto error;
		}
	    }
	    lastBar = i;
	    cmdCount++;
	    break;

	case '<':
	    if (inputClose != 0) {
		inputClose = 0;
		Tcl_DStringFree(&inputFileName);
		TclCloseFile(inputFile);
	    }
	    if (*p == '<') {
		inputFile = NULL;
		inputLiteral = p + 1;
		skip = 1;
		if (*inputLiteral == '\0') {
		    inputLiteral = argv[i + 1];
		    if (inputLiteral == NULL) {
			Tcl_AppendResult(interp, "can't specify \"", argv[i],
				"\" as last word in command", (char *) NULL);
			goto error;
		    }
		    skip = 2;
		}
	    } else {
		inputLiteral = NULL;
		inputFile = FileForRedirect(interp, p, 1, argv[i], 
			argv[i + 1], O_RDONLY, &skip, &inputClose, 
			&inputFileName);
		if (inputFile == NULL) {
		    goto error;
		}
	    }
	    break;

	case '>':
	    atOK = 1;
	    flags = O_WRONLY | O_CREAT | O_TRUNC;
	    errorToOutput = 0;
	    if (*p == '>') {
		p++;
		atOK = 0;
		flags = O_WRONLY | O_CREAT;
	    }
	    if (*p == '&') {
		if (errorClose != 0) {
		    errorClose = 0;
		    Tcl_DStringFree(&errorFileName);
		    TclCloseFile(errorFile);
		}
		errorToOutput = 1;
		p++;
	    }

	    if (outputClose != 0) {
		outputClose = 0;
		Tcl_DStringFree(&outputFileName);
		TclCloseFile(outputFile);
	    }
	    outputFile = FileForRedirect(interp, p, atOK, argv[i], 
		    argv[i + 1], flags, &skip, &outputClose, 
		    &outputFileName);
	    if (outputFile == NULL) {
		goto error;
	    }
	    if (atOK == 0) {
		TclSeekFile(outputFile, 0, SEEK_END);
	    }
	    if (errorToOutput) {
		errorClose = 0;
		errorFile = outputFile;
	    }
	    break;

	case '2':
	    if (*p != '>') {
		break;
	    }
	    p++;
	    atOK = 1;
	    flags = O_WRONLY | O_CREAT | O_TRUNC;
	    if (*p == '>') {
		p++;
		atOK = 0;
		flags = O_WRONLY | O_CREAT;
	    }
	    if (errorClose != 0) {
		errorClose = 0;
		Tcl_DStringFree(&errorFileName);
		TclCloseFile(errorFile);
	    }
	    errorFile = FileForRedirect(interp, p, atOK, argv[i], 
		    argv[i + 1], flags, &skip, &errorClose, 
		    &errorFileName);
	    if (errorFile == NULL) {
		goto error;
	    }
	    if (atOK == 0) {
		TclSeekFile(errorFile, 0, SEEK_END);
	    }
	    break;
	}

	if (skip != 0) {
	    for (j = i + skip; j < argc; j++) {
		argv[j - skip] = argv[j];
	    }
	    argc -= skip;
	    i -= 1;
	}
    }

    if (inputFile == NULL) {
	if (inputLiteral != NULL) {
	    /*
	     * The input for the first process is immediate data coming from
	     * Tcl.  Create a temporary file for it and put the data into the
	     * file.
	     */
	    inputFile = TclCreateTempFile(inputLiteral, &inputFileName);
	    if (inputFile == NULL) {
		Tcl_AppendResult(interp,
			"couldn't create input file for command: ",
			Tcl_PosixError(interp), (char *) NULL);
		goto error;
	    }
	    inputClose = 1;
	} else if (inPipePtr != NULL) {
	    /*
	     * The input for the first process in the pipeline is to
	     * come from a pipe that can be written from by the caller.
	     */

	    if (TclCreatePipe(&inputFile, inPipePtr) == 0) {
		Tcl_AppendResult(interp, 
			"couldn't create input pipe for command: ",
			Tcl_PosixError(interp), (char *) NULL);
		goto error;
	    }
	    inputClose = 1;
	} else {
	    /*
	     * The input for the first process comes from stdin.
	     */

	    channel = Tcl_GetStdChannel(TCL_STDIN);
	    if (channel != NULL) {
		inputFile = Tcl_GetChannelFile(channel, TCL_READABLE);
	    }
	}
    }

    if (outputFile == NULL) {
	if (outPipePtr != NULL) {
	    /*
	     * Output from the last process in the pipeline is to go to a
	     * pipe that can be read by the caller.
	     */

	    if (TclCreatePipe(outPipePtr, &outputFile) == 0) {
		Tcl_AppendResult(interp, 
			"couldn't create output pipe for command: ",
			Tcl_PosixError(interp), (char *) NULL);
		goto error;
	    }
	    outputClose = 1;
	} else {
	    /*
	     * The output for the last process goes to stdout.
	     */

	    channel = Tcl_GetStdChannel(TCL_STDOUT);
	    if (channel) {
		outputFile = Tcl_GetChannelFile(channel, TCL_WRITABLE);
	    }
	}
    }

    if (errorFile == NULL) {
	if (errFilePtr != NULL) {
	    /*
	     * Set up the standard error output sink for the pipeline, if
	     * requested.  Use a temporary file which is opened, then deleted.
	     * Could potentially just use pipe, but if it filled up it could
	     * cause the pipeline to deadlock:  we'd be waiting for processes
	     * to complete before reading stderr, and processes couldn't 
	     * complete because stderr was backed up.
	     */

	    errorFile = TclCreateTempFile(NULL, &errorFileName);
	    if (errorFile == NULL) {
		Tcl_AppendResult(interp,
			"couldn't create error file for command: ",
			Tcl_PosixError(interp), (char *) NULL);
		goto error;
	    }
	    *errFilePtr = errorFile;
	} else {
	    /*
	     * Errors from the pipeline go to stderr.
	     */

	    channel = Tcl_GetStdChannel(TCL_STDERR);
	    if (channel) {
		errorFile = Tcl_GetChannelFile(channel, TCL_WRITABLE);
	    }
	}
    }
	
    /*
     * Scan through the argc array, creating a process for each
     * group of arguments between the "|" characters.
     */

    Tcl_ReapDetachedProcs();
    pidPtr = (int *) ckalloc((unsigned) (cmdCount * sizeof(int)));

    curInFile = inputFile;
    curInFileName = Tcl_DStringValue(&inputFileName);
    if (curInFileName[0] == '\0') {
	curInFileName = NULL;
    }

    for (i = 0; i < argc; i = lastArg + 1) { 
	int joinThisError, pid;

	/*
	 * Convert the program name into native form. 
	 */

	argv[i] = Tcl_TranslateFileName(interp, argv[i], &execBuffer);
	if (argv[i] == NULL) {
	    goto error;
	}

	/*
	 * Find the end of the current segment of the pipeline.
	 */

	joinThisError = 0;
	for (lastArg = i; lastArg < argc; lastArg++) {
	    if (argv[lastArg][0] == '|') { 
		if (argv[lastArg][1] == '\0') { 
		    break;
		}
		if ((argv[lastArg][1] == '&') && (argv[lastArg][2] == '\0')) {
		    joinThisError = 1;
		    break;
		}
	    }
	}
	argv[lastArg] = NULL;

	/*
	 * If this is the last segment, use the specified outputFile.
	 * Otherwise create an intermediate pipe.  pipeIn will become the
	 * curInFile for the next segment of the pipe.
	 */

	if (lastArg == argc) { 
	    curOutFile = outputFile;
	    curOutFileName = Tcl_DStringValue(&outputFileName);
	    if (curOutFileName[0] == '\0') {
		curOutFileName = NULL;
	    }
	} else {
	    if (TclCreatePipe(&pipeIn, &curOutFile) == 0) {
		Tcl_AppendResult(interp, "couldn't create pipe: ",
			Tcl_PosixError(interp), (char *) NULL);
		goto error;
	    }
	    curOutFileName = NULL;
	}

	if (joinThisError != 0) {
	    curErrFile = curOutFile;
	    curErrFileName = curOutFileName;
	} else {
	    curErrFile = errorFile;
	    curErrFileName = Tcl_DStringValue(&errorFileName);
	    if (curErrFileName[0] == '\0') {
		curErrFileName = NULL;
	    }
	}

	if (TclpCreateProcess(interp, lastArg - i, argv + i,
		curInFile, curOutFile, curErrFile, curInFileName, 
		curOutFileName, curErrFileName, &pid) != TCL_OK) {
	    goto error;
	}
	Tcl_DStringFree(&execBuffer);

	pidPtr[numPids] = pid;
	numPids++;

	/*
	 * Close off our copies of file descriptors that were set up for
	 * this child, then set up the input for the next child.
	 */

	if ((curInFile != NULL) && (curInFile != inputFile)) {
	    TclCloseFile(curInFile);
	}
	curInFile = pipeIn;
	curInFileName = NULL;
	pipeIn = NULL;

	if ((curOutFile != NULL) && (curOutFile != outputFile)) {
	    TclCloseFile(curOutFile);
	}
	curOutFile = NULL;
    }

    *pidArrayPtr = pidPtr;

    /*
     * All done.  Cleanup open files lying around and then return.
     */

cleanup:
    Tcl_DStringFree(&inputFileName);
    Tcl_DStringFree(&outputFileName);
    Tcl_DStringFree(&errorFileName);
    Tcl_DStringFree(&execBuffer);

    if (inputClose) {
	TclCloseFile(inputFile);
    }
    if (outputClose) {
	TclCloseFile(outputFile);
    }
    if (errorClose) {
	TclCloseFile(errorFile);
    }
    return numPids;

    /*
     * An error occurred.  There could have been extra files open, such
     * as pipes between children.  Clean them all up.  Detach any child
     * processes that have been created.
     */

error:
    if (pipeIn != NULL) {
	TclCloseFile(pipeIn);
    }
    if ((curOutFile != NULL) && (curOutFile != outputFile)) {
	TclCloseFile(curOutFile);
    }
    if ((curInFile != NULL) && (curInFile != inputFile)) {
	TclCloseFile(curInFile);
    }
    if ((inPipePtr != NULL) && (*inPipePtr != NULL)) {
	TclCloseFile(*inPipePtr);
	*inPipePtr = NULL;
    }
    if ((outPipePtr != NULL) && (*outPipePtr != NULL)) {
	TclCloseFile(*outPipePtr);
	*outPipePtr = NULL;
    }
    if ((errFilePtr != NULL) && (*errFilePtr != NULL)) {
	TclCloseFile(*errFilePtr);
	*errFilePtr = NULL;
    }
    if (pidPtr != NULL) {
	for (i = 0; i < numPids; i++) {
	    if (pidPtr[i] != -1) {
		Tcl_DetachPids(1, &pidPtr[i]);
	    }
	}
	ckfree((char *) pidPtr);
    }
    numPids = -1;
    goto cleanup;
#endif /* !MAC_TCL */
}
#endif

/*
 *----------------------------------------------------------------------
 *
 * Tcl_GetErrno --
 *
 *	Gets the current value of the Tcl error code variable. This is
 *	currently the global variable "errno" but could in the future
 *	change to something else.
 *
 * Results:
 *	The value of the Tcl error code variable.
 *
 * Side effects:
 *	None. Note that the value of the Tcl error code variable is
 *	UNDEFINED if a call to Tcl_SetErrno did not precede this call.
 *
 *----------------------------------------------------------------------
 */

int
Tcl_GetErrno()
{
    return errno;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_SetErrno --
 *
 *	Sets the Tcl error code variable to the supplied value.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Modifies the value of the Tcl error code variable.
 *
 *----------------------------------------------------------------------
 */

void
Tcl_SetErrno(err)
    int err;			/* The new value. */
{
    errno = err;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_PosixError --
 *
 *	This procedure is typically called after UNIX kernel calls
 *	return errors.  It stores machine-readable information about
 *	the error in $errorCode returns an information string for
 *	the caller's use.
 *
 * Results:
 *	The return value is a human-readable string describing the
 *	error.
 *
 * Side effects:
 *	The global variable $errorCode is reset.
 *
 *----------------------------------------------------------------------
 */

char *
Tcl_PosixError(interp)
    Tcl_Interp *interp;		/* Interpreter whose $errorCode variable
				 * is to be changed. */
{
    char *id, *msg;

    msg = Tcl_ErrnoMsg(errno);
    id = Tcl_ErrnoId();
    Tcl_SetErrorCode(interp, "POSIX", id, msg, (char *) NULL);
    return msg;
}

#if 1
/*
 *----------------------------------------------------------------------
 *
 * Tcl_OpenCommandChannel --
 *
 *	Opens an I/O channel to one or more subprocesses specified
 *	by argc and argv.  The flags argument determines the
 *	disposition of the stdio handles.  If the TCL_STDIN flag is
 *	set then the standard input for the first subprocess will
 *	be tied to the channel:  writing to the channel will provide
 *	input to the subprocess.  If TCL_STDIN is not set, then
 *	standard input for the first subprocess will be the same as
 *	this application's standard input.  If TCL_STDOUT is set then
 *	standard output from the last subprocess can be read from the
 *	channel;  otherwise it goes to this application's standard
 *	output.  If TCL_STDERR is set, standard error output for all
 *	subprocesses is returned to the channel and results in an error
 *	when the channel is closed;  otherwise it goes to this
 *	application's standard error.  If TCL_ENFORCE_MODE is not set,
 *	then argc and argv can redirect the stdio handles to override
 *	TCL_STDIN, TCL_STDOUT, and TCL_STDERR;  if it is set, then it 
 *	is an error for argc and argv to override stdio channels for
 *	which TCL_STDIN, TCL_STDOUT, and TCL_STDERR have been set.
 *
 * Results:
 *	A new command channel, or NULL on failure with an error
 *	message left in interp.
 *
 * Side effects:
 *	Creates processes, opens pipes.
 *
 *----------------------------------------------------------------------
 */

Tcl_Channel
Tcl_OpenCommandChannel(interp, argc, argv, flags)
    Tcl_Interp *interp;		/* Interpreter for error reporting. Can
                                 * NOT be NULL. */
    int argc;			/* How many arguments. */
    char **argv;		/* Array of arguments for command pipe. */
    int flags;			/* Or'ed combination of TCL_STDIN, TCL_STDOUT,
				 * TCL_STDERR, and TCL_ENFORCE_MODE. */
{
    Tcl_File *inPipePtr, *outPipePtr, *errFilePtr;
    Tcl_File inPipe, outPipe, errFile;
    int numPids, *pidPtr;
    Tcl_Channel channel;

    inPipe = outPipe = errFile = NULL;

    inPipePtr = (flags & TCL_STDIN) ? &inPipe : NULL;
    outPipePtr = (flags & TCL_STDOUT) ? &outPipe : NULL;
    errFilePtr = (flags & TCL_STDERR) ? &errFile : NULL;
    
    numPids = TclCreatePipeline(interp, argc, argv, &pidPtr, inPipePtr,
            outPipePtr, errFilePtr);

    if (numPids < 0) {
	goto error;
    }

    /*
     * Verify that the pipes that were created satisfy the
     * readable/writable constraints. 
     */

    if (flags & TCL_ENFORCE_MODE) {
	if ((flags & TCL_STDOUT) && (outPipe == NULL)) {
	    Tcl_AppendResult(interp, "can't read output from command:",
		    " standard output was redirected", (char *) NULL);
	    goto error;
	}
	if ((flags & TCL_STDIN) && (inPipe == NULL)) {
	    Tcl_AppendResult(interp, "can't write input to command:",
		    " standard input was redirected", (char *) NULL);
	    goto error;
	}
    }
    
    channel = TclCreateCommandChannel(outPipe, inPipe, errFile,
	    numPids, pidPtr);

    if (channel == (Tcl_Channel) NULL) {
        Tcl_AppendResult(interp, "pipe for command could not be created",
                (char *) NULL);
	goto error;
    }
    return channel;

error:
#if 0
    if (numPids > 0) {
	Tcl_DetachPids(numPids, pidPtr);
	ckfree((char *) pidPtr);
    }
#endif
    if (inPipe != NULL) {
	TclClosePipeFile(inPipe);
    }
    if (outPipe != NULL) {
	TclClosePipeFile(outPipe);
    }
    if (errFile != NULL) {
	TclClosePipeFile(errFile);
    }
    return NULL;
}
#endif
