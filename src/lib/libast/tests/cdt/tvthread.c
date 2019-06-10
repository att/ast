/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1999-2013 AT&T Intellectual Property          *
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

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "aso.h"
#include "cdt.h"
#include "terror.h"
#include "tm.h"

/* Test concurrency by volleying objects between two dictionaries. */

#define N_THREADS 8  /* #players        */
#define N_OBJ 100000 /* #objects        */

/* an object to be bounced back and forth */
typedef struct obj_s {
    int value;  /* decimal value                */
    int ins[2]; /* #insertion into Dict[0,1]    */
    int del[2]; /* #deletion from Dict[0,1]     */
} Obj_t;

/* Cdt discipline to allocate memory */
typedef struct _mydisc_s {
    Dtdisc_t disc; /* cdt discipline            */
} Mydisc_t;

static Dt_t *Dict[2];
static int Nthreads;

/* allocate data from the shared memory region */
void *mymemory(Dt_t *dt, void *data, size_t size, Dtdisc_t *disc) {
    UNUSED(dt);
    UNUSED(disc);

    return realloc(data, size);
}

/* compare two objects by their integer keys */
static int mycompare(Dt_t *dt, void *key1, void *key2, Dtdisc_t *disc) {
    UNUSED(dt);
    UNUSED(disc);

    return *((int *)key1) - *((int *)key2);
}

unsigned int myhash(Dt_t *dt, void *key, Dtdisc_t *disc) {
    UNUSED(dt);
    UNUSED(disc);

    return *((unsigned *)key);
}

// Open a shared dictionary.
static Dt_t *opendictionary(Mydisc_t *dc) {
    // Discipline for objects identified by their decimal values.
    dc->disc.key = DTOFFSET(Obj_t, value);
    dc->disc.size = sizeof(int);
    dc->disc.link = -1;
    dc->disc.makef = NULL;
    dc->disc.freef = NULL;
    dc->disc.comparf = mycompare;
    dc->disc.hashf = myhash;
    dc->disc.memoryf = mymemory;
    dc->disc.eventf = NULL;

    Dt_t *dt = dtopen(&dc->disc, Dtrhset);  // open dictionary with hash-trie
    if (!dt) terror("Can't open dictionary");
    dtcustomize(dt, DT_SHARE, 1);  // turn on concurrent access mode
    return dt;
}

/* swap objects from one dictionary to another */
static void *volley(void *arg) {
    int deldt, insdt, n_move, dir, k;
    Obj_t obj, *o, *rv;

    Nthreads += 1; /* wait until all threads have been started */
    while (Nthreads < N_THREADS) tmsleep(0, 1);

    if ((deldt = (int)((long)arg)) < 0 || deldt > 1) {
        terror("Thread number must be 0 or 1, not %d", deldt);
    }
    insdt = !deldt;

    n_move = 0;
    for (dir = 1; dir >= -1; dir -= 2) {
        for (k = dir > 0 ? 0 : N_OBJ - 1; k >= 0 && k < N_OBJ; k += dir) {
            obj.value = k;
            if (!(o = dtsearch(Dict[deldt], &obj))) continue;

            if ((rv = dtdelete(Dict[deldt], o)) == o) {
                asoincint(&o->del[deldt]);

                if ((rv = dtinsert(Dict[insdt], o)) != o) terror("Insert %d failed", o->value);

                asoincint(&o->ins[insdt]);

                n_move += 1;
            } else if (rv) {
                terror("Unknown object %d", rv->value);
            }

            if (k % 100 == 0) tmsleep(0, 1);
        }
    }
    tinfo("Move %d (Dict[%d] -> Dict[%d])", n_move, deldt, insdt);

    return 0;
}

tmain() {
    UNUSED(argc);
    UNUSED(argv);
    pthread_t thread[N_THREADS];
    size_t k, p, n;
    Mydisc_t disc[2];
    Obj_t *o, *list[2], obj;

    topts();

    /* create two dictionaries to volley objects back and forth */
    for (n = 0; n < 2; ++n) {
        if (!(Dict[n] = opendictionary(&disc[n]))) terror("Can't open dictionary %d", n);

        /* make objects */
        if (!(list[n] = malloc((N_OBJ / 2) * sizeof(Obj_t)))) terror("malloc failed %d", n);
        memset(list[n], 0, (N_OBJ / 2) * sizeof(Obj_t));

        for (o = list[n], k = 0; k < N_OBJ / 2; ++k, ++o) {
            o->value = n == 0 ? k : k + N_OBJ / 2;

            if (dtinsert(Dict[n], o) != o) terror("Insert failed n=%d k=%d", n, k);
            if (dtsearch(Dict[n], o) != o) { /* verify insert succeeded */
                terror("Search failed n=%d k=%d", n, k);
            }

            o->ins[n] += 1;
        }
    }

    for (p = 0; p < N_THREADS; ++p) pthread_create(&thread[p], 0, volley, (void *)((long)(p % 2)));

    for (p = 0; p < N_THREADS; ++p) pthread_join(thread[p], 0);

    tinfo("\tCheck integrity");
    n = dtsize(Dict[0]);
    p = dtsize(Dict[1]);
    tinfo("Dict[0]=%d Dict[1]=%d", n, p);
    if ((n + p) != N_OBJ) {
        for (k = 0; k < N_OBJ; ++k) {
            obj.value = k;
            if ((o = dtsearch(Dict[0], &obj))) continue;
            if ((o = dtsearch(Dict[1], &obj))) continue;
            terror("%d not found", k);
            dtsearch(Dict[0], &obj);
            dtsearch(Dict[1], &obj);
        }

        terror("Expecting %d objects but got (Dict[0]=%d + Dict[1]=%d) = %d", N_OBJ, n, p, n + p);
    }

    texit(0);
}
