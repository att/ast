/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2013 AT&T Intellectual Property          *
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
 * dss file support
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "dsshdr.h"

#include <ls.h>
#include <pzip.h>

/*
 * not open for read
 */

static int
noreadf(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	if (disc->errorf)
		(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: not open for read", file->path);
	return -1;
}

/*
 * empty input
 */

static int
nullreadf(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	return 0;
}

/*
 * not open for write
 */

static int
nowritef(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	if (disc->errorf)
		(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: not open for write", file->path);
	return -1;
}

/*
 * file open
 */

Dssfile_t*
dssfopen(Dss_t* dss, const char* path, Sfio_t* io, Dssflags_t flags, Dssformat_t* format)
{
	Dssfile_t*	file;
	Vmalloc_t*	vm;
	char*		s;
	size_t		n;
	int		i;
	struct stat	st;
	Sfdisc_t	top;
	char		buf[PATH_MAX];

	if (flags & DSS_FILE_WRITE)
	{
		if (io)
		{
			memset(&top, 0, sizeof(top));
			if (sfdisc(io, &top))
			{
				n = top.disc == &dss->state->compress_preferred;
				sfdisc(io, SF_POPDISC);
				if (n)
				{
					sfdisc(io, SF_POPDISC);
					sfdczip(io, path, dss->meth->compress ? dss->meth->compress : "gzip", dss->disc->errorf);
				}
			}
		}
		if (dss->flags & DSS_APPEND)
			flags |= DSS_FILE_APPEND;
	}
	if (!path || !*path || streq(path, "-"))
	{
		if (flags & DSS_FILE_WRITE)
		{
			if (io)
				path = "output-stream";
			else
			{
				path = "/dev/stdout";
				io = sfstdout;
			}
		}
		else if (io)
			path = "input-stream";
		else
		{
			path = "/dev/stdin";
			io = sfstdin;
		}
		flags |= DSS_FILE_KEEP;
	}
	else if (io)
		flags |= DSS_FILE_KEEP;
	else if (flags & DSS_FILE_WRITE)
	{
		if (!(io = sfopen(NiL, path, (flags & DSS_FILE_APPEND) ? "a" : "w")))
		{
			if (dss->disc->errorf)
				(*dss->disc->errorf)(NiL, dss->disc, ERROR_SYSTEM|2, "%s: cannot open", path);
			return 0;
		}
	}
	else if (!(io = dssfind(path, "", DSS_VERBOSE, buf, sizeof(buf), dss->disc)))
		return 0;
	else
		path = (const char*)buf;
	if (!(vm = vmopen(Vmdcheap, Vmbest, 0)))
	{
		if (dss->disc->errorf)
			(*dss->disc->errorf)(NiL, dss->disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	if (!(file = vmnewof(vm, 0, Dssfile_t, 1, strlen(path) + 1)))
	{
		if (dss->disc->errorf)
			(*dss->disc->errorf)(NiL, dss->disc, ERROR_SYSTEM|2, "out of space");
		if (!(flags & DSS_FILE_KEEP))
			sfclose(io);
		vmclose(vm);
		return 0;
	}
	strcpy(file->path = (char*)(file + 1), path);
	file->dss = dss;
	file->vm = vm;
	file->io = io;
	file->flags = flags;
	if (flags & DSS_FILE_WRITE)
	{
		if (!(file->format = format) && !(file->format = dss->format))
		{
			if (dss->disc->errorf)
				(*dss->disc->errorf)(NiL, dss->disc, 2, "output method format must be specified");
			if (!(flags & DSS_FILE_KEEP))
				sfclose(io);
			return 0;
		}
		file->readf = noreadf;
		file->writef = file->format->writef;
	}
	else
	{
		if (sfsize(file->io) || !fstat(sffileno(file->io), &st) && (S_ISFIFO(st.st_mode)
#ifdef S_ISSOCK
			|| S_ISSOCK(st.st_mode)
#endif
			))
		{
			if (sfdczip(file->io, file->path, NiL, dss->disc->errorf) < 0)
			{
				if (dss->disc->errorf)
					(*dss->disc->errorf)(NiL, dss->disc, ERROR_SYSTEM|2, "%s: inflate error", file->path);
				dssfclose(file);
				return 0;
			}
			s = sfreserve(file->io, SF_UNBOUND, SF_LOCKR);
			n = sfvalue(file->io);
			if (!s)
			{
				if (n && dss->disc->errorf)
					(*dss->disc->errorf)(NiL, dss->disc, ERROR_SYSTEM|2, "%s: cannot peek", file->path);
				dssfclose(file);
				return 0;
			}
			for (file->format = (Dssformat_t*)dtfirst(dss->meth->formats); file->format && !(i = (*file->format->identf)(file, s, n, dss->disc)); file->format = (Dssformat_t*)dtnext(dss->meth->formats, file->format));
			sfread(file->io, s, 0);
			if (!file->format)
			{
				if (dss->disc->errorf)
					(*dss->disc->errorf)(NiL, dss->disc, 2, "%s: unknown %s format", file->path, dss->meth->name);
				dssfclose(file);
				return 0;
			}
			if (i < 0)
				return 0;
			if (format && format != file->format)
			{
				if (dss->disc->errorf)
					(*dss->disc->errorf)(NiL, dss->disc, 2, "%s: %s file format %s incompatible with %s", file->path, dss->meth->name, file->format->name, format->name);
				dssfclose(file);
				return 0;
			}
			if ((dss->flags & DSS_VERBOSE) && dss->disc->errorf)
				(*dss->disc->errorf)(dss, dss->disc, 1, "%s: %s method %s format", file->path, dss->meth->name, file->format->name);
			file->readf = file->format->readf;
		}
		else
		{
			file->format = format ? format : dss->format ? dss->format : (Dssformat_t*)dtfirst(dss->meth->formats);
			file->readf = nullreadf;
		}
		file->writef = nowritef;
		if (!dss->format)
			dss->format = file->format;
	}
	if (!file->format)
	{
		if (dss->disc->errorf)
			(*dss->disc->errorf)(NiL, dss->disc, 2, "%s: %s method did not set file format", file->path, dss->meth->name);
		dssfclose(file);
		return 0;
	}
	file->record.file = file;
	if ((*file->format->openf)(file, dss->disc))
	{
		dssfclose(file);
		return 0;
	}
	return file;
}

/*
 * file close
 */

int
dssfclose(Dssfile_t* file)
{
	int		r;
	Dss_t*		dss;

	if (!file)
		return -1;
	dss = file->dss;
	if (!file->io)
		r = -1;
	else
	{
		r = file->format ? (*file->format->closef)(file, dss->disc) : 0;
		if ((file->flags & DSS_FILE_WRITE) && sfsync(file->io))
		{
			if (dss->disc->errorf)
				(*dss->disc->errorf)(NiL, dss->disc, ERROR_SYSTEM|2, "%s: write error", file->path);
			r = -1;
		}
		if (!(file->flags & DSS_FILE_KEEP))
			sfclose(file->io);
		if (!r && (file->flags & DSS_FILE_ERROR))
			r = -1;
	}
	vmclose(file->vm);
	return r;
}

/*
 * file read
 */

Dssrecord_t*
dssfread(Dssfile_t* file)
{
	int	r;

	file->count++;
	file->offset += file->length;
	if ((r = (*file->readf)(file, &file->record, file->dss->disc)) <= 0)
	{
		if (r < 0)
			file->flags |= DSS_FILE_ERROR;
		file->count--;
		return 0;
	}
	file->length = sftell(file->io) - file->offset;
	return &file->record;
}

/*
 * file write
 */

int
dssfwrite(Dssfile_t* file, Dssrecord_t* record)
{
	return (*file->writef)(file, record, file->dss->disc);
}

/*
 * file tell
 */

Sfoff_t
dssftell(Dssfile_t* file)
{
	return file->seekf ? (*file->seekf)(file, DSS_TELL, file->dss->disc) : file->offset;
}

/*
 * file seek
 */

int
dssfseek(Dssfile_t* file, Sfoff_t offset)
{
	return (file->seekf ? (*file->seekf)(file, offset, file->dss->disc) : sfseek(file->io, offset, SEEK_SET)) == offset ? 0 : -1;
}

/*
 * save record state
 */

Dssrecord_t*
dsssave(Dssrecord_t* record)
{
	return record->file->format->savef ? (*record->file->format->savef)(record->file, record, record->file->dss->disc) : (Dssrecord_t*)0;
}

/*
 * drop saved record
 */

int
dssdrop(Dssrecord_t* record)
{
	return record->file->format->dropf ? (*record->file->format->dropf)(record->file, record, record->file->dss->disc) : -1;
}

/*
 * common diagnostics
 */

Dssrecord_t*
dss_no_fread(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	if (disc->errorf)
		(*disc->errorf)(NiL, disc, 2, "%s: %s file format %s read not implemented", file->path, file->dss->meth->name, file->format->name);
	return 0;
}

int	
dss_no_fwrite(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	if (disc->errorf)
		(*disc->errorf)(NiL, disc, 2, "%s: %s file format %s write not implemented", file->path, file->dss->meth->name, file->format->name);
	return -1;
}

Sfoff_t
dss_no_fseek(Dssfile_t* file, Sfoff_t offset, Dssdisc_t* disc)
{
	if (disc->errorf)
		(*disc->errorf)(NiL, disc, 2, "%s: %s file format %s seek not implemented", file->path, file->dss->meth->name, file->format->name);
	return -1;
}
