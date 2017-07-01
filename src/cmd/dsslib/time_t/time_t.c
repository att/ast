/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2011 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                 Eclipse Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*          http://www.eclipse.org/org/documents/epl-v10.html           *
*         (with md5 checksum b35adb5213ca9657e911e9befb180842)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * dss time type library
 *
 * Glenn Fowler
 * AT&T Research
 */

#include <dsslib.h>
#include <tmx.h>

#define NS			1000000000

#if TMX_FLOAT
#define SS			4294967296.0
#endif

typedef struct Precise_s
{
	const char*		format;
	size_t			size;
	int			shift;
} Precise_t;

static ssize_t
time_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	char*	s;
	time_t	t;

	if (!size)
		return 40;
	t = value->number;
	s = tmfmt(buf, size, CXDETAILS(details, format, type, "%K"), &t);
	if (s == (buf + size - 1))
		return 2 * size;
	return s - buf;
}

static ssize_t
time_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	char*	e;
	char*	f;

	buf = (const char*)cxcvt(cx, buf, size);
	if (CXDETAILS(details, format, type, 0))
	{
		ret->value.number = tmscan(buf, &e, details, &f, NiL, 0);
		if (!*f && e > (char*)buf)
			return e - (char*)buf;
	}
	ret->value.number = tmdate(buf, &e, NiL);
	return e - (char*)buf;
}

static void*
ns_init(void* data, Cxdisc_t* disc)
{
	Precise_t*	precise;

	if (!(precise = newof(0, Precise_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	precise->format = "%K.%9N";
	precise->size = 40;
	precise->shift = 0;
	return precise;
}

static void*
stamp_init(void* data, Cxdisc_t* disc)
{
	Precise_t*	precise;

	if (!(precise = newof(0, Precise_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	precise->format = "%K.%N";
	precise->size = 40;
	precise->shift = 32;
	return precise;
}

static Time_t
n2s(Time_t t, int s)
{
	Time_t		m;

#if TMX_FLOAT
	m = t;
	t /= NS;
	t = (Tmxsec_t)t;
	m -= t * NS;
	t *= SS;
	m *= SS;
#else
	m = t % NS;
	t /= NS;
	t <<= s;
	m <<= s;
#endif
	m /= NS;
	return t + m;
}

static Time_t
s2n(Time_t t, int s)
{
	Time_t		m;

#if TMX_FLOAT
	m = t / SS;
	m = (Tmxnsec_t)m;
	m *= NS;
	m = t - m;
	m /= SS;
	t /= SS;
	t *= NS;
#else
	m = 1;
	m <<= s;
	m--;
	m &= t;
	t >>= s;
	t *= NS;
	m *= NS;
	m >>= s;
#endif
	return t + m;
}

static ssize_t
precise_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	char*		s;
	Time_t		t;
	Precise_t*	precise = (Precise_t*)type->data;

	if (!size)
		return precise->size;
	t = value->number;
	if (precise->shift)
		t = s2n(t, precise->shift);
	s = tmxfmt(buf, size, CXDETAILS(details, format, type, precise->format), t);
	if (s == (buf + size - 1))
		return 2 * size;
	return s - buf;
}

static ssize_t
precise_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	char*		e;
	char*		f;
	Precise_t*	precise = (Precise_t*)type->data;
	Time_t		now = TMX_NOW;
	Time_t		t;

	buf = (const char*)cxcvt(cx, buf, size);
	if (CXDETAILS(details, format, type, 0))
	{
		t = tmxscan(buf, &e, details, &f, now, 0);
		if (*f || (e - (char*)buf) < size)
			t = tmxdate(buf, &e, now);
	}
	else
		t = tmxdate(buf, &e, now);
	if (precise->shift)
		t = n2s(t, precise->shift);
	ret->value.number = t;
	return e - (char*)buf;
}

static ssize_t
elapsed_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	char*	s;
	ssize_t	n;

	s = fmtelapsed((unsigned long)value->number, 1000);
	n = strlen(s);
	if ((n + 1) > size)
		return n + 1;
	memcpy(buf, s, n + 1);
	return n;
}

static ssize_t
elapsed_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	char*	e;

	ret->value.number = strelapsed(buf, &e, 1000);
	if (e == (char*)buf)
		return -1;
	return e - (char*)buf;
}

static ssize_t
tm_hour_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	char*	s;
	int	v;
	ssize_t	n;

	v = value->number;
	CXDETAILS(details, format, type, "%d");
	if (strchr(details, 's'))
	{
		s = tm_info.format[TM_MERIDIAN + (v >= 12)];
		if (v > 12)
			v -= 12;
		n = strlen(s) + (v >= 10) + 2;
		if ((n + 1) > size)
			return n + 1;
		n = sfsprintf(buf, size, "%d%s", v, s);
	}
	else
	{
		n = sfsprintf(buf, size, details, v);
		if ((n + 1) > size)
			n++;
	}
	return n;
}

static ssize_t
tm_hour_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	char*	e;

	ret->value.number = strntol(buf, size, &e, 10);
	if (e == (char*)buf)
		return -1;
	if (tmlex(e, &e, tm_info.format + TM_MERIDIAN, TM_UT - TM_MERIDIAN, NiL, 0) == 1)
		ret->value.number += 12;
	return e - (char*)buf;
}

static ssize_t
tm_mon_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	char*	s;
	int	v;
	ssize_t	n;

	v = value->number;
	if (v <= 0)
		v = 0;
	else
		v %= 12;
	CXDETAILS(details, format, type, "%d");
	if (strchr(details, 's'))
	{
		s = tm_info.format[TM_MONTH + v];
		n = strlen(s);
		if ((n + 1) > size)
			return n + 1;
		strcpy(buf, s);
	}
	else
	{
		n = sfsprintf(buf, size, details, v + 1);
		if ((n + 1) > size)
			n++;
	}
	return n;
}

static ssize_t
tm_mon_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	char*	e;
	int	v;

	v = (int)strntol(buf, size, &e, 10);
	if (e != (char*)buf)
	{
		if (v < 1 || v > 12)
			return -1;
		v--;
	}
	else if ((v = tmlex(buf, &e, tm_info.format + TM_MONTH_ABBREV, TM_DAY_ABBREV - TM_MONTH_ABBREV, NiL, 0)) < 0)
		return -1;
	else if (v >= 12)
		v -= 12;
	ret->value.number = v;
	return e - (char*)buf;
}

static ssize_t
tm_wday_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	char*	s;
	int	v;
	ssize_t	n;

	v = value->number;
	if (v <= 0)
		v = 0;
	else
		v %= 7;
	CXDETAILS(details, format, type, "%d");
	if (strchr(details, 's'))
	{
		s = tm_info.format[TM_DAY + v];
		n = strlen(s);
		if ((n + 1) > size)
			return n + 1;
		strcpy(buf, s);
	}
	else
	{
		n = sfsprintf(buf, size, details, v + 1);
		if ((n + 1) > size)
			n++;
	}
	return n;
}

static ssize_t
tm_wday_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxoperand_t* ret, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	char*	e;
	int	v;

	v = (int)strntol(buf, size, &e, 10);
	if (e != (char*)buf)
	{
		if (v < 1 || v > 7)
			return -1;
		v--;
	}
	else if ((v = tmlex(buf, &e, tm_info.format + TM_DAY_ABBREV, TM_TIME - TM_DAY_ABBREV, NiL, 0)) < 0)
		return -1;
	else if (v >= 7)
		v -= 7;
	ret->value.number = v;
	return e - (char*)buf;
}

#define TIME_T_sec		1
#define TIME_T_min		2
#define TIME_T_hour		3
#define TIME_T_mday		4
#define TIME_T_mon		5
#define TIME_T_year		6
#define TIME_T_wday		7
#define TIME_T_yday		8
#define TIME_T_isdst		9
#define TIME_T_ns		10

static Cxvariable_t tm_struct[] =
{
CXV("sec",   "number",   TIME_T_sec,   "Seconds after the minute [0-61].")
CXV("min",   "number",   TIME_T_min,   "Minutes after the hour [0-59].")
CXV("hour",  "tm_hour_t",TIME_T_hour,  "Hour since midnight [0-23].")
CXV("mday",  "number",   TIME_T_mday,  "Day of the month [1-31].")
CXV("mon",   "tm_mon_t", TIME_T_mon,   "Months since January [0-11].")
CXV("year",  "number",   TIME_T_year,  "4-digit year [1969-2038].")
CXV("wday",  "tm_wday_t",TIME_T_wday,  "Days since Sunday [0-6].")
CXV("yday",  "number",   TIME_T_yday,  "Days since January 1 [0-365].")
CXV("isdst", "number",   TIME_T_isdst, "Daylight savings time in effect [0-1].")
CXV("ns",    "number",   TIME_T_ns,    "Residual nanoseconds [0-999999999].")
{0}
};

typedef struct Tm_state_s
{
	Time_t		t;
	Tm_t		tm;
} Tm_state_t;

static void*
tm_init(void* data, Cxdisc_t* disc)
{
	Tm_state_t*	state;

	if (!(state = newof(0, Tm_state_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	state->tm = *tmxmake(state->t);
	return state;
}

static int
tm_get(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	Tm_state_t*	state = (Tm_state_t*)pc->data.variable->member->data;
	Time_t		t;
	int		shift;

	t = r->value.number;
	if (r->type && r->type->data)
	{
		if (shift = ((Precise_t*)r->type->data)->shift)
			t = s2n(t, shift);
	}
	else if (b && b->type && b->type->data)
	{
		if (shift = ((Precise_t*)b->type->data)->shift)
			t = s2n(t, shift);
	}
	else
		t *= NS;
	if (state->t != t)
	{
		state->t = t;
		state->tm = *tmxmake(t);
	}
	switch (pc->data.variable->index)
	{
	case TIME_T_sec:
		r->value.number = state->tm.tm_sec;
		break;
	case TIME_T_min:
		r->value.number = state->tm.tm_min;
		break;
	case TIME_T_hour:
		r->value.number = state->tm.tm_hour;
		break;
	case TIME_T_mday:
		r->value.number = state->tm.tm_mday;
		break;
	case TIME_T_mon:
		r->value.number = state->tm.tm_mon;
		break;
	case TIME_T_year:
		r->value.number = 1900 + state->tm.tm_year;
		break;
	case TIME_T_wday:
		r->value.number = state->tm.tm_wday;
		break;
	case TIME_T_yday:
		r->value.number = state->tm.tm_yday;
		break;
	case TIME_T_isdst:
		r->value.number = state->tm.tm_isdst;
		break;
	case TIME_T_ns:
		r->value.number = state->tm.tm_nsec;
		break;
	default:
		return -1;
	}
	return 0;
}

static int
tm_set(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	Tm_state_t*	state = (Tm_state_t*)pc->data.variable->member->data;
	Time_t		t;
	int		i;
	int		shift;

	t = r->value.number;
	if (r->type && r->type->data)
	{
		if (shift = ((Precise_t*)r->type->data)->shift)
			t = s2n(t, shift);
	}
	else if (b && b->type && b->type->data)
	{
		if (shift = ((Precise_t*)b->type->data)->shift)
			t = s2n(t, shift);
	}
	else
	{
		shift = -1;
		t *= NS;
	}
	if (state->t != t)
	{
		state->t = t;
		state->tm = *tmxmake(t);
	}
	switch (pc->data.variable->index)
	{
	case TIME_T_sec:
		state->tm.tm_sec = a->value.number;
		break;
	case TIME_T_min:
		state->tm.tm_min = a->value.number;
		break;
	case TIME_T_hour:
		state->tm.tm_hour = a->value.number;
		break;
	case TIME_T_mday:
		state->tm.tm_mday = a->value.number;
		break;
	case TIME_T_mon:
		state->tm.tm_mon = a->value.number;
		break;
	case TIME_T_year:
		if ((state->tm.tm_year = a->value.number) >= 1900)
			state->tm.tm_year -= 1900;
		break;
	case TIME_T_wday:
		i = a->value.number;
		if ((i -= state->tm.tm_wday) < 0)
			i += 7;
		state->tm.tm_mday += i;
		state->tm.tm_wday = a->value.number;
		break;
	case TIME_T_yday:
		i = a->value.number;
		if ((i -= state->tm.tm_yday) < 0)
			i += 365 + tmisleapyear(state->tm.tm_year);
		state->tm.tm_mday += i;
		state->tm.tm_yday = a->value.number;
		break;
	case TIME_T_isdst:
		state->tm.tm_isdst = a->value.number;
		break;
	case TIME_T_ns:
		state->tm.tm_nsec = a->value.number;
		return -1;
	default:
		return -1;
	}
	state->t = t = tmxtime(&state->tm, TM_LOCALZONE);
	if (shift < 0)
		t /= NS;
	else if (shift)
		t = n2s(t, shift);
	r->value.number = t;
	return 0;
}

static Cxmember_t	tm_member =
{
	tm_get,
	tm_set,
	(Dt_t*)&tm_struct[0]
};

static Cxtype_t types[] =
{
	{ "tm_hour_t",	"Hour since midnight with optional meridian (AM/PM).", CXH, (Cxtype_t*)"number", 0, tm_hour_external, tm_hour_internal, 0, 0, 0, 0, { "The format details string is a \bprintf\b(3) format string.", "%d", CX_UNSIGNED|CX_INTEGER, 1 } },
	{ "tm_mon_t",	"Month name represented as a number [0-11], starting at January.", CXH, (Cxtype_t*)"number", 0, tm_mon_external, tm_mon_internal, 0, 0, 0, 0, { "The format details string is a \bprintf\b(3) format string.", "%s", CX_UNSIGNED|CX_INTEGER, 1 } },
	{ "tm_wday_t",	"Weekday name represented as a number [0-6], starting at Sunday.", CXH, (Cxtype_t*)"number", 0, tm_wday_external, tm_wday_internal, 0, 0, 0, 0, { "The format details string is a \bprintf\b(3) format string.", "%s", CX_UNSIGNED|CX_INTEGER, 1 } },
	{ "tm_t",	"Time parts.", CXH, (Cxtype_t*)"number", tm_init, 0, 0, 0, 0, 0, 0, { 0, 0, CX_UNSIGNED|CX_INTEGER, 4 }, 0, &tm_member	},
	{ "elapsed_t",	"Elapsed time in milliseconds.", CXH, (Cxtype_t*)"number", 0, elapsed_external, elapsed_internal, 0, 0, 0, 0, { 0, 0, CX_INTEGER, 4 }	},
	{ "ns_t",	"64 bit nanoseconds since the epoch.", CXH, (Cxtype_t*)"tm_t", ns_init, precise_external, precise_internal, 0, 0, 0, 0, { "The format details string is a \bstrftime\b(3)/\bstrptime\b(3) format string.", "%K.%9N", CX_UNSIGNED|CX_INTEGER, 8 } },
	{ "stamp_t",	"64 bit 1/2**32 seconds since the epoch.", CXH, (Cxtype_t*)"tm_t", stamp_init, precise_external, precise_internal, 0, 0, 0, 0, { "The format details string is a \bstrftime\b(3)/\bstrptime\b(3) format string.", "%K.%N", CX_UNSIGNED|CX_INTEGER, 8 } },
	{ "time_t",	"32 bit seconds since the epoch.", CXH, (Cxtype_t*)"tm_t", 0, time_external, time_internal, 0, 0, 0, 0, { "The format details string is a \bstrftime\b(3)/\bstrptime\b(3) format string.", "%K", CX_UNSIGNED|CX_INTEGER, 4 } },
	{ 0, 0 }
};

Dsslib_t dss_lib_time_t =
{
	"time_t",
	"time type support"
	"[-?\n@(#)$Id: dss time type library (AT&T Research) 2011-09-10 $\n]"
	USAGE_LICENSE,
	CXH,
	0,
	0,
	&types[0],
};
