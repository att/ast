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
 * syscall message server side receipt
 */

#include "msglib.h"

/*
 * blast message from msg->data,msg->size into msg->argv
 */

ssize_t
msgblast(register Msg_call_t* msg)
{
	register long		n;
	register unsigned long	at;
	register Msg_arg_t*	ap;
	long			r;
	char*			b;
	char*			e;
	long*			np;
	Msg_file_t*		ip;
	struct dirent*		dp;
	struct statvfs*		fp;
	struct stat*		sp;
	char**			vp = (char**)msg->value;

	b = msg->data + MSG_SIZE_SIZE;
	e = msg->data + msg->size;
	if ((msg->version = msggetu(&b, e)) != MSG_VERSION)
		return -1;
	msg->channel = msggetu(&b, e);
	msg->call = msggetu(&b, e);
	msg->stamp = msggetu(&b, e);
	if (msg->call & MSG_ACK)
	{
		msg->ack.addr = msggetu(&b, e);
		msg->ack.port = msggetu(&b, e);
	}
	at = msg->call >> MSG_ARG_CALL;
	if (msg->call & MSG_VALUE) switch (at & ((1 << MSG_ARG_TYPE) - 1))
	{
	case 0:
		break;
	case MSG_ARG_file:
		for (n = 0; n < sizeof(msg->ret.file) / sizeof(msg->ret.file.fid[0]); n++)
			msg->ret.file.fid[n] = msggetu(&b, e);
		break;
	default:
		msg->ret.number = msggetu(&b, e);
		break;
	}
	ap = msg->argv;
	while (ap < msg->argv + elementsof(msg->argv))
	{
		switch ((at >>= MSG_ARG_TYPE) & ((1 << MSG_ARG_TYPE) - 1))
		{
		case MSG_ARG_array:
			(ap++)->number = r = msggetu(&b, e);
			(ap++)->array = np = (long*)vp;
			while (r-- > 0)
			{
				n = msggetu(&b, e);
				if (np < (((long*)&msg->value[sizeof(msg->value)]) - 1))
					*np++ = n;
				if (e - b >= n) b += n;
			}
			if (np < (((long*)&msg->value[sizeof(msg->value)]) - 1)) np++;
			vp = (char**)np;
			continue;
		case MSG_ARG_file:
			(ap++)->file = ip = (Msg_file_t*)vp;
			for (n = 0; n < sizeof(*ip) / sizeof(ip->fid[0]); n++)
				ip->fid[n] = msggetu(&b, e);
			vp = (char**)(((char*)ip) + sizeof(*ip));
			continue;
		case MSG_ARG_input:
			n = msggetu(&b, e);
			(ap++)->pointer = b;
			(ap++)->number = n;
			if (e - b >= n) b += n;
			at >>= MSG_ARG_TYPE;
			continue;
		case MSG_ARG_number:
			(ap++)->number = msggetu(&b, e);
			continue;
		case MSG_ARG_output:
			if (msg->call & MSG_VALUE) switch (MSG_CALL(msg->call))
			{
			case MSG_CALL(MSG_getdents):
				(ap++)->pointer = (void*)(dp = (struct dirent*)vp);
				while (n = msggetz(&b, e, dp->d_name, sizeof(dp->d_name)))
				{
					n--;
#if _mem_d_namlen_dirent
					dp->d_namlen = n;
#endif
#if _mem_d_fileno_dirent
					dp->d_fileno =
#endif
					msggetu(&b, e);
#if _mem_d_off_dirent
					dp->d_off = 0;
#endif
					n = D_RECSIZ(dp, n);
#if _mem_d_reclen_dirent
					dp->d_reclen = n;
#endif
					dp = (struct dirent*)((char*)dp + n);
				}
				(ap++)->number = (char*)dp - (char*)vp;
				msggetu(&b, e);
				at >>= MSG_ARG_TYPE;
				break;
			case MSG_CALL(MSG_pipe):
				(ap++)->file = ip = (Msg_file_t*)vp;
				for (r = 0; r < 2; r++)
				{
					for (n = 0; n < sizeof(*ip) / sizeof(ip->fid[0]); n++)
						ip->fid[n] = msggetu(&b, e);
					ip++;
				}
				vp = (char**)ip;
				break;
			case MSG_CALL(MSG_stat):
				(ap++)->pointer = (void*)(sp = (struct stat*)vp);
				sp->st_dev = msggetu(&b, e);
				sp->st_ino = msggetu(&b, e);
				sp->st_mode = msggetu(&b, e);
				sp->st_nlink = msggetu(&b, e);
				sp->st_uid = msggetu(&b, e);
				sp->st_gid = msggetu(&b, e);
				sp->st_size = msggetu(&b, e);
				sp->st_atime = msggetu(&b, e);
				sp->st_mtime = msggetu(&b, e);
				sp->st_ctime = msggetu(&b, e);
#if _mem_st_blksize_stat
				sp->st_blksize =
#endif
					msggetu(&b, e);
#if _mem_st_blocks_stat
				sp->st_blocks =
#endif
					msggetu(&b, e);
				break;
			case MSG_CALL(MSG_statfs):
				(ap++)->pointer = (void*)(fp = (struct statvfs*)vp);
				fp->f_bsize = msggetu(&b, e);
				fp->f_frsize = msggetu(&b, e);
				fp->f_blocks = msggetu(&b, e);
				fp->f_bfree = msggetu(&b, e);
				fp->f_bavail = msggetu(&b, e);
				fp->f_files = msggetu(&b, e);
				fp->f_ffree = msggetu(&b, e);
				fp->f_favail = msggetu(&b, e);
#if _mem_f_fsid_statvfs
				fp->f_fsid =
#endif
					msggetu(&b, e);
#if _mem_f_basetype_statvfs
				msggetz(&b, e, fp->f_basetype, sizeof(fp->f_basetype));
#else
				msggetz(&b, e, NiL, 0);
#endif
				fp->f_flag = msggetu(&b, e);
				fp->f_namemax = msggetu(&b, e);
#if _mem_f_fstr_statvfs
				msggetz(&b, e, fp->f_fstr, sizeof(fp->f_fstr));
#endif
				break;
			default:
				n = msggetu(&b, e);
				(ap++)->pointer = (void*)b;
				(ap++)->number = n;
				at >>= MSG_ARG_TYPE;
				if (e - b >= n) b += n;
				break;
			}
			else (ap++)->pointer = 0;
			continue;
		case MSG_ARG_string:
			if (n = msggetu(&b, e))
			{
				(ap++)->string = b;
				if (e - b >= n) b += n;
			}
			else (ap++)->string = 0;
			continue;
		case MSG_ARG_vector:
			(ap++)->vector = vp;
			while (n = msggetu(&b, e))
			{
				if (vp < (((char**)&msg->value[sizeof(msg->value)]) - 1))
					*vp++ = b;
				if (e - b >= n) b += n;
			}
			*vp = 0;
			if (vp < (((char**)&msg->value[sizeof(msg->value)]) - 1)) vp++;
			continue;
		}
		break;
	}
	msg->argc = ap - msg->argv;
	return msg->size;
}
