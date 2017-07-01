/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2013 AT&T Intellectual Property          *
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
 * pax zip format
 */

#include <paxlib.h>
#include <codex.h>
#include <dt.h>
#include <swap.h>
#include <tm.h>
#include <vmalloc.h>

#define SUM		"sum-crc-0xedb88320-init-done"

#define ZIP_HEADER	46		/* largest header size		*/
#define ZIP_COPY	0		/* no need to unzip		*/

#define ZIP_CEN_HEADER	46		/* central header size		*/
#define ZIP_CEN_MAGIC	0x504b0102L	/* central header magic		*/
#define ZIP_CEN_VEM	4		/* version made by		*/
#define ZIP_CEN_VER	6		/* version needed to extract	*/
#define ZIP_CEN_FLG	8		/* encrypt, deflate flags	*/
#define ZIP_CEN_METHOD	10		/* compression method		*/
#define ZIP_CEN_TIM	12		/* DOS format modify time	*/
#define ZIP_CEN_DAT	14		/* DOS format modify date	*/
#define ZIP_CEN_CRC	16		/* uncompressed data crc-32	*/
#define ZIP_CEN_SIZ	20		/* compressed data size		*/
#define ZIP_CEN_LEN	24		/* uncompressed data size	*/
#define ZIP_CEN_NAM	28		/* length of filename		*/
#define ZIP_CEN_EXT	30		/* length of extra field	*/
#define ZIP_CEN_COM	32		/* file comment length		*/
#define ZIP_CEN_DSK	34		/* disk number start		*/
#define ZIP_CEN_ATT	36		/* internal file attributes	*/
#define ZIP_CEN_ATX	38		/* external file attributes	*/
#define ZIP_CEN_OFF	42		/* local header relative offset	*/

#define ZIP_LOC_HEADER	30		/* local header size		*/
#define ZIP_LOC_MAGIC	0x504b0304L	/* local header magic		*/
#define ZIP_LOC_VER	4		/* version needed to extract	*/
#define ZIP_LOC_FLG	6		/* encrypt, deflate flags	*/
#define ZIP_LOC_METHOD	8		/* compression method		*/
#define ZIP_LOC_TIM	10		/* DOS format modify time	*/
#define ZIP_LOC_DAT	12		/* DOS format modify date	*/
#define ZIP_LOC_CRC	14		/* uncompressed data crc-32	*/
#define ZIP_LOC_SIZ	18		/* compressed data size		*/
#define ZIP_LOC_LEN	22		/* uncompressed data size	*/
#define ZIP_LOC_NAM	26		/* length of filename		*/
#define ZIP_LOC_EXT	28		/* length of extra field	*/

#define ZIP_END_HEADER	22		/* end header size		*/
#define ZIP_END_MAGIC	0x504b0506L	/* end header magic		*/
#define ZIP_END_DSK	4		/* number of this disk		*/
#define ZIP_END_BEG	6		/* number of the starting disk	*/
#define ZIP_END_SUB	8		/* entries on this disk		*/
#define ZIP_END_TOT	10		/* total number of entries	*/
#define ZIP_END_SIZ	12		/* central directory total size	*/
#define ZIP_END_OFF	16		/* central offset starting disk	*/
#define ZIP_END_COM	20		/* length of zip file comment	*/

#define ZIP_EXT_HEADER	16		/* ext header size		*/
#define ZIP_EXT_MAGIC	0x504b0708L	/* ext header magic		*/
#define ZIP_EXT_SIZ	8		/* compressed data size		*/
#define ZIP_EXT_LEN	12		/* uncompressed data size	*/

typedef struct Ar_s
{
	Codexdisc_t	codexdisc;
	Dtdisc_t	memdisc;
	Pax_t*		pax;
	Paxarchive_t*	ap;
	char		method[128];
	unsigned int	index;
	unsigned long	checksum;
	off_t		end;
	Dt_t*		mem;
	Vmalloc_t*	vm;
} Ar_t;

typedef struct Mem_s
{
	Dtlink_t	link;
	off_t		encoded;
	off_t		decoded;
	unsigned long	checksum;
	char		name[1];
} Mem_t;

static int
zip_done(Pax_t* pax, register Paxarchive_t* ap)
{
	register Ar_t*	ar = (Ar_t*)ap->data;

	if (!ar || !ar->vm)
		return -1;
	vmclose(ar->vm);
	ap->data = 0;
	return 0;
}

static int
zip_getprologue(Pax_t* pax, Paxformat_t* fp, register Paxarchive_t* ap, Paxfile_t* f, unsigned char* buf, size_t size)
{
	register Ar_t*		ar;
	unsigned long		magic;
	unsigned char*		hdr;
	int			n;
	int			ext;
	int			com;
	off_t			pos;
	Mem_t*			mem;
	Vmalloc_t*		vm;

	if (size < ZIP_LOC_HEADER || (magic = (unsigned long)swapget(0, buf, 4)) != ZIP_LOC_MAGIC && magic != ZIP_CEN_MAGIC)
		return 0;
	if (!(vm = vmopen(Vmdcheap, Vmbest, 0)))
		return paxnospace(pax);
	if (!(ar = vmnewof(vm, 0, Ar_t, 1, 0)))
	{
		vmclose(vm);
		return paxnospace(pax);
	}
	ap->data = ar;
	ar->vm = vm;
	ar->memdisc.key = offsetof(Mem_t, name);
	if (paxseek(pax, ap, -(off_t)ZIP_END_HEADER, SEEK_END, 1) > 0 &&
	    (hdr = paxget(pax, ap, ZIP_END_HEADER, NiL)) &&
	    swapget(0, &hdr[0], 4) == ZIP_END_MAGIC)
	{
		if (!(ar->mem = dtnew(ar->vm, &ar->memdisc, Dtset)))
		{
			zip_done(pax, ap);
			return paxnospace(pax);
		}
		pos = swapget(3, &hdr[ZIP_END_OFF], 4);
		if (paxseek(pax, ap, pos, SEEK_SET, 1) != pos)
		{
			(*pax->errorf)(NiL, pax, 2, "%s: %s central header seek error", ap->name, fp->name);
			zip_done(pax, ap);
			return -1;
		}
		ar->end = pos;
		while ((hdr = paxget(pax, ap, -ZIP_CEN_HEADER, NiL)) && swapget(0, &hdr[0], 4) == ZIP_CEN_MAGIC)
		{
			n = swapget(3, &hdr[ZIP_CEN_NAM], 2);
			if (!(mem = vmnewof(ar->vm, 0, Mem_t, 1, n)))
			{
				zip_done(pax, ap);
				return paxnospace(pax);
			}
			mem->encoded = swapget(3, &hdr[ZIP_CEN_SIZ], 4);
			mem->decoded = swapget(3, &hdr[ZIP_CEN_LEN], 4);
			mem->checksum = swapget(3, &hdr[ZIP_CEN_CRC], 4);
			ext = swapget(3, &hdr[ZIP_CEN_EXT], 2);
			com = swapget(3, &hdr[ZIP_CEN_COM], 2);
			if (paxread(pax, ap, mem->name, (off_t)n, (off_t)0, 0) <= 0)
			{
				(*pax->errorf)(NiL, pax, 2, "%s: invalid %s format verification header name [size=%I*u]", ap->name, fp->name, sizeof(n), n);
				zip_done(pax, ap);
				return -1;
			}
			if (mem->name[n - 1] == '/')
				n--;
			mem->name[n] = 0;
			if (ext && paxread(pax, ap, NiL, (off_t)ext, (off_t)0, 0) <= 0)
			{
				(*pax->errorf)(NiL, pax, 2, "%s: %s: invalid %s format verification header extended data [size=%I*u]", ap->name, mem->name, fp->name, sizeof(ext), ext);
				zip_done(pax, ap);
				return -1;
			}
			if (com && paxread(pax, ap, NiL, (off_t)com, (off_t)0, 0) <= 0)
			{
				(*pax->errorf)(NiL, pax, 2, "%s: %s: invalid %s format verification header comment data [size=%I*u]", ap->name, mem->name, fp->name, sizeof(com), com);
				zip_done(pax, ap);
				return -1;
			}
			dtinsert(ar->mem, mem);
		}
		if (paxseek(pax, ap, (off_t)0, SEEK_SET, 1))
		{
			(*pax->errorf)(NiL, pax, 2, "%s: %s central header reposition error", ap->name, fp->name);
			zip_done(pax, ap);
			return -1;
		}
	}
	ar->pax = pax;
	ar->ap = ap;
	codexinit(&ar->codexdisc, pax->errorf);
	return 1;
}

static int
zip_getheader(Pax_t* pax, register Paxarchive_t* ap, register Paxfile_t* f)
{
	register Ar_t*	ar = (Ar_t*)ap->data;
	unsigned char*	hdr;
	Mem_t*		mem;
	long		n;
	ssize_t		num;
	int		i;
	int		m;
	Tm_t		tm;

	while (hdr = paxget(pax, ap, -(num = ZIP_LOC_HEADER), NiL))
	{
		switch ((long)swapget(0, hdr, 4))
		{
		case ZIP_LOC_MAGIC:
			n = swapget(3, &hdr[ZIP_LOC_NAM], 2);
			m = swapget(3, &hdr[ZIP_LOC_EXT], 2);
			paxunread(pax, ap, hdr, num);
			num += n + m;
			if (!(hdr = paxget(pax, ap, -num, NiL)))
				break;
			f->name = paxstash(pax, &ap->stash.head, NiL, n);
			memcpy(f->name, hdr + ZIP_LOC_HEADER, n);
			if (n > 0 && f->name[n - 1] == '/' || !n && f->name[0] == '/')
			{
				if (n)
					n--;
				f->st->st_mode = (X_IFDIR|X_IRUSR|X_IWUSR|X_IXUSR|X_IRGRP|X_IWGRP|X_IXGRP|X_IROTH|X_IWOTH|X_IXOTH);
			}
			else
				f->st->st_mode = (X_IFREG|X_IRUSR|X_IWUSR|X_IRGRP|X_IWGRP|X_IROTH|X_IWOTH);
			f->st->st_mode &= pax->modemask;
			f->name[n] = 0;
			f->st->st_dev = 0;
			f->st->st_ino = 0;
			f->st->st_uid = pax->uid;
			f->st->st_gid = pax->gid;
			f->st->st_nlink = 1;
			IDEVICE(f->st, 0);
			n = swapget(3, &hdr[ZIP_LOC_TIM], 4);
			memset(&tm, 0, sizeof(tm));
			tm.tm_year = ((n>>25)&0377) + 80;
			tm.tm_mon = ((n>>21)&017) - 1;
			tm.tm_mday = ((n>>16)&037);
			tm.tm_hour = ((n>>11)&037);
			tm.tm_min = ((n>>5)&077);
			tm.tm_sec = ((n<<1)&037);
			f->st->st_mtime = tmtime(&tm, TM_LOCALZONE);
			f->linktype = PAX_NOLINK;
			f->linkpath = 0;
			if (ar->mem && (mem = (Mem_t*)dtmatch(ar->mem, f->name)))
			{
				f->st->st_size = mem->encoded;
				f->uncompressed = mem->decoded;
				ar->checksum = mem->checksum;
			}
			else
			{
				f->st->st_size = swapget(3, &hdr[ZIP_LOC_SIZ], 4);
				f->uncompressed = swapget(3, &hdr[ZIP_LOC_LEN], 4);
				ar->checksum = swapget(3, &hdr[ZIP_LOC_CRC], 4);
			}
			i = 0;
			if (swapget(3, &hdr[ZIP_LOC_FLG], 2) & 0x0001)
				i += sfsprintf(ar->method, sizeof(ar->method), "crypt-zip+SIZE=%I*u|", sizeof(f->st->st_size), f->st->st_size);
			if (!f->st->st_size || (m = swapget(3, &hdr[ZIP_LOC_METHOD], 2)) == ZIP_COPY || (pax->test & 2))
				i += sfsprintf(ar->method + i, sizeof(ar->method) - i, "copy");
			else
				i += sfsprintf(ar->method + i, sizeof(ar->method) - i, "zip-%u+SIZE=%I*u", m, sizeof(f->uncompressed), f->uncompressed);
			sfsprintf(ar->method + i, sizeof(ar->method) - i, "|%s", SUM);
			if (error_info.trace)
				(*pax->errorf)(NiL, pax, -1, "archive=%s name=%s method=%s", ap->name, f->name, ar->method);
			return 1;
		case ZIP_EXT_MAGIC:
			paxunread(pax, ap, hdr, num);
			if (paxread(pax, ap, NiL, (off_t)ZIP_EXT_HEADER, (off_t)0, 0) <= 0)
			{
				(*pax->errorf)(NiL, pax, 2, "%s: invalid %s format verification header", ap->name, ap->format->name);
				return 0;
			}
			break;
		default:
			paxunread(pax, ap, hdr, num);
			if (paxseek(pax, ap, (off_t)0, SEEK_CUR, 0) == ar->end)
				paxseek(pax, ap, (off_t)0, SEEK_END, 1);
			return 0;
		}
	}
	return 0;
}

static int
zip_getdata(Pax_t* pax, register Paxarchive_t* ap, register Paxfile_t* f, int fd)
{
	register Ar_t*	ar = (Ar_t*)ap->data;
	Sfio_t*		sp;
	off_t		pos;
	ssize_t		n;
	int		r;
	int		pop;
	Codexdata_t	sum;

	if (!f->st->st_size)
		return 1;
	pos = paxseek(pax, ap, 0, SEEK_CUR, 0) + f->st->st_size;
	r = -1;
	if (fd < 0)
		r = 1;
	else if (sp = paxpart(pax, ap, f->st->st_size))
	{
		if ((pop = codex(sp, ar->method, CODEX_DECODE, &ar->codexdisc, NiL)) < 0)
			(*pax->errorf)(NiL, pax, 2, "%s: %s: cannot decode method %s", ap->name, f->name, ar->method);
		else
		{
			for (;;)
			{
				if ((n = sfread(sp, pax->buf, sizeof(pax->buf))) < 0)
				{
					(*pax->errorf)(NiL, pax, 2, "%s: %s: unexpected EOF", ap->name, f->name);
					break;
				}
				else if (n == 0)
				{
					if (codexdata(sp, &sum) <= 0)
						(*pax->errorf)(NiL, pax, 2, "%s: %s: checksum discipline error", ap->name, f->name);
					else if (!paxchecksum(pax, ap, f, ar->checksum, sum.num))
						r = 1;
					break;
				}
				if (paxdata(pax, ap, f, fd, pax->buf, n))
					break;
			}
			codexpop(sp, pop);
		}
	}
	if (paxseek(pax, ap, pos, SEEK_SET, 0) != pos)
	{
		(*pax->errorf)(NiL, pax, 2, "%s: %s: cannot seek past %s format data", ap->name, f->name, ap->format->name);
		r = -1;
	}
	return r;
}

Paxformat_t	pax_zip_format =
{
	"zip",
	0,
	"zip 2.1 / PKZIP 2.04g archive.",
	0,
	PAX_ARCHIVE|PAX_DOS|PAX_NOHARDLINKS|PAX_KEEPSIZE|PAX_IN,
	PAX_DEFBUFFER,
	PAX_DEFBLOCKS,
	0,
	PAXNEXT(zip),
	0,
	zip_done,
	zip_getprologue,
	zip_getheader,
	zip_getdata,
};

PAXLIB(zip)
