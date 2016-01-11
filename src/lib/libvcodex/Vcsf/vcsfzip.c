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
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
# include	"vchdr.h"
#include	"vcsfio.h"
#include	<zlib.h>
#include	<sys/types.h>
#include	<sys/wait.h>
#include	<unistd.h>

/*	Zipping and Unzipping files.
	Written by Kiem-Phong Vo
*/

#define	ZTRAINZ		(4*128*1024)	/* training data size	*/

/* Gzip magic header bytes */
#define GZ_HEADER0	0x1f
#define GZ_HEADER1	0x8b

/* selector for different transform sequences */
typedef struct _vcz_s
{	char*	trans;	/* data tranformation sequence		*/
	double	factor;	/* improvement over zlib before picking	*/
} Vcz_t;

static Vcz_t	Vcz[] = /* transform sequences to pick from	*/
{	{ "transpose,rle,huffman", 0.90 },
	{ "ama,transpose,rle,huffman", 0.75 },

	{ "ama,table,rle,huffman", 0.70 },
	{ "table,mtf,rle.0,huffgroup", 0.60 },
	{ "ama,table,mtf,rle.0,huffgroup", 0.60 },

	{ "ama.nl,transpose,rle,huffman", 0.60 },
	{ "ama.nl,table,mtf,rle.0,huffgroup", 0.60 },

	{ "rtable,mtf,rle.0,huffgroup", 0.50 },

	{ 0, 0.0 }
};

static char	*Sfx[] = /* allowed suffixes */
{	"gz",	/* Gzip */
	"vz",	/* Vczip */
	0
};

static char* zpick(Vcchar_t* data, ssize_t dtsz)
{
	Vcodex_t	*vc;
	ssize_t		bestcz, bestdz, zlibz, cz, dz;
	int		bestk, k;
#define ZSLACK		64 /* additional space for any header data */
	Vcchar_t	dt[ZTRAINZ+ZSLACK];
	Sfio_t		*sf = NIL(Sfio_t*); /* temp stream */
	gzFile		zf =  NIL(gzFile); /* zlib stream */

	if(!data || dtsz <= 0)
		return NIL(char*);
	if(dtsz > ZTRAINZ)
		dtsz = ZTRAINZ;

	zlibz = dtsz; /* compute gzip compress size */
	if((sf = sftmp(0)) != NIL(Sfio_t*) &&
	   (k  = dup(sffileno(sf))) >= 0 &&
	   (zf = gzdopen(k, "wb")) != NIL(gzFile) &&
	   gzwrite(zf, data, dtsz) == dtsz )
	{	gzclose(zf); close(k);
		zf = NIL(gzFile); /* write out compressed data */	
		sfseek(sf, (off_t)0, 0);
		if((zlibz = sfread(sf, dt, dtsz+ZSLACK)) <= 0)
			zlibz = dtsz;
	}
	if(sf)
		sfclose(sf);
	if(zf)
		gzclose(zf);

	/* compute the best choice */
	bestcz = bestdz = dtsz; bestk = -1;
	for(k = 0; Vcz[k].trans; ++k)
	{	if(!(vc = vcmake(Vcz[k].trans, VC_ENCODE)) )
			continue;

		/* run compression transforms */
		cz = vcapply(vc, data, dtsz, NIL(Void_t*) );
		dz = dtsz - vcundone(vc); /* amount actually processed */
		vcclose(vc);

		/* see if improving over current choice */
		if(cz > 0 && dz > dtsz/4 &&
		   ((double)cz)/dz < (Vcz[k].factor*zlibz)/dtsz &&
		   ((double)cz)/dz < ((double)bestcz)/bestdz )
		{	bestcz = cz;
			bestdz = dz;
			bestk  = k;
			if((2.5*bestcz)/bestdz < ((double)zlibz)/dtsz )
				break;
		}
	}

	return bestk >= 0 ? Vcz[bestk].trans : NIL(char*);
}


/* set binary, unshared I/O modes for read && write streams */
static int iomode(Sfio_t* rf, Sfio_t* wf)
{
	Vcchar_t	*dt;

	if(rf)
	{	if(sfsize(rf) == 0) /* see if this is an empty stream */
		{	if(!(dt = (Vcchar_t*)sfreserve(rf, -1, SF_LOCKR)) || sfvalue(rf) == 0 )
				return 0; /* do nothing here */
			sfread(rf, dt, 0); /* reset IO pointer */
		}
		sfset(rf, SF_SHARE, 0); /* avoid peeking on unseekable devices */
#if _WIN32 /* on Windows systems, use binary mode for file I/O */
		setmode(sffileno(rf), O_BINARY);
#endif
	}

	if(wf)
	{	sfset(wf, SF_SHARE, 0);
#if _WIN32 /* on Windows systems, use binary mode for file I/O */
		setmode(sffileno(wf), O_BINARY);
#endif
	}

	return 1;
}


/* compress a file */
#if __STD_C
ssize_t vcsfzip(char* ifile, Vcsfdata_t* sfdt, int keep)
#else
ssize_t vcsfzip(ifile, sfdt, keep)
char*		ifile;	/* input file, NULL for stdin 	*/
Vcsfdata_t*	sfdt;	/* parameters for compression	*/
int		keep;	/* !0 if keeping original file	*/
#endif
{
	ssize_t		dtsz, sz, psz;
	Vcchar_t	data[ZTRAINZ], *dt;
	char		*trans, *sfx;
	gzFile		gzf = NIL(gzFile);
	Vcsfio_t	*vzf = NIL(Vcsfio_t*);
	Sfio_t		*rf = NIL(Sfio_t*), *wf = NIL(Sfio_t*);
	ssize_t		rv = -1;

	if(!sfdt)
		return -1;

	/* default suffix is "vz" */
	sfx = (sfdt->type&(VCSF_GZSUFFIX)) ? "gz" : "vz";

	if(!ifile) /* using standard input/output */
	{	rf = sfstdin;
		wf = sfstdout;
		if(iomode(rf,wf) == 0) /* there is no data */
			return 0;
	}
	else /* open input/output files */
	{	if(!(rf = sfopen(0, ifile, "rb")) )
			goto done;

		/* name of output file */
		sprintf((char*)data, "%s.%s", ifile, sfx);

		/* if output file already exists, check if we should overwrite */
		if((wf = sfopen(0, (char*)data, "r")) != NIL(Sfio_t*) )
		{	sfclose(wf);
			if(sfdt->eventf && (*sfdt->eventf)(sfdt, (char*)data, VCSF_OVERWRITE) < 0 )
				goto done;
		}

		if(!(wf = sfopen(0, (char*)data, "wb")) )
			goto done;
	}

	/* read one buffer of data */
	if((dtsz = sfread(rf, data, sizeof(data))) < 0 )
		goto done;
	if(!sfdt->ppvc) /* run data through preprocessing */
		{ sz = dtsz; dt = data; }
	else if((sz = vcapply(sfdt->ppvc, data, dtsz, &dt)) < 0)
		goto done;

	if(sfdt->type&(VCSF_GZIP|VCSF_GZONLY)) /* GZIP is involved */
	{	if(sz > 0 && !(sfdt->type&VCSF_GZONLY))
			trans = zpick(dt, sz);
		else	trans = NIL(char*);
	}
	else /* only vczip methods */
	{	if(!(trans = sfdt->trans) )
			goto done;
	}

	if(trans) /* compressing with vcodex methods */
	{	sfdt->type &= ~(VCSF_GZIP|VCSF_GZONLY|VCSF_GZSUFFIX);
		sfdt->trans = trans;	
		if(!(vzf = vcsfio(wf, sfdt, VC_ENCODE)) )
			goto done;
	}
	else /* compression with gzip */
	{	if(!(gzf = gzdopen(sffileno(wf), "wb")) )
			goto done;
	}

	for(psz = 0; sz > 0; )
	{	psz += sz; /* tally amount processed */

		sz = vzf ? vcsfwrite(vzf, dt, sz) : gzwrite(gzf, dt, sz);
		if(sfdt->ppvc) /* clear pre-processor buffer */
			vcbuffer(sfdt->ppvc, NIL(Vcchar_t*), -1, -1);
		if(sz < 0)
			goto done;

		/* read new data */
		if((dtsz = sfread(rf, data, sizeof(data))) < 0)
			goto done;
		if(!sfdt->ppvc) /* run data through preprocessing */
			{ sz = dtsz; dt = data; }
		else if((sz = vcapply(sfdt->ppvc, data, dtsz, &dt)) < 0)
			goto done;
	}
	rv = psz < 0 ? 0 : psz; /* successful */

done:
	if(gzf)
		gzclose(gzf);
	if(vzf)
		vcsfclose(vzf);
	if(rf && rf != sfstdin)
		sfclose(rf);
	if(wf && wf != sfstdout && !vzf)
		sfclose(wf);

	if(rv < 0 && sfdt->eventf)
		(*sfdt->eventf)(sfdt, "Error in processing data", VCSF_ERROR);

	if(rv >= 0 && ifile && !keep)
		(void)unlink(ifile);

	return rv;
}

#if __STD_C
ssize_t vcsfunzip(char* ifile, Vcsfdata_t* sfdt, Vcchar_t* undt, ssize_t unsz, int keep)
#else
ssize_t vcsfunzip(ifile, sfdt, undt, unsz, keep)
char*		ifile;	/* input data to be decoded	*/
Vcsfdata_t*	sfdt;	/* info to pass to vcsfio()	*/
Vcchar_t*	undt;	/* if !NULL, store data here	*/
ssize_t		unsz;	/* size of the buffer undt	*/
int		keep;	/* !0 to remove ifile when done	*/
#endif
{
	ssize_t		n, k, dtsz, psz;
	Vcchar_t	*dt, data[ZTRAINZ], head[2], cfile[8*1024];
	pid_t		pid = -1;
	gzFile		gzf = NIL(gzFile);
	Vcsfio_t	*vzf = NIL(Vcsfio_t*);
	Sfio_t		*rf = NIL(Sfio_t*), *wf = NIL(Sfio_t*);
	int		rv = -1;

	if(!ifile)
	{	if(undt && unsz > 0)
			return -1;
		undt = NIL(Vcchar_t*); unsz = 0;

		rf = sfstdin;
		wf = sfstdout;
		if(iomode(rf,wf) == 0) /* empty data */
			return 0;
	}
	else if(!undt || unsz <= 0)
	{	/* compute the output file name */
		dtsz = 0;
		for(n = strlen(ifile); n > 1; --n)
		{	if(ifile[n-1] == '.') /* find the suffix */
			{	for(k = 0; Sfx[k]; ++k)
					if(strcmp(ifile+n, Sfx[k]) == 0)
						break;
				if(Sfx[k])
				{	dtsz = n-1;
					break;
				}
			}
			else if(ifile[n-1] == '/')
				break;
		}

		if(dtsz > 0) /* a good compressed data file name */
		{	if(!(rf = sfopen(0, ifile, "rb")) )
				GOTO(done);

			/* name of output file */
			if(dtsz >= sizeof(data) )
				GOTO(done);
			memcpy(data, ifile, dtsz);
			data[dtsz] = 0;
		}
		else /* see if there is a file with a correct suffix */
		{	for(rf = NIL(Sfio_t*), k = 0; Sfx[k]; ++k )
			{	sprintf((char*)data, "%s.%s", ifile, Sfx[k]);
				if((rf = sfopen(0, (char*)data, "rb")) != NIL(Sfio_t*))
					break;
			}
			if(!rf) /* no valid input file found */
				GOTO(done);

			/* the output file name was actually given in 'ifile' */
			strcpy((char*)cfile, (char*)data); /* save input file name */
			strcpy((char*)data, ifile); /* set actual output file */
			ifile = (char*)cfile; /* point ifile to proper name */
		}

		/* if output file already exists, check if we should overwrite */
		if((wf = sfopen(0, (char*)data, "r")) != NIL(Sfio_t*) )
		{	sfclose(wf); wf = NIL(Sfio_t*);
			if(sfdt->eventf && (*sfdt->eventf)(sfdt, (char*)data, VCSF_OVERWRITE) < 0 )
			{	rv = 0; /* return as if success to avoid error messages */
				ifile = NIL(char*);
				GOTO(done);
			}
		}

		if(!(wf = sfopen(0, (char*)data, "wb")) )
			GOTO(done);

		undt = NIL(Vcchar_t*); unsz = 0;
	}

	/* read one buffer of data */
	if((n = sfread(rf, head, sizeof(head))) < 0 )
		GOTO(done);

	psz = 0; /* accumulate amount decompressed */
	if(n < 2 || (int)head[0] != GZ_HEADER0 || (int)head[1] != GZ_HEADER1 )
	{	/* not gzip format - try vcodex methods */	
		sfdt->initdc = head;
		sfdt->initsz = n;
		if(!(vzf = vcsfio(rf, sfdt, VC_DECODE)) )
			GOTO(done);	

		psz = 0;
		while((dtsz = vcsfread(vzf, data, sizeof(data))) > 0)
		{	if(!sfdt->ppvc) /* perform post-processing if necessary */
				{ dt = data; n = dtsz; }
			else if((n = vcapply(sfdt->ppvc, data, dtsz, &dt)) <= 0)
				GOTO(done);

			if(undt)
			{	if(n > unsz)
					n = unsz;
				memcpy(undt, dt, n);
				undt += n; unsz -= n;
			}
			else
			{	if(sfwrite(wf, dt, n) != n)
					GOTO(done);
			}
			if(psz >= 0) /* not yet wrapped around */
				psz += n;

			if(sfdt->ppvc) /* free post-processed data */
				vcbuffer(sfdt->ppvc, dt, -1, -1);
		}
	}
	else if(lseek(sffileno(rf), (long)0, 0) == 0 ) /* gzip and seekable input file */
	{	if(!(gzf = gzdopen(sffileno(rf), "rb")) ) /* just normal processing */
			GOTO(done);
		while((dtsz = gzread(gzf, data, sizeof(data))) > 0)
		{	if(!sfdt->ppvc) /* perform post-processing if necessary */
				{ dt = data; n = dtsz; }
			else if((n = vcapply(sfdt->ppvc, data, dtsz, &dt)) <= 0)
				GOTO(done);

			if(undt)
			{	if(n > unsz)
					n = unsz;
				memcpy(undt, dt, n);
				undt += n; unsz -= n;
			}
			else
			{	if(sfwrite(wf, dt, n) != n)
					GOTO(done);
			}
			if(psz >= 0) /* if not already wrapped around */
				psz += n;

			if(sfdt->ppvc) /* free post-processed data */
				vcbuffer(sfdt->ppvc, dt, -1, -1);
		}
	}
	else /* unseekable input file. Must fork a subprocess to pass "intact" data */
	{	int	pp[2];
		if(pipe(pp) < 0)
			GOTO(done);
		if((pid = fork()) < 0)
		{	close(pp[0]); close(pp[1]);
			GOTO(done);
		}
		else if(pid == 0) /* child process does the real work */
		{	if(!(gzf = gzdopen(pp[0], "rb")) )
				exit(-1);
			while((dtsz = gzread(gzf, data, sizeof(data))) > 0)
			{	if(!sfdt->ppvc) /* perform post-processing if necessary */
					{ dt = data; n = dtsz; }
				else if((n = vcapply(sfdt->ppvc, data, dtsz, &dt)) <= 0)
					exit(-1);
				if(sfwrite(wf, dt, n) != n)
					exit(-1);
				if(sfdt->ppvc) /* free post-processed data */
					vcbuffer(sfdt->ppvc, dt, -1, -1);
			}
			exit(0);
		}
		else /* parent process just passes data to child */
		{	close(pp[0]);
			memcpy(data, head, n); dtsz = n;
			while(dtsz > 0)
			{	if(write(pp[1], data, dtsz) != dtsz)
				{	close(pp[1]);
					GOTO(done);
				}
				dtsz = sfread(rf, data, sizeof(data));
			}
			close(pp[1]);
		}
	}

	if((rv = psz) < 0) /* success but count wrapped around */
		rv = 0;

done:
	if(pid >= 0) /* prevent orphans */
		waitpid(pid, 0, 0);

	if(gzf)
		gzclose(gzf);
	if(vzf)
		vcsfclose(vzf);
	if(rf && !vzf)
		sfclose(rf);
	if(wf)
		sfclose(wf);

	if(ifile && !keep && rv >= 0)
		(void)unlink(ifile);

	return rv;
}
