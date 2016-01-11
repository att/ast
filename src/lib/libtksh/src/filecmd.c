#pragma prototyped
#include "tkshlib.h"
#include "tclPort.h"

/* FROM: tclUnizAZ.c in Tcl 7.4.  Avoiding 7.5 version because the
 * platform stuff isn't needed
 *	Warning: options join pathtype split missing
 */

static char *           GetFileType _ANSI_ARGS_((int mode));
char *          GetOpenMode _ANSI_ARGS_((Tcl_Interp *interp,
                            char *string, int *modePtr));
static int              StoreStatData _ANSI_ARGS_((Tcl_Interp *interp,
                            char *varName, struct stat *statPtr));


/*
 *----------------------------------------------------------------------
 *
 * Tcl_FileCmd --
 *
 *	This procedure is invoked to process the "file" Tcl command.
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
Tcl_FileCmd1(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    char *p, *fileName;
    int c, statOp, result;
    size_t length;
    int mode = 0;			/* Initialized only to prevent
					 * compiler warning message. */
    struct stat statBuf;
    Tcl_DString buffer;

    if (argc < 3) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" option name ?arg ...?\"", (char *) NULL);
	return TCL_ERROR;
    }
    c = argv[1][0];
    length = strlen(argv[1]);
    result = TCL_OK;

    /*
     * First handle operations on the file name.
     */

    fileName = Tcl_TildeSubst(interp, argv[2], &buffer);
    if (fileName == NULL) {
	result = TCL_ERROR;
	goto done;
    }
    if ((c == 'd') && (strncmp(argv[1], "dirname", length) == 0)) {
	if (argc != 3) {
	    argv[1] = "dirname";
	    not3Args:
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		    " ", argv[1], " name\"", (char *) NULL);
	    result = TCL_ERROR;
	    goto done;
	}
	p = strrchr(fileName, '/');
	if (p == NULL) {
	    interp->result = ".";
	} else if (p == fileName) {
	    interp->result = "/";
	} else {
	    *p = 0;
	    Tcl_SetResult(interp, fileName, TCL_VOLATILE);
	    *p = '/';
	}
	goto done;
    } else if ((c == 'r') && (strncmp(argv[1], "rootname", length) == 0)
	    && (length >= 2)) {
	char *lastSlash;

	if (argc != 3) {
	    argv[1] = "rootname";
	    goto not3Args;
	}
	p = strrchr(fileName, '.');
	lastSlash = strrchr(fileName, '/');
	if ((p == NULL) || ((lastSlash != NULL) && (lastSlash > p))) {
	    Tcl_SetResult(interp, fileName, TCL_VOLATILE);
	} else {
	    *p = 0;
	    Tcl_SetResult(interp, fileName, TCL_VOLATILE);
	    *p = '.';
	}
	goto done;
    } else if ((c == 'e') && (strncmp(argv[1], "extension", length) == 0)
	    && (length >= 3)) {
	char *lastSlash;

	if (argc != 3) {
	    argv[1] = "extension";
	    goto not3Args;
	}
	p = strrchr(fileName, '.');
	lastSlash = strrchr(fileName, '/');
	if ((p != NULL) && ((lastSlash == NULL) || (lastSlash < p))) {
	    Tcl_SetResult(interp, p, TCL_VOLATILE);
	}
	goto done;
    } else if ((c == 't') && (strncmp(argv[1], "tail", length) == 0)
	    && (length >= 2)) {
	if (argc != 3) {
	    argv[1] = "tail";
	    goto not3Args;
	}
	p = strrchr(fileName, '/');
	if (p != NULL) {
	    Tcl_SetResult(interp, p+1, TCL_VOLATILE);
	} else {
	    Tcl_SetResult(interp, fileName, TCL_VOLATILE);
	}
	goto done;
    }

    /*
     * Next, handle operations that can be satisfied with the "access"
     * kernel call.
     */

    if (fileName == NULL) {
	result = TCL_ERROR;
	goto done;
    }
    if ((c == 'r') && (strncmp(argv[1], "readable", length) == 0)
	    && (length >= 5)) {
	if (argc != 3) {
	    argv[1] = "readable";
	    goto not3Args;
	}
	mode = R_OK;
	checkAccess:
	if (access(fileName, mode) == -1) {
	    interp->result = "0";
	} else {
	    interp->result = "1";
	}
	goto done;
    } else if ((c == 'w') && (strncmp(argv[1], "writable", length) == 0)) {
	if (argc != 3) {
	    argv[1] = "writable";
	    goto not3Args;
	}
	mode = W_OK;
	goto checkAccess;
    } else if ((c == 'e') && (strncmp(argv[1], "executable", length) == 0)
	    && (length >= 3)) {
	if (argc != 3) {
	    argv[1] = "executable";
	    goto not3Args;
	}
	mode = X_OK;
	goto checkAccess;
    } else if ((c == 'e') && (strncmp(argv[1], "exists", length) == 0)
	    && (length >= 3)) {
	if (argc != 3) {
	    argv[1] = "exists";
	    goto not3Args;
	}
	mode = F_OK;
	goto checkAccess;
    }

    /*
     * Lastly, check stuff that requires the file to be stat-ed.
     */

    if ((c == 'a') && (strncmp(argv[1], "atime", length) == 0)) {
	if (argc != 3) {
	    argv[1] = "atime";
	    goto not3Args;
	}
	if (stat(fileName, &statBuf) == -1) {
	    goto badStat;
	}
	sprintf(interp->result, "%ld", statBuf.st_atime);
	goto done;
    } else if ((c == 'i') && (strncmp(argv[1], "isdirectory", length) == 0)
	    && (length >= 3)) {
	if (argc != 3) {
	    argv[1] = "isdirectory";
	    goto not3Args;
	}
	statOp = 2;
    } else if ((c == 'i') && (strncmp(argv[1], "isfile", length) == 0)
	    && (length >= 3)) {
	if (argc != 3) {
	    argv[1] = "isfile";
	    goto not3Args;
	}
	statOp = 1;
    } else if ((c == 'l') && (strncmp(argv[1], "lstat", length) == 0)) {
	if (argc != 4) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		    " lstat name varName\"", (char *) NULL);
	    result = TCL_ERROR;
	    goto done;
	}

	if (lstat(fileName, &statBuf) == -1) {
	    Tcl_AppendResult(interp, "couldn't lstat \"", argv[2],
		    "\": ", Tcl_PosixError(interp), (char *) NULL);
	    result = TCL_ERROR;
	    goto done;
	}
	result = StoreStatData(interp, argv[3], &statBuf);
	goto done;
    } else if ((c == 'm') && (strncmp(argv[1], "mtime", length) == 0)) {
	if (argc != 3) {
	    argv[1] = "mtime";
	    goto not3Args;
	}
	if (stat(fileName, &statBuf) == -1) {
	    goto badStat;
	}
	sprintf(interp->result, "%ld", statBuf.st_mtime);
	goto done;
    } else if ((c == 'o') && (strncmp(argv[1], "owned", length) == 0)) {
	if (argc != 3) {
	    argv[1] = "owned";
	    goto not3Args;
	}
	statOp = 0;
    } else if ((c == 'r') && (strncmp(argv[1], "readlink", length) == 0)
	    && (length >= 5)) {
	char linkValue[MAXPATHLEN+1];
	int linkLength;

	if (argc != 3) {
	    argv[1] = "readlink";
	    goto not3Args;
	}

	/*
	 * If S_IFLNK isn't defined it means that the machine doesn't
	 * support symbolic links, so the file can't possibly be a
	 * symbolic link.  Generate an EINVAL error, which is what
	 * happens on machines that do support symbolic links when
	 * you invoke readlink on a file that isn't a symbolic link.
	 */

#ifndef S_IFLNK
	linkLength = -1;
	errno = EINVAL;
#else
	linkLength = readlink(fileName, linkValue, sizeof(linkValue) - 1);
#endif /* S_IFLNK */
	if (linkLength == -1) {
	    Tcl_AppendResult(interp, "couldn't readlink \"", argv[2],
		    "\": ", Tcl_PosixError(interp), (char *) NULL);
	    result = TCL_ERROR;
	    goto done;
	}
	linkValue[linkLength] = 0;
	Tcl_SetResult(interp, linkValue, TCL_VOLATILE);
	goto done;
    } else if ((c == 's') && (strncmp(argv[1], "size", length) == 0)
	    && (length >= 2)) {
	if (argc != 3) {
	    argv[1] = "size";
	    goto not3Args;
	}
	if (stat(fileName, &statBuf) == -1) {
	    goto badStat;
	}
	sprintf(interp->result, "%ld", statBuf.st_size);
	goto done;
    } else if ((c == 's') && (strncmp(argv[1], "stat", length) == 0)
	    && (length >= 2)) {
	if (argc != 4) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		    " stat name varName\"", (char *) NULL);
	    result = TCL_ERROR;
	    goto done;
	}

	if (stat(fileName, &statBuf) == -1) {
	    badStat:
	    Tcl_AppendResult(interp, "couldn't stat \"", argv[2],
		    "\": ", Tcl_PosixError(interp), (char *) NULL);
	    result = TCL_ERROR;
	    goto done;
	}
	result = StoreStatData(interp, argv[3], &statBuf);
	goto done;
    } else if ((c == 't') && (strncmp(argv[1], "type", length) == 0)
	    && (length >= 2)) {
	if (argc != 3) {
	    argv[1] = "type";
	    goto not3Args;
	}
	if (lstat(fileName, &statBuf) == -1) {
	    goto badStat;
	}
	interp->result = GetFileType((int) statBuf.st_mode);
	goto done;
    } else {
	Tcl_AppendResult(interp, "bad option \"", argv[1],
		"\": should be atime, dirname, executable, exists, ",
		"extension, isdirectory, isfile, lstat, mtime, owned, ",
		"readable, readlink, ",
		"root, size, stat, tail, type, ",
		"or writable",
		(char *) NULL);
	result = TCL_ERROR;
	goto done;
    }
    if (stat(fileName, &statBuf) == -1) {
	interp->result = "0";
	goto done;
    }
    switch (statOp) {
	case 0:
	    mode = (geteuid() == statBuf.st_uid);
	    break;
	case 1:
	    mode = S_ISREG(statBuf.st_mode);
	    break;
	case 2:
	    mode = S_ISDIR(statBuf.st_mode);
	    break;
    }
    if (mode) {
	interp->result = "1";
    } else {
	interp->result = "0";
    }

    done:
    Tcl_DStringFree(&buffer);
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * StoreStatData --
 *
 *	This is a utility procedure that breaks out the fields of a
 *	"stat" structure and stores them in textual form into the
 *	elements of an associative array.
 *
 * Results:
 *	Returns a standard Tcl return value.  If an error occurs then
 *	a message is left in interp->result.
 *
 * Side effects:
 *	Elements of the associative array given by "varName" are modified.
 *
 *----------------------------------------------------------------------
 */

static int
StoreStatData(interp, varName, statPtr)
    Tcl_Interp *interp;			/* Interpreter for error reports. */
    char *varName;			/* Name of associative array variable
					 * in which to store stat results. */
    struct stat *statPtr;		/* Pointer to buffer containing
					 * stat data to store in varName. */
{
    char string[30];

    sprintf(string, "%ld", statPtr->st_dev);
    if (Tcl_SetVar2(interp, varName, "dev", string, TCL_LEAVE_ERR_MSG)
	    == NULL) {
	return TCL_ERROR;
    }
    sprintf(string, "%ld", statPtr->st_ino);
    if (Tcl_SetVar2(interp, varName, "ino", string, TCL_LEAVE_ERR_MSG)
	    == NULL) {
	return TCL_ERROR;
    }
    sprintf(string, "%ld", statPtr->st_mode);
    if (Tcl_SetVar2(interp, varName, "mode", string, TCL_LEAVE_ERR_MSG)
	    == NULL) {
	return TCL_ERROR;
    }
    sprintf(string, "%ld", statPtr->st_nlink);
    if (Tcl_SetVar2(interp, varName, "nlink", string, TCL_LEAVE_ERR_MSG)
	    == NULL) {
	return TCL_ERROR;
    }
    sprintf(string, "%ld", (long) statPtr->st_uid);
    if (Tcl_SetVar2(interp, varName, "uid", string, TCL_LEAVE_ERR_MSG)
	    == NULL) {
	return TCL_ERROR;
    }
    sprintf(string, "%ld", (long) statPtr->st_gid);
    if (Tcl_SetVar2(interp, varName, "gid", string, TCL_LEAVE_ERR_MSG)
	    == NULL) {
	return TCL_ERROR;
    }
    sprintf(string, "%ld", statPtr->st_size);
    if (Tcl_SetVar2(interp, varName, "size", string, TCL_LEAVE_ERR_MSG)
	    == NULL) {
	return TCL_ERROR;
    }
    sprintf(string, "%ld", statPtr->st_atime);
    if (Tcl_SetVar2(interp, varName, "atime", string, TCL_LEAVE_ERR_MSG)
	    == NULL) {
	return TCL_ERROR;
    }
    sprintf(string, "%ld", statPtr->st_mtime);
    if (Tcl_SetVar2(interp, varName, "mtime", string, TCL_LEAVE_ERR_MSG)
	    == NULL) {
	return TCL_ERROR;
    }
    sprintf(string, "%ld", statPtr->st_ctime);
    if (Tcl_SetVar2(interp, varName, "ctime", string, TCL_LEAVE_ERR_MSG)
	    == NULL) {
	return TCL_ERROR;
    }
    if (Tcl_SetVar2(interp, varName, "type",
	    GetFileType((int) statPtr->st_mode), TCL_LEAVE_ERR_MSG) == NULL) {
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * GetFileType --
 *
 *	Given a mode word, returns a string identifying the type of a
 *	file.
 *
 * Results:
 *	A static text string giving the file type from mode.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static char *
GetFileType(mode)
    int mode;
{
    if (S_ISREG(mode)) {
	return "file";
    } else if (S_ISDIR(mode)) {
	return "directory";
    } else if (S_ISCHR(mode)) {
	return "characterSpecial";
    } else if (S_ISBLK(mode)) {
	return "blockSpecial";
    } else if (S_ISFIFO(mode)) {
	return "fifo";
    } else if (S_ISLNK(mode)) {
	return "link";
    } else if (S_ISSOCK(mode)) {
	return "socket";
    }
    return "unknown";
}



/* FROM tcFileName.c.  Modified to only use UNIX versions */

static char * SplitUnixPath(char *path, Tcl_DString *bufPtr);

/*
 *----------------------------------------------------------------------
 *
 * Tcl_SplitPath --
 *
 *	Split a path into a list of path components.  The first element
 *	of the list will have the same path type as the original path.
 *
 * Results:
 *	Returns a standard Tcl result.  The interpreter result contains
 *	a list of path components.
 *	*argvPtr will be filled in with the address of an array
 *	whose elements point to the elements of path, in order.
 *	*argcPtr will get filled in with the number of valid elements
 *	in the array.  A single block of memory is dynamically allocated
 *	to hold both the argv array and a copy of the path elements.
 *	The caller must eventually free this memory by calling ckfree()
 *	on *argvPtr.  Note:  *argvPtr and *argcPtr are only modified
 *	if the procedure returns normally.
 *
 * Side effects:
 *	Allocates memory.
 *
 *----------------------------------------------------------------------
 */

void
Tcl_SplitPath(path, argcPtr, argvPtr)
    char *path;			/* Pointer to string containing a path. */
    int *argcPtr;		/* Pointer to location to fill in with
				 * the number of elements in the path. */
    char ***argvPtr;		/* Pointer to place to store pointer to array
				 * of pointers to path elements. */
{
    int i, size;
    char *p;
    Tcl_DString buffer;
    Tcl_DStringInit(&buffer);

    /*
     * Perform platform specific splitting.  These routines will leave the
     * result in the specified buffer.  Individual elements are terminated
     * with a null character.
     */

    p = SplitUnixPath(path, &buffer);

    /*
     * Compute the number of elements in the result.
     */

    size = Tcl_DStringLength(&buffer);
    *argcPtr = 0;
    for (i = 0; i < size; i++) {
	if (p[i] == '\0') {
	    (*argcPtr)++;
	}
    }
    
    /*
     * Allocate a buffer large enough to hold the contents of the
     * DString plus the argv pointers and the terminating NULL pointer.
     */

    *argvPtr = (char **) ckalloc((unsigned)
	    ((((*argcPtr) + 1) * sizeof(char *)) + size));

    /*
     * Position p after the last argv pointer and copy the contents of
     * the DString.
     */

    p = (char *) &(*argvPtr)[(*argcPtr) + 1];
    memcpy((VOID *) p, (VOID *) Tcl_DStringValue(&buffer), (size_t) size);

    /*
     * Now set up the argv pointers.
     */

    for (i = 0; i < *argcPtr; i++) {
	(*argvPtr)[i] = p;
	while ((*p++) != '\0') {}
    }
    (*argvPtr)[i] = NULL;

    Tcl_DStringFree(&buffer);
}

/*
 *----------------------------------------------------------------------
 *
 * SplitUnixPath --
 *
 *	This routine is used by Tcl_SplitPath to handle splitting
 *	Unix paths.
 *
 * Results:
 *	Stores a null separated array of strings in the specified
 *	Tcl_DString.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static char *
SplitUnixPath(path, bufPtr)
    char *path;			/* Pointer to string containing a path. */
    Tcl_DString *bufPtr;	/* Pointer to DString to use for the result. */
{
    int length;
    char *p, *elementStart;

    /*
     * Deal with the root directory as a special case.
     */

    if (path[0] == '/') {
	Tcl_DStringAppend(bufPtr, "/", 2);
	p = path+1;
    } else {
	p = path;
    }

    /*
     * Split on slashes.  Embedded elements that start with tilde will be
     * prefixed with "./" so they are not affected by tilde substitution.
     */

    for (;;) {
	elementStart = p;
	while ((*p != '\0') && (*p != '/')) {
	    p++;
	}
	length = p - elementStart;
	if (length > 0) {
	    if ((elementStart[0] == '~') && (elementStart != path)) {
		Tcl_DStringAppend(bufPtr, "./", 2);
	    }
	    Tcl_DStringAppend(bufPtr, elementStart, length);
	    Tcl_DStringAppend(bufPtr, "", 1);
	}
	if (*p++ == '\0') {
	    break;
	}
    }
    return Tcl_DStringValue(bufPtr);
}


/* FROM: tclIOCmd.c - ksh implementations of Exec, CreatePipeline */



/* MODIFIED Tcl_ExecCmd */

extern int Tksh_CreatePipeline(Tcl_Interp *interp, int argc, char *argv[],
        Tcl_DString *buffer);

int
Tcl_ExecCmd(dummy, interp, argc, argv)
    ClientData dummy;                   /* Not used. */
    Tcl_Interp *interp;                 /* Current interpreter. */
    int argc;                           /* Number of arguments. */
    char **argv;                        /* Argument strings. */
{
	Interp *iPtr = (Interp *) interp;
	int oldInterpType = iPtr->interpType, result = TCL_ERROR;
	Tcl_DString buffer;

	iPtr->interpType = INTERP_KSH;
	TkshSetListMode(INTERP_KSH);

	if (Tksh_CreatePipeline(interp, argc-1, argv+1, &buffer) == TCL_OK)
	{
		dprintf(("EXEC: %s%s%s\n", ".sh.result=$(",
			buffer.string, " 2>&1)"));
		result = Tcl_VarEval(interp, ".sh.result=$(", buffer.string,
			 ")", NULL);

		/* the following won't be needed when .sh.result is linked to
		   interp->result */

		Tcl_SetResult(interp, Tcl_GetVar2(interp, ".sh.result", NULL,
			TCL_GLOBAL_ONLY), TCL_VOLATILE);
	}

	Tcl_DStringFree(&buffer);
	iPtr->interpType = oldInterpType;
	TkshSetListMode(oldInterpType);
	return result;
}


int Tksh_CreatePipeline(Tcl_Interp *interp, int argc, char *argv[],
	Tcl_DString *buffer)
{
    int cmdCount;		/* Count of number of distinct commands
				 * found in argc/argv. */
    char *input = NULL;		/* If non-null, then this points to a
				 * string containing input data (specified
				 * via <<) to be piped to the first process
				 * in the pipeline. */
    int lastBar;
    int i;
    char *p;
    int skip=0;
    int useForStdErr=0;

    Tcl_DStringInit(buffer);

    cmdCount = 1;
    lastBar = -1;
    for (i = 0; i < argc; i++) {
	useForStdErr = 0;
	if ((argv[i][0] == '|') && (((argv[i][1] == 0))
		|| ((argv[i][1] == '&') && (argv[i][2] == 0)))) {
	    if ((i == (lastBar+1)) || (i == (argc-1))) {
		interp->result = "illegal use of | or |& in command";
		return -1;
	    }
	    lastBar = i;
	    cmdCount++;
	    if (argv[i][1]) {
		Tcl_DStringAppend(buffer, " 2>&1 |", -1);
		useForStdErr = 1;
	    } else {
		Tcl_DStringAppend(buffer, " 2>/dev/null |", -1);
	    }
	    if (input)
	    {
	        Tcl_DStringAppend(buffer, "\n", -1);
	        Tcl_DStringAppend(buffer, input, -1);
	        Tcl_DStringAppend(buffer, "\nTKSH_HUP\n", -1);
		input = NULL;
	    }
	    continue;
	} else if (argv[i][0] == '<') {
	    if (argv[i][1] == '<') {
		input = argv[i]+2;
		if (*input == 0) {
		    input = argv[i+1];
		    if (input == 0) {
			Tcl_AppendResult(interp, "can't specify \"", argv[i],
				"\" as last word in command", (char *) NULL);
			goto error;
		    }
		    skip=1;
		}
	        Tcl_DStringAppend(buffer, " <<\\TKSH_HUP", -1);
	    } else {
		input = 0;
	        Tcl_DStringAppend(buffer, " ", -1);
	        Tcl_DStringAppend(buffer, argv[i], -1);
	    }
	} else if (argv[i][0] == '>') {
	    char *rtype;
	    if (argv[i][1] == 0)
		Tcl_DStringAppend(buffer, " >|", -1);
	    else {
		if (argv[i][1] == '>') {
			p = argv[i] + 2;
			rtype = " >>";
		} else {
			p = argv[i] + 1;
			rtype = " >|";
		}
		if ( (!(*p)) && (! argv[i+1]))
			goto errorSpec;
		if (*p == '&') {
			p++;
			Tcl_DStringAppend(buffer, rtype, -1);
			if (*p)
				Tcl_DStringAppend(buffer, p, -1);
			else
			{
			   Tcl_DStringAppend(buffer, argv[i+1], -1);
			   skip = 1;
			}
			Tcl_DStringAppend(buffer, " 2>&1", -1);
	    		useForStdErr = 1;
		} else {
			Tcl_DStringAppend(buffer, " ", -1);
			Tcl_DStringAppend(buffer, argv[i], -1);
		}
	    }

	} else if ((argv[i][0] == '2') && (argv[i][1] == '>')) {
	    int atOk;

	    useForStdErr = 1;
	    p = argv[i] + 2;
	    if (*p == '>') {
		p++;
		atOk = 0;
	    } else {
		atOk = 1;
	    }
	  	if (atOk && (argv[i][2] == '@')) {
			/* Not fully handled... */
			if (strcmp(argv[i]+3, "stdout") == 0)
	    			Tcl_DStringAppend(buffer, " 2>&1", -1);
		} else {
			Tcl_DStringAppend(buffer, " ", -1);
			Tcl_DStringAppend(buffer, argv[i], -1);
		}
	} else {
	    Tcl_DStringAppendElement(buffer, argv[i]);
	    continue;
	}
	i += skip;
	skip=0;
        continue;
    }
    if (argc == 0) {
	interp->result =  "didn't specify command to execute";
	return -1;
    }
    if (input)
    {
        Tcl_DStringAppend(buffer, "\n", -1);
        Tcl_DStringAppend(buffer, input, -1);
        Tcl_DStringAppend(buffer, "\nTKSH_HUP\n", -1);
    }
    if (! useForStdErr)
	Tcl_DStringAppend(buffer, " 2>&1", -1);
    return TCL_OK;

    errorSpec:
	Tcl_AppendResult(interp, "can't specify \"",
		argv[i], "\" as last word in command", NULL);
    error:
	return -1;
}

#if 1

#ifndef SH_IOCOPROCESS
#define SH_IOCOPROCESS -2
#endif
extern Sfio_t *sh_iogetiop(int fd, int mode);

int TclCreatePipeline(Tcl_Interp *interp, int argc, char *argv[],
	int **pidPtr, Tcl_File *inPipePtr, Tcl_File *outPipePtr)
{
	Tcl_DString buffer;
	int fdIn = -1, fdOut = -1;
	Sfio_t *iop;
	int result;

	if (Tksh_CreatePipeline(interp, argc, argv, &buffer) != TCL_OK)
		return TCL_ERROR;

	/* Tcl_DStringAppend(buffer, " |&", -1); */
	result = Tcl_VarEval(interp, "#!ksh\n", buffer.string, "|&", NULL);
	if (result != TCL_OK)
		return TCL_ERROR;

	iop = sh_iogetiop(SH_IOCOPROCESS, SF_READ);
	if (outPipePtr)
	{
		fdOut = fcntl(sffileno(iop), F_DUPFD,10);
		*outPipePtr = Tcl_GetFile((ClientData) fdOut, TCL_UNIX_FD);
	}
	iop = sh_iogetiop(SH_IOCOPROCESS, SF_WRITE);
	if (inPipePtr)
	{
		fdIn = fcntl(sffileno(iop), F_DUPFD,10);
		*inPipePtr = Tcl_GetFile((ClientData) fdIn, TCL_UNIX_FD);
	}
	*pidPtr = (int *) malloc(sizeof(int));
	**pidPtr = 999;
	fprintf(stderr, "%s : %d %d\n", buffer.string, fdIn, fdOut);
	sh_trap("exec 3<&p 3<&- 4>&p 4>&-",0);
	return 1;
}
#endif

/************

Tcl_JoinPath
Tcl_GetPathType
TclGetExtension


**************/

char * TclGetExtension(char *name)
{
	char *p, *lastSep;
	lastSep = strrchr(name, '/');
	p = strrchr(name, '.');
	if ((p != NULL) && (lastSep != NULL) && (lastSep > p))
		p = NULL;
	/* Back up to the first period in a series of contiguous dots.*/
	if (p != NULL) {
		while ((p > name) && *(p-1) == '.') {
			p--;
		}
	}
	return p;
}

char *
Tcl_JoinPath(argc, argv, resultPtr)
    int argc;
    char **argv;
    Tcl_DString *resultPtr;     /* Pointer to previously initialized DString. */
{
    int oldLength, length, i;
    Tcl_DString buffer;
    char *p, *dest;

    Tcl_DStringInit(&buffer);
    oldLength = Tcl_DStringLength(resultPtr);

            for (i = 0; i < argc; i++) {
                p = argv[i];
                /*
                 * If the path is absolute, reset the result buffer.
                 * Consume any duplicate leading slashes or a ./ in
                 * front of a tilde prefixed path that isn't at the
                 * beginning of the path.
                 */

                if (*p == '/') {
                    Tcl_DStringSetLength(resultPtr, oldLength);
                    Tcl_DStringAppend(resultPtr, "/", 1);
                    while (*p == '/') {
                        p++;
                    }
                } else if (*p == '~') {
                    Tcl_DStringSetLength(resultPtr, oldLength);
                } else if ((Tcl_DStringLength(resultPtr) != oldLength)
                        && (p[0] == '.') && (p[1] == '/')
                        && (p[2] == '~')) {
                    p += 2;
                }

                if (*p == '\0') {
                    continue;
                }

                /*
                 * Append a separator if needed.
                 */

                length = Tcl_DStringLength(resultPtr);
                if ((length != oldLength)
                        && (Tcl_DStringValue(resultPtr)[length-1] != '/')) {
                    Tcl_DStringAppend(resultPtr, "/", 1);
                    length++;
                }

                /*
                 * Append the element, eliminating duplicate and trailing
                 * slashes.
                 */

                Tcl_DStringSetLength(resultPtr, (int) (length + strlen(p)));
                dest = Tcl_DStringValue(resultPtr) + length;
                for (; *p != '\0'; p++) {
                    if (*p == '/') {
                        while (p[1] == '/') {
                            p++;
                        }
                        if (p[1] != '\0') {
                            *dest++ = '/';
                        }
                    } else {
                        *dest++ = *p;
                    }
                }
                length = dest - Tcl_DStringValue(resultPtr);
                Tcl_DStringSetLength(resultPtr, length);
            }

    Tcl_DStringFree(&buffer);
    return Tcl_DStringValue(resultPtr);
}

Tcl_PathType Tcl_GetPathType(char *path)
{
	if ((path[0] != '/') && (path[0] != '~'))
		return TCL_PATH_RELATIVE;
	else
		return TCL_PATH_ABSOLUTE;
}
