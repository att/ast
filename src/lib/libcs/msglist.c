/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2011 AT&T Intellectual Property          *
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
 * list system call message
 */

#include "msglib.h"

#include <ccode.h>
#include <ctype.h>
#include <tm.h>

/*
 * print buffer s,n on sp
 */

#define PUT(c)	(r++,sfputc(sp,(c)))

static int
buffer(register Sfio_t* sp, register char* s, int n)
{
	register char*	se;
	register int	c;
	register int	r = 0;

	if (n < 0) return sfprintf(sp, " %p", s);
	se = s + n;
	PUT(' ');
	PUT('"');
	while (s < se)
	{
		c = *((unsigned char*)s++);
		if (iscntrl(c) || !isprint(c) || c == '"')
		{
			PUT('\\');
			switch (c)
			{
			case CC_bel:
				c = 'a';
				break;
			case '\b':
				c = 'b';
				break;
			case '\f':
				c = 'f';
				break;
			case '\n':
				c = 'n';
				break;
			case '\r':
				c = 'r';
				break;
			case '\t':
				c = 't';
				break;
			case CC_vt:
				c = 'v';
				break;
			case CC_esc:
				c = 'E';
				break;
			case '"':
				break;
			default:
				PUT('0' + ((c >> 6) & 03));
				PUT('0' + ((c >> 3) & 07));
				c = '0' + (c & 07);
				break;
			}
		}
		PUT(c);
	}
	PUT('"');
	return r;
}

/*
 * list msg on sp
 */

int
msglist(Sfio_t* sp, register Msg_call_t* msg, int flags, unsigned long terse)
{
	register unsigned long	at;
	register Msg_arg_t*	ap;
	register char*		p;
	register long		n;
	register int		r;
	int			i;
	long*			np;
	long*			ne;
	Msg_file_t*		ip;
	struct dirent*		dp;
	struct dirent*		de;
	struct statvfs*		fp;
	struct stat*		st;
	char**			vp;

	r = 0;
	if (flags & (MSG_LIST_ID|MSG_LIST_STAMP))
	{
		r += sfprintf(sp, "[");
		if (flags & MSG_LIST_ID)
			r += sfprintf(sp, " %5d %5d", MSG_CHANNEL_USR(msg->channel), MSG_CHANNEL_SYS(msg->channel));
		if (flags & MSG_LIST_STAMP)
		{
			if (((unsigned long)msg->stamp) <= USHRT_MAX)
				r += sfprintf(sp, " %lu", msg->stamp);
			else
				r += sfprintf(sp, " %s", fmttime("%?%K", (time_t)msg->stamp));
		}
		r += sfprintf(sp, " ] ");
	}
	r += sfprintf(sp, "%s (", msgname(msg->call));
	ap = msg->argv;
	at = msg->call >> MSG_ARG_CALL;
	for (;;)
	{
		switch ((at >>= MSG_ARG_TYPE) & ((1 << MSG_ARG_TYPE) - 1))
		{
		case MSG_ARG_array:
			n = (ap++)->number;
			np = (ap++)->array;
			r += sfprintf(sp, " [");
			ne = np + n;
			while (np < ne)
				r += sfprintf(sp, " %lu", *np++);
			r += sfprintf(sp, " ]");
			continue;
		case MSG_ARG_file:
			ip = (ap++)->file;
			r += sfprintf(sp, " %lu", ip->fid[0]);
			for (n = 1; n < sizeof(*ip) / sizeof(ip->fid[0]); n++)
				r += sfprintf(sp, "%s%ld", ip->fid[n] >= 0 ? "+" : "", ip->fid[n]);
			continue;
		case MSG_ARG_input:
			p = (ap++)->pointer;
			if (MSG_MASK(msg->call) & terse) r += sfprintf(sp, " %p", p);
			else r += buffer(sp, p, (ap)->number);
			continue;
		case MSG_ARG_output:
			if ((msg->call & MSG_VALUE) && !(MSG_MASK(msg->call) & terse))
			{
				switch (MSG_CALL(msg->call))
				{
				case MSG_CALL(MSG_getdents):
					dp = (struct dirent*)(ap++)->pointer;
					de = (struct dirent*)((char*)dp + msg->ret.number);
					r += sfprintf(sp, " [");
					while (dp < de)
					{
#if _mem_d_fileno_dirent
						r += sfprintf(sp, " %lu", dp->d_fileno);
#endif
						i = D_NAMLEN(dp);
						r += buffer(sp, dp->d_name, i);
#if _mem_d_reclen_dirent
						i = dp->d_reclen;
#else
						i = D_RECSIZ(dp, i);
#endif
						dp = (struct dirent*)((char*)dp + i);
					}
					r += sfprintf(sp, " ]");
					break;
				case MSG_CALL(MSG_pipe):
					ip = (ap++)->file;
					for (i = 0; i < 2; i++)
					{
						r += sfprintf(sp, " %lu", ip->fid[0]);
						for (n = 1; n < sizeof(*ip) / sizeof(ip->fid[0]); n++)
							r += sfprintf(sp, "%s%ld", ip->fid[n] >= 0 ? "+" : "", ip->fid[n]);
						ip++;
					}
					break;
				case MSG_CALL(MSG_stat):
					st = (struct stat*)(ap++)->pointer;
					r += sfprintf(sp, " [ dev=%d ino=%u mode=0%o nlink=%d uid=%d gid=%d size=%lu", st->st_dev, st->st_ino, st->st_mode, st->st_nlink, st->st_uid, st->st_gid, st->st_size);
#if _mem_st_blocks_stat
					r += sfprintf(sp, " blocks=%u", st->st_blocks);
#endif
#if _mem_st_blksize_stat
					r += sfprintf(sp, " blksize=%u", st->st_blksize);
#endif
					r += sfprintf(sp, " atime=%lu mtime=%lu ctime=%lu ]", st->st_atime, st->st_mtime, st->st_ctime);
					break;
				case MSG_CALL(MSG_statfs):
					fp = (struct statvfs*)(ap++)->pointer;
					r += sfprintf(sp, " [ bsize=%d frsize=%d blocks=%d bfree=%d bavail=%d files=%d ffree=%d favail=%d", fp->f_bsize, fp->f_frsize, fp->f_blocks, fp->f_bfree, fp->f_bavail, fp->f_files, fp->f_ffree, fp->f_favail);
#if _mem_f_fsid_statvfs
					r += sfprintf(sp, " fsid=%u", fp->f_fsid);
#endif
#if _mem_f_basetype_statvfs
					r += sfprintf(sp, " basetype=%s", fp->f_basetype);
#endif
					r += sfprintf(sp, " flag=%o namemax=%d", fp->f_flag, fp->f_namemax);
#if _mem_f_fstr_statvfs
					r += sfprintf(sp, " fstr=%s", fp->f_fstr);
#endif
					r += sfprintf(sp, " ]");
					break;
				default:
					p = (ap++)->pointer;
					r += buffer(sp, p, msg->ret.number);
					break;
				}
				continue;
			}
			/*FALLTHROUGH*/
		case MSG_ARG_number:
			n = (ap++)->number;
			r += sfprintf(sp, n > 0xffff ? " 0x%x" : " %lu", n);
			continue;
		case MSG_ARG_string:
			if (p = (ap++)->pointer) r += buffer(sp, p, strlen(p));
			else r += sfprintf(sp, " 0");
			continue;
		case MSG_ARG_vector:
			vp = (ap++)->vector;
			if (MSG_MASK(msg->call) & terse) r += sfprintf(sp, " %p", vp);
			else
			{
				r += sfprintf(sp, " [");
				while (p = *vp++)
					r += buffer(sp, p, strlen(p));
				r += sfprintf(sp, " ]");
			}
			continue;
		}
		break;
	}
	r += sfprintf(sp, " )");
	if (msg->call & MSG_VALUE) switch (MSG_ARG(msg->call, 0))
	{
	case 0:
		break;
	case MSG_ARG_file:
		ip = &msg->ret.file;
		r += sfprintf(sp, " = %lu", ip->fid[0]);
		for (n = 1; n < sizeof(*ip) / sizeof(ip->fid[0]); n++)
			r += sfprintf(sp, "%s%ld", ip->fid[n] >= 0 ? "+" : "", ip->fid[n]);
		break;
	default:
		r += sfprintf(sp, " = %ld", msg->ret.number);
		break;
	}
	if (msg->call & MSG_ACK)
		r += sfprintf(sp, " + ack ( 0x%08x %lu %lu )", msg->ack.addr, msg->ack.port, msg->stamp);
	r += sfprintf(sp, "\n");
	return r;
}
