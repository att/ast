/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2013 AT&T Intellectual Property          *
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
 * Glenn Fowler
 * AT&T Research
 *
 * tw expression definitions
 */

#include "tw.h"

#include <ctype.h>
#include <tm.h>
#include <sfdisc.h>
#include <sum.h>

#define STAT(f,b)	((state.ftwflags&FTW_PHYSICAL)?lstat(f,b):pathstat(f,b))

typedef struct Exsum_s
{
	Sum_t*		sum;
	Sfio_t*		buf;
} Exsum_t;

static Exid_t	symbols[] =
{
	EXID("AGAIN",	CONSTANT,	C_AGAIN,	INTEGER,	0),
	EXID("BLK",	CONSTANT,	C_BLK,		INTEGER,	0),
	EXID("C",	CONSTANT,	C_C,		INTEGER,	0),
	EXID("CHR",	CONSTANT,	C_CHR,		INTEGER,	0),
	EXID("CTG",	CONSTANT,	C_CTG,		INTEGER,	0),
	EXID("D",	CONSTANT,	C_D,		INTEGER,	0),
	EXID("DC",	CONSTANT,	C_DC,		INTEGER,	0),
	EXID("DIR",	CONSTANT,	C_DIR,		INTEGER,	0),
	EXID("DOOR",	CONSTANT,	C_DOOR,		INTEGER,	0),
	EXID("DNR",	CONSTANT,	C_DNR,		INTEGER,	0),
	EXID("DNX",	CONSTANT,	C_DNX,		INTEGER,	0),
	EXID("FIFO",	CONSTANT,	C_FIFO,		INTEGER,	0),
	EXID("FMT",	CONSTANT,	C_FMT,		INTEGER,	0),
	EXID("FOLLOW",	CONSTANT,	C_FOLLOW,	INTEGER,	0),
	EXID("LNK",	CONSTANT,	C_LNK,		INTEGER,	0),
	EXID("NOPOST",	CONSTANT,	C_NOPOST,	INTEGER,	0),
	EXID("NR",	CONSTANT,	C_NR,		INTEGER,	0),
	EXID("NS",	CONSTANT,	C_NS,		INTEGER,	0),
	EXID("NX",	CONSTANT,	C_NX,		INTEGER,	0),
	EXID("REG",	CONSTANT,	C_REG,		INTEGER,	0),
	EXID("SKIP",	CONSTANT,	C_SKIP,		INTEGER,	0),
	EXID("SOCK",	CONSTANT,	C_SOCK,		INTEGER,	0),
	EXID("args",	ID,		F_args,		INTEGER,	0),
	EXID("atime",	ID,		F_atime,	T_DATE,		0),
	EXID("blksize",	ID,		F_blksize,	INTEGER,	0),
	EXID("blocks",	ID,		F_blocks,	INTEGER,	0),
	EXID("checksum",ID,		F_checksum,	STRING,		0),
	EXID("cmdarg",	FUNCTION,	X_cmdarg,	I|A(1,S),	0),
	EXID("cmdflush",FUNCTION,	X_cmdflush,	I,		0),
	EXID("ctime",	ID,		F_ctime,	T_DATE,		0),
	EXID("date_t",	DECLARE,   	T_DATE,		T_DATE,		0),
	EXID("dev",	ID,		F_dev,		INTEGER,	0),
	EXID("fstype",	ID,		F_fstype,	STRING,		0),
	EXID("gid",	ID,		F_gid,		T_GID,		0),
	EXID("gid_t",	DECLARE,   	T_GID,		T_GID,		0),
	EXID("gidok",	ID,		F_gidok,	INTEGER,	0),
	EXID("info",	ID,		F_info,		INTEGER,	0),
	EXID("ino",	ID,		F_ino,		INTEGER,	0),
	EXID("level",	ID,		F_level,	INTEGER,	0),
	EXID("local",	ID,		F_local,	INTEGER,	0),
	EXID("magic",	ID,		F_magic,	STRING,		0),
	EXID("md5sum",	ID,		F_md5sum,	STRING,		0),
	EXID("mime",	ID,		F_mime,		STRING,		0),
	EXID("mode",	ID,		F_mode,		T_MODE,		0),
	EXID("mode_t",	DECLARE,   	T_MODE,		T_MODE,		0),
	EXID("mtime",	ID,		F_mtime,	T_DATE,		0),
	EXID("name",	ID,		F_name,		STRING,		0),
	EXID("nlink",	ID,		F_nlink,	INTEGER,	0),
	EXID("parent",	ID,		F_parent,	INTEGER,	0),
	EXID("path",	ID,		F_path,		STRING,		0),
	EXID("perm",	ID,		F_perm,		T_PERM,		0),
	EXID("perm_t",	DECLARE,   	T_PERM,		T_PERM,		0),
	EXID("rdev",	ID,		F_rdev,		INTEGER,	0),
	EXID("size",	ID,		F_size,		INTEGER,	0),
	EXID("status",	ID,		F_status,	INTEGER,	0),
	EXID("sum",	FUNCTION,	X_sum,		S|A(1,S),	0),
	EXID("symlink",	ID,		F_symlink,	STRING,		0),
	EXID("type",	ID,		F_type,		INTEGER,	0),
	EXID("uid",	ID,		F_uid,		T_UID,		0),
	EXID("uid_t",	DECLARE,   	T_UID,		T_UID,		0),
	EXID("uidok",	ID,		F_uidok,	INTEGER,	0),
	EXID("url",	ID,		F_url,		STRING,		0),
	EXID("view",	ID,		F_view,		INTEGER,	0),
	EXID("visit",	ID,		F_visit,	INTEGER,	0),
	EXID(0,		0,		0,		0,		0)
};

/*
 * dereference ftw
 */

static Ftw_t*
deref(register Ftw_t* ftw, Exid_t* sym, register Exref_t* ref)
{
	register char*	path;

	path = ftw->path;
	while (ref)
	{
		if (!ftw || ref->symbol->index != F_parent)
		{
			if (ftw && !ref->next && MEMINDEX(sym->index)) break;
			error(3, "%s: invalid %s.%s reference", path, ref->symbol->name, sym->name);
		}
		ref = ref->next;
		ftw = ftw->parent;
	}
	return ftw;
}

/*
 * return sum() for path
 */

static char*
sum(Exsum_t* sum, const char* path)
{
	Sfio_t*			sp;
	char*			s;
	int			r;

	suminit(sum->sum);
	if (!(sp = sfopen(NiL, path, "r")))
		goto bad;
	while (s = (char*)sfreserve(sp, SF_UNBOUND, 0))
		sumblock(sum->sum, s, sfvalue(sp));
	r = !!sfvalue(sp);
	if (sfclose(sp) || r)
		goto bad;
	sumdone(sum->sum);
	sumprint(sum->sum, sum->buf, 0, 0);
	return sfstruse(sum->buf);
 bad:
	return "*******READ-ERROR*******";
}

/*
 * format urlized path
 */

static char*
fmturl(register const char* path)
{
	register unsigned char*	p;
	register char*		s;
	register int		n;
	register int		c;
	char*			r;

	static const char	hex[] = "0123456789ABCDEF";

	n = 0;
	p = (unsigned char*)path;
	while (c = *p++)
		if (!(isalnum(c) || c == '_' || c == '-' || c == '+' || c == '=' || c == '/' || c == '.'))
			n++;
	r = s = fmtbuf(n * 2 + (p - (unsigned char*)path));
	p = (unsigned char*)path;
	while (c = *p++)
		if (!(isalnum(c) || c == '_' || c == '-' || c == '+' || c == '=' || c == '/' || c == '.'))
		{
			*s++ = '%';
			*s++ = hex[(c>>4)&0xF];
			*s++ = hex[c&0xF];
		}
		else
			*s++ = c;
	*s = 0;
	return r;
}

/*
 * return value for symbol sym
 */

static Extype_t
getval(Expr_t* pgm, Exnode_t* node, Exid_t* sym, Exref_t* ref, void* env, int elt, Exdisc_t* disc)
{
	register struct stat*	st;
	register Ftw_t*		ftw;
	Extype_t		v;
	Local_t*		lp;
	Sfio_t*			fp;
	Fileid_t		id;
	Visit_t*		vp;
	Exsum_t*		sp;

	static char		text[PATH_MAX];

	NoP(pgm);
	NoP(node);
	NoP(disc);
	if (elt >= EX_SCALAR)
	{
		ftw = (Ftw_t*)env;
		if (ref && !(ftw = deref(ftw, sym, ref)))
		{
			v.integer = 0;
			return v;
		}
		st = &ftw->statb;
	}
	switch (sym->index)
	{
	case F_args:
		v.integer = state.args;
		break;
	case F_atime:
		v.integer = st->st_atime;
		break;
	case F_blocks:
		v.integer = iblocks(st);
		break;
	case F_blksize:
#ifdef stat_blksize
		v.integer = st->st_blksize;
#else
		v.integer = 1024;
#endif
		break;
	case F_checksum:
		v.string = sum((Exsum_t*)node->local.pointer, PATH(ftw));
		goto string;
	case F_ctime:
		v.integer = st->st_ctime;
		break;
	case F_dev:
		v.integer = st->st_dev;
		break;
	case F_fstype:
		v.string = fmtfs(st);
		goto string;
	case F_gid:
		v.integer = st->st_gid;
		break;
	case F_gidok:
		v.integer = !isdigit(*fmtgid(st->st_gid));
		break;
	case F_info:
		v.integer = ftw->info;
		break;
	case F_ino:
		v.integer = st->st_ino;
		break;
	case F_level:
		v.integer = ftw->level;
		break;
	case F_local:
		if (state.localmem)
			error(3, "%s: reference invalid when members declared", sym->name);
		else
			v.integer = ftw->local.number;
		break;
	case F_magic:
		fp = sfopen(NiL, PATH(ftw), "r");
		v.string = magictype(state.magic, fp, PATH(ftw), st);
		if (fp)
			sfclose(fp);
		goto string;
	case F_md5sum:
		v.string = sum((Exsum_t*)node->local.pointer, PATH(ftw));
		break;
	case F_mime:
		fp = sfopen(NiL, PATH(ftw), "r");
		state.magicdisc.flags |= MAGIC_MIME;
		v.string = magictype(state.magic, fp, PATH(ftw), st);
		state.magicdisc.flags &= ~MAGIC_MIME;
		if (fp)
			sfclose(fp);
		goto string;
	case F_mode:
		v.integer = st->st_mode;
		break;
	case F_mtime:
		v.integer = st->st_mtime;
		break;
	case F_name:
		v.string = ftw->name;
		goto string;
	case F_nlink:
		v.integer = st->st_nlink;
		break;
	case F_path:
		v.string = ftw->path;
		goto string;
	case F_perm:
		v.integer = st->st_mode & (S_IRWXU|S_IRWXG|S_IRWXO);
		break;
	case F_rdev:
		v.integer = idevice(st);
		break;
	case F_size:
		v.integer = st->st_size;
		break;
	case F_status:
		v.integer = ftw->status;
		break;
	case F_symlink:
		if (!S_ISLNK(st->st_mode) || pathgetlink(PATH(ftw), text, sizeof(text)) < 0)
			text[0] = 0;
		v.string = text;
		goto string;
	case F_type:
		if (S_ISREG(st->st_mode)) v.integer = C_REG;
		else if (S_ISDIR(st->st_mode)) v.integer = C_DIR;
		else if (S_ISLNK(st->st_mode)) v.integer = C_LNK;
		else if (S_ISBLK(st->st_mode)) v.integer = C_BLK;
		else if (S_ISCHR(st->st_mode)) v.integer = C_CHR;
		else if (S_ISFIFO(st->st_mode)) v.integer = C_FIFO;
#ifdef S_ISCTG
		else if (S_ISCTG(st->st_mode)) v.integer = C_CTG;
#endif
#ifdef S_ISSOCK
		else if (S_ISSOCK(st->st_mode)) v.integer = C_SOCK;
#endif
#ifdef S_ISDOOR
		else if (S_ISDOOR(st->st_mode)) v.integer = C_DOOR;
#endif
		else v.integer = 0;
		break;
	case F_uid:
		v.integer = st->st_uid;
		break;
	case F_uidok:
		v.integer = !isdigit(*fmtuid(st->st_uid));
		break;
	case F_url:
		v.string = fmturl(ftw->path ? ftw->path : ftw->name);
		goto string;
	case F_view:
		v.integer = iview(st);
		break;
	case F_visit:
		if (state.visitmem)
			error(3, "%s: reference invalid when members declared", sym->name);
		id.di[0] = st->st_dev;
		id.di[1] = st->st_ino;
		if (vp = (Visit_t*)dtmatch(state.vistab, &id))
			v = vp->value[0];
		else
			v = exzero(INTEGER);
		break;
	case X_cmdarg:
		cmdarg(state.cmd, ((Extype_t*)env)[0].string, strlen(((Extype_t*)env)[0].string));
		v.integer = 1;
		break;
	case X_cmdflush:
		cmdflush(state.cmd);
		v.integer = 1;
		break;
	case X_sum:
		sp = (Exsum_t*)node->local.pointer;
		if (!sp->sum && !(sp->sum = sumopen(((Extype_t*)env)[0].string)))
			error(ERROR_SYSTEM|3, "%s checksum initialization error", ((Extype_t*)env)[0].string);
		ftw = (Ftw_t*)(((Extype_t*)env)[-1].string);
		v.string = sum(sp, PATH(ftw));
		break;
	default:
		switch (MEMINDEX(sym->index))
		{
		case F_local:
			v = (lp = (Local_t*)ftw->local.pointer) ? lp->value[MEMOFFSET(sym->index)] : exzero(sym->type);
			break;
		case F_visit:
			id.di[0] = st->st_dev;
			id.di[1] = st->st_ino;
			v = (vp = (Visit_t*)dtmatch(state.vistab, &id)) ? vp->value[MEMOFFSET(sym->index)] : exzero(sym->type);
			break;
		default:
			error(3, "%s: invalid identifier reference [ index=%d memindex=%d memoffset=%d ]", sym->name, sym->index, MEMINDEX(sym->index), MEMOFFSET(sym->index));
			break;
		}
		break;
	}
	return v;
 string:
	if (!state.compiled && v.string)
		v.string = strdup(v.string);
	return v;
}

/*
 * initialize the visit table
 */

static void
initvisit(Exid_t* sym)
{
	Dtdisc_t*	disc;

	if (!state.vistab)
	{
		if (!(disc = newof(0, Dtdisc_t, 1, 0)))
			error(ERROR_SYSTEM|3, "out of space [visit table]");
		disc->key = offsetof(Visit_t, id);
		disc->size = sizeof(Fileid_t);
		if (!(state.vistab = dtopen(disc, Dtset)))
			error(3, "%s: cannot allocate file identifier hash table", sym->name);
	}
}

/*
 * sym referenced in expression
 */

static Extype_t
refval(Expr_t* pgm, Exnode_t* node, Exid_t* sym, Exref_t* ref, char* str, int elt, Exdisc_t* disc)
{
	Ftw_t		ftwbuf;
	Extype_t	v;
	Exsum_t*	sp;
	char*		m;

	if (elt >= 0)
		error(3, "%s: arrays not supported", sym->name);
	if (str)
	{
		if (STAT(str, &ftwbuf.statb))
			memzero(&ftwbuf.statb, sizeof(ftwbuf.statb));
		return getval(pgm, node, sym, ref, &ftwbuf, elt, disc);
	}
	v = exzero(sym->type);
	switch (sym->index)
	{
	case MEMBER:
		while (ref)
		{
			switch (ref->symbol->index)
			{
			case F_parent:
				break;
			case F_local:
				if (!ref->next)
				{
					sym->index = MEMMAKE(ref->symbol->index, state.localmem++);
					break;
				}
				/*FALLTHROUGH*/
			case F_visit:
				if (!ref->next)
				{
					if (!state.vistab)
						initvisit(sym);
					sym->index = MEMMAKE(ref->symbol->index, state.visitmem++);
					break;
				}
				/*FALLTHROUGH*/
			default:
				error(3, "invalid %s.%s reference", ref->symbol->name, sym->name);
				break;
			}
			ref = ref->next;
		}
		break;
	case C_AGAIN:
		v.integer = FTW_AGAIN;
		break;
	case C_BLK:
	case C_CHR:
	case C_DIR:
	case C_DOOR:
	case C_FIFO:
	case C_LNK:
	case C_REG:
	case C_SOCK:
		v.integer = sym->index;
		break;
	case C_C:
		v.integer = FTW_C;
		break;
	case C_D:
		v.integer = FTW_D;
		break;
	case C_DC:
		v.integer = FTW_DC;
		break;
	case C_DNR:
		v.integer = FTW_DNR;
		break;
	case C_DNX:
		v.integer = FTW_DNX;
		break;
	case C_DP:
		v.integer = FTW_DP;
		break;
	case C_FOLLOW:
		v.integer = FTW_FOLLOW;
		break;
	case C_NOPOST:
		v.integer = FTW_NOPOST;
		break;
	case C_NR:
		v.integer = FTW_NR;
		break;
	case C_NS:
		v.integer = FTW_NS;
		break;
	case C_NX:
		v.integer = FTW_NX;
		break;
	case C_SKIP:
		v.integer = FTW_SKIP;
		break;
	case F_info:
		state.info = 1;
		break;
	case F_magic:
	case F_mime:
		if (!state.magic)
		{
			state.magicdisc.version = MAGIC_VERSION;
			state.magicdisc.flags = 0;
			state.magicdisc.errorf = errorf;
			if (!(state.magic = magicopen(&state.magicdisc)) || magicload(state.magic, NiL, 0))
				error(3, "%s: cannot load magic file", MAGIC_FILE);
		}
		break;
	case F_visit:
		if (!state.vistab)
			initvisit(sym);
		break;
	case F_atime:
	case F_blksize:
	case F_blocks:
	case F_ctime:
	case F_dev:
	case F_fstype:
	case F_gid:
	case F_gidok:
	case F_ino:
	case F_mode:
	case F_mtime:
	case F_nlink:
	case F_perm:
	case F_rdev:
	case F_size:
	case F_symlink:
	case F_type:
	case F_uid:
	case F_uidok:
		state.ftwflags &= ~FTW_DELAY;
		break;
	case X_cmdarg:
		state.cmdflags &= ~CMD_IMPLICIT;
		break;
	case F_checksum:
		m = "ast4";
		goto sum;
	case F_md5sum:
		m = "md5sum";
		goto sum;
	case X_sum:
		m = 0;
	sum:
		if (!(sp = newof(0, Exsum_t, 1, 0)) || !(sp->buf = sfstropen()))
			error(ERROR_SYSTEM|3, "out of space [sum]");
		node->local.pointer = (char*)sp;
		if (m && !(sp->sum = sumopen(m)))
			error(ERROR_SYSTEM|3, "sum(\"%s\") initialization error", m);
		break;
	}
	return v;
}

/*
 * set value for symbol sym to val
 */

static int
setval(Expr_t* pgm, Exnode_t* node, Exid_t* sym, Exref_t* ref, void* env, int elt, Extype_t val, Exdisc_t* disc)
{
	register Ftw_t*	ftw;
	Fileid_t	id;
	Visit_t*	vp;
	Local_t*	lp;

	NoP(pgm);
	NoP(node);
	NoP(disc);
	if (elt >= 0)
		error(3, "%s: arrays not supported", sym->name);
	if (!(ftw = (Ftw_t*)env) || ref && !(ftw = deref(ftw, sym, ref))) return -1;
	switch (sym->index)
	{
	case F_local:
		if (state.localmem) error(3, "%s: reference invalid when members declared", sym->name);
		else ftw->local.number = val.integer;
		break;
	case F_status:
		ftw->status = val.integer;
		break;
	case F_visit:
		if (state.visitmem)
			error(3, "%s: reference invalid when members declared", sym->name);
		id.di[0] = ftw->statb.st_dev;
		id.di[1] = ftw->statb.st_ino;
		if (!(vp = (Visit_t*)dtmatch(state.vistab, &id)))
		{
			if (!(vp = newof(0, Visit_t, 1, 0)))
				error(ERROR_SYSTEM|3, "out of space [visit]");
			vp->id = id;
			dtinsert(state.vistab, vp);
		}
		vp->value[0] = val;
		break;
	default:
		switch (MEMINDEX(sym->index))
		{
		case F_local:
			if (!(lp = (Local_t*)ftw->local.pointer))
			{
				if (state.local)
				{
					lp = state.local;
					state.local = state.local->next;
					memzero(lp, sizeof(Local_t) + sizeof(Extype_t) * (state.localmem - 1));
				}
				else if (!(lp = newof(0, Local_t, 1, sizeof(Extype_t) * (state.localmem - 1))))
					error(3, "out of space");
				lp = (Local_t*)ftw->local.pointer;
			}
			lp->value[MEMOFFSET(sym->index)] = val;
			break;
		case F_visit:
			id.di[0] = ftw->statb.st_dev;
			id.di[1] = ftw->statb.st_ino;
			if (!(vp = (Visit_t*)dtmatch(state.vistab, &id)))
			{
				if (!(vp = newof(0, Visit_t, 1, sizeof(Extype_t) * (state.visitmem - 1))))
					error(3, "out of space [visit]");
				vp->id = id;
				dtinsert(state.vistab, vp);
			}
			vp->value[MEMOFFSET(sym->index)] = val;
			break;
		default:
			return -1;
		}
		break;
	}
	return 0;
}

/*
 * match str against pat
 */

static int
matchval(Expr_t* pgm, Exnode_t* xstr, const char* str, Exnode_t* xpat, const char* pat, void* env, Exdisc_t* disc)
{
	return strgrpmatch(str, pat, NiL, 0, STR_MAXIMAL|STR_LEFT|STR_RIGHT|(env?((Ftw_t*)env)->ignorecase:0));
}

/*
 * convert operands
 */

static int
convert(Expr_t* prog, register Exnode_t* x, int type, register Exid_t* xref, int arg, Exdisc_t* disc)
{
	register Sflong_t	n;
	time_t			t;
	char*			s;

	static char		buf[64];

	NoP(prog);
	NoP(disc);
	if (type < 0200)
	{
		if (x->type == FLOATING)
			n = x->data.constant.value.floating;
		else if (x->type == INTEGER)
			n = x->data.constant.value.integer;
		else switch (type)
		{
		case T_DATE:
			n = tmdate(x->data.constant.value.string, &s, NiL);
			if (*s)
				goto ignored;
			break;
		case T_GID:
			if ((n = strgid(x->data.constant.value.string)) < 0 && !arg)
				error(1, "%s: %s not found", xref->name, x->data.constant.value.string);
			break;
		case T_MODE:
		case T_PERM:
			n = strperm(x->data.constant.value.string, &s, 0);
			break;
		case T_UID:
			if ((n = struid(x->data.constant.value.string)) < 0 && !arg)
				error(1, "%s: %s not found", xref->name, x->data.constant.value.string);
			break;
		}
		x->data.constant.value.integer = n;
		type = INTEGER;
	}
	else
	{
		n = x->data.constant.value.integer;
		switch (type)
		{
		case FLOATING:
			x->data.constant.value.floating = n;
			break;
		case STRING:
			switch (x->type)
			{
			case T_DATE:
				t = n;
				tmfmt(s = buf, sizeof(buf), "%?%QL", &t);
				break;
			case T_GID:
				s = fmtgid((gid_t)n);
				break;
			case T_MODE:
				s = fmtmode((mode_t)n, 0);
				break;
			case T_PERM:
				s = fmtmode((mode_t)n, 0) + 1;
				break;
			case T_UID:
				s = fmtuid((uid_t)n);
				break;
			default:
				sfsprintf(s = buf, sizeof(buf), "%ld", n);
				break;
			}
			if (!state.compiled && s)
				s = strdup(s);
			if (!(x->data.constant.value.string = s))
				return -1;
			break;
		}
	}
	x->type = type;
	return 0;
 ignored:
	if (xref)
		error(1, "%s: `%s' ignored in conversion", xref->name, s);
	else
		error(1, "`%s' ignored in conversion", s);
	x->type = type;
	return 0;
}

/*
 * initialize the expression state
 */

static void
init(void)
{
	if (!state.program)
	{
		state.expr.version = EX_VERSION;
		state.expr.flags = EX_CHARSTRING|EX_FATAL;
		state.expr.symbols = symbols;
		state.expr.convertf = convert;
		state.expr.errorf = errorf;
		state.expr.getf = getval;
		state.expr.reff = refval;
		state.expr.setf = setval;
		state.expr.matchf = matchval;
		if (!(state.program = exopen(&state.expr)))
			error(3, "expression allocation error");
	}
}

/*
 * compile the tw expression in s
 */

void
compile(char* s, int file)
{
	Sfio_t*		sp;

	if (!state.program)
		init();
	state.compiled = 0;
	if (file)
	{
		if (!(sp = sfopen(NiL, s, "r")))
			error(3|ERROR_SYSTEM, "%s: cannot read", s);
		else if (excomp(state.program, s, 1, NiL, sp))
			error(3, "%s: expression compile error", s);
		else
			sfclose(sp);
	}
	else if (excomp(state.program, NiL, 0, s, NiL))
		error(3, "expression compile error");
	state.compiled = 1;
	if (error_info.trace)
		exdump(state.program, NiL, sfstdout);
}

/*
 * evaluate x w.r.t. ftw
 */

long
eval(Exnode_t* x, Ftw_t* ftw)
{
	Extype_t	v;

	v = exeval(state.program, x, ftw);
	return v.integer;
}

/*
 * return integer value for id
 */

long
getnum(Exid_t* sym, Ftw_t* ftw)
{
	Extype_t	v;

	v = getval(NiL, NiL, sym, NiL, ftw, 0, NiL);
	return v.integer;
}

/*
 * sfkeyprintf() lookup
 */

static int
key(void* handle, register Sffmt_t* fp, const char* arg, char** ps, Sflong_t* pn)
{
	Ftw_t*		ftw = (Ftw_t*)handle;
	Exid_t*		sym;
	Extype_t	v;

	if (!fp->t_str)
		return 0;
	if (!(sym = (Exid_t*)dtmatch(state.program->symbols, fp->t_str)))
	{
		error(3, "%s: unknown format key", fp->t_str);
		return 0;
	}
	v = getval(NiL, NiL, sym, NiL, ftw, 0, NiL);
	if ((sym->type & (F|I|S)) == S)
		*ps = v.string;
	else
		*pn = v.integer;
	return 1;
}

ssize_t
print(Sfio_t* sp, Ftw_t* ftw, const char* format)
{
	if (!state.program)
	{
		init();
		state.compiled = 1;
	}
	return sfkeyprintf(sp, ftw, format, key, NiL);
}
