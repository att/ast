# regression tests for the proto utility

TEST 01 'basics'
	EXEC -nh
		INPUT - $'#pragma prototyped

void (*FUN)(struct namnod*);

void (*signal(int, void (*)(int)))(int);

main()
{
\tint n;

\tn = nv_scan(root, (void(*)(struct namnod*))0,flag,flag);
}'
		OUTPUT - $'                  

void (*FUN) __PROTO__((struct namnod*));

void (*signal __PROTO__((int, void (*)(int)))) __PROTO__((int));

main()
{
\tint n;

\tn = nv_scan(root, (void(*)(struct namnod*))0,flag,flag);
}'
	EXEC -nh
		INPUT - $'#pragma prototyped
extern int\topen(), read(), write(), close();'
		OUTPUT - $'                  
extern __MANGLE__ int\topen(__VARARG__), read(__VARARG__), write(__VARARG__), close(__VARARG__);'
	EXEC -nh
		INPUT - $'#pragma prototyped

char*\ta = "x\\ax";
int\tb = \'\\v\';
char*\tc = "a\\x1aFb";

extern char* strcpy(const char*, const char*);

typedef void (*signal_t)(int);

int\tnl = \'\\n\';
char*\ty = "y\\
z";

typedef char (*oops);

struct test
{
\tint\t(*call)(int, ...);
};

#include <stdarg.h>

int error(int level, char* format, ...)
{
\tva_list\tap;

\tva_start(ap, format);
\tva_end(ap);
}

int main(int argc, char** argv)
{
\tvoid exit(int);
\tif (0);
\telse while(xxx);
\tdo return(0); while (1);
\tif (ok) return(1);
\texit(0);
}'
		OUTPUT - $'                  

char*\ta = "x\\007x";
int\tb = \'\\013\';
char*\tc = "a\\657b";

extern __MANGLE__ char* strcpy __PROTO__((const char*, const char*));

typedef void (*signal_t) __PROTO__((int));

int\tnl = \'\\n\';
char*\ty = "y\\
z";

typedef char (*oops);

struct test
{
\tint\t(*call) __PROTO__((int, ...));
};

#if !defined(va_start)
#if defined(__STDARG__)
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#endif

#line 22


int error __PARAM__((int level, char* format, ...), (va_alist)) __OTORP__(va_dcl)
{ __OTORP__(int level; char* format; )
#line 25

\tva_list\tap;

\t__VA_START__(ap, format); __OTORP__(level = va_arg(ap, int );format = va_arg(ap, char* );)
\tva_end(ap);
}

int main __PARAM__((int argc, char** argv), (argc, argv)) __OTORP__(int argc; char** argv;)
#line 33
{
\tvoid exit __PROTO__((int));
\tif (0);
\telse while(xxx);
\tdo return(0); while (1);
\tif (ok) return(1);
\texit(0);
}'
	EXEC -nh
		INPUT - $'#pragma prototyped
extern const char foo[];'
		OUTPUT - $'                  
extern __MANGLE__ const char foo[];'
	EXEC -nh
		INPUT - $'#pragma prototyped
   extern void fun(int);
/* extern void ( *signal( in1, void (*)(in2) ) )(in3); */
   extern void ( *signal( in1, void (*)(in2) ) )(in3);
/* extern void ( *foo(in1, in2) )(in3); */
   extern void ( *foo(in1, in2) )(in3);'
		OUTPUT - $'                  
   extern __MANGLE__ void fun __PROTO__((int));
/* extern void ( *signal( in1, void (*)(in2) ) )(in3); */
   extern __MANGLE__ void ( *signal __PROTO__(( in1, void (*)(in2) ) )) __PROTO__((in3));
/* extern void ( *foo(in1, in2) )(in3); */
   extern __MANGLE__ void ( *foo __PROTO__((in1, in2) )) __PROTO__((in3));'
	EXEC -nh
		INPUT - $'#pragma prototyped

extern unsigned long\tcsaddr(char*, int);
extern int\t\tcscreat(char*, int);
extern int\t\tcsdaemon(void);
extern int\t\tcsfdmax(void);
extern unsigned long\tcsinet(char*, unsigned short*, int*);
extern char*\t\tcsname(void);
extern int\t\tcsnote(char*);
extern char*\t\tcsntoa(unsigned long);
extern int\t\tcsopen(char*);
extern char*\t\tcspath(unsigned long, char*, int);
extern int\t\tcspoll(int, char*, char*, long);
extern int\t\tcsrecv(int, char*, int*, int);
extern int\t\tcssend(int, int*, int);
extern int\t\tcsstat(char*, char*);'
		OUTPUT - $'                  

extern __MANGLE__ unsigned long\tcsaddr __PROTO__((char*, int));
extern __MANGLE__ int\t\tcscreat __PROTO__((char*, int));
extern __MANGLE__ int\t\tcsdaemon __PROTO__((void));
extern __MANGLE__ int\t\tcsfdmax __PROTO__((void));
extern __MANGLE__ unsigned long\tcsinet __PROTO__((char*, unsigned short*, int*));
extern __MANGLE__ char*\t\tcsname __PROTO__((void));
extern __MANGLE__ int\t\tcsnote __PROTO__((char*));
extern __MANGLE__ char*\t\tcsntoa __PROTO__((unsigned long));
extern __MANGLE__ int\t\tcsopen __PROTO__((char*));
extern __MANGLE__ char*\t\tcspath __PROTO__((unsigned long, char*, int));
extern __MANGLE__ int\t\tcspoll __PROTO__((int, char*, char*, long));
extern __MANGLE__ int\t\tcsrecv __PROTO__((int, char*, int*, int));
extern __MANGLE__ int\t\tcssend __PROTO__((int, int*, int));
extern __MANGLE__ int\t\tcsstat __PROTO__((char*, char*));'
	EXEC -nh
		INPUT - $'#pragma prototyped

int error(int (*function)(int), char* format)
{
}'
		OUTPUT - $'                  

int error __PARAM__((int (*function)(int), char* format), (function, format)) __OTORP__(int (*function)(); char* format;)
#line 4
{
}'
	EXEC -nh
		INPUT - $'#pragma prototyped
disp_form(pkt)
register packet *pkt;
{
    char *rjust(), *ptr;

    return(0);
}'
		OUTPUT - $'                  \ndisp_form(pkt)
register packet *pkt;
{
    char *rjust(__VARARG__), *ptr;

    return(0);
}'
	EXEC -nh
		INPUT - $'#pragma prototyped
int i = 0xbeef;
int I = 0XBEEF;
float f = 12.34f;
float F = 56.78F;'
		OUTPUT - $'                  \nint i = 0xbeef;
int I = 0XBEEF;
float f = 12.34;
float F = 56.78;'

TEST 02 'subtleties'
	EXEC -nh
		INPUT - $'#pragma prototyped

/*
 * license validation
 * vulnerable to debugging and binary edits
 */

#include <ast.h>
#include <cs.h>
#include <error.h>
#include <hashpart.h>
#include <tm.h>

#ifndef KEY
#define KEY\t0x001010
#endif

static void
signature(char* key, int keysize, const char* package,
\t\tconst char* data, const char* text)
{
\tregister int\t\t\tc;
\tregister const char*\t\td;
\tregister char*\t\t\tk;
\tregister const char*\t\tp;
\tregister const unsigned char*\tt;
\tregister unsigned long\t\th;

\tstatic char\t\tsalt[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.";

\tmemzero(key, keysize--);
\td = data;
\tk = key;
\tp = package;
\tt = (unsigned char*)text;
\th = 0;
\twhile (c = *d++)
\t{
\t\tHASHPART(h, c);
\t\tk[h % keysize] ^= h >> 1;
\t\tif (!(c = *p++)) p = package;
\t\tHASHPART(h, c);
\t\tk[h % keysize] ^= h >> 3;
\t\tHASHPART(h, *t++);
\t\tk[h % keysize] ^= h >> 5;
\t\tc = 0;
\t\twhile ((h & KEY) != KEY && c++ < 100)
\t\t{
\t\t\tHASHPART(h, 0);
\t\t\tk[h % keysize] ^= h >> 7;
\t\t}
\t}
\tfor (; k < &key[keysize]; k++)
\t\t*k = salt[*(unsigned char*)k % (sizeof(salt) - 1)];
}

/*
 * validate license for tool in package
 * if pc!=0 then it is filled with license field data
 */

void
pathcheck(const char* package, const char* tool, register PATHCHECK* pc)
{
\tint\tc;
\tint\tn;
\tchar*\tb;
\tchar*\tk;
\tchar*\tv;
\tchar*\tu;
\tlong\texpire;
\ttime_t\tnow;
\tchar\tkey[14];
\tchar\tlicense[PATH_MAX];
\tchar\tdata[PATH_MAX];
\tchar\ttext[PATH_MAX];
\t
#ifdef\tpathcheck
\tif (gen_license) (void)strcpy(data, gen_license);
\telse
#endif
\t(void)sfsprintf(data, sizeof(data), "lib/%s/license", package);
\tif (!pathpath(license, data,
#ifdef\tpathcheck
\t\tgen_license ? NiL :
#endif
\t\ttool, PATH_REGULAR|PATH_READ))
\t\t\terror(3, "%s: not found", data);
\tif ((n = open(license, 0)) < 0)
\t\terror(3, "%s: cannot open", license);
\tif ((c = read(n, data, sizeof(data) - 1)) < 0)
\t\terror(3, "%s: cannot read", license);
\tclose(n);
\tdata[c] = 0;
\tif (!pathpath(text, tool, NiL, PATH_REGULAR|PATH_READ|PATH_EXECUTE))
\t\terror(3, "%s: not found", tool);
\tif ((n = open(text, 0)) < 0)
\t\terror(3, "%s: cannot open", text);
\t(void)lseek(n, lseek(n, 0, SEEK_END) / 4, SEEK_SET);
\tmemzero(text, c);
\t(void)read(n, text, c);
\tclose(n);

\t/*
\t * check signature
\t */

\tb = data;
\twhile (tokscan(b, &b, " #%s
", NiL) == 1);
\tif (tokscan(b, &b, " %s ", &v) != 1)
\t\terror(3, "%s: garbled", license);
\tsignature(key, sizeof(key), package, b, text);

\t/*
\t * NOTE: no lib routines for signature compare/clear
\t */

\tn = 0;
\twhile (*v)
\t{
\t\tk = key;
\t\twhile (*v && *v != \'|\' && *k++ == *v++);
\t\tif (!*k)
\t\t{
\t\t\tn = 1;
\t\t\tbreak;
\t\t}
\t\twhile (*v && *v++ != \'|\');
\t}
#ifdef\tpathcheck
\tif (!n)
\t{
\t\tsfprintf(sfstdout, "|%s|
", key);
\t\texit(0);
\t}
#endif
\tfor (k = key; *k; *k++ = 0);
\tif (!n) error(3, "%s: invalid", license);

\t/*
\t * loop on the fields
\t */

\tif (pc) memzero(pc, sizeof(*pc));
\tfor (n = 0;; n++)
\t{
\t\twhile (tokscan(b, &b, " #%s
", NiL) == 1);
\t\tif (tokscan(b, &u, " %s=%s ", &k, &v) != 2) break;
\t\tb = u;
\t\tswitch (*k)
\t\t{
\t\tcase \'e\':
\t\t\tif (!strcmp(k, "expire"))
\t\t\t{
\t\t\t\tnow = time(NiL);
\t\t\t\texpire = (unsigned long)tmdate(v, &u, &now) - (unsigned long)now;
\t\t\t\tif (*u)
\t\t\t\t\terror(3, "%s: %s: invalid expiration date", license, v);
\t\t\t\tif (expire <= 0)
\t\t\t\t\terror(3, "license expired");
\t\t\t\tif (expire < (60 * 60 * 24 * 7))
\t\t\t\t\terror(1, "license expires in %s", fmtelapsed(expire, 1));
\t\t\t\tif (pc) pc->date = expire;
\t\t\t\tcontinue;
\t\t\t}
\t\t\tbreak;
\t\tcase \'h\':
\t\t\tif (!strcmp(k, "host"))
\t\t\t{
\t\t\t\tu = csname();
\t\t\t\tif (!strmatch(u, v) && !strmatch(csntoa(csaddr(NiL, -1), 0), v))
\t\t\t\t\terror(3, "%s: %s=%s not licensed", license, k, u);
\t\t\t\tif (pc) pc->host = strdup(v);
\t\t\t\tcontinue;
\t\t\t}
\t\t\tbreak;
\t\tcase \'u\':
\t\t\tif (!strcmp(k, "user"))
\t\t\t{
\t\t\t\tu = fmtuid(getuid());
\t\t\t\tif (!strmatch(u, v))
\t\t\t\t\terror(3, "%s: %s=%s not licensed", license, k, u);
\t\t\t\tif (pc) pc->user = strdup(v);
\t\t\t\tcontinue;
\t\t\t}
\t\t\tbreak;
\t\t}
#ifdef\tpathcheck
\t\terror(1, "%s: %s: invalid key", license, k);
#endif
\t}
\tif (*b) error(3, "%s: %s: invalid field", license, b);
\tif (!n) error(3, "%s: empty", license);
}'
		OUTPUT - $'                  

/*
 * license validation
 * vulnerable to debugging and binary edits
 */

#include <ast.h>
#include <cs.h>
#include <error.h>
#include <hashpart.h>
#include <tm.h>

#ifndef KEY
#define KEY\t0x001010
#endif

static void
signature __PARAM__((char* key, int keysize, const char* package,
\t\tconst char* data, const char* text), (key, keysize, package, data, text)) __OTORP__(char* key; int keysize; const char* package;
\t\tconst char* data; const char* text;)
#line 21
{
\tregister int\t\t\tc;
\tregister const char*\t\td;
\tregister char*\t\t\tk;
\tregister const char*\t\tp;
\tregister const unsigned char*\tt;
\tregister unsigned long\t\th;

\tstatic char\t\tsalt[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.";

\tmemzero(key, keysize--);
\td = data;
\tk = key;
\tp = package;
\tt = (unsigned char*)text;
\th = 0;
\twhile (c = *d++)
\t{
\t\tHASHPART(h, c);
\t\tk[h % keysize] ^= h >> 1;
\t\tif (!(c = *p++)) p = package;
\t\tHASHPART(h, c);
\t\tk[h % keysize] ^= h >> 3;
\t\tHASHPART(h, *t++);
\t\tk[h % keysize] ^= h >> 5;
\t\tc = 0;
\t\twhile ((h & KEY) != KEY && c++ < 100)
\t\t{
\t\t\tHASHPART(h, 0);
\t\t\tk[h % keysize] ^= h >> 7;
\t\t}
\t}
\tfor (; k < &key[keysize]; k++)
\t\t*k = salt[*(unsigned char*)k % (sizeof(salt) - 1)];
}

/*
 * validate license for tool in package
 * if pc!=0 then it is filled with license field data
 */

void
pathcheck __PARAM__((const char* package, const char* tool, register PATHCHECK* pc), (package, tool, pc)) __OTORP__(const char* package; const char* tool; register PATHCHECK* pc;)
#line 64
{
\tint\tc;
\tint\tn;
\tchar*\tb;
\tchar*\tk;
\tchar*\tv;
\tchar*\tu;
\tlong\texpire;
\ttime_t\tnow;
\tchar\tkey[14];
\tchar\tlicense[PATH_MAX];
\tchar\tdata[PATH_MAX];
\tchar\ttext[PATH_MAX];
\t
#ifdef\tpathcheck
\tif (gen_license) (void)strcpy(data, gen_license);
\telse
#endif
\t(void)sfsprintf(data, sizeof(data), "lib/%s/license", package);
\tif (!pathpath(license, data,
#ifdef\tpathcheck
\t\tgen_license ? NiL :
#endif
\t\ttool, PATH_REGULAR|PATH_READ))
\t\t\terror(3, "%s: not found", data);
\tif ((n = open(license, 0)) < 0)
\t\terror(3, "%s: cannot open", license);
\tif ((c = read(n, data, sizeof(data) - 1)) < 0)
\t\terror(3, "%s: cannot read", license);
\tclose(n);
\tdata[c] = 0;
\tif (!pathpath(text, tool, NiL, PATH_REGULAR|PATH_READ|PATH_EXECUTE))
\t\terror(3, "%s: not found", tool);
\tif ((n = open(text, 0)) < 0)
\t\terror(3, "%s: cannot open", text);
\t(void)lseek(n, lseek(n, 0, SEEK_END) / 4, SEEK_SET);
\tmemzero(text, c);
\t(void)read(n, text, c);
\tclose(n);

\t/*
\t * check signature
\t */

\tb = data;
\twhile (tokscan(b, &b, " #%s
", NiL) == 1);
\tif (tokscan(b, &b, " %s ", &v) != 1)
\t\terror(3, "%s: garbled", license);
\tsignature(key, sizeof(key), package, b, text);

\t/*
\t * NOTE: no lib routines for signature compare/clear
\t */

\tn = 0;
\twhile (*v)
\t{
\t\tk = key;
\t\twhile (*v && *v != \'|\' && *k++ == *v++);
\t\tif (!*k)
\t\t{
\t\t\tn = 1;
\t\t\tbreak;
\t\t}
\t\twhile (*v && *v++ != \'|\');
\t}
#ifdef\tpathcheck
\tif (!n)
\t{
\t\tsfprintf(sfstdout, "|%s|
", key);
\t\texit(0);
\t}
#endif
\tfor (k = key; *k; *k++ = 0);
\tif (!n) error(3, "%s: invalid", license);

\t/*
\t * loop on the fields
\t */

\tif (pc) memzero(pc, sizeof(*pc));
\tfor (n = 0;; n++)
\t{
\t\twhile (tokscan(b, &b, " #%s
", NiL) == 1);
\t\tif (tokscan(b, &u, " %s=%s ", &k, &v) != 2) break;
\t\tb = u;
\t\tswitch (*k)
\t\t{
\t\tcase \'e\':
\t\t\tif (!strcmp(k, "expire"))
\t\t\t{
\t\t\t\tnow = time(NiL);
\t\t\t\texpire = (unsigned long)tmdate(v, &u, &now) - (unsigned long)now;
\t\t\t\tif (*u)
\t\t\t\t\terror(3, "%s: %s: invalid expiration date", license, v);
\t\t\t\tif (expire <= 0)
\t\t\t\t\terror(3, "license expired");
\t\t\t\tif (expire < (60 * 60 * 24 * 7))
\t\t\t\t\terror(1, "license expires in %s", fmtelapsed(expire, 1));
\t\t\t\tif (pc) pc->date = expire;
\t\t\t\tcontinue;
\t\t\t}
\t\t\tbreak;
\t\tcase \'h\':
\t\t\tif (!strcmp(k, "host"))
\t\t\t{
\t\t\t\tu = csname();
\t\t\t\tif (!strmatch(u, v) && !strmatch(csntoa(csaddr(NiL, -1), 0), v))
\t\t\t\t\terror(3, "%s: %s=%s not licensed", license, k, u);
\t\t\t\tif (pc) pc->host = strdup(v);
\t\t\t\tcontinue;
\t\t\t}
\t\t\tbreak;
\t\tcase \'u\':
\t\t\tif (!strcmp(k, "user"))
\t\t\t{
\t\t\t\tu = fmtuid(getuid());
\t\t\t\tif (!strmatch(u, v))
\t\t\t\t\terror(3, "%s: %s=%s not licensed", license, k, u);
\t\t\t\tif (pc) pc->user = strdup(v);
\t\t\t\tcontinue;
\t\t\t}
\t\t\tbreak;
\t\t}
#ifdef\tpathcheck
\t\terror(1, "%s: %s: invalid key", license, k);
#endif
\t}
\tif (*b) error(3, "%s: %s: invalid field", license, b);
\tif (!n) error(3, "%s: empty", license);
}'
	EXEC -nh
		INPUT - $'extern char *malloc(), *realloc();

# line 2 "parser.y"
#pragma prototyped
#include <ast.h>

/************************************************************************
*\t\t\t\t\t\t\t\t\t*
*    David S. Rosenblum\t\t\t\t\t\t\t*
*    AT&T Bell Laboratories\t\t\t\t\t\t*
*    MH 3C-541\t\t\t\t\t\t\t\t*
*    ulysses!dsr\t\t\t\t\t\t\t*
*\t\t\t\t\t\t\t\t\t*
*************************************************************************
*\t\t\t\t\t\t\t\t\t*
*\tYACC SPECIFICATION OF PARSER FOR APP TOOL\t\t\t*
*\tVersion 2.00\t\t\t\t\t\t\t*
*\t\t\t\t\t\t\t\t\t*
************************************************************************/

#include <error.h>
#include <stdio.h>
#include <hash.h>

/*  pp.h is included just before the first %%, to avoid warning messages
**  however we need hash.h (which is included by pp.h) now, for following
**  #include\'s:
*/

#include <app.h>
#include <typetbl.h>
#include <symbol.h>
#include <output.h>
#include <state.h>
#include <cctypes.h>
#include <pdecls.h>\t/*  The declarations section is in pdecls.h,
\t\t\t**  so that proto doesn\'t get confused.
\t\t\t*/

# line 39 "parser.y"
typedef union  {
    struct token_type* ltoken;
    T1WORD             exptype;
    BOOL\t       success;
    int\t\t       count;
    SIZE\t       expval;
    struct {
\tint\t      count;
\tOUTFILE_KIND  okval;
    }\t\t       inexpr;
    struct {
\tstruct token_type* ltoken;
\tT1WORD\t\t   exptype;
    }\t\t       exptoken;
    struct {
\tunsigned long\t   star_count;
\tunsigned long\t   const_star_flags;
    } star_specs;
} YYSTYPE;
# define T_DOUBLE 292
# define T_DOUBLE_L 293
# define T_FLOAT 294
# define T_DECIMAL 288
# define T_DECIMAL_L 289
# define T_DECIMAL_U 290
# define T_DECIMAL_UL 291
# define T_OCTAL 296
# define T_OCTAL_L 297
# define T_OCTAL_U 298
# define T_OCTAL_UL 299
# define T_HEXADECIMAL 304
# define T_HEXADECIMAL_L 305
# define T_HEXADECIMAL_U 306
# define T_HEXADECIMAL_UL 307
# define T_ID 257
# define T_INVALID 258
# define T_CHARCONST 260
# define T_WCHARCONST 261
# define T_STRING 262
# define T_WSTRING 263
# define T_PTRMEM 264
# define T_ADDADD 265
# define T_SUBSUB 266
# define T_LSHIFT 267
# define T_RSHIFT 268
# define T_LE 269
# define T_GE 270
# define T_EQ 271
# define T_NE 272
# define T_ANDAND 273
# define T_OROR 274
# define T_MPYEQ 275
# define T_DIVEQ 276
# define T_MODEQ 277
# define T_ADDEQ 278
# define T_SUBEQ 279
# define T_LSHIFTEQ 280
# define T_RSHIFTEQ 281
# define T_ANDEQ 282
# define T_XOREQ 283
# define T_OREQ 284
# define T_VARIADIC 286
# define T_DOTREF 320
# define T_PTRMEMREF 321
# define T_SCOPE 322
# define T_UMINUS 323
# define T_AUTO 324
# define T_BREAK 325
# define T_CASE 326
# define T_CHAR 327
# define T_CONTINUE 328
# define T_DEFAULT 329
# define T_DO 330
# define T_DOUBLE_T 331
# define T_ELSE 332
# define T_EXTERN 333
# define T_FLOAT_T 334
# define T_FOR 335
# define T_GOTO 336
# define T_IF 337
# define T_INT 338
# define T_LONG 339
# define T_REGISTER 340
# define T_RETURN 341
# define T_SHORT 342
# define T_SIZEOF 343
# define T_STATIC 344
# define T_STRUCT 345
# define T_SWITCH 346
# define T_TYPEDEF 347
# define T_UNION 348
# define T_UNSIGNED 349
# define T_WHILE 350
# define T_CONST 351
# define T_ENUM 352
# define T_SIGNED 353
# define T_VOID 354
# define T_VOLATILE 355
# define T_ASM 356
# define T_CATCH 357
# define T_CLASS 358
# define T_DELETE 359
# define T_FRIEND 360
# define T_INLINE 361
# define T_NEW 362
# define T_OPERATOR 363
# define T_OVERLOAD 364
# define T_PRIVATE 365
# define T_PROTECTED 366
# define T_PUBLIC 367
# define T_TEMPLATE 368
# define T_THIS 369
# define T_THROW 370
# define T_TRY 371
# define T_VIRTUAL 372
# define T_NOISES 373
# define T_NOISE 374
# define T_X_GROUP 375
# define T_X_LINE 376
# define T_X_STATEMENT 377
# define T_ASSERT 400
# define T_ASSUME 401
# define T_IN 402
# define T_PROMISE 403
# define T_WHERE 404
# define T_ALL 405
# define T_SOME 406
# define T_LANNO_DELIMITER 407
# define T_RANNO_DELIMITER 408
# define T_USERTYPE 409
# define T_LOWEST_PRECEDENCE 410

# line 146 "parser.y"
#include <pp.h>
/* take 1 out of next comment for 4096 drop of format = .................. */
/* 23456789 */
#ifdef YYDEBUG
/**/
print_trace(char* format, ...)
{
    va_list ap;

    va_start(ap, format);
    error(parser_tracing, "%:", format, ap);\t/*  %: descriptor says that
\t\t\t\t\t\t**  format and va_list follow
\t\t\t\t\t\t*/
    va_end(ap);
}
#endif  /* YYDEBUG */'
		OUTPUT - $'extern __MANGLE__ char *malloc(__VARARG__), *realloc(__VARARG__);

# line 2 "parser.y"
                  
#include <ast.h>

/************************************************************************
*\t\t\t\t\t\t\t\t\t*
*    David S. Rosenblum\t\t\t\t\t\t\t*
*    AT&T Bell Laboratories\t\t\t\t\t\t*
*    MH 3C-541\t\t\t\t\t\t\t\t*
*    ulysses!dsr\t\t\t\t\t\t\t*
*\t\t\t\t\t\t\t\t\t*
*************************************************************************
*\t\t\t\t\t\t\t\t\t*
*\tYACC SPECIFICATION OF PARSER FOR APP TOOL\t\t\t*
*\tVersion 2.00\t\t\t\t\t\t\t*
*\t\t\t\t\t\t\t\t\t*
************************************************************************/

#include <error.h>
#include <stdio.h>
#include <hash.h>

/*  pp.h is included just before the first %%, to avoid warning messages
**  however we need hash.h (which is included by pp.h) now, for following
**  #include\'s:
*/

#include <app.h>
#include <typetbl.h>
#include <symbol.h>
#include <output.h>
#include <state.h>
#include <cctypes.h>
#include <pdecls.h>\t/*  The declarations section is in pdecls.h,
\t\t\t**  so that proto doesn\'t get confused.
\t\t\t*/

# line 39 "parser.y"
typedef union  {
    struct token_type* ltoken;
    T1WORD             exptype;
    BOOL\t       success;
    int\t\t       count;
    SIZE\t       expval;
    struct {
\tint\t      count;
\tOUTFILE_KIND  okval;
    }\t\t       inexpr;
    struct {
\tstruct token_type* ltoken;
\tT1WORD\t\t   exptype;
    }\t\t       exptoken;
    struct {
\tunsigned long\t   star_count;
\tunsigned long\t   const_star_flags;
    } star_specs;
} YYSTYPE;
# define T_DOUBLE 292
# define T_DOUBLE_L 293
# define T_FLOAT 294
# define T_DECIMAL 288
# define T_DECIMAL_L 289
# define T_DECIMAL_U 290
# define T_DECIMAL_UL 291
# define T_OCTAL 296
# define T_OCTAL_L 297
# define T_OCTAL_U 298
# define T_OCTAL_UL 299
# define T_HEXADECIMAL 304
# define T_HEXADECIMAL_L 305
# define T_HEXADECIMAL_U 306
# define T_HEXADECIMAL_UL 307
# define T_ID 257
# define T_INVALID 258
# define T_CHARCONST 260
# define T_WCHARCONST 261
# define T_STRING 262
# define T_WSTRING 263
# define T_PTRMEM 264
# define T_ADDADD 265
# define T_SUBSUB 266
# define T_LSHIFT 267
# define T_RSHIFT 268
# define T_LE 269
# define T_GE 270
# define T_EQ 271
# define T_NE 272
# define T_ANDAND 273
# define T_OROR 274
# define T_MPYEQ 275
# define T_DIVEQ 276
# define T_MODEQ 277
# define T_ADDEQ 278
# define T_SUBEQ 279
# define T_LSHIFTEQ 280
# define T_RSHIFTEQ 281
# define T_ANDEQ 282
# define T_XOREQ 283
# define T_OREQ 284
# define T_VARIADIC 286
# define T_DOTREF 320
# define T_PTRMEMREF 321
# define T_SCOPE 322
# define T_UMINUS 323
# define T_AUTO 324
# define T_BREAK 325
# define T_CASE 326
# define T_CHAR 327
# define T_CONTINUE 328
# define T_DEFAULT 329
# define T_DO 330
# define T_DOUBLE_T 331
# define T_ELSE 332
# define T_EXTERN 333
# define T_FLOAT_T 334
# define T_FOR 335
# define T_GOTO 336
# define T_IF 337
# define T_INT 338
# define T_LONG 339
# define T_REGISTER 340
# define T_RETURN 341
# define T_SHORT 342
# define T_SIZEOF 343
# define T_STATIC 344
# define T_STRUCT 345
# define T_SWITCH 346
# define T_TYPEDEF 347
# define T_UNION 348
# define T_UNSIGNED 349
# define T_WHILE 350
# define T_CONST 351
# define T_ENUM 352
# define T_SIGNED 353
# define T_VOID 354
# define T_VOLATILE 355
# define T_ASM 356
# define T_CATCH 357
# define T_CLASS 358
# define T_DELETE 359
# define T_FRIEND 360
# define T_INLINE 361
# define T_NEW 362
# define T_OPERATOR 363
# define T_OVERLOAD 364
# define T_PRIVATE 365
# define T_PROTECTED 366
# define T_PUBLIC 367
# define T_TEMPLATE 368
# define T_THIS 369
# define T_THROW 370
# define T_TRY 371
# define T_VIRTUAL 372
# define T_NOISES 373
# define T_NOISE 374
# define T_X_GROUP 375
# define T_X_LINE 376
# define T_X_STATEMENT 377
# define T_ASSERT 400
# define T_ASSUME 401
# define T_IN 402
# define T_PROMISE 403
# define T_WHERE 404
# define T_ALL 405
# define T_SOME 406
# define T_LANNO_DELIMITER 407
# define T_RANNO_DELIMITER 408
# define T_USERTYPE 409
# define T_LOWEST_PRECEDENCE 410

# line 146 "parser.y"
#include <pp.h>
/* take 1 out of next comment for 4096 drop of format = .................. */
/* 23456789 */
#ifdef YYDEBUG
/**/
print_trace __PARAM__((char* format, ...), (va_alist)) __OTORP__(va_dcl)
{ __OTORP__(char* format; )
#line 152

    va_list ap;

    __VA_START__(ap, format); __OTORP__(format = va_arg(ap, char* );)
    error(parser_tracing, "%:", format, ap);\t/*  %: descriptor says that
\t\t\t\t\t\t**  format and va_list follow
\t\t\t\t\t\t*/
    va_end(ap);
}
#endif  /* YYDEBUG */'
	EXEC -nh
		INPUT - $'#pragma prototyped
disp_form(pkt)
register packet *pkt;
{
        /* FUNCTION STUB */

    long atol();
    char *rjust(), *foo(), *ptr, *atime(), *disp_stfdit();
    int i;

    return(0);
}'
		OUTPUT - $'                  
disp_form(pkt)
register packet *pkt;
{
        /* FUNCTION STUB */

    long atol(__VARARG__);
    char *rjust(__VARARG__), *foo(__VARARG__), *ptr, *atime(__VARARG__), *disp_stfdit(__VARARG__);
    int i;

    return(0);
}'
	EXEC -nh
		INPUT - $'#pragma prototyped
disp_form(pkt)
register packet *pkt;
{
        /* FUNCTION STUB */

    long atol();
    extern char *fubar(int, int,int);
    char *rjust(int), *foo(int), *ptr, *atime(int), *disp_stfdit(int);
    int i;

    return(0);
}'
		OUTPUT - $'                  
disp_form(pkt)
register packet *pkt;
{
        /* FUNCTION STUB */

    long atol(__VARARG__);
    extern __MANGLE__ char *fubar __PROTO__((int, int,int));
    char *rjust __PROTO__((int)), *foo __PROTO__((int)), *ptr, *atime __PROTO__((int)), *disp_stfdit __PROTO__((int));
    int i;

    return(0);
}'
	EXEC -nh
		INPUT - $'#pragma prototyped

static Exid_t\t\tsymbols[] =
{
\tID,\t   BUILTIN(B_CLOCK),\tT_ELAPSED,
\tID,\t   BUILTIN(B_STATE),\t0,
\tID,\t   BUILTIN(B_DATE),\tT_DATE,
\tDECLARE,   T_DATE,\t\tT_DATE,
};'
		OUTPUT - $'                  

static Exid_t\t\tsymbols[] =
{
\tID,\t   BUILTIN(B_CLOCK),\tT_ELAPSED,
\tID,\t   BUILTIN(B_STATE),\t0,
\tID,\t   BUILTIN(B_DATE),\tT_DATE,
\tDECLARE,   T_DATE,\t\tT_DATE,
};'
	EXEC -nh
		INPUT - $'#pragma prototyped
void Tfreehelper (void *p) {
    Ttable_t *tp;
    Tkvlist_t *kvlp;
    int i;

    /* must be a table */
    tp = (Ttable_t *) p;
    for (i = 0; i < tp->ln; i++) {
        if ((kvlp = tp->lp[i])) {
            ((Mheader_t *) kvlp)->size = M_BYTE2SIZE (T_KVLISTSIZE (kvlp->n));
            Mfree (kvlp);
        }
    }
    ((Mheader_t *) tp->lp)->size = M_BYTE2SIZE (tp->ln * T_KVLISTPTRSIZE);
    Mfree (tp->lp);
}'
		OUTPUT - $'                  
void Tfreehelper  __PARAM__((__V_ *p), (p)) __OTORP__(__V_ *p;)
#line 2
{
    Ttable_t *tp;
    Tkvlist_t *kvlp;
    int i;

    /* must be a table */
    tp = (Ttable_t *) p;
    for (i = 0; i < tp->ln; i++) {
        if ((kvlp = tp->lp[i])) {
            ((Mheader_t *) kvlp)->size = M_BYTE2SIZE (T_KVLISTSIZE (kvlp->n));
            Mfree (kvlp);
        }
    }
    ((Mheader_t *) tp->lp)->size = M_BYTE2SIZE (tp->ln * T_KVLISTPTRSIZE);
    Mfree (tp->lp);
}'
	EXEC -nh
		INPUT - $'#pragma prototyped
static void scalarstr (Tobj to) {
    switch (Tgettype (to)) {
    case T_INTEGER:
        appendi (Tgetinteger (to));
        break;
    case T_REAL:
        appends (to), appendd (Tgetreal (to));
        break;
    case T_STRING:
        appends ("\\""), appends (Tgetstring (to)), appends ("\\"");
        break;
    case T_CODE:
        codestr (to, 0);
        break;
    }
}'
		OUTPUT - $'                  
static void scalarstr  __PARAM__((Tobj to), (to)) __OTORP__(Tobj to;)
#line 2
{
    switch (Tgettype (to)) {
    case T_INTEGER:
        appendi (Tgetinteger (to));
        break;
    case T_REAL:
        appends (to), appendd (Tgetreal (to));
        break;
    case T_STRING:
        appends ("\\""), appends (Tgetstring (to)), appends ("\\"");
        break;
    case T_CODE:
        codestr (to, 0);
        break;
    }
}'
	EXEC -nh
		INPUT - $'#pragma prototyped
/*
 * G. S. Fowler
 * D. G. Korn
 * AT&T Bell Laboratories
 *
 * shell library support
 */

#include <ast.h>

/*
 * return pointer to the full path name of the shell
 *
 * SHELL is read from the environment and must start with /
 *
 * if set-uid or set-gid then the executable and its containing
 * directory must not be writable by the real user
 *
 * /bin/sh is returned by default
 *
 * NOTE: csh is rejected because the bsh/csh differentiation is
 *       not done for `csh script arg ...\'
 */

char*
getshell(void)
{
\tregister char*\ts;
\tregister char*\tsh;
\tregister int\ti;

\tif ((sh = getenv("SHELL")) && *sh == \'/\' && strmatch(sh, "*/(sh|*[!cC]sh)"))
\t{
\t\tif (!(i = getuid()))
\t\t{
\t\t\tif (!strmatch(sh, "?(/usr)?(/local)/?(l)bin/?([a-z])sh")) goto defshell;
\t\t}
\t\telse if (i != geteuid() || getgid() != getegid())
\t\t{
\t\t\tif (!access(sh, 2)) goto defshell;
\t\t\ts = strrchr(sh, \'/\');
\t\t\t*s = 0;
\t\t\ti = access(sh, 2);
\t\t\t*s = \'/\';
\t\t\tif (!i) goto defshell;
\t\t}
\t\treturn(sh);
\t}
 defshell:
\treturn("/bin/sh");
}'
		OUTPUT - $'                  
/*
 * G. S. Fowler
 * D. G. Korn
 * AT&T Bell Laboratories
 *
 * shell library support
 */

#include <ast.h>

/*
 * return pointer to the full path name of the shell
 *
 * SHELL is read from the environment and must start with /
 *
 * if set-uid or set-gid then the executable and its containing
 * directory must not be writable by the real user
 *
 * /bin/sh is returned by default
 *
 * NOTE: csh is rejected because the bsh/csh differentiation is
 *       not done for `csh script arg ...\'
 */

char*
getshell __PARAM__((void), ())
#line 28
{
\tregister char*\ts;
\tregister char*\tsh;
\tregister int\ti;

\tif ((sh = getenv("SHELL")) && *sh == \'/\' && strmatch(sh, "*/(sh|*[!cC]sh)"))
\t{
\t\tif (!(i = getuid()))
\t\t{
\t\t\tif (!strmatch(sh, "?(/usr)?(/local)/?(l)bin/?([a-z])sh")) goto defshell;
\t\t}
\t\telse if (i != geteuid() || getgid() != getegid())
\t\t{
\t\t\tif (!access(sh, 2)) goto defshell;
\t\t\ts = strrchr(sh, \'/\');
\t\t\t*s = 0;
\t\t\ti = access(sh, 2);
\t\t\t*s = \'/\';
\t\t\tif (!i) goto defshell;
\t\t}
\t\treturn(sh);
\t}
 defshell:
\treturn("/bin/sh");
}'
	EXEC -nh
		INPUT - $'#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * single quote s into sp
 * if type!=0 then /<getenv(<CO_ENV_TYPE>)/ translated to /<CO_ENV_TYPE>/
 */

#include "colib.h"

void
coquote(register Sfio_t* sp, register const char* s, int type)
{
\tregister int\tc;

\tstatic char*\tmatch;

\tif (type)
\t{
\t\tif (!match && !(match = getenv(CO_ENV_TYPE))) match = "";
\t\tif (!*match) type = 0;
\t}
\twhile (c = *s++)
\t{
\t\tsfputc(sp, c);
\t\tif (c == \'\\\'\')
\t\t{
\t\t\tsfputc(sp, \'\\\\\');
\t\t\tsfputc(sp, \'\\\'\');
\t\t\tsfputc(sp, \'\\\'\');
\t\t}
\t\telse if (type && c == \'/\' && *s == *match)
\t\t{
\t\t\tregister const char*\tx = s;
\t\t\tregister char*\t\tt = match;

\t\t\twhile (*t && *t++ == *x) x++;
\t\t\tif (!*t && *x == \'/\')
\t\t\t{
\t\t\t\ts = x;
\t\t\t\tsfprintf(sp, "\'$%s\'", CO_ENV_TYPE);
\t\t\t}
\t\t}
\t}
}'
		OUTPUT - $'                  
/*
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * single quote s into sp
 * if type!=0 then /<getenv(<CO_ENV_TYPE>)/ translated to /<CO_ENV_TYPE>/
 */

#include "colib.h"

void
coquote __PARAM__((register Sfio_t* sp, register const char* s, int type), (sp, s, type)) __OTORP__(register Sfio_t* sp; register const char* s; int type;)
#line 14
{
\tregister int\tc;

\tstatic char*\tmatch;

\tif (type)
\t{
\t\tif (!match && !(match = getenv(CO_ENV_TYPE))) match = "";
\t\tif (!*match) type = 0;
\t}
\twhile (c = *s++)
\t{
\t\tsfputc(sp, c);
\t\tif (c == \'\\\'\')
\t\t{
\t\t\tsfputc(sp, \'\\\\\');
\t\t\tsfputc(sp, \'\\\'\');
\t\t\tsfputc(sp, \'\\\'\');
\t\t}
\t\telse if (type && c == \'/\' && *s == *match)
\t\t{
\t\t\tregister const char*\tx = s;
\t\t\tregister char*\t\tt = match;

\t\t\twhile (*t && *t++ == *x) x++;
\t\t\tif (!*t && *x == \'/\')
\t\t\t{
\t\t\t\ts = x;
\t\t\t\tsfprintf(sp, "\'$%s\'", CO_ENV_TYPE);
\t\t\t}
\t\t}
\t}
}'
	EXEC -nh
		INPUT - $'#pragma prototyped

#include <stdarg.h>

int error(int (*function)(int), char* format, ...)
{
\tva_list\tap;

\tva_start(ap, format);
\toops(1,2);
\t(*call)(3, 4);
\tva_end(ap);
}'
		OUTPUT - $'                  

#if !defined(va_start)
#if defined(__STDARG__)
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#endif

#line 3


int error __PARAM__((int (*function)(int), char* format, ...), (va_alist)) __OTORP__(va_dcl)
{ __OTORP__(int (*function)(); char* format; )
#line 6

\tva_list\tap;

\t__VA_START__(ap, format); __OTORP__(function = va_arg(ap, int (*)());format = va_arg(ap, char* );)
\toops(1,2);
\t(*call)(3, 4);
\tva_end(ap);
}'
	EXEC -nh
		INPUT - $'#pragma prototyped

#if 1
int hello(int a)
#else
int hello(long a)
#endif
{
\treturn a != 0;
}'
		OUTPUT - $'                  

#if 1
int hello __PARAM__((int a), (a)) __OTORP__(int a;)
#line 5

#else
int hello __PARAM__((long a), (a)) __OTORP__(long a;)
#line 7

#endif
{
\treturn a != 0;
}'
	EXEC -nh
		INPUT - $'#pragma prototyped

#ifdef foo
NoN(foo)
#else
int hello(long a)
{
\treturn a != 0;
}
#endif'
		OUTPUT - $'                  

#ifdef foo
NoN(foo)
#else
int hello __PARAM__((long a), (a)) __OTORP__(long a;)
#line 7
{
\treturn a != 0;
}
#endif'
	EXEC -nh
		INPUT - $'#pragma prototyped

#ifdef foo
NoNo(long a, long b)
#else
int NoNo(long a)
#endif
{
\treturn a != 0;
}'
		OUTPUT - $'                  

#ifdef foo
NoNo __PARAM__((long a, long b), (a, b)) __OTORP__(long a; long b;)
#line 5

#else
int NoNo __PARAM__((long a), (a)) __OTORP__(long a;)
#line 7

#endif
{
\treturn a != 0;
}'
	EXEC -nh
		INPUT - $'#pragma prototyped

#include <stdarg.h>

int error(int *function, char* format, ...)
{
\tva_list\tap;

\tva_start(ap, format);
\tva_end(ap);
}'
		OUTPUT - $'                  

#if !defined(va_start)
#if defined(__STDARG__)
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#endif

#line 3


int error __PARAM__((int *function, char* format, ...), (va_alist)) __OTORP__(va_dcl)
{ __OTORP__(int *function; char* format; )
#line 6

\tva_list\tap;

\t__VA_START__(ap, format); __OTORP__(function = va_arg(ap, int *);format = va_arg(ap, char* );)
\tva_end(ap);
}'

TEST 03 'headers'
	EXEC -nh
		INPUT - $'#pragma prototyped
/*
 * K. P. Vo
 * AT&T Bell Laboratories
 *
 * sfio public definitions
 */

#ifndef SF_READ\t\t/* protect against multiple #includes */

#ifndef NULL
#define NULL\t0
#endif
#ifndef EOF
#define EOF\t(-1)
#endif
#ifndef SEEK_SET
#define SEEK_SET\t0
#define SEEK_CUR\t1
#define SEEK_END\t2
#endif

typedef struct _sfdc_\tSfdisc_t;
typedef struct _sfio_\tSfile_t;

#define SFIO\tSfile_t
#define SFDISC\tSfdisc_t

/* discipline structure */
struct _sfdc_
{
\tint\t(*readf)(Sfile_t*, unsigned char*, int, char*);
\tint\t(*writef)(Sfile_t*, const unsigned char*, int, char*);
\tlong\t(*seekf)(Sfile_t*, long, int, char*);
\tint\t(*exceptf)(Sfile_t*, int, char*);
\tchar\t*handle;\t/* to store any state information\t*/
};

/* a file structure */
struct _sfio_
{
\tunsigned char\t*next;\t/* next position to read/write from\t*/
\tunsigned char\t*endw;\t/* end of write buffer\t\t\t*/
\tunsigned char\t*endr;\t/* end of read buffer\t\t\t*/
\tunsigned char\t*endb;\t/* end of buffer\t\t\t*/
\tstruct _sfio_\t*push;\t/* the stream that was pushed on\t*/
\tshort\t\tflags;\t/* type of stream\t\t\t*/
\tshort\t\tfile;\t/* file descriptor\t\t\t*/
\tlong\t\torig;\t/* where we start in the file\t\t*/
#ifdef _SFIO_PRIVATE
\t_SFIO_PRIVATE
#endif
};

/* bits for various types of files */
#define\tSF_READ\t\t000001\t/* open for reading\t\t\t*/
#define SF_WRITE\t000002\t/* open for writing\t\t\t*/
#define SF_STRING\t000004\t/* a string stream\t\t\t*/
#define SF_APPEND\t000010\t/* associated file is in append mode\t*/
#define SF_RELATIVE\t000020\t/* file pos is relative to starting pos\t*/
#define SF_MALLOC\t000040\t/* buffered space malloc-ed\t\t*/
#define SF_LINE\t\t000100\t/* line buffering\t\t\t*/
#define SF_KEEPFD\t000200\t/* keep file opened when closing stream\t*/
#define SF_SHARE\t000400\t/* file stream that is shared\t\t*/
#define SF_REUSE\t001000\t/* keep stream space after closing\t*/
#define SF_FLAGS\t000777\t/* PUBLIC FLAGS PASSABLE TO SFNEW()\t*/
#define SF_SETS\t\t001743\t/* flags passable to sfsetflag()\t*/

#define SF_EOF\t\t002000\t/* eof was detected\t\t\t*/
#define SF_ERROR\t004000\t/* an error happened\t\t\t*/

#define SF_BUFSIZE\t8192\t/* suggested default buffer size\t*/
#define SF_UNBOUND\t(-1)\t/* unbounded buffer size\t\t*/

#define\tsfstdin\t\t(&_Sfstdin)\t/* standard input stream\t*/
#define\tsfstdout\t(&_Sfstdout)\t/* standard output stream\t*/
#define\tsfstderr\t(&_Sfstderr)\t/* standard error stream\t*/

/* fast in-line functions */
#define sfputc(f,c)\t((f)->next >= (f)->endw ? \\
\t\t\t\t_sfflsbuf(f,(int)((unsigned char)(c))) : \\
\t\t\t\t(int)(*(f)->next++ = (unsigned char)(c)))
#define sfgetc(f)\t((f)->next >= (f)->endr ? _sffilbuf(f,1) : (int)(*(f)->next++))
#define sfslen()\t(_Sfi)
#define sffileno(f)\t((f)->file)
#define sforigin(f)\t(((f)->flags&SF_STRING) ? 0L : (f)->orig)
#define sfeof(f)\t(((f)->flags&(SF_EOF|SF_STRING)) && (f)->next >= (f)->endb)
#define sferror(f)\t((f)->flags&SF_ERROR)
#define sfclearerr(f)\t((f)->flags &= ~SF_ERROR)
#define sfpushed(f)\t((f)->push)

#define sfpeek(f,bufp)\t(((bufp) || \\
\t\t\t  ((f)->flags&(SF_READ|SF_WRITE|SF_STRING)) == \\
\t\t\t   (SF_READ|SF_WRITE|SF_STRING)) ? _sfpeek(f,bufp) : \\
\t\t\t\t((f)->endb - (f)->next))
#define sfsync(f)\t(((f) && (((Sfile_t*)(f))->flags&SF_STRING)) ? 0 : _sfsync(f))

/* coding long integers in a portable and compact fashion */
#define SF_SBITS\t6
#define SF_UBITS\t7
#define SF_SIGN\t\t(1 << SF_SBITS)
#define SF_MORE\t\t(1 << SF_UBITS)
#define SF_U1\t\tSF_MORE
#define SF_U2\t\t(SF_U1*SF_U1)
#define SF_U3\t\t(SF_U2*SF_U1)
#define SF_U4\t\t(SF_U3*SF_U1)
#define sfulen(v)\t((v) < SF_U1 ? 1 : (v) < SF_U2 ? 2 : \\
\t\t\t (v) < SF_U3 ? 3 : (v) < SF_U4 ? 4 : 5)
#define sfgetu(f)\t((_Sfi = sfgetc(f)) < 0 ? -1 : \\
\t\t\t\t((_Sfi&SF_MORE) ? _sfgetu(f) : (unsigned long)_Sfi))
#define sfgetl(f)\t((_Sfi = sfgetc(f)) < 0 ? -1 : \\
\t\t\t\t((_Sfi&(SF_MORE|SF_SIGN)) ? _sfgetl(f) : (long)_Sfi))
#define sfputu(f,v)\t_sfputu((f),(unsigned long)(v))
#define sfputl(f,v)\t_sfputl((f),(long)(v))
#define sfputd(f,v)\t_sfputd((f),(double)(v))

#define sfecvt(v,n,d,s)\t_sfcvt((v),(n),(d),(s),1)
#define sffcvt(v,n,d,s)\t_sfcvt((v),(n),(d),(s),0)

extern int\t\t_Sfi;
extern Sfile_t\t\t_Sfstdin, _Sfstdout, _Sfstderr;

extern Sfile_t\t\t*sfnew(Sfile_t*, unsigned char*, int, int, int);
extern Sfile_t\t\t*sfopen(Sfile_t*, const char*, const char*);
extern Sfile_t\t\t*sfdopen(int, const char*);
extern Sfile_t\t\t*sfpopen(const char*, const char*, Sfile_t**);
extern Sfile_t\t\t*sfstack(Sfile_t*, Sfile_t*);
extern Sfile_t\t\t*sftmpfile(void);
extern int\t\t_sfflsbuf(Sfile_t*, int);
extern int\t\t_sffilbuf(Sfile_t*, int);
extern int\t\t_sfsync(Sfile_t*);
extern int\t\t_sfpeek(Sfile_t*, unsigned char**);
extern int\t\tsfclrlock(Sfile_t*);
extern unsigned char*\tsfsetbuf(Sfile_t*, unsigned char*, int);
extern Sfdisc_t*\tsfsetdisc(Sfile_t*,Sfdisc_t*);
extern int\t\tsfsetflag(Sfile_t*, int, int);
extern int\t\tsfpool(Sfile_t*, Sfile_t*, int);
extern int\t\tsfread(Sfile_t*, unsigned char*, int);
extern int\t\tsfwrite(Sfile_t*, const unsigned char*, int);
extern int\t\tsfmove(Sfile_t*, Sfile_t*, long, int);
extern int\t\tsfclose(Sfile_t*);
extern long\t\tsftell(Sfile_t*);
extern long\t\tsfseek(Sfile_t*, long, int);
extern int\t\tsfllen(long);
extern int\t\tsfdlen(double);
extern int\t\tsfputs(Sfile_t*, const char*, int);
extern char\t\t*sfgets(Sfile_t*, char*, int);
extern int\t\tsfnputc(Sfile_t*, unsigned char, int);
extern int\t\t_sfputu(Sfile_t*, unsigned long);
extern int\t\t_sfputl(Sfile_t*, long);
extern long\t\t_sfgetl(Sfile_t*);
extern unsigned long\t_sfgetu(Sfile_t*);
extern long\t\t_sfgetl(Sfile_t*);
extern int\t\t_sfputd(Sfile_t*, double);
extern double\t\tsfgetd(Sfile_t*);
extern int\t\tsfungetc(Sfile_t*, int);
extern char\t\t*_sfcvt(double, int, int*, int*, int);
extern int\t\tsfprintf(Sfile_t*, const char*, ...);
extern int\t\tsfsprintf(char*, int, const char*, ...);
extern int\t\tsfscanf(Sfile_t*, const char*, ...);
extern int\t\tsfsscanf(const char*, const char*, ...);
extern Sfile_t*\t\tsfstring(Sfile_t*);
extern int\t\tsfvprintf(Sfile_t*, const char*, va_list);
extern int\t\tsfvscanf(Sfile_t*, const char*, va_list);

#endif /* SF_READ */'
		OUTPUT - $'                  
/*
 * K. P. Vo
 * AT&T Bell Laboratories
 *
 * sfio public definitions
 */

#ifndef SF_READ\t\t/* protect against multiple #includes */

#ifndef NULL
#define NULL\t0
#endif
#ifndef EOF
#define EOF\t(-1)
#endif
#ifndef SEEK_SET
#define SEEK_SET\t0
#define SEEK_CUR\t1
#define SEEK_END\t2
#endif

typedef struct _sfdc_\tSfdisc_t;
typedef struct _sfio_\tSfile_t;

#define SFIO\tSfile_t
#define SFDISC\tSfdisc_t

/* discipline structure */
struct _sfdc_
{
\tint\t(*readf) __PROTO__((Sfile_t*, unsigned char*, int, char*));
\tint\t(*writef) __PROTO__((Sfile_t*, const unsigned char*, int, char*));
\tlong\t(*seekf) __PROTO__((Sfile_t*, long, int, char*));
\tint\t(*exceptf) __PROTO__((Sfile_t*, int, char*));
\tchar\t*handle;\t/* to store any state information\t*/
};

/* a file structure */
struct _sfio_
{
\tunsigned char\t*next;\t/* next position to read/write from\t*/
\tunsigned char\t*endw;\t/* end of write buffer\t\t\t*/
\tunsigned char\t*endr;\t/* end of read buffer\t\t\t*/
\tunsigned char\t*endb;\t/* end of buffer\t\t\t*/
\tstruct _sfio_\t*push;\t/* the stream that was pushed on\t*/
\tshort\t\tflags;\t/* type of stream\t\t\t*/
\tshort\t\tfile;\t/* file descriptor\t\t\t*/
\tlong\t\torig;\t/* where we start in the file\t\t*/
#ifdef _SFIO_PRIVATE
\t_SFIO_PRIVATE
#endif
};

/* bits for various types of files */
#define\tSF_READ\t\t000001\t/* open for reading\t\t\t*/
#define SF_WRITE\t000002\t/* open for writing\t\t\t*/
#define SF_STRING\t000004\t/* a string stream\t\t\t*/
#define SF_APPEND\t000010\t/* associated file is in append mode\t*/
#define SF_RELATIVE\t000020\t/* file pos is relative to starting pos\t*/
#define SF_MALLOC\t000040\t/* buffered space malloc-ed\t\t*/
#define SF_LINE\t\t000100\t/* line buffering\t\t\t*/
#define SF_KEEPFD\t000200\t/* keep file opened when closing stream\t*/
#define SF_SHARE\t000400\t/* file stream that is shared\t\t*/
#define SF_REUSE\t001000\t/* keep stream space after closing\t*/
#define SF_FLAGS\t000777\t/* PUBLIC FLAGS PASSABLE TO SFNEW()\t*/
#define SF_SETS\t\t001743\t/* flags passable to sfsetflag()\t*/

#define SF_EOF\t\t002000\t/* eof was detected\t\t\t*/
#define SF_ERROR\t004000\t/* an error happened\t\t\t*/

#define SF_BUFSIZE\t8192\t/* suggested default buffer size\t*/
#define SF_UNBOUND\t(-1)\t/* unbounded buffer size\t\t*/

#define\tsfstdin\t\t(&_Sfstdin)\t/* standard input stream\t*/
#define\tsfstdout\t(&_Sfstdout)\t/* standard output stream\t*/
#define\tsfstderr\t(&_Sfstderr)\t/* standard error stream\t*/

/* fast in-line functions */
#define sfputc(f,c)\t((f)->next >= (f)->endw ? \\
\t\t\t\t_sfflsbuf(f,(int)((unsigned char)(c))) : \\
\t\t\t\t(int)(*(f)->next++ = (unsigned char)(c)))
#define sfgetc(f)\t((f)->next >= (f)->endr ? _sffilbuf(f,1) : (int)(*(f)->next++))
#define sfslen()\t(_Sfi)
#define sffileno(f)\t((f)->file)
#define sforigin(f)\t(((f)->flags&SF_STRING) ? 0L : (f)->orig)
#define sfeof(f)\t(((f)->flags&(SF_EOF|SF_STRING)) && (f)->next >= (f)->endb)
#define sferror(f)\t((f)->flags&SF_ERROR)
#define sfclearerr(f)\t((f)->flags &= ~SF_ERROR)
#define sfpushed(f)\t((f)->push)

#define sfpeek(f,bufp)\t(((bufp) || \\
\t\t\t  ((f)->flags&(SF_READ|SF_WRITE|SF_STRING)) == \\
\t\t\t   (SF_READ|SF_WRITE|SF_STRING)) ? _sfpeek(f,bufp) : \\
\t\t\t\t((f)->endb - (f)->next))
#define sfsync(f)\t(((f) && (((Sfile_t*)(f))->flags&SF_STRING)) ? 0 : _sfsync(f))

/* coding long integers in a portable and compact fashion */
#define SF_SBITS\t6
#define SF_UBITS\t7
#define SF_SIGN\t\t(1 << SF_SBITS)
#define SF_MORE\t\t(1 << SF_UBITS)
#define SF_U1\t\tSF_MORE
#define SF_U2\t\t(SF_U1*SF_U1)
#define SF_U3\t\t(SF_U2*SF_U1)
#define SF_U4\t\t(SF_U3*SF_U1)
#define sfulen(v)\t((v) < SF_U1 ? 1 : (v) < SF_U2 ? 2 : \\
\t\t\t (v) < SF_U3 ? 3 : (v) < SF_U4 ? 4 : 5)
#define sfgetu(f)\t((_Sfi = sfgetc(f)) < 0 ? -1 : \\
\t\t\t\t((_Sfi&SF_MORE) ? _sfgetu(f) : (unsigned long)_Sfi))
#define sfgetl(f)\t((_Sfi = sfgetc(f)) < 0 ? -1 : \\
\t\t\t\t((_Sfi&(SF_MORE|SF_SIGN)) ? _sfgetl(f) : (long)_Sfi))
#define sfputu(f,v)\t_sfputu((f),(unsigned long)(v))
#define sfputl(f,v)\t_sfputl((f),(long)(v))
#define sfputd(f,v)\t_sfputd((f),(double)(v))

#define sfecvt(v,n,d,s)\t_sfcvt((v),(n),(d),(s),1)
#define sffcvt(v,n,d,s)\t_sfcvt((v),(n),(d),(s),0)

extern __MANGLE__ int\t\t_Sfi;
extern __MANGLE__ Sfile_t\t\t_Sfstdin, _Sfstdout, _Sfstderr;

extern __MANGLE__ Sfile_t\t\t*sfnew __PROTO__((Sfile_t*, unsigned char*, int, int, int));
extern __MANGLE__ Sfile_t\t\t*sfopen __PROTO__((Sfile_t*, const char*, const char*));
extern __MANGLE__ Sfile_t\t\t*sfdopen __PROTO__((int, const char*));
extern __MANGLE__ Sfile_t\t\t*sfpopen __PROTO__((const char*, const char*, Sfile_t**));
extern __MANGLE__ Sfile_t\t\t*sfstack __PROTO__((Sfile_t*, Sfile_t*));
extern __MANGLE__ Sfile_t\t\t*sftmpfile __PROTO__((void));
extern __MANGLE__ int\t\t_sfflsbuf __PROTO__((Sfile_t*, int));
extern __MANGLE__ int\t\t_sffilbuf __PROTO__((Sfile_t*, int));
extern __MANGLE__ int\t\t_sfsync __PROTO__((Sfile_t*));
extern __MANGLE__ int\t\t_sfpeek __PROTO__((Sfile_t*, unsigned char**));
extern __MANGLE__ int\t\tsfclrlock __PROTO__((Sfile_t*));
extern __MANGLE__ unsigned char*\tsfsetbuf __PROTO__((Sfile_t*, unsigned char*, int));
extern __MANGLE__ Sfdisc_t*\tsfsetdisc __PROTO__((Sfile_t*,Sfdisc_t*));
extern __MANGLE__ int\t\tsfsetflag __PROTO__((Sfile_t*, int, int));
extern __MANGLE__ int\t\tsfpool __PROTO__((Sfile_t*, Sfile_t*, int));
extern __MANGLE__ int\t\tsfread __PROTO__((Sfile_t*, unsigned char*, int));
extern __MANGLE__ int\t\tsfwrite __PROTO__((Sfile_t*, const unsigned char*, int));
extern __MANGLE__ int\t\tsfmove __PROTO__((Sfile_t*, Sfile_t*, long, int));
extern __MANGLE__ int\t\tsfclose __PROTO__((Sfile_t*));
extern __MANGLE__ long\t\tsftell __PROTO__((Sfile_t*));
extern __MANGLE__ long\t\tsfseek __PROTO__((Sfile_t*, long, int));
extern __MANGLE__ int\t\tsfllen __PROTO__((long));
extern __MANGLE__ int\t\tsfdlen __PROTO__((double));
extern __MANGLE__ int\t\tsfputs __PROTO__((Sfile_t*, const char*, int));
extern __MANGLE__ char\t\t*sfgets __PROTO__((Sfile_t*, char*, int));
extern __MANGLE__ int\t\tsfnputc __PROTO__((Sfile_t*, unsigned char, int));
extern __MANGLE__ int\t\t_sfputu __PROTO__((Sfile_t*, unsigned long));
extern __MANGLE__ int\t\t_sfputl __PROTO__((Sfile_t*, long));
extern __MANGLE__ long\t\t_sfgetl __PROTO__((Sfile_t*));
extern __MANGLE__ unsigned long\t_sfgetu __PROTO__((Sfile_t*));
extern __MANGLE__ long\t\t_sfgetl __PROTO__((Sfile_t*));
extern __MANGLE__ int\t\t_sfputd __PROTO__((Sfile_t*, double));
extern __MANGLE__ double\t\tsfgetd __PROTO__((Sfile_t*));
extern __MANGLE__ int\t\tsfungetc __PROTO__((Sfile_t*, int));
extern __MANGLE__ char\t\t*_sfcvt __PROTO__((double, int, int*, int*, int));
extern __MANGLE__ int\t\tsfprintf __PROTO__((Sfile_t*, const char*, ...));
extern __MANGLE__ int\t\tsfsprintf __PROTO__((char*, int, const char*, ...));
extern __MANGLE__ int\t\tsfscanf __PROTO__((Sfile_t*, const char*, ...));
extern __MANGLE__ int\t\tsfsscanf __PROTO__((const char*, const char*, ...));
extern __MANGLE__ Sfile_t*\t\tsfstring __PROTO__((Sfile_t*));
extern __MANGLE__ int\t\tsfvprintf __PROTO__((Sfile_t*, const char*, va_list));
extern __MANGLE__ int\t\tsfvscanf __PROTO__((Sfile_t*, const char*, va_list));

#endif /* SF_READ */'
	EXEC -nh
		INPUT - $'#pragma prototyped

extern int\terrcount;\t\t/* level>=ERROR_ERROR count\t*/
extern int\terrno;\t\t\t/* system call error status\t*/

extern void\terror(int, ...);
extern void\tliberror(char*, int, ...);
extern void*\tseterror(int, ...);
extern void\tverror(char*, int, va_list);

extern int\tftwalk(char*, int (*)(struct FTW*), int, int (*)(struct FTW*, struct FTW*));

int
ftwalk(char* file, int (*userf)(struct FTW*), int flags, int (*comparf)(struct FTW*, struct FTW*))
{
\tfunction(0);
\treturn(0);
}

void
test()
{
\tint\t(*fun)(char*, int);

\treturn((*fun)("", 0));
}'
		OUTPUT - $'                  

extern __MANGLE__ int\terrcount;\t\t/* level>=ERROR_ERROR count\t*/
extern __MANGLE__ int\terrno;\t\t\t/* system call error status\t*/

extern __MANGLE__ void\terror __PROTO__((int, ...));
extern __MANGLE__ void\tliberror __PROTO__((char*, int, ...));
extern __MANGLE__ __V_*\tseterror __PROTO__((int, ...));
extern __MANGLE__ void\tverror __PROTO__((char*, int, va_list));

extern __MANGLE__ int\tftwalk __PROTO__((char*, int (*)(struct FTW*), int, int (*)(struct FTW*, struct FTW*)));

int
ftwalk __PARAM__((char* file, int (*userf)(struct FTW*), int flags, int (*comparf)(struct FTW*, struct FTW*)), (file, userf, flags, comparf)) __OTORP__(char* file; int (*userf)(); int flags; int (*comparf)();)
#line 15
{
\tfunction(0);
\treturn(0);
}

void
test()
{
\tint\t(*fun) __PROTO__((char*, int));

\treturn((*fun)("", 0));
}'
	EXEC -nh
		INPUT - $'#pragma prototyped

/*
 * 02/20/91 not processed correctly yet
 */

f()
{
\tx = ((void(*)(int)))b;
\tx(a, ((void(*)(int)))b, c);
}'
		OUTPUT - $'                  

/*
 * 02/20/91 not processed correctly yet
 */

f()
{
\tx = ((void(*)(int)))b;
\tx(a, ((void(*)(int)))b, c);
}'
	EXEC -nh
		INPUT - $'#pragma prototyped
static void (*signal)(void (*handler)(int))
{
\tcall(*p);
}'
		OUTPUT - $'                  
static void (*signal) __PARAM__((void (*handler)(int)), (handler)) __OTORP__(void (*handler)();)
#line 3
{
\tcall(*p);
}'
	EXEC -nh
		INPUT - $'#pragma prototyped
typedef void (*SH_SIGTYPE)(int,void(*)(int));'
		OUTPUT - $'                  
typedef void (*SH_SIGTYPE) __PROTO__((int,void(*)(int)));'
	EXEC -nh
		INPUT - $'#pragma prototyped
/*
 * Main program for various posix programs
 */

#define str(x)\t#x
#define string(x)\tstr(x)
#define paste(a,b)\ta##b
#define fun_name(x)\tpaste(b_,x)

#if !lint
  static const char id[]="
@(#)"string(PROG)" (AT&T Bell Laboratories) "__DATE__"
";
#endif

main(int argc, char *argv[])
{
\texit(fun_name(PROG)(argc,argv));
}'
		OUTPUT - $'                  
/*
 * Main program for various posix programs
 */

#if defined(__STDC__) || defined(__STDPP__)
#define str(x)\t#x
#else
#define str(x)"x"
#endif

#line 7
#define string(x)\tstr(x)
#if defined(__STDC__) || defined(__STDPP__)
#define paste(a,b)\ta##b
#else
#define paste(a,b)\ta/**/b
#endif

#line 9
#define fun_name(x)\tpaste(b_,x)

#if !lint
  static const char id[]="
@(#)"string(PROG)" (AT&T Bell Laboratories) "__DATE__"
";
#endif

main __PARAM__((int argc, char *argv[]), (argc, argv)) __OTORP__(int argc; char *argv[];)
#line 18
{
\texit(fun_name(PROG)(argc,argv));
}'

TEST 04 'more headers'
	EXEC -nh
		INPUT - $'#pragma prototyped
/*
 * Advanced Software Technology Department
 * AT&T Bell Laboratories
 *
 * libast miscellaneous interface definitions plus
 *
 *\t<limits.h>
 *\t<stddef.h>
 *\t<stdlib.h>
 *\t<sys/types.h>
 *\t<string.h>
 *\t<unistd.h>
 */

#ifndef __AST_H__
#define __AST_H__

#include <limits.h>

struct stat;
struct _sfio_;

#if defined(__STDC__) || defined(__cplusplus)

#include <stddef.h>
#include <stdlib.h>

#define strcopy\t\t_SYS_strcopy
#define strdup\t\t_SYS_strdup
#define strelapsed\t_SYS_strelapsed
#define strerror\t_SYS_strerror
#define stresc\t\t_SYS_stresc
#define streval\t\t_SYS_streval
#define strgid\t\t_SYS_strgid
#define strmatch\t_SYS_strmatch
#define strmode\t\t_SYS_strmode
#define stropt\t\t_SYS_stropt
#define strperm\t\t_SYS_strperm
#define strsignal\t_SYS_strsignal
#define strtape\t\t_SYS_strtape
#define strton\t\t_SYS_strton
#define struid\t\t_SYS_struid

#include <string.h>

#undef\tstrcopy
#undef\tstrdup
#undef\tstrelapsed
#undef\tstrerror
#undef\tstresc
#undef\tstreval
#undef\tstrgid
#undef\tstrmatch
#undef\tstrmode
#undef\tstropt
#undef\tstrperm
#undef\tstrsignal
#undef\tstrtape
#undef\tstrton
#undef\tstruid

#else

/* <stddef.h> */

#ifndef _PTRDIFF_T
#define _PTRDIFF_T
typedef long ptrdiff_t;
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned size_t;
#endif

#ifndef _WCHAR_T
#define _WCHAR_T
typedef char wchar_t;
#endif

#define offsetof(type,member) ((size_t)&(((type*)0)->member))

/* <stdlib.h> */

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0
#define MB_CUR_MAX 1
#define RAND_MAX 32767

typedef struct
{
\tint quot;
\tint rem;
} div_t;

typedef struct
{
\tlong quot;
\tlong rem;
} ldiv_t;

extern double\t\tatof(const char*);
extern int\t\tatoi(const char*);
extern long\t\tatol(const char*);
extern double\t\tstrtod(const char*, char**);
extern long\t\tstrtol(const char*, char**, int);
extern unsigned long\tstrtoul(const char*, char**, int);

extern int\t\trand(void);
extern void\t\tsrand(unsigned int);

extern void*\t\tcalloc(size_t, size_t);
extern void\t\tfree(void*);
extern void*\t\tmalloc(size_t);
extern void*\t\trealloc(void*, size_t);

extern void\t\tabort(void);
extern int\t\tatexit(void(*)(void));
extern void\t\texit(int);
extern char*\t\tgetenv(const char*);
extern int\t\tsystem(const char*);

extern void*\t\tbsearch(const void*, const void*, size_t, size_t,
\t\t \t\tint(*)(const void*, const void*));
extern void\t\tqsort(void*, size_t, size_t,
\t\t\t\tint(*)(const void*, const void*));

extern int\t\tabs(int);
extern div_t\t\tdiv(int, int);
extern long\t\tlabs(long);
extern ldiv_t\t\tldiv(long, long);

extern int\t\tmblen(const char*, size_t);
extern int\t\tmbtowc(wchar_t*, const char*, size_t);
extern int\t\twctomb(char*, wchar_t);
extern size_t\t\tmbstowcs(wchar_t*, const char*, size_t);
extern size_t\t\twcstombs(char*, const wchar_t*, size_t);

/* <string.h> */

extern void*\t\tmemchr(const void*, int, size_t);
extern int\t\tmemcmp(const void*, const void*, size_t);
extern void*\t\tmemcpy(void*, const void*, size_t);
extern void*\t\tmemmove(void*, const void*, size_t);
extern void*\t\tmemset(void*, int, size_t);
extern char*\t\tstrcat(char*, const char*);
extern char*\t\tstrchr(const char*, int);
extern int\t\tstrcmp(const char*, const char*);
extern int\t\tstrcoll(const char*, const char*);
extern char*\t\tstrcpy(char*, const char*);
extern size_t\t\tstrcspn(const char*, const char*);
extern size_t\t\tstrlen(const char*);
extern char*\t\tstrncat(char*, const char*, size_t);
extern int\t\tstrncmp(const char*, const char*, size_t);
extern char*\t\tstrncpy(char*, const char*, size_t);
extern char*\t\tstrpbrk(const char*, const char*);
extern char*\t\tstrrchr(const char*, int);
extern size_t\t\tstrspn(const char*, const char*);
extern char*\t\tstrstr(const char*, const char*);
extern char*\t\tstrtok(char*, const char*);
extern size_t\t\tstrxfrm(char*, const char*, size_t);

#endif

#ifndef ptrdiff_t
#define _UNDEF_ptrdiff_t
#define ptrdiff_t\t_SYS_ptrdiff_t
#endif
#ifndef size_t
#define _UNDEF_size_t
#define size_t\t\t_SYS_size_t
#endif
#ifndef wchar_t
#define _UNDEF_wchar_t
#define wchar_t\t\t_SYS_wchar_t
#endif

#include <sys/types.h>

#ifdef\t_UNDEF_ptrdiff_t
#undef\t_UNDEF_ptrdiff_t
#undef\tptrdiff_t
#endif
#ifdef\t_UNDEF_size_t
#undef\t_UNDEF_size_t
#undef\tsize_t
#endif
#ifdef\t_UNDEF_wchar_t
#undef\t_UNDEF_wchar_t
#undef\twchar_t
#endif

#if defined(__cplusplus) || defined(_POSIX_SOURCE)

#include <unistd.h>

#else

/* <unistd.h> */

#include <ast.unistd.h>

#define\tSTDIN_FILENO\t0
#define\tSTDOUT_FILENO\t1
#define\tSTDERR_FILENO\t2

#ifndef NULL
#define\tNULL\t\t0
#endif

#ifndef SEEK_SET
#define\tSEEK_SET\t0
#define\tSEEK_CUR\t1
#define\tSEEK_END\t2
#endif

#ifndef\tF_OK
#define\tF_OK\t\t0
#define\tX_OK\t\t1
#define\tW_OK\t\t2
#define\tR_OK\t\t4
#endif

extern void\t\t_exit(int);
extern int\t\taccess(const char*, int);
extern unsigned\t\talarm(unsigned);
extern int\t\tchdir(const char*);
extern int\t\tchown(const char*, int, int);
extern int\t\tclose(int);
extern int\t\tdup(int);
extern int\t\tdup2(int, int);
extern int\t\texecl(const char*, ...);
extern int\t\texecle(const char*, ...);
extern int\t\texeclp(const char*, ...);
extern int\t\texecv(const char*, char*[]);
extern int\t\texecve(const char*, char*[], char*[]);
extern int\t\texecvp(const char*, char*[]);
extern int\t\tfork(void);
extern long\t\tfpathconf(int, int);
extern char*\t\tgetcwd(char*, int);
extern int\t\tgetegid(void);
extern int\t\tgeteuid(void);
extern int\t\tgetgid(void);
extern int\t\tgetgroups(int, int[]);
extern char*\t\tgetlogin(void);
extern int\t\tgetpgrp(void);
extern int\t\tgetpid(void);
extern int\t\tgetppid(void);
extern int\t\tgetuid(void);
extern int\t\tisatty(int);
extern int\t\tlink(const char*, const char*);
extern long\t\tlseek(int, long, int);
extern long\t\tpathconf(const char*, int);
extern int\t\tpause(void);
extern int\t\tpipe(int[]);
extern int\t\tread(int, char*, unsigned int);
extern int\t\trmdir(const char*);
extern int\t\tsetgid(int);
extern int\t\tsetpgid(int, int);
extern int\t\tsetsid(void);
extern int\t\tsetuid(int);
extern unsigned\t\tsleep(unsigned int);
extern long\t\tsysconf(int);
extern int\t\ttcgetpgrp(int);
extern int\t\ttcsetpgrp(int, int);
extern char*\t\tttyname(int);
extern int\t\tunlink(const char*);
extern int\t\twrite(int, const char*, unsigned int);

#endif

/* ast */

#include <sfio.h>

#ifndef PATH_MAX
#define PATH_MAX\t1024
#endif

/*
 * pathcanon() flags
 */

#define PATH_PHYSICAL\t01
#define PATH_DOTDOT\t02
#define PATH_EXISTS\t04

/*
 * pathaccess() flags
 */

#define PATH_READ\t004
#define PATH_WRITE\t002
#define PATH_EXECUTE\t001
#define\tPATH_REGULAR\t010
#define PATH_ABSOLUTE\t020

#define allocate(t,n,x)\t(t*)calloc(1,sizeof(t)*(n)+(x))
#define deallocate(p)\tfree((char*)(p))
#define elements(x)\t(sizeof(x)/sizeof(x[0]))
#define reallocate(p,t,n,x)\t(t*)realloc((char*)(p),sizeof(t)*(n)+(x))
#define round(x,y)\t(((x)+(y)-1)&~((y)-1))
#define streq(a,b)\t(*(a)==*(b)&&!strcmp(a,b))

#define NOT_USED(x)\t(&x,1)

extern int\t\tchresc(const char*, char**);
extern int\t\tcmdclose(int);
extern int\t\tcmdkill(int, int);
extern int\t\tcmdopen(const char*, char**, char**, int*, const char*);
extern int\t\tcmdrun(const char*, char**);
extern long\t\tcopy(int, int, int);
extern int\t\tctoi(const char*);
extern void\t\tcvtatoe(const char*, const char*, int);
extern void\t\tcvtetoa(const char*, const char*, int);
extern int\t\tfclex(int, int);
extern char*\t\tfmtbase(long, int, int);
extern char*\t\tfmtdev(struct stat*);
extern char*\t\tfmtelapsed(unsigned long, int);
extern char*\t\tfmtfs(struct stat*);
extern char*\t\tfmtgid(int);
extern char*\t\tfmtmatch(const char*);
extern char*\t\tfmtmode(int, int);
extern char*\t\tfmtperm(int);
extern char*\t\tfmtuid(int);
extern char*\t\tgetcwd(char*, int);
extern char*\t\tgetpath(void);
extern char*\t\tgetshell(void);
extern int\t\tgetsymlink(const char*, char*, int);
extern char*\t\tgetuniv(void);
extern void\t\thsort(char**, int, int(*)(const char*, const char*));
extern struct _sfio_*\tlexline(const char*, int, int*);
extern int\t\tlexscan(char*, char**, const char*, ...);
extern int\t\tlpstat(const char*, struct stat*);
extern char*\t\tmemdup(const char*, int);
extern char*\t\tpathaccess(char*, const char*, const char*, const char*, int);
extern char*\t\tpathcanon(char*, int);
extern char*\t\tpathcat(char*, const char*, int, const char*, const char*);
extern void\t\tpathcheck(const char*, const char*, const char*);
extern char*\t\tpathkey(char*, char*, const char*, const char*);
extern char*\t\tpathpath(char*, const char*, const char*, int);
extern char*\t\tpathprobe(char*, char*, const char*, const char*, int);
extern char*\t\tpathrepl(char*, const char*, const char*);
extern char*\t\tpathtemp(char*, const char*, const char*);
extern int\t\tputsymlink(const char*, const char*);
extern int\t\tquery(int, const char*, ...);
extern int\t\tsetcwd(const char*, char*);
extern int\t\tsetuniv(const char*);
extern int\t\tsigcritical(int);
extern char*\t\tstrcopy(char*, const char*);
extern char*\t\tstrdup(const char*);
extern unsigned long\tstrelapsed(const char*, char**, int);
extern char*\t\tstrerror(int);
extern int\t\tstresc(char*);
extern long\t\tstreval(const char*, char**, long(*)(const char*, char**));
extern int\t\tstrgid(const char*);
extern int\t\tstrmatch(const char*, const char*);
extern int\t\tstrmode(const char*);
extern int\t\tstropt(const char*, char*, int, int(*)(char*, char*, int, const char*), char*);
extern int\t\tstrperm(const char*, char**, int);
extern char*\t\tstrsignal(int);
extern char*\t\tstrtape(const char*, char**);
extern long\t\tstrton(const char*, char**, char*, int);
extern int\t\tstruid(const char*);
extern int\t\ttabindex(char*, int, char*);
extern char*\t\ttablook(char*, int, char*);
extern long\t\ttblocks(struct stat*);
extern char*\t\ttokopen(char*, int);
extern void\t\ttokclose(char*);
extern char*\t\ttokread(char*);
extern int\t\ttouch(const char*, time_t, time_t, int);
extern void\t\twinsize(int*, int*);

#endif'
		OUTPUT - $'                  
/*
 * Advanced Software Technology Department
 * AT&T Bell Laboratories
 *
 * libast miscellaneous interface definitions plus
 *
 *\t<limits.h>
 *\t<stddef.h>
 *\t<stdlib.h>
 *\t<sys/types.h>
 *\t<string.h>
 *\t<unistd.h>
 */

#ifndef __AST_H__
#define __AST_H__

#include <limits.h>

struct stat;
struct _sfio_;

#if defined(__STDC__) || defined(__cplusplus)

#include <stddef.h>
#include <stdlib.h>

#define strcopy\t\t_SYS_strcopy
#define strdup\t\t_SYS_strdup
#define strelapsed\t_SYS_strelapsed
#define strerror\t_SYS_strerror
#define stresc\t\t_SYS_stresc
#define streval\t\t_SYS_streval
#define strgid\t\t_SYS_strgid
#define strmatch\t_SYS_strmatch
#define strmode\t\t_SYS_strmode
#define stropt\t\t_SYS_stropt
#define strperm\t\t_SYS_strperm
#define strsignal\t_SYS_strsignal
#define strtape\t\t_SYS_strtape
#define strton\t\t_SYS_strton
#define struid\t\t_SYS_struid

#include <string.h>

#undef\tstrcopy
#undef\tstrdup
#undef\tstrelapsed
#undef\tstrerror
#undef\tstresc
#undef\tstreval
#undef\tstrgid
#undef\tstrmatch
#undef\tstrmode
#undef\tstropt
#undef\tstrperm
#undef\tstrsignal
#undef\tstrtape
#undef\tstrton
#undef\tstruid

#else

/* <stddef.h> */

#ifndef _PTRDIFF_T
#define _PTRDIFF_T
typedef long ptrdiff_t;
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned size_t;
#endif

#ifndef _WCHAR_T
#define _WCHAR_T
typedef char wchar_t;
#endif

#define offsetof(type,member) ((size_t)&(((type*)0)->member))

/* <stdlib.h> */

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0
#define MB_CUR_MAX 1
#define RAND_MAX 32767

typedef struct
{
\tint quot;
\tint rem;
} div_t;

typedef struct
{
\tlong quot;
\tlong rem;
} ldiv_t;

extern __MANGLE__ double\t\tatof __PROTO__((const char*));
extern __MANGLE__ int\t\tatoi __PROTO__((const char*));
extern __MANGLE__ long\t\tatol __PROTO__((const char*));
extern __MANGLE__ double\t\tstrtod __PROTO__((const char*, char**));
extern __MANGLE__ long\t\tstrtol __PROTO__((const char*, char**, int));
extern __MANGLE__ unsigned long\tstrtoul __PROTO__((const char*, char**, int));

extern __MANGLE__ int\t\trand __PROTO__((void));
extern __MANGLE__ void\t\tsrand __PROTO__((unsigned int));

extern __MANGLE__ __V_*\t\tcalloc __PROTO__((size_t, size_t));
extern __MANGLE__ void\t\tfree __PROTO__((__V_*));
extern __MANGLE__ __V_*\t\tmalloc __PROTO__((size_t));
extern __MANGLE__ __V_*\t\trealloc __PROTO__((__V_*, size_t));

extern __MANGLE__ void\t\tabort __PROTO__((void));
extern __MANGLE__ int\t\tatexit __PROTO__((void(*)(void)));
extern __MANGLE__ void\t\texit __PROTO__((int));
extern __MANGLE__ char*\t\tgetenv __PROTO__((const char*));
extern __MANGLE__ int\t\tsystem __PROTO__((const char*));

extern __MANGLE__ __V_*\t\tbsearch __PROTO__((const __V_*, const __V_*, size_t, size_t,
\t\t \t\tint(*)(const __V_*, const __V_*)));
extern __MANGLE__ void\t\tqsort __PROTO__((__V_*, size_t, size_t,
\t\t\t\tint(*)(const __V_*, const __V_*)));

extern __MANGLE__ int\t\tabs __PROTO__((int));
extern __MANGLE__ div_t\t\tdiv __PROTO__((int, int));
extern __MANGLE__ long\t\tlabs __PROTO__((long));
extern __MANGLE__ ldiv_t\t\tldiv __PROTO__((long, long));

extern __MANGLE__ int\t\tmblen __PROTO__((const char*, size_t));
extern __MANGLE__ int\t\tmbtowc __PROTO__((wchar_t*, const char*, size_t));
extern __MANGLE__ int\t\twctomb __PROTO__((char*, wchar_t));
extern __MANGLE__ size_t\t\tmbstowcs __PROTO__((wchar_t*, const char*, size_t));
extern __MANGLE__ size_t\t\twcstombs __PROTO__((char*, const wchar_t*, size_t));

/* <string.h> */

extern __MANGLE__ __V_*\t\tmemchr __PROTO__((const __V_*, int, size_t));
extern __MANGLE__ int\t\tmemcmp __PROTO__((const __V_*, const __V_*, size_t));
extern __MANGLE__ __V_*\t\tmemcpy __PROTO__((__V_*, const __V_*, size_t));
extern __MANGLE__ __V_*\t\tmemmove __PROTO__((__V_*, const __V_*, size_t));
extern __MANGLE__ __V_*\t\tmemset __PROTO__((__V_*, int, size_t));
extern __MANGLE__ char*\t\tstrcat __PROTO__((char*, const char*));
extern __MANGLE__ char*\t\tstrchr __PROTO__((const char*, int));
extern __MANGLE__ int\t\tstrcmp __PROTO__((const char*, const char*));
extern __MANGLE__ int\t\tstrcoll __PROTO__((const char*, const char*));
extern __MANGLE__ char*\t\tstrcpy __PROTO__((char*, const char*));
extern __MANGLE__ size_t\t\tstrcspn __PROTO__((const char*, const char*));
extern __MANGLE__ size_t\t\tstrlen __PROTO__((const char*));
extern __MANGLE__ char*\t\tstrncat __PROTO__((char*, const char*, size_t));
extern __MANGLE__ int\t\tstrncmp __PROTO__((const char*, const char*, size_t));
extern __MANGLE__ char*\t\tstrncpy __PROTO__((char*, const char*, size_t));
extern __MANGLE__ char*\t\tstrpbrk __PROTO__((const char*, const char*));
extern __MANGLE__ char*\t\tstrrchr __PROTO__((const char*, int));
extern __MANGLE__ size_t\t\tstrspn __PROTO__((const char*, const char*));
extern __MANGLE__ char*\t\tstrstr __PROTO__((const char*, const char*));
extern __MANGLE__ char*\t\tstrtok __PROTO__((char*, const char*));
extern __MANGLE__ size_t\t\tstrxfrm __PROTO__((char*, const char*, size_t));

#endif

#ifndef ptrdiff_t
#define _UNDEF_ptrdiff_t
#define ptrdiff_t\t_SYS_ptrdiff_t
#endif
#ifndef size_t
#define _UNDEF_size_t
#define size_t\t\t_SYS_size_t
#endif
#ifndef wchar_t
#define _UNDEF_wchar_t
#define wchar_t\t\t_SYS_wchar_t
#endif

#include <sys/types.h>

#ifdef\t_UNDEF_ptrdiff_t
#undef\t_UNDEF_ptrdiff_t
#undef\tptrdiff_t
#endif
#ifdef\t_UNDEF_size_t
#undef\t_UNDEF_size_t
#undef\tsize_t
#endif
#ifdef\t_UNDEF_wchar_t
#undef\t_UNDEF_wchar_t
#undef\twchar_t
#endif

#if defined(__cplusplus) || defined(_POSIX_SOURCE)

#include <unistd.h>

#else

/* <unistd.h> */

#include <ast.unistd.h>

#define\tSTDIN_FILENO\t0
#define\tSTDOUT_FILENO\t1
#define\tSTDERR_FILENO\t2

#ifndef NULL
#define\tNULL\t\t0
#endif

#ifndef SEEK_SET
#define\tSEEK_SET\t0
#define\tSEEK_CUR\t1
#define\tSEEK_END\t2
#endif

#ifndef\tF_OK
#define\tF_OK\t\t0
#define\tX_OK\t\t1
#define\tW_OK\t\t2
#define\tR_OK\t\t4
#endif

extern __MANGLE__ void\t\t_exit __PROTO__((int));
extern __MANGLE__ int\t\taccess __PROTO__((const char*, int));
extern __MANGLE__ unsigned\t\talarm __PROTO__((unsigned));
extern __MANGLE__ int\t\tchdir __PROTO__((const char*));
extern __MANGLE__ int\t\tchown __PROTO__((const char*, int, int));
extern __MANGLE__ int\t\tclose __PROTO__((int));
extern __MANGLE__ int\t\tdup __PROTO__((int));
extern __MANGLE__ int\t\tdup2 __PROTO__((int, int));
extern __MANGLE__ int\t\texecl __PROTO__((const char*, ...));
extern __MANGLE__ int\t\texecle __PROTO__((const char*, ...));
extern __MANGLE__ int\t\texeclp __PROTO__((const char*, ...));
extern __MANGLE__ int\t\texecv __PROTO__((const char*, char*[]));
extern __MANGLE__ int\t\texecve __PROTO__((const char*, char*[], char*[]));
extern __MANGLE__ int\t\texecvp __PROTO__((const char*, char*[]));
extern __MANGLE__ int\t\tfork __PROTO__((void));
extern __MANGLE__ long\t\tfpathconf __PROTO__((int, int));
extern __MANGLE__ char*\t\tgetcwd __PROTO__((char*, int));
extern __MANGLE__ int\t\tgetegid __PROTO__((void));
extern __MANGLE__ int\t\tgeteuid __PROTO__((void));
extern __MANGLE__ int\t\tgetgid __PROTO__((void));
extern __MANGLE__ int\t\tgetgroups __PROTO__((int, int[]));
extern __MANGLE__ char*\t\tgetlogin __PROTO__((void));
extern __MANGLE__ int\t\tgetpgrp __PROTO__((void));
extern __MANGLE__ int\t\tgetpid __PROTO__((void));
extern __MANGLE__ int\t\tgetppid __PROTO__((void));
extern __MANGLE__ int\t\tgetuid __PROTO__((void));
extern __MANGLE__ int\t\tisatty __PROTO__((int));
extern __MANGLE__ int\t\tlink __PROTO__((const char*, const char*));
extern __MANGLE__ long\t\tlseek __PROTO__((int, long, int));
extern __MANGLE__ long\t\tpathconf __PROTO__((const char*, int));
extern __MANGLE__ int\t\tpause __PROTO__((void));
extern __MANGLE__ int\t\tpipe __PROTO__((int[]));
extern __MANGLE__ int\t\tread __PROTO__((int, char*, unsigned int));
extern __MANGLE__ int\t\trmdir __PROTO__((const char*));
extern __MANGLE__ int\t\tsetgid __PROTO__((int));
extern __MANGLE__ int\t\tsetpgid __PROTO__((int, int));
extern __MANGLE__ int\t\tsetsid __PROTO__((void));
extern __MANGLE__ int\t\tsetuid __PROTO__((int));
extern __MANGLE__ unsigned\t\tsleep __PROTO__((unsigned int));
extern __MANGLE__ long\t\tsysconf __PROTO__((int));
extern __MANGLE__ int\t\ttcgetpgrp __PROTO__((int));
extern __MANGLE__ int\t\ttcsetpgrp __PROTO__((int, int));
extern __MANGLE__ char*\t\tttyname __PROTO__((int));
extern __MANGLE__ int\t\tunlink __PROTO__((const char*));
extern __MANGLE__ int\t\twrite __PROTO__((int, const char*, unsigned int));

#endif

/* ast */

#include <sfio.h>

#ifndef PATH_MAX
#define PATH_MAX\t1024
#endif

/*
 * pathcanon() flags
 */

#define PATH_PHYSICAL\t01
#define PATH_DOTDOT\t02
#define PATH_EXISTS\t04

/*
 * pathaccess() flags
 */

#define PATH_READ\t004
#define PATH_WRITE\t002
#define PATH_EXECUTE\t001
#define\tPATH_REGULAR\t010
#define PATH_ABSOLUTE\t020

#define allocate(t,n,x)\t(t*)calloc(1,sizeof(t)*(n)+(x))
#define deallocate(p)\tfree((char*)(p))
#define elements(x)\t(sizeof(x)/sizeof(x[0]))
#define reallocate(p,t,n,x)\t(t*)realloc((char*)(p),sizeof(t)*(n)+(x))
#define round(x,y)\t(((x)+(y)-1)&~((y)-1))
#define streq(a,b)\t(*(a)==*(b)&&!strcmp(a,b))

#define NOT_USED(x)\t(&x,1)

extern __MANGLE__ int\t\tchresc __PROTO__((const char*, char**));
extern __MANGLE__ int\t\tcmdclose __PROTO__((int));
extern __MANGLE__ int\t\tcmdkill __PROTO__((int, int));
extern __MANGLE__ int\t\tcmdopen __PROTO__((const char*, char**, char**, int*, const char*));
extern __MANGLE__ int\t\tcmdrun __PROTO__((const char*, char**));
extern __MANGLE__ long\t\tcopy __PROTO__((int, int, int));
extern __MANGLE__ int\t\tctoi __PROTO__((const char*));
extern __MANGLE__ void\t\tcvtatoe __PROTO__((const char*, const char*, int));
extern __MANGLE__ void\t\tcvtetoa __PROTO__((const char*, const char*, int));
extern __MANGLE__ int\t\tfclex __PROTO__((int, int));
extern __MANGLE__ char*\t\tfmtbase __PROTO__((long, int, int));
extern __MANGLE__ char*\t\tfmtdev __PROTO__((struct stat*));
extern __MANGLE__ char*\t\tfmtelapsed __PROTO__((unsigned long, int));
extern __MANGLE__ char*\t\tfmtfs __PROTO__((struct stat*));
extern __MANGLE__ char*\t\tfmtgid __PROTO__((int));
extern __MANGLE__ char*\t\tfmtmatch __PROTO__((const char*));
extern __MANGLE__ char*\t\tfmtmode __PROTO__((int, int));
extern __MANGLE__ char*\t\tfmtperm __PROTO__((int));
extern __MANGLE__ char*\t\tfmtuid __PROTO__((int));
extern __MANGLE__ char*\t\tgetcwd __PROTO__((char*, int));
extern __MANGLE__ char*\t\tgetpath __PROTO__((void));
extern __MANGLE__ char*\t\tgetshell __PROTO__((void));
extern __MANGLE__ int\t\tgetsymlink __PROTO__((const char*, char*, int));
extern __MANGLE__ char*\t\tgetuniv __PROTO__((void));
extern __MANGLE__ void\t\thsort __PROTO__((char**, int, int(*)(const char*, const char*)));
extern __MANGLE__ struct _sfio_*\tlexline __PROTO__((const char*, int, int*));
extern __MANGLE__ int\t\tlexscan __PROTO__((char*, char**, const char*, ...));
extern __MANGLE__ int\t\tlpstat __PROTO__((const char*, struct stat*));
extern __MANGLE__ char*\t\tmemdup __PROTO__((const char*, int));
extern __MANGLE__ char*\t\tpathaccess __PROTO__((char*, const char*, const char*, const char*, int));
extern __MANGLE__ char*\t\tpathcanon __PROTO__((char*, int));
extern __MANGLE__ char*\t\tpathcat __PROTO__((char*, const char*, int, const char*, const char*));
extern __MANGLE__ void\t\tpathcheck __PROTO__((const char*, const char*, const char*));
extern __MANGLE__ char*\t\tpathkey __PROTO__((char*, char*, const char*, const char*));
extern __MANGLE__ char*\t\tpathpath __PROTO__((char*, const char*, const char*, int));
extern __MANGLE__ char*\t\tpathprobe __PROTO__((char*, char*, const char*, const char*, int));
extern __MANGLE__ char*\t\tpathrepl __PROTO__((char*, const char*, const char*));
extern __MANGLE__ char*\t\tpathtemp __PROTO__((char*, const char*, const char*));
extern __MANGLE__ int\t\tputsymlink __PROTO__((const char*, const char*));
extern __MANGLE__ int\t\tquery __PROTO__((int, const char*, ...));
extern __MANGLE__ int\t\tsetcwd __PROTO__((const char*, char*));
extern __MANGLE__ int\t\tsetuniv __PROTO__((const char*));
extern __MANGLE__ int\t\tsigcritical __PROTO__((int));
extern __MANGLE__ char*\t\tstrcopy __PROTO__((char*, const char*));
extern __MANGLE__ char*\t\tstrdup __PROTO__((const char*));
extern __MANGLE__ unsigned long\tstrelapsed __PROTO__((const char*, char**, int));
extern __MANGLE__ char*\t\tstrerror __PROTO__((int));
extern __MANGLE__ int\t\tstresc __PROTO__((char*));
extern __MANGLE__ long\t\tstreval __PROTO__((const char*, char**, long(*)(const char*, char**)));
extern __MANGLE__ int\t\tstrgid __PROTO__((const char*));
extern __MANGLE__ int\t\tstrmatch __PROTO__((const char*, const char*));
extern __MANGLE__ int\t\tstrmode __PROTO__((const char*));
extern __MANGLE__ int\t\tstropt __PROTO__((const char*, char*, int, int(*)(char*, char*, int, const char*), char*));
extern __MANGLE__ int\t\tstrperm __PROTO__((const char*, char**, int));
extern __MANGLE__ char*\t\tstrsignal __PROTO__((int));
extern __MANGLE__ char*\t\tstrtape __PROTO__((const char*, char**));
extern __MANGLE__ long\t\tstrton __PROTO__((const char*, char**, char*, int));
extern __MANGLE__ int\t\tstruid __PROTO__((const char*));
extern __MANGLE__ int\t\ttabindex __PROTO__((char*, int, char*));
extern __MANGLE__ char*\t\ttablook __PROTO__((char*, int, char*));
extern __MANGLE__ long\t\ttblocks __PROTO__((struct stat*));
extern __MANGLE__ char*\t\ttokopen __PROTO__((char*, int));
extern __MANGLE__ void\t\ttokclose __PROTO__((char*));
extern __MANGLE__ char*\t\ttokread __PROTO__((char*));
extern __MANGLE__ int\t\ttouch __PROTO__((const char*, time_t, time_t, int));
extern __MANGLE__ void\t\twinsize __PROTO__((int*, int*));

#endif'
	EXEC -nh
		INPUT - $'#pragma prototyped

int (*_3d_sysent[])() = { _exit };'
		OUTPUT - $'                  

int (*_3d_sysent[])(__VARARG__)= { _exit };'
	EXEC -nh
		INPUT - $'#pragma prototyped

#define a\tz
#define q(s)\ta #s b
#define n(a,b)\tA a ## b Z
#define z\ta
#define H(s)\t"/usr/include" # s ".h"'
		OUTPUT - $'                  

#define a\tz
#if defined(__STDC__) || defined(__STDPP__)
#define q(s)\ta #s b
#else
#define q(s)\ta"s"b
#endif

#line 5
#if defined(__STDC__) || defined(__STDPP__)
#define n(a,b)\tA a ## b Z
#else
#define n(a,b)\tA a/**/b Z
#endif

#line 6
#define z\ta
#if defined(__STDC__) || defined(__STDPP__)
#define H(s)\t"/usr/include" # s ".h"
#else
#define H(s)\t"/usr/include"s.h"
#endif

#line 8'
	EXEC -nh
		INPUT - $'#pragma prototyped
f()
{
\ti = 1;
#ifdef XENIX
        if(ex_xenix(p))
#endif ;
\texecve(p, &t[0] ,xecenv);
}'
		OUTPUT - $'                  
f()
{
\ti = 1;
#ifdef XENIX
        if(ex_xenix(p))
#endif ;
\texecve(p, &t[0] ,xecenv);
}'

TEST 05 'intermixed code'
	EXEC -nh
		INPUT - $'#pragma prototyped
/*
 *  edit.c - common routines for vi and emacs one line editors in shell
 *
 *   David Korn\t\t\t\tP.D. Sullivan
 *   AT&T Bell Laboratories\t\tAT&T Bell Laboratories
 *   Tel. x7975\t\t\t\tTel. x 2655
 *
 *   Coded April 1983.
 */

#include\t<errno.h>
#include\t<ast.h>
#include\t<sfio.h>
#include\t<ctype.h>
#include\t"FEATURE/options"
#include\t"lexstates.h"
#include\t"path.h"
#include\t"FEATURE/sigrestart"
#include\t"FEATURE/select"
#include\t"FEATURE/time"

#define MAXTRY\t12
#ifdef KSHELL
#   include\t"defs.h"
#   include\t"fault.h"
#   include\t"io.h"
#   include\t"terminal.h"
#   include\t"builtins.h"
#else
#   include\t"io.h"
#   include\t"terminal.h"
#   undef SIG_NORESTART
#   define SIG_NORESTART\t1
#   define _sobuf\ted_errbuf
    extern char ed_errbuf[];
    char e_version[] = "
@(#)Editlib version 01/31/92-alpha1\\0
";
#endif\t/* KSHELL */
#include\t"history.h"
#include\t"edit.h"

#define BAD\t-1
#define GOOD\t0
#define\tTRUE\t(-1)
#define\tFALSE\t0
#define\tSYSERR\t-1

#ifdef SHOPT_OLDTERMIO
#   undef tcgetattr
#   undef tcsetattr
#endif /* SHOPT_OLDTERMIO */

#ifdef RT
#   define VENIX 1
#endif\t/* RT */

#define lookahead\teditb.e_index
#define env\t\teditb.e_env
#define previous\teditb.e_lbuf
#define fildes\t\teditb.e_fd


#ifdef _hdr_sgtty
#   ifdef TIOCGETP
\tstatic int l_mask;
\tstatic struct tchars l_ttychars;
\tstatic struct ltchars l_chars;
\tstatic  char  l_changed;\t/* set if mode bits changed */
#\tdefine L_CHARS\t4
#\tdefine T_CHARS\t2
#\tdefine L_MASK\t1
#   endif /* TIOCGETP */
#endif /* _hdr_sgtty */

#ifdef KSHELL
     static char macro[]\t= "_??";
#    define slowsig()\t(sh.trapnote&SH_SIGSLOW)
     static int keytrap(char*, int, int, int);
#else
     struct edit editb;
     extern int errno;
#    define slowsig()\t(0)
#endif\t/* KSHELL */


static struct termios savetty;
static int savefd = -1;
#ifdef future
    static int compare(const char*, const char*, int);
#endif  /* future */
#if SHOPT_VSH || SHOPT_ESH
    static struct termios ttyparm;\t/* initial tty parameters */
    static struct termios nttyparm;\t/* raw tty parameters */
    static char bellchr[] = "\\7";\t/* bell char */
    static char *overlay(char*,const char*);
#endif /* SHOPT_VSH || SHOPT_ESH */


/*
 * This routine returns true if fd refers to a terminal
 * This should be equivalent to isatty
 */

int tty_check(int fd)
{
\tsavefd = -1;
\treturn(tty_get(fd,(struct termios*)0)==0);
}

/*
 * Get the current terminal attributes
 * This routine remembers the attributes and just returns them if it
 *   is called again without an intervening tty_set()
 */

int tty_get(int fd, struct termios *tty)
{
\tif(fd != savefd)
\t{
#ifndef SIG_NORESTART
\t\tvoid (*savint)() = sh.intfn;
\t\tsh.intfn = 0;
#endif\t/* SIG_NORESTART */
\t\twhile(tcgetattr(fd,&savetty) == SYSERR)
\t\t{
\t\t\tif(errno !=EINTR)
\t\t\t{
#ifndef SIG_NORESTART
\t\t\t\tsh.intfn = savint;
#endif\t/* SIG_NORESTART */
\t\t\t\treturn(SYSERR);
\t\t\t}
\t\t\terrno = 0;
\t\t}
#ifndef SIG_NORESTART
\t\tsh.intfn = savint;
#endif\t/* SIG_NORESTART */
\t\tsavefd = fd;
\t}
\tif(tty)
\t\t*tty = savetty;
\treturn(0);
}

/*
 * Set the terminal attributes
 * If fd<0, then current attributes are invalidated
 */

int tty_set(int fd, int action, struct termios *tty)
{
\tif(fd >=0)
\t{
#ifndef SIG_NORESTART
\t\tvoid (*savint)() = sh.intfn;
#endif\t/* SIG_NORESTART */
#ifdef future
\t\tif(savefd>=0 && compare(&savetty,tty,sizeof(struct termios)))
\t\t\treturn(0);
#endif
#ifndef SIG_NORESTART
\t\tsh.intfn = 0;
#endif\t/* SIG_NORESTART */
\t\twhile(tcsetattr(fd, action, tty) == SYSERR)
\t\t{
\t\t\tif(errno !=EINTR)
\t\t\t{
#ifndef SIG_NORESTART
\t\t\t\tsh.intfn = savint;
#endif\t/* SIG_NORESTART */
\t\t\t\treturn(SYSERR);
\t\t\t}
\t\t\terrno = 0;
\t\t}
#ifndef SIG_NORESTART
\t\tsh.intfn = savint;
#endif\t/* SIG_NORESTART */
\t\tsavetty = *tty;
\t}
\tsavefd = fd;
\treturn(0);
}

#if SHOPT_ESH || SHOPT_VSH
/*{\tTTY_COOKED( fd )
 *
 *\tThis routine will set the tty in cooked mode.
 *\tIt is also called by error.done().
 *
}*/

void tty_cooked(register int fd)
{

\tif(editb.e_raw==0)
\t\treturn;
\tif(fd < 0)
\t\tfd = savefd;
#ifdef L_MASK
\t/* restore flags */
\tif(l_changed&L_MASK)
\t\tioctl(fd,TIOCLSET,&l_mask);
\tif(l_changed&T_CHARS)
\t\t/* restore alternate break character */
\t\tioctl(fd,TIOCSETC,&l_ttychars);
\tif(l_changed&L_CHARS)
\t\t/* restore alternate break character */
\t\tioctl(fd,TIOCSLTC,&l_chars);
\tl_changed = 0;
#endif\t/* L_MASK */
\t/*** don\'t do tty_set unless ttyparm has valid data ***/
\tif(savefd<0 || tty_set(fd, TCSANOW, &ttyparm) == SYSERR)
\t\treturn;
\teditb.e_raw = 0;
\treturn;
}

/*{\tTTY_RAW( fd )
 *
 *\tThis routine will set the tty in raw mode.
 *
}*/

tty_raw(register int fd)
{
#ifdef L_MASK
\tstruct ltchars lchars;
#endif\t/* L_MASK */
\tif(editb.e_raw==RAWMODE)
\t\treturn(GOOD);
#ifndef SHOPT_RAWONLY
\tif(editb.e_raw != ALTMODE)
#endif /* SHOPT_RAWONLY */
\t{
\t\tif(tty_get(fd,&ttyparm) == SYSERR)
\t\t\treturn(BAD);
\t}
#if  L_MASK || VENIX
\tif(!(ttyparm.sg_flags&ECHO) || (ttyparm.sg_flags&LCASE))
\t\treturn(BAD);
\tnttyparm = ttyparm;
\tnttyparm.sg_flags &= ~(ECHO | TBDELAY);
#   ifdef CBREAK
\tnttyparm.sg_flags |= CBREAK;
#   else
\tnttyparm.sg_flags |= RAW;
#   endif /* CBREAK */
\teditb.e_erase = ttyparm.sg_erase;
\teditb.e_kill = ttyparm.sg_kill;
\teditb.e_eof = cntl(\'D\');
\tif( tty_set(fd, TCSADRAIN, &nttyparm) == SYSERR )
\t\treturn(BAD);
\teditb.e_ttyspeed = (ttyparm.sg_ospeed>=B1200?FAST:SLOW);
#   ifdef TIOCGLTC
\t/* try to remove effect of ^V  and ^Y and ^O */
\tif(ioctl(fd,TIOCGLTC,&l_chars) != SYSERR)
\t{
\t\tlchars = l_chars;
\t\tlchars.t_lnextc = -1;
\t\tlchars.t_flushc = -1;
\t\tlchars.t_dsuspc = -1;\t/* no delayed stop process signal */
\t\tif(ioctl(fd,TIOCSLTC,&lchars) != SYSERR)
\t\t\tl_changed |= L_CHARS;
\t}
#   endif\t/* TIOCGLTC */
#else

\tif (!(ttyparm.c_lflag & ECHO ))
\t\treturn(BAD);

#   ifdef FLUSHO
\tttyparm.c_lflag &= ~FLUSHO;
#   endif /* FLUSHO */
\tnttyparm = ttyparm;
#  ifndef u370
\tnttyparm.c_iflag &= ~(IGNPAR|PARMRK|INLCR|IGNCR|ICRNL);
\tnttyparm.c_iflag |= BRKINT;
#   else
\tnttyparm.c_iflag &= 
\t\t\t~(IGNBRK|PARMRK|INLCR|IGNCR|ICRNL|INPCK);
\tnttyparm.c_iflag |= (BRKINT|IGNPAR);
#   endif\t/* u370 */
\tnttyparm.c_lflag &= ~(ICANON|ECHO|ECHOK);
\tnttyparm.c_cc[VTIME] = 0;
\tnttyparm.c_cc[VMIN] = 1;
#   ifdef VDISCARD
\tnttyparm.c_cc[VDISCARD] = 0;
#   endif /* VDISCARD */
\teditb.e_eof = ttyparm.c_cc[VEOF];
\teditb.e_erase = ttyparm.c_cc[VERASE];
\teditb.e_kill = ttyparm.c_cc[VKILL];
\tif( tty_set(fd, TCSADRAIN, &nttyparm) == SYSERR )
\t\treturn(BAD);
\teditb.e_ttyspeed = (cfgetospeed(&ttyparm)>=B1200?FAST:SLOW);
#endif
\teditb.e_raw = RAWMODE;
\treturn(GOOD);
}

#ifndef SHOPT_RAWONLY

/*
 *
 *\tGet tty parameters and make ESC and \'\\r\' wakeup characters.
 *
 */

#   ifdef TIOCGETC
tty_alt(register int fd)
{
\tint mask;
\tstruct tchars ttychars;
\tif(editb.e_raw==ALTMODE)
\t\treturn(GOOD);
\tif(editb.e_raw==RAWMODE)
\t\ttty_cooked(fd);
\tl_changed = 0;
\tif( editb.e_ttyspeed == 0)
\t{
\t\tif((tty_get(fd,&ttyparm) != SYSERR))
\t\t\teditb.e_ttyspeed = (ttyparm.sg_ospeed>=B1200?FAST:SLOW);
\t\teditb.e_raw = ALTMODE;
\t}
\tif(ioctl(fd,TIOCGETC,&l_ttychars) == SYSERR)
\t\treturn(BAD);
\tif(ioctl(fd,TIOCLGET,&l_mask)==SYSERR)
\t\treturn(BAD);
\tttychars = l_ttychars;
\tmask =  LCRTBS|LCRTERA|LCTLECH|LPENDIN|LCRTKIL;
\tif((l_mask|mask) != l_mask)
\t\tl_changed = L_MASK;
\tif(ioctl(fd,TIOCLBIS,&mask)==SYSERR)
\t\treturn(BAD);
\tif(ttychars.t_brkc!=ESC)
\t{
\t\tttychars.t_brkc = ESC;
\t\tl_changed |= T_CHARS;
\t\tif(ioctl(fd,TIOCSETC,&ttychars) == SYSERR)
\t\t\treturn(BAD);
\t}
\treturn(GOOD);
}
#   else
#\tifndef PENDIN
#\t    define PENDIN\t0
#\tendif /* PENDIN */
#\tifndef IEXTEN
#\t    define IEXTEN\t0
#\tendif /* IEXTEN */

tty_alt(register int fd)
{
\tif(editb.e_raw==ALTMODE)
\t\treturn(GOOD);
\tif(editb.e_raw==RAWMODE)
\t\ttty_cooked(fd);
\tif((tty_get(fd, &ttyparm)==SYSERR) || (!(ttyparm.c_lflag&ECHO)))
\t\treturn(BAD);
#\tifdef FLUSHO
\t    ttyparm.c_lflag &= ~FLUSHO;
#\tendif /* FLUSHO */
\tnttyparm = ttyparm;
\teditb.e_eof = ttyparm.c_cc[VEOF];
#\tifdef ECHOCTL
\t    /* escape character echos as ^[ */
\t    nttyparm.c_lflag |= (ECHOE|ECHOK|ECHOCTL|PENDIN|IEXTEN);
\t    nttyparm.c_cc[VEOL2] = ESC;
#\telse
\t    /* switch VEOL2 and EOF, since EOF isn\'t echo\'d by driver */
\t    nttyparm.c_iflag &= ~(IGNCR|ICRNL);
\t    nttyparm.c_iflag |= INLCR;
\t    nttyparm.c_lflag |= (ECHOE|ECHOK);
\t    nttyparm.c_cc[VEOF] = ESC;\t/* make ESC the eof char */
\t    nttyparm.c_cc[VEOL] = \'\\r\';\t/* make CR an eol char */
\t    nttyparm.c_cc[VEOL2] = editb.e_eof;\t/* make EOF an eol char */
#\tendif /* ECHOCTL */
#\tifdef VWERASE
\t    nttyparm.c_cc[VWERASE] = cntl(\'W\');
#\tendif /* VWERASE */
#\tifdef VLNEXT
\t    nttyparm.c_cc[VLNEXT] = cntl(\'V\');
#\tendif /* VLNEXT */
\teditb.e_erase = ttyparm.c_cc[VERASE];
\teditb.e_kill = ttyparm.c_cc[VKILL];
\tif( tty_set(fd, TCSADRAIN, &nttyparm) == SYSERR )
\t\treturn(BAD);
\teditb.e_ttyspeed = (cfgetospeed(&ttyparm)>=B1200?FAST:SLOW);
\teditb.e_raw = ALTMODE;
\treturn(GOOD);
}

#   endif /* TIOCGETC */
#endif\t/* SHOPT_RAWONLY */

/*
 *\tE_WINDOW()
 *
 *\treturn the window size
 */

int ed_window(void)
{
\tint\trows;
\tint\tcols = DFLTWINDOW-1;
\tregister char *cp = nv_strval(COLUMNS);
\tif(cp)
\t{
\t\tcols = atoi(cp)-1;
\t\tif(cols > MAXWINDOW)
\t\t\tcols = MAXWINDOW;
\t}
\twinsize(&rows,&cols);
\tif(cols < MINWINDOW)
\t\tcols = MINWINDOW;
\treturn(cols);
}

/*\tE_FLUSH()
 *
 *\tFlush the output buffer.
 *
 */

void ed_flush(void)
{
\tregister int n = editb.e_outptr-editb.e_outbase;
\tregister int fd = ERRIO;
\tif(n<=0)
\t\treturn;
\twrite(fd,editb.e_outbase,(unsigned)n);
\teditb.e_outptr = editb.e_outbase;
}

/*
 * send the bell character ^G to the terminal
 */

void ed_ringbell(void)
{
\twrite(ERRIO,bellchr,1);
}

/*
 * send a carriage return line feed to the terminal
 */

void ed_crlf(void)
{
#ifdef cray
\ted_putchar(\'\\r\');
#endif /* cray */
#ifdef u370
\ted_putchar(\'\\r\');
#endif\t/* u370 */
#ifdef VENIX
\ted_putchar(\'\\r\');
#endif /* VENIX */
\ted_putchar(\'\\n\');
\ted_flush();
}
 
/*\tED_SETUP( max_prompt_size )
 *
 *\tThis routine sets up the prompt string
 *\tThe following is an unadvertised feature.
 *\t  Escape sequences in the prompt can be excluded from the calculated
 *\t  prompt length.  This is accomplished as follows:
 *\t  - if the prompt string starts with "%\\r, or contains \\r%\\r", where %
 *\t    represents any char, then % is taken to be the quote character.
 *\t  - strings enclosed by this quote character, and the quote character,
 *\t    are not counted as part of the prompt length.
 */

void\ted_setup(int fd)
{
\tregister char *pp;
\tregister char *last;
\tchar *ppmax;
\tint myquote = 0;
\tint qlen = 1;
\tchar inquote = 0;
\teditb.e_fd = fd;
#ifdef KSHELL
\tif(!(last = sh.prompt))
\t\tlast = "";
\tsh.prompt = 0;
#else
\tlast = editb.e_prbuff;
#endif /* KSHELL */
\tif(sh.hist_ptr)
\t{
\t\tregister History_t *hp = sh.hist_ptr;
\t\teditb.e_hismax = hist_max(hp);
\t\teditb.e_hismin = hist_min(hp);
\t\teditb.e_hloff = 0;
\t}
\telse
\t{
\t\teditb.e_hismax = editb.e_hismin = editb.e_hloff = 0;
\t}
\teditb.e_hline = editb.e_hismax;
\teditb.e_wsize = ed_window()-2;
\teditb.e_crlf = YES;
\tpp = editb.e_prompt;
\tppmax = pp+PRSIZE-1;
\t*pp++ = \'\\r\';
\t{
\t\tregister int c;
\t\twhile(c= *last++) switch(c)
\t\t{
\t\t\tcase \'\\r\':
\t\t\t\tif(pp == (editb.e_prompt+2)) /* quote char */
\t\t\t\t\tmyquote = *(pp-1);
\t\t\t\t/*FALLTHROUGH*/

\t\t\tcase \'\\n\':
\t\t\t\t/* start again */
\t\t\t\teditb.e_crlf = YES;
\t\t\t\tqlen = 1;
\t\t\t\tinquote = 0;
\t\t\t\tpp = editb.e_prompt+1;
\t\t\t\tbreak;

\t\t\tcase \'\\t\':
\t\t\t\t/* expand tabs */
\t\t\t\twhile((pp-editb.e_prompt)%TABSIZE)
\t\t\t\t{
\t\t\t\t\tif(pp >= ppmax)
\t\t\t\t\t\tbreak;
\t\t\t\t\t*pp++ = \' \';
\t\t\t\t}
\t\t\t\tbreak;

\t\t\tcase BELL:
\t\t\t\t/* cut out bells */
\t\t\t\tbreak;

\t\t\tdefault:
\t\t\t\tif(c==myquote)
\t\t\t\t{
\t\t\t\t\tqlen += inquote;
\t\t\t\t\tinquote ^= 1;
\t\t\t\t}
\t\t\t\tif(pp < ppmax)
\t\t\t\t{
\t\t\t\t\tqlen += inquote;
\t\t\t\t\t*pp++ = c;
\t\t\t\t\tif(!inquote && !isprint(c))
\t\t\t\t\t\teditb.e_crlf = NO;
\t\t\t\t}
\t\t}
\t}
\teditb.e_plen = pp - editb.e_prompt - qlen;
\t*pp = 0;
\tif((editb.e_wsize -= editb.e_plen) < 7)
\t{
\t\tregister int shift = 7-editb.e_wsize;
\t\teditb.e_wsize = 7;
\t\tpp = editb.e_prompt+1;
\t\tstrcpy(pp,pp+shift);
\t\teditb.e_plen -= shift;
\t\tlast[-editb.e_plen-2] = \'\\r\';
\t}
\tsfsync(sfstderr);
\tqlen = sfpeek(sfstderr,&editb.e_outptr,SF_UNBOUND);
\tsfwrite(sfstderr,editb.e_outptr,0);
\teditb.e_outbase = editb.e_outptr;
\teditb.e_outlast = editb.e_outptr + qlen;
}

#ifdef KSHELL
/*
 * look for edit macro named _i
 * if found, puts the macro definition into lookahead buffer and returns 1
 */

ed_macro(register int i)
{
}
#endif /*KSHELL*/'
		OUTPUT - $'                  
/*
 *  edit.c - common routines for vi and emacs one line editors in shell
 *
 *   David Korn\t\t\t\tP.D. Sullivan
 *   AT&T Bell Laboratories\t\tAT&T Bell Laboratories
 *   Tel. x7975\t\t\t\tTel. x 2655
 *
 *   Coded April 1983.
 */

#include\t<errno.h>
#include\t<ast.h>
#include\t<sfio.h>
#include\t<ctype.h>
#include\t"FEATURE/options"
#include\t"lexstates.h"
#include\t"path.h"
#include\t"FEATURE/sigrestart"
#include\t"FEATURE/select"
#include\t"FEATURE/time"

#define MAXTRY\t12
#ifdef KSHELL
#   include\t"defs.h"
#   include\t"fault.h"
#   include\t"io.h"
#   include\t"terminal.h"
#   include\t"builtins.h"
#else
#   include\t"io.h"
#   include\t"terminal.h"
#   undef SIG_NORESTART
#   define SIG_NORESTART\t1
#   define _sobuf\ted_errbuf
    extern __MANGLE__ char ed_errbuf[];
    char e_version[] = "
@(#)Editlib version 01/31/92-alpha1\\0
";
#endif\t/* KSHELL */
#include\t"history.h"
#include\t"edit.h"

#define BAD\t-1
#define GOOD\t0
#define\tTRUE\t(-1)
#define\tFALSE\t0
#define\tSYSERR\t-1

#ifdef SHOPT_OLDTERMIO
#   undef tcgetattr
#   undef tcsetattr
#endif /* SHOPT_OLDTERMIO */

#ifdef RT
#   define VENIX 1
#endif\t/* RT */

#define lookahead\teditb.e_index
#define env\t\teditb.e_env
#define previous\teditb.e_lbuf
#define fildes\t\teditb.e_fd


#ifdef _hdr_sgtty
#   ifdef TIOCGETP
\tstatic int l_mask;
\tstatic struct tchars l_ttychars;
\tstatic struct ltchars l_chars;
\tstatic  char  l_changed;\t/* set if mode bits changed */
#\tdefine L_CHARS\t4
#\tdefine T_CHARS\t2
#\tdefine L_MASK\t1
#   endif /* TIOCGETP */
#endif /* _hdr_sgtty */

#ifdef KSHELL
     static char macro[]\t= "_??";
#    define slowsig()\t(sh.trapnote&SH_SIGSLOW)
     static int keytrap __PROTO__((char*, int, int, int));
#else
     struct edit editb;
     extern __MANGLE__ int errno;
#    define slowsig()\t(0)
#endif\t/* KSHELL */


static struct termios savetty;
static int savefd = -1;
#ifdef future
    static int compare __PROTO__((const char*, const char*, int));
#endif  /* future */
#if SHOPT_VSH || SHOPT_ESH
    static struct termios ttyparm;\t/* initial tty parameters */
    static struct termios nttyparm;\t/* raw tty parameters */
    static char bellchr[] = "\\7";\t/* bell char */
    static char *overlay __PROTO__((char*,const char*));
#endif /* SHOPT_VSH || SHOPT_ESH */


/*
 * This routine returns true if fd refers to a terminal
 * This should be equivalent to isatty
 */

int tty_check __PARAM__((int fd), (fd)) __OTORP__(int fd;)
#line 107
{
\tsavefd = -1;
\treturn(tty_get(fd,(struct termios*)0)==0);
}

/*
 * Get the current terminal attributes
 * This routine remembers the attributes and just returns them if it
 *   is called again without an intervening tty_set()
 */

int tty_get __PARAM__((int fd, struct termios *tty), (fd, tty)) __OTORP__(int fd; struct termios *tty;)
#line 119
{
\tif(fd != savefd)
\t{
#ifndef SIG_NORESTART
\t\tvoid (*savint)(__VARARG__)= sh.intfn;
\t\tsh.intfn = 0;
#endif\t/* SIG_NORESTART */
\t\twhile(tcgetattr(fd,&savetty) == SYSERR)
\t\t{
\t\t\tif(errno !=EINTR)
\t\t\t{
#ifndef SIG_NORESTART
\t\t\t\tsh.intfn = savint;
#endif\t/* SIG_NORESTART */
\t\t\t\treturn(SYSERR);
\t\t\t}
\t\t\terrno = 0;
\t\t}
#ifndef SIG_NORESTART
\t\tsh.intfn = savint;
#endif\t/* SIG_NORESTART */
\t\tsavefd = fd;
\t}
\tif(tty)
\t\t*tty = savetty;
\treturn(0);
}

/*
 * Set the terminal attributes
 * If fd<0, then current attributes are invalidated
 */

int tty_set __PARAM__((int fd, int action, struct termios *tty), (fd, action, tty)) __OTORP__(int fd; int action; struct termios *tty;)
#line 153
{
\tif(fd >=0)
\t{
#ifndef SIG_NORESTART
\t\tvoid (*savint)(__VARARG__)= sh.intfn;
#endif\t/* SIG_NORESTART */
#ifdef future
\t\tif(savefd>=0 && compare(&savetty,tty,sizeof(struct termios)))
\t\t\treturn(0);
#endif
#ifndef SIG_NORESTART
\t\tsh.intfn = 0;
#endif\t/* SIG_NORESTART */
\t\twhile(tcsetattr(fd, action, tty) == SYSERR)
\t\t{
\t\t\tif(errno !=EINTR)
\t\t\t{
#ifndef SIG_NORESTART
\t\t\t\tsh.intfn = savint;
#endif\t/* SIG_NORESTART */
\t\t\t\treturn(SYSERR);
\t\t\t}
\t\t\terrno = 0;
\t\t}
#ifndef SIG_NORESTART
\t\tsh.intfn = savint;
#endif\t/* SIG_NORESTART */
\t\tsavetty = *tty;
\t}
\tsavefd = fd;
\treturn(0);
}

#if SHOPT_ESH || SHOPT_VSH
/*{\tTTY_COOKED( fd )
 *
 *\tThis routine will set the tty in cooked mode.
 *\tIt is also called by error.done().
 *
}*/

void tty_cooked __PARAM__((register int fd), (fd)) __OTORP__(register int fd;)
#line 195
{

\tif(editb.e_raw==0)
\t\treturn;
\tif(fd < 0)
\t\tfd = savefd;
#ifdef L_MASK
\t/* restore flags */
\tif(l_changed&L_MASK)
\t\tioctl(fd,TIOCLSET,&l_mask);
\tif(l_changed&T_CHARS)
\t\t/* restore alternate break character */
\t\tioctl(fd,TIOCSETC,&l_ttychars);
\tif(l_changed&L_CHARS)
\t\t/* restore alternate break character */
\t\tioctl(fd,TIOCSLTC,&l_chars);
\tl_changed = 0;
#endif\t/* L_MASK */
\t/*** don\'t do tty_set unless ttyparm has valid data ***/
\tif(savefd<0 || tty_set(fd, TCSANOW, &ttyparm) == SYSERR)
\t\treturn;
\teditb.e_raw = 0;
\treturn;
}

/*{\tTTY_RAW( fd )
 *
 *\tThis routine will set the tty in raw mode.
 *
}*/

tty_raw __PARAM__((register int fd), (fd)) __OTORP__(register int fd;)
#line 227
{
#ifdef L_MASK
\tstruct ltchars lchars;
#endif\t/* L_MASK */
\tif(editb.e_raw==RAWMODE)
\t\treturn(GOOD);
#ifndef SHOPT_RAWONLY
\tif(editb.e_raw != ALTMODE)
#endif /* SHOPT_RAWONLY */
\t{
\t\tif(tty_get(fd,&ttyparm) == SYSERR)
\t\t\treturn(BAD);
\t}
#if  L_MASK || VENIX
\tif(!(ttyparm.sg_flags&ECHO) || (ttyparm.sg_flags&LCASE))
\t\treturn(BAD);
\tnttyparm = ttyparm;
\tnttyparm.sg_flags &= ~(ECHO | TBDELAY);
#   ifdef CBREAK
\tnttyparm.sg_flags |= CBREAK;
#   else
\tnttyparm.sg_flags |= RAW;
#   endif /* CBREAK */
\teditb.e_erase = ttyparm.sg_erase;
\teditb.e_kill = ttyparm.sg_kill;
\teditb.e_eof = cntl(\'D\');
\tif( tty_set(fd, TCSADRAIN, &nttyparm) == SYSERR )
\t\treturn(BAD);
\teditb.e_ttyspeed = (ttyparm.sg_ospeed>=B1200?FAST:SLOW);
#   ifdef TIOCGLTC
\t/* try to remove effect of ^V  and ^Y and ^O */
\tif(ioctl(fd,TIOCGLTC,&l_chars) != SYSERR)
\t{
\t\tlchars = l_chars;
\t\tlchars.t_lnextc = -1;
\t\tlchars.t_flushc = -1;
\t\tlchars.t_dsuspc = -1;\t/* no delayed stop process signal */
\t\tif(ioctl(fd,TIOCSLTC,&lchars) != SYSERR)
\t\t\tl_changed |= L_CHARS;
\t}
#   endif\t/* TIOCGLTC */
#else

\tif (!(ttyparm.c_lflag & ECHO ))
\t\treturn(BAD);

#   ifdef FLUSHO
\tttyparm.c_lflag &= ~FLUSHO;
#   endif /* FLUSHO */
\tnttyparm = ttyparm;
#  ifndef u370
\tnttyparm.c_iflag &= ~(IGNPAR|PARMRK|INLCR|IGNCR|ICRNL);
\tnttyparm.c_iflag |= BRKINT;
#   else
\tnttyparm.c_iflag &= 
\t\t\t~(IGNBRK|PARMRK|INLCR|IGNCR|ICRNL|INPCK);
\tnttyparm.c_iflag |= (BRKINT|IGNPAR);
#   endif\t/* u370 */
\tnttyparm.c_lflag &= ~(ICANON|ECHO|ECHOK);
\tnttyparm.c_cc[VTIME] = 0;
\tnttyparm.c_cc[VMIN] = 1;
#   ifdef VDISCARD
\tnttyparm.c_cc[VDISCARD] = 0;
#   endif /* VDISCARD */
\teditb.e_eof = ttyparm.c_cc[VEOF];
\teditb.e_erase = ttyparm.c_cc[VERASE];
\teditb.e_kill = ttyparm.c_cc[VKILL];
\tif( tty_set(fd, TCSADRAIN, &nttyparm) == SYSERR )
\t\treturn(BAD);
\teditb.e_ttyspeed = (cfgetospeed(&ttyparm)>=B1200?FAST:SLOW);
#endif
\teditb.e_raw = RAWMODE;
\treturn(GOOD);
}

#ifndef SHOPT_RAWONLY

/*
 *
 *\tGet tty parameters and make ESC and \'\\r\' wakeup characters.
 *
 */

#   ifdef TIOCGETC
tty_alt __PARAM__((register int fd), (fd)) __OTORP__(register int fd;)
#line 312
{
\tint mask;
\tstruct tchars ttychars;
\tif(editb.e_raw==ALTMODE)
\t\treturn(GOOD);
\tif(editb.e_raw==RAWMODE)
\t\ttty_cooked(fd);
\tl_changed = 0;
\tif( editb.e_ttyspeed == 0)
\t{
\t\tif((tty_get(fd,&ttyparm) != SYSERR))
\t\t\teditb.e_ttyspeed = (ttyparm.sg_ospeed>=B1200?FAST:SLOW);
\t\teditb.e_raw = ALTMODE;
\t}
\tif(ioctl(fd,TIOCGETC,&l_ttychars) == SYSERR)
\t\treturn(BAD);
\tif(ioctl(fd,TIOCLGET,&l_mask)==SYSERR)
\t\treturn(BAD);
\tttychars = l_ttychars;
\tmask =  LCRTBS|LCRTERA|LCTLECH|LPENDIN|LCRTKIL;
\tif((l_mask|mask) != l_mask)
\t\tl_changed = L_MASK;
\tif(ioctl(fd,TIOCLBIS,&mask)==SYSERR)
\t\treturn(BAD);
\tif(ttychars.t_brkc!=ESC)
\t{
\t\tttychars.t_brkc = ESC;
\t\tl_changed |= T_CHARS;
\t\tif(ioctl(fd,TIOCSETC,&ttychars) == SYSERR)
\t\t\treturn(BAD);
\t}
\treturn(GOOD);
}
#   else
#\tifndef PENDIN
#\t    define PENDIN\t0
#\tendif /* PENDIN */
#\tifndef IEXTEN
#\t    define IEXTEN\t0
#\tendif /* IEXTEN */

tty_alt __PARAM__((register int fd), (fd)) __OTORP__(register int fd;)
#line 354
{
\tif(editb.e_raw==ALTMODE)
\t\treturn(GOOD);
\tif(editb.e_raw==RAWMODE)
\t\ttty_cooked(fd);
\tif((tty_get(fd, &ttyparm)==SYSERR) || (!(ttyparm.c_lflag&ECHO)))
\t\treturn(BAD);
#\tifdef FLUSHO
\t    ttyparm.c_lflag &= ~FLUSHO;
#\tendif /* FLUSHO */
\tnttyparm = ttyparm;
\teditb.e_eof = ttyparm.c_cc[VEOF];
#\tifdef ECHOCTL
\t    /* escape character echos as ^[ */
\t    nttyparm.c_lflag |= (ECHOE|ECHOK|ECHOCTL|PENDIN|IEXTEN);
\t    nttyparm.c_cc[VEOL2] = ESC;
#\telse
\t    /* switch VEOL2 and EOF, since EOF isn\'t echo\'d by driver */
\t    nttyparm.c_iflag &= ~(IGNCR|ICRNL);
\t    nttyparm.c_iflag |= INLCR;
\t    nttyparm.c_lflag |= (ECHOE|ECHOK);
\t    nttyparm.c_cc[VEOF] = ESC;\t/* make ESC the eof char */
\t    nttyparm.c_cc[VEOL] = \'\\r\';\t/* make CR an eol char */
\t    nttyparm.c_cc[VEOL2] = editb.e_eof;\t/* make EOF an eol char */
#\tendif /* ECHOCTL */
#\tifdef VWERASE
\t    nttyparm.c_cc[VWERASE] = cntl(\'W\');
#\tendif /* VWERASE */
#\tifdef VLNEXT
\t    nttyparm.c_cc[VLNEXT] = cntl(\'V\');
#\tendif /* VLNEXT */
\teditb.e_erase = ttyparm.c_cc[VERASE];
\teditb.e_kill = ttyparm.c_cc[VKILL];
\tif( tty_set(fd, TCSADRAIN, &nttyparm) == SYSERR )
\t\treturn(BAD);
\teditb.e_ttyspeed = (cfgetospeed(&ttyparm)>=B1200?FAST:SLOW);
\teditb.e_raw = ALTMODE;
\treturn(GOOD);
}

#   endif /* TIOCGETC */
#endif\t/* SHOPT_RAWONLY */

/*
 *\tE_WINDOW()
 *
 *\treturn the window size
 */

int ed_window __PARAM__((void), ())
#line 404
{
\tint\trows;
\tint\tcols = DFLTWINDOW-1;
\tregister char *cp = nv_strval(COLUMNS);
\tif(cp)
\t{
\t\tcols = atoi(cp)-1;
\t\tif(cols > MAXWINDOW)
\t\t\tcols = MAXWINDOW;
\t}
\twinsize(&rows,&cols);
\tif(cols < MINWINDOW)
\t\tcols = MINWINDOW;
\treturn(cols);
}

/*\tE_FLUSH()
 *
 *\tFlush the output buffer.
 *
 */

void ed_flush __PARAM__((void), ())
#line 427
{
\tregister int n = editb.e_outptr-editb.e_outbase;
\tregister int fd = ERRIO;
\tif(n<=0)
\t\treturn;
\twrite(fd,editb.e_outbase,(unsigned)n);
\teditb.e_outptr = editb.e_outbase;
}

/*
 * send the bell character ^G to the terminal
 */

void ed_ringbell __PARAM__((void), ())
#line 441
{
\twrite(ERRIO,bellchr,1);
}

/*
 * send a carriage return line feed to the terminal
 */

void ed_crlf __PARAM__((void), ())
#line 450
{
#ifdef cray
\ted_putchar(\'\\r\');
#endif /* cray */
#ifdef u370
\ted_putchar(\'\\r\');
#endif\t/* u370 */
#ifdef VENIX
\ted_putchar(\'\\r\');
#endif /* VENIX */
\ted_putchar(\'\\n\');
\ted_flush();
}
 
/*\tED_SETUP( max_prompt_size )
 *
 *\tThis routine sets up the prompt string
 *\tThe following is an unadvertised feature.
 *\t  Escape sequences in the prompt can be excluded from the calculated
 *\t  prompt length.  This is accomplished as follows:
 *\t  - if the prompt string starts with "%\\r, or contains \\r%\\r", where %
 *\t    represents any char, then % is taken to be the quote character.
 *\t  - strings enclosed by this quote character, and the quote character,
 *\t    are not counted as part of the prompt length.
 */

void\ted_setup __PARAM__((int fd), (fd)) __OTORP__(int fd;)
#line 477
{
\tregister char *pp;
\tregister char *last;
\tchar *ppmax;
\tint myquote = 0;
\tint qlen = 1;
\tchar inquote = 0;
\teditb.e_fd = fd;
#ifdef KSHELL
\tif(!(last = sh.prompt))
\t\tlast = "";
\tsh.prompt = 0;
#else
\tlast = editb.e_prbuff;
#endif /* KSHELL */
\tif(sh.hist_ptr)
\t{
\t\tregister History_t *hp = sh.hist_ptr;
\t\teditb.e_hismax = hist_max(hp);
\t\teditb.e_hismin = hist_min(hp);
\t\teditb.e_hloff = 0;
\t}
\telse
\t{
\t\teditb.e_hismax = editb.e_hismin = editb.e_hloff = 0;
\t}
\teditb.e_hline = editb.e_hismax;
\teditb.e_wsize = ed_window()-2;
\teditb.e_crlf = YES;
\tpp = editb.e_prompt;
\tppmax = pp+PRSIZE-1;
\t*pp++ = \'\\r\';
\t{
\t\tregister int c;
\t\twhile(c= *last++) switch(c)
\t\t{
\t\t\tcase \'\\r\':
\t\t\t\tif(pp == (editb.e_prompt+2)) /* quote char */
\t\t\t\t\tmyquote = *(pp-1);
\t\t\t\t/*FALLTHROUGH*/

\t\t\tcase \'\\n\':
\t\t\t\t/* start again */
\t\t\t\teditb.e_crlf = YES;
\t\t\t\tqlen = 1;
\t\t\t\tinquote = 0;
\t\t\t\tpp = editb.e_prompt+1;
\t\t\t\tbreak;

\t\t\tcase \'\\t\':
\t\t\t\t/* expand tabs */
\t\t\t\twhile((pp-editb.e_prompt)%TABSIZE)
\t\t\t\t{
\t\t\t\t\tif(pp >= ppmax)
\t\t\t\t\t\tbreak;
\t\t\t\t\t*pp++ = \' \';
\t\t\t\t}
\t\t\t\tbreak;

\t\t\tcase BELL:
\t\t\t\t/* cut out bells */
\t\t\t\tbreak;

\t\t\tdefault:
\t\t\t\tif(c==myquote)
\t\t\t\t{
\t\t\t\t\tqlen += inquote;
\t\t\t\t\tinquote ^= 1;
\t\t\t\t}
\t\t\t\tif(pp < ppmax)
\t\t\t\t{
\t\t\t\t\tqlen += inquote;
\t\t\t\t\t*pp++ = c;
\t\t\t\t\tif(!inquote && !isprint(c))
\t\t\t\t\t\teditb.e_crlf = NO;
\t\t\t\t}
\t\t}
\t}
\teditb.e_plen = pp - editb.e_prompt - qlen;
\t*pp = 0;
\tif((editb.e_wsize -= editb.e_plen) < 7)
\t{
\t\tregister int shift = 7-editb.e_wsize;
\t\teditb.e_wsize = 7;
\t\tpp = editb.e_prompt+1;
\t\tstrcpy(pp,pp+shift);
\t\teditb.e_plen -= shift;
\t\tlast[-editb.e_plen-2] = \'\\r\';
\t}
\tsfsync(sfstderr);
\tqlen = sfpeek(sfstderr,&editb.e_outptr,SF_UNBOUND);
\tsfwrite(sfstderr,editb.e_outptr,0);
\teditb.e_outbase = editb.e_outptr;
\teditb.e_outlast = editb.e_outptr + qlen;
}

#ifdef KSHELL
/*
 * look for edit macro named _i
 * if found, puts the macro definition into lookahead buffer and returns 1
 */

ed_macro __PARAM__((register int i), (i)) __OTORP__(register int i;)
#line 580
{
}
#endif /*KSHELL*/'
	EXEC -nh
		INPUT - $'#pragma prototyped

typedef int (*signal_t)(int);

struct test
{
\tint\t(*call)(int, ...);
};

int main(int argc, char** argv)
{
\texit(0);
}'
		OUTPUT - $'                  

typedef int (*signal_t) __PROTO__((int));

struct test
{
\tint\t(*call) __PROTO__((int, ...));
};

int main __PARAM__((int argc, char** argv), (argc, argv)) __OTORP__(int argc; char** argv;)
#line 11
{
\texit(0);
}'
	EXEC -nh
		INPUT - $'#pragma prototyped

extern int\terrcount;\t\t/* level>=ERROR_ERROR count\t*/
extern int\terrno;\t\t\t/* system call error status\t*/

extern void\terror(int, ...);
extern void\tliberror(char*, int, ...);
extern void*\tseterror(int, ...);
extern void\tverror(char*, int, va_list);

static int\tftwalk(char*, int (*)(struct FTW*), int, int (*)(struct FTW*, struct FTW*));

static int
ftwalk(char* file, int (*userf)(struct FTW*), int flags, int (*comparf)(struct FTW*, struct FTW*))
{
\tfunction(0);
\treturn(0);
}

int\t(*fun)(char*, int);

struct xxx
{
int\t(*fun)(char*, int);
};

void
test()
{
\tint (*fun)(int, int);
\treturn (*fun)(a, b);
}

void
test(int i)
{
\tint (*fun)(int, int);
\tint (*fun)(int, int);
\treturn (*fun)(a, b);
}

void
xxx()
{
\tint\t(*fun)(char*, int);

\treturn((*fun)("", 0));
}'
		OUTPUT - $'                  

extern __MANGLE__ int\terrcount;\t\t/* level>=ERROR_ERROR count\t*/
extern __MANGLE__ int\terrno;\t\t\t/* system call error status\t*/

extern __MANGLE__ void\terror __PROTO__((int, ...));
extern __MANGLE__ void\tliberror __PROTO__((char*, int, ...));
extern __MANGLE__ __V_*\tseterror __PROTO__((int, ...));
extern __MANGLE__ void\tverror __PROTO__((char*, int, va_list));

static int\tftwalk __PROTO__((char*, int (*)(struct FTW*), int, int (*)(struct FTW*, struct FTW*)));

static int
ftwalk __PARAM__((char* file, int (*userf)(struct FTW*), int flags, int (*comparf)(struct FTW*, struct FTW*)), (file, userf, flags, comparf)) __OTORP__(char* file; int (*userf)(); int flags; int (*comparf)();)
#line 15
{
\tfunction(0);
\treturn(0);
}

int\t(*fun) __PROTO__((char*, int));

struct xxx
{
int\t(*fun) __PROTO__((char*, int));
};

void
test()
{
\tint (*fun) __PROTO__((int, int));
\treturn (*fun)(a, b);
}

void
test __PARAM__((int i), (i)) __OTORP__(int i;)
#line 36
{
\tint (*fun) __PROTO__((int, int));
\tint (*fun) __PROTO__((int, int));
\treturn (*fun)(a, b);
}

void
xxx()
{
\tint\t(*fun) __PROTO__((char*, int));

\treturn((*fun)("", 0));
}'
	EXEC -nh
		INPUT - $'#pragma prototyped
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Rewritten by David Korn
 * AT&T Bell Laboratories
 *
 */

#include\t"defs.h"
#include\t"jobs.h"
#include\t"history.h"
#include\t"shnodes.h"


/* These routines are used by this module but defined elsewhere */
extern void\tmac_check(void);
extern void\tname_unscope(void);
extern void\trm_files(char*);
#ifdef VFORK
    extern void\tvfork_restore(void);
#endif\t/* VFORK */

/* ========\terror handling\t======== */

\t/* Find out if it is time to go away.
\t * `trapnote\' is set to SIGSET when fault is seen and
\t * no trap has been set.
\t */

void sh_cfail(MSG message)
{
\tsh_fail(sh.cmdname,message);
}

/*
 *  This routine is called when fatal errors are encountered
 *  A message is printed out and the shell tries to exit
 */

void sh_fail(register const char *s1,MSG s2)
{
\tmac_check();
\tp_prp(s1);
\tif(s2)
\t\tsfprintf(sfstderr,": %s
",s2);
\telse
\t\tsfputc(sfstderr,\'\\n\');
\tsh_exit(ERROR);
}

/* Arrive here from `FATAL\' errors
 *  a) exit command,
 *  b) default trap,
 *  c) fault with no trap set.
 *
 * Action is to return to command level or exit.
 */


void sh_exit(int xno)
{
\tregister unsigned state=(st.states&~(SH_ERREXIT|SH_MONITOR));
\tsh.exitval=xno;
\tif(xno==SIGFAIL)
\t\tsh.exitval |= sh.lastsig;
\tsh.un.com = 0;
\tif(state&(BUILTIN|LASTPIPE))
\t{
#if VSH || ESH
\t\ttty_cooked(-1);
#endif
\t\tio_clear((Sfile_t*)0);
\t\tLONGJMP(*sh.freturn,1);
\t}
\tstate |= (unsigned)sh_isoption(SH_ERREXIT|SH_MONITOR);
\tif( (state&(SH_ERREXIT|SH_FORKED)) || 
\t\t(!(state&(SH_PROFILE|SH_INTERACTIVE|FUNCTION)) && job_close() >= 0))
\t{
\t\tst.states = state;
\t\tsh_done(0);
\t}
\telse
\t{
\t\tif(!(state&FUNCTION))
\t\t{
\t\t\tsfsync(sfstderr);
\t\t\tname_unscope();
\t\t\targ_clear();
\t\t\tio_clear((Sfile_t*)0);
\t\t\tio_restore(0);
\t\t\tif(sh.input.file && sfpeek(sh.input.file,(unsigned char**)0)>0)
\t\t\t{
\t\t\t\tregister int n;
\t\t\t\tunsigned char *bp;
\t\t\t\t/* discard input buffer */
\t\t\t\tif((n = sfpeek(sh.input.file,&bp)) > 0)
\t\t\t\t\tsfread(sh.input.file,bp,n);
\t\t\t}
\t\t}
#ifdef VFORK
\t\tvfork_restore();
#endif\t/* VFORK */
\t\tst.execbrk = st.breakcnt = 0;
\t\tst.exec_flag = 0;
\t\tst.dot_depth = 0;
\t\thist_flush();
\t\tstate &= ~(FUNCTION|SH_HISTORY|SH_INTERACTIVE|SH_VERBOSE|
\t\t\tSH_MONITOR|BUILTIN|LASTPIPE|SH_VFORKED);
\t\tstate |= (unsigned)sh_isoption(SH_INTERACTIVE|SH_VERBOSE|SH_MONITOR);
\t\tjob.pipeflag = 0;
\t\tst.states = state;
\t\tLONGJMP(*sh.freturn,1);
\t}
}

#ifdef JOBS
    /* send signal to background process groups */
static int job_terminate(register struct process *pw,register int sig)
    {
\tif(pw->p_pgrp)
\t\tjob_kill(pw,sig);
\treturn(0);
    }
#endif /* JOBS */

/*
 * This is the exit routine for the shell
 */

void sh_done(register int sig)
{
\tregister char *t;
\tregister int savxit = sh.exitval;
\tif(sh.trapnote&SIGBEGIN)
\t\treturn;
\tsh.trapnote = 0;
\tif(t=st.trapcom[0])
\t{
\t\tst.trapcom[0]=0; /*should free but not long */
\t\tsh.oldexit = savxit;
\t\tsh.intrap++;
\t\tsh_eval(t);
\t\tsh.intrap--;
\t}
\telse
\t{
\t\t/* avoid recursive call for set -e */
\t\tst.states &= ~SH_ERREXIT;
\t\tsh_chktrap();
\t}
\tsh_freeup();
#ifdef ACCT
\tdoacct();
#endif\t/* ACCT */
#if VSH || ESH
\tif(sh_isoption(SH_EMACS|SH_VI|SH_GMACS))
\t\ttty_cooked(-1);
#endif
\tif(st.states&SH_RMTMP)
\t/* clean up all temp files */
\t\trm_files(io_tmpname);
#ifdef JOBS
\tif(sig==SIGHUP || (sh_isoption(SH_INTERACTIVE)&&sh.login_sh))
\t\tjob_walk(sfstderr,job_terminate,SIGHUP,(char**)0);
#endif\t/* JOBS */
\tjob_close();
\tsfsync((Sfile_t*)sfstdin);
\tsfsync((Sfile_t*)sfstderr);
\tif(sig)
\t{
\t\t/* generate fault termination code */
\t\tsignal(sig,SIG_DFL);
\t\tsigrelease(sig);
\t\tkill(getpid(),sig);
\t\tpause();
\t}
\t_exit(savxit&EXITMASK);
}
'
		OUTPUT - $'                  
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Rewritten by David Korn
 * AT&T Bell Laboratories
 *
 */

#include\t"defs.h"
#include\t"jobs.h"
#include\t"history.h"
#include\t"shnodes.h"


/* These routines are used by this module but defined elsewhere */
extern __MANGLE__ void\tmac_check __PROTO__((void));
extern __MANGLE__ void\tname_unscope __PROTO__((void));
extern __MANGLE__ void\trm_files __PROTO__((char*));
#ifdef VFORK
    extern __MANGLE__ void\tvfork_restore __PROTO__((void));
#endif\t/* VFORK */

/* ========\terror handling\t======== */

\t/* Find out if it is time to go away.
\t * `trapnote\' is set to SIGSET when fault is seen and
\t * no trap has been set.
\t */

void sh_cfail __PARAM__((MSG message), (message)) __OTORP__(MSG message;)
#line 33
{
\tsh_fail(sh.cmdname,message);
}

/*
 *  This routine is called when fatal errors are encountered
 *  A message is printed out and the shell tries to exit
 */

void sh_fail __PARAM__((register const char *s1,MSG s2), (s1, s2)) __OTORP__(register const char *s1;MSG s2;)
#line 43
{
\tmac_check();
\tp_prp(s1);
\tif(s2)
\t\tsfprintf(sfstderr,": %s
",s2);
\telse
\t\tsfputc(sfstderr,\'\\n\');
\tsh_exit(ERROR);
}

/* Arrive here from `FATAL\' errors
 *  a) exit command,
 *  b) default trap,
 *  c) fault with no trap set.
 *
 * Action is to return to command level or exit.
 */


void sh_exit __PARAM__((int xno), (xno)) __OTORP__(int xno;)
#line 64
{
\tregister unsigned state=(st.states&~(SH_ERREXIT|SH_MONITOR));
\tsh.exitval=xno;
\tif(xno==SIGFAIL)
\t\tsh.exitval |= sh.lastsig;
\tsh.un.com = 0;
\tif(state&(BUILTIN|LASTPIPE))
\t{
#if VSH || ESH
\t\ttty_cooked(-1);
#endif
\t\tio_clear((Sfile_t*)0);
\t\tLONGJMP(*sh.freturn,1);
\t}
\tstate |= (unsigned)sh_isoption(SH_ERREXIT|SH_MONITOR);
\tif( (state&(SH_ERREXIT|SH_FORKED)) || 
\t\t(!(state&(SH_PROFILE|SH_INTERACTIVE|FUNCTION)) && job_close() >= 0))
\t{
\t\tst.states = state;
\t\tsh_done(0);
\t}
\telse
\t{
\t\tif(!(state&FUNCTION))
\t\t{
\t\t\tsfsync(sfstderr);
\t\t\tname_unscope();
\t\t\targ_clear();
\t\t\tio_clear((Sfile_t*)0);
\t\t\tio_restore(0);
\t\t\tif(sh.input.file && sfpeek(sh.input.file,(unsigned char**)0)>0)
\t\t\t{
\t\t\t\tregister int n;
\t\t\t\tunsigned char *bp;
\t\t\t\t/* discard input buffer */
\t\t\t\tif((n = sfpeek(sh.input.file,&bp)) > 0)
\t\t\t\t\tsfread(sh.input.file,bp,n);
\t\t\t}
\t\t}
#ifdef VFORK
\t\tvfork_restore();
#endif\t/* VFORK */
\t\tst.execbrk = st.breakcnt = 0;
\t\tst.exec_flag = 0;
\t\tst.dot_depth = 0;
\t\thist_flush();
\t\tstate &= ~(FUNCTION|SH_HISTORY|SH_INTERACTIVE|SH_VERBOSE|
\t\t\tSH_MONITOR|BUILTIN|LASTPIPE|SH_VFORKED);
\t\tstate |= (unsigned)sh_isoption(SH_INTERACTIVE|SH_VERBOSE|SH_MONITOR);
\t\tjob.pipeflag = 0;
\t\tst.states = state;
\t\tLONGJMP(*sh.freturn,1);
\t}
}

#ifdef JOBS
    /* send signal to background process groups */
static int job_terminate __PARAM__((register struct process *pw,register int sig), (pw, sig)) __OTORP__(register struct process *pw;register int sig;)
#line 122
{
\tif(pw->p_pgrp)
\t\tjob_kill(pw,sig);
\treturn(0);
    }
#endif /* JOBS */

/*
 * This is the exit routine for the shell
 */

void sh_done __PARAM__((register int sig), (sig)) __OTORP__(register int sig;)
#line 134
{
\tregister char *t;
\tregister int savxit = sh.exitval;
\tif(sh.trapnote&SIGBEGIN)
\t\treturn;
\tsh.trapnote = 0;
\tif(t=st.trapcom[0])
\t{
\t\tst.trapcom[0]=0; /*should free but not long */
\t\tsh.oldexit = savxit;
\t\tsh.intrap++;
\t\tsh_eval(t);
\t\tsh.intrap--;
\t}
\telse
\t{
\t\t/* avoid recursive call for set -e */
\t\tst.states &= ~SH_ERREXIT;
\t\tsh_chktrap();
\t}
\tsh_freeup();
#ifdef ACCT
\tdoacct();
#endif\t/* ACCT */
#if VSH || ESH
\tif(sh_isoption(SH_EMACS|SH_VI|SH_GMACS))
\t\ttty_cooked(-1);
#endif
\tif(st.states&SH_RMTMP)
\t/* clean up all temp files */
\t\trm_files(io_tmpname);
#ifdef JOBS
\tif(sig==SIGHUP || (sh_isoption(SH_INTERACTIVE)&&sh.login_sh))
\t\tjob_walk(sfstderr,job_terminate,SIGHUP,(char**)0);
#endif\t/* JOBS */
\tjob_close();
\tsfsync((Sfile_t*)sfstdin);
\tsfsync((Sfile_t*)sfstderr);
\tif(sig)
\t{
\t\t/* generate fault termination code */
\t\tsignal(sig,SIG_DFL);
\t\tsigrelease(sig);
\t\tkill(getpid(),sig);
\t\tpause();
\t}
\t_exit(savxit&EXITMASK);
}
'
	EXEC -nh
		INPUT - $'#pragma prototyped

extern char* strcpy(const char*, const char*);

typedef void (*signal_t)(int);

struct test
{
\tint\t(*call)(int, ...);
};

static void (*signal)(void (*handler)(int))
{
\tint\t(*call)(int, ...);
\t(*call)(2, 2, 0);
\tcall(2,3,4);
}

#include <stdarg.h>

int error(int level, char* format, ...)
{
\tva_list\tap;

\tva_start(ap, format);
\tva_end(ap);
}'
		OUTPUT - $'                  

extern __MANGLE__ char* strcpy __PROTO__((const char*, const char*));

typedef void (*signal_t) __PROTO__((int));

struct test
{
\tint\t(*call) __PROTO__((int, ...));
};

static void (*signal) __PARAM__((void (*handler)(int)), (handler)) __OTORP__(void (*handler)();)
#line 13
{
\tint\t(*call) __PROTO__((int, ...));
\t(*call)(2, 2, 0);
\tcall(2,3,4);
}

#if !defined(va_start)
#if defined(__STDARG__)
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#endif

#line 19


int error __PARAM__((int level, char* format, ...), (va_alist)) __OTORP__(va_dcl)
{ __OTORP__(int level; char* format; )
#line 22

\tva_list\tap;

\t__VA_START__(ap, format); __OTORP__(level = va_arg(ap, int );format = va_arg(ap, char* );)
\tva_end(ap);
}'
	EXEC -nh
		INPUT - $'#pragma prototyped

f()
{
\tif(*cp==\'?\')
\t\t(cp++,flag++);
\tif(1)
\t\t(x);
\tif(*1)
\t\t(x);
\tif(2)
\t\t(y,z);
}'
		OUTPUT - $'                  

f()
{
\tif(*cp==\'?\')
\t\t(cp++,flag++);
\tif(1)
\t\t(x);
\tif(*1)
\t\t(x);
\tif(2)
\t\t(y,z);
}'
	EXEC -nh
		INPUT - $'#pragma prototyped

extern int a, char b;

static void (*signal)(void (*handler)(int))
{
\tint\t(*call)(int, ...);
\t(*call)(2, 2, 0);
\tcall(2,3,4);
\tcall(*p);
}'
		OUTPUT - $'                  

extern __MANGLE__ int a, char b;

static void (*signal) __PARAM__((void (*handler)(int)), (handler)) __OTORP__(void (*handler)();)
#line 6
{
\tint\t(*call) __PROTO__((int, ...));
\t(*call)(2, 2, 0);
\tcall(2,3,4);
\tcall(*p);
}'

TEST 06 'option exercize'
	EXEC -nhp
		INPUT - $'extern int test(const char*);'
		OUTPUT - $'extern int test(const char*);'
	EXEC -nhpf
		INPUT - $'#pragma noprototyped\nextern int test(const char*);'
		OUTPUT - $'                    \nextern int test(const char*);'
	EXEC -nhpf
		INPUT - $'#pragma prototyped\nextern int test(const char*);'
		OUTPUT - $'                  \nextern __MANGLE__ int test __PROTO__((const char*));'

TEST 07 'in place'
	EXEC -fhnr hello.c
		INPUT hello.c $'static int hello(const char* msg);
main(int argc, char** argv)
{
	return hello("hello world");
}
static int hello(const char* msg)
{
	printf("%s.\\n", msg);
	return 0;
}'
		OUTPUT hello.c $'static int hello __PROTO__((const char* msg));
main __PARAM__((int argc, char** argv), (argc, argv)) __OTORP__(int argc; char** argv;)
#line 3
{
	return hello("hello world");
}
static int hello __PARAM__((const char* msg), (msg)) __OTORP__(const char* msg;)
#line 7
{
	printf("%s.\\n", msg);
	return 0;
}'

TEST 08 'buffer boundaries'
	EXEC -nhf
		INPUT - $'/*
  xxxxxxxxxxx xxxxx xxxxxxx.
*/
#ifndef xxxxxxxx
#define xxxxxxxx

#if defined(xxxxxxxxxxx) || defined(xxxxxxxxxx)
extern "x" {
#endif

#if defined(xxxxxxxxxxx)
/*
  xxxxx xxxxxxx xx [0..65535].
*/
#define xxxxxxxxx(xxxxxxx)  (((xxxxxxxx long) (xxxxxxx)) >> 8)
#define xxxxxxxxxxxxxx "#%04x%04x%04x"
#define xxxxxx  65535x
#define xxxxxxxxxxxx  65535x
#define xxxxxxxxxxxx  16
#define xxxxxxx(xxxxxxx)  (((xxxxxxxx long) (xxxxxxx))*257)
#define xxxxxxxxxx(xxxxx)  ((xxxxxxxx long) (xxxxx))
#define xxxxxxxx(xxxxx)  ((xxxxxxxx long) (xxxxx))

xxxxxxx xxxxxxxx short xxxxxxx;
#else
/*
  xxxxx xxxxxxx xx [0..255].
*/
#define xxxxxxxxx(xxxxxxx)  ((xxxxxxxx long) (xxxxxxx))
#define xxxxxxxxxxxxxx "#%02x%02x%02x"
#define xxxxxx  255
#define xxxxxxxxxxxx  255
#define xxxxxxxxxxxx  8
#define xxxxxxx(xxxxxxx)  ((xxxxxxxx long) (xxxxxxx))
#define xxxxxxxxxx(xxxxx)  (((xxxxxxxx long) (xxxxx)) >> 8)
#define xxxxxxxx(xxxxx)  (((xxxxxxxx long) (xxxxx))*257)

xxxxxxx xxxxxxxx char xxxxxxx;
#endif

/*
  3x xxxxxxx.
*/
#define xxxxxxxxxxxxxxxxxx  xxxxxxx(80)
#define xxxxxxxxxxxxxxxxx  xxxxxxx(125)
#define xxxxxxxxxxxxxx  xxxxxxx(135)
#define xxxxxxxxxxxxx  xxxxxxx(185)
#define xxxxxxxxxxxxxx  xxxxxxx(110)

/*
  xxxxxxx xxxxxxxxxxxx.
*/
xxxxxxx struct xxxxxxxxx
{
  char
    *xxxx;

  long
    xxxxxx,
    xxxxxx,
    xxxxxx,
    xxxxxxx;
} xxxxxxxx;

xxxxxxx struct xxxxxxxxxxxxxx
{
  char
    *xxxx;

  xxxxxxxx char
    xxx,
    xxxxx,
    xxxx;
} xxxxxxxxxxxxx;

xxxxxxx struct xxxxxxxxxxxx
{
  xxxxxxxx short
    xxx,
    xxxxx,
    xxxx,
    xxxxx;

  xxxxxxxx char
    xxxxx;

  char
    xxx[3];

  xxxxxxxx long
    xxxxx;
} xxxxxxxxxxx;

xxxxxxx struct xxxxxxxxxxxxxxxxx
{
  int
    xxxxx;

  double
    xxxxxx;
} xxxxxxxxxxxxxxxx;

xxxxxxx struct xxxxxxxxxxx
{
  double
    (*xxxxxxxx)(double),
    xxxxxxx;
} xxxxxxxxxx;

xxxxxxx struct xxxxxxxxxx
{
  int
    x,
    x;

  xxxxxxxx int
    xxxxx,
    xxxxxx;

  int
    xxxxxxxxxxx,
    xxxxxxxxxxx;
} xxxxxxxxx;

xxxxxxx struct xxxxxxxxxx
{
  /*
    xxxx xxxxxx.
  */
  xxxxxxxx
    xxxx;
  /*
    xxxx xxx xxxxx xxxxxxxxx xxxxxxx.
  */
  xxxx
    *xxxx;

  char
    xxxxxxxx[xxxxxxxxxxxxx],
    xxxxxx[xxxxxxxxxxxxx],
    xxxxxx[xxxxxxxxxxxxx],
    xxxx[xxxxxxxxxxxxx];

  xxxxxxxx int
    xxxxxx,
    xxxxxxxxx,
    xxxxxx,
    xxxxxxxx,
    xxxxxxxx,
    xxxxx,
    xxxx;

  char
    *xxxx,
    *xxxx,
    *xxxx;

  xxxxxxxxxxxxx
    xxxxxxxxx;

  xxxxxxxxxxxxxx
    xxxxx;

  /*
    xxxxxxxxxxx xxxxxxx.
  */
  xxxxxxxxxxxxxxx
    xxxxxxxxxxx;

  xxxxxxxx int
    xxxxxxx;

  /*
    xxxxxxxxxx xxxxxxx.
  */
  char
    *xxxxxxxxxxx,
    *xxx,
    *xxxx,
    *xxx,
    *xxxxxxx,
    *xxxxxxx;

  xxxxxxxx int
    xxxxxxxxx,
    xxxxxxxxx,
    xxxxxxxxx;

  int
    xxxx;

  char
    *xxxxxxxxxxxxxxxx,
    *xxxxxxxxxxxx,
    *xxxxxxxxxxx;

  /*
    xxxxx xxxxxxxxx xxxxxxx.
  */
  xxxxxxxx int
    xxxxxx,
    xxxxxxxxxx;

  xxxxxxxxxxxxxx
    xxxxxxxxxx;

  /*
    xxxxxxxxx xxxxxxx.
  */
  char
    *xxxxxxx,
    *xxxxx,
    *xxxxxxxxxx;

  xxxxxxxx int
    xxxxxxxxxxxxxxxxxxxxxx,
    xxxxxxxxxxxxxxx,
    xxxxxxxxxxxxxxxx;

  /*
    xxxxxxxxxxxxx xxxxxxx.
  */
  xxxxxxxx int
    xxxxxxx;

  xxxxxxxxxxx
    xxxxxxxxxxxx;

  char
    *xxxx;

  long
    xxxxx;
} xxxxxxxxx;

xxxxxxx struct xxxxxxxxxxxx
{
  char
    xxxxxxxx[xxxxxxxxxxxxx];

  char
    *xxxxxxxx,
    *xxxx,
    *xxxxxxxxxxxxxxxx,
    *xxxxxxxxxxxx,
    *xxxxxxxxxxx,
    *xxxxx,
    *xxxxx,
    *xxxxxxx,
    *xxx,
    *xxxx;

  xxxxxxxx int
    xxxxxxxxx,
    xxxxxxxxxxxx,
    xxxxxxx,
    xxxxxx;

  xxxxxxxxxxxxxxxxx
    xxxxxxx;
} xxxxxxxxxxx;

xxxxxxx struct xxxxxxxxxx
{
  double
    x,
    x,
    x;
} xxxxxxxxx;

xxxxxxx struct xxxxxxxxxxxxxx
{
  xxxxxxxxxxxxx
    xxxxxxxxx;

  xxxxxxxx int
    xxxxxxxxxxx;

  double
    x,
    x;

  xxxxxxxxxxx
    xxxxxx;

  char
    *xxxx;
} xxxxxxxxxxxxx;

xxxxxxx struct xxxxxxxxxxxxxx
{
  xxxxxxxx int
    xxxxx,
    xxxxxx;

  int
    x,
    x;
} xxxxxxxxxxxxx;

xxxxxxx struct xxxxxxxxxxxxxxxx
{
  xxxxxxx
    xxx,
    xxxxx,
    xxxx,
    xxxxxx;

  xxxxxxxx short
    xxxxx;
} xxxxxxxxxxxxxxx;

xxxxxxx struct xxxxxxxxxxxx
{
  double
    xx,
    xx,
    xx,
    xx;
} xxxxxxxxxxx;

xxxxxxx struct xxxxxxxxxxxx
{
  xxxxxxxx int
    xxxxxx;

  xxxxxxxx char
    *xxxx;
} xxxxxxxxxxx;

xxxxxxx struct xxxxxxxxxxxxxxxxx
{
  xxxxxxxxx
    xxxxxxxxxxx,
    xxxxxxxxxxxxx,
    xxxxxxxxxxxx,
    xxxxxxxxxxx;
} xxxxxxxxxxxxxxxx;

xxxxxxx struct xxxxxx
{
  xxxxxxxx
    xxxx;

  xxxx
    *xxxx;

  int
    xxxxxx,
    xxxxxx,
    xxxxxxxxx;

  char
    xxxxxxxx[xxxxxxxxxxxxx];

  long int
    xxxxxxxx;

  int
    xxxx;

  char
    xxxxxx[xxxxxxxxxxxxx],
    *xxxxxxxx,
    *xxxxx;

  xxxxxxxxx
#if defined(xxxxxxxxxxx) || defined(xxxxxxxxxx)
    xxxxxxx;
#else
    xxxxx;
#endif

  xxxxxxxx int
    xxxxx;

  xxxxxxxxxxxxxxx
    xxxxxxxxxxx;

  xxxxxxxx int
    xxxxxxx,
    xxxx,
    xxxxx;

  int
    xxxxxx;

  xxxxxxxxxxxxx
    xxxxxxxxx;

  xxxxxxxxxxxxx
    xxxxxxxxx;

  xxxxxxxx int
    xxxxx;

  char
    *xxxxxxx,
    *xxxxxxxxx;

  xxxxxxxxxxx
    *xxxxxxxx;

  xxxxxxxx int
    xxxxxx;

  xxxxxxxxxxxxxx
    xxxxxxxxxx;

  xxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxx;

  double
    xxxxx;

  xxxxxxxxxxxxxxxx
    xxxxxxxxxxxx;

  xxxxxxxxxxx
    xxxxxxxxxxxxx,
    xxxxxxxxxxxx;

  xxxxxxxxxxxxxx
    xxxxx;

  double
    xxxxxxxxxxxx,
    xxxxxxxxxxxx;

  char
    *xxxxxxxxx;

  xxxxxxxxxxxxxxx
    *xxxxxx;

  xxxxxxxx long
    xxxxxxx;

  xxxxxxxx int
    xxxxxxxxxxx;

  xxxxxxxx char
    *xxxxxxxxxxxxx;

  xxxxxxxxxxx
    xxxxxxxxxxxxxxxx,
    xxxxxxxxxxxx,
    xxxxxxxxxxx;

  char
    *xxxxxxxx,
    *xxxx;

  xxxxxxxx int
    xxxxxxx,
    xxxxx,
    xxxxxxxxxx;

  int
    xxxx;

  xxxxxxxxxx
    xxxxxx;

  double
    xxxx;

  xxxxxxxx long
    xxxxxxxxxxxx;

  xxxxxxxx int
    xxxxxxxxxxxxxxxxxxxx;

  double
    xxxxxxxxxxxxxxxxxxxxx,
    xxxxxxxxxxxxxxxxxxxxxxxx;

  long int
    xxxxxxxxxxx;

  char
    xxxxxxxxxxxxxxx[xxxxxxxxxxxxx];

  xxxxxxxx int
    xxxxxxxxxxxxxx,
    xxxxxxxxxxx;

  int
    xxxxxxxxxxxxxxxxxxxxxx,
    xxxxxxx;

  xxxxxxxx int
    xxxxxx;

  struct xxxxxx
    *xxxxxxxx,
    *xxxx,
    *xxxx;
} xxxxx;

xxxxxxx struct xxxxxxxxxxxxx
{
  xxxxxxxxx
    *xxxxxxxxxx;

  xxxxxxxx int
    xxxxxxx;

  char
    *xxxxxxxx,
    *xxxx,
    *xxxxxxxxx,
    *xxxxxxxxx;

  double
    xxxxxxx;

  xxxxx
    *xxxx;

  xxxxxxxxxxxxx
    xxxxxx;
} xxxxxxxxxxxx;

xxxxxxx struct xxxxxxxxxxx
{
  const char
    *xxx;

  xxxxx
    *(*xxxxxxx)(const xxxxxxxxx *);

  xxxxxxxx int
    (*xxxxxxx)(const xxxxxxxxx *,xxxxx *),
    (*xxxxxx)(const xxxxxxxx char *,const xxxxxxxx int),
    xxxxxx,
    xxxxxxxxxxxx;

  const char
    *xxxxxxxxxxx;

  xxxx
    *xxxx;

  struct xxxxxxxxxxx
    *xxxxxxxx,
    *xxxx;
} xxxxxxxxxx;

/*
  xxxxx const xxxxxxxxxxxx.
*/
extern const char
  *xxxxxxxx,
  *xxxxxxxxxxxxxxx,
  *xxxxxxxxxxx,
  *xxxxxxxxxxxxxxxx,
  *xxxxxxxxxxxxxxxx,
  *xxxxxxxxxxxxxxxxxxx,
  *xxxxxxxxxxxxxxxx,
  *xxxxxxxxxxxxxxx,
  *xxxxxxxxxx,
  *xxxxxxxxxxxxx,
  *xxxxxxxxxxxxxx,
  *xxxxxxxxxxxxxxxxx,
  *xxxxxxxxxxxxxx,
  *xxxxxxxxxxxxxx,
  *xxxxxxxxxxxxxxxxxxxxxxxx,
  *xxxxxxxxxxxxx,
  *xxxxxxxxxxxxxx,
  *xxxxxxxxxxxxxxx;

extern const xxxxxxxxxxxxx
  xxxxxxxxxxxx[235],
  xxxxxxxxxx[757];

/*
  xxxxx xxxxxxxxx xxxxxxx.
*/
extern xxxxxx xxxxxxxxxxxx
  *xxxxxxxxxxxxxxxxx(const xxxxxxxxx *,const xxxxxxxxxxxx *);

extern xxxxxx xxxxx
  *xxxxxxxxxxxxx(const xxxxx *,const xxxxxxxxx),
  *xxxxxxxxxxxxx(const xxxxxxxxx *),
  *xxxxxxxxxxxx(xxxxx *,const xxxxxxxx int),
  *xxxxxxxxxxxxx(const xxxxx *),
  *xxxxxxxxx(const xxxxx *,const double),
  *xxxxxxxxxxx(const xxxxx *,const xxxxxxxxxxxxx *),
  *xxxxxxxxx(const xxxxx *,const xxxxxxxxxxxxx *),
  *xxxxxxxxxx(const xxxxx *,const xxxxxxxx int,const xxxxxxxx int,
    const xxxxxxxx int),
  *xxxxxxxxxxx(const xxxxxxxx int,const xxxxxxxx int,const xxxxx *,
    const xxxxx *,const xxxxx *,const xxxxx *),
  *xxxxxxxxx(const xxxxx *,const xxxxxxxxxxxxx *),
  *xxxxxxxxxxxxxx(xxxxx *),
  *xxxxxxxxx(const xxxxx *,const double),
  *xxxxxxxxxxx(const xxxxx *),
  *xxxxxxxxxxxx(const xxxxx *),
  *xxxxxxxxx(const xxxxx *),
  *xxxxxxxxx(const xxxxx *),
  *xxxxxxxxxx(const xxxxx *,const xxxxxxxxx *),
  *xxxxxxxxxxxx(xxxxx *,const double),
  **xxxxxxxxxxxxxxxx(const xxxxx *,xxxxxxxx int *),
  *xxxxxxxxxxxx(xxxxx *),
  *xxxxxxxxxxx(xxxxx *),
  *xxxxxxxxxxxxx(const xxxxx *,const xxxxxxxxxxx *),
  *xxxxxxxxxxx(xxxxx *,const xxxxxxxx int),
  *xxxxxxxxxxxxx(xxxxx *,const xxxxxxxx int),
  *xxxxxxxxx(const xxxxxxxxx *),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxx(xxxxxxxxx *),
  *xxxxxxxxxx(xxxxxxxxx *),
  *xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx),
  *xxxxxxxxxxxxxxxx(const xxxxx *),
  *xxxxxxxxx(const xxxxx *,const int,const int),
  *xxxxxxxxxxx(const xxxxx *,const double,const xxxxxxxx int,
    const xxxxxxxx int),
  *xxxxxxxxxxx(const xxxxx *,const xxxxxxxx int,const xxxxxxxx int),
  *xxxxxxxxxx(const xxxxx *,const xxxxxxxx int,const xxxxxxxx int),
  *xxxxxxxxxx(xxxxx *,const xxxxxxxx int,double,double),
  *xxxxxxxxxxxx(const xxxxx *,const double),
  *xxxxxxxxxx(const xxxxx *,const double,const double,const xxxxxxxx int),
  *xxxxxxxxxxx(xxxxx *,const xxxxxxxx int),
  *xxxxxxxxxxxx(xxxxx *,xxxxx *),
  *xxxxxxxxxxx(xxxxx *,const xxxxx *),
  *xxxxxxxxxx(xxxxx *,double),
  *xxxxxxxxx(xxxxx *,const double,const double),
  *xxxxxxxxx(xxxxx *,const xxxxxxxx int,const xxxxxxxx int);

extern xxxxxx xxxxxxxxx
  *xxxxxxxxxxxxxx(const xxxxxxxxx *);

extern xxxxxx xxxxxxxxx
  xxxxxxxxxxxx(xxxxx *);

extern xxxxxx int
  xxxxxxxxxxxxx(const char *,int *,int *,xxxxxxxx int *,xxxxxxxx int *),
  xxxxxxxxxxxxxxxxxx(const char *,int *,int *,xxxxxxxx int *,xxxxxxxx int *);

extern xxxxxx xxxxxxxxxx
  *xxxxxxxxxxxxx(const char *),
  *xxxxxxxxxxxxxxxxxx(const char *,xxxxx *(*)(const xxxxxxxxx *),
    xxxxxxxx int (*)(const xxxxxxxxx *,xxxxx *),
    xxxxxxxx int (*)(const xxxxxxxx char *,const xxxxxxxx int),
    const xxxxxxxx int,const xxxxxxxx int,const char *);

extern xxxxxx xxxxxxxx int
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxxxx(const xxxxx *),
  xxxxxxxxxx(const char *),
  xxxxxxxxxxx(xxxxx *),
  xxxxxxxxxxxxxxxxx(xxxxx *),
  xxxxxxxxxxxxx(xxxxx *),
  xxxxxxxxxx(const char *,const xxxxxxxx int),
  xxxxxxxxx(const xxxxx *),
  xxxxxxxxxxx(xxxxx *,const xxxxxxxxxxx *,int,int),
  xxxxxxxxxxxxxxxxxx(const char *,xxxxxxxxxxx *),
  xxxxxxxxxxxxxxx(xxxxx *),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx, xxxxx *xxxxx),
  xxxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxx(const xxxxxxxxx *,xxxxx *),
  xxxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxx(const xxxxxxxxx *,xxxxx *),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx),
  xxxxxxxxxxxxx(const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx);

extern xxxxxx xxxx
  xxxxxxxxxxxxxxxxx(const xxxxxxxxx *,xxxxx *),
  xxxxxxxxxxxxx(xxxxx *,const xxxxxxxxxxxx *),
  xxxxxxxxxxxxxx(xxxxx *),
  xxxxxxxxxxxxxxxxxxx(xxxxx *,const xxxxxxxxxxxxxxx *,xxxxx *,const int x,
    const int x,const xxxxxxxxxxx),
  xxxxxxxxxxxxx(xxxxx *,const char *,const char *),
  xxxxxxxxxxxx(xxxxx *,const char *),
  xxxxxxxxxxxxxx(xxxxx *,const xxxxxxxxxxxxxxxxx,xxxxx *,const int,const int),
  xxxxxxxxxxxxxxxx(xxxxx *),
  xxxxxxxxxxxxx(xxxxx *),
  xxxxxxxxxxxxx(xxxxx *,const xxxxxxxx int),
  xxxxxxxxxxxxxxxxxx(xxxxx *,const int),
  xxxxxxxxxxxxxxxxx(xxxxx *),
  xxxxxxxxxxxxx(xxxxx *,xxxx *,const xxxxxxxx int),
  xxxxxxxxxxxxxxxxxxx(xxxxxxxxxxxx *),
  xxxxxxxxxxxx(xxxxx *),
  xxxxxxxxxxxxxxxx(xxxxxxxxx *),
  xxxxxxxxxxxxx(xxxxx *),
  xxxxxxxxxxxxxxxxxx(xxxxxxxxxxx *),
  xxxxxxxxx(xxxxx *,const xxxxxxxxxxxx *),
  xxxxxxxxxxxxx(xxxxx *),
  xxxxxxxxxx(xxxxx *,const char *),
  xxxxxxxxxxxxxxx(const xxxxxxxxx *,xxxxxxxxxxxx *),
  xxxxxxxxxxxx(xxxxxxxxx *),
  xxxxxxxxxxxxxx(xxxxxxxxxxx *),
  xxxxxxxxx(const xxxxx *,xxxxx *,xxxxx *,xxxxx *,xxxxx *),
  xxxxxxxxxx(xxxxx *,const char *),
  xxxxxxxxxx(xxxxx *,const xxxxxxxxx),
  xxxxxxxxxxxxxx(xxxx *),
  xxxxxxxxxxxxxxxxxxx(xxxxx *,const xxxxxxxxxxxxxxx *,const xxxxxxxx int,
    const int x,const int x,const xxxxxxxxxxx),
  xxxxxxxxxx(xxxxx *),
  xxxxxxxxxxxxx(xxxxx *,const char *),
  xxxxxxxxxxxx(const xxxxxxxxx *,const int,char **,xxxxx **),
  xxxxxxxxxxxxx(const xxxxxxxxx *,const int,char **,xxxxx **),
  xxxxxxxxxxx(xxxxx *,const xxxxxxxx int),
  xxxxxxxxxxxxxx(xxxxx *),
  xxxxxxxxxxx(xxxxx *,const char *,const char *),
  xxxxxxxxxx(xxxxx *,const xxxxxxxxxxxxx *,const int),
  xxxxxxxxxxxxxxxxx(xxxxx *,const xxxxxxxxxxxxxx),
  xxxxxxxx(xxxxx *),
  xxxxxxxxxxxx(xxxxxxxxx *,const xxxxxxxx int),
  xxxxxxxxxxxxxx(xxxxx *),
  xxxxxxxxxxxxx(xxxxx *,const double),
  xxxxxxxxxxxxxxxxxxxxxxx(xxxxx *),
  xxxxxxxxx(xxxxx *),
  xxxxxxxxxxxx(xxxxx *,xxxxx *),
  xxxxxxxxxxxxxx(xxxxx *,const double),
  xxxxxxxxxxxx(const xxxxxxx,const xxxxxxx,const xxxxxxx,double *,double *,
    double *),
  xxxxxxxxxxxxxx(xxxxx **,const char *,const char *),
  xxxxxxxxxxxxxxxxx(xxxxx *,const xxxxxxxxxxxxxx),
  xxxxxxxxxxxxxxxx(xxxxx *,const char *);

#if defined(xxxxxxxxxxx) || defined(xxxxxxxxxx)
}
#endif

#endif'
		OUTPUT - $'/*
  xxxxxxxxxxx xxxxx xxxxxxx.
*/
#ifndef xxxxxxxx
#define xxxxxxxx

#if defined(xxxxxxxxxxx) || defined(xxxxxxxxxx)
extern __MANGLE__ "x" {
#endif

#if defined(xxxxxxxxxxx)
/*
  xxxxx xxxxxxx xx [0..65535].
*/
#define xxxxxxxxx(xxxxxxx)  (((xxxxxxxx long) (xxxxxxx)) >> 8)
#define xxxxxxxxxxxxxx "#%04x%04x%04x"
#define xxxxxx  65535x
#define xxxxxxxxxxxx  65535x
#define xxxxxxxxxxxx  16
#define xxxxxxx(xxxxxxx)  (((xxxxxxxx long) (xxxxxxx))*257)
#define xxxxxxxxxx(xxxxx)  ((xxxxxxxx long) (xxxxx))
#define xxxxxxxx(xxxxx)  ((xxxxxxxx long) (xxxxx))

xxxxxxx xxxxxxxx short xxxxxxx;
#else
/*
  xxxxx xxxxxxx xx [0..255].
*/
#define xxxxxxxxx(xxxxxxx)  ((xxxxxxxx long) (xxxxxxx))
#define xxxxxxxxxxxxxx "#%02x%02x%02x"
#define xxxxxx  255
#define xxxxxxxxxxxx  255
#define xxxxxxxxxxxx  8
#define xxxxxxx(xxxxxxx)  ((xxxxxxxx long) (xxxxxxx))
#define xxxxxxxxxx(xxxxx)  (((xxxxxxxx long) (xxxxx)) >> 8)
#define xxxxxxxx(xxxxx)  (((xxxxxxxx long) (xxxxx))*257)

xxxxxxx xxxxxxxx char xxxxxxx;
#endif

/*
  3x xxxxxxx.
*/
#define xxxxxxxxxxxxxxxxxx  xxxxxxx(80)
#define xxxxxxxxxxxxxxxxx  xxxxxxx(125)
#define xxxxxxxxxxxxxx  xxxxxxx(135)
#define xxxxxxxxxxxxx  xxxxxxx(185)
#define xxxxxxxxxxxxxx  xxxxxxx(110)

/*
  xxxxxxx xxxxxxxxxxxx.
*/
xxxxxxx struct xxxxxxxxx
{
  char
    *xxxx;

  long
    xxxxxx,
    xxxxxx,
    xxxxxx,
    xxxxxxx;
} xxxxxxxx;

xxxxxxx struct xxxxxxxxxxxxxx
{
  char
    *xxxx;

  xxxxxxxx char
    xxx,
    xxxxx,
    xxxx;
} xxxxxxxxxxxxx;

xxxxxxx struct xxxxxxxxxxxx
{
  xxxxxxxx short
    xxx,
    xxxxx,
    xxxx,
    xxxxx;

  xxxxxxxx char
    xxxxx;

  char
    xxx[3];

  xxxxxxxx long
    xxxxx;
} xxxxxxxxxxx;

xxxxxxx struct xxxxxxxxxxxxxxxxx
{
  int
    xxxxx;

  double
    xxxxxx;
} xxxxxxxxxxxxxxxx;

xxxxxxx struct xxxxxxxxxxx
{
  double
    (*xxxxxxxx) __PROTO__((double)),
    xxxxxxx;
} xxxxxxxxxx;

xxxxxxx struct xxxxxxxxxx
{
  int
    x,
    x;

  xxxxxxxx int
    xxxxx,
    xxxxxx;

  int
    xxxxxxxxxxx,
    xxxxxxxxxxx;
} xxxxxxxxx;

xxxxxxx struct xxxxxxxxxx
{
  /*
    xxxx xxxxxx.
  */
  xxxxxxxx
    xxxx;
  /*
    xxxx xxx xxxxx xxxxxxxxx xxxxxxx.
  */
  xxxx
    *xxxx;

  char
    xxxxxxxx[xxxxxxxxxxxxx],
    xxxxxx[xxxxxxxxxxxxx],
    xxxxxx[xxxxxxxxxxxxx],
    xxxx[xxxxxxxxxxxxx];

  xxxxxxxx int
    xxxxxx,
    xxxxxxxxx,
    xxxxxx,
    xxxxxxxx,
    xxxxxxxx,
    xxxxx,
    xxxx;

  char
    *xxxx,
    *xxxx,
    *xxxx;

  xxxxxxxxxxxxx
    xxxxxxxxx;

  xxxxxxxxxxxxxx
    xxxxx;

  /*
    xxxxxxxxxxx xxxxxxx.
  */
  xxxxxxxxxxxxxxx
    xxxxxxxxxxx;

  xxxxxxxx int
    xxxxxxx;

  /*
    xxxxxxxxxx xxxxxxx.
  */
  char
    *xxxxxxxxxxx,
    *xxx,
    *xxxx,
    *xxx,
    *xxxxxxx,
    *xxxxxxx;

  xxxxxxxx int
    xxxxxxxxx,
    xxxxxxxxx,
    xxxxxxxxx;

  int
    xxxx;

  char
    *xxxxxxxxxxxxxxxx,
    *xxxxxxxxxxxx,
    *xxxxxxxxxxx;

  /*
    xxxxx xxxxxxxxx xxxxxxx.
  */
  xxxxxxxx int
    xxxxxx,
    xxxxxxxxxx;

  xxxxxxxxxxxxxx
    xxxxxxxxxx;

  /*
    xxxxxxxxx xxxxxxx.
  */
  char
    *xxxxxxx,
    *xxxxx,
    *xxxxxxxxxx;

  xxxxxxxx int
    xxxxxxxxxxxxxxxxxxxxxx,
    xxxxxxxxxxxxxxx,
    xxxxxxxxxxxxxxxx;

  /*
    xxxxxxxxxxxxx xxxxxxx.
  */
  xxxxxxxx int
    xxxxxxx;

  xxxxxxxxxxx
    xxxxxxxxxxxx;

  char
    *xxxx;

  long
    xxxxx;
} xxxxxxxxx;

xxxxxxx struct xxxxxxxxxxxx
{
  char
    xxxxxxxx[xxxxxxxxxxxxx];

  char
    *xxxxxxxx,
    *xxxx,
    *xxxxxxxxxxxxxxxx,
    *xxxxxxxxxxxx,
    *xxxxxxxxxxx,
    *xxxxx,
    *xxxxx,
    *xxxxxxx,
    *xxx,
    *xxxx;

  xxxxxxxx int
    xxxxxxxxx,
    xxxxxxxxxxxx,
    xxxxxxx,
    xxxxxx;

  xxxxxxxxxxxxxxxxx
    xxxxxxx;
} xxxxxxxxxxx;

xxxxxxx struct xxxxxxxxxx
{
  double
    x,
    x,
    x;
} xxxxxxxxx;

xxxxxxx struct xxxxxxxxxxxxxx
{
  xxxxxxxxxxxxx
    xxxxxxxxx;

  xxxxxxxx int
    xxxxxxxxxxx;

  double
    x,
    x;

  xxxxxxxxxxx
    xxxxxx;

  char
    *xxxx;
} xxxxxxxxxxxxx;

xxxxxxx struct xxxxxxxxxxxxxx
{
  xxxxxxxx int
    xxxxx,
    xxxxxx;

  int
    x,
    x;
} xxxxxxxxxxxxx;

xxxxxxx struct xxxxxxxxxxxxxxxx
{
  xxxxxxx
    xxx,
    xxxxx,
    xxxx,
    xxxxxx;

  xxxxxxxx short
    xxxxx;
} xxxxxxxxxxxxxxx;

xxxxxxx struct xxxxxxxxxxxx
{
  double
    xx,
    xx,
    xx,
    xx;
} xxxxxxxxxxx;

xxxxxxx struct xxxxxxxxxxxx
{
  xxxxxxxx int
    xxxxxx;

  xxxxxxxx char
    *xxxx;
} xxxxxxxxxxx;

xxxxxxx struct xxxxxxxxxxxxxxxxx
{
  xxxxxxxxx
    xxxxxxxxxxx,
    xxxxxxxxxxxxx,
    xxxxxxxxxxxx,
    xxxxxxxxxxx;
} xxxxxxxxxxxxxxxx;

xxxxxxx struct xxxxxx
{
  xxxxxxxx
    xxxx;

  xxxx
    *xxxx;

  int
    xxxxxx,
    xxxxxx,
    xxxxxxxxx;

  char
    xxxxxxxx[xxxxxxxxxxxxx];

  long int
    xxxxxxxx;

  int
    xxxx;

  char
    xxxxxx[xxxxxxxxxxxxx],
    *xxxxxxxx,
    *xxxxx;

  xxxxxxxxx
#if defined(xxxxxxxxxxx) || defined(xxxxxxxxxx)
    xxxxxxx;
#else
    xxxxx;
#endif

  xxxxxxxx int
    xxxxx;

  xxxxxxxxxxxxxxx
    xxxxxxxxxxx;

  xxxxxxxx int
    xxxxxxx,
    xxxx,
    xxxxx;

  int
    xxxxxx;

  xxxxxxxxxxxxx
    xxxxxxxxx;

  xxxxxxxxxxxxx
    xxxxxxxxx;

  xxxxxxxx int
    xxxxx;

  char
    *xxxxxxx,
    *xxxxxxxxx;

  xxxxxxxxxxx
    *xxxxxxxx;

  xxxxxxxx int
    xxxxxx;

  xxxxxxxxxxxxxx
    xxxxxxxxxx;

  xxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxx;

  double
    xxxxx;

  xxxxxxxxxxxxxxxx
    xxxxxxxxxxxx;

  xxxxxxxxxxx
    xxxxxxxxxxxxx,
    xxxxxxxxxxxx;

  xxxxxxxxxxxxxx
    xxxxx;

  double
    xxxxxxxxxxxx,
    xxxxxxxxxxxx;

  char
    *xxxxxxxxx;

  xxxxxxxxxxxxxxx
    *xxxxxx;

  xxxxxxxx long
    xxxxxxx;

  xxxxxxxx int
    xxxxxxxxxxx;

  xxxxxxxx char
    *xxxxxxxxxxxxx;

  xxxxxxxxxxx
    xxxxxxxxxxxxxxxx,
    xxxxxxxxxxxx,
    xxxxxxxxxxx;

  char
    *xxxxxxxx,
    *xxxx;

  xxxxxxxx int
    xxxxxxx,
    xxxxx,
    xxxxxxxxxx;

  int
    xxxx;

  xxxxxxxxxx
    xxxxxx;

  double
    xxxx;

  xxxxxxxx long
    xxxxxxxxxxxx;

  xxxxxxxx int
    xxxxxxxxxxxxxxxxxxxx;

  double
    xxxxxxxxxxxxxxxxxxxxx,
    xxxxxxxxxxxxxxxxxxxxxxxx;

  long int
    xxxxxxxxxxx;

  char
    xxxxxxxxxxxxxxx[xxxxxxxxxxxxx];

  xxxxxxxx int
    xxxxxxxxxxxxxx,
    xxxxxxxxxxx;

  int
    xxxxxxxxxxxxxxxxxxxxxx,
    xxxxxxx;

  xxxxxxxx int
    xxxxxx;

  struct xxxxxx
    *xxxxxxxx,
    *xxxx,
    *xxxx;
} xxxxx;

xxxxxxx struct xxxxxxxxxxxxx
{
  xxxxxxxxx
    *xxxxxxxxxx;

  xxxxxxxx int
    xxxxxxx;

  char
    *xxxxxxxx,
    *xxxx,
    *xxxxxxxxx,
    *xxxxxxxxx;

  double
    xxxxxxx;

  xxxxx
    *xxxx;

  xxxxxxxxxxxxx
    xxxxxx;
} xxxxxxxxxxxx;

xxxxxxx struct xxxxxxxxxxx
{
  const char
    *xxx;

  xxxxx
    *(*xxxxxxx) __PROTO__((const xxxxxxxxx *));

  xxxxxxxx int
    (*xxxxxxx) __PROTO__((const xxxxxxxxx *,xxxxx *)),
    (*xxxxxx) __PROTO__((const xxxxxxxx char *,const xxxxxxxx int)),
    xxxxxx,
    xxxxxxxxxxxx;

  const char
    *xxxxxxxxxxx;

  xxxx
    *xxxx;

  struct xxxxxxxxxxx
    *xxxxxxxx,
    *xxxx;
} xxxxxxxxxx;

/*
  xxxxx const xxxxxxxxxxxx.
*/
extern __MANGLE__ const char
  *xxxxxxxx,
  *xxxxxxxxxxxxxxx,
  *xxxxxxxxxxx,
  *xxxxxxxxxxxxxxxx,
  *xxxxxxxxxxxxxxxx,
  *xxxxxxxxxxxxxxxxxxx,
  *xxxxxxxxxxxxxxxx,
  *xxxxxxxxxxxxxxx,
  *xxxxxxxxxx,
  *xxxxxxxxxxxxx,
  *xxxxxxxxxxxxxx,
  *xxxxxxxxxxxxxxxxx,
  *xxxxxxxxxxxxxx,
  *xxxxxxxxxxxxxx,
  *xxxxxxxxxxxxxxxxxxxxxxxx,
  *xxxxxxxxxxxxx,
  *xxxxxxxxxxxxxx,
  *xxxxxxxxxxxxxxx;

extern __MANGLE__ const xxxxxxxxxxxxx
  xxxxxxxxxxxx[235],
  xxxxxxxxxx[757];

/*
  xxxxx xxxxxxxxx xxxxxxx.
*/
extern __MANGLE__ xxxxxx xxxxxxxxxxxx
  *xxxxxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *,const xxxxxxxxxxxx *));

extern __MANGLE__ xxxxxx xxxxx
  *xxxxxxxxxxxxx __PROTO__((const xxxxx *,const xxxxxxxxx)),
  *xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *)),
  *xxxxxxxxxxxx __PROTO__((xxxxx *,const xxxxxxxx int)),
  *xxxxxxxxxxxxx __PROTO__((const xxxxx *)),
  *xxxxxxxxx __PROTO__((const xxxxx *,const double)),
  *xxxxxxxxxxx __PROTO__((const xxxxx *,const xxxxxxxxxxxxx *)),
  *xxxxxxxxx __PROTO__((const xxxxx *,const xxxxxxxxxxxxx *)),
  *xxxxxxxxxx __PROTO__((const xxxxx *,const xxxxxxxx int,const xxxxxxxx int,
    const xxxxxxxx int)),
  *xxxxxxxxxxx __PROTO__((const xxxxxxxx int,const xxxxxxxx int,const xxxxx *,
    const xxxxx *,const xxxxx *,const xxxxx *)),
  *xxxxxxxxx __PROTO__((const xxxxx *,const xxxxxxxxxxxxx *)),
  *xxxxxxxxxxxxxx __PROTO__((xxxxx *)),
  *xxxxxxxxx __PROTO__((const xxxxx *,const double)),
  *xxxxxxxxxxx __PROTO__((const xxxxx *)),
  *xxxxxxxxxxxx __PROTO__((const xxxxx *)),
  *xxxxxxxxx __PROTO__((const xxxxx *)),
  *xxxxxxxxx __PROTO__((const xxxxx *)),
  *xxxxxxxxxx __PROTO__((const xxxxx *,const xxxxxxxxx *)),
  *xxxxxxxxxxxx __PROTO__((xxxxx *,const double)),
  **xxxxxxxxxxxxxxxx __PROTO__((const xxxxx *,xxxxxxxx int *)),
  *xxxxxxxxxxxx __PROTO__((xxxxx *)),
  *xxxxxxxxxxx __PROTO__((xxxxx *)),
  *xxxxxxxxxxxxx __PROTO__((const xxxxx *,const xxxxxxxxxxx *)),
  *xxxxxxxxxxx __PROTO__((xxxxx *,const xxxxxxxx int)),
  *xxxxxxxxxxxxx __PROTO__((xxxxx *,const xxxxxxxx int)),
  *xxxxxxxxx __PROTO__((const xxxxxxxxx *)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxx __PROTO__((xxxxxxxxx *)),
  *xxxxxxxxxx __PROTO__((xxxxxxxxx *)),
  *xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx)),
  *xxxxxxxxxxxxxxxx __PROTO__((const xxxxx *)),
  *xxxxxxxxx __PROTO__((const xxxxx *,const int,const int)),
  *xxxxxxxxxxx __PROTO__((const xxxxx *,const double,const xxxxxxxx int,
    const xxxxxxxx int)),
  *xxxxxxxxxxx __PROTO__((const xxxxx *,const xxxxxxxx int,const xxxxxxxx int)),
  *xxxxxxxxxx __PROTO__((const xxxxx *,const xxxxxxxx int,const xxxxxxxx int)),
  *xxxxxxxxxx __PROTO__((xxxxx *,const xxxxxxxx int,double,double)),
  *xxxxxxxxxxxx __PROTO__((const xxxxx *,const double)),
  *xxxxxxxxxx __PROTO__((const xxxxx *,const double,const double,const xxxxxxxx int)),
  *xxxxxxxxxxx __PROTO__((xxxxx *,const xxxxxxxx int)),
  *xxxxxxxxxxxx __PROTO__((xxxxx *,xxxxx *)),
  *xxxxxxxxxxx __PROTO__((xxxxx *,const xxxxx *)),
  *xxxxxxxxxx __PROTO__((xxxxx *,double)),
  *xxxxxxxxx __PROTO__((xxxxx *,const double,const double)),
  *xxxxxxxxx __PROTO__((xxxxx *,const xxxxxxxx int,const xxxxxxxx int));

extern __MANGLE__ xxxxxx xxxxxxxxx
  *xxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *));

extern __MANGLE__ xxxxxx xxxxxxxxx
  xxxxxxxxxxxx __PROTO__((xxxxx *));

extern __MANGLE__ xxxxxx int
  xxxxxxxxxxxxx __PROTO__((const char *,int *,int *,xxxxxxxx int *,xxxxxxxx int *)),
  xxxxxxxxxxxxxxxxxx __PROTO__((const char *,int *,int *,xxxxxxxx int *,xxxxxxxx int *));

extern __MANGLE__ xxxxxx xxxxxxxxxx
  *xxxxxxxxxxxxx __PROTO__((const char *)),
  *xxxxxxxxxxxxxxxxxx __PROTO__((const char *,xxxxx *(*)(const xxxxxxxxx *),
    xxxxxxxx int (*)(const xxxxxxxxx *,xxxxx *),
    xxxxxxxx int (*)(const xxxxxxxx char *,const xxxxxxxx int),
    const xxxxxxxx int,const xxxxxxxx int,const char *));

extern __MANGLE__ xxxxxx xxxxxxxx int
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxxxx __PROTO__((const xxxxx *)),
  xxxxxxxxxx __PROTO__((const char *)),
  xxxxxxxxxxx __PROTO__((xxxxx *)),
  xxxxxxxxxxxxxxxxx __PROTO__((xxxxx *)),
  xxxxxxxxxxxxx __PROTO__((xxxxx *)),
  xxxxxxxxxx __PROTO__((const char *,const xxxxxxxx int)),
  xxxxxxxxx __PROTO__((const xxxxx *)),
  xxxxxxxxxxx __PROTO__((xxxxx *,const xxxxxxxxxxx *,int,int)),
  xxxxxxxxxxxxxxxxxx __PROTO__((const char *,xxxxxxxxxxx *)),
  xxxxxxxxxxxxxxx __PROTO__((xxxxx *)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx, xxxxx *xxxxx)),
  xxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxx __PROTO__((const xxxxxxxxx *,xxxxx *)),
  xxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *,xxxxx *)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *xxxxxxxxxx,xxxxx *xxxxx));

extern __MANGLE__ xxxxxx xxxx
  xxxxxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *,xxxxx *)),
  xxxxxxxxxxxxx __PROTO__((xxxxx *,const xxxxxxxxxxxx *)),
  xxxxxxxxxxxxxx __PROTO__((xxxxx *)),
  xxxxxxxxxxxxxxxxxxx __PROTO__((xxxxx *,const xxxxxxxxxxxxxxx *,xxxxx *,const int x,
    const int x,const xxxxxxxxxxx)),
  xxxxxxxxxxxxx __PROTO__((xxxxx *,const char *,const char *)),
  xxxxxxxxxxxx __PROTO__((xxxxx *,const char *)),
  xxxxxxxxxxxxxx __PROTO__((xxxxx *,const xxxxxxxxxxxxxxxxx,xxxxx *,const int,const int)),
  xxxxxxxxxxxxxxxx __PROTO__((xxxxx *)),
  xxxxxxxxxxxxx __PROTO__((xxxxx *)),
  xxxxxxxxxxxxx __PROTO__((xxxxx *,const xxxxxxxx int)),
  xxxxxxxxxxxxxxxxxx __PROTO__((xxxxx *,const int)),
  xxxxxxxxxxxxxxxxx __PROTO__((xxxxx *)),
  xxxxxxxxxxxxx __PROTO__((xxxxx *,xxxx *,const xxxxxxxx int)),
  xxxxxxxxxxxxxxxxxxx __PROTO__((xxxxxxxxxxxx *)),
  xxxxxxxxxxxx __PROTO__((xxxxx *)),
  xxxxxxxxxxxxxxxx __PROTO__((xxxxxxxxx *)),
  xxxxxxxxxxxxx __PROTO__((xxxxx *)),
  xxxxxxxxxxxxxxxxxx __PROTO__((xxxxxxxxxxx *)),
  xxxxxxxxx __PROTO__((xxxxx *,const xxxxxxxxxxxx *)),
  xxxxxxxxxxxxx __PROTO__((xxxxx *)),
  xxxxxxxxxx __PROTO__((xxxxx *,const char *)),
  xxxxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *,xxxxxxxxxxxx *)),
  xxxxxxxxxxxx __PROTO__((xxxxxxxxx *)),
  xxxxxxxxxxxxxx __PROTO__((xxxxxxxxxxx *)),
  xxxxxxxxx __PROTO__((const xxxxx *,xxxxx *,xxxxx *,xxxxx *,xxxxx *)),
  xxxxxxxxxx __PROTO__((xxxxx *,const char *)),
  xxxxxxxxxx __PROTO__((xxxxx *,const xxxxxxxxx)),
  xxxxxxxxxxxxxx __PROTO__((xxxx *)),
  xxxxxxxxxxxxxxxxxxx __PROTO__((xxxxx *,const xxxxxxxxxxxxxxx *,const xxxxxxxx int,
    const int x,const int x,const xxxxxxxxxxx)),
  xxxxxxxxxx __PROTO__((xxxxx *)),
  xxxxxxxxxxxxx __PROTO__((xxxxx *,const char *)),
  xxxxxxxxxxxx __PROTO__((const xxxxxxxxx *,const int,char **,xxxxx **)),
  xxxxxxxxxxxxx __PROTO__((const xxxxxxxxx *,const int,char **,xxxxx **)),
  xxxxxxxxxxx __PROTO__((xxxxx *,const xxxxxxxx int)),
  xxxxxxxxxxxxxx __PROTO__((xxxxx *)),
  xxxxxxxxxxx __PROTO__((xxxxx *,const char *,const char *)),
  xxxxxxxxxx __PROTO__((xxxxx *,const xxxxxxxxxxxxx *,const int)),
  xxxxxxxxxxxxxxxxx __PROTO__((xxxxx *,const xxxxxxxxxxxxxx)),
  xxxxxxxx __PROTO__((xxxxx *)),
  xxxxxxxxxxxx __PROTO__((xxxxxxxxx *,const xxxxxxxx int)),
  xxxxxxxxxxxxxx __PROTO__((xxxxx *)),
  xxxxxxxxxxxxx __PROTO__((xxxxx *,const double)),
  xxxxxxxxxxxxxxxxxxxxxxx __PROTO__((xxxxx *)),
  xxxxxxxxx __PROTO__((xxxxx *)),
  xxxxxxxxxxxx __PROTO__((xxxxx *,xxxxx *)),
  xxxxxxxxxxxxxx __PROTO__((xxxxx *,const double)),
  xxxxxxxxxxxx __PROTO__((const xxxxxxx,const xxxxxxx,const xxxxxxx,double *,double *,
    double *)),
  xxxxxxxxxxxxxx __PROTO__((xxxxx **,const char *,const char *)),
  xxxxxxxxxxxxxxxxx __PROTO__((xxxxx *,const xxxxxxxxxxxxxx)),
  xxxxxxxxxxxxxxxx __PROTO__((xxxxx *,const char *));

#if defined(xxxxxxxxxxx) || defined(xxxxxxxxxx)
}
#endif

#endif'
	EXEC -fhn
		INPUT - $'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;
f(x);'
		SAME OUTPUT INPUT

TEST 09 'more buffer boundaries'
	EXEC -nh
		INPUT - $'#pragma prototyped
/*
 *\t$XConsortium: X.h,v 1.66 88/09/06 15:55:56 jim Exp $
 */

/* Definitions for the X window system likely to be used by applications */

#ifndef X_H
#define X_H

/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
#define X_PROTOCOL\t11\t\t/* current protocol version */
#define X_PROTOCOL_REVISION 0\t\t/* current minor version */

/* Resources */

typedef unsigned long XID;

typedef XID Window;
typedef XID Drawable;
typedef XID Font;
typedef XID Pixmap;
typedef XID Cursor;
typedef XID Colormap;
typedef XID GContext;
typedef XID KeySym;

typedef unsigned long Mask;

typedef unsigned long Atom;

typedef unsigned long VisualID;

typedef unsigned long Time;

typedef unsigned char KeyCode;

/*****************************************************************
 * RESERVED RESOURCE AND CONSTANT DEFINITIONS
 *****************************************************************/

#define None                 0L\t/* universal null resource or null atom */

#define ParentRelative       1L\t/* background pixmap in CreateWindow
\t\t\t\t    and ChangeWindowAttributes */

#define CopyFromParent       0L\t/* border pixmap in CreateWindow
\t\t\t\t       and ChangeWindowAttributes
\t\t\t\t   special VisualID and special window
\t\t\t\t       class passed to CreateWindow */

#define PointerWindow        0L\t/* destination window in SendEvent */
#define InputFocus           1L\t/* destination window in SendEvent */

#define PointerRoot          1L\t/* focus window in SetInputFocus */

#define AnyPropertyType      0L\t/* special Atom, passed to GetProperty */

#define AnyKey\t\t     0L\t/* special Key Code, passed to GrabKey */

#define AnyButton            0L\t/* special Button Code, passed to GrabButton */

#define AllTemporary         0L\t/* special Resource ID passed to KillClient */

#define CurrentTime          0L\t/* special Time */

#define NoSymbol\t     0L\t/* special KeySym */

/***************************************************************** 
 * EVENT DEFINITIONS 
 *****************************************************************/

/* Input Event Masks. Used as event-mask window attribute and as arguments
   to Grab requests.  Not to be confused with event names.  */

#define NoEventMask\t\t\t0L
#define KeyPressMask\t\t\t(1L<<0)  
#define KeyReleaseMask\t\t\t(1L<<1)  
#define ButtonPressMask\t\t\t(1L<<2)  
#define ButtonReleaseMask\t\t(1L<<3)  
#define EnterWindowMask\t\t\t(1L<<4)  
#define LeaveWindowMask\t\t\t(1L<<5)  
#define PointerMotionMask\t\t(1L<<6)  
#define PointerMotionHintMask\t\t(1L<<7)  
#define Button1MotionMask\t\t(1L<<8)  
#define Button2MotionMask\t\t(1L<<9)  
#define Button3MotionMask\t\t(1L<<10) 
#define Button4MotionMask\t\t(1L<<11) 
#define Button5MotionMask\t\t(1L<<12) 
#define ButtonMotionMask\t\t(1L<<13) 
#define KeymapStateMask\t\t\t(1L<<14)
#define ExposureMask\t\t\t(1L<<15) 
#define VisibilityChangeMask\t\t(1L<<16) 
#define StructureNotifyMask\t\t(1L<<17) 
#define ResizeRedirectMask\t\t(1L<<18) 
#define SubstructureNotifyMask\t\t(1L<<19) 
#define SubstructureRedirectMask\t(1L<<20) 
#define FocusChangeMask\t\t\t(1L<<21) 
#define PropertyChangeMask\t\t(1L<<22) 
#define ColormapChangeMask\t\t(1L<<23) 
#define OwnerGrabButtonMask\t\t(1L<<24) 

/* Event names.  Used in "type" field in XEvent structures.  Not to be
confused with event masks above.  They start from 2 because 0 and 1
are reserved in the protocol for errors and replies. */

#define KeyPress\t\t2
#define KeyRelease\t\t3
#define ButtonPress\t\t4
#define ButtonRelease\t\t5
#define MotionNotify\t\t6
#define EnterNotify\t\t7
#define LeaveNotify\t\t8
#define FocusIn\t\t\t9
#define FocusOut\t\t10
#define KeymapNotify\t\t11
#define Expose\t\t\t12
#define GraphicsExpose\t\t13
#define NoExpose\t\t14
#define VisibilityNotify\t15
#define CreateNotify\t\t16
#define DestroyNotify\t\t17
#define UnmapNotify\t\t18
#define MapNotify\t\t19
#define MapRequest\t\t20
#define ReparentNotify\t\t21
#define ConfigureNotify\t\t22
#define ConfigureRequest\t23
#define GravityNotify\t\t24
#define ResizeRequest\t\t25
#define CirculateNotify\t\t26
#define CirculateRequest\t27
#define PropertyNotify\t\t28
#define SelectionClear\t\t29
#define SelectionRequest\t30
#define SelectionNotify\t\t31
#define ColormapNotify\t\t32
#define ClientMessage\t\t33
#define MappingNotify\t\t34
#define LASTEvent\t\t35\t/* must be bigger than any event # */


/* Key masks. Used as modifiers to GrabButton and GrabKey, results of QueryPointer,
   state in various key-, mouse-, and button-related events. */

#define ShiftMask\t\t(1<<0)
#define LockMask\t\t(1<<1)
#define ControlMask\t\t(1<<2)
#define Mod1Mask\t\t(1<<3)
#define Mod2Mask\t\t(1<<4)
#define Mod3Mask\t\t(1<<5)
#define Mod4Mask\t\t(1<<6)
#define Mod5Mask\t\t(1<<7)

/* modifier names.  Used to build a SetModifierMapping request or
   to read a GetModifierMapping request.  These correspond to the
   masks defined above. */
#define ShiftMapIndex\t\t0
#define LockMapIndex\t\t1
#define ControlMapIndex\t\t2
#define Mod1MapIndex\t\t3
#define Mod2MapIndex\t\t4
#define Mod3MapIndex\t\t5
#define Mod4MapIndex\t\t6
#define Mod5MapIndex\t\t7


/* button masks.  Used in same manner as Key masks above. Not to be confused
   with button names below. */

#define Button1Mask\t\t(1<<8)
#define Button2Mask\t\t(1<<9)
#define Button3Mask\t\t(1<<10)
#define Button4Mask\t\t(1<<11)
#define Button5Mask\t\t(1<<12)

#define AnyModifier\t\t(1<<15)  /* used in GrabButton, GrabKey */


/* button names. Used as arguments to GrabButton and as detail in ButtonPress
   and ButtonRelease events.  Not to be confused with button masks above.
   Note that 0 is already defined above as "AnyButton".  */

#define Button1\t\t\t1
#define Button2\t\t\t2
#define Button3\t\t\t3
#define Button4\t\t\t4
#define Button5\t\t\t5

/* Notify modes */

#define NotifyNormal\t\t0
#define NotifyGrab\t\t1
#define NotifyUngrab\t\t2
#define NotifyWhileGrabbed\t3

#define NotifyHint\t\t1\t/* for MotionNotify events */
\t\t       
/* Notify detail */

#define NotifyAncestor\t\t0
#define NotifyVirtual\t\t1
#define NotifyInferior\t\t2
#define NotifyNonlinear\t\t3
#define NotifyNonlinearVirtual\t4
#define NotifyPointer\t\t5
#define NotifyPointerRoot\t6
#define NotifyDetailNone\t7

/* Visibility notify */

#define VisibilityUnobscured\t\t0
#define VisibilityPartiallyObscured\t1
#define VisibilityFullyObscured\t\t2

/* Circulation request */

#define PlaceOnTop\t\t0
#define PlaceOnBottom\t\t1

/* protocol families */

#define FamilyInternet\t\t0
#define FamilyDECnet\t\t1
#define FamilyChaos\t\t2

/* Property notification */

#define PropertyNewValue\t0
#define PropertyDelete\t\t1

/* Color Map notification */

#define ColormapUninstalled\t0
#define ColormapInstalled\t1

/* GrabPointer, GrabButton, GrabKeyboard, GrabKey Modes */

#define GrabModeSync\t\t0
#define GrabModeAsync\t\t1

/* GrabPointer, GrabKeyboard reply status */

#define GrabSuccess\t\t0
#define AlreadyGrabbed\t\t1
#define GrabInvalidTime\t\t2
#define GrabNotViewable\t\t3
#define GrabFrozen\t\t4

/* AllowEvents modes */

#define AsyncPointer\t\t0
#define SyncPointer\t\t1
#define ReplayPointer\t\t2
#define AsyncKeyboard\t\t3
#define SyncKeyboard\t\t4
#define ReplayKeyboard\t\t5
#define AsyncBoth\t\t6
#define SyncBoth\t\t7

/* Used in SetInputFocus, GetInputFocus */

#define RevertToNone\t\t(int)None
#define RevertToPointerRoot\t(int)PointerRoot
#define RevertToParent\t\t2

/*****************************************************************
 * ERROR CODES 
 *****************************************************************/

#define Success\t\t   0\t/* everything\'s okay */
#define BadRequest\t   1\t/* bad request code */
#define BadValue\t   2\t/* int parameter out of range */
#define BadWindow\t   3\t/* parameter not a Window */
#define BadPixmap\t   4\t/* parameter not a Pixmap */
#define BadAtom\t\t   5\t/* parameter not an Atom */
#define BadCursor\t   6\t/* parameter not a Cursor */
#define BadFont\t\t   7\t/* parameter not a Font */
#define BadMatch\t   8\t/* parameter mismatch */
#define BadDrawable\t   9\t/* parameter not a Pixmap or Window */
#define BadAccess\t  10\t/* depending on context:
\t\t\t\t - key/button already grabbed
\t\t\t\t - attempt to free an illegal 
\t\t\t\t   cmap entry 
\t\t\t\t- attempt to store into a read-only 
\t\t\t\t   color map entry.
 \t\t\t\t- attempt to modify the access control
\t\t\t\t   list from other than the local host.
\t\t\t\t*/
#define BadAlloc\t  11\t/* insufficient resources */
#define BadColor\t  12\t/* no such colormap */
#define BadGC\t\t  13\t/* parameter not a GC */
#define BadIDChoice\t  14\t/* choice not in range or already used */
#define BadName\t\t  15\t/* font or color name doesn\'t exist */
#define BadLength\t  16\t/* Request length incorrect */
#define BadImplementation 17\t/* server is defective */

#define FirstExtensionError\t128
#define LastExtensionError\t255

/*****************************************************************
 * WINDOW DEFINITIONS 
 *****************************************************************/

/* Window classes used by CreateWindow */
/* Note that CopyFromParent is already defined as 0 above */

#define InputOutput\t\t1
#define InputOnly\t\t2

/* Window attributes for CreateWindow and ChangeWindowAttributes */

#define CWBackPixmap\t\t(1L<<0)
#define CWBackPixel\t\t(1L<<1)
#define CWBorderPixmap\t\t(1L<<2)
#define CWBorderPixel           (1L<<3)
#define CWBitGravity\t\t(1L<<4)
#define CWWinGravity\t\t(1L<<5)
#define CWBackingStore          (1L<<6)
#define CWBackingPlanes\t        (1L<<7)
#define CWBackingPixel\t        (1L<<8)
#define CWOverrideRedirect\t(1L<<9)
#define CWSaveUnder\t\t(1L<<10)
#define CWEventMask\t\t(1L<<11)
#define CWDontPropagate\t        (1L<<12)
#define CWColormap\t\t(1L<<13)
#define CWCursor\t        (1L<<14)

/* ConfigureWindow structure */

#define CWX\t\t\t(1<<0)
#define CWY\t\t\t(1<<1)
#define CWWidth\t\t\t(1<<2)
#define CWHeight\t\t(1<<3)
#define CWBorderWidth\t\t(1<<4)
#define CWSibling\t\t(1<<5)
#define CWStackMode\t\t(1<<6)


/* Bit Gravity */

#define ForgetGravity\t\t0
#define NorthWestGravity\t1
#define NorthGravity\t\t2
#define NorthEastGravity\t3
#define WestGravity\t\t4
#define CenterGravity\t\t5
#define EastGravity\t\t6
#define SouthWestGravity\t7
#define SouthGravity\t\t8
#define SouthEastGravity\t9
#define StaticGravity\t\t10

/* Window gravity + bit gravity above */

#define UnmapGravity\t\t0

/* Used in CreateWindow for backing-store hint */

#define NotUseful               0
#define WhenMapped              1
#define Always                  2

/* Used in GetWindowAttributes reply */

#define IsUnmapped\t\t0
#define IsUnviewable\t\t1
#define IsViewable\t\t2

/* Used in ChangeSaveSet */

#define SetModeInsert           0
#define SetModeDelete           1

/* Used in ChangeCloseDownMode */

#define DestroyAll              0
#define RetainPermanent         1
#define RetainTemporary         2

/* Window stacking method (in configureWindow) */

#define Above                   0
#define Below                   1
#define TopIf                   2
#define BottomIf                3
#define Opposite                4

/* Circulation direction */

#define RaiseLowest             0
#define LowerHighest            1

/* Property modes */

#define PropModeReplace         0
#define PropModePrepend         1
#define PropModeAppend          2

/*****************************************************************
 * GRAPHICS DEFINITIONS
 *****************************************************************/

/* graphics functions, as in GC.alu */

#define\tGXclear\t\t\t0x0\t\t/* 0 */
#define GXand\t\t\t0x1\t\t/* src AND dst */
#define GXandReverse\t\t0x2\t\t/* src AND NOT dst */
#define GXcopy\t\t\t0x3\t\t/* src */
#define GXandInverted\t\t0x4\t\t/* NOT src AND dst */
#define\tGXnoop\t\t\t0x5\t\t/* dst */
#define GXxor\t\t\t0x6\t\t/* src XOR dst */
#define GXor\t\t\t0x7\t\t/* src OR dst */
#define GXnor\t\t\t0x8\t\t/* NOT src AND NOT dst */
#define GXequiv\t\t\t0x9\t\t/* NOT src XOR dst */
#define GXinvert\t\t0xa\t\t/* NOT dst */
#define GXorReverse\t\t0xb\t\t/* src OR NOT dst */
#define GXcopyInverted\t\t0xc\t\t/* NOT src */
#define GXorInverted\t\t0xd\t\t/* NOT src OR dst */
#define GXnand\t\t\t0xe\t\t/* NOT src OR NOT dst */
#define GXset\t\t\t0xf\t\t/* 1 */

/* LineStyle */

#define LineSolid\t\t0
#define LineOnOffDash\t\t1
#define LineDoubleDash\t\t2

/* capStyle */

#define CapNotLast\t\t0
#define CapButt\t\t\t1
#define CapRound\t\t2
#define CapProjecting\t\t3

/* joinStyle */

#define JoinMiter\t\t0
#define JoinRound\t\t1
#define JoinBevel\t\t2

/* fillStyle */

#define FillSolid\t\t0
#define FillTiled\t\t1
#define FillStippled\t\t2
#define FillOpaqueStippled\t3

/* fillRule */

#define EvenOddRule\t\t0
#define WindingRule\t\t1

/* subwindow mode */

#define ClipByChildren\t\t0
#define IncludeInferiors\t1

/* SetClipRectangles ordering */

#define Unsorted\t\t0
#define YSorted\t\t\t1
#define YXSorted\t\t2
#define YXBanded\t\t3

/* CoordinateMode for drawing routines */

#define CoordModeOrigin\t\t0\t/* relative to the origin */
#define CoordModePrevious       1\t/* relative to previous point */

/* Polygon shapes */

#define Complex\t\t\t0\t/* paths may intersect */
#define Nonconvex\t\t1\t/* no paths intersect, but not convex */
#define Convex\t\t\t2\t/* wholly convex */

/* Arc modes for PolyFillArc */

#define ArcChord\t\t0\t/* join endpoints of arc */
#define ArcPieSlice\t\t1\t/* join endpoints to center of arc */

/* GC components: masks used in CreateGC, CopyGC, ChangeGC, OR\'ed into
   GC.stateChanges */

#define GCFunction              (1L<<0)
#define GCPlaneMask             (1L<<1)
#define GCForeground            (1L<<2)
#define GCBackground            (1L<<3)
#define GCLineWidth             (1L<<4)
#define GCLineStyle             (1L<<5)
#define GCCapStyle              (1L<<6)
#define GCJoinStyle\t\t(1L<<7)
#define GCFillStyle\t\t(1L<<8)
#define GCFillRule\t\t(1L<<9) 
#define GCTile\t\t\t(1L<<10)
#define GCStipple\t\t(1L<<11)
#define GCTileStipXOrigin\t(1L<<12)
#define GCTileStipYOrigin\t(1L<<13)
#define GCFont \t\t\t(1L<<14)
#define GCSubwindowMode\t\t(1L<<15)
#define GCGraphicsExposures     (1L<<16)
#define GCClipXOrigin\t\t(1L<<17)
#define GCClipYOrigin\t\t(1L<<18)
#define GCClipMask\t\t(1L<<19)
#define GCDashOffset\t\t(1L<<20)
#define GCDashList\t\t(1L<<21)
#define GCArcMode\t\t(1L<<22)

#define GCLastBit\t\t22
/*****************************************************************
 * FONTS 
 *****************************************************************/

/* used in QueryFont -- draw direction */

#define FontLeftToRight\t\t0
#define FontRightToLeft\t\t1

#define FontChange\t\t255

/*****************************************************************
 *  IMAGING 
 *****************************************************************/

/* ImageFormat -- PutImage, GetImage */

#define XYBitmap\t\t0\t/* depth 1, XYFormat */
#define XYPixmap\t\t1\t/* depth == drawable depth */
#define ZPixmap\t\t\t2\t/* depth == drawable depth */

/*****************************************************************
 *  COLOR MAP STUFF 
 *****************************************************************/

/* For CreateColormap */

#define AllocNone\t\t0\t/* create map with no entries */
#define AllocAll\t\t1\t/* allocate entire map writeable */


/* Flags used in StoreNamedColor, StoreColors */

#define DoRed\t\t\t(1<<0)
#define DoGreen\t\t\t(1<<1)
#define DoBlue\t\t\t(1<<2)

/*****************************************************************
 * CURSOR STUFF
 *****************************************************************/

/* QueryBestSize Class */

#define CursorShape\t\t0\t/* largest size that can be displayed */
#define TileShape\t\t1\t/* size tiled fastest */
#define StippleShape\t\t2\t/* size stippled fastest */

/***************************************************************** 
 * KEYBOARD/POINTER STUFF
 *****************************************************************/

#define AutoRepeatModeOff\t0
#define AutoRepeatModeOn\t1
#define AutoRepeatModeDefault\t2

#define LedModeOff\t\t0
#define LedModeOn\t\t1

/* masks for ChangeKeyboardControl */

#define KBKeyClickPercent\t(1L<<0)
#define KBBellPercent\t\t(1L<<1)
#define KBBellPitch\t\t(1L<<2)
#define KBBellDuration\t\t(1L<<3)
#define KBLed\t\t\t(1L<<4)
#define KBLedMode\t\t(1L<<5)
#define KBKey\t\t\t(1L<<6)
#define KBAutoRepeatMode\t(1L<<7)

#define MappingSuccess     \t0
#define MappingBusy        \t1
#define MappingFailed\t\t2

#define MappingModifier\t\t0
#define MappingKeyboard\t\t1
#define MappingPointer\t\t2

/*****************************************************************
 * SCREEN SAVER STUFF 
 *****************************************************************/

#define DontPreferBlanking\t0
#define PreferBlanking\t\t1
#define DefaultBlanking\t\t2

#define DisableScreenSaver\t0
#define DisableScreenInterval\t0

#define DontAllowExposures\t0
#define AllowExposures\t\t1
#define DefaultExposures\t2

/* for ForceScreenSaver */

#define ScreenSaverReset 0
#define ScreenSaverActive 1

/*****************************************************************
 * HOSTS AND CONNECTIONS
 *****************************************************************/

/* for ChangeHosts */

#define HostInsert\t\t0
#define HostDelete\t\t1

/* for ChangeAccessControl */

#define EnableAccess\t\t1      
#define DisableAccess\t\t0

/* Display classes  used in opening the connection 
 * Note that the statically allocated ones are even numbered and the
 * dynamically changeable ones are odd numbered */

#define StaticGray\t\t0
#define GrayScale\t\t1
#define StaticColor\t\t2
#define PseudoColor\t\t3
#define TrueColor\t\t4
#define DirectColor\t\t5


/* Byte order  used in imageByteOrder and bitmapBitOrder */

#define LSBFirst\t\t0
#define MSBFirst\t\t1

#endif /* X_H */'
		OUTPUT - $'                  
/*
 *\t$XConsortium: X.h,v 1.66 88/09/06 15:55:56 jim Exp $
 */

/* Definitions for the X window system likely to be used by applications */

#ifndef X_H
#define X_H

/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
#define X_PROTOCOL\t11\t\t/* current protocol version */
#define X_PROTOCOL_REVISION 0\t\t/* current minor version */

/* Resources */

typedef unsigned long XID;

typedef XID Window;
typedef XID Drawable;
typedef XID Font;
typedef XID Pixmap;
typedef XID Cursor;
typedef XID Colormap;
typedef XID GContext;
typedef XID KeySym;

typedef unsigned long Mask;

typedef unsigned long Atom;

typedef unsigned long VisualID;

typedef unsigned long Time;

typedef unsigned char KeyCode;

/*****************************************************************
 * RESERVED RESOURCE AND CONSTANT DEFINITIONS
 *****************************************************************/

#define None                 0L\t/* universal null resource or null atom */

#define ParentRelative       1L\t/* background pixmap in CreateWindow
\t\t\t\t    and ChangeWindowAttributes */

#define CopyFromParent       0L\t/* border pixmap in CreateWindow
\t\t\t\t       and ChangeWindowAttributes
\t\t\t\t   special VisualID and special window
\t\t\t\t       class passed to CreateWindow */

#define PointerWindow        0L\t/* destination window in SendEvent */
#define InputFocus           1L\t/* destination window in SendEvent */

#define PointerRoot          1L\t/* focus window in SetInputFocus */

#define AnyPropertyType      0L\t/* special Atom, passed to GetProperty */

#define AnyKey\t\t     0L\t/* special Key Code, passed to GrabKey */

#define AnyButton            0L\t/* special Button Code, passed to GrabButton */

#define AllTemporary         0L\t/* special Resource ID passed to KillClient */

#define CurrentTime          0L\t/* special Time */

#define NoSymbol\t     0L\t/* special KeySym */

/***************************************************************** 
 * EVENT DEFINITIONS 
 *****************************************************************/

/* Input Event Masks. Used as event-mask window attribute and as arguments
   to Grab requests.  Not to be confused with event names.  */

#define NoEventMask\t\t\t0L
#define KeyPressMask\t\t\t(1L<<0)  
#define KeyReleaseMask\t\t\t(1L<<1)  
#define ButtonPressMask\t\t\t(1L<<2)  
#define ButtonReleaseMask\t\t(1L<<3)  
#define EnterWindowMask\t\t\t(1L<<4)  
#define LeaveWindowMask\t\t\t(1L<<5)  
#define PointerMotionMask\t\t(1L<<6)  
#define PointerMotionHintMask\t\t(1L<<7)  
#define Button1MotionMask\t\t(1L<<8)  
#define Button2MotionMask\t\t(1L<<9)  
#define Button3MotionMask\t\t(1L<<10) 
#define Button4MotionMask\t\t(1L<<11) 
#define Button5MotionMask\t\t(1L<<12) 
#define ButtonMotionMask\t\t(1L<<13) 
#define KeymapStateMask\t\t\t(1L<<14)
#define ExposureMask\t\t\t(1L<<15) 
#define VisibilityChangeMask\t\t(1L<<16) 
#define StructureNotifyMask\t\t(1L<<17) 
#define ResizeRedirectMask\t\t(1L<<18) 
#define SubstructureNotifyMask\t\t(1L<<19) 
#define SubstructureRedirectMask\t(1L<<20) 
#define FocusChangeMask\t\t\t(1L<<21) 
#define PropertyChangeMask\t\t(1L<<22) 
#define ColormapChangeMask\t\t(1L<<23) 
#define OwnerGrabButtonMask\t\t(1L<<24) 

/* Event names.  Used in "type" field in XEvent structures.  Not to be
confused with event masks above.  They start from 2 because 0 and 1
are reserved in the protocol for errors and replies. */

#define KeyPress\t\t2
#define KeyRelease\t\t3
#define ButtonPress\t\t4
#define ButtonRelease\t\t5
#define MotionNotify\t\t6
#define EnterNotify\t\t7
#define LeaveNotify\t\t8
#define FocusIn\t\t\t9
#define FocusOut\t\t10
#define KeymapNotify\t\t11
#define Expose\t\t\t12
#define GraphicsExpose\t\t13
#define NoExpose\t\t14
#define VisibilityNotify\t15
#define CreateNotify\t\t16
#define DestroyNotify\t\t17
#define UnmapNotify\t\t18
#define MapNotify\t\t19
#define MapRequest\t\t20
#define ReparentNotify\t\t21
#define ConfigureNotify\t\t22
#define ConfigureRequest\t23
#define GravityNotify\t\t24
#define ResizeRequest\t\t25
#define CirculateNotify\t\t26
#define CirculateRequest\t27
#define PropertyNotify\t\t28
#define SelectionClear\t\t29
#define SelectionRequest\t30
#define SelectionNotify\t\t31
#define ColormapNotify\t\t32
#define ClientMessage\t\t33
#define MappingNotify\t\t34
#define LASTEvent\t\t35\t/* must be bigger than any event # */


/* Key masks. Used as modifiers to GrabButton and GrabKey, results of QueryPointer,
   state in various key-, mouse-, and button-related events. */

#define ShiftMask\t\t(1<<0)
#define LockMask\t\t(1<<1)
#define ControlMask\t\t(1<<2)
#define Mod1Mask\t\t(1<<3)
#define Mod2Mask\t\t(1<<4)
#define Mod3Mask\t\t(1<<5)
#define Mod4Mask\t\t(1<<6)
#define Mod5Mask\t\t(1<<7)

/* modifier names.  Used to build a SetModifierMapping request or
   to read a GetModifierMapping request.  These correspond to the
   masks defined above. */
#define ShiftMapIndex\t\t0
#define LockMapIndex\t\t1
#define ControlMapIndex\t\t2
#define Mod1MapIndex\t\t3
#define Mod2MapIndex\t\t4
#define Mod3MapIndex\t\t5
#define Mod4MapIndex\t\t6
#define Mod5MapIndex\t\t7


/* button masks.  Used in same manner as Key masks above. Not to be confused
   with button names below. */

#define Button1Mask\t\t(1<<8)
#define Button2Mask\t\t(1<<9)
#define Button3Mask\t\t(1<<10)
#define Button4Mask\t\t(1<<11)
#define Button5Mask\t\t(1<<12)

#define AnyModifier\t\t(1<<15)  /* used in GrabButton, GrabKey */


/* button names. Used as arguments to GrabButton and as detail in ButtonPress
   and ButtonRelease events.  Not to be confused with button masks above.
   Note that 0 is already defined above as "AnyButton".  */

#define Button1\t\t\t1
#define Button2\t\t\t2
#define Button3\t\t\t3
#define Button4\t\t\t4
#define Button5\t\t\t5

/* Notify modes */

#define NotifyNormal\t\t0
#define NotifyGrab\t\t1
#define NotifyUngrab\t\t2
#define NotifyWhileGrabbed\t3

#define NotifyHint\t\t1\t/* for MotionNotify events */
\t\t       
/* Notify detail */

#define NotifyAncestor\t\t0
#define NotifyVirtual\t\t1
#define NotifyInferior\t\t2
#define NotifyNonlinear\t\t3
#define NotifyNonlinearVirtual\t4
#define NotifyPointer\t\t5
#define NotifyPointerRoot\t6
#define NotifyDetailNone\t7

/* Visibility notify */

#define VisibilityUnobscured\t\t0
#define VisibilityPartiallyObscured\t1
#define VisibilityFullyObscured\t\t2

/* Circulation request */

#define PlaceOnTop\t\t0
#define PlaceOnBottom\t\t1

/* protocol families */

#define FamilyInternet\t\t0
#define FamilyDECnet\t\t1
#define FamilyChaos\t\t2

/* Property notification */

#define PropertyNewValue\t0
#define PropertyDelete\t\t1

/* Color Map notification */

#define ColormapUninstalled\t0
#define ColormapInstalled\t1

/* GrabPointer, GrabButton, GrabKeyboard, GrabKey Modes */

#define GrabModeSync\t\t0
#define GrabModeAsync\t\t1

/* GrabPointer, GrabKeyboard reply status */

#define GrabSuccess\t\t0
#define AlreadyGrabbed\t\t1
#define GrabInvalidTime\t\t2
#define GrabNotViewable\t\t3
#define GrabFrozen\t\t4

/* AllowEvents modes */

#define AsyncPointer\t\t0
#define SyncPointer\t\t1
#define ReplayPointer\t\t2
#define AsyncKeyboard\t\t3
#define SyncKeyboard\t\t4
#define ReplayKeyboard\t\t5
#define AsyncBoth\t\t6
#define SyncBoth\t\t7

/* Used in SetInputFocus, GetInputFocus */

#define RevertToNone\t\t(int)None
#define RevertToPointerRoot\t(int)PointerRoot
#define RevertToParent\t\t2

/*****************************************************************
 * ERROR CODES 
 *****************************************************************/

#define Success\t\t   0\t/* everything\'s okay */
#define BadRequest\t   1\t/* bad request code */
#define BadValue\t   2\t/* int parameter out of range */
#define BadWindow\t   3\t/* parameter not a Window */
#define BadPixmap\t   4\t/* parameter not a Pixmap */
#define BadAtom\t\t   5\t/* parameter not an Atom */
#define BadCursor\t   6\t/* parameter not a Cursor */
#define BadFont\t\t   7\t/* parameter not a Font */
#define BadMatch\t   8\t/* parameter mismatch */
#define BadDrawable\t   9\t/* parameter not a Pixmap or Window */
#define BadAccess\t  10\t/* depending on context:
\t\t\t\t - key/button already grabbed
\t\t\t\t - attempt to free an illegal 
\t\t\t\t   cmap entry 
\t\t\t\t- attempt to store into a read-only 
\t\t\t\t   color map entry.
 \t\t\t\t- attempt to modify the access control
\t\t\t\t   list from other than the local host.
\t\t\t\t*/
#define BadAlloc\t  11\t/* insufficient resources */
#define BadColor\t  12\t/* no such colormap */
#define BadGC\t\t  13\t/* parameter not a GC */
#define BadIDChoice\t  14\t/* choice not in range or already used */
#define BadName\t\t  15\t/* font or color name doesn\'t exist */
#define BadLength\t  16\t/* Request length incorrect */
#define BadImplementation 17\t/* server is defective */

#define FirstExtensionError\t128
#define LastExtensionError\t255

/*****************************************************************
 * WINDOW DEFINITIONS 
 *****************************************************************/

/* Window classes used by CreateWindow */
/* Note that CopyFromParent is already defined as 0 above */

#define InputOutput\t\t1
#define InputOnly\t\t2

/* Window attributes for CreateWindow and ChangeWindowAttributes */

#define CWBackPixmap\t\t(1L<<0)
#define CWBackPixel\t\t(1L<<1)
#define CWBorderPixmap\t\t(1L<<2)
#define CWBorderPixel           (1L<<3)
#define CWBitGravity\t\t(1L<<4)
#define CWWinGravity\t\t(1L<<5)
#define CWBackingStore          (1L<<6)
#define CWBackingPlanes\t        (1L<<7)
#define CWBackingPixel\t        (1L<<8)
#define CWOverrideRedirect\t(1L<<9)
#define CWSaveUnder\t\t(1L<<10)
#define CWEventMask\t\t(1L<<11)
#define CWDontPropagate\t        (1L<<12)
#define CWColormap\t\t(1L<<13)
#define CWCursor\t        (1L<<14)

/* ConfigureWindow structure */

#define CWX\t\t\t(1<<0)
#define CWY\t\t\t(1<<1)
#define CWWidth\t\t\t(1<<2)
#define CWHeight\t\t(1<<3)
#define CWBorderWidth\t\t(1<<4)
#define CWSibling\t\t(1<<5)
#define CWStackMode\t\t(1<<6)


/* Bit Gravity */

#define ForgetGravity\t\t0
#define NorthWestGravity\t1
#define NorthGravity\t\t2
#define NorthEastGravity\t3
#define WestGravity\t\t4
#define CenterGravity\t\t5
#define EastGravity\t\t6
#define SouthWestGravity\t7
#define SouthGravity\t\t8
#define SouthEastGravity\t9
#define StaticGravity\t\t10

/* Window gravity + bit gravity above */

#define UnmapGravity\t\t0

/* Used in CreateWindow for backing-store hint */

#define NotUseful               0
#define WhenMapped              1
#define Always                  2

/* Used in GetWindowAttributes reply */

#define IsUnmapped\t\t0
#define IsUnviewable\t\t1
#define IsViewable\t\t2

/* Used in ChangeSaveSet */

#define SetModeInsert           0
#define SetModeDelete           1

/* Used in ChangeCloseDownMode */

#define DestroyAll              0
#define RetainPermanent         1
#define RetainTemporary         2

/* Window stacking method (in configureWindow) */

#define Above                   0
#define Below                   1
#define TopIf                   2
#define BottomIf                3
#define Opposite                4

/* Circulation direction */

#define RaiseLowest             0
#define LowerHighest            1

/* Property modes */

#define PropModeReplace         0
#define PropModePrepend         1
#define PropModeAppend          2

/*****************************************************************
 * GRAPHICS DEFINITIONS
 *****************************************************************/

/* graphics functions, as in GC.alu */

#define\tGXclear\t\t\t0x0\t\t/* 0 */
#define GXand\t\t\t0x1\t\t/* src AND dst */
#define GXandReverse\t\t0x2\t\t/* src AND NOT dst */
#define GXcopy\t\t\t0x3\t\t/* src */
#define GXandInverted\t\t0x4\t\t/* NOT src AND dst */
#define\tGXnoop\t\t\t0x5\t\t/* dst */
#define GXxor\t\t\t0x6\t\t/* src XOR dst */
#define GXor\t\t\t0x7\t\t/* src OR dst */
#define GXnor\t\t\t0x8\t\t/* NOT src AND NOT dst */
#define GXequiv\t\t\t0x9\t\t/* NOT src XOR dst */
#define GXinvert\t\t0xa\t\t/* NOT dst */
#define GXorReverse\t\t0xb\t\t/* src OR NOT dst */
#define GXcopyInverted\t\t0xc\t\t/* NOT src */
#define GXorInverted\t\t0xd\t\t/* NOT src OR dst */
#define GXnand\t\t\t0xe\t\t/* NOT src OR NOT dst */
#define GXset\t\t\t0xf\t\t/* 1 */

/* LineStyle */

#define LineSolid\t\t0
#define LineOnOffDash\t\t1
#define LineDoubleDash\t\t2

/* capStyle */

#define CapNotLast\t\t0
#define CapButt\t\t\t1
#define CapRound\t\t2
#define CapProjecting\t\t3

/* joinStyle */

#define JoinMiter\t\t0
#define JoinRound\t\t1
#define JoinBevel\t\t2

/* fillStyle */

#define FillSolid\t\t0
#define FillTiled\t\t1
#define FillStippled\t\t2
#define FillOpaqueStippled\t3

/* fillRule */

#define EvenOddRule\t\t0
#define WindingRule\t\t1

/* subwindow mode */

#define ClipByChildren\t\t0
#define IncludeInferiors\t1

/* SetClipRectangles ordering */

#define Unsorted\t\t0
#define YSorted\t\t\t1
#define YXSorted\t\t2
#define YXBanded\t\t3

/* CoordinateMode for drawing routines */

#define CoordModeOrigin\t\t0\t/* relative to the origin */
#define CoordModePrevious       1\t/* relative to previous point */

/* Polygon shapes */

#define Complex\t\t\t0\t/* paths may intersect */
#define Nonconvex\t\t1\t/* no paths intersect, but not convex */
#define Convex\t\t\t2\t/* wholly convex */

/* Arc modes for PolyFillArc */

#define ArcChord\t\t0\t/* join endpoints of arc */
#define ArcPieSlice\t\t1\t/* join endpoints to center of arc */

/* GC components: masks used in CreateGC, CopyGC, ChangeGC, OR\'ed into
   GC.stateChanges */

#define GCFunction              (1L<<0)
#define GCPlaneMask             (1L<<1)
#define GCForeground            (1L<<2)
#define GCBackground            (1L<<3)
#define GCLineWidth             (1L<<4)
#define GCLineStyle             (1L<<5)
#define GCCapStyle              (1L<<6)
#define GCJoinStyle\t\t(1L<<7)
#define GCFillStyle\t\t(1L<<8)
#define GCFillRule\t\t(1L<<9) 
#define GCTile\t\t\t(1L<<10)
#define GCStipple\t\t(1L<<11)
#define GCTileStipXOrigin\t(1L<<12)
#define GCTileStipYOrigin\t(1L<<13)
#define GCFont \t\t\t(1L<<14)
#define GCSubwindowMode\t\t(1L<<15)
#define GCGraphicsExposures     (1L<<16)
#define GCClipXOrigin\t\t(1L<<17)
#define GCClipYOrigin\t\t(1L<<18)
#define GCClipMask\t\t(1L<<19)
#define GCDashOffset\t\t(1L<<20)
#define GCDashList\t\t(1L<<21)
#define GCArcMode\t\t(1L<<22)

#define GCLastBit\t\t22
/*****************************************************************
 * FONTS 
 *****************************************************************/

/* used in QueryFont -- draw direction */

#define FontLeftToRight\t\t0
#define FontRightToLeft\t\t1

#define FontChange\t\t255

/*****************************************************************
 *  IMAGING 
 *****************************************************************/

/* ImageFormat -- PutImage, GetImage */

#define XYBitmap\t\t0\t/* depth 1, XYFormat */
#define XYPixmap\t\t1\t/* depth == drawable depth */
#define ZPixmap\t\t\t2\t/* depth == drawable depth */

/*****************************************************************
 *  COLOR MAP STUFF 
 *****************************************************************/

/* For CreateColormap */

#define AllocNone\t\t0\t/* create map with no entries */
#define AllocAll\t\t1\t/* allocate entire map writeable */


/* Flags used in StoreNamedColor, StoreColors */

#define DoRed\t\t\t(1<<0)
#define DoGreen\t\t\t(1<<1)
#define DoBlue\t\t\t(1<<2)

/*****************************************************************
 * CURSOR STUFF
 *****************************************************************/

/* QueryBestSize Class */

#define CursorShape\t\t0\t/* largest size that can be displayed */
#define TileShape\t\t1\t/* size tiled fastest */
#define StippleShape\t\t2\t/* size stippled fastest */

/***************************************************************** 
 * KEYBOARD/POINTER STUFF
 *****************************************************************/

#define AutoRepeatModeOff\t0
#define AutoRepeatModeOn\t1
#define AutoRepeatModeDefault\t2

#define LedModeOff\t\t0
#define LedModeOn\t\t1

/* masks for ChangeKeyboardControl */

#define KBKeyClickPercent\t(1L<<0)
#define KBBellPercent\t\t(1L<<1)
#define KBBellPitch\t\t(1L<<2)
#define KBBellDuration\t\t(1L<<3)
#define KBLed\t\t\t(1L<<4)
#define KBLedMode\t\t(1L<<5)
#define KBKey\t\t\t(1L<<6)
#define KBAutoRepeatMode\t(1L<<7)

#define MappingSuccess     \t0
#define MappingBusy        \t1
#define MappingFailed\t\t2

#define MappingModifier\t\t0
#define MappingKeyboard\t\t1
#define MappingPointer\t\t2

/*****************************************************************
 * SCREEN SAVER STUFF 
 *****************************************************************/

#define DontPreferBlanking\t0
#define PreferBlanking\t\t1
#define DefaultBlanking\t\t2

#define DisableScreenSaver\t0
#define DisableScreenInterval\t0

#define DontAllowExposures\t0
#define AllowExposures\t\t1
#define DefaultExposures\t2

/* for ForceScreenSaver */

#define ScreenSaverReset 0
#define ScreenSaverActive 1

/*****************************************************************
 * HOSTS AND CONNECTIONS
 *****************************************************************/

/* for ChangeHosts */

#define HostInsert\t\t0
#define HostDelete\t\t1

/* for ChangeAccessControl */

#define EnableAccess\t\t1      
#define DisableAccess\t\t0

/* Display classes  used in opening the connection 
 * Note that the statically allocated ones are even numbered and the
 * dynamically changeable ones are odd numbered */

#define StaticGray\t\t0
#define GrayScale\t\t1
#define StaticColor\t\t2
#define PseudoColor\t\t3
#define TrueColor\t\t4
#define DirectColor\t\t5


/* Byte order  used in imageByteOrder and bitmapBitOrder */

#define LSBFirst\t\t0
#define MSBFirst\t\t1

#endif /* X_H */'
	EXEC -nh
		INPUT - $'#pragma prototyped
#include <ast.h>
# include "stdio.h"
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX BUFSIZ
# define output(c) putc(c,yyout)
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar==\'\\n\')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng; extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin = {stdin}, *yyout = {stdout};
extern int yylineno;
struct yysvf { 
\tstruct yywork *yystoff;
\tstruct yysvf *yyother;
\tint *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
/* 
 * lex.l - scanner for yeast announcements/specifications
 * 
 * Author:\tBalachander Krishnamurthy
 * \t\tDavid S. Rosenblum
 * \t\tAT&T Bell Laboratories
 * Date:\tMon Feb 27 22:40:42 1989
 *
 */

/*
 * Modification history:
 * \tMon Mar 12 13:38:46 1990 -- undef\'ing input to define own routine
 *      Tue Jun 26 14:14:24 1990 -- changed BLANKFREESTRING from * to +
 *
 * If any tokens are added here yeast.y has to change - :-(
 */

#undef\tinput\t /* defeat lex */

extern void savesb(char*);
extern void FixString(char[]);
extern void errmsg(char*);
extern char input(void);

char c;
#include <h/globals.h>
#include <yeast.h>

extern bool fParseFailed;
# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:
 ;
break;
case 2:
        {savesb("3dfile"); return(DDDFILECLASS);}
break;
case 3:
\t{savesb("addspec");return(ADDSPEC);}
break;
case 4:
\t{savesb("and"); return(AND);}
break;
case 5:
{savesb("announce"); return(ANNOUNCE);}
break;
case 6:
\t{savesb("at"); return(AT);}
break;
case 7:
\t{savesb("atime"); return(ATIME);}
break;
case 8:
{savesb("authattr"); return(AUTHATTR);}
break;
case 9:
\t{savesb("authobj"); return(AUTHOBJ);}
break;
case 10:
\t{savesb("between"); return(BETWEEN);}
break;
case 11:
\t{savesb("by"); return(BY);}
break;
case 12:
{savesb("capacity"); return(CAPACITY);}
break;
case 13:
\t{savesb("changed"); return(CHANGED);}
break;
case 14:
\t{savesb("count"); return(COUNT);}
break;
case 15:
\t{savesb("daily"); return(DAILY);}
break;
case 16:
\t{savesb("defattr"); return(DEFATTR);}
break;
case 17:
\t{savesb("defobj"); return(DEFOBJ);}
break;
case 18:
        {savesb("dir"); return(DIRCLASS);}
break;
case 19:
\t{savesb("do"); return(DO);}
break;
case 20:
{savesb("do_notify"); return(DO_NOTIFY);}
break;
case 21:
\t{savesb("do_rpc"); return(DO_RPC);}
break;
case 22:
\t{savesb("etime"); return(PETIME);}
break;
case 23:
\t{savesb("false"); return(FALSE);}
break;
case 24:
\t{savesb("fgspec"); return(FGSPEC);}
break;
case 25:
        {savesb("file"); return(FILECLASS);}
break;
case 26:
\t{savesb("filesys"); return(FILESYSTEMCLASS);}
break;
case 27:
\t{savesb("host"); return(HOSTCLASS);}
break;
case 28:
\t{savesb("in"); return(IN);}
break;
case 29:
\t{savesb("load"); return(LOAD);}
break;
case 30:
{savesb("where"); return(LOCATION);}
break;
case 31:
{savesb("loggedin"); return(LOGGEDIN);}
break;
case 32:
\t{savesb("lsspec"); return(LSSPEC);}
break;
case 33:
\t{savesb("mode"); return(MODE);}
break;
case 34:
\t{savesb("modgrp"); return(MODGRP);}
break;
case 35:
\t{savesb("monthly"); return(MONTHLY);}
break;
case 36:
{savesb("midnight"); return(MIDNIGHT);}
break;
case 37:
\t{savesb("mtime"); return(MTIME);}
break;
case 38:
\t{savesb("no"); return(NOON);}
break;
case 39:
\t{savesb("or"); return(OR);}
break;
case 40:
\t{savesb("owner"); return(OWNER);}
break;
case 41:
\t{savesb("process"); return(PROCESSCLASS);}
break;
case 42:
{savesb("readspec"); return(READSPEC);}
break;
case 43:
{savesb("regyeast"); return(REGYEAST);}
break;
case 44:
\t{savesb("repeat"); return(REPEAT);}
break;
case 45:
\t{savesb("rmattr"); return(RMATTR);}
break;
case 46:
\t{savesb("rmobj"); return(RMOBJ);}
break;
case 47:
\t{savesb("rmspec"); return(RMSPEC);}
break;
case 48:
\t{savesb("size"); return(SIZE);}
break;
case 49:
\t{savesb("status"); return(STATUS);}
break;
case 50:
\t{savesb("stime"); return(STIME);}
break;
case 51:
{savesb("suspspec"); return(SUSPSPEC);}
break;
case 52:
\t{savesb("then"); return(THEN);}
break;
case 53:
{savesb("timestamp"); return(TIMESTAMP);}
break;
case 54:
\t{savesb("today"); return(TODAY);}
break;
case 55:
{savesb("tomorrow"); return(TOMORROW);}
break;
case 56:
\t{savesb("true"); return(TRUE);}
break;
case 57:
\t{savesb("tty"); return(TTYCLASS);}
break;
case 58:
{savesb("unchanged"); return(UNCHANGED);}
break;
case 59:
\t{savesb("until"); return(UNTIL);}
break;
case 60:
\t{savesb("up"); return(UP);}
break;
case 61:
\t{savesb("user"); return(USERCLASS);}
break;
case 62:
\t{savesb("users"); return(USERS);}
break;
case 63:
\t{savesb("utime"); return(UTIME);}
break;
case 64:
\t{savesb("week"); return(WEEK);}
break;
case 65:
\t{savesb("weekly"); return(WEEKLY);}
break;
case 66:
\t{savesb("within"); return(WITHIN);}
break;
case 67:
\t{savesb("yearly"); return(YEARLY);}
break;
case 68:
  \t\treturn(NEWLINE);
break;
case 69:
\treturn (ASSIGN);
break;
case 70:
\treturn (COMMA);
break;
case 71:
\treturn (EQ);
break;
case 72:
\treturn (GE);
break;
case 73:
\treturn (GT);
break;
case 74:
\treturn (LE);
break;
case 75:
\treturn (LPAREN);
break;
case 76:
\treturn (LT);
break;
case 77:
\treturn (NE);
break;
case 78:
\treturn (RPAREN);
break;
case 79:
\treturn (PERCENT);
break;
case 80:
\treturn (LCURLY);
break;
case 81:
\treturn (RCURLY);
break;
case 82:
{savesb(yytext); return(HEX);}
break;
case 83:
\t{savesb(yytext); return(OCTAL);}
break;
case 84:
\t{savesb(yytext); return(INTEGER);}
break;
case 85:
{savesb(yytext); return(INTRANGE);}
break;
case 86:
{savesb(yytext); return(REAL);}
break;
case 87:
 { savesb(yytext); return(HOURSPEC); }
break;
case 88:
{ savesb(yytext); return(HOURMINSPEC); }
break;
case 89:
{ savesb(yytext); return(HOURMINSPEC); }
break;
case 90:
{
\t\tif((yylval.sb =  malloc (yyleng+1)) != NULL)
\t\t    strcpy(yylval.sb,yytext);
\t\treturn(GROUPLABEL);
\t    }
break;
case 91:
{
\t\tif((yylval.sb =  malloc (yyleng+1)) != NULL) {
\t\t    yytext[yyleng-1] = \'\\0\';
\t\t    strcpy(yylval.sb,yytext);
\t      }
\t\treturn(EVENTLABEL);
\t    }
break;
case 92:
{\t\t\t\t
\t\tif((yylval.sb =  malloc (yyleng+1)) != NULL)
\t\t    strcpy(yylval.sb,yytext);
\t\treturn(BLANKFREESTRING);
\t    }
break;
case 93:
{                       /* Quoted String Checker */ 
             if(yytext[yyleng-1] == \'\\\\\') 
                yymore() ;
             else {
               c = input() ;
               if (c == \'\\n\') {
                  unput(c) ;
                  errmsg("String constant flows over line
 ") ;
                  yytext[yyleng] = \'\\0\' ;
                  return(QUOTEDSTRING) ;
               }
               else {
                  yytext[yyleng++] = \'"\' ;
                  yytext[yyleng] = \'\\0\' ;
                  FixString(yytext) ;
                  if((yylval.sb = malloc (yyleng+1)) != NULL)
                     strcpy(yylval.sb,yytext) ;
                  return(QUOTEDSTRING) ;
               }
             }  
            }
break;
case 94:
\t    { 
\t        errmsg("unknown character in input
") ;
\t    }
break;
case -1:
break;
default:
fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */

/*
 * ====================================================================
 * savesb - saves string value of token
 * ====================================================================
 */
    
void
savesb(char* sb)
{
    if((yylval.sb = (char *)malloc(strlen(sb)+1)) != NULL) 
\tstrcpy(yylval.sb,sb) ;
}

/****
 * Routine FixString to delete the double quotes in the string 
 ****/
void
FixString(char yytext[]) 
{
 int i; 

 for (i=1 ; i < yyleng -1 ; i++)
     yytext[i-1] = yytext[i] ;
 yytext[i-1] = \'\\0\' ;
}

/*
 * ====================================================================
 * errmsg - Error routine for lex
 * ====================================================================
 */
void
errmsg(char* sb)
{
    sfprintf(sbMsg, "lexical error: %s", sb);
    fParseFailed = 1;
}

char
input(void)\t\t\t\t\t\t/* a la awk/gpr */
{
    register c;
    extern char *lexprog;

    if (yysptr > yysbuf)
\tc = U(*--yysptr);
    else
\tc = *lexprog++;
    return(c); 
}
int yyvstop[] = {
0,

92,
94,
0,

1,
92,
94,
0,

68,
0,

1,
94,
0,

92,
94,
0,

92,
93,
94,
0,

79,
92,
94,
0,

75,
94,
0,

78,
92,
94,
0,

92,
94,
0,

70,
94,
0,

92,
94,
0,

83,
84,
92,
94,
0,

84,
92,
94,
0,

84,
92,
94,
0,

76,
92,
94,
0,

69,
92,
94,
0,

73,
92,
94,
0,

94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

80,
92,
94,
0,

81,
92,
94,
0,

92,
0,

91,
92,
0,

77,
92,
0,

92,
93,
0,

93,
0,

91,
92,
93,
0,

90,
92,
0,

90,
92,
0,

90,
91,
92,
0,

86,
92,
0,

92,
0,

92,
0,

83,
84,
92,
0,

84,
92,
0,

91,
92,
0,

87,
92,
0,

92,
0,

92,
0,

74,
92,
0,

71,
92,
0,

72,
92,
0,

92,
0,

92,
0,

6,
92,
0,

92,
0,

92,
0,

11,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

19,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

28,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

39,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

60,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

85,
92,
0,

88,
92,
0,

87,
92,
0,

82,
92,
0,

92,
0,

92,
0,

4,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

18,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

57,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

89,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

25,
92,
0,

27,
92,
0,

29,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

33,
92,
0,

92,
0,

92,
0,

92,
0,

38,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

48,
92,
0,

92,
0,

92,
0,

92,
0,

52,
92,
0,

92,
0,

92,
0,

92,
0,

56,
92,
0,

92,
0,

92,
0,

61,
92,
0,

92,
0,

64,
92,
0,

92,
0,

92,
0,

89,
92,
0,

92,
0,

92,
0,

92,
0,

7,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

14,
92,
0,

15,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

22,
92,
0,

23,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

37,
92,
0,

40,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

46,
92,
0,

92,
0,

92,
0,

50,
92,
0,

92,
0,

92,
0,

54,
92,
0,

92,
0,

92,
0,

59,
92,
0,

62,
92,
0,

63,
92,
0,

92,
0,

92,
0,

92,
0,

2,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

17,
92,
0,

92,
0,

21,
92,
0,

24,
92,
0,

92,
0,

92,
0,

92,
0,

32,
92,
0,

92,
0,

34,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

44,
92,
0,

45,
92,
0,

47,
92,
0,

49,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

65,
92,
0,

66,
92,
0,

67,
92,
0,

3,
92,
0,

92,
0,

92,
0,

9,
92,
0,

10,
92,
0,

92,
0,

13,
92,
0,

16,
92,
0,

92,
0,

26,
92,
0,

92,
0,

92,
0,

92,
0,

35,
92,
0,

41,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

5,
92,
0,

8,
92,
0,

12,
92,
0,

92,
0,

30,
92,
0,

31,
92,
0,

36,
92,
0,

42,
92,
0,

43,
92,
0,

51,
92,
0,

92,
0,

55,
92,
0,

92,
0,

20,
92,
0,

53,
92,
0,

58,
92,
0,
0};
# define YYTYPE int
struct yywork { YYTYPE verify, advance; } yycrank[] = {
0,0,\t0,0,\t1,3,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t1,4,\t1,5,\t
0,0,\t0,0,\t0,0,\t0,0,\t
46,0,\t48,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t1,6,\t1,7,\t1,8,\t
0,0,\t0,0,\t1,9,\t0,0,\t
0,0,\t1,10,\t1,11,\t0,0,\t
1,12,\t1,13,\t0,0,\t1,14,\t
0,0,\t1,15,\t1,16,\t1,16,\t
1,17,\t1,16,\t1,16,\t1,16,\t
1,16,\t1,16,\t0,0,\t0,0,\t
4,0,\t1,18,\t1,19,\t1,20,\t
46,48,\t48,48,\t1,3,\t1,3,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t1,3,\t0,0,\t
0,0,\t1,3,\t4,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t4,0,\t0,0,\t
0,0,\t0,0,\t4,0,\t1,21,\t
0,0,\t0,0,\t1,22,\t1,23,\t
1,24,\t1,25,\t1,26,\t1,27,\t
0,0,\t1,28,\t1,29,\t0,0,\t
4,44,\t1,30,\t1,31,\t1,32,\t
1,33,\t1,34,\t0,0,\t1,35,\t
1,36,\t1,37,\t1,38,\t2,7,\t
1,39,\t0,0,\t1,40,\t2,9,\t
1,41,\t0,0,\t1,42,\t2,11,\t
0,0,\t9,0,\t2,13,\t0,0,\t
2,14,\t0,0,\t0,0,\t2,16,\t
2,16,\t2,17,\t2,16,\t2,16,\t
2,16,\t2,16,\t0,0,\t0,0,\t
4,0,\t11,0,\t2,18,\t2,19,\t
2,20,\t0,0,\t0,0,\t9,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t9,0,\t
0,0,\t0,0,\t0,0,\t9,0,\t
0,0,\t0,0,\t0,0,\t11,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t3,43,\t0,0,\t11,0,\t
0,0,\t9,44,\t0,0,\t11,0,\t
2,21,\t3,43,\t3,0,\t2,22,\t
2,23,\t2,24,\t2,25,\t2,26,\t
2,27,\t0,0,\t2,28,\t2,29,\t
7,0,\t11,44,\t2,30,\t2,31,\t
2,32,\t2,33,\t2,34,\t0,0,\t
2,35,\t2,36,\t2,37,\t2,38,\t
3,0,\t2,39,\t3,43,\t2,40,\t
0,0,\t2,41,\t0,0,\t2,42,\t
3,0,\t9,0,\t7,0,\t3,43,\t
3,0,\t0,0,\t0,0,\t0,0,\t
3,43,\t0,0,\t7,0,\t8,46,\t
0,0,\t0,0,\t7,0,\t0,0,\t
3,43,\t11,0,\t3,44,\t8,46,\t
8,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t3,43,\t3,43,\t0,0,\t
7,44,\t0,0,\t0,0,\t7,45,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t3,43,\t12,49,\t0,0,\t
3,43,\t0,0,\t8,47,\t0,0,\t
8,43,\t0,0,\t12,49,\t12,0,\t
14,0,\t0,0,\t8,47,\t0,0,\t
0,0,\t8,46,\t3,0,\t0,0,\t
0,0,\t0,0,\t8,46,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
7,0,\t0,0,\t8,46,\t0,0,\t
8,48,\t12,0,\t14,0,\t12,49,\t
0,0,\t0,0,\t0,0,\t8,46,\t
8,46,\t12,0,\t14,0,\t18,0,\t
12,50,\t12,0,\t14,0,\t0,0,\t
0,0,\t12,49,\t14,52,\t8,46,\t
15,0,\t0,0,\t8,46,\t0,0,\t
0,0,\t12,49,\t14,52,\t12,51,\t
14,44,\t0,0,\t0,0,\t0,0,\t
0,0,\t18,0,\t12,49,\t12,49,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t18,0,\t15,0,\t16,0,\t
0,0,\t18,0,\t12,49,\t0,0,\t
0,0,\t12,49,\t15,0,\t0,0,\t
19,0,\t0,0,\t15,0,\t15,53,\t
15,54,\t0,0,\t15,55,\t18,44,\t
0,0,\t0,0,\t18,61,\t12,0,\t
14,0,\t16,0,\t15,56,\t0,0,\t
15,57,\t0,0,\t0,0,\t17,0,\t
0,0,\t16,0,\t19,0,\t15,58,\t
0,0,\t16,0,\t16,53,\t16,54,\t
20,0,\t16,56,\t19,0,\t0,0,\t
0,0,\t0,0,\t19,0,\t0,0,\t
0,0,\t16,56,\t15,58,\t16,57,\t
0,0,\t17,0,\t0,0,\t18,0,\t
0,0,\t0,0,\t16,58,\t22,0,\t
19,44,\t17,0,\t20,0,\t19,62,\t
15,0,\t17,0,\t17,53,\t17,54,\t
0,0,\t17,56,\t20,0,\t0,0,\t
23,0,\t16,58,\t20,0,\t0,0,\t
0,0,\t17,56,\t0,0,\t17,57,\t
0,0,\t22,0,\t0,0,\t24,0,\t
0,0,\t0,0,\t17,58,\t16,0,\t
20,44,\t22,0,\t15,59,\t20,63,\t
0,0,\t22,0,\t23,0,\t0,0,\t
19,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t17,58,\t23,0,\t25,0,\t
26,0,\t24,0,\t23,0,\t22,44,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t24,0,\t0,0,\t17,0,\t
0,0,\t24,0,\t0,0,\t0,0,\t
23,44,\t17,60,\t0,0,\t28,0,\t
20,0,\t25,0,\t26,0,\t0,0,\t
27,0,\t0,0,\t0,0,\t24,44,\t
0,0,\t25,0,\t26,0,\t0,0,\t
0,0,\t25,0,\t26,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t22,0,\t
0,0,\t28,0,\t0,0,\t0,0,\t
0,0,\t22,64,\t27,0,\t25,44,\t
26,44,\t28,0,\t0,0,\t0,0,\t
23,0,\t28,0,\t27,0,\t22,65,\t
0,0,\t0,0,\t27,0,\t23,68,\t
0,0,\t22,66,\t22,67,\t24,0,\t
0,0,\t29,0,\t24,70,\t28,44,\t
0,0,\t0,0,\t0,0,\t0,0,\t
27,44,\t24,71,\t0,0,\t30,0,\t
0,0,\t0,0,\t0,0,\t23,69,\t
24,72,\t0,0,\t31,0,\t25,0,\t
26,0,\t0,0,\t25,73,\t29,0,\t
32,0,\t0,0,\t25,74,\t0,0,\t
0,0,\t0,0,\t25,75,\t29,0,\t
0,0,\t30,0,\t0,0,\t29,0,\t
25,76,\t0,0,\t0,0,\t28,0,\t
31,0,\t30,0,\t26,77,\t33,0,\t
27,0,\t30,0,\t32,0,\t27,78,\t
31,0,\t29,44,\t0,0,\t0,0,\t
31,0,\t27,79,\t32,0,\t27,80,\t
28,81,\t0,0,\t32,0,\t30,44,\t
0,0,\t0,0,\t34,0,\t0,0,\t
0,0,\t33,0,\t31,44,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
32,44,\t33,0,\t0,0,\t36,0,\t
0,0,\t33,0,\t0,0,\t0,0,\t
0,0,\t35,0,\t0,0,\t0,0,\t
34,0,\t29,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t33,44,\t
34,0,\t0,0,\t0,0,\t30,0,\t
34,0,\t36,0,\t0,0,\t0,0,\t
37,0,\t29,82,\t31,0,\t35,0,\t
0,0,\t36,0,\t40,0,\t0,0,\t
32,0,\t36,0,\t34,44,\t35,0,\t
30,83,\t31,85,\t0,0,\t35,0,\t
30,84,\t0,0,\t0,0,\t31,86,\t
0,0,\t38,0,\t37,0,\t36,44,\t
31,87,\t32,88,\t0,0,\t33,0,\t
40,0,\t35,44,\t37,0,\t0,0,\t
0,0,\t0,0,\t37,0,\t0,0,\t
40,0,\t0,0,\t0,0,\t0,0,\t
40,0,\t39,0,\t0,0,\t38,0,\t
0,0,\t0,0,\t34,0,\t33,89,\t
37,44,\t0,0,\t0,0,\t38,0,\t
33,90,\t0,0,\t40,44,\t38,0,\t
0,0,\t0,0,\t0,0,\t36,0,\t
0,0,\t0,0,\t0,0,\t39,0,\t
0,0,\t35,0,\t34,91,\t0,0,\t
0,0,\t38,44,\t36,94,\t39,0,\t
35,92,\t0,0,\t0,0,\t39,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
35,93,\t36,95,\t36,96,\t0,0,\t
37,0,\t41,0,\t42,0,\t0,0,\t
0,0,\t39,44,\t40,0,\t0,0,\t
0,0,\t0,0,\t37,97,\t37,98,\t
0,0,\t40,108,\t0,0,\t0,0,\t
0,0,\t37,99,\t0,0,\t0,0,\t
37,100,\t38,0,\t37,101,\t41,0,\t
42,0,\t43,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t41,0,\t
42,0,\t0,0,\t44,0,\t41,0,\t
42,0,\t38,102,\t0,0,\t38,103,\t
45,0,\t39,0,\t38,104,\t38,105,\t
0,0,\t0,0,\t0,0,\t43,0,\t
39,106,\t41,44,\t42,44,\t0,0,\t
39,107,\t0,0,\t0,0,\t43,0,\t
44,0,\t0,0,\t47,47,\t43,0,\t
0,0,\t0,0,\t45,0,\t0,0,\t
44,0,\t0,0,\t47,47,\t47,0,\t
44,0,\t0,0,\t45,0,\t0,0,\t
50,0,\t43,44,\t45,0,\t0,0,\t
0,0,\t49,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t44,44,\t0,0,\t
0,0,\t41,0,\t42,0,\t0,0,\t
45,44,\t47,47,\t0,0,\t47,0,\t
0,0,\t0,0,\t50,0,\t0,0,\t
0,0,\t47,47,\t0,0,\t49,0,\t
47,47,\t0,0,\t50,0,\t0,0,\t
0,0,\t47,47,\t50,0,\t49,0,\t
0,0,\t43,0,\t49,49,\t49,0,\t
0,0,\t47,47,\t51,0,\t0,0,\t
0,0,\t0,0,\t44,0,\t52,0,\t
50,51,\t0,0,\t47,47,\t47,47,\t
45,0,\t49,51,\t0,0,\t0,0,\t
0,0,\t0,0,\t53,0,\t0,0,\t
0,0,\t0,0,\t47,47,\t0,0,\t
51,0,\t47,47,\t0,0,\t0,0,\t
0,0,\t52,0,\t0,0,\t0,0,\t
51,0,\t0,0,\t0,0,\t51,49,\t
51,0,\t52,0,\t0,0,\t54,0,\t
53,0,\t52,0,\t0,0,\t0,0,\t
50,0,\t52,52,\t55,0,\t0,0,\t
53,0,\t49,0,\t51,51,\t0,0,\t
53,0,\t52,52,\t0,0,\t52,44,\t
53,109,\t0,0,\t0,0,\t0,0,\t
0,0,\t54,0,\t0,0,\t0,0,\t
53,109,\t0,0,\t53,44,\t0,0,\t
55,0,\t54,0,\t0,0,\t0,0,\t
0,0,\t54,0,\t56,0,\t0,0,\t
55,0,\t54,52,\t0,0,\t0,0,\t
55,0,\t55,53,\t55,54,\t0,0,\t
55,55,\t54,52,\t51,0,\t54,44,\t
0,0,\t0,0,\t0,0,\t52,0,\t
55,56,\t0,0,\t55,57,\t0,0,\t
56,0,\t57,0,\t0,0,\t0,0,\t
0,0,\t55,58,\t53,0,\t58,0,\t
56,0,\t0,0,\t0,0,\t0,0,\t
56,0,\t56,53,\t56,54,\t0,0,\t
56,56,\t0,0,\t0,0,\t0,0,\t
55,58,\t0,0,\t59,0,\t57,0,\t
56,56,\t0,0,\t56,57,\t54,0,\t
0,0,\t58,0,\t0,0,\t57,0,\t
0,0,\t56,58,\t55,0,\t57,0,\t
0,0,\t58,0,\t0,0,\t57,110,\t
0,0,\t58,0,\t60,0,\t0,0,\t
59,0,\t61,0,\t0,0,\t57,110,\t
56,58,\t57,44,\t0,0,\t0,0,\t
59,0,\t0,0,\t0,0,\t58,44,\t
59,0,\t0,0,\t0,0,\t0,0,\t
59,112,\t0,0,\t56,0,\t0,0,\t
60,0,\t62,0,\t63,0,\t61,0,\t
59,112,\t0,0,\t59,44,\t0,0,\t
60,0,\t0,0,\t58,111,\t61,0,\t
60,0,\t59,112,\t59,112,\t61,0,\t
0,0,\t0,0,\t64,0,\t0,0,\t
0,0,\t57,0,\t0,0,\t62,0,\t
63,0,\t65,0,\t60,44,\t58,0,\t
0,0,\t61,44,\t0,0,\t62,0,\t
63,0,\t0,0,\t66,0,\t62,0,\t
63,0,\t0,0,\t0,0,\t0,0,\t
64,0,\t0,0,\t59,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t65,0,\t
64,0,\t62,44,\t63,44,\t0,0,\t
64,0,\t67,0,\t0,0,\t65,0,\t
66,0,\t0,0,\t68,0,\t65,0,\t
0,0,\t0,0,\t60,0,\t0,0,\t
66,0,\t61,0,\t64,44,\t0,0,\t
66,0,\t69,0,\t60,113,\t0,0,\t
0,0,\t65,44,\t0,0,\t67,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
68,0,\t0,0,\t66,44,\t67,0,\t
0,0,\t62,0,\t63,0,\t67,0,\t
68,0,\t0,0,\t70,0,\t69,0,\t
68,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t71,0,\t0,0,\t69,0,\t
0,0,\t67,44,\t64,0,\t69,0,\t
0,0,\t0,0,\t68,44,\t0,0,\t
64,114,\t65,0,\t72,0,\t0,0,\t
70,0,\t73,0,\t0,0,\t65,115,\t
0,0,\t69,44,\t66,0,\t71,0,\t
70,0,\t0,0,\t0,0,\t0,0,\t
70,0,\t65,116,\t0,0,\t71,0,\t
0,0,\t66,117,\t0,0,\t71,0,\t
72,0,\t0,0,\t0,0,\t73,0,\t
0,0,\t67,0,\t70,44,\t0,0,\t
72,0,\t74,0,\t68,0,\t73,0,\t
72,0,\t71,44,\t0,0,\t73,0,\t
75,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t69,0,\t0,0,\t0,0,\t
0,0,\t76,0,\t72,44,\t67,118,\t
0,0,\t73,44,\t0,0,\t74,0,\t
68,119,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t75,0,\t74,0,\t
0,0,\t77,0,\t70,0,\t74,0,\t
78,0,\t0,0,\t75,0,\t76,0,\t
0,0,\t71,0,\t75,0,\t0,0,\t
71,121,\t0,0,\t0,0,\t76,0,\t
0,0,\t74,44,\t0,0,\t76,0,\t
70,120,\t0,0,\t72,0,\t77,0,\t
75,44,\t73,0,\t78,0,\t0,0,\t
79,0,\t80,0,\t0,0,\t77,0,\t
0,0,\t76,44,\t78,0,\t77,0,\t
73,123,\t0,0,\t78,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
81,0,\t72,122,\t0,0,\t0,0,\t
0,0,\t77,44,\t79,0,\t80,0,\t
78,44,\t74,0,\t0,0,\t0,0,\t
0,0,\t82,0,\t79,0,\t80,0,\t
75,0,\t74,124,\t79,0,\t80,0,\t
0,0,\t0,0,\t81,0,\t83,0,\t
0,0,\t76,0,\t76,126,\t0,0,\t
0,0,\t84,0,\t81,0,\t0,0,\t
79,44,\t80,44,\t81,0,\t82,0,\t
75,125,\t0,0,\t0,0,\t0,0,\t
85,0,\t77,0,\t86,0,\t82,0,\t
78,0,\t83,0,\t0,0,\t82,0,\t
81,44,\t0,0,\t0,0,\t84,0,\t
77,127,\t83,0,\t0,0,\t0,0,\t
0,0,\t83,0,\t78,128,\t84,0,\t
0,0,\t82,44,\t85,0,\t84,0,\t
86,0,\t0,0,\t0,0,\t0,0,\t
79,0,\t80,0,\t85,0,\t83,44,\t
86,0,\t0,0,\t85,0,\t0,0,\t
86,0,\t84,44,\t0,0,\t0,0,\t
0,0,\t87,0,\t0,0,\t80,130,\t
81,0,\t0,0,\t0,0,\t88,0,\t
85,44,\t79,129,\t86,44,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t82,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t87,0,\t
0,0,\t81,131,\t89,0,\t83,0,\t
0,0,\t88,0,\t83,132,\t87,0,\t
83,133,\t84,0,\t0,0,\t87,0,\t
83,134,\t88,0,\t0,0,\t90,0,\t
0,0,\t88,0,\t91,0,\t0,0,\t
85,0,\t92,0,\t86,0,\t0,0,\t
89,0,\t87,44,\t85,136,\t0,0,\t
86,137,\t0,0,\t84,135,\t88,44,\t
89,0,\t0,0,\t93,0,\t0,0,\t
89,0,\t90,0,\t86,138,\t0,0,\t
91,0,\t0,0,\t0,0,\t92,0,\t
0,0,\t90,0,\t0,0,\t0,0,\t
91,0,\t90,0,\t89,44,\t92,0,\t
91,0,\t94,0,\t0,0,\t92,0,\t
93,0,\t0,0,\t95,0,\t96,0,\t
0,0,\t87,0,\t0,0,\t90,44,\t
93,0,\t0,0,\t91,44,\t88,0,\t
93,0,\t92,44,\t0,0,\t0,0,\t
87,139,\t0,0,\t0,0,\t94,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
95,0,\t96,0,\t93,44,\t94,0,\t
88,140,\t0,0,\t89,0,\t94,0,\t
95,0,\t96,0,\t97,0,\t0,0,\t
95,0,\t96,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t90,0,\t
98,0,\t94,44,\t91,0,\t0,0,\t
0,0,\t92,0,\t95,44,\t96,44,\t
92,143,\t0,0,\t99,0,\t0,0,\t
97,0,\t0,0,\t92,144,\t90,141,\t
0,0,\t0,0,\t93,0,\t91,142,\t
97,0,\t93,146,\t98,0,\t92,145,\t
97,0,\t0,0,\t0,0,\t100,0,\t
0,0,\t101,0,\t98,0,\t0,0,\t
99,0,\t0,0,\t98,0,\t93,147,\t
0,0,\t94,0,\t97,44,\t93,148,\t
99,0,\t0,0,\t95,0,\t96,0,\t
99,0,\t95,150,\t0,0,\t0,0,\t
98,44,\t100,0,\t0,0,\t101,0,\t
0,0,\t95,151,\t0,0,\t0,0,\t
0,0,\t100,0,\t99,44,\t101,0,\t
0,0,\t100,0,\t0,0,\t101,0,\t
96,152,\t94,149,\t0,0,\t102,0,\t
103,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t104,0,\t97,0,\t100,44,\t
0,0,\t101,44,\t0,0,\t0,0,\t
0,0,\t97,153,\t0,0,\t0,0,\t
98,0,\t0,0,\t0,0,\t0,0,\t
105,0,\t102,0,\t103,0,\t0,0,\t
0,0,\t0,0,\t99,0,\t104,0,\t
0,0,\t102,0,\t103,0,\t98,154,\t
99,155,\t102,0,\t103,0,\t104,0,\t
0,0,\t106,0,\t0,0,\t104,0,\t
0,0,\t99,156,\t105,0,\t100,0,\t
0,0,\t101,0,\t0,0,\t102,44,\t
103,44,\t0,0,\t105,0,\t0,0,\t
107,0,\t104,44,\t105,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t106,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t108,0,\t100,157,\t106,0,\t
105,44,\t0,0,\t0,0,\t106,0,\t
101,158,\t0,0,\t107,0,\t0,0,\t
109,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t107,0,\t102,0,\t
103,0,\t106,44,\t107,0,\t108,0,\t
102,159,\t104,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t108,0,\t
104,161,\t0,0,\t109,0,\t108,0,\t
107,44,\t0,0,\t0,0,\t0,0,\t
105,0,\t102,160,\t109,0,\t110,0,\t
111,0,\t0,0,\t109,0,\t0,0,\t
0,0,\t108,44,\t109,109,\t105,162,\t
0,0,\t113,0,\t0,0,\t0,0,\t
0,0,\t106,0,\t109,109,\t0,0,\t
109,44,\t0,0,\t0,0,\t0,0,\t
106,163,\t110,0,\t111,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
107,0,\t110,0,\t111,0,\t113,0,\t
0,0,\t110,0,\t111,0,\t0,0,\t
0,0,\t110,110,\t0,0,\t113,0,\t
112,0,\t108,0,\t114,0,\t113,0,\t
108,165,\t110,110,\t0,0,\t110,44,\t
111,44,\t0,0,\t107,164,\t115,0,\t
109,0,\t0,0,\t110,166,\t0,0,\t
0,0,\t113,44,\t0,0,\t0,0,\t
0,0,\t0,0,\t112,0,\t0,0,\t
114,0,\t0,0,\t0,0,\t116,0,\t
0,0,\t110,166,\t112,0,\t0,0,\t
114,0,\t115,0,\t112,0,\t0,0,\t
114,0,\t0,0,\t112,112,\t0,0,\t
117,0,\t115,0,\t0,0,\t110,0,\t
111,0,\t115,0,\t112,112,\t0,0,\t
112,44,\t116,0,\t114,44,\t118,0,\t
0,0,\t113,0,\t0,0,\t112,112,\t
112,112,\t116,0,\t0,0,\t115,44,\t
119,0,\t116,0,\t117,0,\t0,0,\t
113,167,\t0,0,\t0,0,\t120,0,\t
0,0,\t0,0,\t117,0,\t0,0,\t
0,0,\t118,0,\t117,0,\t116,44,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t118,0,\t119,0,\t121,0,\t
112,0,\t118,0,\t114,0,\t0,0,\t
117,44,\t120,0,\t119,0,\t0,0,\t
122,0,\t0,0,\t119,0,\t115,0,\t
0,0,\t120,0,\t0,0,\t118,44,\t
0,0,\t120,0,\t0,0,\t0,0,\t
0,0,\t121,0,\t0,0,\t114,168,\t
119,44,\t0,0,\t0,0,\t116,0,\t
123,0,\t121,0,\t122,0,\t120,44,\t
0,0,\t121,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t122,0,\t0,0,\t
117,0,\t124,0,\t122,0,\t125,0,\t
116,169,\t0,0,\t0,0,\t121,44,\t
0,0,\t0,0,\t123,0,\t118,0,\t
126,0,\t0,0,\t0,0,\t117,170,\t
122,44,\t0,0,\t123,0,\t0,0,\t
119,0,\t118,171,\t123,0,\t124,0,\t
0,0,\t125,0,\t0,0,\t120,0,\t
0,0,\t0,0,\t120,173,\t124,0,\t
0,0,\t125,0,\t126,0,\t124,0,\t
123,44,\t125,0,\t0,0,\t0,0,\t
0,0,\t127,0,\t126,0,\t121,0,\t
128,0,\t119,172,\t126,0,\t0,0,\t
0,0,\t124,44,\t0,0,\t125,44,\t
122,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t121,174,\t
126,44,\t129,0,\t0,0,\t127,0,\t
0,0,\t0,0,\t128,0,\t0,0,\t
122,175,\t0,0,\t0,0,\t127,0,\t
123,0,\t0,0,\t128,0,\t127,0,\t
130,0,\t0,0,\t128,0,\t0,0,\t
0,0,\t131,0,\t0,0,\t129,0,\t
0,0,\t124,0,\t123,176,\t125,0,\t
124,177,\t127,44,\t0,0,\t129,0,\t
128,44,\t0,0,\t0,0,\t129,0,\t
126,0,\t0,0,\t130,0,\t0,0,\t
132,0,\t0,0,\t124,178,\t131,0,\t
0,0,\t133,0,\t130,0,\t0,0,\t
0,0,\t129,44,\t130,0,\t131,0,\t
126,179,\t0,0,\t0,0,\t131,0,\t
126,180,\t0,0,\t0,0,\t0,0,\t
134,0,\t135,0,\t132,0,\t0,0,\t
130,44,\t127,0,\t0,0,\t133,0,\t
128,0,\t131,44,\t132,0,\t0,0,\t
0,0,\t0,0,\t132,0,\t133,0,\t
0,0,\t0,0,\t0,0,\t133,0,\t
127,181,\t0,0,\t134,0,\t135,0,\t
0,0,\t129,0,\t0,0,\t0,0,\t
132,44,\t128,182,\t134,0,\t135,0,\t
136,0,\t133,44,\t134,0,\t135,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
130,0,\t138,0,\t0,0,\t129,183,\t
0,0,\t131,0,\t0,0,\t130,184,\t
134,44,\t135,44,\t0,0,\t0,0,\t
137,0,\t0,0,\t136,0,\t0,0,\t
0,0,\t139,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t136,0,\t138,0,\t
132,0,\t0,0,\t136,0,\t131,185,\t
0,0,\t133,0,\t132,186,\t138,0,\t
133,187,\t0,0,\t137,0,\t138,0,\t
140,0,\t0,0,\t0,0,\t139,0,\t
136,44,\t0,0,\t137,0,\t0,0,\t
134,0,\t135,0,\t137,0,\t139,0,\t
0,0,\t138,44,\t0,0,\t139,0,\t
0,0,\t134,188,\t0,0,\t0,0,\t
141,0,\t0,0,\t140,0,\t0,0,\t
137,44,\t142,0,\t0,0,\t135,189,\t
0,0,\t139,44,\t140,0,\t0,0,\t
0,0,\t0,0,\t140,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
136,0,\t0,0,\t141,0,\t0,0,\t
143,0,\t0,0,\t0,0,\t142,0,\t
140,44,\t138,0,\t141,0,\t0,0,\t
0,0,\t144,0,\t141,0,\t142,0,\t
136,190,\t0,0,\t0,0,\t142,0,\t
137,0,\t0,0,\t0,0,\t0,0,\t
145,0,\t139,0,\t143,0,\t137,191,\t
141,44,\t137,192,\t0,0,\t138,193,\t
0,0,\t142,44,\t143,0,\t144,0,\t
0,0,\t0,0,\t143,0,\t0,0,\t
139,194,\t0,0,\t0,0,\t144,0,\t
140,0,\t146,0,\t145,0,\t144,0,\t
147,0,\t0,0,\t0,0,\t148,0,\t
143,44,\t0,0,\t145,0,\t0,0,\t
0,0,\t0,0,\t145,0,\t0,0,\t
140,195,\t144,44,\t0,0,\t0,0,\t
141,0,\t0,0,\t0,0,\t146,0,\t
149,0,\t142,0,\t147,0,\t141,196,\t
145,44,\t148,0,\t142,197,\t146,0,\t
0,0,\t0,0,\t147,0,\t146,0,\t
0,0,\t148,0,\t147,0,\t150,0,\t
0,0,\t148,0,\t0,0,\t0,0,\t
143,0,\t151,0,\t149,0,\t0,0,\t
152,0,\t146,44,\t143,198,\t0,0,\t
147,44,\t144,0,\t149,0,\t148,44,\t
0,0,\t0,0,\t149,0,\t0,0,\t
0,0,\t150,0,\t0,0,\t0,0,\t
145,0,\t0,0,\t0,0,\t151,0,\t
153,0,\t150,0,\t152,0,\t145,200,\t
149,44,\t150,0,\t0,0,\t151,0,\t
0,0,\t0,0,\t152,0,\t151,0,\t
144,199,\t0,0,\t152,0,\t154,0,\t
0,0,\t146,0,\t0,0,\t150,44,\t
147,0,\t0,0,\t153,0,\t148,0,\t
147,202,\t151,44,\t0,0,\t0,0,\t
152,44,\t0,0,\t153,0,\t0,0,\t
155,0,\t0,0,\t153,0,\t0,0,\t
0,0,\t154,0,\t156,0,\t146,201,\t
149,0,\t148,203,\t0,0,\t0,0,\t
0,0,\t154,0,\t0,0,\t149,204,\t
153,44,\t154,0,\t0,0,\t0,0,\t
0,0,\t157,0,\t155,0,\t150,0,\t
0,0,\t0,0,\t0,0,\t158,0,\t
156,0,\t151,0,\t155,0,\t154,44,\t
152,0,\t0,0,\t155,0,\t0,0,\t
156,0,\t0,0,\t0,0,\t0,0,\t
156,0,\t0,0,\t0,0,\t157,0,\t
151,206,\t150,205,\t0,0,\t0,0,\t
155,44,\t158,0,\t152,207,\t157,0,\t
153,0,\t0,0,\t156,44,\t157,0,\t
0,0,\t158,0,\t159,0,\t0,0,\t
0,0,\t158,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t154,0,\t
153,208,\t157,44,\t0,0,\t0,0,\t
0,0,\t0,0,\t154,209,\t158,44,\t
0,0,\t0,0,\t0,0,\t160,0,\t
159,0,\t0,0,\t0,0,\t0,0,\t
155,0,\t0,0,\t161,0,\t155,210,\t
159,0,\t0,0,\t156,0,\t0,0,\t
159,0,\t0,0,\t0,0,\t162,0,\t
163,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t160,0,\t164,0,\t0,0,\t
0,0,\t157,0,\t159,44,\t156,211,\t
161,0,\t160,0,\t0,0,\t158,0,\t
157,212,\t160,0,\t0,0,\t0,0,\t
161,0,\t162,0,\t163,0,\t165,0,\t
161,0,\t0,0,\t0,0,\t0,0,\t
164,0,\t162,0,\t163,0,\t160,44,\t
0,0,\t162,0,\t163,0,\t0,0,\t
164,0,\t0,0,\t161,44,\t166,0,\t
164,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t165,0,\t159,0,\t162,44,\t
163,44,\t0,0,\t0,0,\t0,0,\t
0,0,\t165,0,\t164,44,\t167,0,\t
159,213,\t165,0,\t0,0,\t0,0,\t
0,0,\t166,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t160,0,\t
0,0,\t166,0,\t0,0,\t165,44,\t
0,0,\t166,0,\t161,0,\t0,0,\t
0,0,\t167,0,\t160,214,\t0,0,\t
168,0,\t0,0,\t169,0,\t162,0,\t
163,0,\t167,0,\t0,0,\t166,44,\t
0,0,\t167,0,\t164,0,\t0,0,\t
0,0,\t0,0,\t161,215,\t170,0,\t
0,0,\t163,217,\t162,216,\t0,0,\t
164,218,\t0,0,\t168,0,\t167,44,\t
169,0,\t0,0,\t166,220,\t165,0,\t
0,0,\t0,0,\t168,0,\t171,0,\t
169,0,\t0,0,\t168,0,\t0,0,\t
169,0,\t170,0,\t0,0,\t0,0,\t
172,0,\t0,0,\t0,0,\t166,0,\t
0,0,\t170,0,\t173,0,\t165,219,\t
168,44,\t170,0,\t169,44,\t0,0,\t
0,0,\t171,0,\t0,0,\t174,0,\t
0,0,\t0,0,\t0,0,\t167,0,\t
0,0,\t171,0,\t172,0,\t170,44,\t
0,0,\t171,0,\t0,0,\t0,0,\t
173,0,\t0,0,\t172,0,\t175,0,\t
0,0,\t167,221,\t172,0,\t0,0,\t
173,0,\t174,0,\t176,0,\t171,44,\t
173,0,\t0,0,\t0,0,\t0,0,\t
168,0,\t174,0,\t169,0,\t0,0,\t
172,44,\t174,0,\t0,0,\t0,0,\t
0,0,\t175,0,\t173,44,\t177,0,\t
0,0,\t0,0,\t0,0,\t170,0,\t
176,0,\t175,0,\t168,222,\t174,44,\t
0,0,\t175,0,\t170,224,\t0,0,\t
176,0,\t169,223,\t0,0,\t178,0,\t
176,0,\t0,0,\t0,0,\t171,0,\t
179,0,\t177,0,\t171,225,\t175,44,\t
0,0,\t0,0,\t180,0,\t0,0,\t
172,0,\t177,0,\t176,44,\t181,0,\t
0,0,\t177,0,\t173,0,\t172,227,\t
171,226,\t178,0,\t0,0,\t173,228,\t
0,0,\t0,0,\t179,0,\t174,0,\t
0,0,\t178,0,\t0,0,\t177,44,\t
180,0,\t178,0,\t179,0,\t0,0,\t
174,229,\t181,0,\t179,0,\t0,0,\t
180,0,\t0,0,\t182,0,\t175,0,\t
180,0,\t181,0,\t0,0,\t178,44,\t
183,0,\t181,0,\t176,0,\t0,0,\t
179,44,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t180,44,\t0,0,\t
0,0,\t0,0,\t0,0,\t181,44,\t
182,0,\t175,230,\t0,0,\t177,0,\t
0,0,\t0,0,\t183,0,\t0,0,\t
182,0,\t184,0,\t0,0,\t0,0,\t
182,0,\t176,231,\t183,0,\t185,0,\t
186,0,\t0,0,\t183,0,\t178,0,\t
0,0,\t0,0,\t187,0,\t178,233,\t
179,0,\t177,232,\t182,44,\t0,0,\t
0,0,\t0,0,\t180,0,\t184,0,\t
183,44,\t0,0,\t0,0,\t181,0,\t
0,0,\t185,0,\t186,0,\t184,0,\t
0,0,\t179,234,\t181,236,\t184,0,\t
187,0,\t185,0,\t186,0,\t0,0,\t
180,235,\t185,0,\t186,0,\t0,0,\t
187,0,\t0,0,\t188,0,\t189,0,\t
187,0,\t184,44,\t0,0,\t0,0,\t
190,0,\t0,0,\t182,0,\t185,44,\t
186,44,\t0,0,\t0,0,\t0,0,\t
183,0,\t182,237,\t187,44,\t0,0,\t
0,0,\t0,0,\t0,0,\t183,238,\t
188,0,\t189,0,\t0,0,\t191,0,\t
0,0,\t0,0,\t190,0,\t0,0,\t
188,0,\t189,0,\t0,0,\t0,0,\t
188,0,\t189,0,\t190,0,\t0,0,\t
0,0,\t184,0,\t190,0,\t192,0,\t
0,0,\t0,0,\t0,0,\t185,0,\t
186,0,\t191,0,\t188,44,\t189,44,\t
0,0,\t0,0,\t187,0,\t0,0,\t
190,44,\t191,0,\t0,0,\t0,0,\t
0,0,\t191,0,\t184,239,\t0,0,\t
0,0,\t192,0,\t193,0,\t194,0,\t
195,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t192,0,\t0,0,\t191,44,\t
187,240,\t192,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t196,0,\t
0,0,\t0,0,\t188,0,\t189,0,\t
193,0,\t194,0,\t195,0,\t192,44,\t
190,0,\t188,241,\t189,242,\t0,0,\t
193,0,\t194,0,\t195,0,\t0,0,\t
193,0,\t194,0,\t195,0,\t190,243,\t
0,0,\t196,0,\t197,0,\t198,0,\t
199,0,\t0,0,\t0,0,\t191,0,\t
0,0,\t196,0,\t193,44,\t194,44,\t
195,44,\t196,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t200,0,\t
0,0,\t0,0,\t0,0,\t192,0,\t
197,0,\t198,0,\t199,0,\t196,44,\t
0,0,\t0,0,\t0,0,\t0,0,\t
197,0,\t198,0,\t199,0,\t0,0,\t
197,0,\t198,0,\t199,0,\t0,0,\t
0,0,\t200,0,\t201,0,\t192,244,\t
202,0,\t0,0,\t193,0,\t194,0,\t
195,0,\t200,0,\t197,44,\t198,44,\t
199,44,\t200,0,\t194,246,\t0,0,\t
193,245,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t196,0,\t
201,0,\t0,0,\t202,0,\t200,44,\t
0,0,\t0,0,\t0,0,\t0,0,\t
201,0,\t0,0,\t202,0,\t203,0,\t
201,0,\t204,0,\t202,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t196,247,\t
0,0,\t0,0,\t197,0,\t198,0,\t
199,0,\t0,0,\t201,44,\t0,0,\t
202,44,\t197,248,\t0,0,\t199,250,\t
0,0,\t203,0,\t0,0,\t204,0,\t
0,0,\t0,0,\t205,0,\t200,0,\t
0,0,\t203,0,\t200,251,\t204,0,\t
198,249,\t203,0,\t0,0,\t204,0,\t
206,0,\t0,0,\t0,0,\t207,0,\t
0,0,\t208,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t209,0,\t203,44,\t
205,0,\t204,44,\t201,0,\t0,0,\t
202,0,\t0,0,\t0,0,\t0,0,\t
205,0,\t0,0,\t206,0,\t0,0,\t
205,0,\t207,0,\t0,0,\t208,0,\t
202,253,\t0,0,\t206,0,\t0,0,\t
209,0,\t207,0,\t206,0,\t208,0,\t
201,252,\t207,0,\t205,44,\t208,0,\t
209,0,\t0,0,\t210,0,\t0,0,\t
209,0,\t211,0,\t0,0,\t203,0,\t
206,44,\t204,0,\t0,0,\t207,44,\t
0,0,\t208,44,\t203,254,\t0,0,\t
0,0,\t0,0,\t209,44,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
210,0,\t0,0,\t212,0,\t211,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
210,0,\t213,0,\t205,0,\t211,0,\t
210,0,\t0,0,\t0,0,\t211,0,\t
0,0,\t0,0,\t214,0,\t215,0,\t
206,0,\t0,0,\t0,0,\t207,0,\t
212,0,\t208,0,\t210,44,\t206,256,\t
0,0,\t211,44,\t209,0,\t213,0,\t
212,0,\t205,255,\t216,0,\t0,0,\t
212,0,\t0,0,\t0,0,\t213,0,\t
214,0,\t215,0,\t0,0,\t213,0,\t
207,257,\t0,0,\t0,0,\t217,0,\t
214,0,\t215,0,\t212,44,\t209,258,\t
214,0,\t215,0,\t0,0,\t0,0,\t
216,0,\t213,44,\t218,0,\t0,0,\t
0,0,\t0,0,\t210,0,\t0,0,\t
216,0,\t211,0,\t214,44,\t215,44,\t
216,0,\t217,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t219,0,\t
0,0,\t217,0,\t220,0,\t0,0,\t
218,0,\t217,0,\t216,44,\t0,0,\t
0,0,\t211,260,\t212,0,\t0,0,\t
218,0,\t210,259,\t0,0,\t0,0,\t
218,0,\t213,0,\t0,0,\t217,44,\t
213,261,\t219,0,\t221,0,\t222,0,\t
220,0,\t0,0,\t214,0,\t215,0,\t
0,0,\t219,0,\t218,44,\t0,0,\t
220,0,\t219,0,\t0,0,\t0,0,\t
220,0,\t0,0,\t0,0,\t0,0,\t
214,262,\t0,0,\t216,0,\t0,0,\t
221,0,\t222,0,\t223,0,\t219,44,\t
215,263,\t216,264,\t220,44,\t224,0,\t
221,0,\t222,0,\t0,0,\t217,0,\t
221,0,\t222,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t218,0,\t0,0,\t
223,0,\t217,265,\t221,44,\t222,44,\t
0,0,\t224,0,\t225,0,\t0,0,\t
223,0,\t218,266,\t0,0,\t0,0,\t
223,0,\t224,0,\t0,0,\t219,0,\t
0,0,\t224,0,\t220,0,\t0,0,\t
0,0,\t226,0,\t227,0,\t0,0,\t
0,0,\t0,0,\t223,44,\t228,0,\t
225,0,\t219,267,\t0,0,\t224,44,\t
0,0,\t0,0,\t0,0,\t0,0,\t
225,0,\t0,0,\t221,0,\t222,0,\t
225,0,\t0,0,\t0,0,\t226,0,\t
227,0,\t221,268,\t222,269,\t0,0,\t
0,0,\t228,0,\t229,0,\t226,0,\t
227,0,\t0,0,\t225,44,\t226,0,\t
227,0,\t228,0,\t0,0,\t230,0,\t
0,0,\t228,0,\t223,0,\t0,0,\t
0,0,\t0,0,\t231,0,\t224,0,\t
0,0,\t226,44,\t227,44,\t0,0,\t
229,0,\t0,0,\t0,0,\t228,44,\t
0,0,\t0,0,\t223,270,\t0,0,\t
229,0,\t230,0,\t0,0,\t0,0,\t
229,0,\t0,0,\t0,0,\t0,0,\t
231,0,\t230,0,\t225,0,\t0,0,\t
0,0,\t230,0,\t232,0,\t0,0,\t
231,0,\t233,0,\t229,44,\t234,0,\t
231,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t226,0,\t227,0,\t230,44,\t
0,0,\t226,272,\t235,0,\t228,0,\t
225,271,\t227,273,\t231,44,\t0,0,\t
232,0,\t0,0,\t0,0,\t233,0,\t
0,0,\t234,0,\t228,274,\t0,0,\t
232,0,\t0,0,\t236,0,\t233,0,\t
232,0,\t234,0,\t0,0,\t233,0,\t
235,0,\t234,0,\t229,0,\t0,0,\t
0,0,\t237,0,\t0,0,\t238,0,\t
235,0,\t229,275,\t232,44,\t230,0,\t
235,0,\t233,44,\t0,0,\t234,44,\t
236,0,\t0,0,\t231,0,\t0,0,\t
0,0,\t0,0,\t239,0,\t0,0,\t
236,0,\t0,0,\t235,44,\t237,0,\t
236,0,\t238,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t237,0,\t
0,0,\t238,0,\t0,0,\t237,0,\t
0,0,\t238,0,\t236,44,\t0,0,\t
239,0,\t0,0,\t232,0,\t240,0,\t
0,0,\t233,0,\t241,0,\t234,0,\t
239,0,\t237,44,\t0,0,\t238,44,\t
239,0,\t0,0,\t0,0,\t0,0,\t
242,0,\t233,277,\t235,0,\t243,0,\t
0,0,\t0,0,\t0,0,\t235,279,\t
232,276,\t240,0,\t239,44,\t0,0,\t
241,0,\t234,278,\t0,0,\t0,0,\t
0,0,\t240,0,\t236,0,\t244,0,\t
241,0,\t240,0,\t242,0,\t0,0,\t
241,0,\t243,0,\t0,0,\t0,0,\t
245,0,\t237,0,\t242,0,\t238,0,\t
0,0,\t243,0,\t242,0,\t240,44,\t
238,280,\t243,0,\t241,44,\t0,0,\t
0,0,\t244,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t239,0,\t246,0,\t
242,44,\t244,0,\t245,0,\t243,44,\t
0,0,\t244,0,\t0,0,\t0,0,\t
247,0,\t0,0,\t245,0,\t0,0,\t
0,0,\t0,0,\t245,0,\t248,0,\t
0,0,\t0,0,\t0,0,\t244,44,\t
0,0,\t246,0,\t0,0,\t240,0,\t
249,0,\t239,281,\t241,0,\t0,0,\t
245,44,\t246,0,\t247,0,\t250,0,\t
241,283,\t246,0,\t240,282,\t0,0,\t
242,0,\t248,0,\t247,0,\t243,0,\t
251,0,\t242,284,\t247,0,\t0,0,\t
0,0,\t248,0,\t249,0,\t246,44,\t
243,285,\t248,0,\t0,0,\t0,0,\t
0,0,\t250,0,\t249,0,\t244,0,\t
247,44,\t0,0,\t249,0,\t252,0,\t
0,0,\t250,0,\t251,0,\t248,44,\t
245,0,\t250,0,\t0,0,\t0,0,\t
253,0,\t0,0,\t251,0,\t0,0,\t
249,44,\t244,286,\t251,0,\t254,0,\t
0,0,\t0,0,\t245,287,\t250,44,\t
0,0,\t252,0,\t0,0,\t246,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
251,44,\t252,0,\t253,0,\t255,0,\t
247,0,\t252,0,\t0,0,\t0,0,\t
0,0,\t254,0,\t253,0,\t248,0,\t
256,0,\t0,0,\t253,0,\t0,0,\t
0,0,\t254,0,\t257,0,\t252,44,\t
249,0,\t254,0,\t0,0,\t0,0,\t
0,0,\t255,0,\t0,0,\t250,0,\t
253,44,\t0,0,\t250,290,\t258,0,\t
248,288,\t255,0,\t256,0,\t254,44,\t
251,0,\t255,0,\t249,289,\t0,0,\t
257,0,\t0,0,\t256,0,\t0,0,\t
0,0,\t0,0,\t256,0,\t259,0,\t
257,0,\t0,0,\t260,0,\t255,44,\t
257,0,\t258,0,\t0,0,\t252,0,\t
261,0,\t0,0,\t251,291,\t0,0,\t
256,44,\t258,0,\t0,0,\t262,0,\t
253,0,\t258,0,\t257,44,\t0,0,\t
0,0,\t259,0,\t0,0,\t254,0,\t
260,0,\t0,0,\t0,0,\t252,292,\t
254,293,\t259,0,\t261,0,\t258,44,\t
260,0,\t259,0,\t0,0,\t0,0,\t
260,0,\t262,0,\t261,0,\t255,0,\t
263,0,\t0,0,\t261,0,\t264,0,\t
0,0,\t262,0,\t265,0,\t259,44,\t
256,0,\t262,0,\t260,44,\t0,0,\t
0,0,\t0,0,\t257,0,\t0,0,\t
261,44,\t0,0,\t0,0,\t0,0,\t
255,294,\t0,0,\t263,0,\t262,44,\t
0,0,\t264,0,\t0,0,\t258,0,\t
265,0,\t0,0,\t263,0,\t266,0,\t
257,295,\t264,0,\t263,0,\t0,0,\t
265,0,\t264,0,\t0,0,\t0,0,\t
265,0,\t267,0,\t0,0,\t259,0,\t
0,0,\t0,0,\t260,0,\t0,0,\t
263,44,\t258,296,\t0,0,\t264,44,\t
261,0,\t266,0,\t265,44,\t0,0,\t
0,0,\t0,0,\t268,0,\t262,0,\t
0,0,\t266,0,\t0,0,\t267,0,\t
269,0,\t266,0,\t260,297,\t0,0,\t
261,298,\t0,0,\t0,0,\t267,0,\t
0,0,\t270,0,\t271,0,\t267,0,\t
0,0,\t0,0,\t0,0,\t266,44,\t
268,0,\t0,0,\t0,0,\t0,0,\t
263,0,\t0,0,\t269,0,\t264,0,\t
268,0,\t267,44,\t265,0,\t0,0,\t
268,0,\t0,0,\t269,0,\t270,0,\t
271,0,\t0,0,\t269,0,\t0,0,\t
0,0,\t0,0,\t272,0,\t270,0,\t
271,0,\t0,0,\t268,44,\t270,0,\t
271,0,\t0,0,\t0,0,\t273,0,\t
269,44,\t274,0,\t0,0,\t266,0,\t
0,0,\t265,299,\t0,0,\t0,0,\t
0,0,\t270,44,\t271,44,\t0,0,\t
272,0,\t267,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t266,300,\t
272,0,\t273,0,\t275,0,\t274,0,\t
272,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t273,0,\t268,0,\t274,0,\t
276,0,\t273,0,\t0,0,\t274,0,\t
269,0,\t0,0,\t272,44,\t277,0,\t
267,301,\t269,302,\t278,0,\t0,0,\t
275,0,\t270,0,\t271,0,\t273,44,\t
0,0,\t274,44,\t270,303,\t0,0,\t
275,0,\t0,0,\t276,0,\t0,0,\t
275,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t277,0,\t276,0,\t0,0,\t
278,0,\t0,0,\t276,0,\t279,0,\t
271,304,\t277,0,\t275,44,\t0,0,\t
278,0,\t277,0,\t272,0,\t0,0,\t
278,0,\t0,0,\t0,0,\t0,0,\t
276,44,\t0,0,\t0,0,\t273,0,\t
0,0,\t274,0,\t272,305,\t277,44,\t
0,0,\t279,0,\t278,44,\t280,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t279,0,\t281,0,\t273,306,\t
282,0,\t279,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t275,0,\t274,307,\t
0,0,\t0,0,\t0,0,\t283,0,\t
275,308,\t280,0,\t0,0,\t279,44,\t
276,0,\t0,0,\t0,0,\t0,0,\t
281,0,\t280,0,\t282,0,\t277,0,\t
0,0,\t280,0,\t278,0,\t0,0,\t
281,0,\t0,0,\t282,0,\t284,0,\t
281,0,\t283,0,\t282,0,\t0,0,\t
276,309,\t278,310,\t285,0,\t280,44,\t
286,0,\t283,0,\t0,0,\t0,0,\t
0,0,\t283,0,\t281,44,\t0,0,\t
282,44,\t0,0,\t0,0,\t279,0,\t
0,0,\t284,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t283,44,\t
285,0,\t284,0,\t286,0,\t287,0,\t
0,0,\t284,0,\t0,0,\t0,0,\t
285,0,\t0,0,\t286,0,\t0,0,\t
285,0,\t0,0,\t286,0,\t280,0,\t
0,0,\t0,0,\t288,0,\t284,44,\t
289,0,\t0,0,\t281,0,\t290,0,\t
282,0,\t287,0,\t285,44,\t0,0,\t
286,44,\t0,0,\t0,0,\t0,0,\t
0,0,\t287,0,\t0,0,\t283,0,\t
0,0,\t287,0,\t0,0,\t0,0,\t
288,0,\t282,312,\t289,0,\t281,311,\t
0,0,\t290,0,\t283,313,\t291,0,\t
288,0,\t0,0,\t289,0,\t287,44,\t
288,0,\t290,0,\t289,0,\t284,0,\t
0,0,\t290,0,\t292,0,\t0,0,\t
293,0,\t0,0,\t285,0,\t0,0,\t
286,0,\t0,0,\t288,44,\t294,0,\t
289,44,\t291,0,\t0,0,\t290,44,\t
285,314,\t0,0,\t0,0,\t0,0,\t
0,0,\t291,0,\t0,0,\t0,0,\t
292,0,\t291,0,\t293,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t287,0,\t
292,0,\t294,0,\t293,0,\t295,0,\t
292,0,\t0,0,\t293,0,\t291,44,\t
0,0,\t294,0,\t0,0,\t0,0,\t
296,0,\t294,0,\t288,0,\t0,0,\t
289,0,\t0,0,\t292,44,\t290,0,\t
293,44,\t0,0,\t0,0,\t289,317,\t
0,0,\t295,0,\t287,315,\t294,44,\t
297,0,\t0,0,\t0,0,\t298,0,\t
0,0,\t295,0,\t296,0,\t288,316,\t
0,0,\t295,0,\t0,0,\t0,0,\t
290,318,\t0,0,\t296,0,\t291,0,\t
299,0,\t0,0,\t296,0,\t300,0,\t
0,0,\t0,0,\t297,0,\t295,44,\t
0,0,\t298,0,\t292,0,\t0,0,\t
293,0,\t0,0,\t297,0,\t0,0,\t
296,44,\t298,0,\t297,0,\t294,0,\t
0,0,\t298,0,\t299,0,\t0,0,\t
301,0,\t300,0,\t0,0,\t302,0,\t
0,0,\t0,0,\t299,0,\t0,0,\t
297,44,\t300,0,\t299,0,\t298,44,\t
0,0,\t300,0,\t0,0,\t0,0,\t
303,0,\t0,0,\t0,0,\t295,0,\t
0,0,\t0,0,\t301,0,\t0,0,\t
299,44,\t302,0,\t295,319,\t300,44,\t
296,0,\t0,0,\t301,0,\t296,320,\t
304,0,\t302,0,\t301,0,\t305,0,\t
0,0,\t302,0,\t303,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
297,0,\t306,0,\t303,0,\t298,0,\t
301,44,\t0,0,\t303,0,\t302,44,\t
0,0,\t0,0,\t304,0,\t307,0,\t
298,322,\t305,0,\t0,0,\t0,0,\t
299,0,\t297,321,\t304,0,\t300,0,\t
303,44,\t305,0,\t304,0,\t306,0,\t
0,0,\t305,0,\t0,0,\t0,0,\t
308,0,\t0,0,\t309,0,\t306,0,\t
0,0,\t307,0,\t0,0,\t306,0,\t
304,44,\t0,0,\t0,0,\t305,44,\t
301,0,\t307,0,\t0,0,\t302,0,\t
0,0,\t307,0,\t0,0,\t0,0,\t
0,0,\t306,44,\t308,0,\t0,0,\t
309,0,\t0,0,\t0,0,\t0,0,\t
303,0,\t310,0,\t308,0,\t307,44,\t
309,0,\t0,0,\t308,0,\t303,323,\t
309,0,\t0,0,\t0,0,\t311,0,\t
312,0,\t0,0,\t0,0,\t0,0,\t
304,0,\t0,0,\t0,0,\t305,0,\t
308,44,\t313,0,\t309,44,\t310,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t306,0,\t0,0,\t310,0,\t
0,0,\t311,0,\t312,0,\t310,0,\t
304,324,\t0,0,\t0,0,\t307,0,\t
314,0,\t311,0,\t312,0,\t313,0,\t
0,0,\t311,0,\t312,0,\t0,0,\t
0,0,\t310,44,\t315,0,\t313,0,\t
0,0,\t316,0,\t0,0,\t313,0,\t
308,0,\t0,0,\t309,0,\t311,44,\t
312,44,\t0,0,\t314,0,\t0,0,\t
0,0,\t0,0,\t307,325,\t0,0,\t
0,0,\t313,44,\t314,0,\t0,0,\t
315,0,\t0,0,\t314,0,\t316,0,\t
0,0,\t317,0,\t0,0,\t0,0,\t
315,0,\t0,0,\t0,0,\t316,0,\t
315,0,\t310,0,\t0,0,\t316,0,\t
314,44,\t0,0,\t0,0,\t0,0,\t
318,0,\t310,326,\t0,0,\t311,0,\t
312,0,\t319,0,\t315,44,\t317,0,\t
0,0,\t316,44,\t0,0,\t0,0,\t
0,0,\t313,0,\t0,0,\t317,0,\t
320,0,\t0,0,\t0,0,\t317,0,\t
312,327,\t0,0,\t318,0,\t0,0,\t
0,0,\t321,0,\t0,0,\t319,0,\t
0,0,\t313,328,\t318,0,\t0,0,\t
314,0,\t317,44,\t318,0,\t319,0,\t
0,0,\t0,0,\t320,0,\t319,0,\t
0,0,\t0,0,\t315,0,\t0,0,\t
0,0,\t316,0,\t320,0,\t321,0,\t
318,44,\t322,0,\t320,0,\t0,0,\t
0,0,\t319,44,\t314,329,\t321,0,\t
0,0,\t0,0,\t0,0,\t321,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
320,44,\t323,0,\t0,0,\t0,0,\t
324,0,\t317,0,\t0,0,\t322,0,\t
0,0,\t321,44,\t317,330,\t0,0,\t
0,0,\t0,0,\t0,0,\t322,0,\t
0,0,\t0,0,\t0,0,\t322,0,\t
318,0,\t0,0,\t0,0,\t323,0,\t
325,0,\t319,0,\t324,0,\t0,0,\t
0,0,\t0,0,\t319,332,\t323,0,\t
0,0,\t322,44,\t324,0,\t323,0,\t
320,0,\t0,0,\t324,0,\t0,0,\t
0,0,\t326,0,\t318,331,\t327,0,\t
328,0,\t321,0,\t325,0,\t0,0,\t
0,0,\t323,44,\t0,0,\t320,333,\t
324,44,\t0,0,\t325,0,\t0,0,\t
0,0,\t0,0,\t325,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t326,0,\t
329,0,\t327,0,\t328,0,\t0,0,\t
0,0,\t322,0,\t321,334,\t326,0,\t
325,44,\t327,0,\t328,0,\t326,0,\t
322,335,\t327,0,\t328,0,\t0,0,\t
0,0,\t330,0,\t0,0,\t331,0,\t
332,0,\t323,0,\t329,0,\t0,0,\t
324,0,\t326,44,\t0,0,\t327,44,\t
328,44,\t0,0,\t329,0,\t0,0,\t
0,0,\t0,0,\t329,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t330,0,\t
0,0,\t331,0,\t332,0,\t0,0,\t
325,0,\t0,0,\t0,0,\t330,0,\t
329,44,\t331,0,\t332,0,\t330,0,\t
333,0,\t331,0,\t332,0,\t0,0,\t
0,0,\t334,0,\t0,0,\t335,0,\t
0,0,\t326,0,\t0,0,\t327,0,\t
328,0,\t330,44,\t0,0,\t331,44,\t
332,44,\t0,0,\t0,0,\t0,0,\t
336,0,\t0,0,\t333,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t334,0,\t
0,0,\t335,0,\t333,0,\t0,0,\t
329,0,\t0,0,\t333,0,\t334,0,\t
326,336,\t335,0,\t0,0,\t334,0,\t
337,0,\t335,0,\t336,0,\t0,0,\t
0,0,\t338,0,\t0,0,\t0,0,\t
333,44,\t330,0,\t336,0,\t331,0,\t
332,0,\t334,44,\t336,0,\t335,44,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t337,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t338,0,\t
336,44,\t0,0,\t337,0,\t0,0,\t
0,0,\t0,0,\t337,0,\t338,0,\t
0,0,\t0,0,\t0,0,\t338,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
333,0,\t0,0,\t0,0,\t0,0,\t
337,44,\t334,0,\t0,0,\t335,0,\t
0,0,\t338,44,\t0,0,\t0,0,\t
0,0,\t335,338,\t0,0,\t0,0,\t
0,0,\t0,0,\t333,337,\t0,0,\t
336,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
337,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t338,0,\t0,0,\t0,0,\t
0,0};'
		OUTPUT - $'                  
#include <ast.h>
# include "stdio.h"
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX BUFSIZ
# define output(c) putc(c,yyout)
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar==\'\\n\')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng; extern __MANGLE__ char yytext[];
int yymorfg;
extern __MANGLE__ char *yysptr, yysbuf[];
int yytchar;
FILE *yyin = {stdin}, *yyout = {stdout};
extern __MANGLE__ int yylineno;
struct yysvf { 
\tstruct yywork *yystoff;
\tstruct yysvf *yyother;
\tint *yystops;};
struct yysvf *yyestate;
extern __MANGLE__ struct yysvf yysvec[], *yybgin;
/* 
 * lex.l - scanner for yeast announcements/specifications
 * 
 * Author:\tBalachander Krishnamurthy
 * \t\tDavid S. Rosenblum
 * \t\tAT&T Bell Laboratories
 * Date:\tMon Feb 27 22:40:42 1989
 *
 */

/*
 * Modification history:
 * \tMon Mar 12 13:38:46 1990 -- undef\'ing input to define own routine
 *      Tue Jun 26 14:14:24 1990 -- changed BLANKFREESTRING from * to +
 *
 * If any tokens are added here yeast.y has to change - :-(
 */

#undef\tinput\t /* defeat lex */

extern __MANGLE__ void savesb __PROTO__((char*));
extern __MANGLE__ void FixString __PROTO__((char[]));
extern __MANGLE__ void errmsg __PROTO__((char*));
extern __MANGLE__ char input __PROTO__((void));

char c;
#include <h/globals.h>
#include <yeast.h>

extern __MANGLE__ bool fParseFailed;
# define YYNEWLINE 10
yylex(){
int nstr; extern __MANGLE__ int yyprevious;
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:
 ;
break;
case 2:
        {savesb("3dfile"); return(DDDFILECLASS);}
break;
case 3:
\t{savesb("addspec");return(ADDSPEC);}
break;
case 4:
\t{savesb("and"); return(AND);}
break;
case 5:
{savesb("announce"); return(ANNOUNCE);}
break;
case 6:
\t{savesb("at"); return(AT);}
break;
case 7:
\t{savesb("atime"); return(ATIME);}
break;
case 8:
{savesb("authattr"); return(AUTHATTR);}
break;
case 9:
\t{savesb("authobj"); return(AUTHOBJ);}
break;
case 10:
\t{savesb("between"); return(BETWEEN);}
break;
case 11:
\t{savesb("by"); return(BY);}
break;
case 12:
{savesb("capacity"); return(CAPACITY);}
break;
case 13:
\t{savesb("changed"); return(CHANGED);}
break;
case 14:
\t{savesb("count"); return(COUNT);}
break;
case 15:
\t{savesb("daily"); return(DAILY);}
break;
case 16:
\t{savesb("defattr"); return(DEFATTR);}
break;
case 17:
\t{savesb("defobj"); return(DEFOBJ);}
break;
case 18:
        {savesb("dir"); return(DIRCLASS);}
break;
case 19:
\t{savesb("do"); return(DO);}
break;
case 20:
{savesb("do_notify"); return(DO_NOTIFY);}
break;
case 21:
\t{savesb("do_rpc"); return(DO_RPC);}
break;
case 22:
\t{savesb("etime"); return(PETIME);}
break;
case 23:
\t{savesb("false"); return(FALSE);}
break;
case 24:
\t{savesb("fgspec"); return(FGSPEC);}
break;
case 25:
        {savesb("file"); return(FILECLASS);}
break;
case 26:
\t{savesb("filesys"); return(FILESYSTEMCLASS);}
break;
case 27:
\t{savesb("host"); return(HOSTCLASS);}
break;
case 28:
\t{savesb("in"); return(IN);}
break;
case 29:
\t{savesb("load"); return(LOAD);}
break;
case 30:
{savesb("where"); return(LOCATION);}
break;
case 31:
{savesb("loggedin"); return(LOGGEDIN);}
break;
case 32:
\t{savesb("lsspec"); return(LSSPEC);}
break;
case 33:
\t{savesb("mode"); return(MODE);}
break;
case 34:
\t{savesb("modgrp"); return(MODGRP);}
break;
case 35:
\t{savesb("monthly"); return(MONTHLY);}
break;
case 36:
{savesb("midnight"); return(MIDNIGHT);}
break;
case 37:
\t{savesb("mtime"); return(MTIME);}
break;
case 38:
\t{savesb("no"); return(NOON);}
break;
case 39:
\t{savesb("or"); return(OR);}
break;
case 40:
\t{savesb("owner"); return(OWNER);}
break;
case 41:
\t{savesb("process"); return(PROCESSCLASS);}
break;
case 42:
{savesb("readspec"); return(READSPEC);}
break;
case 43:
{savesb("regyeast"); return(REGYEAST);}
break;
case 44:
\t{savesb("repeat"); return(REPEAT);}
break;
case 45:
\t{savesb("rmattr"); return(RMATTR);}
break;
case 46:
\t{savesb("rmobj"); return(RMOBJ);}
break;
case 47:
\t{savesb("rmspec"); return(RMSPEC);}
break;
case 48:
\t{savesb("size"); return(SIZE);}
break;
case 49:
\t{savesb("status"); return(STATUS);}
break;
case 50:
\t{savesb("stime"); return(STIME);}
break;
case 51:
{savesb("suspspec"); return(SUSPSPEC);}
break;
case 52:
\t{savesb("then"); return(THEN);}
break;
case 53:
{savesb("timestamp"); return(TIMESTAMP);}
break;
case 54:
\t{savesb("today"); return(TODAY);}
break;
case 55:
{savesb("tomorrow"); return(TOMORROW);}
break;
case 56:
\t{savesb("true"); return(TRUE);}
break;
case 57:
\t{savesb("tty"); return(TTYCLASS);}
break;
case 58:
{savesb("unchanged"); return(UNCHANGED);}
break;
case 59:
\t{savesb("until"); return(UNTIL);}
break;
case 60:
\t{savesb("up"); return(UP);}
break;
case 61:
\t{savesb("user"); return(USERCLASS);}
break;
case 62:
\t{savesb("users"); return(USERS);}
break;
case 63:
\t{savesb("utime"); return(UTIME);}
break;
case 64:
\t{savesb("week"); return(WEEK);}
break;
case 65:
\t{savesb("weekly"); return(WEEKLY);}
break;
case 66:
\t{savesb("within"); return(WITHIN);}
break;
case 67:
\t{savesb("yearly"); return(YEARLY);}
break;
case 68:
  \t\treturn(NEWLINE);
break;
case 69:
\treturn (ASSIGN);
break;
case 70:
\treturn (COMMA);
break;
case 71:
\treturn (EQ);
break;
case 72:
\treturn (GE);
break;
case 73:
\treturn (GT);
break;
case 74:
\treturn (LE);
break;
case 75:
\treturn (LPAREN);
break;
case 76:
\treturn (LT);
break;
case 77:
\treturn (NE);
break;
case 78:
\treturn (RPAREN);
break;
case 79:
\treturn (PERCENT);
break;
case 80:
\treturn (LCURLY);
break;
case 81:
\treturn (RCURLY);
break;
case 82:
{savesb(yytext); return(HEX);}
break;
case 83:
\t{savesb(yytext); return(OCTAL);}
break;
case 84:
\t{savesb(yytext); return(INTEGER);}
break;
case 85:
{savesb(yytext); return(INTRANGE);}
break;
case 86:
{savesb(yytext); return(REAL);}
break;
case 87:
 { savesb(yytext); return(HOURSPEC); }
break;
case 88:
{ savesb(yytext); return(HOURMINSPEC); }
break;
case 89:
{ savesb(yytext); return(HOURMINSPEC); }
break;
case 90:
{
\t\tif((yylval.sb =  malloc (yyleng+1)) != NULL)
\t\t    strcpy(yylval.sb,yytext);
\t\treturn(GROUPLABEL);
\t    }
break;
case 91:
{
\t\tif((yylval.sb =  malloc (yyleng+1)) != NULL) {
\t\t    yytext[yyleng-1] = \'\\0\';
\t\t    strcpy(yylval.sb,yytext);
\t      }
\t\treturn(EVENTLABEL);
\t    }
break;
case 92:
{\t\t\t\t
\t\tif((yylval.sb =  malloc (yyleng+1)) != NULL)
\t\t    strcpy(yylval.sb,yytext);
\t\treturn(BLANKFREESTRING);
\t    }
break;
case 93:
{                       /* Quoted String Checker */ 
             if(yytext[yyleng-1] == \'\\\\\') 
                yymore() ;
             else {
               c = input() ;
               if (c == \'\\n\') {
                  unput(c) ;
                  errmsg("String constant flows over line
 ") ;
                  yytext[yyleng] = \'\\0\' ;
                  return(QUOTEDSTRING) ;
               }
               else {
                  yytext[yyleng++] = \'"\' ;
                  yytext[yyleng] = \'\\0\' ;
                  FixString(yytext) ;
                  if((yylval.sb = malloc (yyleng+1)) != NULL)
                     strcpy(yylval.sb,yytext) ;
                  return(QUOTEDSTRING) ;
               }
             }  
            }
break;
case 94:
\t    { 
\t        errmsg("unknown character in input
") ;
\t    }
break;
case -1:
break;
default:
fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */

/*
 * ====================================================================
 * savesb - saves string value of token
 * ====================================================================
 */
    
void
savesb __PARAM__((char* sb), (sb)) __OTORP__(char* sb;)
#line 402
{
    if((yylval.sb = (char *)malloc(strlen(sb)+1)) != NULL) 
\tstrcpy(yylval.sb,sb) ;
}

/****
 * Routine FixString to delete the double quotes in the string 
 ****/
void
FixString __PARAM__((char yytext[]), (yytext)) __OTORP__(char yytext[];)
#line 412
{
 int i; 

 for (i=1 ; i < yyleng -1 ; i++)
     yytext[i-1] = yytext[i] ;
 yytext[i-1] = \'\\0\' ;
}

/*
 * ====================================================================
 * errmsg - Error routine for lex
 * ====================================================================
 */
void
errmsg __PARAM__((char* sb), (sb)) __OTORP__(char* sb;)
#line 427
{
    sfprintf(sbMsg, "lexical error: %s", sb);
    fParseFailed = 1;
}

char
input __PARAM__((void), ())
#line 434
{
    register c;
    extern __MANGLE__ char *lexprog;

    if (yysptr > yysbuf)
\tc = U(*--yysptr);
    else
\tc = *lexprog++;
    return(c); 
}
int yyvstop[] = {
0,

92,
94,
0,

1,
92,
94,
0,

68,
0,

1,
94,
0,

92,
94,
0,

92,
93,
94,
0,

79,
92,
94,
0,

75,
94,
0,

78,
92,
94,
0,

92,
94,
0,

70,
94,
0,

92,
94,
0,

83,
84,
92,
94,
0,

84,
92,
94,
0,

84,
92,
94,
0,

76,
92,
94,
0,

69,
92,
94,
0,

73,
92,
94,
0,

94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

92,
94,
0,

80,
92,
94,
0,

81,
92,
94,
0,

92,
0,

91,
92,
0,

77,
92,
0,

92,
93,
0,

93,
0,

91,
92,
93,
0,

90,
92,
0,

90,
92,
0,

90,
91,
92,
0,

86,
92,
0,

92,
0,

92,
0,

83,
84,
92,
0,

84,
92,
0,

91,
92,
0,

87,
92,
0,

92,
0,

92,
0,

74,
92,
0,

71,
92,
0,

72,
92,
0,

92,
0,

92,
0,

6,
92,
0,

92,
0,

92,
0,

11,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

19,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

28,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

39,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

60,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

85,
92,
0,

88,
92,
0,

87,
92,
0,

82,
92,
0,

92,
0,

92,
0,

4,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

18,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

57,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

89,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

25,
92,
0,

27,
92,
0,

29,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

33,
92,
0,

92,
0,

92,
0,

92,
0,

38,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

48,
92,
0,

92,
0,

92,
0,

92,
0,

52,
92,
0,

92,
0,

92,
0,

92,
0,

56,
92,
0,

92,
0,

92,
0,

61,
92,
0,

92,
0,

64,
92,
0,

92,
0,

92,
0,

89,
92,
0,

92,
0,

92,
0,

92,
0,

7,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

14,
92,
0,

15,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

22,
92,
0,

23,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

37,
92,
0,

40,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

46,
92,
0,

92,
0,

92,
0,

50,
92,
0,

92,
0,

92,
0,

54,
92,
0,

92,
0,

92,
0,

59,
92,
0,

62,
92,
0,

63,
92,
0,

92,
0,

92,
0,

92,
0,

2,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

17,
92,
0,

92,
0,

21,
92,
0,

24,
92,
0,

92,
0,

92,
0,

92,
0,

32,
92,
0,

92,
0,

34,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

44,
92,
0,

45,
92,
0,

47,
92,
0,

49,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

65,
92,
0,

66,
92,
0,

67,
92,
0,

3,
92,
0,

92,
0,

92,
0,

9,
92,
0,

10,
92,
0,

92,
0,

13,
92,
0,

16,
92,
0,

92,
0,

26,
92,
0,

92,
0,

92,
0,

92,
0,

35,
92,
0,

41,
92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

92,
0,

5,
92,
0,

8,
92,
0,

12,
92,
0,

92,
0,

30,
92,
0,

31,
92,
0,

36,
92,
0,

42,
92,
0,

43,
92,
0,

51,
92,
0,

92,
0,

55,
92,
0,

92,
0,

20,
92,
0,

53,
92,
0,

58,
92,
0,
0};
# define YYTYPE int
struct yywork { YYTYPE verify, advance; } yycrank[] = {
0,0,\t0,0,\t1,3,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t1,4,\t1,5,\t
0,0,\t0,0,\t0,0,\t0,0,\t
46,0,\t48,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t1,6,\t1,7,\t1,8,\t
0,0,\t0,0,\t1,9,\t0,0,\t
0,0,\t1,10,\t1,11,\t0,0,\t
1,12,\t1,13,\t0,0,\t1,14,\t
0,0,\t1,15,\t1,16,\t1,16,\t
1,17,\t1,16,\t1,16,\t1,16,\t
1,16,\t1,16,\t0,0,\t0,0,\t
4,0,\t1,18,\t1,19,\t1,20,\t
46,48,\t48,48,\t1,3,\t1,3,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t1,3,\t0,0,\t
0,0,\t1,3,\t4,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t4,0,\t0,0,\t
0,0,\t0,0,\t4,0,\t1,21,\t
0,0,\t0,0,\t1,22,\t1,23,\t
1,24,\t1,25,\t1,26,\t1,27,\t
0,0,\t1,28,\t1,29,\t0,0,\t
4,44,\t1,30,\t1,31,\t1,32,\t
1,33,\t1,34,\t0,0,\t1,35,\t
1,36,\t1,37,\t1,38,\t2,7,\t
1,39,\t0,0,\t1,40,\t2,9,\t
1,41,\t0,0,\t1,42,\t2,11,\t
0,0,\t9,0,\t2,13,\t0,0,\t
2,14,\t0,0,\t0,0,\t2,16,\t
2,16,\t2,17,\t2,16,\t2,16,\t
2,16,\t2,16,\t0,0,\t0,0,\t
4,0,\t11,0,\t2,18,\t2,19,\t
2,20,\t0,0,\t0,0,\t9,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t9,0,\t
0,0,\t0,0,\t0,0,\t9,0,\t
0,0,\t0,0,\t0,0,\t11,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t3,43,\t0,0,\t11,0,\t
0,0,\t9,44,\t0,0,\t11,0,\t
2,21,\t3,43,\t3,0,\t2,22,\t
2,23,\t2,24,\t2,25,\t2,26,\t
2,27,\t0,0,\t2,28,\t2,29,\t
7,0,\t11,44,\t2,30,\t2,31,\t
2,32,\t2,33,\t2,34,\t0,0,\t
2,35,\t2,36,\t2,37,\t2,38,\t
3,0,\t2,39,\t3,43,\t2,40,\t
0,0,\t2,41,\t0,0,\t2,42,\t
3,0,\t9,0,\t7,0,\t3,43,\t
3,0,\t0,0,\t0,0,\t0,0,\t
3,43,\t0,0,\t7,0,\t8,46,\t
0,0,\t0,0,\t7,0,\t0,0,\t
3,43,\t11,0,\t3,44,\t8,46,\t
8,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t3,43,\t3,43,\t0,0,\t
7,44,\t0,0,\t0,0,\t7,45,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t3,43,\t12,49,\t0,0,\t
3,43,\t0,0,\t8,47,\t0,0,\t
8,43,\t0,0,\t12,49,\t12,0,\t
14,0,\t0,0,\t8,47,\t0,0,\t
0,0,\t8,46,\t3,0,\t0,0,\t
0,0,\t0,0,\t8,46,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
7,0,\t0,0,\t8,46,\t0,0,\t
8,48,\t12,0,\t14,0,\t12,49,\t
0,0,\t0,0,\t0,0,\t8,46,\t
8,46,\t12,0,\t14,0,\t18,0,\t
12,50,\t12,0,\t14,0,\t0,0,\t
0,0,\t12,49,\t14,52,\t8,46,\t
15,0,\t0,0,\t8,46,\t0,0,\t
0,0,\t12,49,\t14,52,\t12,51,\t
14,44,\t0,0,\t0,0,\t0,0,\t
0,0,\t18,0,\t12,49,\t12,49,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t18,0,\t15,0,\t16,0,\t
0,0,\t18,0,\t12,49,\t0,0,\t
0,0,\t12,49,\t15,0,\t0,0,\t
19,0,\t0,0,\t15,0,\t15,53,\t
15,54,\t0,0,\t15,55,\t18,44,\t
0,0,\t0,0,\t18,61,\t12,0,\t
14,0,\t16,0,\t15,56,\t0,0,\t
15,57,\t0,0,\t0,0,\t17,0,\t
0,0,\t16,0,\t19,0,\t15,58,\t
0,0,\t16,0,\t16,53,\t16,54,\t
20,0,\t16,56,\t19,0,\t0,0,\t
0,0,\t0,0,\t19,0,\t0,0,\t
0,0,\t16,56,\t15,58,\t16,57,\t
0,0,\t17,0,\t0,0,\t18,0,\t
0,0,\t0,0,\t16,58,\t22,0,\t
19,44,\t17,0,\t20,0,\t19,62,\t
15,0,\t17,0,\t17,53,\t17,54,\t
0,0,\t17,56,\t20,0,\t0,0,\t
23,0,\t16,58,\t20,0,\t0,0,\t
0,0,\t17,56,\t0,0,\t17,57,\t
0,0,\t22,0,\t0,0,\t24,0,\t
0,0,\t0,0,\t17,58,\t16,0,\t
20,44,\t22,0,\t15,59,\t20,63,\t
0,0,\t22,0,\t23,0,\t0,0,\t
19,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t17,58,\t23,0,\t25,0,\t
26,0,\t24,0,\t23,0,\t22,44,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t24,0,\t0,0,\t17,0,\t
0,0,\t24,0,\t0,0,\t0,0,\t
23,44,\t17,60,\t0,0,\t28,0,\t
20,0,\t25,0,\t26,0,\t0,0,\t
27,0,\t0,0,\t0,0,\t24,44,\t
0,0,\t25,0,\t26,0,\t0,0,\t
0,0,\t25,0,\t26,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t22,0,\t
0,0,\t28,0,\t0,0,\t0,0,\t
0,0,\t22,64,\t27,0,\t25,44,\t
26,44,\t28,0,\t0,0,\t0,0,\t
23,0,\t28,0,\t27,0,\t22,65,\t
0,0,\t0,0,\t27,0,\t23,68,\t
0,0,\t22,66,\t22,67,\t24,0,\t
0,0,\t29,0,\t24,70,\t28,44,\t
0,0,\t0,0,\t0,0,\t0,0,\t
27,44,\t24,71,\t0,0,\t30,0,\t
0,0,\t0,0,\t0,0,\t23,69,\t
24,72,\t0,0,\t31,0,\t25,0,\t
26,0,\t0,0,\t25,73,\t29,0,\t
32,0,\t0,0,\t25,74,\t0,0,\t
0,0,\t0,0,\t25,75,\t29,0,\t
0,0,\t30,0,\t0,0,\t29,0,\t
25,76,\t0,0,\t0,0,\t28,0,\t
31,0,\t30,0,\t26,77,\t33,0,\t
27,0,\t30,0,\t32,0,\t27,78,\t
31,0,\t29,44,\t0,0,\t0,0,\t
31,0,\t27,79,\t32,0,\t27,80,\t
28,81,\t0,0,\t32,0,\t30,44,\t
0,0,\t0,0,\t34,0,\t0,0,\t
0,0,\t33,0,\t31,44,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
32,44,\t33,0,\t0,0,\t36,0,\t
0,0,\t33,0,\t0,0,\t0,0,\t
0,0,\t35,0,\t0,0,\t0,0,\t
34,0,\t29,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t33,44,\t
34,0,\t0,0,\t0,0,\t30,0,\t
34,0,\t36,0,\t0,0,\t0,0,\t
37,0,\t29,82,\t31,0,\t35,0,\t
0,0,\t36,0,\t40,0,\t0,0,\t
32,0,\t36,0,\t34,44,\t35,0,\t
30,83,\t31,85,\t0,0,\t35,0,\t
30,84,\t0,0,\t0,0,\t31,86,\t
0,0,\t38,0,\t37,0,\t36,44,\t
31,87,\t32,88,\t0,0,\t33,0,\t
40,0,\t35,44,\t37,0,\t0,0,\t
0,0,\t0,0,\t37,0,\t0,0,\t
40,0,\t0,0,\t0,0,\t0,0,\t
40,0,\t39,0,\t0,0,\t38,0,\t
0,0,\t0,0,\t34,0,\t33,89,\t
37,44,\t0,0,\t0,0,\t38,0,\t
33,90,\t0,0,\t40,44,\t38,0,\t
0,0,\t0,0,\t0,0,\t36,0,\t
0,0,\t0,0,\t0,0,\t39,0,\t
0,0,\t35,0,\t34,91,\t0,0,\t
0,0,\t38,44,\t36,94,\t39,0,\t
35,92,\t0,0,\t0,0,\t39,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
35,93,\t36,95,\t36,96,\t0,0,\t
37,0,\t41,0,\t42,0,\t0,0,\t
0,0,\t39,44,\t40,0,\t0,0,\t
0,0,\t0,0,\t37,97,\t37,98,\t
0,0,\t40,108,\t0,0,\t0,0,\t
0,0,\t37,99,\t0,0,\t0,0,\t
37,100,\t38,0,\t37,101,\t41,0,\t
42,0,\t43,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t41,0,\t
42,0,\t0,0,\t44,0,\t41,0,\t
42,0,\t38,102,\t0,0,\t38,103,\t
45,0,\t39,0,\t38,104,\t38,105,\t
0,0,\t0,0,\t0,0,\t43,0,\t
39,106,\t41,44,\t42,44,\t0,0,\t
39,107,\t0,0,\t0,0,\t43,0,\t
44,0,\t0,0,\t47,47,\t43,0,\t
0,0,\t0,0,\t45,0,\t0,0,\t
44,0,\t0,0,\t47,47,\t47,0,\t
44,0,\t0,0,\t45,0,\t0,0,\t
50,0,\t43,44,\t45,0,\t0,0,\t
0,0,\t49,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t44,44,\t0,0,\t
0,0,\t41,0,\t42,0,\t0,0,\t
45,44,\t47,47,\t0,0,\t47,0,\t
0,0,\t0,0,\t50,0,\t0,0,\t
0,0,\t47,47,\t0,0,\t49,0,\t
47,47,\t0,0,\t50,0,\t0,0,\t
0,0,\t47,47,\t50,0,\t49,0,\t
0,0,\t43,0,\t49,49,\t49,0,\t
0,0,\t47,47,\t51,0,\t0,0,\t
0,0,\t0,0,\t44,0,\t52,0,\t
50,51,\t0,0,\t47,47,\t47,47,\t
45,0,\t49,51,\t0,0,\t0,0,\t
0,0,\t0,0,\t53,0,\t0,0,\t
0,0,\t0,0,\t47,47,\t0,0,\t
51,0,\t47,47,\t0,0,\t0,0,\t
0,0,\t52,0,\t0,0,\t0,0,\t
51,0,\t0,0,\t0,0,\t51,49,\t
51,0,\t52,0,\t0,0,\t54,0,\t
53,0,\t52,0,\t0,0,\t0,0,\t
50,0,\t52,52,\t55,0,\t0,0,\t
53,0,\t49,0,\t51,51,\t0,0,\t
53,0,\t52,52,\t0,0,\t52,44,\t
53,109,\t0,0,\t0,0,\t0,0,\t
0,0,\t54,0,\t0,0,\t0,0,\t
53,109,\t0,0,\t53,44,\t0,0,\t
55,0,\t54,0,\t0,0,\t0,0,\t
0,0,\t54,0,\t56,0,\t0,0,\t
55,0,\t54,52,\t0,0,\t0,0,\t
55,0,\t55,53,\t55,54,\t0,0,\t
55,55,\t54,52,\t51,0,\t54,44,\t
0,0,\t0,0,\t0,0,\t52,0,\t
55,56,\t0,0,\t55,57,\t0,0,\t
56,0,\t57,0,\t0,0,\t0,0,\t
0,0,\t55,58,\t53,0,\t58,0,\t
56,0,\t0,0,\t0,0,\t0,0,\t
56,0,\t56,53,\t56,54,\t0,0,\t
56,56,\t0,0,\t0,0,\t0,0,\t
55,58,\t0,0,\t59,0,\t57,0,\t
56,56,\t0,0,\t56,57,\t54,0,\t
0,0,\t58,0,\t0,0,\t57,0,\t
0,0,\t56,58,\t55,0,\t57,0,\t
0,0,\t58,0,\t0,0,\t57,110,\t
0,0,\t58,0,\t60,0,\t0,0,\t
59,0,\t61,0,\t0,0,\t57,110,\t
56,58,\t57,44,\t0,0,\t0,0,\t
59,0,\t0,0,\t0,0,\t58,44,\t
59,0,\t0,0,\t0,0,\t0,0,\t
59,112,\t0,0,\t56,0,\t0,0,\t
60,0,\t62,0,\t63,0,\t61,0,\t
59,112,\t0,0,\t59,44,\t0,0,\t
60,0,\t0,0,\t58,111,\t61,0,\t
60,0,\t59,112,\t59,112,\t61,0,\t
0,0,\t0,0,\t64,0,\t0,0,\t
0,0,\t57,0,\t0,0,\t62,0,\t
63,0,\t65,0,\t60,44,\t58,0,\t
0,0,\t61,44,\t0,0,\t62,0,\t
63,0,\t0,0,\t66,0,\t62,0,\t
63,0,\t0,0,\t0,0,\t0,0,\t
64,0,\t0,0,\t59,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t65,0,\t
64,0,\t62,44,\t63,44,\t0,0,\t
64,0,\t67,0,\t0,0,\t65,0,\t
66,0,\t0,0,\t68,0,\t65,0,\t
0,0,\t0,0,\t60,0,\t0,0,\t
66,0,\t61,0,\t64,44,\t0,0,\t
66,0,\t69,0,\t60,113,\t0,0,\t
0,0,\t65,44,\t0,0,\t67,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
68,0,\t0,0,\t66,44,\t67,0,\t
0,0,\t62,0,\t63,0,\t67,0,\t
68,0,\t0,0,\t70,0,\t69,0,\t
68,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t71,0,\t0,0,\t69,0,\t
0,0,\t67,44,\t64,0,\t69,0,\t
0,0,\t0,0,\t68,44,\t0,0,\t
64,114,\t65,0,\t72,0,\t0,0,\t
70,0,\t73,0,\t0,0,\t65,115,\t
0,0,\t69,44,\t66,0,\t71,0,\t
70,0,\t0,0,\t0,0,\t0,0,\t
70,0,\t65,116,\t0,0,\t71,0,\t
0,0,\t66,117,\t0,0,\t71,0,\t
72,0,\t0,0,\t0,0,\t73,0,\t
0,0,\t67,0,\t70,44,\t0,0,\t
72,0,\t74,0,\t68,0,\t73,0,\t
72,0,\t71,44,\t0,0,\t73,0,\t
75,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t69,0,\t0,0,\t0,0,\t
0,0,\t76,0,\t72,44,\t67,118,\t
0,0,\t73,44,\t0,0,\t74,0,\t
68,119,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t75,0,\t74,0,\t
0,0,\t77,0,\t70,0,\t74,0,\t
78,0,\t0,0,\t75,0,\t76,0,\t
0,0,\t71,0,\t75,0,\t0,0,\t
71,121,\t0,0,\t0,0,\t76,0,\t
0,0,\t74,44,\t0,0,\t76,0,\t
70,120,\t0,0,\t72,0,\t77,0,\t
75,44,\t73,0,\t78,0,\t0,0,\t
79,0,\t80,0,\t0,0,\t77,0,\t
0,0,\t76,44,\t78,0,\t77,0,\t
73,123,\t0,0,\t78,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
81,0,\t72,122,\t0,0,\t0,0,\t
0,0,\t77,44,\t79,0,\t80,0,\t
78,44,\t74,0,\t0,0,\t0,0,\t
0,0,\t82,0,\t79,0,\t80,0,\t
75,0,\t74,124,\t79,0,\t80,0,\t
0,0,\t0,0,\t81,0,\t83,0,\t
0,0,\t76,0,\t76,126,\t0,0,\t
0,0,\t84,0,\t81,0,\t0,0,\t
79,44,\t80,44,\t81,0,\t82,0,\t
75,125,\t0,0,\t0,0,\t0,0,\t
85,0,\t77,0,\t86,0,\t82,0,\t
78,0,\t83,0,\t0,0,\t82,0,\t
81,44,\t0,0,\t0,0,\t84,0,\t
77,127,\t83,0,\t0,0,\t0,0,\t
0,0,\t83,0,\t78,128,\t84,0,\t
0,0,\t82,44,\t85,0,\t84,0,\t
86,0,\t0,0,\t0,0,\t0,0,\t
79,0,\t80,0,\t85,0,\t83,44,\t
86,0,\t0,0,\t85,0,\t0,0,\t
86,0,\t84,44,\t0,0,\t0,0,\t
0,0,\t87,0,\t0,0,\t80,130,\t
81,0,\t0,0,\t0,0,\t88,0,\t
85,44,\t79,129,\t86,44,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t82,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t87,0,\t
0,0,\t81,131,\t89,0,\t83,0,\t
0,0,\t88,0,\t83,132,\t87,0,\t
83,133,\t84,0,\t0,0,\t87,0,\t
83,134,\t88,0,\t0,0,\t90,0,\t
0,0,\t88,0,\t91,0,\t0,0,\t
85,0,\t92,0,\t86,0,\t0,0,\t
89,0,\t87,44,\t85,136,\t0,0,\t
86,137,\t0,0,\t84,135,\t88,44,\t
89,0,\t0,0,\t93,0,\t0,0,\t
89,0,\t90,0,\t86,138,\t0,0,\t
91,0,\t0,0,\t0,0,\t92,0,\t
0,0,\t90,0,\t0,0,\t0,0,\t
91,0,\t90,0,\t89,44,\t92,0,\t
91,0,\t94,0,\t0,0,\t92,0,\t
93,0,\t0,0,\t95,0,\t96,0,\t
0,0,\t87,0,\t0,0,\t90,44,\t
93,0,\t0,0,\t91,44,\t88,0,\t
93,0,\t92,44,\t0,0,\t0,0,\t
87,139,\t0,0,\t0,0,\t94,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
95,0,\t96,0,\t93,44,\t94,0,\t
88,140,\t0,0,\t89,0,\t94,0,\t
95,0,\t96,0,\t97,0,\t0,0,\t
95,0,\t96,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t90,0,\t
98,0,\t94,44,\t91,0,\t0,0,\t
0,0,\t92,0,\t95,44,\t96,44,\t
92,143,\t0,0,\t99,0,\t0,0,\t
97,0,\t0,0,\t92,144,\t90,141,\t
0,0,\t0,0,\t93,0,\t91,142,\t
97,0,\t93,146,\t98,0,\t92,145,\t
97,0,\t0,0,\t0,0,\t100,0,\t
0,0,\t101,0,\t98,0,\t0,0,\t
99,0,\t0,0,\t98,0,\t93,147,\t
0,0,\t94,0,\t97,44,\t93,148,\t
99,0,\t0,0,\t95,0,\t96,0,\t
99,0,\t95,150,\t0,0,\t0,0,\t
98,44,\t100,0,\t0,0,\t101,0,\t
0,0,\t95,151,\t0,0,\t0,0,\t
0,0,\t100,0,\t99,44,\t101,0,\t
0,0,\t100,0,\t0,0,\t101,0,\t
96,152,\t94,149,\t0,0,\t102,0,\t
103,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t104,0,\t97,0,\t100,44,\t
0,0,\t101,44,\t0,0,\t0,0,\t
0,0,\t97,153,\t0,0,\t0,0,\t
98,0,\t0,0,\t0,0,\t0,0,\t
105,0,\t102,0,\t103,0,\t0,0,\t
0,0,\t0,0,\t99,0,\t104,0,\t
0,0,\t102,0,\t103,0,\t98,154,\t
99,155,\t102,0,\t103,0,\t104,0,\t
0,0,\t106,0,\t0,0,\t104,0,\t
0,0,\t99,156,\t105,0,\t100,0,\t
0,0,\t101,0,\t0,0,\t102,44,\t
103,44,\t0,0,\t105,0,\t0,0,\t
107,0,\t104,44,\t105,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t106,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t108,0,\t100,157,\t106,0,\t
105,44,\t0,0,\t0,0,\t106,0,\t
101,158,\t0,0,\t107,0,\t0,0,\t
109,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t107,0,\t102,0,\t
103,0,\t106,44,\t107,0,\t108,0,\t
102,159,\t104,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t108,0,\t
104,161,\t0,0,\t109,0,\t108,0,\t
107,44,\t0,0,\t0,0,\t0,0,\t
105,0,\t102,160,\t109,0,\t110,0,\t
111,0,\t0,0,\t109,0,\t0,0,\t
0,0,\t108,44,\t109,109,\t105,162,\t
0,0,\t113,0,\t0,0,\t0,0,\t
0,0,\t106,0,\t109,109,\t0,0,\t
109,44,\t0,0,\t0,0,\t0,0,\t
106,163,\t110,0,\t111,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
107,0,\t110,0,\t111,0,\t113,0,\t
0,0,\t110,0,\t111,0,\t0,0,\t
0,0,\t110,110,\t0,0,\t113,0,\t
112,0,\t108,0,\t114,0,\t113,0,\t
108,165,\t110,110,\t0,0,\t110,44,\t
111,44,\t0,0,\t107,164,\t115,0,\t
109,0,\t0,0,\t110,166,\t0,0,\t
0,0,\t113,44,\t0,0,\t0,0,\t
0,0,\t0,0,\t112,0,\t0,0,\t
114,0,\t0,0,\t0,0,\t116,0,\t
0,0,\t110,166,\t112,0,\t0,0,\t
114,0,\t115,0,\t112,0,\t0,0,\t
114,0,\t0,0,\t112,112,\t0,0,\t
117,0,\t115,0,\t0,0,\t110,0,\t
111,0,\t115,0,\t112,112,\t0,0,\t
112,44,\t116,0,\t114,44,\t118,0,\t
0,0,\t113,0,\t0,0,\t112,112,\t
112,112,\t116,0,\t0,0,\t115,44,\t
119,0,\t116,0,\t117,0,\t0,0,\t
113,167,\t0,0,\t0,0,\t120,0,\t
0,0,\t0,0,\t117,0,\t0,0,\t
0,0,\t118,0,\t117,0,\t116,44,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t118,0,\t119,0,\t121,0,\t
112,0,\t118,0,\t114,0,\t0,0,\t
117,44,\t120,0,\t119,0,\t0,0,\t
122,0,\t0,0,\t119,0,\t115,0,\t
0,0,\t120,0,\t0,0,\t118,44,\t
0,0,\t120,0,\t0,0,\t0,0,\t
0,0,\t121,0,\t0,0,\t114,168,\t
119,44,\t0,0,\t0,0,\t116,0,\t
123,0,\t121,0,\t122,0,\t120,44,\t
0,0,\t121,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t122,0,\t0,0,\t
117,0,\t124,0,\t122,0,\t125,0,\t
116,169,\t0,0,\t0,0,\t121,44,\t
0,0,\t0,0,\t123,0,\t118,0,\t
126,0,\t0,0,\t0,0,\t117,170,\t
122,44,\t0,0,\t123,0,\t0,0,\t
119,0,\t118,171,\t123,0,\t124,0,\t
0,0,\t125,0,\t0,0,\t120,0,\t
0,0,\t0,0,\t120,173,\t124,0,\t
0,0,\t125,0,\t126,0,\t124,0,\t
123,44,\t125,0,\t0,0,\t0,0,\t
0,0,\t127,0,\t126,0,\t121,0,\t
128,0,\t119,172,\t126,0,\t0,0,\t
0,0,\t124,44,\t0,0,\t125,44,\t
122,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t121,174,\t
126,44,\t129,0,\t0,0,\t127,0,\t
0,0,\t0,0,\t128,0,\t0,0,\t
122,175,\t0,0,\t0,0,\t127,0,\t
123,0,\t0,0,\t128,0,\t127,0,\t
130,0,\t0,0,\t128,0,\t0,0,\t
0,0,\t131,0,\t0,0,\t129,0,\t
0,0,\t124,0,\t123,176,\t125,0,\t
124,177,\t127,44,\t0,0,\t129,0,\t
128,44,\t0,0,\t0,0,\t129,0,\t
126,0,\t0,0,\t130,0,\t0,0,\t
132,0,\t0,0,\t124,178,\t131,0,\t
0,0,\t133,0,\t130,0,\t0,0,\t
0,0,\t129,44,\t130,0,\t131,0,\t
126,179,\t0,0,\t0,0,\t131,0,\t
126,180,\t0,0,\t0,0,\t0,0,\t
134,0,\t135,0,\t132,0,\t0,0,\t
130,44,\t127,0,\t0,0,\t133,0,\t
128,0,\t131,44,\t132,0,\t0,0,\t
0,0,\t0,0,\t132,0,\t133,0,\t
0,0,\t0,0,\t0,0,\t133,0,\t
127,181,\t0,0,\t134,0,\t135,0,\t
0,0,\t129,0,\t0,0,\t0,0,\t
132,44,\t128,182,\t134,0,\t135,0,\t
136,0,\t133,44,\t134,0,\t135,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
130,0,\t138,0,\t0,0,\t129,183,\t
0,0,\t131,0,\t0,0,\t130,184,\t
134,44,\t135,44,\t0,0,\t0,0,\t
137,0,\t0,0,\t136,0,\t0,0,\t
0,0,\t139,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t136,0,\t138,0,\t
132,0,\t0,0,\t136,0,\t131,185,\t
0,0,\t133,0,\t132,186,\t138,0,\t
133,187,\t0,0,\t137,0,\t138,0,\t
140,0,\t0,0,\t0,0,\t139,0,\t
136,44,\t0,0,\t137,0,\t0,0,\t
134,0,\t135,0,\t137,0,\t139,0,\t
0,0,\t138,44,\t0,0,\t139,0,\t
0,0,\t134,188,\t0,0,\t0,0,\t
141,0,\t0,0,\t140,0,\t0,0,\t
137,44,\t142,0,\t0,0,\t135,189,\t
0,0,\t139,44,\t140,0,\t0,0,\t
0,0,\t0,0,\t140,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
136,0,\t0,0,\t141,0,\t0,0,\t
143,0,\t0,0,\t0,0,\t142,0,\t
140,44,\t138,0,\t141,0,\t0,0,\t
0,0,\t144,0,\t141,0,\t142,0,\t
136,190,\t0,0,\t0,0,\t142,0,\t
137,0,\t0,0,\t0,0,\t0,0,\t
145,0,\t139,0,\t143,0,\t137,191,\t
141,44,\t137,192,\t0,0,\t138,193,\t
0,0,\t142,44,\t143,0,\t144,0,\t
0,0,\t0,0,\t143,0,\t0,0,\t
139,194,\t0,0,\t0,0,\t144,0,\t
140,0,\t146,0,\t145,0,\t144,0,\t
147,0,\t0,0,\t0,0,\t148,0,\t
143,44,\t0,0,\t145,0,\t0,0,\t
0,0,\t0,0,\t145,0,\t0,0,\t
140,195,\t144,44,\t0,0,\t0,0,\t
141,0,\t0,0,\t0,0,\t146,0,\t
149,0,\t142,0,\t147,0,\t141,196,\t
145,44,\t148,0,\t142,197,\t146,0,\t
0,0,\t0,0,\t147,0,\t146,0,\t
0,0,\t148,0,\t147,0,\t150,0,\t
0,0,\t148,0,\t0,0,\t0,0,\t
143,0,\t151,0,\t149,0,\t0,0,\t
152,0,\t146,44,\t143,198,\t0,0,\t
147,44,\t144,0,\t149,0,\t148,44,\t
0,0,\t0,0,\t149,0,\t0,0,\t
0,0,\t150,0,\t0,0,\t0,0,\t
145,0,\t0,0,\t0,0,\t151,0,\t
153,0,\t150,0,\t152,0,\t145,200,\t
149,44,\t150,0,\t0,0,\t151,0,\t
0,0,\t0,0,\t152,0,\t151,0,\t
144,199,\t0,0,\t152,0,\t154,0,\t
0,0,\t146,0,\t0,0,\t150,44,\t
147,0,\t0,0,\t153,0,\t148,0,\t
147,202,\t151,44,\t0,0,\t0,0,\t
152,44,\t0,0,\t153,0,\t0,0,\t
155,0,\t0,0,\t153,0,\t0,0,\t
0,0,\t154,0,\t156,0,\t146,201,\t
149,0,\t148,203,\t0,0,\t0,0,\t
0,0,\t154,0,\t0,0,\t149,204,\t
153,44,\t154,0,\t0,0,\t0,0,\t
0,0,\t157,0,\t155,0,\t150,0,\t
0,0,\t0,0,\t0,0,\t158,0,\t
156,0,\t151,0,\t155,0,\t154,44,\t
152,0,\t0,0,\t155,0,\t0,0,\t
156,0,\t0,0,\t0,0,\t0,0,\t
156,0,\t0,0,\t0,0,\t157,0,\t
151,206,\t150,205,\t0,0,\t0,0,\t
155,44,\t158,0,\t152,207,\t157,0,\t
153,0,\t0,0,\t156,44,\t157,0,\t
0,0,\t158,0,\t159,0,\t0,0,\t
0,0,\t158,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t154,0,\t
153,208,\t157,44,\t0,0,\t0,0,\t
0,0,\t0,0,\t154,209,\t158,44,\t
0,0,\t0,0,\t0,0,\t160,0,\t
159,0,\t0,0,\t0,0,\t0,0,\t
155,0,\t0,0,\t161,0,\t155,210,\t
159,0,\t0,0,\t156,0,\t0,0,\t
159,0,\t0,0,\t0,0,\t162,0,\t
163,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t160,0,\t164,0,\t0,0,\t
0,0,\t157,0,\t159,44,\t156,211,\t
161,0,\t160,0,\t0,0,\t158,0,\t
157,212,\t160,0,\t0,0,\t0,0,\t
161,0,\t162,0,\t163,0,\t165,0,\t
161,0,\t0,0,\t0,0,\t0,0,\t
164,0,\t162,0,\t163,0,\t160,44,\t
0,0,\t162,0,\t163,0,\t0,0,\t
164,0,\t0,0,\t161,44,\t166,0,\t
164,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t165,0,\t159,0,\t162,44,\t
163,44,\t0,0,\t0,0,\t0,0,\t
0,0,\t165,0,\t164,44,\t167,0,\t
159,213,\t165,0,\t0,0,\t0,0,\t
0,0,\t166,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t160,0,\t
0,0,\t166,0,\t0,0,\t165,44,\t
0,0,\t166,0,\t161,0,\t0,0,\t
0,0,\t167,0,\t160,214,\t0,0,\t
168,0,\t0,0,\t169,0,\t162,0,\t
163,0,\t167,0,\t0,0,\t166,44,\t
0,0,\t167,0,\t164,0,\t0,0,\t
0,0,\t0,0,\t161,215,\t170,0,\t
0,0,\t163,217,\t162,216,\t0,0,\t
164,218,\t0,0,\t168,0,\t167,44,\t
169,0,\t0,0,\t166,220,\t165,0,\t
0,0,\t0,0,\t168,0,\t171,0,\t
169,0,\t0,0,\t168,0,\t0,0,\t
169,0,\t170,0,\t0,0,\t0,0,\t
172,0,\t0,0,\t0,0,\t166,0,\t
0,0,\t170,0,\t173,0,\t165,219,\t
168,44,\t170,0,\t169,44,\t0,0,\t
0,0,\t171,0,\t0,0,\t174,0,\t
0,0,\t0,0,\t0,0,\t167,0,\t
0,0,\t171,0,\t172,0,\t170,44,\t
0,0,\t171,0,\t0,0,\t0,0,\t
173,0,\t0,0,\t172,0,\t175,0,\t
0,0,\t167,221,\t172,0,\t0,0,\t
173,0,\t174,0,\t176,0,\t171,44,\t
173,0,\t0,0,\t0,0,\t0,0,\t
168,0,\t174,0,\t169,0,\t0,0,\t
172,44,\t174,0,\t0,0,\t0,0,\t
0,0,\t175,0,\t173,44,\t177,0,\t
0,0,\t0,0,\t0,0,\t170,0,\t
176,0,\t175,0,\t168,222,\t174,44,\t
0,0,\t175,0,\t170,224,\t0,0,\t
176,0,\t169,223,\t0,0,\t178,0,\t
176,0,\t0,0,\t0,0,\t171,0,\t
179,0,\t177,0,\t171,225,\t175,44,\t
0,0,\t0,0,\t180,0,\t0,0,\t
172,0,\t177,0,\t176,44,\t181,0,\t
0,0,\t177,0,\t173,0,\t172,227,\t
171,226,\t178,0,\t0,0,\t173,228,\t
0,0,\t0,0,\t179,0,\t174,0,\t
0,0,\t178,0,\t0,0,\t177,44,\t
180,0,\t178,0,\t179,0,\t0,0,\t
174,229,\t181,0,\t179,0,\t0,0,\t
180,0,\t0,0,\t182,0,\t175,0,\t
180,0,\t181,0,\t0,0,\t178,44,\t
183,0,\t181,0,\t176,0,\t0,0,\t
179,44,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t180,44,\t0,0,\t
0,0,\t0,0,\t0,0,\t181,44,\t
182,0,\t175,230,\t0,0,\t177,0,\t
0,0,\t0,0,\t183,0,\t0,0,\t
182,0,\t184,0,\t0,0,\t0,0,\t
182,0,\t176,231,\t183,0,\t185,0,\t
186,0,\t0,0,\t183,0,\t178,0,\t
0,0,\t0,0,\t187,0,\t178,233,\t
179,0,\t177,232,\t182,44,\t0,0,\t
0,0,\t0,0,\t180,0,\t184,0,\t
183,44,\t0,0,\t0,0,\t181,0,\t
0,0,\t185,0,\t186,0,\t184,0,\t
0,0,\t179,234,\t181,236,\t184,0,\t
187,0,\t185,0,\t186,0,\t0,0,\t
180,235,\t185,0,\t186,0,\t0,0,\t
187,0,\t0,0,\t188,0,\t189,0,\t
187,0,\t184,44,\t0,0,\t0,0,\t
190,0,\t0,0,\t182,0,\t185,44,\t
186,44,\t0,0,\t0,0,\t0,0,\t
183,0,\t182,237,\t187,44,\t0,0,\t
0,0,\t0,0,\t0,0,\t183,238,\t
188,0,\t189,0,\t0,0,\t191,0,\t
0,0,\t0,0,\t190,0,\t0,0,\t
188,0,\t189,0,\t0,0,\t0,0,\t
188,0,\t189,0,\t190,0,\t0,0,\t
0,0,\t184,0,\t190,0,\t192,0,\t
0,0,\t0,0,\t0,0,\t185,0,\t
186,0,\t191,0,\t188,44,\t189,44,\t
0,0,\t0,0,\t187,0,\t0,0,\t
190,44,\t191,0,\t0,0,\t0,0,\t
0,0,\t191,0,\t184,239,\t0,0,\t
0,0,\t192,0,\t193,0,\t194,0,\t
195,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t192,0,\t0,0,\t191,44,\t
187,240,\t192,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t196,0,\t
0,0,\t0,0,\t188,0,\t189,0,\t
193,0,\t194,0,\t195,0,\t192,44,\t
190,0,\t188,241,\t189,242,\t0,0,\t
193,0,\t194,0,\t195,0,\t0,0,\t
193,0,\t194,0,\t195,0,\t190,243,\t
0,0,\t196,0,\t197,0,\t198,0,\t
199,0,\t0,0,\t0,0,\t191,0,\t
0,0,\t196,0,\t193,44,\t194,44,\t
195,44,\t196,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t200,0,\t
0,0,\t0,0,\t0,0,\t192,0,\t
197,0,\t198,0,\t199,0,\t196,44,\t
0,0,\t0,0,\t0,0,\t0,0,\t
197,0,\t198,0,\t199,0,\t0,0,\t
197,0,\t198,0,\t199,0,\t0,0,\t
0,0,\t200,0,\t201,0,\t192,244,\t
202,0,\t0,0,\t193,0,\t194,0,\t
195,0,\t200,0,\t197,44,\t198,44,\t
199,44,\t200,0,\t194,246,\t0,0,\t
193,245,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t196,0,\t
201,0,\t0,0,\t202,0,\t200,44,\t
0,0,\t0,0,\t0,0,\t0,0,\t
201,0,\t0,0,\t202,0,\t203,0,\t
201,0,\t204,0,\t202,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t196,247,\t
0,0,\t0,0,\t197,0,\t198,0,\t
199,0,\t0,0,\t201,44,\t0,0,\t
202,44,\t197,248,\t0,0,\t199,250,\t
0,0,\t203,0,\t0,0,\t204,0,\t
0,0,\t0,0,\t205,0,\t200,0,\t
0,0,\t203,0,\t200,251,\t204,0,\t
198,249,\t203,0,\t0,0,\t204,0,\t
206,0,\t0,0,\t0,0,\t207,0,\t
0,0,\t208,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t209,0,\t203,44,\t
205,0,\t204,44,\t201,0,\t0,0,\t
202,0,\t0,0,\t0,0,\t0,0,\t
205,0,\t0,0,\t206,0,\t0,0,\t
205,0,\t207,0,\t0,0,\t208,0,\t
202,253,\t0,0,\t206,0,\t0,0,\t
209,0,\t207,0,\t206,0,\t208,0,\t
201,252,\t207,0,\t205,44,\t208,0,\t
209,0,\t0,0,\t210,0,\t0,0,\t
209,0,\t211,0,\t0,0,\t203,0,\t
206,44,\t204,0,\t0,0,\t207,44,\t
0,0,\t208,44,\t203,254,\t0,0,\t
0,0,\t0,0,\t209,44,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
210,0,\t0,0,\t212,0,\t211,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
210,0,\t213,0,\t205,0,\t211,0,\t
210,0,\t0,0,\t0,0,\t211,0,\t
0,0,\t0,0,\t214,0,\t215,0,\t
206,0,\t0,0,\t0,0,\t207,0,\t
212,0,\t208,0,\t210,44,\t206,256,\t
0,0,\t211,44,\t209,0,\t213,0,\t
212,0,\t205,255,\t216,0,\t0,0,\t
212,0,\t0,0,\t0,0,\t213,0,\t
214,0,\t215,0,\t0,0,\t213,0,\t
207,257,\t0,0,\t0,0,\t217,0,\t
214,0,\t215,0,\t212,44,\t209,258,\t
214,0,\t215,0,\t0,0,\t0,0,\t
216,0,\t213,44,\t218,0,\t0,0,\t
0,0,\t0,0,\t210,0,\t0,0,\t
216,0,\t211,0,\t214,44,\t215,44,\t
216,0,\t217,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t219,0,\t
0,0,\t217,0,\t220,0,\t0,0,\t
218,0,\t217,0,\t216,44,\t0,0,\t
0,0,\t211,260,\t212,0,\t0,0,\t
218,0,\t210,259,\t0,0,\t0,0,\t
218,0,\t213,0,\t0,0,\t217,44,\t
213,261,\t219,0,\t221,0,\t222,0,\t
220,0,\t0,0,\t214,0,\t215,0,\t
0,0,\t219,0,\t218,44,\t0,0,\t
220,0,\t219,0,\t0,0,\t0,0,\t
220,0,\t0,0,\t0,0,\t0,0,\t
214,262,\t0,0,\t216,0,\t0,0,\t
221,0,\t222,0,\t223,0,\t219,44,\t
215,263,\t216,264,\t220,44,\t224,0,\t
221,0,\t222,0,\t0,0,\t217,0,\t
221,0,\t222,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t218,0,\t0,0,\t
223,0,\t217,265,\t221,44,\t222,44,\t
0,0,\t224,0,\t225,0,\t0,0,\t
223,0,\t218,266,\t0,0,\t0,0,\t
223,0,\t224,0,\t0,0,\t219,0,\t
0,0,\t224,0,\t220,0,\t0,0,\t
0,0,\t226,0,\t227,0,\t0,0,\t
0,0,\t0,0,\t223,44,\t228,0,\t
225,0,\t219,267,\t0,0,\t224,44,\t
0,0,\t0,0,\t0,0,\t0,0,\t
225,0,\t0,0,\t221,0,\t222,0,\t
225,0,\t0,0,\t0,0,\t226,0,\t
227,0,\t221,268,\t222,269,\t0,0,\t
0,0,\t228,0,\t229,0,\t226,0,\t
227,0,\t0,0,\t225,44,\t226,0,\t
227,0,\t228,0,\t0,0,\t230,0,\t
0,0,\t228,0,\t223,0,\t0,0,\t
0,0,\t0,0,\t231,0,\t224,0,\t
0,0,\t226,44,\t227,44,\t0,0,\t
229,0,\t0,0,\t0,0,\t228,44,\t
0,0,\t0,0,\t223,270,\t0,0,\t
229,0,\t230,0,\t0,0,\t0,0,\t
229,0,\t0,0,\t0,0,\t0,0,\t
231,0,\t230,0,\t225,0,\t0,0,\t
0,0,\t230,0,\t232,0,\t0,0,\t
231,0,\t233,0,\t229,44,\t234,0,\t
231,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t226,0,\t227,0,\t230,44,\t
0,0,\t226,272,\t235,0,\t228,0,\t
225,271,\t227,273,\t231,44,\t0,0,\t
232,0,\t0,0,\t0,0,\t233,0,\t
0,0,\t234,0,\t228,274,\t0,0,\t
232,0,\t0,0,\t236,0,\t233,0,\t
232,0,\t234,0,\t0,0,\t233,0,\t
235,0,\t234,0,\t229,0,\t0,0,\t
0,0,\t237,0,\t0,0,\t238,0,\t
235,0,\t229,275,\t232,44,\t230,0,\t
235,0,\t233,44,\t0,0,\t234,44,\t
236,0,\t0,0,\t231,0,\t0,0,\t
0,0,\t0,0,\t239,0,\t0,0,\t
236,0,\t0,0,\t235,44,\t237,0,\t
236,0,\t238,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t237,0,\t
0,0,\t238,0,\t0,0,\t237,0,\t
0,0,\t238,0,\t236,44,\t0,0,\t
239,0,\t0,0,\t232,0,\t240,0,\t
0,0,\t233,0,\t241,0,\t234,0,\t
239,0,\t237,44,\t0,0,\t238,44,\t
239,0,\t0,0,\t0,0,\t0,0,\t
242,0,\t233,277,\t235,0,\t243,0,\t
0,0,\t0,0,\t0,0,\t235,279,\t
232,276,\t240,0,\t239,44,\t0,0,\t
241,0,\t234,278,\t0,0,\t0,0,\t
0,0,\t240,0,\t236,0,\t244,0,\t
241,0,\t240,0,\t242,0,\t0,0,\t
241,0,\t243,0,\t0,0,\t0,0,\t
245,0,\t237,0,\t242,0,\t238,0,\t
0,0,\t243,0,\t242,0,\t240,44,\t
238,280,\t243,0,\t241,44,\t0,0,\t
0,0,\t244,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t239,0,\t246,0,\t
242,44,\t244,0,\t245,0,\t243,44,\t
0,0,\t244,0,\t0,0,\t0,0,\t
247,0,\t0,0,\t245,0,\t0,0,\t
0,0,\t0,0,\t245,0,\t248,0,\t
0,0,\t0,0,\t0,0,\t244,44,\t
0,0,\t246,0,\t0,0,\t240,0,\t
249,0,\t239,281,\t241,0,\t0,0,\t
245,44,\t246,0,\t247,0,\t250,0,\t
241,283,\t246,0,\t240,282,\t0,0,\t
242,0,\t248,0,\t247,0,\t243,0,\t
251,0,\t242,284,\t247,0,\t0,0,\t
0,0,\t248,0,\t249,0,\t246,44,\t
243,285,\t248,0,\t0,0,\t0,0,\t
0,0,\t250,0,\t249,0,\t244,0,\t
247,44,\t0,0,\t249,0,\t252,0,\t
0,0,\t250,0,\t251,0,\t248,44,\t
245,0,\t250,0,\t0,0,\t0,0,\t
253,0,\t0,0,\t251,0,\t0,0,\t
249,44,\t244,286,\t251,0,\t254,0,\t
0,0,\t0,0,\t245,287,\t250,44,\t
0,0,\t252,0,\t0,0,\t246,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
251,44,\t252,0,\t253,0,\t255,0,\t
247,0,\t252,0,\t0,0,\t0,0,\t
0,0,\t254,0,\t253,0,\t248,0,\t
256,0,\t0,0,\t253,0,\t0,0,\t
0,0,\t254,0,\t257,0,\t252,44,\t
249,0,\t254,0,\t0,0,\t0,0,\t
0,0,\t255,0,\t0,0,\t250,0,\t
253,44,\t0,0,\t250,290,\t258,0,\t
248,288,\t255,0,\t256,0,\t254,44,\t
251,0,\t255,0,\t249,289,\t0,0,\t
257,0,\t0,0,\t256,0,\t0,0,\t
0,0,\t0,0,\t256,0,\t259,0,\t
257,0,\t0,0,\t260,0,\t255,44,\t
257,0,\t258,0,\t0,0,\t252,0,\t
261,0,\t0,0,\t251,291,\t0,0,\t
256,44,\t258,0,\t0,0,\t262,0,\t
253,0,\t258,0,\t257,44,\t0,0,\t
0,0,\t259,0,\t0,0,\t254,0,\t
260,0,\t0,0,\t0,0,\t252,292,\t
254,293,\t259,0,\t261,0,\t258,44,\t
260,0,\t259,0,\t0,0,\t0,0,\t
260,0,\t262,0,\t261,0,\t255,0,\t
263,0,\t0,0,\t261,0,\t264,0,\t
0,0,\t262,0,\t265,0,\t259,44,\t
256,0,\t262,0,\t260,44,\t0,0,\t
0,0,\t0,0,\t257,0,\t0,0,\t
261,44,\t0,0,\t0,0,\t0,0,\t
255,294,\t0,0,\t263,0,\t262,44,\t
0,0,\t264,0,\t0,0,\t258,0,\t
265,0,\t0,0,\t263,0,\t266,0,\t
257,295,\t264,0,\t263,0,\t0,0,\t
265,0,\t264,0,\t0,0,\t0,0,\t
265,0,\t267,0,\t0,0,\t259,0,\t
0,0,\t0,0,\t260,0,\t0,0,\t
263,44,\t258,296,\t0,0,\t264,44,\t
261,0,\t266,0,\t265,44,\t0,0,\t
0,0,\t0,0,\t268,0,\t262,0,\t
0,0,\t266,0,\t0,0,\t267,0,\t
269,0,\t266,0,\t260,297,\t0,0,\t
261,298,\t0,0,\t0,0,\t267,0,\t
0,0,\t270,0,\t271,0,\t267,0,\t
0,0,\t0,0,\t0,0,\t266,44,\t
268,0,\t0,0,\t0,0,\t0,0,\t
263,0,\t0,0,\t269,0,\t264,0,\t
268,0,\t267,44,\t265,0,\t0,0,\t
268,0,\t0,0,\t269,0,\t270,0,\t
271,0,\t0,0,\t269,0,\t0,0,\t
0,0,\t0,0,\t272,0,\t270,0,\t
271,0,\t0,0,\t268,44,\t270,0,\t
271,0,\t0,0,\t0,0,\t273,0,\t
269,44,\t274,0,\t0,0,\t266,0,\t
0,0,\t265,299,\t0,0,\t0,0,\t
0,0,\t270,44,\t271,44,\t0,0,\t
272,0,\t267,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t266,300,\t
272,0,\t273,0,\t275,0,\t274,0,\t
272,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t273,0,\t268,0,\t274,0,\t
276,0,\t273,0,\t0,0,\t274,0,\t
269,0,\t0,0,\t272,44,\t277,0,\t
267,301,\t269,302,\t278,0,\t0,0,\t
275,0,\t270,0,\t271,0,\t273,44,\t
0,0,\t274,44,\t270,303,\t0,0,\t
275,0,\t0,0,\t276,0,\t0,0,\t
275,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t277,0,\t276,0,\t0,0,\t
278,0,\t0,0,\t276,0,\t279,0,\t
271,304,\t277,0,\t275,44,\t0,0,\t
278,0,\t277,0,\t272,0,\t0,0,\t
278,0,\t0,0,\t0,0,\t0,0,\t
276,44,\t0,0,\t0,0,\t273,0,\t
0,0,\t274,0,\t272,305,\t277,44,\t
0,0,\t279,0,\t278,44,\t280,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t279,0,\t281,0,\t273,306,\t
282,0,\t279,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t275,0,\t274,307,\t
0,0,\t0,0,\t0,0,\t283,0,\t
275,308,\t280,0,\t0,0,\t279,44,\t
276,0,\t0,0,\t0,0,\t0,0,\t
281,0,\t280,0,\t282,0,\t277,0,\t
0,0,\t280,0,\t278,0,\t0,0,\t
281,0,\t0,0,\t282,0,\t284,0,\t
281,0,\t283,0,\t282,0,\t0,0,\t
276,309,\t278,310,\t285,0,\t280,44,\t
286,0,\t283,0,\t0,0,\t0,0,\t
0,0,\t283,0,\t281,44,\t0,0,\t
282,44,\t0,0,\t0,0,\t279,0,\t
0,0,\t284,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t283,44,\t
285,0,\t284,0,\t286,0,\t287,0,\t
0,0,\t284,0,\t0,0,\t0,0,\t
285,0,\t0,0,\t286,0,\t0,0,\t
285,0,\t0,0,\t286,0,\t280,0,\t
0,0,\t0,0,\t288,0,\t284,44,\t
289,0,\t0,0,\t281,0,\t290,0,\t
282,0,\t287,0,\t285,44,\t0,0,\t
286,44,\t0,0,\t0,0,\t0,0,\t
0,0,\t287,0,\t0,0,\t283,0,\t
0,0,\t287,0,\t0,0,\t0,0,\t
288,0,\t282,312,\t289,0,\t281,311,\t
0,0,\t290,0,\t283,313,\t291,0,\t
288,0,\t0,0,\t289,0,\t287,44,\t
288,0,\t290,0,\t289,0,\t284,0,\t
0,0,\t290,0,\t292,0,\t0,0,\t
293,0,\t0,0,\t285,0,\t0,0,\t
286,0,\t0,0,\t288,44,\t294,0,\t
289,44,\t291,0,\t0,0,\t290,44,\t
285,314,\t0,0,\t0,0,\t0,0,\t
0,0,\t291,0,\t0,0,\t0,0,\t
292,0,\t291,0,\t293,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t287,0,\t
292,0,\t294,0,\t293,0,\t295,0,\t
292,0,\t0,0,\t293,0,\t291,44,\t
0,0,\t294,0,\t0,0,\t0,0,\t
296,0,\t294,0,\t288,0,\t0,0,\t
289,0,\t0,0,\t292,44,\t290,0,\t
293,44,\t0,0,\t0,0,\t289,317,\t
0,0,\t295,0,\t287,315,\t294,44,\t
297,0,\t0,0,\t0,0,\t298,0,\t
0,0,\t295,0,\t296,0,\t288,316,\t
0,0,\t295,0,\t0,0,\t0,0,\t
290,318,\t0,0,\t296,0,\t291,0,\t
299,0,\t0,0,\t296,0,\t300,0,\t
0,0,\t0,0,\t297,0,\t295,44,\t
0,0,\t298,0,\t292,0,\t0,0,\t
293,0,\t0,0,\t297,0,\t0,0,\t
296,44,\t298,0,\t297,0,\t294,0,\t
0,0,\t298,0,\t299,0,\t0,0,\t
301,0,\t300,0,\t0,0,\t302,0,\t
0,0,\t0,0,\t299,0,\t0,0,\t
297,44,\t300,0,\t299,0,\t298,44,\t
0,0,\t300,0,\t0,0,\t0,0,\t
303,0,\t0,0,\t0,0,\t295,0,\t
0,0,\t0,0,\t301,0,\t0,0,\t
299,44,\t302,0,\t295,319,\t300,44,\t
296,0,\t0,0,\t301,0,\t296,320,\t
304,0,\t302,0,\t301,0,\t305,0,\t
0,0,\t302,0,\t303,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
297,0,\t306,0,\t303,0,\t298,0,\t
301,44,\t0,0,\t303,0,\t302,44,\t
0,0,\t0,0,\t304,0,\t307,0,\t
298,322,\t305,0,\t0,0,\t0,0,\t
299,0,\t297,321,\t304,0,\t300,0,\t
303,44,\t305,0,\t304,0,\t306,0,\t
0,0,\t305,0,\t0,0,\t0,0,\t
308,0,\t0,0,\t309,0,\t306,0,\t
0,0,\t307,0,\t0,0,\t306,0,\t
304,44,\t0,0,\t0,0,\t305,44,\t
301,0,\t307,0,\t0,0,\t302,0,\t
0,0,\t307,0,\t0,0,\t0,0,\t
0,0,\t306,44,\t308,0,\t0,0,\t
309,0,\t0,0,\t0,0,\t0,0,\t
303,0,\t310,0,\t308,0,\t307,44,\t
309,0,\t0,0,\t308,0,\t303,323,\t
309,0,\t0,0,\t0,0,\t311,0,\t
312,0,\t0,0,\t0,0,\t0,0,\t
304,0,\t0,0,\t0,0,\t305,0,\t
308,44,\t313,0,\t309,44,\t310,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t306,0,\t0,0,\t310,0,\t
0,0,\t311,0,\t312,0,\t310,0,\t
304,324,\t0,0,\t0,0,\t307,0,\t
314,0,\t311,0,\t312,0,\t313,0,\t
0,0,\t311,0,\t312,0,\t0,0,\t
0,0,\t310,44,\t315,0,\t313,0,\t
0,0,\t316,0,\t0,0,\t313,0,\t
308,0,\t0,0,\t309,0,\t311,44,\t
312,44,\t0,0,\t314,0,\t0,0,\t
0,0,\t0,0,\t307,325,\t0,0,\t
0,0,\t313,44,\t314,0,\t0,0,\t
315,0,\t0,0,\t314,0,\t316,0,\t
0,0,\t317,0,\t0,0,\t0,0,\t
315,0,\t0,0,\t0,0,\t316,0,\t
315,0,\t310,0,\t0,0,\t316,0,\t
314,44,\t0,0,\t0,0,\t0,0,\t
318,0,\t310,326,\t0,0,\t311,0,\t
312,0,\t319,0,\t315,44,\t317,0,\t
0,0,\t316,44,\t0,0,\t0,0,\t
0,0,\t313,0,\t0,0,\t317,0,\t
320,0,\t0,0,\t0,0,\t317,0,\t
312,327,\t0,0,\t318,0,\t0,0,\t
0,0,\t321,0,\t0,0,\t319,0,\t
0,0,\t313,328,\t318,0,\t0,0,\t
314,0,\t317,44,\t318,0,\t319,0,\t
0,0,\t0,0,\t320,0,\t319,0,\t
0,0,\t0,0,\t315,0,\t0,0,\t
0,0,\t316,0,\t320,0,\t321,0,\t
318,44,\t322,0,\t320,0,\t0,0,\t
0,0,\t319,44,\t314,329,\t321,0,\t
0,0,\t0,0,\t0,0,\t321,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
320,44,\t323,0,\t0,0,\t0,0,\t
324,0,\t317,0,\t0,0,\t322,0,\t
0,0,\t321,44,\t317,330,\t0,0,\t
0,0,\t0,0,\t0,0,\t322,0,\t
0,0,\t0,0,\t0,0,\t322,0,\t
318,0,\t0,0,\t0,0,\t323,0,\t
325,0,\t319,0,\t324,0,\t0,0,\t
0,0,\t0,0,\t319,332,\t323,0,\t
0,0,\t322,44,\t324,0,\t323,0,\t
320,0,\t0,0,\t324,0,\t0,0,\t
0,0,\t326,0,\t318,331,\t327,0,\t
328,0,\t321,0,\t325,0,\t0,0,\t
0,0,\t323,44,\t0,0,\t320,333,\t
324,44,\t0,0,\t325,0,\t0,0,\t
0,0,\t0,0,\t325,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t326,0,\t
329,0,\t327,0,\t328,0,\t0,0,\t
0,0,\t322,0,\t321,334,\t326,0,\t
325,44,\t327,0,\t328,0,\t326,0,\t
322,335,\t327,0,\t328,0,\t0,0,\t
0,0,\t330,0,\t0,0,\t331,0,\t
332,0,\t323,0,\t329,0,\t0,0,\t
324,0,\t326,44,\t0,0,\t327,44,\t
328,44,\t0,0,\t329,0,\t0,0,\t
0,0,\t0,0,\t329,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t330,0,\t
0,0,\t331,0,\t332,0,\t0,0,\t
325,0,\t0,0,\t0,0,\t330,0,\t
329,44,\t331,0,\t332,0,\t330,0,\t
333,0,\t331,0,\t332,0,\t0,0,\t
0,0,\t334,0,\t0,0,\t335,0,\t
0,0,\t326,0,\t0,0,\t327,0,\t
328,0,\t330,44,\t0,0,\t331,44,\t
332,44,\t0,0,\t0,0,\t0,0,\t
336,0,\t0,0,\t333,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t334,0,\t
0,0,\t335,0,\t333,0,\t0,0,\t
329,0,\t0,0,\t333,0,\t334,0,\t
326,336,\t335,0,\t0,0,\t334,0,\t
337,0,\t335,0,\t336,0,\t0,0,\t
0,0,\t338,0,\t0,0,\t0,0,\t
333,44,\t330,0,\t336,0,\t331,0,\t
332,0,\t334,44,\t336,0,\t335,44,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t337,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t338,0,\t
336,44,\t0,0,\t337,0,\t0,0,\t
0,0,\t0,0,\t337,0,\t338,0,\t
0,0,\t0,0,\t0,0,\t338,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
333,0,\t0,0,\t0,0,\t0,0,\t
337,44,\t334,0,\t0,0,\t335,0,\t
0,0,\t338,44,\t0,0,\t0,0,\t
0,0,\t335,338,\t0,0,\t0,0,\t
0,0,\t0,0,\t333,337,\t0,0,\t
336,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t0,0,\t0,0,\t0,0,\t
337,0,\t0,0,\t0,0,\t0,0,\t
0,0,\t338,0,\t0,0,\t0,0,\t
0,0};'
	EXEC -nh
		INPUT - $'#pragma prototyped
/* Adapted for ksh by David Korn */
/*+\tVI.C\t\t\tP.D. Sullivan
 *
 *\tOne line editor for the shell based on the vi editor.
 *
 *\t\tcbosgd!pds
-*/


#include\t<errno.h>
#include\t<stdlib.h>
#include\t<string.h>

#ifdef KSHELL
#   include\t"defs.h"
#else
#   include\t"io.h"
#endif\t/* KSHELL */

#include\t"history.h"
#include\t"edit.h"
#include\t"terminal.h"

#ifdef OLDTERMIO
#   undef ECHOCTL
    extern char echoctl;
#else
#   ifdef ECHOCTL
#\tdefine echoctl\tECHOCTL
#   else
#\tdefine echoctl\t0
#   endif /* ECHOCTL */
#endif /*OLDTERMIO */

#ifndef FIORDCHK
#   define NTICKS\t5\t\t/* number of ticks for typeahead */
#   ifndef KSHELL
#\tifdef _sys_times_
#\tinclude\t<sys/times.h>
#\telse
  \t   struct tms
\t   {
\t\ttime_t\ttms_utime;
\t\ttime_t\ttms_stime;
\t\ttime_t\ttms_cutime;
\t\ttime_t\ttms_cstime;
\t   };
#\tendif /* _sys_times */
#   endif /* KSHELL */
#endif /* FIORDCHK */

#define\tMAXCHAR\tMAXLINE-2\t\t/* max char per line */
#define\tWINDOW\tMAXWINDOW\t\t/* max char in window of which */
\t\t\t\t\t/* WINDOW-2 are available to user */
\t\t\t\t\t/* actual window size may be smaller */


#undef isblank
#ifdef MULTIBYTE
    static int bigvi;
#   define gencpy(a,b)\ted_gencpy(a,b)
#   define genncpy(a,b,n)\ted_genncpy(a,b,n)
#   define genlen(str)\ted_genlen(str)
#   define digit(c)\t((c&~STRIP)==0 && isdigit(c))
#   define is_print(c)\t((c&~STRIP) || isprint(c))
#else
#   define gencpy(a,b)\tstrcpy((char*)(a),(char*)(b))
#   define genncpy(a,b,n) strncpy((char*)(a),(char*)(b),n)
#   define genlen(str)\tstrlen(str)
#   define isalph(v)\tisalnum(virtual[v])
#   define isblank(v)\tisspace(virtual[v])
#   define ismetach(v)\tismeta(virtual[v])
#   define digit(c)\tisdigit(c)
#   define is_print(c)\tisprint(c)
#endif\t/* MULTIBYTE */
#define fold(c)\t\t((c)&~040)\t/* lower and uppercase equivalent */
#ifdef INT16
/* save space by defining functions for these */
#   undef isalph
#   undef isblank
#   undef ismetach
    static int isalph(int);
    static int isblank(int);
    static int ismetach(int);
#endif\t/* INT16 */

#undef putchar
#undef getchar
#define getchar()\ted_getchar()
#define putchar(c)\ted_putchar(c)
#define bell\t\ted_ringbell()\t/* ring terminal\'s bell */
#define crlf\t\ted_crlf()\t/* return and linefeed */

#define in_raw\t\teditb.e_addnl\t\t/* next char input is raw */
#define crallowed\teditb.e_crlf
#define cur_virt\teditb.e_cur\t\t/* current virtual column */
#define cur_phys\teditb.e_pcur\t/* current phys column cursor is at */
#define curhline\teditb.e_hline\t\t/* current history line */
#define env\t\teditb.e_env
#define fildes\t\teditb.e_fd
#define findchar\teditb.e_fchar\t\t/* last find char */
#define first_virt\teditb.e_fcol\t\t/* first allowable column */
#define first_wind\teditb.e_globals[0]\t/* first column of window */
#define\tglobals\t\teditb.e_globals\t\t/* local global variables */
#define histmin\t\teditb.e_hismin
#define histmax\t\teditb.e_hismax
#define last_phys\teditb.e_peol\t\t/* last column in physical */
#define last_virt\teditb.e_eol\t\t/* last column */
#define last_wind\teditb.e_globals[1]\t/* last column in window */
#define\tlastmotion\teditb.e_globals[2]\t/* last motion */
#define lastrepeat\teditb.e_mode\t/* last repeat count for motion cmds */
#define\tlong_char\teditb.e_globals[3]\t/* line bigger than window */
#define\tlong_line\teditb.e_globals[4]\t/* line bigger than window */
#define lsearch\t\teditb.e_search\t\t/* last search string */
#define lookahead\teditb.e_index\t\t/* characters in buffer */
#define previous\teditb.e_lbuf\t\t/* lookahead buffer */
#define max_col\t\teditb.e_llimit\t\t/* maximum column */
#define\tocur_phys\teditb.e_globals[5]   /* old current physical position */
#define\tocur_virt\teditb.e_globals[6]\t/* old last virtual position */
#define\tofirst_wind\teditb.e_globals[7]\t/* old window first col */
#define\to_v_char\teditb.e_globals[8]\t/* prev virtual[ocur_virt] */
#define Prompt\t\teditb.e_prompt\t\t/* pointer to prompt */
#define plen\t\teditb.e_plen\t\t/* length of prompt */
#define physical\teditb.e_physbuf\t\t/* physical image */
#define repeat\t\teditb.e_repeat\t    /* repeat count for motion cmds */
#define ttyspeed\teditb.e_ttyspeed\t/* tty speed */
#define u_column\teditb.e_ucol\t\t/* undo current column */
#define U_saved\t\teditb.e_saved\t\t/* original virtual saved */
#define U_space\t\teditb.e_Ubuf\t\t/* used for U command */
#define u_space\t\teditb.e_ubuf\t\t/* used for u command */
#define usreof\t\teditb.e_eof\t\t/* user defined eof char */
#define usrerase\teditb.e_erase\t\t/* user defined erase char */
#define usrintr\t\teditb.e_intr\t\t/* user defined intr char */
#define usrkill\t\teditb.e_kill\t\t/* user defined kill char */
#define usrquit\t\teditb.e_quit\t\t/* user defined quit char */
#define virtual\t\teditb.e_inbuf\t/* pointer to virtual image buffer */
#define\twindow\t\teditb.e_window\t\t/* window buffer */
#define\tw_size\t\teditb.e_wsize\t\t/* window size */
#define\tinmacro\t\teditb.e_inmacro\t\t/* true when in macro */
#define yankbuf\t\teditb.e_killbuf\t\t/* yank/delete buffer */

extern clock_t times();

#define\tABORT\t-2\t\t\t/* user abort */
#define\tAPPEND\t-10\t\t\t/* append chars */
#define\tBAD\t-1\t\t\t/* failure flag */
#define\tBIGVI\t-15\t\t\t/* user wants real vi */
#define\tCONTROL\t-20\t\t\t/* control mode */
#define\tENTER\t-25\t\t\t/* enter flag */
#define\tGOOD\t0\t\t\t/* success flag */
#define\tINPUT\t-30\t\t\t/* input mode */
#define\tINSERT\t-35\t\t\t/* insert mode */
#define\tREPLACE\t-40\t\t\t/* replace chars */
#define\tSEARCH\t-45\t\t\t/* search flag */
#define\tTRANSLATE\t-50\t\t/* translate virt to phys only */

#define\tDEL\t\'\\177\'\t\t\t/* interrupt char */

#define\tTRUE\t1
#define\tFALSE\t0

#define\tINVALID\t(-1)\t\t\t/* invalid column */
#define\tQUIT_C\t\'\\34\'\t\t\t/* quit char */
#define\tSYSERR\t(-1)\t\t\t/* system error */

static char addnl;\t\t\t/* boolean - add newline flag */
static char last_cmd = \'\\0\';\t\t/* last command */
static char repeat_set;
static char nonewline;
static genchar *lastline;
static char paren_chars[] = "([{)]}";   /* for % command */

#ifdef FIORDCHK
    static clock_t typeahead;\t\t/* typeahead occurred */
#else
    static int typeahead;\t\t/* typeahead occurred */
#endif\t/* FIORDCHK */

static void\tdel_line(int);
static int\tgetcount(int);
static void\tgetline(int);
static int\tgetrchar(void);
static int\tmvcursor(int);
static void\tpr_prompt(void);
static void\tpr_string(const char*);
static void\tputstring(int, int);
static void\trefresh(int);
static void\treplace(int, int);
static void\trestore_v(void);
static void\tsave_last(void);
static void\tsave_v(void);
static int\tsearch(int);
static void\tsync_cursor(void);
static int\ttextmod(int,int);

/*+\tVI_READ( fd, shbuf, nchar )
 *
 *\tThis routine implements a one line version of vi and is
 * called by _filbuf.c
 *
-*/

vi_read(int fd, register char *shbuf, unsigned nchar)
{
\tregister int i;\t\t\t/* general variable */
\tregister int c;\t\t\t/* general variable */
\tregister int term_char;\t/* read() termination character */
\tchar prompt[PRSIZE+2];\t\t/* prompt */
\tgenchar Physical[2*MAXLINE];\t/* physical image */
\tgenchar Ubuf[MAXLINE];\t/* used for U command */
\tgenchar ubuf[MAXLINE];\t/* used for u command */
\tgenchar Window[WINDOW+10];\t/* window image */
\tchar cntl_char;\t\t\t/* TRUE if control character present */
\tint Globals[9];\t\t\t/* local global variables */
#ifndef FIORDCHK
\tclock_t oldtime, newtime;
\tstruct tms dummy;
#endif\t/* FIORDCHK */
\t
\t/*** setup prompt ***/

\tPrompt = prompt;
\ted_setup(fd);

#ifndef RAWONLY
\tif( !sh_isoption(SH_VIRAW) )
\t{
\t\t/*** Change the eol characters to \'\\r\' and eof  ***/
\t\t/* in addition to \'\\n\' and make eof an ESC\t*/

\t\tif( tty_alt(ERRIO) == BAD )
\t\t{
\t\t\treturn(read(fd, shbuf, nchar));
\t\t}

#   ifdef FIORDCHK
\t\tioctl(fd,FIORDCHK,&typeahead);
#   else
\t\t/* time the current line to determine typeahead */
\t\toldtime = times(&dummy);
#   endif /* FIORDCHK */
#   ifdef KSHELL
\t\t/* abort of interrupt has occurred */
\t\tif(sh.trapnote&SIGSET)
\t\t\ti = -1;
\t\telse
#   endif /* KSHELL */
\t\t/*** Read the line ***/
\t\ti = read(fd, shbuf, nchar);
#   ifndef FIORDCHK
\t\tnewtime = times(&dummy);
\t\ttypeahead = ((newtime-oldtime) < NTICKS);
#   endif /* FIORDCHK */
\t    if(echoctl)
\t    {
\t\tif( i <= 0 )
\t\t{
\t\t\t/*** read error or eof typed ***/
\t\t\ttty_cooked(ERRIO);
\t\t\treturn(i);
\t\t}
\t\tterm_char = shbuf[--i];
\t\tif( term_char == \'\\r\' )
\t\t\tterm_char = \'\\n\';
\t\tif( term_char==\'\\n\' || term_char==ESC )
\t\t\tshbuf[i--] = \'\\0\';
\t\telse
\t\t\tshbuf[i+1] = \'\\0\';
\t    }
\t    else
\t    {
\t\tc = shbuf[0];

\t\t/*** Save and remove the last character if its an eol, ***/
\t\t/* changing \'\\r\' to \'\\n\' */

\t\tif( i == 0 )
\t\t{
\t\t\t/*** ESC was typed as first char of line ***/
\t\t\tterm_char = ESC;
\t\t\tshbuf[i--] = \'\\0\';\t/* null terminate line */
\t\t}
\t\telse if( i<0 || c==usreof )
\t\t{
\t\t\t/*** read error or eof typed ***/
\t\t\ttty_cooked(ERRIO);
\t\t\tif( c == usreof )
\t\t\t\ti = 0;
\t\t\treturn(i);
\t\t}
\t\telse
\t\t{
\t\t\tterm_char = shbuf[--i];
\t\t\tif( term_char == \'\\r\' )
\t\t\t\tterm_char = \'\\n\';
\t\t\tif( term_char==\'\\n\' || term_char==usreof )
\t\t\t{
\t\t\t\t/*** remove terminator & null terminate ***/
\t\t\t\tshbuf[i--] = \'\\0\';
\t\t\t}
\t\t\telse
\t\t\t{
\t\t\t\t/** terminator was ESC, which is not xmitted **/
\t\t\t\tterm_char = ESC;
\t\t\t\tshbuf[i+1] = \'\\0\';
\t\t\t}
\t\t}
\t    }
\t}
\telse
#endif\t/* RAWONLY */
\t{
\t\t/*** Set raw mode ***/

#ifndef RAWONLY
\t\tif( ttyspeed == 0 )
\t\t{
\t\t\t/*** never did TCGETA, so do it ***/
\t\t\t/* avoids problem if user does \'sh -o viraw\' */
\t\t\ttty_alt(ERRIO);
\t\t}
#endif /* RAWONLY */
\t\tif( tty_raw(ERRIO) == BAD )
\t\t{
\t\t\treturn(read(fd, shbuf, nchar));
\t\t}
\t\ti = INVALID;
\t}

\t/*** Initialize some things ***/

\tvirtual = (genchar*)shbuf;
#undef virtual
#define virtual\t\t((genchar*)shbuf)
#ifdef MULTIBYTE
\tshbuf[i+1] = 0;
\ti = ed_internal(shbuf,virtual)-1;
#endif /* MULTIBYTE */
\tglobals = Globals;
\tcur_phys = i + 1;
\tcur_virt = i;
\tfildes = fd;
\tfirst_virt = 0;
\tfirst_wind = 0;
\tlast_virt = i;
\tlast_phys = i;
\tlast_wind = i;
\tlong_line = \' \';
\tlong_char = \' \';
\to_v_char = \'\\0\';
\tocur_phys = 0;
\tin_raw = 0;
\tocur_virt = MAXCHAR;
\tofirst_wind = 0;
\tphysical = Physical;
\tu_column = INVALID - 1;
\tU_space = Ubuf;
\tu_space = ubuf;
\twindow = Window;
\twindow[0] = \'\\0\';

#if KSHELL && (2*CHARSIZE*MAXLINE)<IOBSIZE
\tyankbuf = shbuf + MAXLINE*sizeof(genchar);
#else
\tif(yankbuf==0)
\t\tyankbuf = (genchar*)malloc(sizeof(genchar)*(MAXLINE));
#endif
#if KSHELL && (3*CHARSIZE*MAXLINE)<IOBSIZE
\tlastline = shbuf + (MAXLINE+MAXLINE)*sizeof(genchar);
#else
\tif(lastline==0)
\t\tlastline = (genchar*)malloc(sizeof(genchar)*(MAXLINE));
#endif
\tif( last_cmd == \'\\0\' )
\t{
\t\t/*** first time for this shell ***/

\t\tlast_cmd = \'i\';
\t\tfindchar = INVALID;
\t\tlastmotion = \'\\0\';
\t\tlastrepeat = 1;
\t\trepeat = 1;
\t\t*yankbuf = 0;
\t}

\t/*** fiddle around with prompt length ***/
\tif( nchar+plen > MAXCHAR )
\t\tnchar = MAXCHAR - plen;
\tmax_col = nchar - 2;

#ifndef RAWONLY
\tif( !sh_isoption(SH_VIRAW) )
\t{
\t\tint kill_erase = 0;
#   ifndef ECHOCTL
\t\tcntl_char = FALSE;
#   endif /* !ECHOCTL */
\t\tfor(i=(echoctl?last_virt:0); i<=last_virt; ++i )
\t\t{
\t\t\t/*** change \\r to 
, check for control characters, ***/
\t\t\t/* delete appropriate ^Vs,\t\t\t*/
\t\t\t/* and estimate last physical column */

\t\t\tif( virtual[i] == \'\\r\' )
\t\t\t\tvirtual[i] = \'\\n\';
\t\t    if(!echoctl)
\t\t    {
\t\t\tc = virtual[i];
\t\t\tif( c==usrerase || c==usrkill )
\t\t\t{
\t\t\t\t/*** user typed escaped erase or kill char ***/
\t\t\t\tcntl_char = TRUE;
\t\t\t\tif(is_print(c))
\t\t\t\t\tkill_erase++;
\t\t\t}
\t\t\telse if( !is_print(c) )
\t\t\t{
\t\t\t\tcntl_char = TRUE;

\t\t\t\tif( c == cntl(\'V\') )
\t\t\t\t{
\t\t\t\t\tif( i == last_virt )
\t\t\t\t\t{
\t\t\t\t\t\t/*** eol/eof was escaped ***/
\t\t\t\t\t\t/* so replace ^V with it */
\t\t\t\t\t\tvirtual[i] = term_char;
\t\t\t\t\t\tbreak;
\t\t\t\t\t}

\t\t\t\t\t/*** delete ^V ***/
\t\t\t\t\tgencpy((&virtual[i]), (&virtual[i+1]));
\t\t\t\t\t--cur_virt;
\t\t\t\t\t--last_virt;
\t\t\t\t}
\t\t\t}
\t\t    }
\t\t}

\t\t/*** copy virtual image to window ***/
\t\tif(last_virt > 0)
\t\t\tlast_phys = ed_virt_to_phys(virtual,physical,last_virt,0,0);
\t\tif( last_phys >= w_size )
\t\t{
\t\t\t/*** line longer than window ***/
\t\t\tlast_wind = w_size - 1;
\t\t}
\t\telse
\t\t\tlast_wind = last_phys;
\t\tgenncpy(window, virtual, last_wind+1);

\t\tif( term_char!=ESC  && (last_virt==INVALID
\t\t\t|| virtual[last_virt]!=term_char) )
\t\t{
\t\t\t/*** Line not terminated with ESC or escaped (^V) ***/
\t\t\t/* eol, so return after doing a total update */
\t\t\t/* if( (speed is greater or equal to 1200 */
\t\t\t/* and something was typed) and */
\t\t\t/* (control character present */
\t\t\t/* or typeahead occurred) ) */

\t\t\ttty_cooked(ERRIO);
\t\t\tif( ttyspeed==FAST && last_virt!=INVALID
# ifdef ECHOCTL
\t\t\t\t&& typeahead)
# else
\t\t\t\t&& (typeahead || cntl_char==TRUE) )
# endif /*ECHOCTL */
\t\t\t{
\t\t\t\trefresh(TRANSLATE);
\t\t\t\tpr_prompt();
\t\t\t\tputstring(0, last_phys+1);
\t\t\t\tif(echoctl)
\t\t\t\t\tcrlf;
\t\t\t\telse
\t\t\t\t\twhile(kill_erase-- > 0)
\t\t\t\t\t\tputchar(\' \');
\t\t\t}

\t\t\tif( term_char==\'\\n\' )
\t\t\t{
\t\t\t\tif(!echoctl)
\t\t\t\t\tcrlf;
\t\t\t\tvirtual[++last_virt] = \'\\n\';
\t\t\t}
\t\t\tlast_cmd = \'i\';
\t\t\tsave_last();
#ifdef MULTIBYTE
\t\t\tvirtual[last_virt+1] = 0;
\t\t\tlast_virt = ed_external(virtual,shbuf);
\t\t\treturn(last_virt);
#else
\t\t\treturn(++last_virt);
#endif /* MULTIBYTE */
\t\t}

\t\t/*** Line terminated with escape, or escaped eol/eof, ***/
\t\t/*  so set raw mode */

\t\tif( tty_raw(ERRIO) == BAD )
\t\t{
\t\t\ttty_cooked(ERRIO);
\t\t\tvirtual[++last_virt] = \'\\n\';
#ifdef MULTIBYTE
\t\t\tvirtual[last_virt+1] = 0;
\t\t\tlast_virt = ed_external(virtual,shbuf);
\t\t\treturn(last_virt);
#else
\t\t\treturn(++last_virt);
#endif /* MULTIBYTE */
\t\t}

\t\tif(echoctl) /*** for cntl-echo erase the ^[ ***/
\t\t\tpr_string("\\b\\b  \\b\\b");


\t\tif( crallowed == YES )
\t\t{
\t\t\t/*** start over since there may be ***/
\t\t\t/*** a control char, or cursor might not ***/
\t\t\t/*** be at left margin (this lets us know ***/
\t\t\t/*** where we are ***/
\t\t\tcur_phys = 0;
\t\t\twindow[0] = \'\\0\';
\t\t\tpr_prompt();
\t\t\tif( term_char==ESC && virtual[last_virt]!=ESC )
\t\t\t\trefresh(CONTROL);
\t\t\telse
\t\t\t\trefresh(INPUT);
\t\t}
\t\telse
\t\t{
\t\t\t/*** just update everything internally ***/
\t\t\trefresh(TRANSLATE);
\t\t}
\t}
\telse
#endif\t/* RAWONLY */
\t\tvirtual[0] = \'\\0\';

\t/*** Handle usrintr, usrquit, or EOF ***/

\ti = SETJMP(env);
\tif( i != 0 )
\t{
\t\tvirtual[0] = \'\\0\';
\t\ttty_cooked(ERRIO);

\t\tswitch(i)
\t\t{
\t\tcase UEOF:
\t\t\t/*** EOF ***/
\t\t\treturn(0);

\t\tcase UINTR:
\t\t\t/** interrupt **/
\t\t\treturn(SYSERR);
\t\t}
\t\treturn(SYSERR);
\t}

\t/*** Get a line from the terminal ***/

\tU_saved = FALSE;

#ifdef RAWONLY
\tgetline(APPEND);
#else
\tif( sh_isoption(SH_VIRAW) || virtual[last_virt]==term_char )
\t\tgetline(APPEND);
\telse
\t\tgetline(ESC);
#endif\t/* RAWONLY */

\t/*** add a new line if user typed unescaped 
 ***/
\t/* to cause the shell to process the line */
\ttty_cooked(ERRIO);
\tif( addnl )
\t{
\t\tvirtual[++last_virt] = \'\\n\';
\t\tcrlf;
\t}
\tif( ++last_virt >= 0 )
\t{
#ifdef MULTIBYTE
\t\tif(bigvi)
\t\t{
\t\t\tbigvi = 0;
\t\t\tshbuf[last_virt-1] = \'\\n\';
\t\t}
\t\telse
\t\t{
\t\t\tvirtual[last_virt] = 0;
\t\t\tlast_virt = ed_external(virtual,shbuf);
\t\t}
#endif /* MULTIBYTE */
\t\treturn(last_virt);
\t}
\telse
\t\treturn(SYSERR);
}


/*{\tAPPEND( char, mode )
 *
 *\tThis routine will append char after cur_virt in the virtual image.
 * mode\t=\tAPPEND, shift chars right before appending
 *\t\tREPLACE, replace char if possible
 *
}*/

#undef virtual
#define virtual\t\teditb.e_inbuf\t/* pointer to virtual image buffer */

static void append(int c, int mode)
{
\tregister int i;

\tif( last_virt<max_col && last_phys<max_col )
\t{
\t\tif( mode==APPEND || cur_virt==last_virt )
\t\t{
\t\t\tfor(i = ++last_virt;  i > cur_virt; --i)
\t\t\t{
\t\t\t\tvirtual[i] = virtual[i-1];
\t\t\t}
\t\t}
\t\tvirtual[++cur_virt] = c;
\t}
\telse
\t\tbell;
\treturn;
}

/*{\tBACKWORD( nwords, cmd )
 *
 *\tThis routine will position cur_virt at the nth previous word.
 *
}*/

static void backword(int nwords, register int cmd)
{
\tregister int tcur_virt = cur_virt;
\twhile( nwords-- && tcur_virt > first_virt )
\t{
\t\tif( !isblank(tcur_virt) && isblank(tcur_virt-1)
\t\t\t&& tcur_virt>first_virt )
\t\t\t--tcur_virt;
\t\telse if(cmd != \'B\')
\t\t{
\t\t\tregister int last = isalph(tcur_virt-1);
\t\t\tif((!isalph(tcur_virt) && last)
\t\t\t|| (isalph(tcur_virt) && !last))
\t\t\t\t--tcur_virt;
\t\t}
\t\twhile( isblank(tcur_virt) && tcur_virt>=first_virt )
\t\t\t--tcur_virt;
\t\tif( cmd == \'B\' )
\t\t{
\t\t\twhile( !isblank(tcur_virt) && tcur_virt>=first_virt )
\t\t\t\t--tcur_virt;
\t\t}
\t\telse
\t\t{
\t\t\tif( isalph(tcur_virt) )
\t\t\t\twhile( isalph(tcur_virt) && tcur_virt>=first_virt )
\t\t\t\t\t--tcur_virt;
\t\t\telse
\t\t\t\twhile( !isalph(tcur_virt) && !isblank(tcur_virt)
\t\t\t\t\t&& tcur_virt>=first_virt )
\t\t\t\t\t--tcur_virt;
\t\t}
\t\tcur_virt = ++tcur_virt;
\t}
\treturn;
}

/*{\tCNTLMODE()
 *
 *\tThis routine implements the vi command subset.
 *\tThe cursor will always be positioned at the char of interest.
 *
}*/

static int cntlmode(void)
{
\tregister int c;
\tregister int i;
\tgenchar tmp_u_space[MAXLINE];\t/* temporary u_space */
\tgenchar *real_u_space;\t\t/* points to real u_space */
\tint tmp_u_column = INVALID;\t/* temporary u_column */
\tint was_inmacro;

\tif( U_saved == FALSE )
\t{
\t\t/*** save virtual image if never done before ***/
\t\tvirtual[last_virt+1] = \'\\0\';
\t\tgencpy(U_space, virtual);
\t\tU_saved = TRUE;
\t}

\tsave_last();

\treal_u_space = u_space;
\tcurhline = histmax;
\tfirst_virt = 0;
\trepeat = 1;
\tif( cur_virt > INVALID )
\t{
\t\t/*** make sure cursor is at the last char ***/
\t\tsync_cursor();
\t}

\t/*** Read control char until something happens to cause a ***/
\t/* return to APPEND/REPLACE mode\t*/

\twhile( c=getchar() )
\t{
\t\trepeat_set = 0;
\t\twas_inmacro = inmacro;
\t\tif( c == \'0\' )
\t\t{
\t\t\t/*** move to leftmost column ***/
\t\t\tcur_virt = 0;
\t\t\tsync_cursor();
\t\t\tcontinue;
\t\t}

\t\tif( digit(c) )
\t\t{
\t\t\tlastrepeat = repeat;
\t\t\tc = getcount(c);
\t\t\tif( c == \'.\' )
\t\t\t\tlastrepeat = repeat;
\t\t}

\t\t/*** see if it\'s a move cursor command ***/

\t\tif( mvcursor(c) == GOOD )
\t\t{
\t\t\tsync_cursor();
\t\t\trepeat = 1;
\t\t\tcontinue;
\t\t}

\t\t/*** see if it\'s a repeat of the last command ***/

\t\tif( c == \'.\' )
\t\t{
\t\t\tc = last_cmd;
\t\t\trepeat = lastrepeat;
\t\t\ti = textmod(c, c);
\t\t}
\t\telse
\t\t{
\t\t\ti = textmod(c, 0);
\t\t}

\t\t/*** see if it\'s a text modification command ***/

\t\tswitch(i)
\t\t{
\t\tcase BAD:
\t\t\tbreak;

\t\tdefault:\t\t/** input mode **/
\t\t\tif(!was_inmacro)
\t\t\t{
\t\t\t\tlast_cmd = c;
\t\t\t\tlastrepeat = repeat;
\t\t\t}
\t\t\trepeat = 1;
\t\t\tif( i == GOOD )
\t\t\t\tcontinue;
\t\t\treturn(i);
\t\t}

\t\tswitch( c )
\t\t{
\t\t\t/***** Other stuff *****/

\t\tcase cntl(\'L\'):\t\t/** Redraw line **/
\t\t\t/*** print the prompt and ***/
\t\t\t/* force a total refresh */
\t\t\tif(nonewline==0)
\t\t\t\tputchar(\'\\n\');
\t\t\tnonewline = 0;
\t\t\tpr_prompt();
\t\t\twindow[0] = \'\\0\';
\t\t\tcur_phys = first_wind;
\t\t\tofirst_wind = INVALID;
\t\t\tlong_line = \' \';
\t\t\tbreak;

\t\tcase cntl(\'V\'):
\t\t{
\t\t\tregister const char *p = &e_version[5];
\t\t\tsave_v();
\t\t\tdel_line(BAD);
\t\t\twhile(c = *p++)
\t\t\t\tappend(c,APPEND);
\t\t\trefresh(CONTROL);
\t\t\ted_getchar();
\t\t\trestore_v();
\t\t\tbreak;
\t\t}

\t\tcase \'/\':\t\t/** Search **/
\t\tcase \'?\':
\t\tcase \'N\':
\t\tcase \'n\':
\t\t\tsave_v();
\t\t\tswitch( search(c) )
\t\t\t{
\t\t\tcase GOOD:
\t\t\t\t/*** force a total refresh ***/
\t\t\t\twindow[0] = \'\\0\';
\t\t\t\tgoto newhist;

\t\t\tcase BAD:
\t\t\t\t/*** no match ***/
\t\t\t\t\tbell;

\t\t\tdefault:
\t\t\t\tif( u_column == INVALID )
\t\t\t\t\tdel_line(BAD);
\t\t\t\telse
\t\t\t\t\trestore_v();
\t\t\t\tbreak;
\t\t\t}
\t\t\tbreak;

\t\tcase \'j\':\t\t/** get next command **/
\t\tcase \'+\':\t\t/** get next command **/
\t\t\tcurhline += repeat;
\t\t\tif( curhline > histmax )
\t\t\t{
\t\t\t\tcurhline = histmax;
\t\t\t\tgoto ringbell;
\t\t\t}
\t\t\telse if(curhline==histmax && tmp_u_column!=INVALID )
\t\t\t{
\t\t\t\tu_space = tmp_u_space;
\t\t\t\tu_column = tmp_u_column;
\t\t\t\trestore_v();
\t\t\t\tu_space = real_u_space;
\t\t\t\tbreak;
\t\t\t}
\t\t\tsave_v();
\t\t\tgoto newhist;

\t\tcase \'k\':\t\t/** get previous command **/
\t\tcase \'-\':\t\t/** get previous command **/
\t\t\tif( curhline == histmax )
\t\t\t{
\t\t\t\tu_space = tmp_u_space;
\t\t\t\ti = u_column;
\t\t\t\tsave_v();
\t\t\t\tu_space = real_u_space;
\t\t\t\ttmp_u_column = u_column;
\t\t\t\tu_column = i;
\t\t\t}

\t\t\tcurhline -= repeat;
\t\t\tif( curhline <= histmin )
\t\t\t{
\t\t\t\tcurhline = histmin + 1;
\t\t\t\tgoto ringbell;
\t\t\t}
\t\t\tsave_v();
\tnewhist:
\t\t\thist_copy((char*)virtual, MAXLINE, curhline, -1);
#ifdef MULTIBYTE
\t\t\ted_internal((char*)virtual,virtual);
#endif /* MULTIBYTE */
\t\t\tif( (last_virt = genlen((char*)virtual) - 1) >= 0 )
\t\t\t\tcur_virt = 0;
\t\t\telse
\t\t\t\tcur_virt = INVALID;
\t\t\tbreak;


\t\tcase \'u\':\t\t/** undo the last thing done **/
\t\t\trestore_v();
\t\t\tbreak;

\t\tcase \'U\':\t\t/** Undo everything **/
\t\t\tsave_v();
\t\t\tif( virtual[0] == \'\\0\' )
\t\t\t\tgoto ringbell;
\t\t\telse
\t\t\t{
\t\t\t\tgencpy(virtual, U_space);
\t\t\t\tlast_virt = genlen(U_space) - 1;
\t\t\t\tcur_virt = 0;
\t\t\t}
\t\t\tbreak;

#ifdef KSHELL
\t\tcase \'v\':
\t\t\tif(repeat_set==0)
\t\t\t\tgoto vcommand;
#endif /* KSHELL */

\t\tcase \'G\':\t\t/** goto command repeat **/
\t\t\tif(repeat_set==0)
\t\t\t\trepeat = histmin+1;
\t\t\tif( repeat <= histmin || repeat > histmax )
\t\t\t{
\t\t\t\tgoto ringbell;
\t\t\t}
\t\t\tcurhline = repeat;
\t\t\tsave_v();
\t\t\tif(c == \'G\')
\t\t\t\tgoto newhist;

#ifdef KSHELL
\t\tvcommand:
\t\t\tif(ed_fulledit()==GOOD)
\t\t\t\treturn(BIGVI);
\t\t\telse
\t\t\t\tgoto ringbell;
#endif\t/* KSHELL */

\t\tcase \'#\':\t/** insert(delete) # to (no)comment command **/
\t\t\tif( cur_virt != INVALID )
\t\t\t{
\t\t\t\tregister genchar *p = &virtual[last_virt+1];
\t\t\t\t*p = 0;
\t\t\t\t/*** see whether first char is comment char ***/
\t\t\t\tc = (virtual[0]==\'#\');
\t\t\t\twhile(p-- >= virtual)
\t\t\t\t{
\t\t\t\t\tif(*p==\'\\n\' || *p==\';\' || *p==\'|\' || p<virtual)
\t\t\t\t\t{
\t\t\t\t\t\tif(c) /* delete \'#\' */
\t\t\t\t\t\t{
\t\t\t\t\t\t\tif(p[1]==\'#\')
\t\t\t\t\t\t\t{
\t\t\t\t\t\t\t\tlast_virt--;
\t\t\t\t\t\t\t\tgencpy(p+1,p+2);
\t\t\t\t\t\t\t}
\t\t\t\t\t\t}
\t\t\t\t\t\telse
\t\t\t\t\t\t{
\t\t\t\t\t\t\tcur_virt = p-virtual;
\t\t\t\t\t\t\tappend(\'#\', APPEND);
\t\t\t\t\t\t}
\t\t\t\t\t}
\t\t\t\t}
\t\t\t\tif(c)
\t\t\t\t{
\t\t\t\t\tcur_virt = 0;
\t\t\t\t\tbreak;
\t\t\t\t}
\t\t\t\trefresh(INPUT);
\t\t\t}

\t\tcase \'\\n\':\t\t/** send to shell **/
\t\t\treturn(ENTER);

\t\tdefault:
\t\tringbell:
\t\t\tbell;
\t\t\trepeat = 1;
\t\t\tcontinue;
\t\t}

\t\trefresh(CONTROL);
\t\trepeat = 1;
\t}
/* NOTREACHED */
}

/*{\tCURSOR( new_current_physical )
 *
 *\tThis routine will position the virtual cursor at
 * physical column x in the window.
 *
}*/

static void cursor(register int x)
{
\tregister int delta;

#ifdef MULTIBYTE
\twhile(physical[x]==MARKER)
\t\tx++;
#endif /* MULTIBYTE */
\tdelta = x - cur_phys;

\tif( delta == 0 )
\t\treturn;

\tif( delta > 0 )
\t{
\t\t/*** move to right ***/
\t\tputstring(cur_phys, delta);
\t}
\telse
\t{
\t\t/*** move to left ***/

\t\tdelta = -delta;

\t\t/*** attempt to optimize cursor movement ***/
\t\tif( crallowed==NO
\t\t\t|| (delta <= ((cur_phys-first_wind)+plen)>>1) )
\t\t{
\t\t\twhile( delta-- )
\t\t\t\tputchar(\'\\b\');
\t\t}
\t\telse
\t\t{
\t\t\tpr_prompt();
\t\t\tputstring(first_wind, x - first_wind);
\t\t}
\t}
\tcur_phys = x;
\treturn;
}

/*{\tDELETE( nchars, mode )
 *
 *\tDelete nchars from the virtual space and leave cur_virt positioned
 * at cur_virt-1.
 *
 *\tIf mode\t= \'c\', do not save the characters deleted
 *\t\t= \'d\', save them in yankbuf and delete.
 *\t\t= \'y\', save them in yankbuf but do not delete.
 *
}*/

static void cdelete(register int nchars, int mode)
{
\tregister int i;
\tregister genchar *vp;

\tif( cur_virt < first_virt )
\t{
\t\tbell;
\t\treturn;
\t}
\tif( nchars > 0 )
\t{
\t\tvp = virtual+cur_virt;
\t\to_v_char = vp[0];
\t\tif( (cur_virt-- + nchars) > last_virt )
\t\t{
\t\t\t/*** set nchars to number actually deleted ***/
\t\t\tnchars = last_virt - cur_virt;
\t\t}

\t\t/*** save characters to be deleted ***/

\t\tif( mode != \'c\' )
\t\t{
\t\t\ti = vp[nchars];
\t\t\tvp[nchars] = 0;
\t\t\tgencpy(yankbuf,vp);
\t\t\tvp[nchars] = i;
\t\t}

\t\t/*** now delete these characters ***/

\t\tif( mode != \'y\' )
\t\t{
\t\t\tgencpy(vp,vp+nchars);
\t\t\tlast_virt -= nchars;
\t\t}
\t}
\treturn;
}

/*{\tDEL_LINE( mode )
 *
 *\tThis routine will delete the line.
 *\tmode = GOOD, do a save_v()
 *
}*/
static void del_line(int mode)
{
\tif( last_virt == INVALID )
\t\treturn;

\tif( mode == GOOD )
\t\tsave_v();

\tcur_virt = 0;
\tfirst_virt = 0;
\tcdelete(last_virt+1, BAD);
\trefresh(CONTROL);

\tcur_virt = INVALID;
\tcur_phys = 0;
\tfindchar = INVALID;
\tlast_phys = INVALID;
\tlast_virt = INVALID;
\tlast_wind = INVALID;
\tfirst_wind = 0;
\to_v_char = \'\\0\';
\tocur_phys = 0;
\tocur_virt = MAXCHAR;
\tofirst_wind = 0;
\twindow[0] = \'\\0\';
\treturn;
}

/*{\tDELMOTION( motion, mode )
 *
 *\tDelete thru motion.
 *
 *\tmode\t= \'d\', save deleted characters, delete
 *\t\t= \'c\', do not save characters, change
 *\t\t= \'y\', save characters, yank
 *
 *\tReturns GOOD if operation successful; else BAD.
 *
}*/

static int delmotion(int motion, int mode)
{
\tregister int begin;
\tregister int end;
\t/* the following saves a register */
#       define delta end

\tif( cur_virt == INVALID )
\t\treturn(BAD);
\tif( mode != \'y\' )
\t\tsave_v();
\tbegin = cur_virt;

\t/*** fake out the motion routines by appending a blank ***/

\tvirtual[++last_virt] = \' \';
\tend = mvcursor(motion);
\tvirtual[last_virt--] = 0;
\tif(end==BAD)
\t\treturn(BAD);

\tend = cur_virt;
\tif( mode==\'c\' && end>begin && strchr("wW", motion) )
\t{
\t\t/*** called by change operation, user really expects ***/
\t\t/* the effect of the eE commands, so back up to end of word */
\t\twhile( end>begin && isblank(end-1) )
\t\t\t--end;
\t\tif( end == begin )
\t\t\t++end;
\t}

\tdelta = end - begin;
\tif( delta >= 0 )
\t{
\t\tcur_virt = begin;
\t\tif( strchr("eE;,TtFf%", motion) )
\t\t\t++delta;
\t}
\telse
\t{
\t\tdelta = -delta;
\t}

\tcdelete(delta, mode);
\tif( mode == \'y\' )
\t\tcur_virt = begin;
#       undef delta
\treturn(GOOD);
}


/*{\tENDWORD( nwords, cmd )
 *
 *\tThis routine will move cur_virt to the end of the nth word.
 *
}*/

static void endword(int nwords, register int cmd)
{
\tregister int tcur_virt = cur_virt;
\twhile( nwords-- )
\t{
\t\tif( !isblank(tcur_virt) && tcur_virt<=last_virt )
\t\t\t++tcur_virt;
\t\twhile( isblank(tcur_virt) && tcur_virt<=last_virt )
\t\t\t++tcur_virt;\t
\t\tif( cmd == \'E\' )
\t\t{
\t\t\twhile( !isblank(tcur_virt) && tcur_virt<=last_virt )
\t\t\t\t++tcur_virt;
\t\t}
\t\telse
\t\t{
\t\t\tif( isalph(tcur_virt) )
\t\t\t\twhile( isalph(tcur_virt) && tcur_virt<=last_virt )
\t\t\t\t\t++tcur_virt;
\t\t\telse
\t\t\t\twhile( !isalph(tcur_virt) && !isblank(tcur_virt)
\t\t\t\t\t&& tcur_virt<=last_virt )
\t\t\t\t\t++tcur_virt;
\t\t}
\t\tif( tcur_virt > first_virt )
\t\t\ttcur_virt--;
\t}
\tcur_virt = tcur_virt;
\treturn;
}

/*{\tFORWARD( nwords, cmd )
 *
 *\tThis routine will move cur_virt forward to the next nth word.
 *
}*/

static void forward(register int nwords, int cmd)
{
\tregister int tcur_virt = cur_virt;
\twhile( nwords-- )
\t{
\t\tif( cmd == \'W\' )
\t\t{
\t\t\twhile( !isblank(tcur_virt) && tcur_virt < last_virt )
\t\t\t\t++tcur_virt;
\t\t}
\t\telse
\t\t{
\t\t\tif( isalph(tcur_virt) )
\t\t\t{
\t\t\t\twhile( isalph(tcur_virt) && tcur_virt<last_virt )
\t\t\t\t\t++tcur_virt;
\t\t\t}
\t\t\telse
\t\t\t{
\t\t\t\twhile( !isalph(tcur_virt) && !isblank(tcur_virt)
\t\t\t\t\t&& tcur_virt < last_virt )
\t\t\t\t\t++tcur_virt;
\t\t\t}
\t\t}
\t\twhile( isblank(tcur_virt) && tcur_virt < last_virt )
\t\t\t++tcur_virt;
\t}
\tcur_virt = tcur_virt;
\treturn;
}



/*{\tGETCOUNT(c)
 *
 *\tSet repeat to the user typed number and return the terminating
 * character.
 *
}*/

static int getcount(register int c)
{
\tregister int i;

\t/*** get any repeat count ***/

\tif( c == \'0\' )
\t\treturn(c);

\trepeat_set++;
\ti = 0;
\twhile( digit(c) )
\t{
\t\ti = i*10 + c - \'0\';
\t\tc = getchar();
\t}

\tif( i > 0 )
\t\trepeat *= i;
\treturn(c);
}


/*{\tGETLINE( mode )
 *
 *\tThis routine will fetch a line.
 *\tmode\t= APPEND, allow escape to cntlmode subroutine
 *\t\t  appending characters.
 *\t\t= REPLACE, allow escape to cntlmode subroutine
 *\t\t  replacing characters.
 *\t\t= SEARCH, no escape allowed
 *\t\t= ESC, enter control mode immediately
 *
 *\tThe cursor will always be positioned after the last
 * char printed.
 *
 *\tThis routine returns when cr, nl, or (eof in column 0) is
 * received (column 0 is the first char position).
 *
}*/

static void getline(register int mode)
{
\tregister int c;
\tregister int tmp;

\taddnl = TRUE;

\tif( mode == ESC )
\t{
\t\t/*** go directly to control mode ***/
\t\tgoto escape;
\t}

\tfor(;;)
\t{
\t\tif( (c = getchar()) == cntl(\'V\') )
\t\t{
\t\t\t/*** implement ^V to escape next char ***/
\t\t\tin_raw++;
\t\t\tc = getchar();
\t\t\tin_raw = 0;
\t\t\tappend(c, mode);
\t\t\trefresh(INPUT);
\t\t\tcontinue;
\t\t}

\t\tif( c == usreof )
\t\t\tc = UEOF;
\t\telse if( c == usrerase )
\t\t\tc = UERASE;
\t\telse if( c == usrkill )
\t\t\tc = UKILL;

\t\tswitch( c )
\t\t{
\t\tcase ESC:\t\t/** enter control mode **/
\t\t\tif( mode == SEARCH )
\t\t\t{
\t\t\t\tbell;
\t\t\t\tcontinue;
\t\t\t}
\t\t\telse
\t\t\t{
\tescape:
\t\t\t\tif( mode == REPLACE )
\t\t\t\t\t--cur_virt;
\t\t\t\ttmp = cntlmode();
\t\t\t\tif( tmp == ENTER || tmp == BIGVI )
\t\t\t\t{
#ifdef MULTIBYTE
\t\t\t\t\tbigvi = (tmp==BIGVI);
#endif /* MULTIBYTE */
\t\t\t\t\treturn;
\t\t\t\t}
\t\t\t\tif( tmp == INSERT )
\t\t\t\t{
\t\t\t\t\tmode = APPEND;
\t\t\t\t\tcontinue;
\t\t\t\t}
\t\t\t\tmode = tmp;
\t\t\t}
\t\t\tbreak;

\t\tcase UERASE:\t\t/** user erase char **/
\t\t\t\t/*** treat as backspace ***/

\t\tcase \'\\b\':\t\t/** backspace **/
\t\t\tif( virtual[cur_virt] == \'\\\\\' )
\t\t\t{
\t\t\t\tcdelete(1, BAD);
\t\t\t\tappend(usrerase, mode);
\t\t\t}
\t\t\telse
\t\t\t{
\t\t\t\tif( mode==SEARCH && cur_virt==0 )
\t\t\t\t{
\t\t\t\t\tfirst_virt = 0;
\t\t\t\t\tcdelete(1, BAD);
\t\t\t\t\treturn;
\t\t\t\t}
\t\t\t\tcdelete(1, BAD);
\t\t\t}
\t\t\tbreak;

\t\tcase cntl(\'W\'):\t\t/** delete back word **/
\t\t\tif( cur_virt > first_virt && isblank(cur_virt-1) )
\t\t\t{
\t\t\t\tcdelete(1, BAD);
\t\t\t}
\t\t\telse
\t\t\t{
\t\t\t\ttmp = cur_virt;
\t\t\t\tbackword(1, \'b\');
\t\t\t\tcdelete(tmp - cur_virt + 1, BAD);
\t\t\t}
\t\t\tbreak;

\t\tcase UKILL:\t\t/** user kill line char **/
\t\t\tif( virtual[cur_virt] == \'\\\\\' )
\t\t\t{
\t\t\t\tcdelete(1, BAD);
\t\t\t\tappend(usrkill, mode);
\t\t\t}
\t\t\telse
\t\t\t{
\t\t\t\tif( mode == SEARCH )
\t\t\t\t{
\t\t\t\t\tcur_virt = 1;
\t\t\t\t\tdelmotion(\'$\', BAD);
\t\t\t\t}
\t\t\t\telse if(first_virt)
\t\t\t\t{
\t\t\t\t\ttmp = cur_virt;
\t\t\t\t\tcur_virt = first_virt;
\t\t\t\t\tcdelete(tmp - cur_virt + 1, BAD);
\t\t\t\t}
\t\t\t\telse
\t\t\t\t\tdel_line(GOOD);
\t\t\t}
\t\t\tbreak;

\t\tcase UEOF:\t\t/** eof char **/
\t\t\tif( cur_virt != INVALID )
\t\t\t\tcontinue;
\t\t\taddnl = FALSE;

\t\tcase \'\\n\':\t\t/** newline or return **/
\t\t\tif( mode != SEARCH )
\t\t\t\tsave_last();
\t\t\treturn;

\t\tdefault:
\t\t\tif( mode == REPLACE )
\t\t\t{
\t\t\t\tif( cur_virt < last_virt )
\t\t\t\t{
\t\t\t\t\treplace(c, TRUE);
\t\t\t\t\tcontinue;
\t\t\t\t}
\t\t\t\tcdelete(1, BAD);
\t\t\t\tmode = APPEND;
\t\t\t}
\t\t\tappend(c, mode);
\t\t\tbreak;
\t\t}
\t\trefresh(INPUT);

\t}
}

/*{\tMVCURSOR( motion )
 *
 *\tThis routine will move the virtual cursor according to motion
 * for repeat times.
 *
 * It returns GOOD if successful; else BAD.
 *
}*/

static int mvcursor(register int motion)
{
\tregister int count;
\tregister int tcur_virt;
\tregister int incr = -1;
\tregister int bound = 0;
\tstatic int last_find = 0;\t/* last find command */

\tswitch(motion)
\t{
\t\t/***** Cursor move commands *****/

\tcase \'0\':\t\t/** First column **/
\t\ttcur_virt = 0;
\t\tbreak;

\tcase \'^\':\t\t/** First nonblank character **/
\t\ttcur_virt = first_virt;
\t\twhile( isblank(tcur_virt) && tcur_virt < last_virt )
\t\t\t++tcur_virt;
\t\tbreak;

\tcase \'|\':
\t\ttcur_virt = repeat-1;
\t\tif(tcur_virt <= last_virt)
\t\t\tbreak;
\t\t/* fall through */

\tcase \'$\':\t\t/** End of line **/
\t\ttcur_virt = last_virt;
\t\tbreak;

\tcase \'h\':\t\t/** Left one **/
\tcase \'\\b\':
\t\tmotion = first_virt;
\t\tgoto walk;

\tcase \' \':
\tcase \'l\':\t\t/** Right one **/
\t\tmotion = last_virt;
\t\tincr = 1;
\twalk:
\t\ttcur_virt = cur_virt;
\t\tif( incr*tcur_virt < motion)
\t\t{
\t\t\ttcur_virt += repeat*incr;
\t\t\tif( incr*tcur_virt > motion)
\t\t\t\ttcur_virt = motion;
\t\t}
\t\telse
\t\t{
\t\t\treturn(BAD);
\t\t}
\t\tbreak;

\tcase \'B\':
\tcase \'b\':\t\t/** back word **/
\t\ttcur_virt = cur_virt;
\t\tbackword(repeat, motion);
\t\tif( cur_virt == tcur_virt )
\t\t\treturn(BAD);
\t\treturn(GOOD);

\tcase \'E\':
\tcase \'e\':\t\t/** end of word **/
\t\ttcur_virt = cur_virt;
\t\tif(tcur_virt >=0)
\t\t\tendword(repeat, motion);
\t\tif( cur_virt == tcur_virt )
\t\t\treturn(BAD);
\t\treturn(GOOD);

\tcase \',\':\t\t/** reverse find old char **/
\tcase \';\':\t\t/** find old char **/
\t\tswitch(last_find)
\t\t{
\t\tcase \'t\':
\t\tcase \'f\':
\t\t\tif(motion==\';\')
\t\t\t{
\t\t\t\tbound = last_virt;
\t\t\t\tincr = 1;
\t\t\t}
\t\t\tgoto find_b;

\t\tcase \'T\':
\t\tcase \'F\':
\t\t\tif(motion==\',\')
\t\t\t{
\t\t\t\tbound = last_virt;
\t\t\t\tincr = 1;
\t\t\t}
\t\t\tgoto find_b;

\t\tdefault:
\t\t\treturn(BAD);
\t\t}


\tcase \'t\':\t\t/** find up to new char forward **/
\tcase \'f\':\t\t/** find new char forward **/
\t\tbound = last_virt;
\t\tincr = 1;

\tcase \'T\':\t\t/** find up to new char backward **/
\tcase \'F\':\t\t/** find new char backward **/
\t\tlast_find = motion;
\t\tif((findchar=getrchar())==ESC)
\t\t\treturn(GOOD);
find_b:
\t\ttcur_virt = cur_virt;
\t\tcount = repeat;
\t\twhile( count-- )
\t\t{
\t\t\twhile( incr*(tcur_virt+=incr) <= bound
\t\t\t\t&& virtual[tcur_virt] != findchar );
\t\t\tif( incr*tcur_virt > bound )
\t\t\t{
\t\t\t\treturn(BAD);
\t\t\t}
\t\t}
\t\tif( fold(last_find) == \'T\' )
\t\t\ttcur_virt -= incr;
\t\tbreak;

\t/* new, undocumented feature */
        case \'%\':
\t{
\t\tint nextmotion;
\t\tint nextc;
\t\ttcur_virt = cur_virt;
\t\twhile( tcur_virt <= last_virt
\t\t\t&& strchr(paren_chars,virtual[tcur_virt])==(char*)0)
\t\t\t\ttcur_virt++;
\t\tif(tcur_virt > last_virt )
\t\t\treturn(BAD);
\t\tnextc = virtual[tcur_virt];
\t\tcount = strchr(paren_chars,nextc)-paren_chars;
\t\tif(count < 3)
\t\t{
\t\t\tincr = 1;
\t\t\tbound = last_virt;
\t\t\tnextmotion = paren_chars[count+3];
\t\t}
\t\telse
\t\t\tnextmotion = paren_chars[count-3];
\t\tcount = 1;
\t\twhile(count >0 &&  incr*(tcur_virt+=incr) <= bound)
\t\t{
\t\t        if(virtual[tcur_virt] == nextmotion)
\t\t        \tcount--;
\t\t        else if(virtual[tcur_virt]==nextc)
\t\t        \tcount++;
\t\t}
\t\tif(count)
\t\t\treturn(BAD);
\t\tbreak;
\t}

\tcase \'W\':
\tcase \'w\':\t\t/** forward word **/
\t\ttcur_virt = cur_virt;
\t\tforward(repeat, motion);
\t\tif( tcur_virt == cur_virt )
\t\t\treturn(BAD);
\t\treturn(GOOD);

\tdefault:
\t\treturn(BAD);
\t}
\tcur_virt = tcur_virt;

\treturn(GOOD);
}

/*{\tPR_PROMPT()
 *
 *\tPrint the prompt.
 *
}*/

static void pr_prompt(void)
{
\tpr_string(Prompt);
\treturn;
}

/*
 * print a string
 */

static void pr_string(register const char *sp)
{
\t/*** copy string sp ***/
\tregister char *ptr = editb.e_outptr;
\twhile(*sp)
\t\t*ptr++ = *sp++;
\teditb.e_outptr = ptr;
\treturn;
}

/*{\tPUTSTRING( column, nchars )
 *
 *\tPut nchars starting at column of physical into the workspace
 * to be printed.
 *
}*/

static void putstring(register int col, register int nchars)
{
\twhile( nchars-- )
\t\tputchar(physical[col++]);
\treturn;
}

/*{\tREFRESH( mode )
 *
 *\tThis routine will refresh the crt so the physical image matches
 * the virtual image and display the proper window.
 *
 *\tmode\t= CONTROL, refresh in control mode, ie. leave cursor
 *\t\t\tpositioned at last char printed.
 *\t\t= INPUT, refresh in input mode; leave cursor positioned
 *\t\t\tafter last char printed.
 *\t\t= TRANSLATE, perform virtual to physical translation
 *\t\t\tand adjust left margin only.
 *
 *\t\t+-------------------------------+
 *\t\t|   | |    virtual\t  | |   |
 *\t\t+-------------------------------+
 *\t\t  cur_virt\t\tlast_virt
 *
 *\t\t+-----------------------------------------------+
 *\t\t|\t  | |\t        physical\t | |    |
 *\t\t+-----------------------------------------------+
 *\t\t\tcur_phys\t\t\tlast_phys
 *
 *\t\t\t\t0\t\t\tw_size - 1
 *\t\t\t\t+-----------------------+
 *\t\t\t\t| | |  window\t\t|
 *\t\t\t\t+-----------------------+
 *\t\t\t\tcur_window = cur_phys - first_wind
}*/

static void refresh(int mode)
{
\tregister int p;
\tregister int regb;
\tregister int first_w = first_wind;
\tint p_differ;
\tint new_lw;
\tint ncur_phys;
\tint opflag;\t\t\t/* search optimize flag */

#\tdefine\tw\tregb
#\tdefine\tv\tregb

\t/*** find out if it\'s necessary to start translating at beginning ***/

\tif(lookahead>0)
\t{
\t\tp = previous[lookahead-1];
\t\tif(p != ESC && p != \'\\n\' && p != \'\\r\')
\t\t\tmode = TRANSLATE;
\t}
\tv = cur_virt;
\tif( v<ocur_virt || ocur_virt==INVALID
\t\t|| ( v==ocur_virt
\t\t\t&& (!is_print(virtual[v]) || !is_print(o_v_char))) )
\t{
\t\topflag = FALSE;
\t\tp = 0;
\t\tv = 0;
\t}
\telse
\t{
\t\topflag = TRUE;
\t\tp = ocur_phys;
\t\tv = ocur_virt;
\t\tif( !is_print(virtual[v]) )
\t\t{
\t\t\t/*** avoid double ^\'s ***/
\t\t\t++p;
\t\t\t++v;
\t\t}
\t}
\tvirtual[last_virt+1] = 0;
\tncur_phys = ed_virt_to_phys(virtual,physical,cur_virt,v,p);
\tp = genlen(physical);
\tif( --p < 0 )
\t\tlast_phys = 0;
\telse
\t\tlast_phys = p;

\t/*** see if this was a translate only ***/

\tif( mode == TRANSLATE )
\t\treturn;

\t/*** adjust left margin if necessary ***/

\tif( ncur_phys<first_w || ncur_phys>=(first_w + w_size) )
\t{
\t\tcursor(first_w);
\t\tfirst_w = ncur_phys - (w_size>>1);
\t\tif( first_w < 0 )
\t\t\tfirst_w = 0;
\t\tfirst_wind = cur_phys = first_w;
\t}

\t/*** attempt to optimize search somewhat to find ***/
\t/*** out where physical and window images differ ***/

\tif( first_w==ofirst_wind && ncur_phys>=ocur_phys && opflag==TRUE )
\t{
\t\tp = ocur_phys;
\t\tw = p - first_w;
\t}
\telse
\t{
\t\tp = first_w;
\t\tw = 0;
\t}

\tfor(; (p<=last_phys && w<=last_wind); ++p, ++w)
\t{
\t\tif( window[w] != physical[p] )
\t\t\tbreak;
\t}
\tp_differ = p;

\tif( (p>last_phys || p>=first_w+w_size) && w>last_wind
\t\t&& cur_virt==ocur_virt )
\t{
\t\t/*** images are identical ***/
\t\treturn;
\t}

\t/*** copy the physical image to the window image ***/

\tif( last_virt != INVALID )
\t{
\t\twhile( p <= last_phys && w < w_size )
\t\t\twindow[w++] = physical[p++];
\t}
\tnew_lw = w;

\t/*** erase trailing characters if needed ***/

\twhile( w <= last_wind )
\t\twindow[w++] = \' \';
\tlast_wind = --w;

\tp = p_differ;

\t/*** move cursor to start of difference ***/

\tcursor(p);

\t/*** and output difference ***/

\tw = p - first_w;
\twhile( w <= last_wind )
\t\tputchar(window[w++]);

\tcur_phys = w + first_w;
\tlast_wind = --new_lw;

\tif( last_phys >= w_size )
\t{
\t\tif( first_w == 0 )
\t\t\tlong_char = \'>\';
\t\telse if( last_phys < (first_w+w_size) )
\t\t\tlong_char = \'<\';
\t\telse
\t\t\tlong_char = \'*\';
\t}
\telse
\t\tlong_char = \' \';

\tif( long_line != long_char )
\t{
\t\t/*** indicate lines longer than window ***/
\t\twhile( w++ < w_size )
\t\t{
\t\t\tputchar(\' \');
\t\t\t++cur_phys;
\t\t}
\t\tputchar(long_char);
\t\t++cur_phys;
\t\tlong_line = long_char;
\t}

\tocur_phys = ncur_phys;
\tocur_virt = cur_virt;
\tofirst_wind = first_w;

\tif( mode==INPUT && cur_virt>INVALID )
\t\t++ncur_phys;

\tcursor(ncur_phys);
\ted_flush();
\treturn;
}

/*{\tREPLACE( char, increment )
 *
 *\tReplace the cur_virt character with char.  This routine attempts
 * to avoid using refresh().
 *
 *\tincrement\t= TRUE, increment cur_virt after replacement.
 *\t\t\t= FALSE, leave cur_virt where it is.
 *
}*/

static void replace(register int c, register int increment)
{
\tregister int cur_window;

\tif( cur_virt == INVALID )
\t{
\t\t/*** can\'t replace invalid cursor ***/
\t\tbell;
\t\treturn;
\t}
\tcur_window = cur_phys - first_wind;
\tif( ocur_virt == INVALID || !is_print(c)
\t\t|| !is_print(virtual[cur_virt])
\t\t|| !is_print(o_v_char)
#ifdef MULTIBYTE
\t\t|| icharset(c) || out_csize(icharset(o_v_char))>1
#endif /* MULTIBYTE */
\t\t|| (increment==TRUE && (cur_window==w_size-1)
\t\t\t|| !is_print(virtual[cur_virt+1])) )
\t{
\t\t/*** must use standard refresh routine ***/

\t\tcdelete(1, BAD);
\t\tappend(c, APPEND);
\t\tif( increment==TRUE && cur_virt<last_virt )
\t\t\t++cur_virt;
\t\trefresh(CONTROL);
\t}
\telse
\t{
\t\tvirtual[cur_virt] = c;
\t\tphysical[cur_phys] = c;
\t\twindow[cur_window] = c;
\t\tputchar(c);
\t\tif( increment == TRUE )
\t\t{
\t\t\tc = virtual[++cur_virt];
\t\t\t++cur_phys;
\t\t}
\t\telse
\t\t{
\t\t\tputchar(\'\\b\');
\t\t}
\t\to_v_char = c;
\t\ted_flush();
\t}
\treturn;
}

/*
#ifdef xxx
#endif
*/
/*{\tRESTORE_V()
 *
 *\tRestore the contents of virtual space from u_space.
 *
}*/

static void restore_v(void)
{
\tregister int tmpcol;
\tgenchar tmpspace[MAXLINE];

\tif( u_column == INVALID-1 )
\t{
\t\t/*** never saved anything ***/
\t\tbell;
\t\treturn;
\t}
\tgencpy(tmpspace, u_space);
\ttmpcol = u_column;
\tsave_v();
\tgencpy(virtual, tmpspace);
\tcur_virt = tmpcol;
\tlast_virt = genlen(tmpspace) - 1;
\tocur_virt = MAXCHAR;\t/** invalidate refresh optimization **/
\treturn;
}

/*{\tSAVE_LAST()
 *
 *\tIf the user has typed something, save it in last line.
 *
}*/

static void save_last(void)
{
\tregister int i;

\tif( (i = cur_virt - first_virt + 1) > 0 )
\t{
\t\t/*** save last thing user typed ***/
\t\tgenncpy(lastline, (&virtual[first_virt]), i);
\t\tlastline[i] = \'\\0\';
\t}
\treturn;
}

/*{\tSAVE_V()
 *
 *\tThis routine will save the contents of virtual in u_space.
 *
}*/

static void save_v(void)
{
\tif(!inmacro)
\t{
\t\tvirtual[last_virt + 1] = \'\\0\';
\t\tgencpy(u_space, virtual);
\t\tu_column = cur_virt;
\t}
\treturn;
}

/*{\tSEARCH( mode )
 *
 *\tSearch history file for regular expression.
 *
 *\tmode\t= \'/\'\trequire search string and search new to old
 *\tmode\t= \'?\'\trequire search string and search old to new
 *\tmode\t= \'N\'\trepeat last search in reverse direction
 *\tmode\t= \'n\'\trepeat last search
 *
}*/

static int search(register int mode)
{
\tregister int new_direction;
\tregister int oldcurhline;
\tstatic int direction = -1;
\thistloc_t  location;

\tif( mode == \'/\' || mode == \'?\')
\t{
\t\t/*** new search expression ***/
\t\tdel_line(BAD);
\t\tappend(mode, APPEND);
\t\trefresh(INPUT);
\t\tfirst_virt = 1;
\t\tgetline(SEARCH);
\t\tfirst_virt = 0;
\t\tvirtual[last_virt + 1] = \'\\0\';\t/*** make null terminated ***/
\t\tdirection = mode==\'/\' ? -1 : 1;
\t}

\tif( cur_virt == INVALID )
\t{
\t\t/*** no operation ***/
\t\treturn(ABORT);
\t}

\tif( cur_virt==0 ||  fold(mode)==\'N\' )
\t{
\t\t/*** user wants repeat of last search ***/
\t\tdel_line(BAD);
\t\tstrcpy( ((char*)virtual)+1, lsearch);
#ifdef MULTIBYTE
\t\t*((char*)virtual) = \'/\';
\t\ted_internal((char*)virtual,virtual);
#endif /* MULTIBYTE */
\t}

\tif( mode == \'N\' )
\t\tnew_direction = -direction;
\telse
\t\tnew_direction = direction;

\tif( new_direction==1 && curhline >= histmax )
\t\tcurhline = histmin + 1;

\t/*** now search ***/

\toldcurhline = curhline;
#ifdef MULTIBYTE
\ted_external(virtual,(char*)virtual);
#endif /* MULTIBYTE */
\tlocation = hist_find(((char*)virtual)+1, curhline, 1, new_direction);
\tstrncpy(lsearch, ((char*)virtual)+1, SEARCHSIZE);
\tif( (curhline=location.hist_command) >=0 )
\t{
\t\treturn(GOOD);
\t}

\t/*** could not find matching line ***/

\tcurhline = oldcurhline;
\treturn(BAD);
}

/*{\tSYNC_CURSOR()
 *
 *\tThis routine will move the physical cursor to the same
 * column as the virtual cursor.
 *
}*/

static void sync_cursor(void)
{
\tregister int p;
\tregister int v;
\tregister int c;
\tint new_phys;

\tif( cur_virt == INVALID )
\t\treturn;

\t/*** find physical col that corresponds to virtual col ***/

\tnew_phys = 0;
\tif(first_wind==ofirst_wind && cur_virt>ocur_virt && ocur_virt!=INVALID)
\t{
\t\t/*** try to optimize search a little ***/
\t\tp = ocur_phys + 1;
#ifdef MULTIBYTE
\t\twhile(physical[p]==MARKER)
\t\t\tp++;
#endif /* MULTIBYTE */
\t\tv = ocur_virt + 1;
\t}
\telse
\t{
\t\tp = 0;
\t\tv = 0;
\t}
\tfor(; v <= last_virt; ++p, ++v)
\t{
#ifdef MULTIBYTE
\t\tint d;
\t\tc = virtual[v];
\t\tif(d = icharset(c))
\t\t{
\t\t\tif( v != cur_virt )
\t\t\t\tp += (out_csize(d)-1);
\t\t}
\t\telse
#else
\t\tc = virtual[v];
#endif\t/* MULTIBYTE */
\t\tif( !isprint(c) )
\t\t{
\t\t\tif( c == \'\\t\' )
\t\t\t{
\t\t\t\tp -= ((p+editb.e_plen)%TABSIZE);
\t\t\t\tp += (TABSIZE-1);
\t\t\t}
\t\t\telse
\t\t\t{
\t\t\t\t++p;
\t\t\t}
\t\t}
\t\tif( v == cur_virt )
\t\t{
\t\t\tnew_phys = p;
\t\t\tbreak;
\t\t}
\t}

\tif( new_phys < first_wind || new_phys >= first_wind + w_size )
\t{
\t\t/*** asked to move outside of window ***/

\t\twindow[0] = \'\\0\';
\t\trefresh(CONTROL);
\t\treturn;
\t}

\tcursor(new_phys);
\ted_flush();
\tocur_phys = cur_phys;
\tocur_virt = cur_virt;
\to_v_char = virtual[ocur_virt];

\treturn;
}

/*{\tTEXTMOD( command, mode )
 *
 *\tModify text operations.
 *
 *\tmode != 0, repeat previous operation
 *
}*/

static int textmod(register int c, int mode)
{
\tregister int i;
\tregister genchar *p = lastline;
\tregister int trepeat = repeat;
\tstatic int lastmacro;
\tgenchar *savep;

\tif(mode && (fold(lastmotion)==\'F\' || fold(lastmotion)==\'T\')) 
\t\tlastmotion = \';\';

\tif( fold(c) == \'P\' )
\t{
\t\t/*** change p from lastline to yankbuf ***/
\t\tp = yankbuf;
\t}

addin:
\tswitch( c )
\t{
\t\t\t/***** Input commands *****/

#ifdef KSHELL
\tcase \'*\':\t\t/** do file name expansion in place **/
\tcase \'\\\\\':\t\t/** do file name completion in place **/
\t\tif( cur_virt == INVALID )
\t\t\treturn(BAD);
\tcase \'=\':\t\t/** list file name expansions **/
\t\tsave_v();
\t\ti = last_virt;
\t\t++last_virt;
\t\tvirtual[last_virt] = 0;
\t\tif( ed_expand((char*)virtual, &cur_virt, &last_virt, c) )
\t\t{
\t\t\tlast_virt = i;
\t\t\tbell;
\t\t}
\t\telse if(c == \'=\')
\t\t{
\t\t\tlast_virt = i;
\t\t\tnonewline++;
\t\t\ted_ungetchar(cntl(\'L\'));
\t\t\treturn(GOOD);
\t\t}
\t\telse
\t\t{
\t\t\t--cur_virt;
\t\t\t--last_virt;
\t\t\tocur_virt = MAXCHAR;
\t\t\treturn(APPEND);
\t\t}
\t\tbreak;

\tcase \'@\':\t\t/** macro expansion **/
\t\tif( mode )
\t\t\tc = lastmacro;
\t\telse
\t\t\tif((c=getrchar())==ESC)
\t\t\t\treturn(GOOD);
\t\tif(!inmacro)
\t\t\tlastmacro = c;
\t\tif(ed_macro(c))
\t\t{
\t\t\tsave_v();
\t\t\tinmacro++;
\t\t\treturn(GOOD);
\t\t}
\t\tbell;
\t\treturn(BAD);

#endif\t/* KSHELL */
\tcase \'_\':\t\t/** append last argument of prev command **/
\t\tsave_v();
\t\t{
\t\t\tgenchar tmpbuf[MAXLINE];
\t\t\tif(repeat_set==0)
\t\t\t\trepeat = -1;
\t\t\tp = (genchar*)hist_word(tmpbuf,MAXLINE,repeat);
#ifndef KSHELL
\t\t\tif(p==0)
\t\t\t{
\t\t\t\tbell;
\t\t\t\tbreak;
\t\t\t}
#endif\t/* KSHELL */
#ifdef MULTIBYTE
\t\t\ted_internal((char*)p,tmpbuf);
\t\t\tp = tmpbuf;
#endif /* MULTIBYTE */
\t\t\ti = \' \';
\t\t\tdo
\t\t\t{
\t\t\t\tappend(i,APPEND);
\t\t\t}
\t\t\twhile(i = *p++);
\t\t\treturn(APPEND);
\t\t}

\tcase \'A\':\t\t/** append to end of line **/
\t\tcur_virt = last_virt;
\t\tsync_cursor();

\tcase \'a\':\t\t/** append **/
\t\tif( fold(mode) == \'A\' )
\t\t{
\t\t\tc = \'p\';
\t\t\tgoto addin;
\t\t}
\t\tsave_v();
\t\tif( cur_virt != INVALID )
\t\t{
\t\t\tfirst_virt = cur_virt + 1;
\t\t\tcursor(cur_phys + 1);
\t\t\ted_flush();
\t\t}
\t\treturn(APPEND);

\tcase \'I\':\t\t/** insert at beginning of line **/
\t\tcur_virt = first_virt;
\t\tsync_cursor();

\tcase \'i\':\t\t/** insert **/
\t\tif( fold(mode) == \'I\' )
\t\t{
\t\t\tc = \'P\';
\t\t\tgoto addin;
\t\t}
\t\tsave_v();
\t\tif( cur_virt != INVALID )
 \t\t{
 \t\t\to_v_char = virtual[cur_virt];
\t\t\tfirst_virt = cur_virt--;
  \t\t}
\t\treturn(INSERT);

\tcase \'C\':\t\t/** change to eol **/
\t\tc = \'$\';
\t\tgoto chgeol;

\tcase \'c\':\t\t/** change **/
\t\tif( mode )
\t\t\tc = lastmotion;
\t\telse
\t\t\tc = getcount(getchar());
chgeol:
\t\tlastmotion = c;
\t\tif( c == \'c\' )
\t\t{
\t\t\tdel_line(GOOD);
\t\t\treturn(APPEND);
\t\t}

\t\tif( delmotion(c, \'c\') == BAD )
\t\t\treturn(BAD);

\t\tif( mode == \'c\' )
\t\t{
\t\t\tc = \'p\';
\t\t\ttrepeat = 1;
\t\t\tgoto addin;
\t\t}
\t\tfirst_virt = cur_virt + 1;
\t\treturn(APPEND);

\tcase \'D\':\t\t/** delete to eol **/
\t\tc = \'$\';
\t\tgoto deleol;

\tcase \'d\':\t\t/** delete **/
\t\tif( mode )
\t\t\tc = lastmotion;
\t\telse
\t\t\tc = getcount(getchar());
deleol:
\t\tlastmotion = c;
\t\tif( c == \'d\' )
\t\t{
\t\t\tdel_line(GOOD);
\t\t\tbreak;
\t\t}
\t\tif( delmotion(c, \'d\') == BAD )
\t\t\treturn(BAD);
\t\tif( cur_virt < last_virt )
\t\t\t++cur_virt;
\t\tbreak;

\tcase \'P\':
\t\tif( p[0] == \'\\0\' )
\t\t\treturn(BAD);
\t\tif( cur_virt != INVALID )
\t\t{
\t\t\ti = virtual[cur_virt];
\t\t\tif(!is_print(i))
\t\t\t\tocur_virt = INVALID;
\t\t\t--cur_virt;
\t\t}

\tcase \'p\':\t\t/** print **/
\t\tif( p[0] == \'\\0\' )
\t\t\treturn(BAD);

\t\tif( mode != \'s\' && mode != \'c\' )
\t\t{
\t\t\tsave_v();
\t\t\tif( c == \'P\' )
\t\t\t{
\t\t\t\t/*** fix stored cur_virt ***/
\t\t\t\t++u_column;
\t\t\t}
\t\t}
\t\tif( mode == \'R\' )
\t\t\tmode = REPLACE;
\t\telse
\t\t\tmode = APPEND;
\t\tsavep = p;
\t\tfor(i=0; i<trepeat; ++i)
\t\t{
\t\t\twhile(c= *p++)
\t\t\t\tappend(c,mode);
\t\t\tp = savep;
\t\t}
\t\tbreak;

\tcase \'R\':\t\t/* Replace many chars **/
\t\tif( mode == \'R\' )
\t\t{
\t\t\tc = \'P\';
\t\t\tgoto addin;
\t\t}
\t\tsave_v();
\t\tif( cur_virt != INVALID )
\t\t\tfirst_virt = cur_virt;
\t\treturn(REPLACE);

\tcase \'r\':\t\t/** replace **/
\t\tif( mode )
\t\t\tc = *p;
\t\telse
\t\t\tif((c=getrchar())==ESC)
\t\t\t\treturn(GOOD);
\t\t*p = c;
\t\tsave_v();
\t\twhile(trepeat--)
\t\t\treplace(c, trepeat!=0);
\t\treturn(GOOD);

\tcase \'S\':\t\t/** Substitute line - cc **/
\t\tc = \'c\';
\t\tgoto chgeol;

\tcase \'s\':\t\t/** substitute **/
\t\tsave_v();
\t\tcdelete(repeat, BAD);
\t\tif( mode )
\t\t{
\t\t\tc = \'p\';
\t\t\ttrepeat = 1;
\t\t\tgoto addin;
\t\t}
\t\tfirst_virt = cur_virt + 1;
\t\treturn(APPEND);

\tcase \'Y\':\t\t/** Yank to end of line **/
\t\tc = \'$\';
\t\tgoto yankeol;

\tcase \'y\':\t\t/** yank thru motion **/
\t\tif( mode )
\t\t\tc = lastmotion;
\t\telse
\t\t\tc = getcount(getchar());
yankeol:
\t\tlastmotion = c;
\t\tif( c == \'y\' )
\t\t{
\t\t\tgencpy(yankbuf, virtual);
\t\t}
\t\telse if( delmotion(c, \'y\') == BAD )
\t\t{
\t\t\treturn(BAD);
\t\t}
\t\tbreak;

\tcase \'x\':\t\t/** delete repeat chars forward - dl **/
\t\tc = \'l\';
\t\tgoto deleol;

\tcase \'X\':\t\t/** delete repeat chars backward - dh **/
\t\tc = \'h\';
\t\tgoto deleol;

\tcase \'~\':\t\t/** invert case and advance **/
\t\tif( cur_virt != INVALID )
\t\t{
\t\t\tsave_v();
\t\t\ti = INVALID;
\t\t\twhile(trepeat-->0 && i!=cur_virt)
\t\t\t{
\t\t\t\ti = cur_virt;
\t\t\t\tc = virtual[cur_virt];
#ifdef MULTIBYTE
\t\t\t\tif((c&~STRIP)==0)
#endif /* MULTIBYTE */
\t\t\t\tif( isupper(c) )
\t\t\t\t\tc = tolower(c);
\t\t\t\telse if( islower(c) )
\t\t\t\t\tc = toupper(c);
\t\t\t\treplace(c, TRUE);
\t\t\t}
\t\t\treturn(GOOD);
\t\t}
\t\telse
\t\t\treturn(BAD);

\tdefault:
\t\treturn(BAD);
\t}
\trefresh(CONTROL);
\treturn(GOOD);
}

#ifdef INT16

/* making these functions reduces the size of the text region */

static int isalph(register int c)
{
\tregister int v = virtual[c];
\treturn(isalnum(v));
}

static int isblank(register int c)
{
\tregister int v = virtual[c];
\treturn(isspace(v));
}

static int ismetach(register int c)
{
\tregister int v = virtual[c];
\treturn(ismeta(v));
}

#endif\t/* INT16 */


#ifdef MULTIBYTE
int isalph(c)
register int c;
{
\tregister int v = virtual[c];
\treturn((v&~STRIP) || isalnum(v));
}


int isblank(c)
register int c;
{
\tregister int v = virtual[c];
\treturn((v&~STRIP)==0 && isspace(v));
}

int ismetach(c)
register int c;
{
\tregister int v = virtual[c];
\treturn((v&~STRIP)==0 && ismeta(v));
}

#endif\t/* MULTIBYTE */

/*
 * get a character, after ^V processing
 */
static int getrchar()
{
\tregister int c;
\tif((c=getchar())== cntl(\'V\'))
\t{
\t\tin_raw++;
\t\tc = getchar();
\t\tin_raw = 0;
\t}
\treturn(c);
}'
		OUTPUT - $'                  
/* Adapted for ksh by David Korn */
/*+\tVI.C\t\t\tP.D. Sullivan
 *
 *\tOne line editor for the shell based on the vi editor.
 *
 *\t\tcbosgd!pds
-*/


#include\t<errno.h>
#include\t<stdlib.h>
#include\t<string.h>

#ifdef KSHELL
#   include\t"defs.h"
#else
#   include\t"io.h"
#endif\t/* KSHELL */

#include\t"history.h"
#include\t"edit.h"
#include\t"terminal.h"

#ifdef OLDTERMIO
#   undef ECHOCTL
    extern __MANGLE__ char echoctl;
#else
#   ifdef ECHOCTL
#\tdefine echoctl\tECHOCTL
#   else
#\tdefine echoctl\t0
#   endif /* ECHOCTL */
#endif /*OLDTERMIO */

#ifndef FIORDCHK
#   define NTICKS\t5\t\t/* number of ticks for typeahead */
#   ifndef KSHELL
#\tifdef _sys_times_
#\tinclude\t<sys/times.h>
#\telse
  \t   struct tms
\t   {
\t\ttime_t\ttms_utime;
\t\ttime_t\ttms_stime;
\t\ttime_t\ttms_cutime;
\t\ttime_t\ttms_cstime;
\t   };
#\tendif /* _sys_times */
#   endif /* KSHELL */
#endif /* FIORDCHK */

#define\tMAXCHAR\tMAXLINE-2\t\t/* max char per line */
#define\tWINDOW\tMAXWINDOW\t\t/* max char in window of which */
\t\t\t\t\t/* WINDOW-2 are available to user */
\t\t\t\t\t/* actual window size may be smaller */


#undef isblank
#ifdef MULTIBYTE
    static int bigvi;
#   define gencpy(a,b)\ted_gencpy(a,b)
#   define genncpy(a,b,n)\ted_genncpy(a,b,n)
#   define genlen(str)\ted_genlen(str)
#   define digit(c)\t((c&~STRIP)==0 && isdigit(c))
#   define is_print(c)\t((c&~STRIP) || isprint(c))
#else
#   define gencpy(a,b)\tstrcpy((char*)(a),(char*)(b))
#   define genncpy(a,b,n) strncpy((char*)(a),(char*)(b),n)
#   define genlen(str)\tstrlen(str)
#   define isalph(v)\tisalnum(virtual[v])
#   define isblank(v)\tisspace(virtual[v])
#   define ismetach(v)\tismeta(virtual[v])
#   define digit(c)\tisdigit(c)
#   define is_print(c)\tisprint(c)
#endif\t/* MULTIBYTE */
#define fold(c)\t\t((c)&~040)\t/* lower and uppercase equivalent */
#ifdef INT16
/* save space by defining functions for these */
#   undef isalph
#   undef isblank
#   undef ismetach
    static int isalph __PROTO__((int));
    static int isblank __PROTO__((int));
    static int ismetach __PROTO__((int));
#endif\t/* INT16 */

#undef putchar
#undef getchar
#define getchar()\ted_getchar()
#define putchar(c)\ted_putchar(c)
#define bell\t\ted_ringbell()\t/* ring terminal\'s bell */
#define crlf\t\ted_crlf()\t/* return and linefeed */

#define in_raw\t\teditb.e_addnl\t\t/* next char input is raw */
#define crallowed\teditb.e_crlf
#define cur_virt\teditb.e_cur\t\t/* current virtual column */
#define cur_phys\teditb.e_pcur\t/* current phys column cursor is at */
#define curhline\teditb.e_hline\t\t/* current history line */
#define env\t\teditb.e_env
#define fildes\t\teditb.e_fd
#define findchar\teditb.e_fchar\t\t/* last find char */
#define first_virt\teditb.e_fcol\t\t/* first allowable column */
#define first_wind\teditb.e_globals[0]\t/* first column of window */
#define\tglobals\t\teditb.e_globals\t\t/* local global variables */
#define histmin\t\teditb.e_hismin
#define histmax\t\teditb.e_hismax
#define last_phys\teditb.e_peol\t\t/* last column in physical */
#define last_virt\teditb.e_eol\t\t/* last column */
#define last_wind\teditb.e_globals[1]\t/* last column in window */
#define\tlastmotion\teditb.e_globals[2]\t/* last motion */
#define lastrepeat\teditb.e_mode\t/* last repeat count for motion cmds */
#define\tlong_char\teditb.e_globals[3]\t/* line bigger than window */
#define\tlong_line\teditb.e_globals[4]\t/* line bigger than window */
#define lsearch\t\teditb.e_search\t\t/* last search string */
#define lookahead\teditb.e_index\t\t/* characters in buffer */
#define previous\teditb.e_lbuf\t\t/* lookahead buffer */
#define max_col\t\teditb.e_llimit\t\t/* maximum column */
#define\tocur_phys\teditb.e_globals[5]   /* old current physical position */
#define\tocur_virt\teditb.e_globals[6]\t/* old last virtual position */
#define\tofirst_wind\teditb.e_globals[7]\t/* old window first col */
#define\to_v_char\teditb.e_globals[8]\t/* prev virtual[ocur_virt] */
#define Prompt\t\teditb.e_prompt\t\t/* pointer to prompt */
#define plen\t\teditb.e_plen\t\t/* length of prompt */
#define physical\teditb.e_physbuf\t\t/* physical image */
#define repeat\t\teditb.e_repeat\t    /* repeat count for motion cmds */
#define ttyspeed\teditb.e_ttyspeed\t/* tty speed */
#define u_column\teditb.e_ucol\t\t/* undo current column */
#define U_saved\t\teditb.e_saved\t\t/* original virtual saved */
#define U_space\t\teditb.e_Ubuf\t\t/* used for U command */
#define u_space\t\teditb.e_ubuf\t\t/* used for u command */
#define usreof\t\teditb.e_eof\t\t/* user defined eof char */
#define usrerase\teditb.e_erase\t\t/* user defined erase char */
#define usrintr\t\teditb.e_intr\t\t/* user defined intr char */
#define usrkill\t\teditb.e_kill\t\t/* user defined kill char */
#define usrquit\t\teditb.e_quit\t\t/* user defined quit char */
#define virtual\t\teditb.e_inbuf\t/* pointer to virtual image buffer */
#define\twindow\t\teditb.e_window\t\t/* window buffer */
#define\tw_size\t\teditb.e_wsize\t\t/* window size */
#define\tinmacro\t\teditb.e_inmacro\t\t/* true when in macro */
#define yankbuf\t\teditb.e_killbuf\t\t/* yank/delete buffer */

extern __MANGLE__ clock_t times(__VARARG__);

#define\tABORT\t-2\t\t\t/* user abort */
#define\tAPPEND\t-10\t\t\t/* append chars */
#define\tBAD\t-1\t\t\t/* failure flag */
#define\tBIGVI\t-15\t\t\t/* user wants real vi */
#define\tCONTROL\t-20\t\t\t/* control mode */
#define\tENTER\t-25\t\t\t/* enter flag */
#define\tGOOD\t0\t\t\t/* success flag */
#define\tINPUT\t-30\t\t\t/* input mode */
#define\tINSERT\t-35\t\t\t/* insert mode */
#define\tREPLACE\t-40\t\t\t/* replace chars */
#define\tSEARCH\t-45\t\t\t/* search flag */
#define\tTRANSLATE\t-50\t\t/* translate virt to phys only */

#define\tDEL\t\'\\177\'\t\t\t/* interrupt char */

#define\tTRUE\t1
#define\tFALSE\t0

#define\tINVALID\t(-1)\t\t\t/* invalid column */
#define\tQUIT_C\t\'\\34\'\t\t\t/* quit char */
#define\tSYSERR\t(-1)\t\t\t/* system error */

static char addnl;\t\t\t/* boolean - add newline flag */
static char last_cmd = \'\\0\';\t\t/* last command */
static char repeat_set;
static char nonewline;
static genchar *lastline;
static char paren_chars[] = "([{)]}";   /* for % command */

#ifdef FIORDCHK
    static clock_t typeahead;\t\t/* typeahead occurred */
#else
    static int typeahead;\t\t/* typeahead occurred */
#endif\t/* FIORDCHK */

static void\tdel_line __PROTO__((int));
static int\tgetcount __PROTO__((int));
static void\tgetline __PROTO__((int));
static int\tgetrchar __PROTO__((void));
static int\tmvcursor __PROTO__((int));
static void\tpr_prompt __PROTO__((void));
static void\tpr_string __PROTO__((const char*));
static void\tputstring __PROTO__((int, int));
static void\trefresh __PROTO__((int));
static void\treplace __PROTO__((int, int));
static void\trestore_v __PROTO__((void));
static void\tsave_last __PROTO__((void));
static void\tsave_v __PROTO__((void));
static int\tsearch __PROTO__((int));
static void\tsync_cursor __PROTO__((void));
static int\ttextmod __PROTO__((int,int));

/*+\tVI_READ( fd, shbuf, nchar )
 *
 *\tThis routine implements a one line version of vi and is
 * called by _filbuf.c
 *
-*/

vi_read __PARAM__((int fd, register char *shbuf, unsigned nchar), (fd, shbuf, nchar)) __OTORP__(int fd; register char *shbuf; unsigned nchar;)
#line 205
{
\tregister int i;\t\t\t/* general variable */
\tregister int c;\t\t\t/* general variable */
\tregister int term_char;\t/* read() termination character */
\tchar prompt[PRSIZE+2];\t\t/* prompt */
\tgenchar Physical[2*MAXLINE];\t/* physical image */
\tgenchar Ubuf[MAXLINE];\t/* used for U command */
\tgenchar ubuf[MAXLINE];\t/* used for u command */
\tgenchar Window[WINDOW+10];\t/* window image */
\tchar cntl_char;\t\t\t/* TRUE if control character present */
\tint Globals[9];\t\t\t/* local global variables */
#ifndef FIORDCHK
\tclock_t oldtime, newtime;
\tstruct tms dummy;
#endif\t/* FIORDCHK */
\t
\t/*** setup prompt ***/

\tPrompt = prompt;
\ted_setup(fd);

#ifndef RAWONLY
\tif( !sh_isoption(SH_VIRAW) )
\t{
\t\t/*** Change the eol characters to \'\\r\' and eof  ***/
\t\t/* in addition to \'\\n\' and make eof an ESC\t*/

\t\tif( tty_alt(ERRIO) == BAD )
\t\t{
\t\t\treturn(read(fd, shbuf, nchar));
\t\t}

#   ifdef FIORDCHK
\t\tioctl(fd,FIORDCHK,&typeahead);
#   else
\t\t/* time the current line to determine typeahead */
\t\toldtime = times(&dummy);
#   endif /* FIORDCHK */
#   ifdef KSHELL
\t\t/* abort of interrupt has occurred */
\t\tif(sh.trapnote&SIGSET)
\t\t\ti = -1;
\t\telse
#   endif /* KSHELL */
\t\t/*** Read the line ***/
\t\ti = read(fd, shbuf, nchar);
#   ifndef FIORDCHK
\t\tnewtime = times(&dummy);
\t\ttypeahead = ((newtime-oldtime) < NTICKS);
#   endif /* FIORDCHK */
\t    if(echoctl)
\t    {
\t\tif( i <= 0 )
\t\t{
\t\t\t/*** read error or eof typed ***/
\t\t\ttty_cooked(ERRIO);
\t\t\treturn(i);
\t\t}
\t\tterm_char = shbuf[--i];
\t\tif( term_char == \'\\r\' )
\t\t\tterm_char = \'\\n\';
\t\tif( term_char==\'\\n\' || term_char==ESC )
\t\t\tshbuf[i--] = \'\\0\';
\t\telse
\t\t\tshbuf[i+1] = \'\\0\';
\t    }
\t    else
\t    {
\t\tc = shbuf[0];

\t\t/*** Save and remove the last character if its an eol, ***/
\t\t/* changing \'\\r\' to \'\\n\' */

\t\tif( i == 0 )
\t\t{
\t\t\t/*** ESC was typed as first char of line ***/
\t\t\tterm_char = ESC;
\t\t\tshbuf[i--] = \'\\0\';\t/* null terminate line */
\t\t}
\t\telse if( i<0 || c==usreof )
\t\t{
\t\t\t/*** read error or eof typed ***/
\t\t\ttty_cooked(ERRIO);
\t\t\tif( c == usreof )
\t\t\t\ti = 0;
\t\t\treturn(i);
\t\t}
\t\telse
\t\t{
\t\t\tterm_char = shbuf[--i];
\t\t\tif( term_char == \'\\r\' )
\t\t\t\tterm_char = \'\\n\';
\t\t\tif( term_char==\'\\n\' || term_char==usreof )
\t\t\t{
\t\t\t\t/*** remove terminator & null terminate ***/
\t\t\t\tshbuf[i--] = \'\\0\';
\t\t\t}
\t\t\telse
\t\t\t{
\t\t\t\t/** terminator was ESC, which is not xmitted **/
\t\t\t\tterm_char = ESC;
\t\t\t\tshbuf[i+1] = \'\\0\';
\t\t\t}
\t\t}
\t    }
\t}
\telse
#endif\t/* RAWONLY */
\t{
\t\t/*** Set raw mode ***/

#ifndef RAWONLY
\t\tif( ttyspeed == 0 )
\t\t{
\t\t\t/*** never did TCGETA, so do it ***/
\t\t\t/* avoids problem if user does \'sh -o viraw\' */
\t\t\ttty_alt(ERRIO);
\t\t}
#endif /* RAWONLY */
\t\tif( tty_raw(ERRIO) == BAD )
\t\t{
\t\t\treturn(read(fd, shbuf, nchar));
\t\t}
\t\ti = INVALID;
\t}

\t/*** Initialize some things ***/

\tvirtual = (genchar*)shbuf;
#undef virtual
#define virtual\t\t((genchar*)shbuf)
#ifdef MULTIBYTE
\tshbuf[i+1] = 0;
\ti = ed_internal(shbuf,virtual)-1;
#endif /* MULTIBYTE */
\tglobals = Globals;
\tcur_phys = i + 1;
\tcur_virt = i;
\tfildes = fd;
\tfirst_virt = 0;
\tfirst_wind = 0;
\tlast_virt = i;
\tlast_phys = i;
\tlast_wind = i;
\tlong_line = \' \';
\tlong_char = \' \';
\to_v_char = \'\\0\';
\tocur_phys = 0;
\tin_raw = 0;
\tocur_virt = MAXCHAR;
\tofirst_wind = 0;
\tphysical = Physical;
\tu_column = INVALID - 1;
\tU_space = Ubuf;
\tu_space = ubuf;
\twindow = Window;
\twindow[0] = \'\\0\';

#if KSHELL && (2*CHARSIZE*MAXLINE)<IOBSIZE
\tyankbuf = shbuf + MAXLINE*sizeof(genchar);
#else
\tif(yankbuf==0)
\t\tyankbuf = (genchar*)malloc(sizeof(genchar)*(MAXLINE));
#endif
#if KSHELL && (3*CHARSIZE*MAXLINE)<IOBSIZE
\tlastline = shbuf + (MAXLINE+MAXLINE)*sizeof(genchar);
#else
\tif(lastline==0)
\t\tlastline = (genchar*)malloc(sizeof(genchar)*(MAXLINE));
#endif
\tif( last_cmd == \'\\0\' )
\t{
\t\t/*** first time for this shell ***/

\t\tlast_cmd = \'i\';
\t\tfindchar = INVALID;
\t\tlastmotion = \'\\0\';
\t\tlastrepeat = 1;
\t\trepeat = 1;
\t\t*yankbuf = 0;
\t}

\t/*** fiddle around with prompt length ***/
\tif( nchar+plen > MAXCHAR )
\t\tnchar = MAXCHAR - plen;
\tmax_col = nchar - 2;

#ifndef RAWONLY
\tif( !sh_isoption(SH_VIRAW) )
\t{
\t\tint kill_erase = 0;
#   ifndef ECHOCTL
\t\tcntl_char = FALSE;
#   endif /* !ECHOCTL */
\t\tfor(i=(echoctl?last_virt:0); i<=last_virt; ++i )
\t\t{
\t\t\t/*** change \\r to 
, check for control characters, ***/
\t\t\t/* delete appropriate ^Vs,\t\t\t*/
\t\t\t/* and estimate last physical column */

\t\t\tif( virtual[i] == \'\\r\' )
\t\t\t\tvirtual[i] = \'\\n\';
\t\t    if(!echoctl)
\t\t    {
\t\t\tc = virtual[i];
\t\t\tif( c==usrerase || c==usrkill )
\t\t\t{
\t\t\t\t/*** user typed escaped erase or kill char ***/
\t\t\t\tcntl_char = TRUE;
\t\t\t\tif(is_print(c))
\t\t\t\t\tkill_erase++;
\t\t\t}
\t\t\telse if( !is_print(c) )
\t\t\t{
\t\t\t\tcntl_char = TRUE;

\t\t\t\tif( c == cntl(\'V\') )
\t\t\t\t{
\t\t\t\t\tif( i == last_virt )
\t\t\t\t\t{
\t\t\t\t\t\t/*** eol/eof was escaped ***/
\t\t\t\t\t\t/* so replace ^V with it */
\t\t\t\t\t\tvirtual[i] = term_char;
\t\t\t\t\t\tbreak;
\t\t\t\t\t}

\t\t\t\t\t/*** delete ^V ***/
\t\t\t\t\tgencpy((&virtual[i]), (&virtual[i+1]));
\t\t\t\t\t--cur_virt;
\t\t\t\t\t--last_virt;
\t\t\t\t}
\t\t\t}
\t\t    }
\t\t}

\t\t/*** copy virtual image to window ***/
\t\tif(last_virt > 0)
\t\t\tlast_phys = ed_virt_to_phys(virtual,physical,last_virt,0,0);
\t\tif( last_phys >= w_size )
\t\t{
\t\t\t/*** line longer than window ***/
\t\t\tlast_wind = w_size - 1;
\t\t}
\t\telse
\t\t\tlast_wind = last_phys;
\t\tgenncpy(window, virtual, last_wind+1);

\t\tif( term_char!=ESC  && (last_virt==INVALID
\t\t\t|| virtual[last_virt]!=term_char) )
\t\t{
\t\t\t/*** Line not terminated with ESC or escaped (^V) ***/
\t\t\t/* eol, so return after doing a total update */
\t\t\t/* if( (speed is greater or equal to 1200 */
\t\t\t/* and something was typed) and */
\t\t\t/* (control character present */
\t\t\t/* or typeahead occurred) ) */

\t\t\ttty_cooked(ERRIO);
\t\t\tif( ttyspeed==FAST && last_virt!=INVALID
# ifdef ECHOCTL
\t\t\t\t&& typeahead)
# else
\t\t\t\t&& (typeahead || cntl_char==TRUE) )
# endif /*ECHOCTL */
\t\t\t{
\t\t\t\trefresh(TRANSLATE);
\t\t\t\tpr_prompt();
\t\t\t\tputstring(0, last_phys+1);
\t\t\t\tif(echoctl)
\t\t\t\t\tcrlf;
\t\t\t\telse
\t\t\t\t\twhile(kill_erase-- > 0)
\t\t\t\t\t\tputchar(\' \');
\t\t\t}

\t\t\tif( term_char==\'\\n\' )
\t\t\t{
\t\t\t\tif(!echoctl)
\t\t\t\t\tcrlf;
\t\t\t\tvirtual[++last_virt] = \'\\n\';
\t\t\t}
\t\t\tlast_cmd = \'i\';
\t\t\tsave_last();
#ifdef MULTIBYTE
\t\t\tvirtual[last_virt+1] = 0;
\t\t\tlast_virt = ed_external(virtual,shbuf);
\t\t\treturn(last_virt);
#else
\t\t\treturn(++last_virt);
#endif /* MULTIBYTE */
\t\t}

\t\t/*** Line terminated with escape, or escaped eol/eof, ***/
\t\t/*  so set raw mode */

\t\tif( tty_raw(ERRIO) == BAD )
\t\t{
\t\t\ttty_cooked(ERRIO);
\t\t\tvirtual[++last_virt] = \'\\n\';
#ifdef MULTIBYTE
\t\t\tvirtual[last_virt+1] = 0;
\t\t\tlast_virt = ed_external(virtual,shbuf);
\t\t\treturn(last_virt);
#else
\t\t\treturn(++last_virt);
#endif /* MULTIBYTE */
\t\t}

\t\tif(echoctl) /*** for cntl-echo erase the ^[ ***/
\t\t\tpr_string("\\b\\b  \\b\\b");


\t\tif( crallowed == YES )
\t\t{
\t\t\t/*** start over since there may be ***/
\t\t\t/*** a control char, or cursor might not ***/
\t\t\t/*** be at left margin (this lets us know ***/
\t\t\t/*** where we are ***/
\t\t\tcur_phys = 0;
\t\t\twindow[0] = \'\\0\';
\t\t\tpr_prompt();
\t\t\tif( term_char==ESC && virtual[last_virt]!=ESC )
\t\t\t\trefresh(CONTROL);
\t\t\telse
\t\t\t\trefresh(INPUT);
\t\t}
\t\telse
\t\t{
\t\t\t/*** just update everything internally ***/
\t\t\trefresh(TRANSLATE);
\t\t}
\t}
\telse
#endif\t/* RAWONLY */
\t\tvirtual[0] = \'\\0\';

\t/*** Handle usrintr, usrquit, or EOF ***/

\ti = SETJMP(env);
\tif( i != 0 )
\t{
\t\tvirtual[0] = \'\\0\';
\t\ttty_cooked(ERRIO);

\t\tswitch(i)
\t\t{
\t\tcase UEOF:
\t\t\t/*** EOF ***/
\t\t\treturn(0);

\t\tcase UINTR:
\t\t\t/** interrupt **/
\t\t\treturn(SYSERR);
\t\t}
\t\treturn(SYSERR);
\t}

\t/*** Get a line from the terminal ***/

\tU_saved = FALSE;

#ifdef RAWONLY
\tgetline(APPEND);
#else
\tif( sh_isoption(SH_VIRAW) || virtual[last_virt]==term_char )
\t\tgetline(APPEND);
\telse
\t\tgetline(ESC);
#endif\t/* RAWONLY */

\t/*** add a new line if user typed unescaped 
 ***/
\t/* to cause the shell to process the line */
\ttty_cooked(ERRIO);
\tif( addnl )
\t{
\t\tvirtual[++last_virt] = \'\\n\';
\t\tcrlf;
\t}
\tif( ++last_virt >= 0 )
\t{
#ifdef MULTIBYTE
\t\tif(bigvi)
\t\t{
\t\t\tbigvi = 0;
\t\t\tshbuf[last_virt-1] = \'\\n\';
\t\t}
\t\telse
\t\t{
\t\t\tvirtual[last_virt] = 0;
\t\t\tlast_virt = ed_external(virtual,shbuf);
\t\t}
#endif /* MULTIBYTE */
\t\treturn(last_virt);
\t}
\telse
\t\treturn(SYSERR);
}


/*{\tAPPEND( char, mode )
 *
 *\tThis routine will append char after cur_virt in the virtual image.
 * mode\t=\tAPPEND, shift chars right before appending
 *\t\tREPLACE, replace char if possible
 *
}*/

#undef virtual
#define virtual\t\teditb.e_inbuf\t/* pointer to virtual image buffer */

static void append __PARAM__((int c, int mode), (c, mode)) __OTORP__(int c; int mode;)
#line 618
{
\tregister int i;

\tif( last_virt<max_col && last_phys<max_col )
\t{
\t\tif( mode==APPEND || cur_virt==last_virt )
\t\t{
\t\t\tfor(i = ++last_virt;  i > cur_virt; --i)
\t\t\t{
\t\t\t\tvirtual[i] = virtual[i-1];
\t\t\t}
\t\t}
\t\tvirtual[++cur_virt] = c;
\t}
\telse
\t\tbell;
\treturn;
}

/*{\tBACKWORD( nwords, cmd )
 *
 *\tThis routine will position cur_virt at the nth previous word.
 *
}*/

static void backword __PARAM__((int nwords, register int cmd), (nwords, cmd)) __OTORP__(int nwords; register int cmd;)
#line 644
{
\tregister int tcur_virt = cur_virt;
\twhile( nwords-- && tcur_virt > first_virt )
\t{
\t\tif( !isblank(tcur_virt) && isblank(tcur_virt-1)
\t\t\t&& tcur_virt>first_virt )
\t\t\t--tcur_virt;
\t\telse if(cmd != \'B\')
\t\t{
\t\t\tregister int last = isalph(tcur_virt-1);
\t\t\tif((!isalph(tcur_virt) && last)
\t\t\t|| (isalph(tcur_virt) && !last))
\t\t\t\t--tcur_virt;
\t\t}
\t\twhile( isblank(tcur_virt) && tcur_virt>=first_virt )
\t\t\t--tcur_virt;
\t\tif( cmd == \'B\' )
\t\t{
\t\t\twhile( !isblank(tcur_virt) && tcur_virt>=first_virt )
\t\t\t\t--tcur_virt;
\t\t}
\t\telse
\t\t{
\t\t\tif( isalph(tcur_virt) )
\t\t\t\twhile( isalph(tcur_virt) && tcur_virt>=first_virt )
\t\t\t\t\t--tcur_virt;
\t\t\telse
\t\t\t\twhile( !isalph(tcur_virt) && !isblank(tcur_virt)
\t\t\t\t\t&& tcur_virt>=first_virt )
\t\t\t\t\t--tcur_virt;
\t\t}
\t\tcur_virt = ++tcur_virt;
\t}
\treturn;
}

/*{\tCNTLMODE()
 *
 *\tThis routine implements the vi command subset.
 *\tThe cursor will always be positioned at the char of interest.
 *
}*/

static int cntlmode __PARAM__((void), ())
#line 688
{
\tregister int c;
\tregister int i;
\tgenchar tmp_u_space[MAXLINE];\t/* temporary u_space */
\tgenchar *real_u_space;\t\t/* points to real u_space */
\tint tmp_u_column = INVALID;\t/* temporary u_column */
\tint was_inmacro;

\tif( U_saved == FALSE )
\t{
\t\t/*** save virtual image if never done before ***/
\t\tvirtual[last_virt+1] = \'\\0\';
\t\tgencpy(U_space, virtual);
\t\tU_saved = TRUE;
\t}

\tsave_last();

\treal_u_space = u_space;
\tcurhline = histmax;
\tfirst_virt = 0;
\trepeat = 1;
\tif( cur_virt > INVALID )
\t{
\t\t/*** make sure cursor is at the last char ***/
\t\tsync_cursor();
\t}

\t/*** Read control char until something happens to cause a ***/
\t/* return to APPEND/REPLACE mode\t*/

\twhile( c=getchar() )
\t{
\t\trepeat_set = 0;
\t\twas_inmacro = inmacro;
\t\tif( c == \'0\' )
\t\t{
\t\t\t/*** move to leftmost column ***/
\t\t\tcur_virt = 0;
\t\t\tsync_cursor();
\t\t\tcontinue;
\t\t}

\t\tif( digit(c) )
\t\t{
\t\t\tlastrepeat = repeat;
\t\t\tc = getcount(c);
\t\t\tif( c == \'.\' )
\t\t\t\tlastrepeat = repeat;
\t\t}

\t\t/*** see if it\'s a move cursor command ***/

\t\tif( mvcursor(c) == GOOD )
\t\t{
\t\t\tsync_cursor();
\t\t\trepeat = 1;
\t\t\tcontinue;
\t\t}

\t\t/*** see if it\'s a repeat of the last command ***/

\t\tif( c == \'.\' )
\t\t{
\t\t\tc = last_cmd;
\t\t\trepeat = lastrepeat;
\t\t\ti = textmod(c, c);
\t\t}
\t\telse
\t\t{
\t\t\ti = textmod(c, 0);
\t\t}

\t\t/*** see if it\'s a text modification command ***/

\t\tswitch(i)
\t\t{
\t\tcase BAD:
\t\t\tbreak;

\t\tdefault:\t\t/** input mode **/
\t\t\tif(!was_inmacro)
\t\t\t{
\t\t\t\tlast_cmd = c;
\t\t\t\tlastrepeat = repeat;
\t\t\t}
\t\t\trepeat = 1;
\t\t\tif( i == GOOD )
\t\t\t\tcontinue;
\t\t\treturn(i);
\t\t}

\t\tswitch( c )
\t\t{
\t\t\t/***** Other stuff *****/

\t\tcase cntl(\'L\'):\t\t/** Redraw line **/
\t\t\t/*** print the prompt and ***/
\t\t\t/* force a total refresh */
\t\t\tif(nonewline==0)
\t\t\t\tputchar(\'\\n\');
\t\t\tnonewline = 0;
\t\t\tpr_prompt();
\t\t\twindow[0] = \'\\0\';
\t\t\tcur_phys = first_wind;
\t\t\tofirst_wind = INVALID;
\t\t\tlong_line = \' \';
\t\t\tbreak;

\t\tcase cntl(\'V\'):
\t\t{
\t\t\tregister const char *p = &e_version[5];
\t\t\tsave_v();
\t\t\tdel_line(BAD);
\t\t\twhile(c = *p++)
\t\t\t\tappend(c,APPEND);
\t\t\trefresh(CONTROL);
\t\t\ted_getchar();
\t\t\trestore_v();
\t\t\tbreak;
\t\t}

\t\tcase \'/\':\t\t/** Search **/
\t\tcase \'?\':
\t\tcase \'N\':
\t\tcase \'n\':
\t\t\tsave_v();
\t\t\tswitch( search(c) )
\t\t\t{
\t\t\tcase GOOD:
\t\t\t\t/*** force a total refresh ***/
\t\t\t\twindow[0] = \'\\0\';
\t\t\t\tgoto newhist;

\t\t\tcase BAD:
\t\t\t\t/*** no match ***/
\t\t\t\t\tbell;

\t\t\tdefault:
\t\t\t\tif( u_column == INVALID )
\t\t\t\t\tdel_line(BAD);
\t\t\t\telse
\t\t\t\t\trestore_v();
\t\t\t\tbreak;
\t\t\t}
\t\t\tbreak;

\t\tcase \'j\':\t\t/** get next command **/
\t\tcase \'+\':\t\t/** get next command **/
\t\t\tcurhline += repeat;
\t\t\tif( curhline > histmax )
\t\t\t{
\t\t\t\tcurhline = histmax;
\t\t\t\tgoto ringbell;
\t\t\t}
\t\t\telse if(curhline==histmax && tmp_u_column!=INVALID )
\t\t\t{
\t\t\t\tu_space = tmp_u_space;
\t\t\t\tu_column = tmp_u_column;
\t\t\t\trestore_v();
\t\t\t\tu_space = real_u_space;
\t\t\t\tbreak;
\t\t\t}
\t\t\tsave_v();
\t\t\tgoto newhist;

\t\tcase \'k\':\t\t/** get previous command **/
\t\tcase \'-\':\t\t/** get previous command **/
\t\t\tif( curhline == histmax )
\t\t\t{
\t\t\t\tu_space = tmp_u_space;
\t\t\t\ti = u_column;
\t\t\t\tsave_v();
\t\t\t\tu_space = real_u_space;
\t\t\t\ttmp_u_column = u_column;
\t\t\t\tu_column = i;
\t\t\t}

\t\t\tcurhline -= repeat;
\t\t\tif( curhline <= histmin )
\t\t\t{
\t\t\t\tcurhline = histmin + 1;
\t\t\t\tgoto ringbell;
\t\t\t}
\t\t\tsave_v();
\tnewhist:
\t\t\thist_copy((char*)virtual, MAXLINE, curhline, -1);
#ifdef MULTIBYTE
\t\t\ted_internal((char*)virtual,virtual);
#endif /* MULTIBYTE */
\t\t\tif( (last_virt = genlen((char*)virtual) - 1) >= 0 )
\t\t\t\tcur_virt = 0;
\t\t\telse
\t\t\t\tcur_virt = INVALID;
\t\t\tbreak;


\t\tcase \'u\':\t\t/** undo the last thing done **/
\t\t\trestore_v();
\t\t\tbreak;

\t\tcase \'U\':\t\t/** Undo everything **/
\t\t\tsave_v();
\t\t\tif( virtual[0] == \'\\0\' )
\t\t\t\tgoto ringbell;
\t\t\telse
\t\t\t{
\t\t\t\tgencpy(virtual, U_space);
\t\t\t\tlast_virt = genlen(U_space) - 1;
\t\t\t\tcur_virt = 0;
\t\t\t}
\t\t\tbreak;

#ifdef KSHELL
\t\tcase \'v\':
\t\t\tif(repeat_set==0)
\t\t\t\tgoto vcommand;
#endif /* KSHELL */

\t\tcase \'G\':\t\t/** goto command repeat **/
\t\t\tif(repeat_set==0)
\t\t\t\trepeat = histmin+1;
\t\t\tif( repeat <= histmin || repeat > histmax )
\t\t\t{
\t\t\t\tgoto ringbell;
\t\t\t}
\t\t\tcurhline = repeat;
\t\t\tsave_v();
\t\t\tif(c == \'G\')
\t\t\t\tgoto newhist;

#ifdef KSHELL
\t\tvcommand:
\t\t\tif(ed_fulledit()==GOOD)
\t\t\t\treturn(BIGVI);
\t\t\telse
\t\t\t\tgoto ringbell;
#endif\t/* KSHELL */

\t\tcase \'#\':\t/** insert(delete) # to (no)comment command **/
\t\t\tif( cur_virt != INVALID )
\t\t\t{
\t\t\t\tregister genchar *p = &virtual[last_virt+1];
\t\t\t\t*p = 0;
\t\t\t\t/*** see whether first char is comment char ***/
\t\t\t\tc = (virtual[0]==\'#\');
\t\t\t\twhile(p-- >= virtual)
\t\t\t\t{
\t\t\t\t\tif(*p==\'\\n\' || *p==\';\' || *p==\'|\' || p<virtual)
\t\t\t\t\t{
\t\t\t\t\t\tif(c) /* delete \'#\' */
\t\t\t\t\t\t{
\t\t\t\t\t\t\tif(p[1]==\'#\')
\t\t\t\t\t\t\t{
\t\t\t\t\t\t\t\tlast_virt--;
\t\t\t\t\t\t\t\tgencpy(p+1,p+2);
\t\t\t\t\t\t\t}
\t\t\t\t\t\t}
\t\t\t\t\t\telse
\t\t\t\t\t\t{
\t\t\t\t\t\t\tcur_virt = p-virtual;
\t\t\t\t\t\t\tappend(\'#\', APPEND);
\t\t\t\t\t\t}
\t\t\t\t\t}
\t\t\t\t}
\t\t\t\tif(c)
\t\t\t\t{
\t\t\t\t\tcur_virt = 0;
\t\t\t\t\tbreak;
\t\t\t\t}
\t\t\t\trefresh(INPUT);
\t\t\t}

\t\tcase \'\\n\':\t\t/** send to shell **/
\t\t\treturn(ENTER);

\t\tdefault:
\t\tringbell:
\t\t\tbell;
\t\t\trepeat = 1;
\t\t\tcontinue;
\t\t}

\t\trefresh(CONTROL);
\t\trepeat = 1;
\t}
/* NOTREACHED */
}

/*{\tCURSOR( new_current_physical )
 *
 *\tThis routine will position the virtual cursor at
 * physical column x in the window.
 *
}*/

static void cursor __PARAM__((register int x), (x)) __OTORP__(register int x;)
#line 985
{
\tregister int delta;

#ifdef MULTIBYTE
\twhile(physical[x]==MARKER)
\t\tx++;
#endif /* MULTIBYTE */
\tdelta = x - cur_phys;

\tif( delta == 0 )
\t\treturn;

\tif( delta > 0 )
\t{
\t\t/*** move to right ***/
\t\tputstring(cur_phys, delta);
\t}
\telse
\t{
\t\t/*** move to left ***/

\t\tdelta = -delta;

\t\t/*** attempt to optimize cursor movement ***/
\t\tif( crallowed==NO
\t\t\t|| (delta <= ((cur_phys-first_wind)+plen)>>1) )
\t\t{
\t\t\twhile( delta-- )
\t\t\t\tputchar(\'\\b\');
\t\t}
\t\telse
\t\t{
\t\t\tpr_prompt();
\t\t\tputstring(first_wind, x - first_wind);
\t\t}
\t}
\tcur_phys = x;
\treturn;
}

/*{\tDELETE( nchars, mode )
 *
 *\tDelete nchars from the virtual space and leave cur_virt positioned
 * at cur_virt-1.
 *
 *\tIf mode\t= \'c\', do not save the characters deleted
 *\t\t= \'d\', save them in yankbuf and delete.
 *\t\t= \'y\', save them in yankbuf but do not delete.
 *
}*/

static void cdelete __PARAM__((register int nchars, int mode), (nchars, mode)) __OTORP__(register int nchars; int mode;)
#line 1037
{
\tregister int i;
\tregister genchar *vp;

\tif( cur_virt < first_virt )
\t{
\t\tbell;
\t\treturn;
\t}
\tif( nchars > 0 )
\t{
\t\tvp = virtual+cur_virt;
\t\to_v_char = vp[0];
\t\tif( (cur_virt-- + nchars) > last_virt )
\t\t{
\t\t\t/*** set nchars to number actually deleted ***/
\t\t\tnchars = last_virt - cur_virt;
\t\t}

\t\t/*** save characters to be deleted ***/

\t\tif( mode != \'c\' )
\t\t{
\t\t\ti = vp[nchars];
\t\t\tvp[nchars] = 0;
\t\t\tgencpy(yankbuf,vp);
\t\t\tvp[nchars] = i;
\t\t}

\t\t/*** now delete these characters ***/

\t\tif( mode != \'y\' )
\t\t{
\t\t\tgencpy(vp,vp+nchars);
\t\t\tlast_virt -= nchars;
\t\t}
\t}
\treturn;
}

/*{\tDEL_LINE( mode )
 *
 *\tThis routine will delete the line.
 *\tmode = GOOD, do a save_v()
 *
}*/
static void del_line __PARAM__((int mode), (mode)) __OTORP__(int mode;)
#line 1084
{
\tif( last_virt == INVALID )
\t\treturn;

\tif( mode == GOOD )
\t\tsave_v();

\tcur_virt = 0;
\tfirst_virt = 0;
\tcdelete(last_virt+1, BAD);
\trefresh(CONTROL);

\tcur_virt = INVALID;
\tcur_phys = 0;
\tfindchar = INVALID;
\tlast_phys = INVALID;
\tlast_virt = INVALID;
\tlast_wind = INVALID;
\tfirst_wind = 0;
\to_v_char = \'\\0\';
\tocur_phys = 0;
\tocur_virt = MAXCHAR;
\tofirst_wind = 0;
\twindow[0] = \'\\0\';
\treturn;
}

/*{\tDELMOTION( motion, mode )
 *
 *\tDelete thru motion.
 *
 *\tmode\t= \'d\', save deleted characters, delete
 *\t\t= \'c\', do not save characters, change
 *\t\t= \'y\', save characters, yank
 *
 *\tReturns GOOD if operation successful; else BAD.
 *
}*/

static int delmotion __PARAM__((int motion, int mode), (motion, mode)) __OTORP__(int motion; int mode;)
#line 1124
{
\tregister int begin;
\tregister int end;
\t/* the following saves a register */
#       define delta end

\tif( cur_virt == INVALID )
\t\treturn(BAD);
\tif( mode != \'y\' )
\t\tsave_v();
\tbegin = cur_virt;

\t/*** fake out the motion routines by appending a blank ***/

\tvirtual[++last_virt] = \' \';
\tend = mvcursor(motion);
\tvirtual[last_virt--] = 0;
\tif(end==BAD)
\t\treturn(BAD);

\tend = cur_virt;
\tif( mode==\'c\' && end>begin && strchr("wW", motion) )
\t{
\t\t/*** called by change operation, user really expects ***/
\t\t/* the effect of the eE commands, so back up to end of word */
\t\twhile( end>begin && isblank(end-1) )
\t\t\t--end;
\t\tif( end == begin )
\t\t\t++end;
\t}

\tdelta = end - begin;
\tif( delta >= 0 )
\t{
\t\tcur_virt = begin;
\t\tif( strchr("eE;,TtFf%", motion) )
\t\t\t++delta;
\t}
\telse
\t{
\t\tdelta = -delta;
\t}

\tcdelete(delta, mode);
\tif( mode == \'y\' )
\t\tcur_virt = begin;
#       undef delta
\treturn(GOOD);
}


/*{\tENDWORD( nwords, cmd )
 *
 *\tThis routine will move cur_virt to the end of the nth word.
 *
}*/

static void endword __PARAM__((int nwords, register int cmd), (nwords, cmd)) __OTORP__(int nwords; register int cmd;)
#line 1182
{
\tregister int tcur_virt = cur_virt;
\twhile( nwords-- )
\t{
\t\tif( !isblank(tcur_virt) && tcur_virt<=last_virt )
\t\t\t++tcur_virt;
\t\twhile( isblank(tcur_virt) && tcur_virt<=last_virt )
\t\t\t++tcur_virt;\t
\t\tif( cmd == \'E\' )
\t\t{
\t\t\twhile( !isblank(tcur_virt) && tcur_virt<=last_virt )
\t\t\t\t++tcur_virt;
\t\t}
\t\telse
\t\t{
\t\t\tif( isalph(tcur_virt) )
\t\t\t\twhile( isalph(tcur_virt) && tcur_virt<=last_virt )
\t\t\t\t\t++tcur_virt;
\t\t\telse
\t\t\t\twhile( !isalph(tcur_virt) && !isblank(tcur_virt)
\t\t\t\t\t&& tcur_virt<=last_virt )
\t\t\t\t\t++tcur_virt;
\t\t}
\t\tif( tcur_virt > first_virt )
\t\t\ttcur_virt--;
\t}
\tcur_virt = tcur_virt;
\treturn;
}

/*{\tFORWARD( nwords, cmd )
 *
 *\tThis routine will move cur_virt forward to the next nth word.
 *
}*/

static void forward __PARAM__((register int nwords, int cmd), (nwords, cmd)) __OTORP__(register int nwords; int cmd;)
#line 1219
{
\tregister int tcur_virt = cur_virt;
\twhile( nwords-- )
\t{
\t\tif( cmd == \'W\' )
\t\t{
\t\t\twhile( !isblank(tcur_virt) && tcur_virt < last_virt )
\t\t\t\t++tcur_virt;
\t\t}
\t\telse
\t\t{
\t\t\tif( isalph(tcur_virt) )
\t\t\t{
\t\t\t\twhile( isalph(tcur_virt) && tcur_virt<last_virt )
\t\t\t\t\t++tcur_virt;
\t\t\t}
\t\t\telse
\t\t\t{
\t\t\t\twhile( !isalph(tcur_virt) && !isblank(tcur_virt)
\t\t\t\t\t&& tcur_virt < last_virt )
\t\t\t\t\t++tcur_virt;
\t\t\t}
\t\t}
\t\twhile( isblank(tcur_virt) && tcur_virt < last_virt )
\t\t\t++tcur_virt;
\t}
\tcur_virt = tcur_virt;
\treturn;
}



/*{\tGETCOUNT(c)
 *
 *\tSet repeat to the user typed number and return the terminating
 * character.
 *
}*/

static int getcount __PARAM__((register int c), (c)) __OTORP__(register int c;)
#line 1259
{
\tregister int i;

\t/*** get any repeat count ***/

\tif( c == \'0\' )
\t\treturn(c);

\trepeat_set++;
\ti = 0;
\twhile( digit(c) )
\t{
\t\ti = i*10 + c - \'0\';
\t\tc = getchar();
\t}

\tif( i > 0 )
\t\trepeat *= i;
\treturn(c);
}


/*{\tGETLINE( mode )
 *
 *\tThis routine will fetch a line.
 *\tmode\t= APPEND, allow escape to cntlmode subroutine
 *\t\t  appending characters.
 *\t\t= REPLACE, allow escape to cntlmode subroutine
 *\t\t  replacing characters.
 *\t\t= SEARCH, no escape allowed
 *\t\t= ESC, enter control mode immediately
 *
 *\tThe cursor will always be positioned after the last
 * char printed.
 *
 *\tThis routine returns when cr, nl, or (eof in column 0) is
 * received (column 0 is the first char position).
 *
}*/

static void getline __PARAM__((register int mode), (mode)) __OTORP__(register int mode;)
#line 1300
{
\tregister int c;
\tregister int tmp;

\taddnl = TRUE;

\tif( mode == ESC )
\t{
\t\t/*** go directly to control mode ***/
\t\tgoto escape;
\t}

\tfor(;;)
\t{
\t\tif( (c = getchar()) == cntl(\'V\') )
\t\t{
\t\t\t/*** implement ^V to escape next char ***/
\t\t\tin_raw++;
\t\t\tc = getchar();
\t\t\tin_raw = 0;
\t\t\tappend(c, mode);
\t\t\trefresh(INPUT);
\t\t\tcontinue;
\t\t}

\t\tif( c == usreof )
\t\t\tc = UEOF;
\t\telse if( c == usrerase )
\t\t\tc = UERASE;
\t\telse if( c == usrkill )
\t\t\tc = UKILL;

\t\tswitch( c )
\t\t{
\t\tcase ESC:\t\t/** enter control mode **/
\t\t\tif( mode == SEARCH )
\t\t\t{
\t\t\t\tbell;
\t\t\t\tcontinue;
\t\t\t}
\t\t\telse
\t\t\t{
\tescape:
\t\t\t\tif( mode == REPLACE )
\t\t\t\t\t--cur_virt;
\t\t\t\ttmp = cntlmode();
\t\t\t\tif( tmp == ENTER || tmp == BIGVI )
\t\t\t\t{
#ifdef MULTIBYTE
\t\t\t\t\tbigvi = (tmp==BIGVI);
#endif /* MULTIBYTE */
\t\t\t\t\treturn;
\t\t\t\t}
\t\t\t\tif( tmp == INSERT )
\t\t\t\t{
\t\t\t\t\tmode = APPEND;
\t\t\t\t\tcontinue;
\t\t\t\t}
\t\t\t\tmode = tmp;
\t\t\t}
\t\t\tbreak;

\t\tcase UERASE:\t\t/** user erase char **/
\t\t\t\t/*** treat as backspace ***/

\t\tcase \'\\b\':\t\t/** backspace **/
\t\t\tif( virtual[cur_virt] == \'\\\\\' )
\t\t\t{
\t\t\t\tcdelete(1, BAD);
\t\t\t\tappend(usrerase, mode);
\t\t\t}
\t\t\telse
\t\t\t{
\t\t\t\tif( mode==SEARCH && cur_virt==0 )
\t\t\t\t{
\t\t\t\t\tfirst_virt = 0;
\t\t\t\t\tcdelete(1, BAD);
\t\t\t\t\treturn;
\t\t\t\t}
\t\t\t\tcdelete(1, BAD);
\t\t\t}
\t\t\tbreak;

\t\tcase cntl(\'W\'):\t\t/** delete back word **/
\t\t\tif( cur_virt > first_virt && isblank(cur_virt-1) )
\t\t\t{
\t\t\t\tcdelete(1, BAD);
\t\t\t}
\t\t\telse
\t\t\t{
\t\t\t\ttmp = cur_virt;
\t\t\t\tbackword(1, \'b\');
\t\t\t\tcdelete(tmp - cur_virt + 1, BAD);
\t\t\t}
\t\t\tbreak;

\t\tcase UKILL:\t\t/** user kill line char **/
\t\t\tif( virtual[cur_virt] == \'\\\\\' )
\t\t\t{
\t\t\t\tcdelete(1, BAD);
\t\t\t\tappend(usrkill, mode);
\t\t\t}
\t\t\telse
\t\t\t{
\t\t\t\tif( mode == SEARCH )
\t\t\t\t{
\t\t\t\t\tcur_virt = 1;
\t\t\t\t\tdelmotion(\'$\', BAD);
\t\t\t\t}
\t\t\t\telse if(first_virt)
\t\t\t\t{
\t\t\t\t\ttmp = cur_virt;
\t\t\t\t\tcur_virt = first_virt;
\t\t\t\t\tcdelete(tmp - cur_virt + 1, BAD);
\t\t\t\t}
\t\t\t\telse
\t\t\t\t\tdel_line(GOOD);
\t\t\t}
\t\t\tbreak;

\t\tcase UEOF:\t\t/** eof char **/
\t\t\tif( cur_virt != INVALID )
\t\t\t\tcontinue;
\t\t\taddnl = FALSE;

\t\tcase \'\\n\':\t\t/** newline or return **/
\t\t\tif( mode != SEARCH )
\t\t\t\tsave_last();
\t\t\treturn;

\t\tdefault:
\t\t\tif( mode == REPLACE )
\t\t\t{
\t\t\t\tif( cur_virt < last_virt )
\t\t\t\t{
\t\t\t\t\treplace(c, TRUE);
\t\t\t\t\tcontinue;
\t\t\t\t}
\t\t\t\tcdelete(1, BAD);
\t\t\t\tmode = APPEND;
\t\t\t}
\t\t\tappend(c, mode);
\t\t\tbreak;
\t\t}
\t\trefresh(INPUT);

\t}
}

/*{\tMVCURSOR( motion )
 *
 *\tThis routine will move the virtual cursor according to motion
 * for repeat times.
 *
 * It returns GOOD if successful; else BAD.
 *
}*/

static int mvcursor __PARAM__((register int motion), (motion)) __OTORP__(register int motion;)
#line 1459
{
\tregister int count;
\tregister int tcur_virt;
\tregister int incr = -1;
\tregister int bound = 0;
\tstatic int last_find = 0;\t/* last find command */

\tswitch(motion)
\t{
\t\t/***** Cursor move commands *****/

\tcase \'0\':\t\t/** First column **/
\t\ttcur_virt = 0;
\t\tbreak;

\tcase \'^\':\t\t/** First nonblank character **/
\t\ttcur_virt = first_virt;
\t\twhile( isblank(tcur_virt) && tcur_virt < last_virt )
\t\t\t++tcur_virt;
\t\tbreak;

\tcase \'|\':
\t\ttcur_virt = repeat-1;
\t\tif(tcur_virt <= last_virt)
\t\t\tbreak;
\t\t/* fall through */

\tcase \'$\':\t\t/** End of line **/
\t\ttcur_virt = last_virt;
\t\tbreak;

\tcase \'h\':\t\t/** Left one **/
\tcase \'\\b\':
\t\tmotion = first_virt;
\t\tgoto walk;

\tcase \' \':
\tcase \'l\':\t\t/** Right one **/
\t\tmotion = last_virt;
\t\tincr = 1;
\twalk:
\t\ttcur_virt = cur_virt;
\t\tif( incr*tcur_virt < motion)
\t\t{
\t\t\ttcur_virt += repeat*incr;
\t\t\tif( incr*tcur_virt > motion)
\t\t\t\ttcur_virt = motion;
\t\t}
\t\telse
\t\t{
\t\t\treturn(BAD);
\t\t}
\t\tbreak;

\tcase \'B\':
\tcase \'b\':\t\t/** back word **/
\t\ttcur_virt = cur_virt;
\t\tbackword(repeat, motion);
\t\tif( cur_virt == tcur_virt )
\t\t\treturn(BAD);
\t\treturn(GOOD);

\tcase \'E\':
\tcase \'e\':\t\t/** end of word **/
\t\ttcur_virt = cur_virt;
\t\tif(tcur_virt >=0)
\t\t\tendword(repeat, motion);
\t\tif( cur_virt == tcur_virt )
\t\t\treturn(BAD);
\t\treturn(GOOD);

\tcase \',\':\t\t/** reverse find old char **/
\tcase \';\':\t\t/** find old char **/
\t\tswitch(last_find)
\t\t{
\t\tcase \'t\':
\t\tcase \'f\':
\t\t\tif(motion==\';\')
\t\t\t{
\t\t\t\tbound = last_virt;
\t\t\t\tincr = 1;
\t\t\t}
\t\t\tgoto find_b;

\t\tcase \'T\':
\t\tcase \'F\':
\t\t\tif(motion==\',\')
\t\t\t{
\t\t\t\tbound = last_virt;
\t\t\t\tincr = 1;
\t\t\t}
\t\t\tgoto find_b;

\t\tdefault:
\t\t\treturn(BAD);
\t\t}


\tcase \'t\':\t\t/** find up to new char forward **/
\tcase \'f\':\t\t/** find new char forward **/
\t\tbound = last_virt;
\t\tincr = 1;

\tcase \'T\':\t\t/** find up to new char backward **/
\tcase \'F\':\t\t/** find new char backward **/
\t\tlast_find = motion;
\t\tif((findchar=getrchar())==ESC)
\t\t\treturn(GOOD);
find_b:
\t\ttcur_virt = cur_virt;
\t\tcount = repeat;
\t\twhile( count-- )
\t\t{
\t\t\twhile( incr*(tcur_virt+=incr) <= bound
\t\t\t\t&& virtual[tcur_virt] != findchar );
\t\t\tif( incr*tcur_virt > bound )
\t\t\t{
\t\t\t\treturn(BAD);
\t\t\t}
\t\t}
\t\tif( fold(last_find) == \'T\' )
\t\t\ttcur_virt -= incr;
\t\tbreak;

\t/* new, undocumented feature */
        case \'%\':
\t{
\t\tint nextmotion;
\t\tint nextc;
\t\ttcur_virt = cur_virt;
\t\twhile( tcur_virt <= last_virt
\t\t\t&& strchr(paren_chars,virtual[tcur_virt])==(char*)0)
\t\t\t\ttcur_virt++;
\t\tif(tcur_virt > last_virt )
\t\t\treturn(BAD);
\t\tnextc = virtual[tcur_virt];
\t\tcount = strchr(paren_chars,nextc)-paren_chars;
\t\tif(count < 3)
\t\t{
\t\t\tincr = 1;
\t\t\tbound = last_virt;
\t\t\tnextmotion = paren_chars[count+3];
\t\t}
\t\telse
\t\t\tnextmotion = paren_chars[count-3];
\t\tcount = 1;
\t\twhile(count >0 &&  incr*(tcur_virt+=incr) <= bound)
\t\t{
\t\t        if(virtual[tcur_virt] == nextmotion)
\t\t        \tcount--;
\t\t        else if(virtual[tcur_virt]==nextc)
\t\t        \tcount++;
\t\t}
\t\tif(count)
\t\t\treturn(BAD);
\t\tbreak;
\t}

\tcase \'W\':
\tcase \'w\':\t\t/** forward word **/
\t\ttcur_virt = cur_virt;
\t\tforward(repeat, motion);
\t\tif( tcur_virt == cur_virt )
\t\t\treturn(BAD);
\t\treturn(GOOD);

\tdefault:
\t\treturn(BAD);
\t}
\tcur_virt = tcur_virt;

\treturn(GOOD);
}

/*{\tPR_PROMPT()
 *
 *\tPrint the prompt.
 *
}*/

static void pr_prompt __PARAM__((void), ())
#line 1640
{
\tpr_string(Prompt);
\treturn;
}

/*
 * print a string
 */

static void pr_string __PARAM__((register const char *sp), (sp)) __OTORP__(register const char *sp;)
#line 1650
{
\t/*** copy string sp ***/
\tregister char *ptr = editb.e_outptr;
\twhile(*sp)
\t\t*ptr++ = *sp++;
\teditb.e_outptr = ptr;
\treturn;
}

/*{\tPUTSTRING( column, nchars )
 *
 *\tPut nchars starting at column of physical into the workspace
 * to be printed.
 *
}*/

static void putstring __PARAM__((register int col, register int nchars), (col, nchars)) __OTORP__(register int col; register int nchars;)
#line 1667
{
\twhile( nchars-- )
\t\tputchar(physical[col++]);
\treturn;
}

/*{\tREFRESH( mode )
 *
 *\tThis routine will refresh the crt so the physical image matches
 * the virtual image and display the proper window.
 *
 *\tmode\t= CONTROL, refresh in control mode, ie. leave cursor
 *\t\t\tpositioned at last char printed.
 *\t\t= INPUT, refresh in input mode; leave cursor positioned
 *\t\t\tafter last char printed.
 *\t\t= TRANSLATE, perform virtual to physical translation
 *\t\t\tand adjust left margin only.
 *
 *\t\t+-------------------------------+
 *\t\t|   | |    virtual\t  | |   |
 *\t\t+-------------------------------+
 *\t\t  cur_virt\t\tlast_virt
 *
 *\t\t+-----------------------------------------------+
 *\t\t|\t  | |\t        physical\t | |    |
 *\t\t+-----------------------------------------------+
 *\t\t\tcur_phys\t\t\tlast_phys
 *
 *\t\t\t\t0\t\t\tw_size - 1
 *\t\t\t\t+-----------------------+
 *\t\t\t\t| | |  window\t\t|
 *\t\t\t\t+-----------------------+
 *\t\t\t\tcur_window = cur_phys - first_wind
}*/

static void refresh __PARAM__((int mode), (mode)) __OTORP__(int mode;)
#line 1703
{
\tregister int p;
\tregister int regb;
\tregister int first_w = first_wind;
\tint p_differ;
\tint new_lw;
\tint ncur_phys;
\tint opflag;\t\t\t/* search optimize flag */

#\tdefine\tw\tregb
#\tdefine\tv\tregb

\t/*** find out if it\'s necessary to start translating at beginning ***/

\tif(lookahead>0)
\t{
\t\tp = previous[lookahead-1];
\t\tif(p != ESC && p != \'\\n\' && p != \'\\r\')
\t\t\tmode = TRANSLATE;
\t}
\tv = cur_virt;
\tif( v<ocur_virt || ocur_virt==INVALID
\t\t|| ( v==ocur_virt
\t\t\t&& (!is_print(virtual[v]) || !is_print(o_v_char))) )
\t{
\t\topflag = FALSE;
\t\tp = 0;
\t\tv = 0;
\t}
\telse
\t{
\t\topflag = TRUE;
\t\tp = ocur_phys;
\t\tv = ocur_virt;
\t\tif( !is_print(virtual[v]) )
\t\t{
\t\t\t/*** avoid double ^\'s ***/
\t\t\t++p;
\t\t\t++v;
\t\t}
\t}
\tvirtual[last_virt+1] = 0;
\tncur_phys = ed_virt_to_phys(virtual,physical,cur_virt,v,p);
\tp = genlen(physical);
\tif( --p < 0 )
\t\tlast_phys = 0;
\telse
\t\tlast_phys = p;

\t/*** see if this was a translate only ***/

\tif( mode == TRANSLATE )
\t\treturn;

\t/*** adjust left margin if necessary ***/

\tif( ncur_phys<first_w || ncur_phys>=(first_w + w_size) )
\t{
\t\tcursor(first_w);
\t\tfirst_w = ncur_phys - (w_size>>1);
\t\tif( first_w < 0 )
\t\t\tfirst_w = 0;
\t\tfirst_wind = cur_phys = first_w;
\t}

\t/*** attempt to optimize search somewhat to find ***/
\t/*** out where physical and window images differ ***/

\tif( first_w==ofirst_wind && ncur_phys>=ocur_phys && opflag==TRUE )
\t{
\t\tp = ocur_phys;
\t\tw = p - first_w;
\t}
\telse
\t{
\t\tp = first_w;
\t\tw = 0;
\t}

\tfor(; (p<=last_phys && w<=last_wind); ++p, ++w)
\t{
\t\tif( window[w] != physical[p] )
\t\t\tbreak;
\t}
\tp_differ = p;

\tif( (p>last_phys || p>=first_w+w_size) && w>last_wind
\t\t&& cur_virt==ocur_virt )
\t{
\t\t/*** images are identical ***/
\t\treturn;
\t}

\t/*** copy the physical image to the window image ***/

\tif( last_virt != INVALID )
\t{
\t\twhile( p <= last_phys && w < w_size )
\t\t\twindow[w++] = physical[p++];
\t}
\tnew_lw = w;

\t/*** erase trailing characters if needed ***/

\twhile( w <= last_wind )
\t\twindow[w++] = \' \';
\tlast_wind = --w;

\tp = p_differ;

\t/*** move cursor to start of difference ***/

\tcursor(p);

\t/*** and output difference ***/

\tw = p - first_w;
\twhile( w <= last_wind )
\t\tputchar(window[w++]);

\tcur_phys = w + first_w;
\tlast_wind = --new_lw;

\tif( last_phys >= w_size )
\t{
\t\tif( first_w == 0 )
\t\t\tlong_char = \'>\';
\t\telse if( last_phys < (first_w+w_size) )
\t\t\tlong_char = \'<\';
\t\telse
\t\t\tlong_char = \'*\';
\t}
\telse
\t\tlong_char = \' \';

\tif( long_line != long_char )
\t{
\t\t/*** indicate lines longer than window ***/
\t\twhile( w++ < w_size )
\t\t{
\t\t\tputchar(\' \');
\t\t\t++cur_phys;
\t\t}
\t\tputchar(long_char);
\t\t++cur_phys;
\t\tlong_line = long_char;
\t}

\tocur_phys = ncur_phys;
\tocur_virt = cur_virt;
\tofirst_wind = first_w;

\tif( mode==INPUT && cur_virt>INVALID )
\t\t++ncur_phys;

\tcursor(ncur_phys);
\ted_flush();
\treturn;
}

/*{\tREPLACE( char, increment )
 *
 *\tReplace the cur_virt character with char.  This routine attempts
 * to avoid using refresh().
 *
 *\tincrement\t= TRUE, increment cur_virt after replacement.
 *\t\t\t= FALSE, leave cur_virt where it is.
 *
}*/

static void replace __PARAM__((register int c, register int increment), (c, increment)) __OTORP__(register int c; register int increment;)
#line 1874
{
\tregister int cur_window;

\tif( cur_virt == INVALID )
\t{
\t\t/*** can\'t replace invalid cursor ***/
\t\tbell;
\t\treturn;
\t}
\tcur_window = cur_phys - first_wind;
\tif( ocur_virt == INVALID || !is_print(c)
\t\t|| !is_print(virtual[cur_virt])
\t\t|| !is_print(o_v_char)
#ifdef MULTIBYTE
\t\t|| icharset(c) || out_csize(icharset(o_v_char))>1
#endif /* MULTIBYTE */
\t\t|| (increment==TRUE && (cur_window==w_size-1)
\t\t\t|| !is_print(virtual[cur_virt+1])) )
\t{
\t\t/*** must use standard refresh routine ***/

\t\tcdelete(1, BAD);
\t\tappend(c, APPEND);
\t\tif( increment==TRUE && cur_virt<last_virt )
\t\t\t++cur_virt;
\t\trefresh(CONTROL);
\t}
\telse
\t{
\t\tvirtual[cur_virt] = c;
\t\tphysical[cur_phys] = c;
\t\twindow[cur_window] = c;
\t\tputchar(c);
\t\tif( increment == TRUE )
\t\t{
\t\t\tc = virtual[++cur_virt];
\t\t\t++cur_phys;
\t\t}
\t\telse
\t\t{
\t\t\tputchar(\'\\b\');
\t\t}
\t\to_v_char = c;
\t\ted_flush();
\t}
\treturn;
}

/*
#ifdef xxx
#endif
*/
/*{\tRESTORE_V()
 *
 *\tRestore the contents of virtual space from u_space.
 *
}*/

static void restore_v __PARAM__((void), ())
#line 1933
{
\tregister int tmpcol;
\tgenchar tmpspace[MAXLINE];

\tif( u_column == INVALID-1 )
\t{
\t\t/*** never saved anything ***/
\t\tbell;
\t\treturn;
\t}
\tgencpy(tmpspace, u_space);
\ttmpcol = u_column;
\tsave_v();
\tgencpy(virtual, tmpspace);
\tcur_virt = tmpcol;
\tlast_virt = genlen(tmpspace) - 1;
\tocur_virt = MAXCHAR;\t/** invalidate refresh optimization **/
\treturn;
}

/*{\tSAVE_LAST()
 *
 *\tIf the user has typed something, save it in last line.
 *
}*/

static void save_last __PARAM__((void), ())
#line 1960
{
\tregister int i;

\tif( (i = cur_virt - first_virt + 1) > 0 )
\t{
\t\t/*** save last thing user typed ***/
\t\tgenncpy(lastline, (&virtual[first_virt]), i);
\t\tlastline[i] = \'\\0\';
\t}
\treturn;
}

/*{\tSAVE_V()
 *
 *\tThis routine will save the contents of virtual in u_space.
 *
}*/

static void save_v __PARAM__((void), ())
#line 1979
{
\tif(!inmacro)
\t{
\t\tvirtual[last_virt + 1] = \'\\0\';
\t\tgencpy(u_space, virtual);
\t\tu_column = cur_virt;
\t}
\treturn;
}

/*{\tSEARCH( mode )
 *
 *\tSearch history file for regular expression.
 *
 *\tmode\t= \'/\'\trequire search string and search new to old
 *\tmode\t= \'?\'\trequire search string and search old to new
 *\tmode\t= \'N\'\trepeat last search in reverse direction
 *\tmode\t= \'n\'\trepeat last search
 *
}*/

static int search __PARAM__((register int mode), (mode)) __OTORP__(register int mode;)
#line 2001
{
\tregister int new_direction;
\tregister int oldcurhline;
\tstatic int direction = -1;
\thistloc_t  location;

\tif( mode == \'/\' || mode == \'?\')
\t{
\t\t/*** new search expression ***/
\t\tdel_line(BAD);
\t\tappend(mode, APPEND);
\t\trefresh(INPUT);
\t\tfirst_virt = 1;
\t\tgetline(SEARCH);
\t\tfirst_virt = 0;
\t\tvirtual[last_virt + 1] = \'\\0\';\t/*** make null terminated ***/
\t\tdirection = mode==\'/\' ? -1 : 1;
\t}

\tif( cur_virt == INVALID )
\t{
\t\t/*** no operation ***/
\t\treturn(ABORT);
\t}

\tif( cur_virt==0 ||  fold(mode)==\'N\' )
\t{
\t\t/*** user wants repeat of last search ***/
\t\tdel_line(BAD);
\t\tstrcpy( ((char*)virtual)+1, lsearch);
#ifdef MULTIBYTE
\t\t*((char*)virtual) = \'/\';
\t\ted_internal((char*)virtual,virtual);
#endif /* MULTIBYTE */
\t}

\tif( mode == \'N\' )
\t\tnew_direction = -direction;
\telse
\t\tnew_direction = direction;

\tif( new_direction==1 && curhline >= histmax )
\t\tcurhline = histmin + 1;

\t/*** now search ***/

\toldcurhline = curhline;
#ifdef MULTIBYTE
\ted_external(virtual,(char*)virtual);
#endif /* MULTIBYTE */
\tlocation = hist_find(((char*)virtual)+1, curhline, 1, new_direction);
\tstrncpy(lsearch, ((char*)virtual)+1, SEARCHSIZE);
\tif( (curhline=location.hist_command) >=0 )
\t{
\t\treturn(GOOD);
\t}

\t/*** could not find matching line ***/

\tcurhline = oldcurhline;
\treturn(BAD);
}

/*{\tSYNC_CURSOR()
 *
 *\tThis routine will move the physical cursor to the same
 * column as the virtual cursor.
 *
}*/

static void sync_cursor __PARAM__((void), ())
#line 2072
{
\tregister int p;
\tregister int v;
\tregister int c;
\tint new_phys;

\tif( cur_virt == INVALID )
\t\treturn;

\t/*** find physical col that corresponds to virtual col ***/

\tnew_phys = 0;
\tif(first_wind==ofirst_wind && cur_virt>ocur_virt && ocur_virt!=INVALID)
\t{
\t\t/*** try to optimize search a little ***/
\t\tp = ocur_phys + 1;
#ifdef MULTIBYTE
\t\twhile(physical[p]==MARKER)
\t\t\tp++;
#endif /* MULTIBYTE */
\t\tv = ocur_virt + 1;
\t}
\telse
\t{
\t\tp = 0;
\t\tv = 0;
\t}
\tfor(; v <= last_virt; ++p, ++v)
\t{
#ifdef MULTIBYTE
\t\tint d;
\t\tc = virtual[v];
\t\tif(d = icharset(c))
\t\t{
\t\t\tif( v != cur_virt )
\t\t\t\tp += (out_csize(d)-1);
\t\t}
\t\telse
#else
\t\tc = virtual[v];
#endif\t/* MULTIBYTE */
\t\tif( !isprint(c) )
\t\t{
\t\t\tif( c == \'\\t\' )
\t\t\t{
\t\t\t\tp -= ((p+editb.e_plen)%TABSIZE);
\t\t\t\tp += (TABSIZE-1);
\t\t\t}
\t\t\telse
\t\t\t{
\t\t\t\t++p;
\t\t\t}
\t\t}
\t\tif( v == cur_virt )
\t\t{
\t\t\tnew_phys = p;
\t\t\tbreak;
\t\t}
\t}

\tif( new_phys < first_wind || new_phys >= first_wind + w_size )
\t{
\t\t/*** asked to move outside of window ***/

\t\twindow[0] = \'\\0\';
\t\trefresh(CONTROL);
\t\treturn;
\t}

\tcursor(new_phys);
\ted_flush();
\tocur_phys = cur_phys;
\tocur_virt = cur_virt;
\to_v_char = virtual[ocur_virt];

\treturn;
}

/*{\tTEXTMOD( command, mode )
 *
 *\tModify text operations.
 *
 *\tmode != 0, repeat previous operation
 *
}*/

static int textmod __PARAM__((register int c, int mode), (c, mode)) __OTORP__(register int c; int mode;)
#line 2159
{
\tregister int i;
\tregister genchar *p = lastline;
\tregister int trepeat = repeat;
\tstatic int lastmacro;
\tgenchar *savep;

\tif(mode && (fold(lastmotion)==\'F\' || fold(lastmotion)==\'T\')) 
\t\tlastmotion = \';\';

\tif( fold(c) == \'P\' )
\t{
\t\t/*** change p from lastline to yankbuf ***/
\t\tp = yankbuf;
\t}

addin:
\tswitch( c )
\t{
\t\t\t/***** Input commands *****/

#ifdef KSHELL
\tcase \'*\':\t\t/** do file name expansion in place **/
\tcase \'\\\\\':\t\t/** do file name completion in place **/
\t\tif( cur_virt == INVALID )
\t\t\treturn(BAD);
\tcase \'=\':\t\t/** list file name expansions **/
\t\tsave_v();
\t\ti = last_virt;
\t\t++last_virt;
\t\tvirtual[last_virt] = 0;
\t\tif( ed_expand((char*)virtual, &cur_virt, &last_virt, c) )
\t\t{
\t\t\tlast_virt = i;
\t\t\tbell;
\t\t}
\t\telse if(c == \'=\')
\t\t{
\t\t\tlast_virt = i;
\t\t\tnonewline++;
\t\t\ted_ungetchar(cntl(\'L\'));
\t\t\treturn(GOOD);
\t\t}
\t\telse
\t\t{
\t\t\t--cur_virt;
\t\t\t--last_virt;
\t\t\tocur_virt = MAXCHAR;
\t\t\treturn(APPEND);
\t\t}
\t\tbreak;

\tcase \'@\':\t\t/** macro expansion **/
\t\tif( mode )
\t\t\tc = lastmacro;
\t\telse
\t\t\tif((c=getrchar())==ESC)
\t\t\t\treturn(GOOD);
\t\tif(!inmacro)
\t\t\tlastmacro = c;
\t\tif(ed_macro(c))
\t\t{
\t\t\tsave_v();
\t\t\tinmacro++;
\t\t\treturn(GOOD);
\t\t}
\t\tbell;
\t\treturn(BAD);

#endif\t/* KSHELL */
\tcase \'_\':\t\t/** append last argument of prev command **/
\t\tsave_v();
\t\t{
\t\t\tgenchar tmpbuf[MAXLINE];
\t\t\tif(repeat_set==0)
\t\t\t\trepeat = -1;
\t\t\tp = (genchar*)hist_word(tmpbuf,MAXLINE,repeat);
#ifndef KSHELL
\t\t\tif(p==0)
\t\t\t{
\t\t\t\tbell;
\t\t\t\tbreak;
\t\t\t}
#endif\t/* KSHELL */
#ifdef MULTIBYTE
\t\t\ted_internal((char*)p,tmpbuf);
\t\t\tp = tmpbuf;
#endif /* MULTIBYTE */
\t\t\ti = \' \';
\t\t\tdo
\t\t\t{
\t\t\t\tappend(i,APPEND);
\t\t\t}
\t\t\twhile(i = *p++);
\t\t\treturn(APPEND);
\t\t}

\tcase \'A\':\t\t/** append to end of line **/
\t\tcur_virt = last_virt;
\t\tsync_cursor();

\tcase \'a\':\t\t/** append **/
\t\tif( fold(mode) == \'A\' )
\t\t{
\t\t\tc = \'p\';
\t\t\tgoto addin;
\t\t}
\t\tsave_v();
\t\tif( cur_virt != INVALID )
\t\t{
\t\t\tfirst_virt = cur_virt + 1;
\t\t\tcursor(cur_phys + 1);
\t\t\ted_flush();
\t\t}
\t\treturn(APPEND);

\tcase \'I\':\t\t/** insert at beginning of line **/
\t\tcur_virt = first_virt;
\t\tsync_cursor();

\tcase \'i\':\t\t/** insert **/
\t\tif( fold(mode) == \'I\' )
\t\t{
\t\t\tc = \'P\';
\t\t\tgoto addin;
\t\t}
\t\tsave_v();
\t\tif( cur_virt != INVALID )
 \t\t{
 \t\t\to_v_char = virtual[cur_virt];
\t\t\tfirst_virt = cur_virt--;
  \t\t}
\t\treturn(INSERT);

\tcase \'C\':\t\t/** change to eol **/
\t\tc = \'$\';
\t\tgoto chgeol;

\tcase \'c\':\t\t/** change **/
\t\tif( mode )
\t\t\tc = lastmotion;
\t\telse
\t\t\tc = getcount(getchar());
chgeol:
\t\tlastmotion = c;
\t\tif( c == \'c\' )
\t\t{
\t\t\tdel_line(GOOD);
\t\t\treturn(APPEND);
\t\t}

\t\tif( delmotion(c, \'c\') == BAD )
\t\t\treturn(BAD);

\t\tif( mode == \'c\' )
\t\t{
\t\t\tc = \'p\';
\t\t\ttrepeat = 1;
\t\t\tgoto addin;
\t\t}
\t\tfirst_virt = cur_virt + 1;
\t\treturn(APPEND);

\tcase \'D\':\t\t/** delete to eol **/
\t\tc = \'$\';
\t\tgoto deleol;

\tcase \'d\':\t\t/** delete **/
\t\tif( mode )
\t\t\tc = lastmotion;
\t\telse
\t\t\tc = getcount(getchar());
deleol:
\t\tlastmotion = c;
\t\tif( c == \'d\' )
\t\t{
\t\t\tdel_line(GOOD);
\t\t\tbreak;
\t\t}
\t\tif( delmotion(c, \'d\') == BAD )
\t\t\treturn(BAD);
\t\tif( cur_virt < last_virt )
\t\t\t++cur_virt;
\t\tbreak;

\tcase \'P\':
\t\tif( p[0] == \'\\0\' )
\t\t\treturn(BAD);
\t\tif( cur_virt != INVALID )
\t\t{
\t\t\ti = virtual[cur_virt];
\t\t\tif(!is_print(i))
\t\t\t\tocur_virt = INVALID;
\t\t\t--cur_virt;
\t\t}

\tcase \'p\':\t\t/** print **/
\t\tif( p[0] == \'\\0\' )
\t\t\treturn(BAD);

\t\tif( mode != \'s\' && mode != \'c\' )
\t\t{
\t\t\tsave_v();
\t\t\tif( c == \'P\' )
\t\t\t{
\t\t\t\t/*** fix stored cur_virt ***/
\t\t\t\t++u_column;
\t\t\t}
\t\t}
\t\tif( mode == \'R\' )
\t\t\tmode = REPLACE;
\t\telse
\t\t\tmode = APPEND;
\t\tsavep = p;
\t\tfor(i=0; i<trepeat; ++i)
\t\t{
\t\t\twhile(c= *p++)
\t\t\t\tappend(c,mode);
\t\t\tp = savep;
\t\t}
\t\tbreak;

\tcase \'R\':\t\t/* Replace many chars **/
\t\tif( mode == \'R\' )
\t\t{
\t\t\tc = \'P\';
\t\t\tgoto addin;
\t\t}
\t\tsave_v();
\t\tif( cur_virt != INVALID )
\t\t\tfirst_virt = cur_virt;
\t\treturn(REPLACE);

\tcase \'r\':\t\t/** replace **/
\t\tif( mode )
\t\t\tc = *p;
\t\telse
\t\t\tif((c=getrchar())==ESC)
\t\t\t\treturn(GOOD);
\t\t*p = c;
\t\tsave_v();
\t\twhile(trepeat--)
\t\t\treplace(c, trepeat!=0);
\t\treturn(GOOD);

\tcase \'S\':\t\t/** Substitute line - cc **/
\t\tc = \'c\';
\t\tgoto chgeol;

\tcase \'s\':\t\t/** substitute **/
\t\tsave_v();
\t\tcdelete(repeat, BAD);
\t\tif( mode )
\t\t{
\t\t\tc = \'p\';
\t\t\ttrepeat = 1;
\t\t\tgoto addin;
\t\t}
\t\tfirst_virt = cur_virt + 1;
\t\treturn(APPEND);

\tcase \'Y\':\t\t/** Yank to end of line **/
\t\tc = \'$\';
\t\tgoto yankeol;

\tcase \'y\':\t\t/** yank thru motion **/
\t\tif( mode )
\t\t\tc = lastmotion;
\t\telse
\t\t\tc = getcount(getchar());
yankeol:
\t\tlastmotion = c;
\t\tif( c == \'y\' )
\t\t{
\t\t\tgencpy(yankbuf, virtual);
\t\t}
\t\telse if( delmotion(c, \'y\') == BAD )
\t\t{
\t\t\treturn(BAD);
\t\t}
\t\tbreak;

\tcase \'x\':\t\t/** delete repeat chars forward - dl **/
\t\tc = \'l\';
\t\tgoto deleol;

\tcase \'X\':\t\t/** delete repeat chars backward - dh **/
\t\tc = \'h\';
\t\tgoto deleol;

\tcase \'~\':\t\t/** invert case and advance **/
\t\tif( cur_virt != INVALID )
\t\t{
\t\t\tsave_v();
\t\t\ti = INVALID;
\t\t\twhile(trepeat-->0 && i!=cur_virt)
\t\t\t{
\t\t\t\ti = cur_virt;
\t\t\t\tc = virtual[cur_virt];
#ifdef MULTIBYTE
\t\t\t\tif((c&~STRIP)==0)
#endif /* MULTIBYTE */
\t\t\t\tif( isupper(c) )
\t\t\t\t\tc = tolower(c);
\t\t\t\telse if( islower(c) )
\t\t\t\t\tc = toupper(c);
\t\t\t\treplace(c, TRUE);
\t\t\t}
\t\t\treturn(GOOD);
\t\t}
\t\telse
\t\t\treturn(BAD);

\tdefault:
\t\treturn(BAD);
\t}
\trefresh(CONTROL);
\treturn(GOOD);
}

#ifdef INT16

/* making these functions reduces the size of the text region */

static int isalph __PARAM__((register int c), (c)) __OTORP__(register int c;)
#line 2484
{
\tregister int v = virtual[c];
\treturn(isalnum(v));
}

static int isblank __PARAM__((register int c), (c)) __OTORP__(register int c;)
#line 2490
{
\tregister int v = virtual[c];
\treturn(isspace(v));
}

static int ismetach __PARAM__((register int c), (c)) __OTORP__(register int c;)
#line 2496
{
\tregister int v = virtual[c];
\treturn(ismeta(v));
}

#endif\t/* INT16 */


#ifdef MULTIBYTE
int isalph(c)
register int c;
{
\tregister int v = virtual[c];
\treturn((v&~STRIP) || isalnum(v));
}


int isblank(c)
register int c;
{
\tregister int v = virtual[c];
\treturn((v&~STRIP)==0 && isspace(v));
}

int ismetach(c)
register int c;
{
\tregister int v = virtual[c];
\treturn((v&~STRIP)==0 && ismeta(v));
}

#endif\t/* MULTIBYTE */

/*
 * get a character, after ^V processing
 */
static int getrchar()
{
\tregister int c;
\tif((c=getchar())== cntl(\'V\'))
\t{
\t\tin_raw++;
\t\tc = getchar();
\t\tin_raw = 0;
\t}
\treturn(c);
}'
	EXEC -nh
		INPUT - $'#pragma prototyped

/*
 * posix regex executor
 * single sized-record interface
 */

/*nclude "reglib.*/

#ifdef _AST_REGEX_DEBUG
#define DEBUG_TEST(f,y,n)	((debug&(f))?(y):(n))
#define DEBUG_CODE(f,y,n)	do if(debug&(f)){y}else{n} while(0)
#define DEBUG_INIT()		do { char* t; if (!debug) { debug = 0x80000000; if (t = getenv("_AST_regex_debug")) debug |= strtoul(t, NiL, 0); } } while (0)
static unsigned long	debug;
#else
#define DEBUG_INIT()
#define DEBUG_TEST(f,y,n)	(n)
#define DEBUG_CODE(f,y,n)	do {n} while(0)
#endif

#define BEG_ALT		1	/* beginning of an alt			*/
#define BEG_ONE		2	/* beginning of one iteration of a rep	*/
#define BEG_REP		3	/* beginning of a repetition		*/
#define BEG_SUB		4	/* beginning of a subexpression		*/
#define END_ANY		5	/* end of any of above			*/

/*
 * returns from parse()
 */

#define NONE		0	/* no parse found			*/
#define GOOD		1	/* some parse was found			*/
#define BEST		2	/* an unbeatable parse was found	*/
#define BAD		3	/* error ocurred			*/

/*
 * REG_SHELL_DOT test
 */

#define LEADING(e,r,s)	((e)->leading&&*(s)==\'.\'&&((s)==(e)->beg||*((s)-1)==(r)->explicit))

/*
 * Pos_t is for comparing parses. An entry is made in the
 * array at the beginning and at the end of each Group_t,
 * each iteration in a Group_t, and each Binary_t.
 */

typedef struct
{
	unsigned char*	p;		/* where in string		*/
	size_t		length;		/* length in string		*/
	short		serial;		/* preorder subpattern number	*/
	short		be;		/* which end of pair		*/
} Pos_t;

/* ===== begin library support ===== */

#define vector(t,v,i)	(((i)<(v)->max)?(t*)((v)->vec+(i)*(v)->siz):(t*)vecseek(&(v),i))

static Vector_t*
vecopen(int inc, int siz)
{
	Vector_t*	v;
	Stk_t*		sp;

	if (inc <= 0)
		inc = 16;
	if (!(sp = stkopen(STK_SMALL|STK_NULL)))
		return 0;
	if (!(v = (Vector_t*)stkseek(sp, sizeof(Vector_t) + inc * siz)))
	{
		stkclose(sp);
		return 0;
	}
	v->stk = sp;
	v->vec = (char*)v + sizeof(Vector_t);
	v->max = v->inc = inc;
	v->siz = siz;
	v->cur = 0;
	return v;
}

static void*
vecseek(Vector_t** p, int index)
{
	Vector_t*	v = *p;

	if (index >= v->max)
	{
		while ((v->max += v->inc) <= index);
		if (!(v = (Vector_t*)stkseek(v->stk, sizeof(Vector_t) + v->max * v->siz)))
			return 0;
		*p = v;
		v->vec = (char*)v + sizeof(Vector_t);
	}
	return v->vec + index * v->siz;
}

static void
vecclose(Vector_t* v)
{
	if (v)
		stkclose(v->stk);
}

typedef struct
{
	Stk_pos_t	pos;
	char		data[1];
} Stk_frame_t;

#define stknew(s,p)	((p)->offset=stktell(s),(p)->base=stkfreeze(s,0))
#define stkold(s,p)	stkset(s,(p)->base,(p)->offset)

#define stkframe(s)	(*((Stk_frame_t**)(s)->_next-1))
#define stkdata(s,t)	((t*)stkframe(s)->data)
#define stkpop(s)	stkold(s,&(stkframe(s)->pos))

static void*
stkpush(Stk_t* sp, size_t size)
{
	Stk_frame_t*	f;
	Stk_pos_t	p;

	stknew(sp, &p);
	size = sizeof(Stk_frame_t) + sizeof(size_t) + size - 1;
	if (!(f = (Stk_frame_t*)stkalloc(sp, sizeof(Stk_frame_t) + sizeof(Stk_frame_t*) + size - 1)))
		return 0;
	f->pos = p;
	stkframe(sp) = f;
	return f->data;
}

/* ===== end library support ===== */

/*
 * Match_frame_t is for saving and restoring match records
 * around alternate attempts, so that fossils will not be
 * left in the match array.  These are the only entries in
 * the match array that are not otherwise guaranteed to
 * have current data in them when they get used.
 */

typedef struct
{
	size_t			size;
	regmatch_t*		match;
	regmatch_t		save[1];
} Match_frame_t;

#define matchframe	stkdata(stkstd,Match_frame_t)
#define matchpush(e,x)	((x)->re.group.number?_matchpush(e,x):0)
#define matchcopy(e,x)	((x)->re.group.number?memcpy(matchframe->match,matchframe->save,matchframe->size):(void*)0)
#define matchpop(e,x)	((x)->re.group.number?memcpy(matchframe->match,matchframe->save,matchframe->size),stkpop(stkstd):(void*)0)

#define pospop(e)	(--(e)->pos->cur)

/*
 * allocate a frame and push a match onto the stack
 */

static int
_matchpush(Env_t* env, Rex_t* rex)
{
	Match_frame_t*	f;
	regmatch_t*	m;
	regmatch_t*	e;
	regmatch_t*	s;
	int		num;

	if (rex->re.group.number <= 0 || (num = rex->re.group.last - rex->re.group.number + 1) <= 0)
		num = 0;
	if (!(f = (Match_frame_t*)stkpush(stkstd, sizeof(Match_frame_t) + (num - 1) * sizeof(regmatch_t))))
	{
		env->error = REG_ESPACE;
		return 1;
	}
	f->size = num * sizeof(regmatch_t);
	f->match = m = env->match + rex->re.group.number;
	e = m + num;
	s = f->save;
	while (m < e)
	{
		*s++ = *m;
		*m++ = state.nomatch;
	}
	return 0;
}

/*
 * allocate a frame and push a pos onto the stack
 */

static int
pospush(Env_t* env, Rex_t* rex, unsigned char* p, int be)
{
	Pos_t*	pos;

	if (!(pos = vector(Pos_t, env->pos, env->pos->cur)))
	{
		env->error = REG_ESPACE;
		return 1;
	}
	pos->serial = rex->serial;
	pos->p = p;
	pos->be = be;
	env->pos->cur++;
	return 0;
}

/*
 * two matches are known to have the same length
 * os is start of old pos array, ns is start of new,
 * oend and nend are end+1 pointers to ends of arrays.
 * oe and ne are ends (not end+1) of subarrays.
 * returns 1 if new is better, -1 if old, else 0.
 */

#if _AST_REGEX_DEBUG

static void
showmatch(regmatch_t* p)
{
	sfputc(sfstdout, \'(\');
	if (p->rm_so < 0)
		sfputc(sfstdout, \'?\');
	else
		sfprintf(sfstdout, "%d", p->rm_so);
	sfputc(sfstdout, \',\');
	if (p->rm_eo < 0)
		sfputc(sfstdout, \'?\');
	else
		sfprintf(sfstdout, "%d", p->rm_eo);
	sfputc(sfstdout, \')\');
}

static int
better(Env_t* env, Pos_t* os, Pos_t* ns, Pos_t* oend, Pos_t* nend, int level)
#define better	_better
{
	int	i;

	DEBUG_CODE(0x0040,{sfprintf(sfstdout, "AHA better old ");for (i = 0; i <= env->nsub; i++)showmatch(&env->best[i]);sfprintf(sfstdout, "\\n           new ");for (i = 0; i <= env->nsub; i++)showmatch(&env->match[i]);sfprintf(sfstdout, "\\n");},{0;});
	i = better(env, os, ns, oend, nend, 0);
	DEBUG_TEST(0x0040,(sfprintf(sfstdout, "           %s\\n", i <= 0 ? "OLD" : "NEW")),(0));
	return i;
}

#endif

static int
better(Env_t* env, Pos_t* os, Pos_t* ns, Pos_t* oend, Pos_t* nend, int level)
{
	Pos_t*	oe;
	Pos_t*	ne;
	int	k;
	int	n;

	if (env->error)
		return -1;
	for (;;)
	{
		DEBUG_CODE(0x0040,{sfprintf(sfstdout, "   %-*.*sold ", (level + 3) * 4, (level + 3) * 4, "");for (oe = os; oe < oend; oe++)sfprintf(sfstdout, "<%d,%d,%d>", oe->p - env->beg, oe->serial, oe->be);sfprintf(sfstdout, "\\n   %-*.*snew ", (level + 3) * 4, (level + 3) * 4, "");for (oe = ns; oe < nend; oe++)sfprintf(sfstdout, "<%d,%d,%d>", oe->p - env->beg, oe->serial, oe->be);sfprintf(sfstdout, "\\n");},{0;});
		if (ns >= nend)
			return DEBUG_TEST(0x2000,(os < oend),(0));
		if (os >= oend)
			return DEBUG_TEST(0x2000,(-1),(1));
		n = os->serial;
		if (ns->serial > n)
			return -1;
		if (n > ns->serial)
		{
			env->error = REG_PANIC;
			return -1;
		}
		if (ns->p > os->p)
			return 1;
		if (os->p > ns->p)
			return -1;
		oe = os;
		k = 0;
		for (;;)
			if ((++oe)->serial == n)
			{
				if (oe->be != END_ANY)
					k++;
				else if (k-- <= 0)
					break;
			}
		ne = ns;
		k = 0;
		for (;;)
			if ((++ne)->serial == n)
			{
				if (ne->be != END_ANY)
					k++;
				else if (k-- <= 0)
					break;
			}
		if (ne->p > oe->p)
			return 1;
		if (oe->p > ne->p)
			return -1;
		if (k = better(env, os + 1, ns + 1, oe, ne, level + 1))
			return k;
		os = oe + 1;
		ns = ne + 1;
	}
}

#undef	better

#define follow(e,r,c,s)	((r)->next?parse(e,(r)->next,c,s):(c)?parse(e,c,0,s):BEST)

static int		parse(Env_t*, Rex_t*, Rex_t*, unsigned char*);

static int
parserep(Env_t* env, Rex_t* rex, Rex_t* cont, unsigned char* s, int n)
{
	int	i;
	int	r = NONE;
	Rex_t	catcher;

	DEBUG_TEST(0x0010,(sfprintf(sfstdout, "AHA#%d parserep %d %d %d `%-.*s\'\\n", __LINE__, rex->lo, n, rex->hi, env->end - s, s)),(0));
	if ((rex->flags & REG_MINIMAL) && n >= rex->lo && n < rex->hi)
	{
		if (env->stack && pospush(env, rex, s, END_ANY))
			return BAD;
		i = follow(env, rex, cont, s);
		if (env->stack)
			pospop(env);
		switch (i)
		{
		case BAD:
			return BAD;
		case BEST:
		case GOOD:
			return BEST;
		}
	}
	if (n < rex->hi)
	{
		catcher.type = REX_REP_CATCH;
		catcher.serial = rex->serial;
		catcher.re.rep_catch.ref = rex;
		catcher.re.rep_catch.cont = cont;
		catcher.re.rep_catch.beg = s;
		catcher.re.rep_catch.n = n + 1;
		catcher.next = rex->next;
		if (env->stack)
		{
DEBUG_TEST(0x0001,(sfprintf(sfstdout,"AHA#%d PUSH (%d,%d)(%d,%d) (%d,%d)(%d,%d)\\n", __LINE__, rex->re.group.number, env->best[0].rm_so, env->best[0].rm_eo, env->best[1].rm_so, env->best[1].rm_eo, env->match[0].rm_so, env->match[0].rm_eo, env->match[1].rm_so, env->match[1].rm_eo)),(0));
			if (matchpush(env, rex))
				return BAD;
			if (pospush(env, rex, s, BEG_ONE))	
				return BAD;
DEBUG_TEST(0x0001,(sfprintf(sfstdout,"AHA#%d PUSH (%d,%d)(%d,%d) (%d,%d)(%d,%d)\\n", __LINE__, rex->re.group.number, env->best[0].rm_so, env->best[0].rm_eo, env->best[1].rm_so, env->best[1].rm_eo, env->match[0].rm_so, env->match[0].rm_eo, env->match[1].rm_so, env->match[1].rm_eo)),(0));
		}
		r = parse(env, rex->re.group.expr.rex, &catcher, s);
		DEBUG_TEST(0x0010,(sfprintf(sfstdout, "AHA#%d parserep parse %d `%-.*s\'\\n", __LINE__, r, env->end - s, s)),(0));
		if (env->stack)
		{
DEBUG_TEST(0x0001,(sfprintf(sfstdout,"AHA#%d POP (%d,%d)(%d,%d) (%d,%d)(%d,%d)\\n", __LINE__, rex->re.group.number, env->best[0].rm_so, env->best[0].rm_eo, env->best[1].rm_so, env->best[1].rm_eo, env->match[0].rm_so, env->match[0].rm_eo, env->match[1].rm_so, env->match[1].rm_eo)),(0));
			pospop(env);
			matchpop(env, rex);
DEBUG_TEST(0x0001,(sfprintf(sfstdout,"AHA#%d POP (%d,%d)(%d,%d) (%d,%d)(%d,%d)\\n", __LINE__, rex->re.group.number, env->best[0].rm_so, env->best[0].rm_eo, env->best[1].rm_so, env->best[1].rm_eo, env->match[0].rm_so, env->match[0].rm_eo, env->match[1].rm_so, env->match[1].rm_eo)),(0));
		}
		switch (r)
		{
		case BAD:
			return BAD;
		case BEST:
			return BEST;
		case GOOD:
			if (rex->flags & REG_MINIMAL)
				return BEST;
			r = GOOD;
			break;
		}
	}
	if (n < rex->lo)
		return r;
	if (!(rex->flags & REG_MINIMAL) || n >= rex->hi)
	{
		if (env->stack && pospush(env, rex, s, END_ANY))
			return BAD;
		i = follow(env, rex, cont, s);
		if (env->stack)
			pospop(env);
		switch (i)
		{
		case BAD:
			r = BAD;
			break;
		case BEST:
			r = BEST;
			break;
		case GOOD:
			r = (rex->flags & REG_MINIMAL) ? BEST : GOOD;
			break;
		}
	}
	return r;
}

static int
parsetrie(Env_t* env, Trie_node_t* x, Rex_t* rex, Rex_t* cont, unsigned char* s)
{
	unsigned char*	p;
	int		r;

	if (rex->flags & REG_ICASE)
	{
		p = state.fold;
		for (;;)
		{
			if (s >= env->end)
				return NONE;
			while (x->c != p[*s])
				if (!(x = x->sib))
					return NONE;
			if (x->end)
				break;
			x = x->son;
			s++;
		}
	}
	else
	{
		for (;;)
		{
			if (s >= env->end)
				return NONE;
			while (x->c != *s)
				if (!(x = x->sib))
					return NONE;
			if (x->end)
				break;
			x = x->son;
			s++;
		}
	}
	s++;
	if (rex->flags & REG_MINIMAL)
		switch (follow(env, rex, cont, s))
		{
		case BAD:
			return BAD;
		case BEST:
		case GOOD:
			return BEST;
		}
	if (x->son)
		switch (parsetrie(env, x->son, rex, cont, s))
		{
		case BAD:
			return BAD;
		case BEST:
			return BEST;
		case GOOD:
			if (rex->flags & REG_MINIMAL)
				return BEST;
			r = GOOD;
			break;
		default:
			r = NONE;
			break;
		}
	else
		r = NONE;
	if (!(rex->flags & REG_MINIMAL))
		switch (follow(env, rex, cont, s))
		{
		case BAD:
			return BAD;
		case BEST:
			return BEST;
		case GOOD:
			return GOOD;
	}
	return r;
}

static int
collmatch(Rex_t* rex, unsigned char* s, unsigned char* e, unsigned char** p)
{
	register Celt_t*	ce;
	unsigned char*		t;
	wchar_t			c;
	int			w;
	int			r;
	int			x;
	Ckey_t			key;
	Ckey_t			elt;

	if ((w = mbsize(s)) > 1)
	{
		memcpy((char*)key, (char*)s, w);
		key[w] = 0;
		mbxfrm(elt, key, COLL_KEY_MAX);
		t = s;
		c = mbchar(t);
		x = 0;
	}
	else
	{
		key[0] = s[0];
		key[1] = 0;
		r = mbxfrm(elt, key, COLL_KEY_MAX);
		while (w < COLL_KEY_MAX && &s[w] < e)
		{
			key[w] = s[w];
			key[w + 1] = 0;
			if (mbxfrm(elt, key, COLL_KEY_MAX) != r)
				break;
			w++;
		}
		key[w] = 0;
		mbxfrm(elt, key, COLL_KEY_MAX);
		c = s[0];
		x = w - 1;
	}
	r = 1;
	for (ce = rex->re.collate.elements;; ce++)
	{
		switch (ce->typ)
		{
		case COLL_call:
			if (!x && (*ce->fun)(c))
				break;
			continue;
		case COLL_char:
			if (!strcmp((char*)ce->beg, (char*)elt))
				break;
			continue;
		case COLL_range:
			if (strcmp((char*)ce->beg, (char*)elt) <= ce->min && strcmp((char*)elt, (char*)ce->end) <= ce->max)
				break;
			continue;
		case COLL_range_lc:
			if (strcmp((char*)ce->beg, (char*)elt) <= ce->min && strcmp((char*)elt, (char*)ce->end) <= ce->max && (islower(c) || !isupper(c)))
				break;
			continue;
		case COLL_range_uc:
			if (strcmp((char*)ce->beg, (char*)elt) <= ce->min && strcmp((char*)elt, (char*)ce->end) <= ce->max && (isupper(c) || !islower(c)))
				break;
			continue;
		default:
			r = 0;
			break;
		}
		if (!x || r)
			break;
		r = 1;
		w = x--;
		key[w] = 0;
		mbxfrm(elt, key, COLL_KEY_MAX);
	}
	*p = s + w;
	return rex->re.collate.invert ? !r : r;
}

static int
parse(Env_t* env, Rex_t* rex, Rex_t* cont, unsigned char* s)
{
	int		c;
	int		d;
	int		i;
	int		m;
	int		n;
	int		r;
	int*		f;
	unsigned char*	p;
	unsigned char*	t;
	unsigned char*	b;
	unsigned char*	e;
	regmatch_t*	o;
	Trie_node_t*	x;
	Rex_t*		q;
	Rex_t		catcher;
	Rex_t		next;

	for (;;)
	{
		switch (rex->type)
		{
		case REX_ALT:
			if (matchpush(env, rex))
				return BAD;
			if (pospush(env, rex, s, BEG_ALT))
				return BAD;
			catcher.type = REX_ALT_CATCH;
			catcher.serial = rex->serial;
			catcher.re.alt_catch.cont = cont;
			catcher.next = rex->next;
			r = parse(env, rex->re.group.expr.binary.left, &catcher, s);
			if (r < BEST || (rex->flags & REG_MINIMAL))
			{
				matchcopy(env, rex);
				((Pos_t*)env->pos->vec + env->pos->cur - 1)->serial = catcher.serial = rex->re.group.expr.binary.serial;
				n = parse(env, rex->re.group.expr.binary.right, &catcher, s);
				if (n != NONE)
					r = n;
			}
			pospop(env);
			matchpop(env, rex);
			return r;
		case REX_ALT_CATCH:
			if (pospush(env, rex, s, END_ANY))
				return BAD;
			r = follow(env, rex, rex->re.alt_catch.cont, s);
			pospop(env);
			return r;
		case REX_BACK:
			o = &env->match[rex->lo];
			if (o->rm_so < 0)
				return NONE;
			i = o->rm_eo - o->rm_so;
			e = s + i;
			if (e > env->end)
				return NONE;
			t = env->beg + o->rm_so;
			if (!(rex->flags & REG_ICASE))
			{
				while (s < e)
					if (*s++ != *t++)
						return NONE;
			}
			else if (!mbwide())
			{
				p = state.fold;
				while (s < e)
					if (p[*s++] != p[*t++])
						return NONE;
			}
			else
			{
				while (s < e)
				{
					c = mbchar(s);
					d = mbchar(t);
					if (toupper(c) != toupper(d))
						return NONE;
				}
			}
			break;
		case REX_BEG:
			if ((!(rex->flags & REG_NEWLINE) || s <= env->beg || *(s - 1) != \'\\n\') && ((env->flags & REG_NOTBOL) || s != env->beg))
				return NONE;
			break;
		case REX_CLASS:
			if (LEADING(env, rex, s))
				return NONE;
			n = rex->hi;
			if (n > env->end - s)
				n = env->end - s;
			m = rex->lo;
			if (m > n)
				return NONE;
			r = NONE;
			if (!(rex->flags & REG_MINIMAL))
			{
				for (i = 0; i < n; i++)
					if (!settst(rex->re.charclass, s[i]))
					{
						n = i;
						break;
					}
				for (s += n; n-- >= m; s--)
					switch (follow(env, rex, cont, s))
					{
					case BAD:
						return BAD;
					case BEST:
						return BEST;
					case GOOD:
						r = GOOD;
						break;
					}
			}
			else
			{
				for (e = s + m; s < e; s++)
					if (!settst(rex->re.charclass, *s))
						return r;
				e += n - m;
				for (;;)
				{
					switch (follow(env, rex, cont, s))
					{
					case BAD:
						return BAD;
					case BEST:
					case GOOD:
						return BEST;
					}
					if (s >= e || !settst(rex->re.charclass, *s))
						break;
					s++;
				}
			}
			return r;
		case REX_COLL_CLASS:
			if (LEADING(env, rex, s))
				return NONE;
			n = rex->hi;
			if (n > env->end - s)
				n = env->end - s;
			m = rex->lo;
			if (m > n)
				return NONE;
			r = NONE;
			e = env->end;
			if (!(rex->flags & REG_MINIMAL))
			{
				if (!(b = (unsigned char*)stkpush(stkstd, n)))
				{
					env->error = REG_ESPACE;
					return BAD;
				}
				for (i = 0; s < e && i < n && collmatch(rex, s, e, &t); i++)
				{
					b[i] = t - s;
					s = t;
				}
				for (; i-- >= rex->lo; s -= b[i])
					switch (follow(env, rex, cont, s))
					{
					case BAD:
						stkpop(stkstd);
						return BAD;
        case BEST:
						stkpop(stkstd);
						return BEST;
					case GOOD:
						r = GOOD;
						break;
					}
				stkpop(stkstd);
			}
		}
	}
}'
	OUTPUT - $'\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \

/*
 * posix regex executor
 * single sized-record interface
 */

/*nclude "reglib.*/

#ifdef _AST_REGEX_DEBUG
#define DEBUG_TEST(f,y,n)	((debug&(f))?(y):(n))
#define DEBUG_CODE(f,y,n)	do if(debug&(f)){y}else{n} while(0)
#define DEBUG_INIT()		do { char* t; if (!debug) { debug = 0x80000000; if (t = getenv("_AST_regex_debug")) debug |= strtoul(t, NiL, 0); } } while (0)
static unsigned long	debug;
#else
#define DEBUG_INIT()
#define DEBUG_TEST(f,y,n)	(n)
#define DEBUG_CODE(f,y,n)	do {n} while(0)
#endif

#define BEG_ALT		1	/* beginning of an alt			*/
#define BEG_ONE		2	/* beginning of one iteration of a rep	*/
#define BEG_REP		3	/* beginning of a repetition		*/
#define BEG_SUB		4	/* beginning of a subexpression		*/
#define END_ANY		5	/* end of any of above			*/

/*
 * returns from parse()
 */

#define NONE		0	/* no parse found			*/
#define GOOD		1	/* some parse was found			*/
#define BEST		2	/* an unbeatable parse was found	*/
#define BAD		3	/* error ocurred			*/

/*
 * REG_SHELL_DOT test
 */

#define LEADING(e,r,s)	((e)->leading&&*(s)==\'.\'&&((s)==(e)->beg||*((s)-1)==(r)->explicit))

/*
 * Pos_t is for comparing parses. An entry is made in the
 * array at the beginning and at the end of each Group_t,
 * each iteration in a Group_t, and each Binary_t.
 */

typedef struct
{
	unsigned char*	p;		/* where in string		*/
	size_t		length;		/* length in string		*/
	short		serial;		/* preorder subpattern number	*/
	short		be;		/* which end of pair		*/
} Pos_t;

/* ===== begin library support ===== */

#define vector(t,v,i)	(((i)<(v)->max)?(t*)((v)->vec+(i)*(v)->siz):(t*)vecseek(&(v),i))

static Vector_t*
vecopen __PARAM__((int inc, int siz), (inc, siz)) __OTORP__(int inc; int siz;)
#line 62
{
	Vector_t*	v;
	Stk_t*		sp;

	if (inc <= 0)
		inc = 16;
	if (!(sp = stkopen(STK_SMALL|STK_NULL)))
		return 0;
	if (!(v = (Vector_t*)stkseek(sp, sizeof(Vector_t) + inc * siz)))
	{
		stkclose(sp);
		return 0;
	}
	v->stk = sp;
	v->vec = (char*)v + sizeof(Vector_t);
	v->max = v->inc = inc;
	v->siz = siz;
	v->cur = 0;
	return v;
}

static __V_*
vecseek __PARAM__((Vector_t** p, int index), (p, index)) __OTORP__(Vector_t** p; int index;)
#line 85
{
	Vector_t*	v = *p;

	if (index >= v->max)
	{
		while ((v->max += v->inc) <= index);
		if (!(v = (Vector_t*)stkseek(v->stk, sizeof(Vector_t) + v->max * v->siz)))
			return 0;
		*p = v;
		v->vec = (char*)v + sizeof(Vector_t);
	}
	return v->vec + index * v->siz;
}

static void
vecclose __PARAM__((Vector_t* v), (v)) __OTORP__(Vector_t* v;)
#line 101
{
	if (v)
		stkclose(v->stk);
}

typedef struct
{
	Stk_pos_t	pos;
	char		data[1];
} Stk_frame_t;

#define stknew(s,p)	((p)->offset=stktell(s),(p)->base=stkfreeze(s,0))
#define stkold(s,p)	stkset(s,(p)->base,(p)->offset)

#define stkframe(s)	(*((Stk_frame_t**)(s)->_next-1))
#define stkdata(s,t)	((t*)stkframe(s)->data)
#define stkpop(s)	stkold(s,&(stkframe(s)->pos))

static __V_*
stkpush __PARAM__((Stk_t* sp, size_t size), (sp, size)) __OTORP__(Stk_t* sp; size_t size;)
#line 121
{
	Stk_frame_t*	f;
	Stk_pos_t	p;

	stknew(sp, &p);
	size = sizeof(Stk_frame_t) + sizeof(size_t) + size - 1;
	if (!(f = (Stk_frame_t*)stkalloc(sp, sizeof(Stk_frame_t) + sizeof(Stk_frame_t*) + size - 1)))
		return 0;
	f->pos = p;
	stkframe(sp) = f;
	return f->data;
}

/* ===== end library support ===== */

/*
 * Match_frame_t is for saving and restoring match records
 * around alternate attempts, so that fossils will not be
 * left in the match array.  These are the only entries in
 * the match array that are not otherwise guaranteed to
 * have current data in them when they get used.
 */

typedef struct
{
	size_t			size;
	regmatch_t*		match;
	regmatch_t		save[1];
} Match_frame_t;

#define matchframe	stkdata(stkstd,Match_frame_t)
#define matchpush(e,x)	((x)->re.group.number?_matchpush(e,x):0)
#define matchcopy(e,x)	((x)->re.group.number?memcpy(matchframe->match,matchframe->save,matchframe->size):(__V_*)0)
#define matchpop(e,x)	((x)->re.group.number?memcpy(matchframe->match,matchframe->save,matchframe->size),stkpop(stkstd):(__V_*)0)

#define pospop(e)	(--(e)->pos->cur)

/*
 * allocate a frame and push a match onto the stack
 */

static int
_matchpush __PARAM__((Env_t* env, Rex_t* rex), (env, rex)) __OTORP__(Env_t* env; Rex_t* rex;)
#line 164
{
	Match_frame_t*	f;
	regmatch_t*	m;
	regmatch_t*	e;
	regmatch_t*	s;
	int		num;

	if (rex->re.group.number <= 0 || (num = rex->re.group.last - rex->re.group.number + 1) <= 0)
		num = 0;
	if (!(f = (Match_frame_t*)stkpush(stkstd, sizeof(Match_frame_t) + (num - 1) * sizeof(regmatch_t))))
	{
		env->error = REG_ESPACE;
		return 1;
	}
	f->size = num * sizeof(regmatch_t);
	f->match = m = env->match + rex->re.group.number;
	e = m + num;
	s = f->save;
	while (m < e)
	{
		*s++ = *m;
		*m++ = state.nomatch;
	}
	return 0;
}

/*
 * allocate a frame and push a pos onto the stack
 */

static int
pospush __PARAM__((Env_t* env, Rex_t* rex, unsigned char* p, int be), (env, rex, p, be)) __OTORP__(Env_t* env; Rex_t* rex; unsigned char* p; int be;)
#line 196
{
	Pos_t*	pos;

	if (!(pos = vector(Pos_t, env->pos, env->pos->cur)))
	{
		env->error = REG_ESPACE;
		return 1;
	}
	pos->serial = rex->serial;
	pos->p = p;
	pos->be = be;
	env->pos->cur++;
	return 0;
}

/*
 * two matches are known to have the same length
 * os is start of old pos array, ns is start of new,
 * oend and nend are end+1 pointers to ends of arrays.
 * oe and ne are ends (not end+1) of subarrays.
 * returns 1 if new is better, -1 if old, else 0.
 */

#if _AST_REGEX_DEBUG

static void
showmatch __PARAM__((regmatch_t* p), (p)) __OTORP__(regmatch_t* p;)
#line 223
{
	sfputc(sfstdout, \'(\');
	if (p->rm_so < 0)
		sfputc(sfstdout, \'?\');
	else
		sfprintf(sfstdout, "%d", p->rm_so);
	sfputc(sfstdout, \',\');
	if (p->rm_eo < 0)
		sfputc(sfstdout, \'?\');
	else
		sfprintf(sfstdout, "%d", p->rm_eo);
	sfputc(sfstdout, \')\');
}

static int
better __PARAM__((Env_t* env, Pos_t* os, Pos_t* ns, Pos_t* oend, Pos_t* nend, int level), (env, os, ns, oend, nend, level)) __OTORP__(Env_t* env; Pos_t* os; Pos_t* ns; Pos_t* oend; Pos_t* nend; int level;)
#line 239

#define better	_better
{
	int	i;

	DEBUG_CODE(0x0040,{sfprintf(sfstdout, "AHA better old ");for (i = 0; i <= env->nsub; i++)showmatch(&env->best[i]);sfprintf(sfstdout, "\\n           new ");for (i = 0; i <= env->nsub; i++)showmatch(&env->match[i]);sfprintf(sfstdout, "\\n");},{0;});
	i = better(env, os, ns, oend, nend, 0);
	DEBUG_TEST(0x0040,(sfprintf(sfstdout, "           %s\\n", i <= 0 ? "OLD" : "NEW")),(0));
	return i;
}

#endif

static int
better __PARAM__((Env_t* env, Pos_t* os, Pos_t* ns, Pos_t* oend, Pos_t* nend, int level), (env, os, ns, oend, nend, level)) __OTORP__(Env_t* env; Pos_t* os; Pos_t* ns; Pos_t* oend; Pos_t* nend; int level;)
#line 253
{
	Pos_t*	oe;
	Pos_t*	ne;
	int	k;
	int	n;

	if (env->error)
		return -1;
	for (;;)
	{
		DEBUG_CODE(0x0040,{sfprintf(sfstdout, "   %-*.*sold ", (level + 3) * 4, (level + 3) * 4, "");for (oe = os; oe < oend; oe++)sfprintf(sfstdout, "<%d,%d,%d>", oe->p - env->beg, oe->serial, oe->be);sfprintf(sfstdout, "\\n   %-*.*snew ", (level + 3) * 4, (level + 3) * 4, "");for (oe = ns; oe < nend; oe++)sfprintf(sfstdout, "<%d,%d,%d>", oe->p - env->beg, oe->serial, oe->be);sfprintf(sfstdout, "\\n");},{0;});
		if (ns >= nend)
			return DEBUG_TEST(0x2000,(os < oend),(0));
		if (os >= oend)
			return DEBUG_TEST(0x2000,(-1),(1));
		n = os->serial;
		if (ns->serial > n)
			return -1;
		if (n > ns->serial)
		{
			env->error = REG_PANIC;
			return -1;
		}
		if (ns->p > os->p)
			return 1;
		if (os->p > ns->p)
			return -1;
		oe = os;
		k = 0;
		for (;;)
			if ((++oe)->serial == n)
			{
				if (oe->be != END_ANY)
					k++;
				else if (k-- <= 0)
					break;
			}
		ne = ns;
		k = 0;
		for (;;)
			if ((++ne)->serial == n)
			{
				if (ne->be != END_ANY)
					k++;
				else if (k-- <= 0)
					break;
			}
		if (ne->p > oe->p)
			return 1;
		if (oe->p > ne->p)
			return -1;
		if (k = better(env, os + 1, ns + 1, oe, ne, level + 1))
			return k;
		os = oe + 1;
		ns = ne + 1;
	}
}

#undef	better

#define follow(e,r,c,s)	((r)->next?parse(e,(r)->next,c,s):(c)?parse(e,c,0,s):BEST)

static int		parse __PROTO__((Env_t*, Rex_t*, Rex_t*, unsigned char*));

static int
parserep __PARAM__((Env_t* env, Rex_t* rex, Rex_t* cont, unsigned char* s, int n), (env, rex, cont, s, n)) __OTORP__(Env_t* env; Rex_t* rex; Rex_t* cont; unsigned char* s; int n;)
#line 319
{
	int	i;
	int	r = NONE;
	Rex_t	catcher;

	DEBUG_TEST(0x0010,(sfprintf(sfstdout, "AHA#%d parserep %d %d %d `%-.*s\'\\n", __LINE__, rex->lo, n, rex->hi, env->end - s, s)),(0));
	if ((rex->flags & REG_MINIMAL) && n >= rex->lo && n < rex->hi)
	{
		if (env->stack && pospush(env, rex, s, END_ANY))
			return BAD;
		i = follow(env, rex, cont, s);
		if (env->stack)
			pospop(env);
		switch (i)
		{
		case BAD:
			return BAD;
		case BEST:
		case GOOD:
			return BEST;
		}
	}
	if (n < rex->hi)
	{
		catcher.type = REX_REP_CATCH;
		catcher.serial = rex->serial;
		catcher.re.rep_catch.ref = rex;
		catcher.re.rep_catch.cont = cont;
		catcher.re.rep_catch.beg = s;
		catcher.re.rep_catch.n = n + 1;
		catcher.next = rex->next;
		if (env->stack)
		{
DEBUG_TEST(0x0001,(sfprintf(sfstdout,"AHA#%d PUSH (%d,%d)(%d,%d) (%d,%d)(%d,%d)\\n", __LINE__, rex->re.group.number, env->best[0].rm_so, env->best[0].rm_eo, env->best[1].rm_so, env->best[1].rm_eo, env->match[0].rm_so, env->match[0].rm_eo, env->match[1].rm_so, env->match[1].rm_eo)),(0));
			if (matchpush(env, rex))
				return BAD;
			if (pospush(env, rex, s, BEG_ONE))	
				return BAD;
DEBUG_TEST(0x0001,(sfprintf(sfstdout,"AHA#%d PUSH (%d,%d)(%d,%d) (%d,%d)(%d,%d)\\n", __LINE__, rex->re.group.number, env->best[0].rm_so, env->best[0].rm_eo, env->best[1].rm_so, env->best[1].rm_eo, env->match[0].rm_so, env->match[0].rm_eo, env->match[1].rm_so, env->match[1].rm_eo)),(0));
		}
		r = parse(env, rex->re.group.expr.rex, &catcher, s);
		DEBUG_TEST(0x0010,(sfprintf(sfstdout, "AHA#%d parserep parse %d `%-.*s\'\\n", __LINE__, r, env->end - s, s)),(0));
		if (env->stack)
		{
DEBUG_TEST(0x0001,(sfprintf(sfstdout,"AHA#%d POP (%d,%d)(%d,%d) (%d,%d)(%d,%d)\\n", __LINE__, rex->re.group.number, env->best[0].rm_so, env->best[0].rm_eo, env->best[1].rm_so, env->best[1].rm_eo, env->match[0].rm_so, env->match[0].rm_eo, env->match[1].rm_so, env->match[1].rm_eo)),(0));
			pospop(env);
			matchpop(env, rex);
DEBUG_TEST(0x0001,(sfprintf(sfstdout,"AHA#%d POP (%d,%d)(%d,%d) (%d,%d)(%d,%d)\\n", __LINE__, rex->re.group.number, env->best[0].rm_so, env->best[0].rm_eo, env->best[1].rm_so, env->best[1].rm_eo, env->match[0].rm_so, env->match[0].rm_eo, env->match[1].rm_so, env->match[1].rm_eo)),(0));
		}
		switch (r)
		{
		case BAD:
			return BAD;
		case BEST:
			return BEST;
		case GOOD:
			if (rex->flags & REG_MINIMAL)
				return BEST;
			r = GOOD;
			break;
		}
	}
	if (n < rex->lo)
		return r;
	if (!(rex->flags & REG_MINIMAL) || n >= rex->hi)
	{
		if (env->stack && pospush(env, rex, s, END_ANY))
			return BAD;
		i = follow(env, rex, cont, s);
		if (env->stack)
			pospop(env);
		switch (i)
		{
		case BAD:
			r = BAD;
			break;
		case BEST:
			r = BEST;
			break;
		case GOOD:
			r = (rex->flags & REG_MINIMAL) ? BEST : GOOD;
			break;
		}
	}
	return r;
}

static int
parsetrie __PARAM__((Env_t* env, Trie_node_t* x, Rex_t* rex, Rex_t* cont, unsigned char* s), (env, x, rex, cont, s)) __OTORP__(Env_t* env; Trie_node_t* x; Rex_t* rex; Rex_t* cont; unsigned char* s;)
#line 408
{
	unsigned char*	p;
	int		r;

	if (rex->flags & REG_ICASE)
	{
		p = state.fold;
		for (;;)
		{
			if (s >= env->end)
				return NONE;
			while (x->c != p[*s])
				if (!(x = x->sib))
					return NONE;
			if (x->end)
				break;
			x = x->son;
			s++;
		}
	}
	else
	{
		for (;;)
		{
			if (s >= env->end)
				return NONE;
			while (x->c != *s)
				if (!(x = x->sib))
					return NONE;
			if (x->end)
				break;
			x = x->son;
			s++;
		}
	}
	s++;
	if (rex->flags & REG_MINIMAL)
		switch (follow(env, rex, cont, s))
		{
		case BAD:
			return BAD;
		case BEST:
		case GOOD:
			return BEST;
		}
	if (x->son)
		switch (parsetrie(env, x->son, rex, cont, s))
		{
		case BAD:
			return BAD;
		case BEST:
			return BEST;
		case GOOD:
			if (rex->flags & REG_MINIMAL)
				return BEST;
			r = GOOD;
			break;
		default:
			r = NONE;
			break;
		}
	else
		r = NONE;
	if (!(rex->flags & REG_MINIMAL))
		switch (follow(env, rex, cont, s))
		{
		case BAD:
			return BAD;
		case BEST:
			return BEST;
		case GOOD:
			return GOOD;
	}
	return r;
}

static int
collmatch __PARAM__((Rex_t* rex, unsigned char* s, unsigned char* e, unsigned char** p), (rex, s, e, p)) __OTORP__(Rex_t* rex; unsigned char* s; unsigned char* e; unsigned char** p;)
#line 486
{
	register Celt_t*	ce;
	unsigned char*		t;
	wchar_t			c;
	int			w;
	int			r;
	int			x;
	Ckey_t			key;
	Ckey_t			elt;

	if ((w = mbsize(s)) > 1)
	{
		memcpy((char*)key, (char*)s, w);
		key[w] = 0;
		mbxfrm(elt, key, COLL_KEY_MAX);
		t = s;
		c = mbchar(t);
		x = 0;
	}
	else
	{
		key[0] = s[0];
		key[1] = 0;
		r = mbxfrm(elt, key, COLL_KEY_MAX);
		while (w < COLL_KEY_MAX && &s[w] < e)
		{
			key[w] = s[w];
			key[w + 1] = 0;
			if (mbxfrm(elt, key, COLL_KEY_MAX) != r)
				break;
			w++;
		}
		key[w] = 0;
		mbxfrm(elt, key, COLL_KEY_MAX);
		c = s[0];
		x = w - 1;
	}
	r = 1;
	for (ce = rex->re.collate.elements;; ce++)
	{
		switch (ce->typ)
		{
		case COLL_call:
			if (!x && (*ce->fun)(c))
				break;
			continue;
		case COLL_char:
			if (!strcmp((char*)ce->beg, (char*)elt))
				break;
			continue;
		case COLL_range:
			if (strcmp((char*)ce->beg, (char*)elt) <= ce->min && strcmp((char*)elt, (char*)ce->end) <= ce->max)
				break;
			continue;
		case COLL_range_lc:
			if (strcmp((char*)ce->beg, (char*)elt) <= ce->min && strcmp((char*)elt, (char*)ce->end) <= ce->max && (islower(c) || !isupper(c)))
				break;
			continue;
		case COLL_range_uc:
			if (strcmp((char*)ce->beg, (char*)elt) <= ce->min && strcmp((char*)elt, (char*)ce->end) <= ce->max && (isupper(c) || !islower(c)))
				break;
			continue;
		default:
			r = 0;
			break;
		}
		if (!x || r)
			break;
		r = 1;
		w = x--;
		key[w] = 0;
		mbxfrm(elt, key, COLL_KEY_MAX);
	}
	*p = s + w;
	return rex->re.collate.invert ? !r : r;
}

static int
parse __PARAM__((Env_t* env, Rex_t* rex, Rex_t* cont, unsigned char* s), (env, rex, cont, s)) __OTORP__(Env_t* env; Rex_t* rex; Rex_t* cont; unsigned char* s;)
#line 565
{
	int		c;
	int		d;
	int		i;
	int		m;
	int		n;
	int		r;
	int*		f;
	unsigned char*	p;
	unsigned char*	t;
	unsigned char*	b;
	unsigned char*	e;
	regmatch_t*	o;
	Trie_node_t*	x;
	Rex_t*		q;
	Rex_t		catcher;
	Rex_t		next;

	for (;;)
	{
		switch (rex->type)
		{
		case REX_ALT:
			if (matchpush(env, rex))
				return BAD;
			if (pospush(env, rex, s, BEG_ALT))
				return BAD;
			catcher.type = REX_ALT_CATCH;
			catcher.serial = rex->serial;
			catcher.re.alt_catch.cont = cont;
			catcher.next = rex->next;
			r = parse(env, rex->re.group.expr.binary.left, &catcher, s);
			if (r < BEST || (rex->flags & REG_MINIMAL))
			{
				matchcopy(env, rex);
				((Pos_t*)env->pos->vec + env->pos->cur - 1)->serial = catcher.serial = rex->re.group.expr.binary.serial;
				n = parse(env, rex->re.group.expr.binary.right, &catcher, s);
				if (n != NONE)
					r = n;
			}
			pospop(env);
			matchpop(env, rex);
			return r;
		case REX_ALT_CATCH:
			if (pospush(env, rex, s, END_ANY))
				return BAD;
			r = follow(env, rex, rex->re.alt_catch.cont, s);
			pospop(env);
			return r;
		case REX_BACK:
			o = &env->match[rex->lo];
			if (o->rm_so < 0)
				return NONE;
			i = o->rm_eo - o->rm_so;
			e = s + i;
			if (e > env->end)
				return NONE;
			t = env->beg + o->rm_so;
			if (!(rex->flags & REG_ICASE))
			{
				while (s < e)
					if (*s++ != *t++)
						return NONE;
			}
			else if (!mbwide())
			{
				p = state.fold;
				while (s < e)
					if (p[*s++] != p[*t++])
						return NONE;
			}
			else
			{
				while (s < e)
				{
					c = mbchar(s);
					d = mbchar(t);
					if (toupper(c) != toupper(d))
						return NONE;
				}
			}
			break;
		case REX_BEG:
			if ((!(rex->flags & REG_NEWLINE) || s <= env->beg || *(s - 1) != \'\\n\') && ((env->flags & REG_NOTBOL) || s != env->beg))
				return NONE;
			break;
		case REX_CLASS:
			if (LEADING(env, rex, s))
				return NONE;
			n = rex->hi;
			if (n > env->end - s)
				n = env->end - s;
			m = rex->lo;
			if (m > n)
				return NONE;
			r = NONE;
			if (!(rex->flags & REG_MINIMAL))
			{
				for (i = 0; i < n; i++)
					if (!settst(rex->re.charclass, s[i]))
					{
						n = i;
						break;
					}
				for (s += n; n-- >= m; s--)
					switch (follow(env, rex, cont, s))
					{
					case BAD:
						return BAD;
					case BEST:
						return BEST;
					case GOOD:
						r = GOOD;
						break;
					}
			}
			else
			{
				for (e = s + m; s < e; s++)
					if (!settst(rex->re.charclass, *s))
						return r;
				e += n - m;
				for (;;)
				{
					switch (follow(env, rex, cont, s))
					{
					case BAD:
						return BAD;
					case BEST:
					case GOOD:
						return BEST;
					}
					if (s >= e || !settst(rex->re.charclass, *s))
						break;
					s++;
				}
			}
			return r;
		case REX_COLL_CLASS:
			if (LEADING(env, rex, s))
				return NONE;
			n = rex->hi;
			if (n > env->end - s)
				n = env->end - s;
			m = rex->lo;
			if (m > n)
				return NONE;
			r = NONE;
			e = env->end;
			if (!(rex->flags & REG_MINIMAL))
			{
				if (!(b = (unsigned char*)stkpush(stkstd, n)))
				{
					env->error = REG_ESPACE;
					return BAD;
				}
				for (i = 0; s < e && i < n && collmatch(rex, s, e, &t); i++)
				{
					b[i] = t - s;
					s = t;
				}
				for (; i-- >= rex->lo; s -= b[i])
					switch (follow(env, rex, cont, s))
					{
					case BAD:
						stkpop(stkstd);
						return BAD;
        case BEST:
						stkpop(stkstd);
						return BEST;
					case GOOD:
						r = GOOD;
						break;
					}
				stkpop(stkstd);
			}
		}
	}
}'

TEST 10 'how did this hide til 2002?'
	EXEC -h
		INPUT - $'#pragma prototyped
void fun(int arg)
{
	SPAN(next) = ratio * STSIZE(arg);
}'
		OUTPUT - $'                  
void fun __PARAM__((int arg), (arg)) __OTORP__(int arg;){
	SPAN(next) = ratio * STSIZE(arg);
}'

TEST 11 'libpp splice'
	EXEC -nhf
	INPUT - $'int a = val>0?vau:0;
int b = val>0?val:0;'
	SAME OUTPUT INPUT

TEST 12 'option exercises'

	EXEC -s
		INPUT - $'/* : : generated by proto : : */
#pragma prototyped
extern int f(int);
extern int g(int);'

	EXEC -s -p
		OUTPUT - $'/* : : generated by proto : : */
#pragma prototyped
extern int f(int);
extern int g(int);'

	EXEC -s -f

	EXEC -s
		INPUT - $'/* : : generated by configure : : */
#pragma prototyped
extern int f(int);
extern int g(int);'
		OUTPUT - $'
/* : : generated by proto : : */
/* : : generated by configure : : */
                  

#if !defined(__PROTO__)
#include <prototyped.h>
#endif
#if !defined(__LINKAGE__)
#define __LINKAGE__		/* 2004-08-11 transition */
#endif
extern __MANGLE__ int f __PROTO__((int));
extern __MANGLE__ int g __PROTO__((int));'

	EXEC -s -o type=test,type=gpl
		INPUT - $'/* : : generated by proto : : */
#pragma prototyped
extern int f(int);
extern int g(int);'
		OUTPUT -

	EXEC -s -p -o type=test,type=gpl
		OUTPUT - $'/***********************************************************************
*                                                                      *
*                          Copyright (c) 2001                          *
*                                                                      *
*        This is free software; you can redistribute it and/or         *
*     modify it under the terms of the GNU General Public License      *
*            as published by the Free Software Foundation;             *
*       either version 2, or (at your option) any later version.       *
*                                                                      *
*           This software is distributed in the hope that it           *
*              will be useful, but WITHOUT ANY WARRANTY;               *
*         without even the implied warranty of MERCHANTABILITY         *
*                 or FITNESS FOR A PARTICULAR PURPOSE.                 *
*         See the GNU General Public License for more details.         *
*                                                                      *
*                You should have received a copy of the                *
*                      GNU General Public License                      *
*           along with this software (see the file COPYING.)           *
*                    If not, a copy is available at                    *
*                 http://www.gnu.org/copyleft/gpl.html                 *
*                                                                      *
***********************************************************************/
/* : : generated by proto : : */
#pragma prototyped
extern int f(int);
extern int g(int);'

	EXEC -s -f -o type=test,type=gpl

	EXEC -s -o type=test,type=gpl
		INPUT - $'/* : : generated by configure : : */
#pragma prototyped
extern int f(int);
extern int g(int);'
		OUTPUT - $'/***********************************************************************
*                                                                      *
*                          Copyright (c) 2001                          *
*                                                                      *
*        This is free software; you can redistribute it and/or         *
*     modify it under the terms of the GNU General Public License      *
*            as published by the Free Software Foundation;             *
*       either version 2, or (at your option) any later version.       *
*                                                                      *
*           This software is distributed in the hope that it           *
*              will be useful, but WITHOUT ANY WARRANTY;               *
*         without even the implied warranty of MERCHANTABILITY         *
*                 or FITNESS FOR A PARTICULAR PURPOSE.                 *
*         See the GNU General Public License for more details.         *
*                                                                      *
*                You should have received a copy of the                *
*                      GNU General Public License                      *
*           along with this software (see the file COPYING.)           *
*                    If not, a copy is available at                    *
*                 http://www.gnu.org/copyleft/gpl.html                 *
*                                                                      *
***********************************************************************/

/* : : generated by proto : : */
/* : : generated by configure : : */
                  

#if !defined(__PROTO__)
#include <prototyped.h>
#endif
#if !defined(__LINKAGE__)
#define __LINKAGE__		/* 2004-08-11 transition */
#endif
extern __MANGLE__ int f __PROTO__((int));
extern __MANGLE__ int g __PROTO__((int));'

TEST 13 'already noticed'

	EXEC -s
		INPUT - $'
/* Copyright (c) 2001 bogomips Corp. */
#pragma prototyped
extern int f(int);
extern int g(int);'
		OUTPUT - $'
/* : : generated by proto : : */

/* Copyright (c) 2001 bogomips Corp. */
                  

#if !defined(__PROTO__)
#include <prototyped.h>
#endif
#if !defined(__LINKAGE__)
#define __LINKAGE__		/* 2004-08-11 transition */
#endif
extern __MANGLE__ int f __PROTO__((int));
extern __MANGLE__ int g __PROTO__((int));'

	EXEC -s -p

	EXEC -s -f

	EXEC -s
		INPUT - $'#pragma prototyped noticed
extern int f(int);
extern int g(int);'
		OUTPUT - $'
/* : : generated by proto : : */
                          

#if !defined(__PROTO__)
#include <prototyped.h>
#endif
#if !defined(__LINKAGE__)
#define __LINKAGE__		/* 2004-08-11 transition */
#endif
extern __MANGLE__ int f __PROTO__((int));
extern __MANGLE__ int g __PROTO__((int));'

	EXEC -s -o type=test,type=gpl
		INPUT - $'/* Copyright (c) 2001 bogomips Corp. */
#pragma prototyped
extern int f(int);
extern int g(int);'
		OUTPUT - $'
/* : : generated by proto : : */
/* Copyright (c) 2001 bogomips Corp. */
                  

#if !defined(__PROTO__)
#include <prototyped.h>
#endif
#if !defined(__LINKAGE__)
#define __LINKAGE__		/* 2004-08-11 transition */
#endif
extern __MANGLE__ int f __PROTO__((int));
extern __MANGLE__ int g __PROTO__((int));'

	EXEC -s -p -o type=test,type=gpl
		OUTPUT - $'
/* : : generated by proto : : */
/* Copyright (c) 2001 bogomips Corp. */
                  

#if !defined(__PROTO__)
#include <prototyped.h>
#endif
#if !defined(__LINKAGE__)
#define __LINKAGE__		/* 2004-08-11 transition */
#endif
extern __MANGLE__ int f __PROTO__((int));
extern __MANGLE__ int g __PROTO__((int));'

	EXEC -s -f -o type=test,type=gpl

	EXEC -s -o type=test,type=gpl
		INPUT - $'#pragma prototyped noticed
extern int f(int);
extern int g(int);'
		OUTPUT - $'
/* : : generated by proto : : */
                          

#if !defined(__PROTO__)
#include <prototyped.h>
#endif
#if !defined(__LINKAGE__)
#define __LINKAGE__		/* 2004-08-11 transition */
#endif
extern __MANGLE__ int f __PROTO__((int));
extern __MANGLE__ int g __PROTO__((int));'

TEST 14 'already prototyped by __P() macros'

	EXEC -f -h
		INPUT - $'typedef int (*fun_f) __P((void const *));
void fun __P((int));'
		SAME OUTPUT INPUT

TEST 15 '#(define|undef) extern intercepts'

	EXEC -f -h
		INPUT - $'#define extern __EXPORT__
extern fun(int);
#undef extern'
		OUTPUT - $'#undef __MANGLE__
#define __MANGLE__ __LINKAGE__ __EXPORT__
extern __MANGLE__ fun(int);
#undef __MANGLE__
#define __MANGLE__ __LINKAGE__'

	EXEC -f -h -n
		OUTPUT - $'#undef __MANGLE__

#line 1
#define __MANGLE__ __LINKAGE__ __EXPORT__
extern __MANGLE__ fun(int);
#undef __MANGLE__

#line 3
#define __MANGLE__ __LINKAGE__'

	EXEC -f -h
		INPUT - $'#define extern extern __IMPORT__
extern fun(int);
#undef extern'
		OUTPUT - $'#undef __MANGLE__
#define __MANGLE__ __LINKAGE__ __IMPORT__
extern __MANGLE__ fun(int);
#undef __MANGLE__
#define __MANGLE__ __LINKAGE__'

	EXEC -f -h -n
		OUTPUT - $'#undef __MANGLE__

#line 1
#define __MANGLE__ __LINKAGE__ __IMPORT__
extern __MANGLE__ fun(int);
#undef __MANGLE__

#line 3
#define __MANGLE__ __LINKAGE__'

TEST 16 'array arg prototypes'

	EXEC -f -h
		INPUT - $'static void f(t1 a[A][B], t2 b[12], t3 c, t4 d)
{
	t5	e;
	return 0;
}'
		OUTPUT - $'static void f __PARAM__((t1 a[A][B], t2 b[12], t3 c, t4 d), (a, b, c, d)) __OTORP__(t1 a[A][B]; t2 b[12]; t3 c; t4 d;){
	t5	e;
	return 0;
}'

TEST 17 'externalize'

	EXEC -nh
		INPUT - $'#pragma prototyped
static int fun(int);
static int beg;
static int fun(int arg) { return !arg; }
static int end;'
		OUTPUT - $'                  \nstatic int fun __PROTO__((int));
static int beg;
static int fun __PARAM__((int arg), (arg)) __OTORP__(int arg;)
#line 4
{ return !arg; }
static int end;'

	EXEC -nhx
		OUTPUT - $'                  \nextern int fun __PROTO__((int));
static int beg;
extern int fun __PARAM__((int arg), (arg)) __OTORP__(int arg;)
#line 4
{ return !arg; }
static int end;'

	EXEC -nh
		INPUT - $'#pragma prototyped
#define __NTH(p)	__attribute__(nothrow) p
static __inline double
__NTH (atof (__const char *__nptr))
{
  return strtod (__nptr, (char **) NULL);
}'
		OUTPUT - $'                  
#define __NTH(p)	__attribute__(nothrow) p
static __inline double
__NTH (atof (__const char *__nptr))
{
  return strtod (__nptr, (char **) NULL);
}'

	EXEC -nhx

	EXEC -nh
		INPUT - $'#pragma prototyped

__BEGIN_NAMESPACE_STD
typedef __clock_t clock_t;
__END_NAMESPACE_STD
#if defined __USE_XOPEN || defined __USE_POSIX || defined __USE_MISC
__USING_NAMESPACE_STD(clock_t)
#endif'
		OUTPUT - $'                  \n
__BEGIN_NAMESPACE_STD
typedef __clock_t clock_t;
__END_NAMESPACE_STD
#if defined __USE_XOPEN || defined __USE_POSIX || defined __USE_MISC
__USING_NAMESPACE_STD(clock_t)
#endif'

	EXEC -nhx
