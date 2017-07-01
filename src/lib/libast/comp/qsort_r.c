/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2013 AT&T Intellectual Property          *
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
*                    David Korn <dgkorn@gmail.com>                     *
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
   qsort_r

   J. L. Bentley and M. D. McIlroy. Engineering a sort function.
   Software---Practice and Experience, 23(11):1249-1265.
*/

#include <ast.h>

#if _lib_qsort_r

void _STUB_qsort_r(){}

#else

typedef long WORD;
#define W sizeof(WORD)   /* must be a power of 2 */
#define SWAPINIT(a, es) swaptype =                \
   (a-(char*)0 | es) % W ? 2 : es > W ? 1 : 0

#define exch(a, b, t) (t = a, a = b, b = t);
#define swap(a, b)                                \
   swaptype != 0 ? swapfunc(a, b, es, swaptype) : \
   (void)exch(*(WORD*)(a), *(WORD*)(b), t)

#define vecswap(a, b, n) if (n > 0) swapfunc(a, b, n, swaptype)

static void swapfunc(char *a, char *b, size_t n, int swaptype)
{
   if (swaptype <= 1) {
      WORD t;
      for( ; n > 0; a += W, b += W, n -= W)
         exch(*(WORD*)a, *(WORD*)b, t);
   } else {
      char t;
      for( ; n > 0; a += 1, b += 1, n -= 1)
         exch(*a, *b, t);
   }
}

#define PVINIT(pv, pm)                          \
   if (swaptype != 0) { pv = a; swap(pv, pm); } \
   else { pv = (char*)&v; v = *(WORD*)pm; }

#define min(a, b) ((a) < (b) ? (a) : (b))

static char *med3(char *a, char *b, char *c, Qsortcmp_r_f cmp, void* h)
{   return cmp(a, b, h) < 0 ?
       (cmp(b, c, h) < 0 ? b : cmp(a, c, h) < 0 ? c : a)
     : (cmp(b, c, h) > 0 ? b : cmp(a, c, h) > 0 ? c : a);
}

void qsort_r(void* va, size_t n, size_t es, Qsortcmp_r_f cmp, void* h)
{
   char *a, *pa, *pb, *pc, *pd, *pl, *pm, *pn, *pv;
   int r, swaptype;
   WORD t, v;
   size_t s;

   a = va;
   SWAPINIT(a, es);
   if (n < 7) {       /* Insertion sort on smallest arrays */
      for (pm = a + es; pm < a + n*es; pm += es)
         for (pl = pm; pl > a && cmp(pl-es, pl, h) > 0; pl -= es)
            swap(pl, pl-es);
      return;
   }
   pm = a + (n/2)*es;      /* Small arrays, middle element */
   if (n > 7) {
      pl = a;
      pn = a + (n-1)*es;
      if (n > 40) {       /* Big arrays, pseudomedian of 9 */
         s = (n/8)*es;
         pl = med3(pl, pl+s, pl+2*s, cmp, h);
         pm = med3(pm-s, pm, pm+s, cmp, h);
         pn = med3(pn-2*s, pn-s, pn, cmp, h);
      }
      pm = med3(pl, pm, pn, cmp, h);    /* Mid-size, med of 3 */
   }
   PVINIT(pv, pm);         /* pv points to partition value */
   pa = pb = a;
   pc = pd = a + (n-1)*es;
   for (;;) {
      while (pb <= pc && (r = cmp(pb, pv, h)) <= 0) {
         if (r == 0) { swap(pa, pb); pa += es; }
         pb += es;
      }
      while (pc >= pb && (r = cmp(pc, pv, h)) >= 0) {
         if (r == 0) { swap(pc, pd); pd -= es; }
         pc -= es;
      }
      if (pb > pc) break;
      swap(pb, pc);
      pb += es;
      pc -= es;
   }
   pn = a + n*es;
   s = min(pa-a,  pb-pa   ); vecswap(a,  pb-s, s);
   s = min(pd-pc, pn-pd-es); vecswap(pb, pn-s, s);
   if ((s = pb-pa) > es) qsort_r(a,    s/es, es, cmp, h);
   if ((s = pd-pc) > es) qsort_r(pn-s, s/es, es, cmp, h);
}

#endif
