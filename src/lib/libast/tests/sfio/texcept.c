/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1999-2011 AT&T Intellectual Property          *
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
#include "config_ast.h"  // IWYU pragma: keep

#include <stddef.h>
#include <unistd.h>

#include "sfio.h"
#include "terror.h"

static int Type;
static int Sfn;

static int except(Sfio_t *f, int type, void *data, Sfdisc_t *disc) {
    UNUSED(f);
    UNUSED(data);
    UNUSED(disc);

    switch (type) {
        case SF_WRITE:
            Sfn += 1;
            return 0;
        case SF_CLOSING:
            if (Type == SF_CLOSING) return 0;
        // FALLTHRU
        case SF_SYNC:
            if (Type == SF_CLOSING) return 0;
            break;
        default:
            break;
    }

    Type = type;
    return -1;
}

static int except2(Sfio_t *f, int type, void *data, Sfdisc_t *disc) {
    UNUSED(f);
    UNUSED(type);
    UNUSED(data);
    UNUSED(disc);

    return 0;
}

static int except3(Sfio_t *f, int type, void *data, Sfdisc_t *disc) {
    UNUSED(f);
    UNUSED(data);
    UNUSED(disc);

    if (type == SF_LOCKED) {
        Type = type;
        return -1;
    }
    return 0;
}

static ssize_t readfunc(Sfio_t *f, void *buf, size_t n, Sfdisc_t *disc) {
    UNUSED(buf);
    UNUSED(n);
    UNUSED(disc);

    if (sfgetc(f) >= 0) terror("Can't access stream here!");
    return 0;
}

static Sfdisc_t Disc, Disc2;

tmain() {
    UNUSED(argc);
    UNUSED(argv);
    Sfio_t *f, *f2;
    char buf[1024];
    char rbuf[4 * 1024];
    off_t o;
    int i;

    if (!(f = sfopen(NULL, tstfile("sf", 0), "w"))) terror("Can't open file");
    sfset(f, SF_IOCHECK, 1);

    Disc.exceptf = except;
    if (!sfdisc(f, &Disc)) terror("Pushing discipline failed");

    sfdisc(f, &Disc);
    if (Type != SF_DPUSH) terror("Did not get push event");

    /* this is to test sfraise(NULL,...) */
    if (!(f2 = sfopen(NULL, tstfile("sf", 0), "w"))) terror("Can't open file");
    sfdisc(f2, &Disc);

    Sfn = 0;
    if (sfraise(0, SF_WRITE, 0) < 0) terror("sfraise failed");
    if (Sfn != 2) terror("Didn't get right event count");

    sfdisc(f, NULL);
    if (Type != SF_DPOP) terror("Did not get pop event");

    sfwrite(f, "123", 3);
    sfsync(f);
    if (Type != SF_SYNC) terror("Did not get sync event");

    sfwrite(f, "123", 3);
    sfpurge(f);
    if (Type != SF_PURGE) terror("Did not get purge event");

    sfclose(f);
    if (Type != SF_CLOSING) terror("Did not get close event");

    sfclose(f);
    if (Type != SF_FINAL) terror("Did not get final event");

    if (!(f = sfopen(NULL, tstfile("sf", 0), "r"))) terror("Can't open file");
    Disc2.readf = readfunc;
    Disc2.exceptf = except3;
    sfdisc(f, &Disc2);
    if (sfgetc(f) >= 0) terror("There should be no data here");
    if (Type != SF_LOCKED) terror("Did not get lock event");

    /* test to see if sfclose() preserves seek location */
    if (!(f = sftmp(0))) terror("Can't create temp file");
    sfsetbuf(f, buf, sizeof(buf));
    for (i = 0; i < sizeof(rbuf); ++i) rbuf[i] = i;
    sfwrite(f, rbuf, sizeof(rbuf));
    sfset(f, SF_WRITE, 0);

    Disc.exceptf = except2;
    sfdisc(f, &Disc);
    sfseek(f, (Sfoff_t)0, 0);
    if (sfread(f, rbuf, 4) != 4) terror("reading 4 bytes");
    for (i = 0; i < 4; ++i) {
        if (rbuf[i] != i) terror("wrong 4 bytes");
    }

    sfsync(f);
    if ((o = lseek(sffileno(f), (off_t)0, SEEK_CUR)) != 4) {
        terror("Wrong seek location %lld", (Sfoff_t)o);
    }

    if ((i = dup(sffileno(f))) < 0) terror("Can't dup file descriptor");
    if ((o = lseek(i, (off_t)0, SEEK_CUR)) != 4) terror("Wrong seek location %lld", (Sfoff_t)o);

    sfclose(f);
    if ((o = lseek(i, (off_t)0, SEEK_CUR)) != 4) terror("Wrong seek location %lld", (Sfoff_t)o);

    texit(0);
}
