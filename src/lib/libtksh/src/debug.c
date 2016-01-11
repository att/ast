#pragma prototyped
#ifndef DEBUG

int _STUB_debug;	/* Some compilers require a statement in the file */

#else

#include "tkshlib.h"
#include <ast.h>

static int debugLevel, debugInitialized;
static void dinit(void);
static void printState(char *file, int line);
static Sfio_t *outFile;
static Interp *debugInterp;

int __dprintfLevel()
{
	return debugLevel;
}

void __dprintfInterp(Tcl_Interp *interp)
{
	debugInterp = (Interp *) interp;
}

int __dprintfOK(char *file, int line, int level)
{
	if (!debugInitialized)
                dinit();
	if (debugLevel < level)
		return 0;
	printState(file, line);
	return 1;
}

void __dprintf(char *format, char *a1, char *a2, char *a3, char *a4,
	char *a5, char *a6, char *a7, char *a8, char *a9, char *a10)
{
	sfprintf(outFile, format, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10);
}

void __dprintfNum(char *before, int num, char *after)
{
	sfprintf(outFile, "%s%d%s", before,num,after);
}

void __dprintfArgs(char *string, int argc, char *argv[])
{
	int arg;
	sfprintf(outFile, string);
	if (argc)
	{
		for (arg=0; arg < argc; arg++)
			sfprintf(outFile, " <%.10s>", argv[arg]);
	}
	else
	{
		for (arg=0; argv[arg]; arg++)
			sfprintf(outFile, " <%.10s>", argv[arg]);
	}
	sfprintf(outFile, "\n");
}

static void dinit()
{
	char *lev;

	debugInitialized = 1;
	if ((lev = getenv("JDEBUG")))
	{
		debugLevel = atoi(lev);
		if (debugLevel == 0)
			debugLevel = 1;
		/* outFile = sfopen(NIL(Sfio_t *), "goober", "w+"); */
		outFile = sfstderr;
		sfprintf(outFile,"Debug routines initialized,compiled %s %s\n",
			__DATE__, __TIME__);
	}
}

static void printState(char *file, int line)
{
	sfprintf(outFile, "[%s]%s:%d: ",
		Tksh_InterpString(debugInterp->interpType), file, line);
}


#endif
