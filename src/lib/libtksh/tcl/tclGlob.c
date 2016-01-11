/* 
 * tclGlob.c --
 *
 *	This file provides procedures and commands for file name
 *	manipulation, such as tilde expansion and globbing.
 *
 * Copyright (c) 1990-1994 The Regents of the University of California.
 * Copyright (c) 1994 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

#ifndef lint
static char sccsid[] = "@(#) tclGlob.c 1.42 95/06/08 10:56:13";
#endif /* not lint */

#include "tclInt.h"
#include "tclPort.h"

/*
 * Declarations for procedures local to this file:
 */

static int		DoGlob _ANSI_ARGS_((Tcl_Interp *interp, char *dir,
			    char *rem));

/*
 *----------------------------------------------------------------------
 *
 * DoGlob --
 *
 *	This recursive procedure forms the heart of the globbing
 *	code.  It performs a depth-first traversal of the tree
 *	given by the path name to be globbed.
 *
 * Results:
 *	The return value is a standard Tcl result indicating whether
 *	an error occurred in globbing.  After a normal return the
 *	result in interp will be set to hold all of the file names
 *	given by the dir and rem arguments.  After an error the
 *	result in interp will hold an error message.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
DoGlob(interp, dir, rem)
    Tcl_Interp *interp;			/* Interpreter to use for error
					 * reporting (e.g. unmatched brace). */
    char *dir;				/* Name of a directory at which to
					 * start glob expansion.  This name
					 * is fixed: it doesn't contain any
					 * globbing chars. */
    char *rem;				/* Path to glob-expand. */
{
    /*
     * When this procedure is entered, the name to be globbed may
     * already have been partly expanded by ancestor invocations of
     * DoGlob.  The part that's already been expanded is in "dir"
     * (this may initially be empty), and the part still to expand
     * is in "rem".  This procedure expands "rem" one level, making
     * recursive calls to itself if there's still more stuff left
     * in the remainder.
     */

    Tcl_DString newName;		/* Holds new name consisting of
					 * dir plus the first part of rem. */
    register char *p;
    register char c;
    char *openBrace, *closeBrace, *name, *dirName;
    int gotSpecial, baseLength;
    int result = TCL_OK;
    struct stat statBuf;

    /*
     * Make sure that the directory part of the name really is a
     * directory.  If the directory name is "", use the name "."
     * instead, because some UNIX systems don't treat "" like "."
     * automatically. Keep the "" for use in generating file names,
     * otherwise "glob foo.c" would return "./foo.c".
     */

    if (*dir == '\0') {
	dirName = ".";
    } else {
	dirName = dir;
    }
    if ((stat(dirName, &statBuf) != 0) || !S_ISDIR(statBuf.st_mode)) {
	return TCL_OK;
    }
    Tcl_DStringInit(&newName);

    /*
     * First, find the end of the next element in rem, checking
     * along the way for special globbing characters.
     */

    gotSpecial = 0;
    openBrace = closeBrace = NULL;
    for (p = rem; ; p++) {
	c = *p;
	if ((c == '\0') || ((openBrace == NULL) && (c == '/'))) {
	    break;
	}
	if ((c == '{') && (openBrace == NULL)) {
	    openBrace = p;
	}
	if ((c == '}') && (openBrace != NULL) && (closeBrace == NULL)) {
	    closeBrace = p;
	}
	if ((c == '*') || (c == '[') || (c == '\\') || (c == '?')) {
	    gotSpecial = 1;
	}
    }

    /*
     * If there is an open brace in the argument, then make a recursive
     * call for each element between the braces.  In this case, the
     * recursive call to DoGlob uses the same "dir" that we got.
     * If there are several brace-pairs in a single name, we just handle
     * one here, and the others will be handled in recursive calls.
     */

    if (openBrace != NULL) {
	char *element;

	if (closeBrace == NULL) {
	    Tcl_ResetResult(interp);
	    interp->result = "unmatched open-brace in file name";
	    result = TCL_ERROR;
	    goto done;
	}
	Tcl_DStringAppend(&newName, rem, openBrace-rem);
	baseLength = newName.length;
	for (p = openBrace; *p != '}'; ) {
	    element = p+1;
	    for (p = element; ((*p != '}') && (*p != ',')); p++) {
		/* Empty loop body. */
	    }
	    Tcl_DStringAppend(&newName, element, p-element);
	    Tcl_DStringAppend(&newName, closeBrace+1, -1);
	    result = DoGlob(interp, dir, newName.string);
	    if (result != TCL_OK) {
		goto done;
	    }
	    newName.length = baseLength;
	}
	goto done;
    }

    /*
     * Start building up the next-level name with dir plus a slash if
     * needed to separate it from the next file name.
     */

    Tcl_DStringAppend(&newName, dir, -1);
    if ((dir[0] != 0) && (newName.string[newName.length-1] != '/')) {
	Tcl_DStringAppend(&newName, "/", 1);
    }
    baseLength = newName.length;

    /*
     * If there were any pattern-matching characters, then scan through
     * the directory to find all the matching names.
     */

    if (gotSpecial) {
	DIR *d;
	struct dirent *entryPtr;
	char savedChar;

	d = opendir(dirName);
	if (d == NULL) {
	    Tcl_ResetResult(interp);
	    Tcl_AppendResult(interp, "couldn't read directory \"",
		    dirName, "\": ", Tcl_PosixError(interp), (char *) NULL);
	    result = TCL_ERROR;
	    goto done;
	}

	/*
	 * Temporarily store a null into rem so that the pattern string
	 * is now null-terminated.
	 */

	savedChar = *p;
	*p = 0;

	while (1) {
	    entryPtr = readdir(d);
	    if (entryPtr == NULL) {
		break;
	    }

	    /*
	     * Don't match names starting with "." unless the "." is
	     * present in the pattern.
	     */

	    if ((*entryPtr->d_name == '.') && (*rem != '.')) {
		continue;
	    }
	    if (Tcl_StringMatch(entryPtr->d_name, rem)) {
		newName.length = baseLength;
		Tcl_DStringAppend(&newName, entryPtr->d_name, -1);
		if (savedChar == 0) {
		    Tcl_AppendElement(interp, newName.string);
		} else {
		    result = DoGlob(interp, newName.string, p+1);
		    if (result != TCL_OK) {
			break;
		    }
		}
	    }
	}
	closedir(d);
	*p = savedChar;
	goto done;
    }

    /*
     * The current element is a simple one with no fancy features.  Add
     * it to the new name.  If there are more elements still to come,
     * then recurse to process them.
     */

    Tcl_DStringAppend(&newName, rem, p-rem);
    if (*p != 0) {
	result = DoGlob(interp, newName.string, p+1);
	goto done;
    }

    /*
     * There are no more elements in the pattern.  Check to be sure the
     * file actually exists, then add its name to the list being formed
     * in interp-result.
     */

    name = newName.string;
    if (*name == 0) {
	name = ".";
    }
    if (access(name, F_OK) != 0) {
	goto done;
    }
    Tcl_AppendElement(interp, name);

    done:
    Tcl_DStringFree(&newName);
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_TildeSubst --
 *
 *	Given a name starting with a tilde, produce a name where
 *	the tilde and following characters have been replaced by
 *	the home directory location for the named user.
 *
 * Results:
 *	The result is a pointer to a static string containing
 *	the new name.  If there was an error in processing the
 *	tilde, then an error message is left in interp->result
 *	and the return value is NULL.  The result may be stored
 *	in bufferPtr; the caller must call Tcl_DStringFree(bufferPtr)
 *	to free the name.
 *
 * Side effects:
 *	Information may be left in bufferPtr.
 *
 *----------------------------------------------------------------------
 */

char *
Tcl_TildeSubst(interp, name, bufferPtr)
    Tcl_Interp *interp;		/* Interpreter in which to store error
				 * message (if necessary). */
    char *name;			/* File name, which may begin with "~/"
				 * (to indicate current user's home directory)
				 * or "~<user>/" (to indicate any user's
				 * home directory). */
    Tcl_DString *bufferPtr;	/* May be used to hold result.  Must not hold
				 * anything at the time of the call, and need
				 * not even be initialized. */
{
    char *dir;
    register char *p;

    Tcl_DStringInit(bufferPtr);
    if (name[0] != '~') {
	return name;
    }

    if ((name[1] == '/') || (name[1] == '\0')) {
	dir = getenv("HOME");
	if (dir == NULL) {
	    Tcl_ResetResult(interp);
	    Tcl_AppendResult(interp, "couldn't find HOME environment ",
		    "variable to expand \"", name, "\"", (char *) NULL);
	    return NULL;
	}
	Tcl_DStringAppend(bufferPtr, dir, -1);
	Tcl_DStringAppend(bufferPtr, name+1, -1);
    } else {
	struct passwd *pwPtr;

	for (p = &name[1]; (*p != 0) && (*p != '/'); p++) {
	    /* Null body;  just find end of name. */
	}
	Tcl_DStringAppend(bufferPtr, name+1, p - (name+1));
	pwPtr = getpwnam(bufferPtr->string);
	if (pwPtr == NULL) {
	    endpwent();
	    Tcl_ResetResult(interp);
	    Tcl_AppendResult(interp, "user \"", bufferPtr->string,
		    "\" doesn't exist", (char *) NULL);
	    Tcl_DStringFree(bufferPtr);
	    return NULL;
	}
	Tcl_DStringFree(bufferPtr);
	Tcl_DStringAppend(bufferPtr, pwPtr->pw_dir, -1);
	Tcl_DStringAppend(bufferPtr, p, -1);
	endpwent();
    }
    return bufferPtr->string;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_GlobCmd --
 *
 *	This procedure is invoked to process the "glob" Tcl command.
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
Tcl_GlobCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    int i, result, noComplain, firstArg;

    if (argc < 2) {
	notEnoughArgs:
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" ?switches? name ?name ...?\"", (char *) NULL);
	return TCL_ERROR;
    }
    noComplain = 0;
    for (firstArg = 1; (firstArg < argc) && (argv[firstArg][0] == '-');
	    firstArg++) {
	if (strcmp(argv[firstArg], "-nocomplain") == 0) {
	    noComplain = 1;
	} else if (strcmp(argv[firstArg], "--") == 0) {
	    firstArg++;
	    break;
	} else {
	    Tcl_AppendResult(interp, "bad switch \"", argv[firstArg],
		    "\": must be -nocomplain or --", (char *) NULL);
	    return TCL_ERROR;
	}
    }
    if (firstArg >= argc) {
	goto notEnoughArgs;
    }

    for (i = firstArg; i < argc; i++) {
	char *thisName;
	Tcl_DString buffer;

	thisName = Tcl_TildeSubst(interp, argv[i], &buffer);
	if (thisName == NULL) {
	    if (noComplain) {
		Tcl_ResetResult(interp);
		continue;
	    } else {
		return TCL_ERROR;
	    }
	}
	if (*thisName == '/') {
	    if (thisName[1] == '/') {
		/*
		 * This is a special hack for systems like those from Apollo
		 * where there is a super-root at "//":  need to treat the
		 * double-slash as a single name.
		 */
		result = DoGlob(interp, "//", thisName+2);
	    } else {
		result = DoGlob(interp, "/", thisName+1);
	    }
	} else {
	    result = DoGlob(interp, "", thisName);
	}
	Tcl_DStringFree(&buffer);
	if (result != TCL_OK) {
	    return result;
	}
    }
    if ((*interp->result == 0) && !noComplain) {
	char *sep = "";

	Tcl_AppendResult(interp, "no files matched glob pattern",
		(argc == 2) ? " \"" : "s \"", (char *) NULL);
	for (i = firstArg; i < argc; i++) {
	    Tcl_AppendResult(interp, sep, argv[i], (char *) NULL);
	    sep = " ";
	}
	Tcl_AppendResult(interp, "\"", (char *) NULL);
	return TCL_ERROR;
    }
    return TCL_OK;
}
