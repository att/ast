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
 * syscall message server side send
 */

#include "msglib.h"

#include <cs.h>

/*
 * send a response to a msgrecv() message
 * message size is returned
 */

ssize_t
msgsend(int fd, register Msg_call_t* msg, unsigned long call, long ret, int err, void* data)
{
	register struct stat*		sp;
	register struct dirent*		dp;
	register struct dirent*		de;
	register struct statvfs*	vp;
	int				i;
	char*				b;
	char*				e;

	if (call & MSG_ACK)
	{
		if ((fd = csbind(&cs, "udp", msg->ack.addr, msg->ack.port, 0L)) < 0)
			return -1;
		ret = ret == -1 ? ~msg->stamp : msg->stamp;
		err = 0;
		data = 0;
	}
	b = msg->data + MSG_SIZE_SIZE;
	e = msg->data + sizeof(msg->data);
	msgputu(&b, e, call);
	msgputu(&b, e, ret);
	if (ret == -1) msgputu(&b, e, err);
	else if (data) switch (MSG_CALL(call))
	{
	case MSG_getdents:
		dp = (struct dirent*)data;
		de = (struct dirent*)((char*)dp + ret);
		while (dp < de)
		{
			i = D_NAMLEN(dp);
			msgputz(&b, e, dp->d_name, i + 1);
			msgputu(&b, e, D_FILENO(dp));
#if _mem_d_reclen_dirent
			i = dp->d_reclen;
#else
			i = D_RECSIZ(dp, i);
#endif
			dp = (struct dirent*)((char*)dp + i);
		}
		msgputu(&b, e, 0);
		break;
	case MSG_stat:
		sp = (struct stat*)data;
		msgputu(&b, e, sp->st_dev);
		msgputu(&b, e, sp->st_ino);
		msgputu(&b, e, sp->st_mode);
		msgputu(&b, e, sp->st_nlink);
		msgputu(&b, e, sp->st_uid);
		msgputu(&b, e, sp->st_gid);
		msgputu(&b, e, sp->st_size);
		msgputu(&b, e, sp->st_atime);
		msgputu(&b, e, sp->st_mtime);
		msgputu(&b, e, sp->st_ctime);
#if _mem_st_blksize_stat
		msgputu(&b, e, sp->st_blksize);
#else
		msgputu(&b, e, 1024);
#endif
#if _mem_st_blocks_stat
		msgputu(&b, e, sp->st_blocks);
#else
		msgputu(&b, e, sp->st_size ? ((sp->st_size - 1) / 1024 + 1) : 0);
#endif
		break;
	case MSG_statfs:
		vp = (struct statvfs*)data;
		msgputu(&b, e, vp->f_bsize);
		msgputu(&b, e, vp->f_frsize);
		msgputu(&b, e, vp->f_blocks);
		msgputu(&b, e, vp->f_bfree);
		msgputu(&b, e, vp->f_bavail);
		msgputu(&b, e, vp->f_files);
		msgputu(&b, e, vp->f_ffree);
		msgputu(&b, e, vp->f_favail);
#if _mem_f_fsid_statvfs
		msgputu(&b, e, vp->f_fsid);
#else
		msgputu(&b, e, 0);
#endif
#if _mem_f_basetype_statvfs
		msgputz(&b, e, vp->f_basetype, strlen(vp->f_basetype) + 1);
#else
		msgputz(&b, e, "ufs", 4);
#endif
		msgputu(&b, e, vp->f_flag);
		msgputu(&b, e, vp->f_namemax);
#if _mem_f_fstr_statvfs
		msgputz(&b, e, vp->f_fstr, strlen(vp->f_fstr) + 1);
#endif
		break;
	default:
		msgputz(&b, e, data, ret);
		break;
	}
	ret = b - msg->data;
	msgsetsize(msg->data, ret);
	if (cswrite(&cs, fd, msg->data, ret) != ret)
		ret = -1;
	if (call & MSG_ACK)
		close(fd);
	return ret;
}
