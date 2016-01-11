/* 
 * tkUnixInit.c --
 *
 *	This file contains Unix-specific interpreter initialization
 *	functions.
 *
 * Copyright (c) 1995-1996 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * SCCS: @(#) tkUnixInit.c 1.13 96/08/22 09:23:05
 */

#if _PACKAGE_ast
#include <ast.h>
#endif

#include "tkInt.h"
#include "tkUnixInt.h"

/*
 * Default directory in which to look for libraries:
 */

static char defaultLibraryDir[PATH_MAX];

/*
 * The following string is the startup script executed in new
 * interpreters.  It looks on disk in several different directories
 * for a script "tk.tcl" that is compatible with this version
 * of Tk.  The tk.tcl script does all of the real work of
 * initialization.
 */

static char initScript[] =
"proc tkInit {} {\n\
    global tk_library tk_version tk_patchLevel env\n\
    rename tkInit {}\n\
    set dirs {}\n\
    if [info exists env(TK_LIBRARY)] {\n\
	lappend dirs $env(TK_LIBRARY)\n\
    }\n\
    lappend dirs $tk_library\n\
    lappend dirs [file dirname [info library]]/lib/tk$tk_version\n\
    set parentDir [file dirname [file dirname [info nameofexecutable]]] \n\
    lappend dirs $parentDir/lib/tk$tk_version\n\
    if [string match {*[ab]*} $tk_patchLevel] {\n\
	set lib tk$tk_patchLevel\n\
    } else {\n\
	set lib tk$tk_version\n\
    }\n\
    lappend dirs [file dirname $parentDir]/$lib/library\n\
    lappend dirs [file dirname [file dirname [info library]]]/$lib/library\n\
    lappend dirs $parentDir/library\n\
    foreach i $dirs {\n\
	set tk_library $i\n\
	if ![catch {uplevel #0 source $i/tk.tcl}] {\n\
	    return\n\
	}\n\
    }\n\
    set msg \"Can't find a usable tk.tcl in the following directories: \n\"\n\
    append msg \"    $dirs\n\"\n\
    append msg \"This probably means that Tk wasn't installed properly.\n\"\n\
    error $msg\n\
}\n\
tkInit";

/*
 *----------------------------------------------------------------------
 *
 * TkPlatformInit --
 *
 *	Performs Unix-specific interpreter initialization related to the
 *      tk_library variable.
 *
 * Results:
 *	Returns a standard Tcl result.  Leaves an error message or result
 *	in interp->result.
 *
 * Side effects:
 *	Sets "tk_library" Tcl variable, runs "tk.tcl" script.
 *
 *----------------------------------------------------------------------
 */

int
TkPlatformInit(interp)
    Tcl_Interp *interp;
{
    char *libDir;

    libDir = Tcl_GetVar(interp, "tk_library", TCL_GLOBAL_ONLY);
    if (libDir == NULL) {
	libDir = pathpath(LIB_DIR, "", PATH_EXECUTE|PATH_READ, defaultLibraryDir, sizeof(defaultLibraryDir));
	if (libDir == NULL)
		sfsprintf(defaultLibraryDir, sizeof(defaultLibraryDir), "/usr/local/%s", LIB_DIR);
	Tcl_SetVar(interp, "tk_library", defaultLibraryDir, TCL_GLOBAL_ONLY);
    }
    TkCreateXEventSource();
    return Tcl_Eval(interp, initScript);
}
