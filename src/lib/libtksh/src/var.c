#pragma prototyped
/* Routines for variables */

#include "tkshlib.h"
#include "nvextra.h"

#define TCL_TRACE_MASK	(TCL_TRACE_WRITES|TCL_TRACE_READS|TCL_TRACE_UNSETS)
#define TRACE_SIZE	(sizeof(Namfun_t) + sizeof(Tcl_Trace_Info))
#define ADISC_SIZE	(sizeof(Namfun_t) + sizeof(void *))

#define TKSH_ARRAY_OPEN		'('
#define TKSH_ARRAY_CLOSE	')'
#define PART1_NOT_PARSED	0x10000
#define OVAR_NOADD		0x1
#define OVAR_GETDISC		0x2
#define OVAR_UNSETDISC		0x4
#define OVAR_ARRAY		0x8	/* If array, must have part2 */
#define OVAR_MAKEDISC		0x10	/* Install array disc. if not there */

#define TKSH_TRACE_ELEMENT	0x400

#define trace_element(ti)	((ti)->flags & TKSH_TRACE_ELEMENT)
#define array_data(nf)		((TkshArrayInfo *) ((nf) + 1))
#define TkshClearArrayData(nv)	do {					\
				  TkshArrayInfo *a = TkshArrayData(nv);	\
				  if (a->clientData)			\
					TkshDeleteSearches(a);		\
				} while(0)

typedef struct Tcl_Trace_Info
{
	Tcl_VarTraceProc	*proc;
	int			flags;
	ClientData		clientData;
	Tcl_Interp		*interp;
	Namval_t		*nv;		/* Namval trace create with */
} Tcl_Trace_Info;

static int		tracesOff;

static void             VarErrMsg _ANSI_ARGS_((Tcl_Interp *interp,
                            char *part1, char *part2, char *operation,
                            char *reason));

static Namval_t *	TkshOpenVar(Tcl_Interp *interp, char **part1,
			char **part2, int flags, int options, char *msg);

static void		TkshClearTraces(Namval_t *nv);

static char *tksh_getval(Namval_t *nv, Namfun_t *nf);
static void tksh_putval(Namval_t *nv, const char *val, int flags, Namfun_t *nf);
static void tksh_arrputval(Namval_t *nv,const char *val,int flags,Namfun_t *nf);
static char *tksh_arrgetval(Namval_t *nv, Namfun_t *nf);
static void tksh_blockput(Namval_t *nv,const char *val,int flags,Namfun_t *nf);
static char *tksh_blockget(Namval_t *nv, Namfun_t *nf);

static Namdisc_t tksh_trace_putval =
{
	TRACE_SIZE, tksh_putval, NULL, NULL, NULL, NULL
};
static Namdisc_t tksh_trace_getval =
{
	TRACE_SIZE, NULL, tksh_getval, NULL, NULL, NULL, NULL
};
static Namdisc_t tksh_trace_all =
{
	TRACE_SIZE, tksh_putval, tksh_getval, NULL, NULL, NULL, NULL
};
static Namdisc_t tksh_trace_stop =
{
	sizeof(Namdisc_t), tksh_blockput, tksh_blockget, NULL, NULL, NULL, NULL
};
static Namdisc_t tksh_trace_array =
{
	ADISC_SIZE, tksh_arrputval, tksh_arrgetval, NULL, NULL, NULL
};

static Namval_t *current_namval;
static char *current_part2;
static char *put_result, *get_result;

static char *noSuchVar =        "no such variable";
static char *isArray =          "variable is array";
static char *needArray =        "variable isn't array";
static char *noSuchElement =    "no such element in array";
/* static char *danglingUpvar = "upvar refers to element in deleted array"; */
 

/*
 * TkshOpenVar -
 *
 * Opens a ksh variable.  Handles the tcl flags TCL_GLOBAL_ONLY and
 * TCL_LEAVE_ERR_MSG.
 *
 */

static Namval_t *TkshOpenVar(Tcl_Interp *interp, char **name1, char **name2,
			     int flags, int options, char *msg)
{
	Namval_t *namval;
	Hashtab_t *tree = NULL;
	char *errmsg, *openParen, *closeParen, *p, *part1=*name1, *part2=*name2;
	int nvflags = (options & OVAR_NOADD) ? NV_NOADD : 0;

	if (flags & TCL_GLOBAL_ONLY)
		tree = hashscope(sh.var_tree);	/* Use parent var tree */
	else
		nvflags |= NV_NOSCOPE;		/* Do not search parent tree */

	/* If name hasn't been parsed into array name and index, do it now. */
	openParen = closeParen = NULL;
	if (flags & PART1_NOT_PARSED)
	{
		part2 = NULL;
		for (p = part1; ; p++)
		{
			if (*p == 0)
				break;
			if (*p != TKSH_ARRAY_OPEN)
				continue;
			openParen = p;
			do p++; while (*p != '\0');
			p--;
			if (*p == TKSH_ARRAY_CLOSE)
			{
				closeParen = p;
				*openParen = 0;
				*closeParen = 0;
				part2 = openParen+1;
			}
			else
				openParen = NULL;
			break;
		}
	}

        if (! (namval = nv_open(part1, tree, nvflags | NV_NOASSIGN )) )
	{
		if (flags & TCL_LEAVE_ERR_MSG)
			VarErrMsg(interp, part1, part2, msg, noSuchVar);
		return NULL;
	}
	if (flags & PART1_NOT_PARSED)
		part1=nv_name(nv_open(part1,tree,nvflags|NV_NOASSIGN|NV_NOREF));

	/* (Read-only) null scalar: fail if no disc. set or part2 supplied */
	if ((nvflags & NV_NOADD) && ! nv_isarray(namval) && nv_isnull(namval)
		&& ( (!namval->nvfun) || part2))
			ov_return(noSuchVar);

	if (part2)  /* Array */
	{
		if (nvflags & NV_NOADD)		/* Read-only */
		{
			if ( ! nv_isarray(namval))
				ov_return(needArray);
			if (! nv_setsub(namval, part2))
			{
				if (! namval->nvfun)
					ov_return(noSuchElement);
				if (options & OVAR_GETDISC)
				{
					current_namval = namval; 
					current_part2 = part2;
					nv_getval(namval);
					current_namval = NULL;
					if (! nv_setsub(namval, part2))
						ov_return(noSuchElement);
				}
				else if (options & OVAR_UNSETDISC)
				{
					if (! nv_subnullf(namval))
						nv_unset(namval);
					ov_return(noSuchElement);
				}
			}
			/* Elements with disciplines succeed in case traces
			   create them */
		}
		else
		{
			if (! nv_isarray(namval))
			{
				if (! nv_isnull(namval))
					ov_return(needArray);
				/* nv_makearray(namval); */
				nv_setarray(namval, nv_associative);
			}
			if (! nv_putsub(namval, part2, ARRAY_ADD))
				ov_return(noSuchVar);
			if (nv_subnull(namval))
				TkshClearArrayData(namval);
			else if (options & OVAR_MAKEDISC)
				TkshArrayData(namval);
		}
		part2 = nv_getsub(namval);
	}
	else	/* Scalar */
	{
		if (nv_isnull(namval))
		{
			if (options & OVAR_GETDISC)
			{
				nv_getval(namval);
				if (nv_isnull(namval))
					ov_return(noSuchVar);
			}
			if (options & OVAR_UNSETDISC)
			{
				nv_unset(namval);  /* Invoke disc */
				ov_return(noSuchVar);
			}
		}
		if ((options & OVAR_ARRAY) && nv_isattr(namval, NV_ARRAY))
			ov_return(isArray);
	}
 cleanup:
	if (flags & PART1_NOT_PARSED)
	{
		*name1=part1;
		*name2=part2;
	}
	if (openParen) *openParen = TKSH_ARRAY_OPEN;
	if (closeParen) *closeParen = TKSH_ARRAY_CLOSE;
	return namval;

 scalar:	/* ov_return goes to here */
	if (flags & TCL_LEAVE_ERR_MSG)
		VarErrMsg(interp, part1, part2, msg, errmsg);
	nv_close(namval);
	namval=NULL; goto cleanup;  /* XX Might be able to do return NULL; */
}

Namval_t *TkshAccessVar(Tcl_Interp *interp, char *part1, char *part2)
{
	return TkshOpenVar(interp,&part1,&part2,0,OVAR_NOADD,"access");
}

void TkshTracesOff(void)
{
	tracesOff = 1;
}


/*
 * TkshArrayData -
 *
 * Given a Namval_t, if necessary installs the standard Tksh array discipline
 * at the end of the stack by going through the nvfun list (unfortunate). 
 * Returns the location of where array data can be stored.
 */

TkshArrayInfo *TkshArrayData(Namval_t *namval)
{
	Namfun_t *nf_arr, *fp;

	for (fp=namval->nvfun; fp; fp = fp->next)
	{
                if ((fp->disc == &tksh_trace_array))
			return array_data(fp);
	}

	nf_arr = (Namfun_t *) calloc(1, tksh_trace_array.dsize);
	nf_arr->next = NULL;
	nf_arr->disc = & tksh_trace_array;
	array_data(nf_arr)->clientData = NULL;

	nv_disc(namval, nf_arr, NV_LAST);

	return array_data(nf_arr);
}


/*
 * TkshUpVar -
 *
 * Creates a reference in the current scope to a variable in another
 * scope.
 */

int TkshUpVar(Tcl_Interp *interp, char *newname, char *part1,
		char *part2, Hashtab_t *scope)
{
	Namval_t *namval;

	if (hashscope(sh.var_tree) && nv_open(newname,sh.var_tree,NV_NOSCOPE|NV_NOADD))
	{
		Tcl_AppendResult(interp, "variable \"", newname,
                	"\" already exists", (char *) NULL);
		return TCL_ERROR;
	}
	if (part2)
	{
		part1 = sfprints("%s[%s]", part1, part2);
		part2 = NULL;
	}
	if (!(namval = TkshOpenVar(interp,&newname,&part2,0,0,"access")))
		return TCL_ERROR;
	nv_putval(namval,part1,NV_NOFREE);
	nv_setref(namval,scope,NV_NOREF);
	nv_close(namval);
	return TCL_OK;
}

static void TkshClearTraces(Namval_t *nv)
{
	Namfun_t *fp, *fpnext;
	Tcl_Trace_Info *traceinfo;

	dprintf(("Clearing traces\n"));

	for (fp=nv->nvfun; fp; fp = fpnext)
	{
		fpnext = fp->next;
                if ((fp->disc->getval != tksh_getval) &&
                    (fp->disc->putval != tksh_putval))
                {
			continue;
		}
		traceinfo = (Tcl_Trace_Info *) (fp + 1);
		if (! (traceinfo->nv))
			traceinfo = (Tcl_Trace_Info *) (traceinfo->clientData);
		traceinfo->flags |= TCL_TRACE_DESTROYED;
		if (nv_disc(nv, fp, NV_POP))
			free(fp);
	}
}

char *Tcl_SetVar2(Tcl_Interp *interp, char *part1, char *part2,
	char *newValue, int flags)
{
	Namval_t *namval;
	char *listValue;
	Namfun_t *nf;

	dprintf(("Tksh: Setting var %s[%s]\n", part1,part2? part2: ""));

        if (! (namval=TkshOpenVar(interp,&part1,&part2,flags,OVAR_ARRAY,"set")))
		return NULL;

	if (newValue == (char *) NULL)
		newValue = "";
	nf = namval->nvfun;
	if (flags & TCL_LIST_ELEMENT)
	{
		int listFlags, len = Tcl_ScanElement(newValue, &listFlags);

		if (len <= 0)
		{
			if (flags & TCL_LEAVE_ERR_MSG)
			      VarErrMsg(interp,part1,part2,"set","bad element");
			goto scalar;
		}

		listValue = (char *) malloc(len + 1);
		Tcl_ConvertElement(newValue, listValue, listFlags);
		newValue = listValue;
	}

	if (flags & TCL_APPEND_VALUE)
	{
		int offset;
		char *oldval, *oldres=interp->result;
		oldval = nv_getval(namval);

		/* The following is a bad hack to see if the trace returned an
		 * error.  Can't use get_result because that is always set.
		 */
                if (oldres != interp->result)
		{
			VarErrMsg(interp, part1, part2, "read", interp->result);
			goto scalar;
		}
		offset = stktell(stkstd);
		if (oldval)
			sfputr(stkstd, oldval, ((flags & TCL_LIST_ELEMENT) &&
			TclNeedSpace(oldval, oldval+strlen(oldval))) ? ' ':-1);
		sfputr(stkstd, newValue, 0);
		stkseek(stkstd, offset);
		newValue = stkptr(stkstd, offset);
	}

	if (newValue)
	{
		put_result = NULL;	/* Get around putval non-return val */
		nv_putval(namval, newValue, 0);
		if (put_result)		/* Error in set trace */
		{
			VarErrMsg(interp, part1, part2, "set", put_result);
			put_result = NULL;
			if (flags & TCL_LIST_ELEMENT)
				free (listValue);
			goto scalar;
		}
	}

	if (flags & TCL_LIST_ELEMENT)
		free (listValue);

	if (nf != namval->nvfun)
		return "";
	nv_getvalue(namval, newValue);	/* Get the value without traces */
	nv_close(namval);
	return newValue ? newValue : ""; 

  scalar:
	nv_close(namval);
	return NULL;
}



char *Tcl_GetVar2(Tcl_Interp *interp, char *part1, char *part2, int flags)
{
	Namval_t *namval;
	char *errmsg, *result = NULL;

	dprintf(("Tksh: Getting var %s[%s]\n", part1,part2? part2: ""));

	namval = TkshOpenVar(interp, &part1, &part2, flags,
			OVAR_ARRAY | OVAR_GETDISC | OVAR_NOADD, "read");

	if (! namval)
		return NULL;

	get_result = NULL;
	if ((result = nv_getval(namval)) == NULL)	/* Error in trace */
	{
		if (flags & TCL_LEAVE_ERR_MSG)
			VarErrMsg(interp, part1, part2, "read",
				get_result? get_result: noSuchVar);
		goto scalar;
	}

	nv_close(namval);
	if (part2 && !*result)
		ov_return(noSuchElement);
	return result;

  scalar:
	dprintf(("GetVar2 failed for %s[%s]\n", part1, part2));

	nv_close(namval);
	return NULL;
}


int Tcl_UnsetVar2(Tcl_Interp *interp, char *part1, char *part2, int flags)
{
	Namval_t *namval, *nvsub;
	Namfun_t *fp;

	dprintf(("Tksh: Unsetting var %s[%s]\n", part1,part2?part2:""));

	namval = TkshOpenVar(interp, &part1, &part2, flags,
		OVAR_NOADD | OVAR_UNSETDISC, "unset");
	if (!namval)
		return TCL_ERROR;

	fp = namval->nvfun;
	if (! part2)
	{
		if (nv_isattr(namval, NV_ARRAY))
			nv_putsub(namval, NULL, ARRAY_UNDEF);
		nv_unset(namval);
	}
	else
	{
		nvsub = nv_opensub(namval);
		nv_unset(namval);
		namval = nvsub;
	}

	if (fp == namval->nvfun)
		TkshClearTraces(namval);

	nv_close(namval);
	return TCL_OK;
}


/*
 * tksh_arrputval -
 *
 * This function is the last discipline called for an array.
 * It invokes the disciplines set on individual elements of the array.
 * During a whole array unset, this function is called only once, with
 * the first subscript selected in scan mode.
 */

static void
tksh_arrputval(Namval_t *nv, const char *val, int flags, Namfun_t *nf)
{
	Namval_t *nvsub = nv_opensub(nv), nvcpy;

	memset(&nvcpy, 0, sizeof(nvcpy));
	if (! nvsub)			/* Don't handle whole array put */
	{
#if 1 /* this hacks around a double-free interaction with nv_move() below */
		nv->nvalue = 0;
#endif
		nv_putv(nv, val, flags, nf);
		return;
	}

	dprintf(("Tksh: put on array element %s[%s]\n", nv_name(nv),
		nvsub ? nv_name(nvsub) : "all"));

	if (val)			/* Set */
	{
		nv_putv(nv, val, flags, nf);
		if ( (! nv_isattr(nv, NV_NODISC)) && (nvsub->nvfun))
			nv_putval(nvsub, val, flags);
		return;
	}
	if (array_data(nf)->clientData)
		TkshDeleteSearches(array_data(nf));
	if (! nv_inscan(nv))		/* Array element unset  */
	{
		nv_putv(nv, val, flags, nf);
		if (nvsub->nvfun)
			nv_putval(nvsub, val, flags);
		return;
	}
	/* Whole array unset */
	nv = nv_move(nv, &nvcpy); /* Copy because array must appear gone */
	do	/* call traces in each array element */
	{
		if ((nvsub = nv_opensub(nv)))
			nv_unset(nvsub);
	}
	while (nv_nextsub(nv) != 0);
	nv_putv(nv, val, flags, nf);		/* Free array */
	nv_putsub(nv, NULL, ARRAY_UNDEF);	/* End scan */
	TkshClearTraces(nv);
}


static char *tksh_arrgetval(Namval_t *nv, Namfun_t *nf)
{
	Namval_t *nvsub = nv_opensub(nv);

	if ((!nvsub) || nv_isattr(nv, NV_NODISC))
		return nv_getv(nv, nf);	

	dprintf(("Tksh: get on array element %s[%s]\n",
		nv_name(nv), nv_name(nvsub)));

	return nv_getval(nvsub);
}

/* Override other set discipline functions */
static void tksh_blockput(Namval_t *nv,const char *val,int flags,Namfun_t *nf)
{
	if (val)
		nv_putvalue(nv, val, flags);	/* set */
	else
		nv_putv(nv, val, flags, nf);	/* unset */
}

/* Override other get discipline functions */
static char *tksh_blockget(Namval_t *nv, Namfun_t *nf)
{
	char *val;
	nv_getvalue(nv, val);
	return val;
}


static void tksh_putval(Namval_t *nv, const char *val, int flags, Namfun_t *nf)
{
	char *arrayname, *subname=NULL, *subcpy=NULL, *result;
	Tcl_Trace_Info trace, *traceinfo;
	int traceFlags;

	if (tracesOff || (val && nv_isattr(nv, NV_NODISC)))
	{
		nv_putv(nv, val, flags, nf);
		return;
	}

	traceinfo = (Tcl_Trace_Info *) (nf + 1);
	traceFlags = (traceinfo->flags & TCL_GLOBAL_ONLY)
			| (val ? TCL_TRACE_WRITES : TCL_TRACE_UNSETS);
	if (! (traceinfo->flags & traceFlags & TCL_TRACE_MASK))
	{
		nv_putv(nv, val, flags, nf);
		return;			/* Not a trace - call next disc. */
	}

	dprintf(("Tksh: invoking tksh_putval (%s)\n", val?val:"unset"));

	trace = traceinfo->nv ? *traceinfo :
		(*((Tcl_Trace_Info *) traceinfo->clientData));

	if (trace_element(traceinfo))	/* Trace on array element */
	{
		arrayname = nv_name(trace.nv);
		subname = nv_name(nv);
	}
	else			/* Trace on scalar or whole array */
	{
		arrayname = nv_name(nv);
		if (nv_getsub(nv) && ! nv_inscan(nv))
			subname = subcpy = strdup(nv_getsub(nv));
	}

	if (val)		/* Set */
	{
		nv_onattr(nv, NV_NODISC); nv_stopdisc(nv);
		nv_putval(nv, val, flags);
		traceinfo->nv = NULL;
		traceinfo->clientData = &trace;
		result = trace.proc(trace.clientData,
		     trace.interp, arrayname, subname, traceFlags);
		if (! (trace.flags & TCL_TRACE_DESTROYED))
		{
			val = nv_getval(nv);
			nv_offattr(nv, NV_NODISC);  nv_resumedisc(nv);
			traceinfo->nv = trace.nv;
			traceinfo->clientData = trace.clientData;
			if (val && !result) 	/* No error from set */
				nv_putv(nv, val, flags, nf);
		}
	}
	else			/* Unset */
	{
		if (trace_element(traceinfo) || !nv_isarray(nv))
		{
			traceinfo->flags |= TCL_TRACE_DESTROYED;
			traceFlags |= TCL_TRACE_DESTROYED;
		}
		nv_putv(nv, val, flags, nf);
		if (trace_element(&trace) || !nv_isarray(nv))
			TkshClearTraces(nv);
		result = trace.proc(trace.clientData, trace.interp,
			arrayname, subname, traceFlags);
	}
	if ((put_result = (val ? result: NULL)))
		Tcl_SetResult(trace.interp, result, TCL_STATIC);
	if (subcpy)
		free (subcpy);
	return;
}

static char *tksh_getval(Namval_t *nv, Namfun_t *nf)
{
	char *result, *part1, *part2;
	Tcl_Trace_Info *traceinfo, trace;
	int traceFlags;
	Fcin_t save;

	if (tracesOff || nv_isattr(nv, NV_NODISC))
		return nv_getv(nv, nf);

	traceinfo = (Tcl_Trace_Info *) (nf + 1);
	if (traceinfo->flags & TCL_TRACE_DESTROYED)
		return nv_getv(nv, nf);
	trace = traceinfo->nv ? *traceinfo :
		(*((Tcl_Trace_Info *) traceinfo->clientData));
 	traceFlags = (trace.flags & TCL_GLOBAL_ONLY) | TCL_TRACE_READS;
	if (trace_element(traceinfo))		/* Trace on array element */
	{
		part1 = nv_name(trace.nv);
		part2 = nv_name(nv);
	}
	else				/* Trace on scalar or whole array */
	{
		part1 = nv_name(nv);
		part2 = (current_namval == nv) ? current_part2 : nv_getsub(nv);
		/* No subscript is set for a null array element,
		   so use global variable current_part2 */
	}
	current_namval = NULL;

	dprintf(("Tksh: invoking getval disc on %s[%s]\n", part1,part2));

	fcsave(&save);
	nv_onattr(nv, NV_NODISC); nv_stopdisc(nv);
	traceinfo->nv = NULL;
	traceinfo->clientData = &trace;
	result = traceinfo->proc(trace.clientData, trace.interp,
		 part1, part2, traceFlags);
	fcrestore(&save);

	if (!(trace.flags & TCL_TRACE_DESTROYED))
	{
		traceinfo->nv = trace.nv;
		traceinfo->clientData = trace.clientData;
	}

	if ((get_result = result))
	{
		Tcl_SetResult(trace.interp, result, TCL_STATIC);
		return NULL;
	}

	if (trace.flags & TCL_TRACE_DESTROYED)
	{
		get_result = (trace_element(&trace) && (trace.nv)->nvalue)?
			noSuchElement: noSuchVar;
		return (trace_element(&trace) && ! (trace.nv)->nvalue) ?
			NULL : (char *) nv->nvalue;
	}
	get_result = (trace_element(traceinfo)&&traceinfo->nv->nvalue)?
		noSuchElement: noSuchVar;
	nv_offattr(nv, NV_NODISC); nv_resumedisc(nv);
	return nv_getv(nv, nf);
}


int Tcl_TraceVar2(Tcl_Interp *interp, char *part1, char *part2, int flags,
	Tcl_VarTraceProc *proc, ClientData clientData)
{
	Namval_t *namval, *nvsub;
	Namfun_t *nf;
	Tcl_Trace_Info *traceinfo;

	if (! (namval = TkshOpenVar(interp, &part1, &part2,
		flags | TCL_LEAVE_ERR_MSG, OVAR_MAKEDISC, "trace")) )
	{
		/* VarErrMsg(interp, part1, part2, "trace", needArray); */
		return TCL_ERROR;
	}

	if (part2 && ! (nvsub = nv_opensub(namval)))
		goto scalar;

	if (! (nf = (Namfun_t *) calloc(1, TRACE_SIZE)))
		goto scalar;

	traceinfo = (Tcl_Trace_Info *) (nf + 1);
	traceinfo->clientData = clientData;
	traceinfo->flags = flags;
	traceinfo->proc = proc;
	traceinfo->interp = interp;
	traceinfo->nv = namval;

	if ((flags & TCL_TRACE_READS) &&
	   ((flags & TCL_TRACE_WRITES) || (flags & TCL_TRACE_UNSETS)))
		nf->disc = & tksh_trace_all;
	else if (flags & TCL_TRACE_READS)
		nf->disc = & tksh_trace_getval;
	else 
		nf->disc = & tksh_trace_putval;

	if (part2)		/* Trace array element */
	{
		traceinfo->flags |= TKSH_TRACE_ELEMENT;
		nv_stack(nvsub, nf);
		dprintf(("Tksh: Trace set for array element %s[%s]\n",
			nv_name(namval), nv_name(nvsub) ));
	}
	else			/* Trace scalar or whole array */
	{
		nv_stack(namval, nf);
		dprintf(("Tksh: Trace set for var %s\n", part1));
	}
	nv_close(namval);
	return TCL_OK;

scalar:
	nv_close(namval);
	return TCL_ERROR;
}


/* XX Alternative: #define first_namfun(nv) (nv->nvfun) */
static Namfun_t *first_namfun(Namval_t *nv)
{
	Namfun_t *first;
	first = nv_stack(nv, (Namfun_t *) NULL);
	nv_stack(nv, first);

	return first;
}

#define share_flag(x, y, f) ( ((x) & f) == ((y) & f) )
static Namfun_t *find_namfun(Namval_t *nv, int flags, Tcl_VarTraceProc *proc,
	ClientData clientData)
{
	Tcl_Trace_Info *traceinfo;
	Namfun_t *first = first_namfun(nv), *node = first;

	while (node)
	{
		traceinfo = (Tcl_Trace_Info *) (node + 1);
		if ((traceinfo->proc == proc) &&
		     share_flag(traceinfo->flags, flags, TCL_GLOBAL_ONLY) &&
		    (traceinfo->clientData == clientData))
		{
			return node;
		}
		node = node->next;
	}
	return NULL;
}

void Tcl_UntraceVar2(Tcl_Interp *interp, char *name1, char *name2,
	int flags, Tcl_VarTraceProc *proc, ClientData clientData)
{
	Namfun_t *nf;
	Namval_t *namval;

	dprintf(("Tksh: Untracing var %s[%s]\n", name1,name2?name2:""));

	namval = TkshOpenVar(interp,&name1,&name2,flags,OVAR_NOADD,"untrace");
	if (!namval)
		goto scalar;

	if (name2 && ! (namval = nv_opensub(namval)) )
		goto scalar;

	if ((nf = find_namfun(namval, flags, proc, clientData)))
		nv_disc(namval, nf, NV_POP);
	return;

  scalar:
	dprintf(("Tksh: Untrace failed\n"));
	return;
}


ClientData Tcl_VarTraceInfo2(Tcl_Interp *interp, char *name1, char *name2,
	int flags, Tcl_VarTraceProc *proc, ClientData clientData)
{
	Namfun_t *nf;
	Namval_t *namval;
	Tcl_Trace_Info *traceinfo;
	int flag = (clientData == NULL);

	if (! (namval=TkshOpenVar(interp,&name1,&name2, flags,OVAR_NOADD,NULL)))
		return NULL;

	/* XX check if nv_isnull(namval) if this should fail for
	   vars with traces set and no value */

	if (name2 && ! (namval = nv_opensub(namval)))
		return NULL;

	nf = first_namfun(namval);
	while (nf)
	{
		traceinfo = (Tcl_Trace_Info *) (nf + 1);
		if ((traceinfo->proc == proc) && 
		     share_flag(traceinfo->flags, flags, TCL_GLOBAL_ONLY))
		{
			if (flag)
			{
				nv_close(namval);
				return traceinfo->clientData;
			}
			if (traceinfo->clientData == clientData)
				flag = 1;
		}
		nf = nf->next;
	}
	nv_close(namval);
	return NULL;
}


char *Tcl_SetVar(Tcl_Interp *interp, char *varName, char *newValue, int flags)
{
	return Tcl_SetVar2(interp, varName, NULL, newValue,
		flags | PART1_NOT_PARSED);
}
char *Tcl_GetVar(Tcl_Interp *interp, char *varName, int flags)
{
	return Tcl_GetVar2(interp, varName, NULL, flags | PART1_NOT_PARSED);
}
int Tcl_TraceVar(Tcl_Interp * interp, char *varName, int flags,
	Tcl_VarTraceProc *proc, ClientData clientData)
{
	return Tcl_TraceVar2(interp,varName,NULL,flags|PART1_NOT_PARSED,
		proc,clientData);
}
int Tcl_UnsetVar(Tcl_Interp *interp, char *varName, int flags)
{
	return Tcl_UnsetVar2(interp,varName,NULL,flags|PART1_NOT_PARSED);
}
void Tcl_UntraceVar(Tcl_Interp * interp, char *varName, int flags,
	Tcl_VarTraceProc *proc, ClientData clientData)
{
	Tcl_UntraceVar2(interp,varName,NULL,flags|PART1_NOT_PARSED,
		proc,clientData);
}
ClientData Tcl_VarTraceInfo(Tcl_Interp *interp, char *varName, int flags,
	Tcl_VarTraceProc *proc, ClientData prevClientData)
{
	return Tcl_VarTraceInfo2(interp,varName,NULL,flags|PART1_NOT_PARSED,
		proc,prevClientData);
}


/*
 *----------------------------------------------------------------------
 *
 * VarErrMsg --
 *
 *	Generate a reasonable error message describing why a variable
 *	operation failed.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Interp->result is reset to hold a message identifying the
 *	variable given by part1 and part2 and describing why the
 *	variable operation failed.
 *
 *----------------------------------------------------------------------
 */

static void
VarErrMsg(interp, part1, part2, operation, reason)
    Tcl_Interp *interp;		/* Interpreter in which to record message. */
    char *part1, *part2;	/* Variable's two-part name. */
    char *operation;		/* String describing operation that failed,
				 * e.g. "read", "set", or "unset". */
    char *reason;		/* String describing why operation failed. */
{
    Tcl_ResetResult(interp);
    Tcl_AppendResult(interp, "can't ", operation, " \"", part1, (char *) NULL);
    if (part2 != NULL) {
	Tcl_AppendResult(interp, "(", part2, ")", (char *) NULL);
    }
    Tcl_AppendResult(interp, "\": ", reason, (char *) NULL);
}
