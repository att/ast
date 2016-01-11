# regression tests for the ast libpp cpp

KEEP "*.h"

function DATA
{
	for f
	do	case $f in
		a.h)	print -r -- $'#include "c.h"' > $f ;;
		b.h)	print -r -- $'#include "c.h"' > $f ;;
		c.h)	print -r -- $'#include "d.h"' > $f ;;
		hdra.c)	print -r -- $'#define _CAT(a,b,c) a##b##c
#define hdra    hdrx
#define hdr     _CAT(<,hdra,.h>)
#include hdr' > $f ;;
		hdrm.c)	print -r -- $'#include <hdrx.h>' > $f ;;
		hdrx.c)	print -r -- $'#define _XAT(a,b,c) a##b##c
#define _CAT(a,b,c) _XAT(a,b,c)
#define hdra    hdrx
#define hdr     _CAT(<,hdra,.h>)
#include hdr' > $f ;;
		hdrx.h)	print -r -- $'int f(){return 0;}' > $f ;;
		map1.c)	print -r -- $'#include <tst_usr.h>\n#include <tst_std.h>' > $f ;;
		map2.c)	print -r -- $'#include <tst_std.h>\n#include <tst_usr.h>' > $f ;;
		pmap1.c)print -r -- $'#pragma pp:mapinclude hosted <tst_usr.h> = "."\n#include <tst_usr.h>\n#include <tst_std.h>' > $f ;;
		pmap2.c)print -r -- $'#pragma pp:mapinclude hosted <tst_usr.h> = "."\n#include <tst_std.h>\n#include <tst_usr.h>' > $f ;;
		lcl/map.map) mkdir -p lcl && print -r -- $'hosted <tst_usr.h>="."' > $f ;;
		lcl/tst_usr.h) mkdir -p lcl && print -r -- $'int lcl_tst_usr;' > $f ;;
		pfx) mkdir -p std/pfx; print '#include <pfx/c.h>' > std/a.h; print '#include "d.h"' > std/pfx/c.h; print 'int d = 0;' > std/pfx/d.h; ;;
		std/tst_std.h) mkdir -p std && print -r -- $'#include <tst_usr.h>' > $f ;;
		std/tst_usr.h) mkdir -p std && print -r -- $'int std_tst_usr;' > $f ;;
		nl1.c)	print -r -- $'#warning before\n#include "nl1.h"\n#warning after' > $f ;;
		nl1.h)	print -r -n -- $'#warning during' > $f ;;
		nl2.c)	print -r -- $'#warning before\n#include "nl2.h"\n#warning after' > $f ;;
		nl2.h)	print -r -n -- $'#ifndef NL2\n#warning during\n#endif' > $f ;;
		nl3.c)	print -r -- $'#warning before\n#include "nl3.h"\n#warning after' > $f ;;
		nl3.h)	print -r -n -- $'#ifndef NL2\nwarning during\n#endif' > $f ;;
		nl4.c)	print -r -- $'#warning before\n#include "nl4.h"\n#warning after' > $f ;;
		nl4.h)	print -r -n -- $'warning during' > $f ;;
		nl5.c)	print -r -- $'#warning before\n#include "nl5.h"\n#warning after' > $f ;;
		nl5.h)	print -r -n -- $'// warning during' > $f ;;
		esac
	done
}

TEST 01 'bug'
	EXEC -I-D
		INPUT - $'#define A(a)\t        a
\n#define B(a,b)\t\tA(a;)
\nB("b",
0);
__LINE__ :: 7'
		OUTPUT - $'# 1 ""
\n\n\n\n
"b";;
7 :: 7'
	EXEC -I-D
		INPUT - $'/*
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx abcdefg
1XXXXXX XXXXXXX XXXXXXX XXXXXXX XXXXXXX XXXXXXX XXXXXXX ABCDEFG
2XXXXXX XXXXXXX XXXXXXX XXXXXXX XXXXXXX XXXXXXX XXXXXXX ABCDEFG
3XXXXXX XXXXXXX XXXXXXX XXXXXXX XXXXXXX XXXXXXX XXXXXXX ABCDEFG
4XXXXXX XXXXXXX XXXXXXX XXXXXXX XXXXXXX XXXXXXX XXXXXXX ABCDEFG
*/
int x;'
		OUTPUT - $'# 1 ""
 
# 135
int x;'
	EXEC -I-D
		INPUT - $'#pragma pp:headerexpand
#pragma pp:include "."
#include "bug" ".03" ".h"'
		OUTPUT - $'# 1 ""'
		ERROR - $'cpp: line 3: bug.03.h: cannot find include file'
		EXIT 1
	EXEC -I-D
		INPUT - $'#pragma pp:headerexpand
#pragma pp:include "."
#define x\t<bug.03.h>
#include\tx'
		OUTPUT - $'# 1 ""'
		ERROR - $'cpp: line 4: bug.03.h: cannot find include file'
	EXEC -I-D
		INPUT - $'#pragma pp:headerexpand
#pragma pp:include "."
#define x\t<y.03.h>
#define y\tbug
#include\tx'
		OUTPUT - $'# 1 ""'
		ERROR - $'cpp: line 5: bug.03.h: cannot find include file'
	EXEC -I-D
		INPUT - $'#pragma pp:headerexpand
#pragma pp:include "."
#define Concat3(a,b,c)a##b##c
#define FOO bug
#define HDR Concat3(<,FOO,.03.h>)
#include HDR'
		OUTPUT - $'# 1 ""'
		ERROR - $'cpp: line 6: FOO.03.h: cannot find include file'

TEST 02 'ess'
	EXEC -I-D
		INPUT - $'#define CON(CHAR) CHAR
int a=0;
int b=0;
int SMSUBRRACT;
int SMSUBRRMACT;

#define SMREGCONV(x,reg,mask)   ( (~(CON(x)ACT ^ reg)) & (CON(x)MACT ^ mask) )
main()
{
  int subrr = SMREGCONV(SMSUBRR, a, b);
}'
		OUTPUT - $'# 1 ""

int a=0;
int b=0;
int SMSUBRRACT;
int SMSUBRRMACT;


main()
{
  int subrr = ( (~(SMSUBRR ACT ^ a)) & (SMSUBRR MACT ^ b) );
}'

TEST 03 'gnu'
	EXEC -I-D
		INPUT - $'#include \\\n</dev/null>'
		OUTPUT - $'# 1 ""

# 1 "/dev/null"

# 3 ""'
		ERROR - $'cpp: line 2: warning: #include: reference to /dev/null is not portable'
	EXEC -I-D
		INPUT - $'#define add(a,b) a+b
add(1,\\\n2);'
		OUTPUT - $'# 1 ""


1+ 2;'
		ERROR -
	EXEC -I-D
		INPUT - ''
		OUTPUT - $'# 1 ""'
	EXEC -I-D
		INPUT - $'/* should yield `rep-list;\' */
#define f()\trep-list
f /* */ ();'
		OUTPUT - $'# 1 ""
 
\nrep-list;'
	EXEC -I-D
		INPUT - $'#define test() replist
test
#include "/dev/null"
();'
		OUTPUT - $'# 1 ""
\ntest
# 1 "/dev/null"
\n# 4 ""
();'
		ERROR - 'cpp: line 3: warning: #include: reference to /dev/null is not portable'
	EXEC -I-D
		INPUT - $'/* should yield `a = + + i;\' */
#define\tp(arg)\t+arg i
a = p(+);'
		OUTPUT - $'# 1 ""
 
\na = + + i;'
		ERROR -
	EXEC -I-D
		INPUT - $'/* non-{tab,space} white space in directives */
#define\fa\tb c'
		OUTPUT - $'# 1 ""\n '
		ERROR - $'cpp: line 2: warning: `formfeed\' invalid in directives'
	EXEC -I-D
		INPUT - $'#include ""\t/* should be syntax error: invalid q-char-sequence */
#include <>\t/* should be syntax error: invalid h-char-sequence */'
		OUTPUT - $'# 1 ""'
		ERROR - $'cpp: line 1: #include: null file name
cpp: line 2: #include: null file name'
		EXIT 2
	EXEC -I-D
		INPUT - $'/* should print "hello world" */
#define f(a)\t#a
puts(f(hello/* space */world));'
		OUTPUT - $'# 1 ""
 
\nputs("hello world");'
		ERROR -
		EXIT 0
	EXEC -I-D
		INPUT - $'#define p(a,b)\ta ## b\t/* example of correct code */
#define q(a,b)\ta ## ##\t/* constraint violation diagnosed */
#define r(a,b)\t## b\t/* constraint violation diagnosed */'
		OUTPUT - $'# 1 ""'
		ERROR - $'cpp: line 2: ## rhs operand omitted
cpp: line 3: # lhs operand omitted'
		EXIT 2
	EXEC -I-D
		INPUT - $'/* second operand of && shouldn\'t
** be evaluated.
*/
#define M 0
#if (M != 0) && (100 / M)
#endif'
		OUTPUT - $'# 1 ""\n '
		ERROR -
		EXIT 0
	EXEC -I-D
		INPUT - $'/* valid constant suffix */
#if 1UL\t
okee
#endif
\n#if \'\\xA\' == 10
dokee
#endif'
		OUTPUT - $'# 1 ""\n \n\nokee\n\n\n\ndokee'
	EXEC -I-D
		INPUT - $'/* promote operand to Ulong */
#if 0xFFFFFFFF < 1
nope
#endif'
		OUTPUT - $'# 1 ""\n '

TEST 04 'gsf'
	EXEC -I-D
		INPUT - $'#macdef type(x)
#ifndef x ## _t_def
#define x ## _t_def 1
declare x ## _t
#endif
#endmac /* type */
\n#macdef flag(x)
#if !strcmp(#x,"0")
#let _flag_ = 0
#else
#ifndef _flag_
#let _flag_ = 0
#endif
_define_expand_(x, (1 << _flag_))
#let _flag_ = _flag_ + 1
#endif
#endmac /* flag */
\n#macdef _define_expand_(x, v)
_define_(x, v)
#endmac /* _define_expand_ */
\n#macdef _define_(x, v)
#define x v
#endmac /* _define_ */'
		OUTPUT - $'# 1 ""'
	EXEC -I-D
		INPUT - $'#define f(x)\tx
#define q(x)\t# x
\nq(@LX@);
q("@LX@");
q(\'@LX@\');
\nf(@LX@);
f("@LX@");
f(\'@LX@\');
\nn(@LX@);
n("@LX@");
n(\'@LX@\');'
		OUTPUT - $'# 1 ""
\n
\n"@LX@";
"\\"@LX@\\"";
"\'@LX@\'";
\n@LX@;
"@LX@";
\'@LX@\';
\nn(@LX@);
n("@LX@");
n(\'@LX@\');'
	EXEC -I-D
		INPUT - $'#define f(x)\t\tx
#define str(s)\t\t# s
#define xstr(s)\t\tstr(s)
#define glue(a, b)\ta ## b
#define xglue(a, b)\tglue(a, b)

str("abc");
str("def\\0x");
str(\'\\4\');
str(@ \\n);
str(@LX@);
str("@LX@");
str(\'@LX@\');
f(@LX@);
f("@LX@");
f(\'@LX@\');

@LX@
"@LX@"
\'@LX@\'
__LINE__'
		OUTPUT - $'# 1 ""\n\n\n\n\n\n
"\\"abc\\"";
"\\"def\\\\0x\\"";
"\'\\\\4\'";
"@ \\n";
"@LX@";
"\\"@LX@\\"";
"\'@LX@\'";
@LX@;
"@LX@";
\'@LX@\';

@LX@
"@LX@"
\'@LX@\'
21'
	EXEC -I-D
		INPUT - $'#define i(x) x
#define q(x) #x
\ni(\\n);
i(\'\\n\');
i("\\n");
i("\'\\n");
i("\'\\n`");
\ni(\'\\"\');
i("\\"");
i("\'\\"");
i("\'\\"`");
\nq(\\n);
q(\'\\n\');
q("\\n");
q("\'\\n");
q("\'\\n`");
\nq(\'\\"\');
q("\\"");
q("\'\\"");
q("\'\\"`");'
		OUTPUT - $'# 1 ""
\n
\n\\n;
\'\\n\';
"\\n";
"\'\\n";
"\'\\n`";
\n\'\\"\';
"\\"";
"\'\\"";
"\'\\"`";
\n"\\n";
"\'\\\\n\'";
"\\"\\\\n\\"";
"\\"\'\\\\n\\"";
"\\"\'\\\\n`\\"";
\n"\'\\\\\\"\'";
"\\"\\\\\\"\\"";
"\\"\'\\\\\\"\\"";
"\\"\'\\\\\\"`\\"";'
	EXEC -I-D
		INPUT - $'#define f(a,b)\ta\\\nb
#define aa\tAA
#define bb\tBB
#define aabb\tZZ
f(aa,bb);'
		OUTPUT - $'# 1 ""
\n
\n
\nab;'
	EXEC -I-D
		INPUT - $'#macdef Mike(x,y) {
\tfor(;x;y) {}
}
#endmac
\nmain () {
\tMike(1,2);
}'
		OUTPUT - $'# 1 ""
\n
\n
\nmain () {
\t
# 1 ":Mike,7"
{ for(; 1; 2) {}
}
# 7 ""
;
}'
	EXEC -I-D
		INPUT - $'#define foo -bar
-foo'
		OUTPUT - $'# 1 ""
\n--bar'
	EXEC -I-D
		INPUT - $'#define ELOG(m) (m)
\n#define Cleanup_Handler(CLASSNAME) ELOG((test, #CLASSNAME, key)); 
 
Cleanup_Handler(Cache_manager)'
		OUTPUT - $'# 1 ""
\n
\n 
( (test, "Cache_manager", key));'
	EXEC -I-D
		INPUT - $'int\ta;
int\tb;
int\trbyte;
\n#define rbyte\tb
\nmain()
{
#pragma pp:compatibility
\ta=0xAE+rbyte;
#pragma pp:nocompatibility
\ta=0xAE+rbyte;
}'
		OUTPUT - $'# 1 ""
int\ta;
int\tb;
int\trbyte;
\n
\nmain()
{
\n\ta=0xAE+b;
\n\ta=0xAE+rbyte;
}'
	EXEC -I-D
		INPUT - $'#ifdef AAA
aaa
#else ifdef BBB
bbb
#else ifndef CCC
!ccc
#else
ddd
#endif'
		OUTPUT - $'# 1 ""\n\n\n\nbbb\n\n'
		ERROR - $'cpp: line 3: warning: ifdef: invalid characters after directive
cpp: line 5: more than one #else for #if
cpp: line 5: warning: ifndef: invalid characters after directive
cpp: line 7: more than one #else for #if'
		EXIT 2
	EXEC -I-D
		INPUT - $'#if AAA
aaa
#else if BBB
bbb
#else if !CCC
!ccc
#else
ddd
#endif'
		ERROR - $'cpp: line 3: warning: if: invalid characters after directive
cpp: line 5: more than one #else for #if
cpp: line 5: warning: if: invalid characters after directive
cpp: line 7: more than one #else for #if'
	EXEC -I-D
		INPUT - $'#define glue(a, b)\ta ## b
#define xglue(a, b)\tglue(a, b)
#define HELLO\t\thello
#define LO\t\tLO world
xglue(HEL, LO)'
		OUTPUT - $'# 1 ""
\n
\n
hello world'
		ERROR -
		EXIT 0
	EXEC -I-D
		INPUT - $'#define <long long> __int64
#define <long double> __float128
#define foo FOO
\nint
foo
#line 10
(
long
int
bar
,
long
long
aha
)
{
\tlong long i;
\tlong double f;
\twhile(i){foo(i--);}
\treturn "FOO";
}'
		OUTPUT - $'# 1 ""
\n
\n
int
FOO
# 10
(
long 
 int
\nbar
,
\n__int64
aha
)
{
\t__int64 i;
\t__float128 f;
\twhile(i){FOO(i--);}
\treturn "FOO";
}'
	EXEC -I-D
		INPUT - $'int
foo()
{
XXabcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
\treturn __FUNCTION__;
}'
		OUTPUT - $'# 1 ""
int
foo()
{
XXabcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdefg,abcdef,
\treturn "foo";
}'
	EXEC -I-D
		INPUT - $'#pragma pp:headerexpand
#define _STD_INCLUDE_DIR\t/e/program files/micrsoft platform sdk/include
#define _STD_HEADER(name)\t<_STD_INCLUDE_DIR/name>
\n#include _STD_HEADER(stdio.h)'
		OUTPUT - $'# 1 ""'
		ERROR - $'cpp: line 5: warning: #include: reference to /e/program files/micrsoft platform sdk/include/stdio.h is not portable
cpp: line 5: /e/program files/micrsoft platform sdk/include/stdio.h: cannot find include file'
		EXIT 1
	EXEC -I-D
		INPUT - $'#define __P(x)\t\tx
\n#pragma pp:hide opendir closedir DIR
\nextern DIR*\topendir __P((const char* __name));
extern int\tclosedir __P((DIR* __dirp));
\n#pragma pp:nohide opendir closedir DIR
\nextern DIR*\topendir __P((const char* __name));
extern int\tclosedir __P((DIR* __dirp));'
		OUTPUT - $'# 1 ""
\n
\n
extern _0_DIR_hIDe*\t_0_opendir_hIDe (const char* __name);
extern int\t_0_closedir_hIDe (_0_DIR_hIDe* __dirp);
\n
\nextern DIR*\topendir (const char* __name);
extern int\tclosedir (DIR* __dirp);'
		ERROR -
		EXIT 0

TEST 05 'stack skew'
	EXEC -I-D
		INPUT - $'#define CAT(a,b)  a##b
#define XCAT(a,b) CAT(a,b)
\n#define R )
#define k(n,a,b,c) XCAT(k,n)(a,b,c)
#define kac(a,b,c) XCAT(c,XCAT(b,a)R
\n(k(ac,1,2,3))'
		OUTPUT - $'# 1 ""
\n
\n
\n
\n(ac1'
	EXEC -I-D
		INPUT - $'#define CAT(a,b)  a##b
#define XCAT(a,b) CAT(a,b)
\n#define kl0 (
#define kl1 )
#define k(n,a,b,c) XCAT(k,n)(a,b,c)
#define kbc(a,b,c) XCAT(a,XCAT(c,b))
#define kab(a,b,c) XCAT(b,XCAT(a,c))
#define kac(a,b,c) XCAT(c,XCAT(b,a)kl1
\n
(k(bc,1,2,3))
(k(ab,1,2,3))
(k(ac,1,2,3))'
		OUTPUT - $'# 1 ""

# 12
(132)
(213)
(ac1'
	EXEC -I-D
		INPUT - $'#define a(x) fishy(x
#define b(y) a(y
#define delay(z) z
a(b)(4)))
delay(a(b)(4))))'
		OUTPUT - $'# 1 ""
\n
\nfishy( fishy( 4)
fishy( fishy( 4))'
		ERROR - $'cpp: line 5: a: `EOF\' in macro argument list'
		EXIT 1
	EXEC -I-D
		INPUT - $'#define A 0x00000080
#define B 0x80000000
#if (A & B)
         junk junk
#endif
\nmain(){}'
		OUTPUT - $'# 1 ""
\n\n\n\n\n\nmain(){}'
		ERROR -
		EXIT 0
	EXEC -I-D
		INPUT - $'#define L(x) "x
#define R(x) x"
L(abc)R(xyz);'
		OUTPUT - $'# 1 ""
\n"x\\n#define R(x) x" R(xyz);'
		ERROR - $'cpp: line 1: warning: `newline\' in string'
	EXEC -I-D
		INPUT - $'#define a(x) x
a(a(1'
		OUTPUT -n - $'# 1 ""\n\n1 '
		ERROR - $'cpp: a: `EOF\' in macro argument list
cpp: line 2: a: `EOF\' in macro argument list'
		EXIT 2
	EXEC -I-D
		INPUT - $'#define hsccs(str) #ident str
\nmain(){
        hsccs("hello");
}'
		OUTPUT - $'# 1 ""
\n
main(){
        ident "hello";
}'
		ERROR - $'cpp: line 1: # must precede a formal parameter'
		EXIT 1

TEST 06 'knr'
	EXEC -I-D
		INPUT - $'#define f(a)\tf(a)
#define g\tf
#define t(a)\ta
\nt(g)(0)'
		OUTPUT - $'# 1 ""
\n
\n
f( 0)'
	EXEC -I-D
		INPUT - $'#define WORDSIZE        ROUND(4)
#define ROUND(y)\t(y)
\nsize = ROUND(WORDSIZE);'
		OUTPUT - $'# 1 ""
\n
\nsize = ( ( 4));'
	EXEC -I-D
		INPUT - $'#define had data.had
had'
		OUTPUT - $'# 1 ""
\ndata.had'
	EXEC -I-D
		INPUT - $'#define i(a)\ta
#define j(a)\ta
i(i)(1);
j(i)(2);'
		OUTPUT - $'# 1 ""
\n
i(1);
2;'
	EXEC -I-D
		INPUT - $'#define a b
#define b(x)\t[x]
#define i(x)\tx
a(1)
b(1)
i(a)(1)
i(b)(1)
i(a + b)(1)'
		OUTPUT - $'# 1 ""
\n
\n[ 1]
[ 1]
[ 1]
[ 1]
b+ [ 1]'

TEST 07 'net'
	EXEC -I-D
		INPUT - $'#define f(a) a*g
#define g(a) f(a)
f(2)(9)\t\t/* involves "split hide sets" */
printf("expands to either of the following (intentionally undefined): \\\n\t2*f(9) \\\n\t2*9*g ");'
		OUTPUT - $'# 1 ""\n\n\n2*9*g \nprintf("expands to either of the following (intentionally undefined): \t2*f(9) \t2*9*g ");\n\n'
	EXEC -I-D
		INPUT - $'#define echo1(a)\t32 + (a)
#define echo2(b)\t{echo1(b); x}
#define x\t\t2 * x
\necho2(x)'
		OUTPUT - $'# 1 ""
\n
\n
{32 + ( 2 * x); 2 * x}'
	EXEC -I-D
		INPUT - $'#define x\t2 * x
#define f(a)\tg(a)
#define g(b)\t(b)

"f(x)" : f(x) ;
"g(x)" : g(x) ;'
		OUTPUT - $'# 1 ""
\n\n\n
"f(x)": ( 2 * x) ;
"g(x)": ( 2 * x) ;'
	EXEC -I-D
		INPUT - $'#define s(x)\t\t#x
#define stringize(x)\ts(x)
#define\tBACKSLASH\t\\\t/* This program prints one backslash. */
\nmain() {
\tputs( stringize( \\BACKSLASH ));
\treturn 0;
}'
		OUTPUT - $'# 1 ""
\n
\n
main() {
\tputs( "\\\\");
\treturn 0;
}'
	EXEC -I-D
		INPUT - $'#define s(x)\t\t#x
#define stringize(x)\ts(x)
#define\tBACKSLASH\t\\\t/* This program prints one backslash. */
\nmain() {
\tputs( stringize( BACKSLASH ));
\treturn 0;
}'
		OUTPUT - $'# 1 ""\n\n\n\n\nmain() {\n\tputs( "\\\\");\n\treturn 0;\n}'
		ERROR -
	EXEC -I-D
		INPUT - $'#define s(x)\t\t#x
#define stringize(x)\ts(x)

\nmain() {
\tputs( stringize( \\ ));
\treturn 0;
}'

TEST 08 'stc'
	EXEC -I-D
		INPUT - $'#define Dimension(info) char * info
\ntypedef unsigned short  Dimension;
\nmain()
{
        Dimension(B);
        Dimension a;
}'
		OUTPUT - $'# 1 ""
\n
typedef unsigned short  Dimension;
\nmain()
{
        char * B;
        Dimension a;
}'

TEST 09 'std'
	EXEC -I-D
		INPUT - $'#define f(a)\tf(x * (a))
#define x\t2
#define z\tz[0]
\nf(f(z))'
		OUTPUT - $'# 1 ""
\n
\n
f(2 * ( f(2 * ( z[0]))))'
	EXEC -I-D
		INPUT - $'#define x\t3
#define f(a)\tf(x * (a))
#undef\tx
#define x\t2
#define g\tf
#define z\tz[0]
#define h\tg(~
#define m(a)\ta(w)
#define w\t0,1
#define t(a)\ta
\nf(y+1) + f(f(z)) % t(t(g)(0) + t)(1);
g(2+(3,4)-w) | h 5) & m
\t(f)^m(m);'
		OUTPUT - $'# 1 ""\n\n# 12\nf(2 * ( y+1)) + f(2 * ( f(2 * ( z[0])))) % f(2 * ( 0)) + t(1);\nf(2 * ( 2+(3,4)-0,1)) | f(2 * ( ~ 5)) & \nf(2 * ( 0,1))^m(0,1);'
	EXEC -I-D
		INPUT - $'#define str(s)\t\t# s
#define xstr(s)\t\tstr(s)
#define debug(s, t)\tprintf("x" # s "= %d, x" # t "= %s", \\\n\t\t\t\tx ## s, x ## t)
#define INCFILE(n)\tvers ## n /* from previous #include example */
#define glue(a, b)\ta ## b
#define xglue(a, b)\tglue(a, b)
#define HIGHLOW\t\t"hello"
#define LOW\t\tLOW ", world"
\ndebug(1, 2);
fputs(str(strncmp("abc\\0d", "abc", \'\\4\') /* this goes away */
\t== 0) str(: @\\n), s);
#include xstr(INCFILE(2).h)
glue(HIGH, LOW)
xglue(HIGH, LOW)'
		OUTPUT - $'# 1 ""

# 11
printf("x1= %d, x2= %s", x1, x2);
fputs(\n"strncmp(\\"abc\\\\0d\\", \\"abc\\", \'\\\\4\') == 0: @\\n", s);\n
"hellohello, world"'
		ERROR - $'cpp: line 14: vers2.h: cannot find include file'
		EXIT 1
	EXEC -I-D
		INPUT vers2.h $'_VERS2_H'
		OUTPUT - $'# 1 ""\n\n# 11\nprintf("x1= %d, x2= %s", x1, x2);
fputs(\n"strncmp(\\"abc\\\\0d\\", \\"abc\\", \'\\\\4\') == 0: @\\n", s);
# 1 "vers2.h"\n_VERS2_H\n# 15 ""\n"hellohello, world"'
		ERROR -
		EXIT 0
	EXEC -I-D
		INPUT - $'#define S(a)\t# a
#define s(a)\tS(a)
\n#define G(a,b)\ta ## b
#define g(a,b)\tG(a,b)
\n#define f(a)\ta
\n#define h\thello
#define w\tworld
\n#define hw\t"h ## w"
#define helloworld\t"hello ## world"'
		OUTPUT - $'# 1 ""'
	EXEC -I-D
		INPUT - $'#define s(a)\t# a
#define xs(a)\ts(a)
\n#define g(a,b)\ta ## b
#define xg(a,b)\tg(a,b)
\n#define f(a)\txg(file,a)
#define h(a)\txs(f(a))
\n#define v\t1
\nf(v)
\nh(v)'
		OUTPUT - $'# 1 ""

# 12
file1

"file1"'
	EXEC -I-D
		INPUT - $'#define a(x)\tx
#define bc\tz
a(b)c'
		OUTPUT - $'# 1 ""
\n
b c'
	EXEC -I-D
		INPUT - $'#define a A
#define b B
#define ab xx
#define AB XX
#define g(a,b) a##b ZZZ
#define x(a,b) g(a,b) ZZZ
\ng(a,b)
x(a,b)'
		OUTPUT - $'# 1 ""
\n
\n
\n
\nxx ZZZ
XX ZZZ ZZZ'
	EXEC -I-D
		INPUT - $'#define glue(a, b)\ta ## b
#define xglue(a, b)\tglue(a, b)
#define HIGHLOW\t\t"hello"
#define LOW\t\tLOW ", world"
#if def
#define highlow\t\txxxx
#endif
glue(high, low);
glue(HIGH, LOW);
xglue(HIGH, LOW);'
		OUTPUT - $'# 1 ""
\n
\n
\n
\nhighlow;
"hello";
"hello, world";'
	EXEC -I-D
		INPUT - $'void a() {
#if -1 > 1
\tprintf("bad\\n");
#else
\tprintf("good\\n");
#endif
}
\nvoid b() { 
#if -1U > 1
\tprintf("good\\n");
#else
\tprintf("bad\\n");
#endif
}'
		OUTPUT - $'# 1 ""\nvoid a() {\n\n\n\n\tprintf("good\\n");\n\n}
\nvoid b() { \n\n\tprintf("good\\n");\n\n\n\n}'
	EXEC -I-D
		INPUT - $'#define echo(a)\t{ a }
#define x\t2 * x
echo(x)'
		OUTPUT - $'# 1 ""
\n
{ 2 * x }'

TEST 10 'extensions'
	EXEC -I-D
		INPUT - $'#define <long long>\t__int64
#define _ARG_(x)\tx
extern long long\tfun _ARG_((long*));'
		OUTPUT - $'# 1 ""


extern __int64	fun (long *   );'
	EXEC -I-D
		INPUT - $'#pragma pp:catliteral
char* a = "aaa" "zzz";'
		OUTPUT - $'# 1 ""

char* a = "aaazzz";'
	EXEC -I-D
		INPUT - $'#pragma pp:catliteral
char* a = "aaa"
/* aha */
"zzz";
int line = __LINE__;'
		OUTPUT - $'# 1 ""

char* a = "aaazzz"
# 4
;
int line = 5;'
	EXEC -I-D
		INPUT - $'#pragma pp:catliteral
#pragma pp:spaceout
char* a = "aaa"
/* aha */
"zzz";
int line = __LINE__;'
		OUTPUT - $'# 1 ""


char* a = "aaa\\
zzz"
# 5
;
int line = 6;'
	EXEC -I-D
		INPUT - $'#pragma pp:pragmaexpand
#pragma pp:catliteral
#define _TEXTSEG(name)  ".text$" #name
#define AFX_COLL_SEG    _TEXTSEG(AFX_COL1)
"AFX_COLL_SEG" : AFX_COLL_SEG ;
#pragma code_seg(AFX_COLL_SEG)'
		OUTPUT - $'# 1 ""




"AFX_COLL_SEG": ".text$AFX_COL1";
#pragma code_seg (".text$" "AFX_COL1")'
	EXEC -I-D
		INPUT - $'#pragma pp:nopragmaexpand
#pragma pp:catliteral
#define _TEXTSEG(name)  ".text$" #name
#define AFX_COLL_SEG    _TEXTSEG(AFX_COL1)
"AFX_COLL_SEG" : AFX_COLL_SEG ;
#pragma code_seg(AFX_COLL_SEG)'
		OUTPUT - $'# 1 ""




"AFX_COLL_SEG": ".text$AFX_COL1";
#pragma code_seg (AFX_COLL_SEG)'
	EXEC -I-D
		INPUT - $'#pragma pp:pragmaexpand
#pragma pp:nocatliteral
#define _TEXTSEG(name)  ".text$" #name
#define AFX_COLL_SEG    _TEXTSEG(AFX_COL1)
"AFX_COLL_SEG" : AFX_COLL_SEG ;
#pragma code_seg(AFX_COLL_SEG)'
		OUTPUT - $'# 1 ""




"AFX_COLL_SEG" : ".text$" "AFX_COL1" ;
#pragma code_seg (".text$" "AFX_COL1")'
	EXEC -I-D
		INPUT - $'#pragma pp:nopragmaexpand
#pragma pp:nocatliteral
#define _TEXTSEG(name)  ".text$" #name
#define AFX_COLL_SEG    _TEXTSEG(AFX_COL1)
"AFX_COLL_SEG" : AFX_COLL_SEG ;
#pragma code_seg(AFX_COLL_SEG)'
		OUTPUT - $'# 1 ""




"AFX_COLL_SEG" : ".text$" "AFX_COL1" ;
#pragma code_seg (AFX_COLL_SEG)'
	EXEC -I-D
		INPUT - $'#pragma pp:nosplicespace\nint a\\      \nb = 1;'
		OUTPUT - $'# 1 ""\n\nint a\\      \nb = 1;'
	EXEC -I-D
		INPUT - $'#pragma pp:splicespace\nint a\\      \nb = 1;'
		OUTPUT - $'# 1 ""\n\nint ab = 1;'
	EXEC -I-D
		INPUT - $'#pragma pp:splicespace\nchar s[] = "\\ ";'
		OUTPUT - $'# 1 ""\n\nchar s[] = "\\ ";'
	EXEC -I-D
		INPUT - $'#pragma pp:splicespace\nchar s[] = "\\\\ ";'
		OUTPUT - $'# 1 ""\n\nchar s[] = "\\\\ ";'

TEST 11 'make dependencies'
	DO	DATA a.h b.h c.h
	EXEC -I-D -D:transition -M t.c
		EXIT 2
		INPUT t.c $'#include "a.h"\n#include "b.h"'
		OUTPUT - $'t.o : t.c a.h c.h b.h'
		ERROR - $'cpp: "c.h", line 1: d.h: cannot find include file\ncpp: "c.h", line 1: d.h: cannot find include file'
	EXEC -I-D -D:transition -MG t.c
		EXIT 0
		OUTPUT - $'t.o : t.c a.h c.h d.h b.h'
		ERROR -
	EXEC -D:transition -MMG t.c
		INPUT t.c $'#include "a.h"\n#include "b.h"\n#include <stdio.h>'
	EXEC -I-D -D:transition -MGD t.c
		INPUT t.c $'#include "a.h"\n#include "b.h"'
		OUTPUT t.d $'t.o : t.c a.h c.h d.h b.h'
		OUTPUT - $'# 1 "t.c"

# 1 "a.h"

# 1 "c.h"

# 2 "a.h"

# 2 "t.c"

# 1 "b.h"

# 1 "c.h"

# 2 "b.h"

# 3 "t.c"'

TEST 12 'variadic macros'
	EXEC -I-D
		EXIT 8
		INPUT - $'#define e1(...,...)
#define e2(... ...)
#define e3(__VA_ARGS__,...)
#define e4(...,__VA_ARGS__)
#define e5(...,a)

#define f1(...)		g1(x,__VA_ARGS__)
#define f2(a,...)	g2(a,x,__VA_ARGS__)
#define f3(a,v...)	g2(a,x,v)

f1();
f1(1);
f1(1,2);

f2(1);
f2(1,2);

f3(1);
f3(1,2);'
		OUTPUT - $'# 1 ""

# 11
g1(x, );
g1(x, 1);
g1(x, 1,2);

g2( 1,x, );
g2( 1,x, 2);

g2( 1,x, );
g2( 1,x, 2);'
		ERROR - $'cpp: line 1: e1: ...: duplicate macro formal argument
cpp: line 1: e1: __VA_ARGS__: duplicate macro formal argument
cpp: line 2: e2: ...: duplicate macro formal argument
cpp: line 3: e3: __VA_ARGS__: invalid macro formal argument
cpp: line 3: e3: __VA_ARGS__: duplicate macro formal argument
cpp: line 4: e4: __VA_ARGS__: macro formal argument cannot follow ...
cpp: line 4: e4: __VA_ARGS__: duplicate macro formal argument
cpp: line 5: e5: a: macro formal argument cannot follow ...'

TEST 13 'headerexpand vs. headerexpandall -- standardize this'
	DO	DATA hdra.c hdrm.c hdrx.c hdrx.h
	EXEC -I-D -I. hdra.c
		OUTPUT - $'# 1 "hdra.c"'
		ERROR - $'cpp: "hdra.c", line 4: warning: _CAT: 3 actual arguments expected
cpp: "hdra.c", line 4: #include: "..." or <...> argument expected'
		EXIT 1
	EXEC -I-D -I. -D:headerexpand hdra.c
		ERROR - $'cpp: "hdra.c", line 4: hdra.h: cannot find include file'
	EXEC -I-D -I. -D:headerexpandall hdra.c
		OUTPUT - $'# 1 "hdra.c"

# 1 "hdrx.h"
int f(){return 0;}
# 5 "hdra.c"'
		ERROR -
		EXIT 0
	EXEC -I-D -I. hdrx.c
		OUTPUT - $'# 1 "hdrx.c"'
		ERROR - $'cpp: "hdrx.c", line 5: warning: _CAT: 3 actual arguments expected
cpp: "hdrx.c", line 5: warning: _XAT: 3 actual arguments expected
cpp: "hdrx.c", line 5: #include: "..." or <...> argument expected'
		EXIT 1
	EXEC -I-D -I. -D:headerexpand hdrx.c
		OUTPUT - $'# 1 "hdrx.c"

# 1 "hdrx.h"
int f(){return 0;}
# 6 "hdrx.c"'
		ERROR -
		EXIT 0
	EXEC -I-D -I. -D:headerexpandall hdrx.c
	EXEC -I-D -I. hdrm.c
		OUTPUT - $'# 1 "hdrm.c"

# 1 "hdrx.h"
int f(){return 0;}
# 2 "hdrm.c"'
	EXEC -I-D -I. -D:headerexpand hdrm.c
	EXEC -I-D -I. -D:headerexpandall hdrm.c
	EXEC -I-D -I. -Dhdrx=hdra hdrm.c
	EXEC -I-D -I. -Dhdrx=hdra -D:headerexpand hdrm.c
	EXEC -I-D -I. -Dhdrx=hdra -D:headerexpandall hdrm.c

TEST 14 'ancient proto stealth memory fault'
	EXEC -I-D -D:compatibility
		INPUT - $'#pragma prototyped
a(Xfio_t* sp){}
b(Xfio_t* sp){}
d(Xfio_t* sp){}
e(Xfio_t* sp){}
f(Xfio_t* sp){}
g(char* buf,int size,Xfio_t* sp){}
h(Xfio_t* sp){}
i(int c,Xfio_t* sp){}
j(char* buf,Xfio_t* sp){}
k(void* buf,size_t size,size_t n,Xfio_t* sp){}
l(void* buf,size_t size,size_t n,Xfio_t* sp){}
m(Xfio_t* sp){}'
		OUTPUT - $'# 1 ""

# 2
a (sp) Xfio_t* sp;
# 2
{}
b (sp) Xfio_t* sp;
# 3
{}
d (sp) Xfio_t* sp;
# 4
{}
e (sp) Xfio_t* sp;
# 5
{}
f (sp) Xfio_t* sp;
# 6
{}
g (buf, size, sp) char* buf;int size;Xfio_t* sp;
# 7
{}
h (sp) Xfio_t* sp;
# 8
{}
i (c, sp) int c;Xfio_t* sp;
# 9
{}
j (buf, sp) char* buf;Xfio_t* sp;
# 10
{}
k (buf, size, n, sp) char* buf;size_t size;size_t n;Xfio_t* sp;
# 11
{}
l (buf, size, n, sp) char* buf;size_t size;size_t n;Xfio_t* sp;
# 12
{}
m (sp) Xfio_t* sp;
# 13
{}'

TEST 15 'pragma passing'
	EXEC -I-D
		INPUT - $'
#pragma option push -b -a8 -pc -A- /*P_O_Push*/
#pragma 	 option 	 push 	 -b 	 -a8 	 -pc 	 -A- 	 /*P_O_Push*/
#pragma 	 option 	 push 	 -b 	 -a8 	 -pc 	 -A- 	 -B- 	 /*P_O_Push*/
#pragma 	 option 	 push 	 -b 	 -a8 	 -pc 	 -A- 	 -B-
#pragma 	 option 	 push 	 @b 	 @a8 	 @pc 	 @A@ 	 @B@
#pragma 	 option 	 push 	 %b 	 %a8 	 %pc 	 %A% 	 %B%'
		OUTPUT - $'# 1 ""

#pragma option push -b -a8 -pc -A-
#pragma option push -b -a8 -pc -A-
#pragma option push -b -a8 -pc -A- -B-
#pragma option push -b -a8 -pc -A- -B-
#pragma option push @b @a8 @pc @A@ @B@
#pragma option push %b %a8 %pc %A% %B%'

TEST 16 'no newline in directive in last include line'
	DO	DATA nl1.c nl1.h nl2.c nl2.h nl3.c nl3.h nl4.c nl4.h nl5.c nl5.h
	EXEC -I-D nl1.c
		OUTPUT - $'# 1 "nl1.c"

# 1 "nl1.h"

# 3 "nl1.c"'
		ERROR - $'cpp: "nl1.c", line 1: warning: before
cpp: "nl1.h", line 1: warning: during
cpp: "nl1.h", line 2: warning: file does not end with `newline\'
cpp: "nl1.c", line 3: warning: after'
	EXEC -I-D nl2.c
		OUTPUT - $'# 1 "nl2.c"

# 1 "nl2.h"

# 3 "nl2.c"'
		ERROR - $'cpp: "nl2.c", line 1: warning: before
cpp: "nl2.h", line 2: warning: during
cpp: "nl2.h", line 4: warning: file does not end with `newline\'
cpp: "nl2.c", line 3: warning: after'
	EXEC -I-D nl3.c
		OUTPUT - $'# 1 "nl3.c"

# 1 "nl3.h"

warning during
# 3 "nl3.c"'
		ERROR - $'cpp: "nl3.c", line 1: warning: before
cpp: "nl3.h", line 4: warning: file does not end with `newline\'
cpp: "nl3.c", line 3: warning: after'
	EXEC -I-D nl4.c
		OUTPUT - $'# 1 "nl4.c"

# 1 "nl4.h"
warning during
# 3 "nl4.c"'
		ERROR - $'cpp: "nl4.c", line 1: warning: before
cpp: "nl4.h", line 1: warning: file does not end with `newline\'
cpp: "nl4.c", line 3: warning: after'
	EXEC -I-D -D-+ nl5.c
		OUTPUT - $'# 1 "nl5.c"

# 1 "nl5.h"
 
# 3 "nl5.c"'
		ERROR - $'cpp: "nl5.c", line 1: warning: before
cpp: "nl5.h", line 1: warning: file does not end with `newline\'
cpp: "nl5.c", line 3: warning: after'

TEST 17 'pp:mapinclude'
	DO	DATA map1.c pmap1.c map2.c pmap2.c lcl/map.map lcl/tst_usr.h std/tst_std.h std/tst_usr.h
	EXEC -I-D -Ilcl -I-H -Istd pmap1.c
		OUTPUT - $'# 1 "pmap1.c"

# 1

# 1 "lcl/tst_usr.h"
int lcl_tst_usr;
# 2 "pmap1.c"

# 1 "std/tst_std.h"

# 1 "std/tst_usr.h"
int std_tst_usr;
# 2 "std/tst_std.h"

# 3 "pmap1.c"'
	EXEC -I-D -D:mapinclude='hosted <tst_usr.h>="."' -Ilcl -I-H -Istd map1.c
		OUTPUT - $'# 1 "map1.c"

# 1 "lcl/tst_usr.h"
int lcl_tst_usr;
# 2 "map1.c"

# 1 "std/tst_std.h"

# 1 "std/tst_usr.h"
int std_tst_usr;
# 2 "std/tst_std.h"

# 3 "map1.c"'
	EXEC -I-D -I-Mmap.map -Ilcl -I-H -Istd map1.c
		OUTPUT - $'# 1 "map.map"

# 1 "map1.c"

# 1 "lcl/tst_usr.h"
int lcl_tst_usr;
# 2 "map1.c"

# 1 "std/tst_std.h"

# 1 "std/tst_usr.h"
int std_tst_usr;
# 2 "std/tst_std.h"

# 3 "map1.c"

# 1'
	EXEC -I-D -Ilcl -I-H -Istd pmap2.c
		OUTPUT - $'# 1 "pmap2.c"

# 1

# 1 "std/tst_std.h"

# 1 "std/tst_usr.h"
int std_tst_usr;
# 2 "std/tst_std.h"

# 2 "pmap2.c"

# 1 "lcl/tst_usr.h"
int lcl_tst_usr;
# 3 "pmap2.c"'
	EXEC -I-D -D:mapinclude='hosted <tst_usr.h>="."' -Ilcl -I-H -Istd map2.c
		OUTPUT - $'# 1 "map2.c"

# 1 "std/tst_std.h"

# 1 "std/tst_usr.h"
int std_tst_usr;
# 2 "std/tst_std.h"

# 2 "map2.c"

# 1 "lcl/tst_usr.h"
int lcl_tst_usr;
# 3 "map2.c"'
	EXEC -I-D -D:mapinclude='hosted <tst_usr.h>="."' -Ilcl -I-H -Istd map2.c
	EXEC -I-D -I-Mmap.map -Ilcl -I-H -Istd map2.c
		OUTPUT - $'# 1 "map.map"

# 1 "map2.c"

# 1 "std/tst_std.h"

# 1 "std/tst_usr.h"
int std_tst_usr;
# 2 "std/tst_std.h"

# 2 "map2.c"

# 1 "lcl/tst_usr.h"
int lcl_tst_usr;
# 3 "map2.c"

# 1'

TEST 18 'inappropriate pushback'
	EXEC -I-D
		INPUT - $'#define A(a)	(a)
#define B	A(C*)
#define C(a)	a
#(getmac A);
B;
#(getmac A);'
		OUTPUT - $'# 1 ""\n\n\n
"( @A1)";
( C*);
"( @A1)";'

TEST 19 'stray macro disable'
	EXEC -I-D
		INPUT - $'#define INIT(x) = x
#define Nullav Null(AV*)
#define Nullfp Null(PerlIO*)
#define Null(type) ((type)NULL)
#define NULL ((void*)0)
#define PerlIO PerlIO
#define PERLVARI(var,type,init) type PL_##var INIT(init);
PERLVARI(Irsfp, PerlIO*, Nullfp)
PERLVARI(Irsfp_filters, AV*, Nullav)'
		OUTPUT - $'# 1 ""\n\n\n\n\n\n\n
PerlIO* PL_Irsfp = (( PerlIO*)((void*)0));
AV* PL_Irsfp_filters = (( AV*)((void*)0));'

TEST 20 'transition splice'
	EXEC -I-D -D:transition -P
		INPUT - $'#pragma prototyped
int a = val>0?vau:0;
int b = val>0?val:0;'
		OUTPUT - $'
int a = val>0?vau:0;
int b = val>0?val:0;'

TEST 21 '#if expressions'
	EXEC -I-D 
		INPUT - $'#define ENV_SEP \':\'
#define IS_ENV_SEP(ch) ((ch) == ENV_SEP)
#if \':\' == ENV_SEP
y : 4 : __LINE__
#else
n : 6 : __LINE__
#endif
#if (\':\') == ENV_SEP
y : 9 : __LINE__
#else
n : 11 : __LINE__
#endif
#if (\':\' == ENV_SEP)
y : 14 : __LINE__
#else
n : 16 : __LINE__
#endif
#if ((\':\') == ENV_SEP)
y : 19 : __LINE__
#else
n : 21 : __LINE__
#endif
#if IS_ENV_SEP(\':\')
y : 24 : __LINE__
#else
n : 26 : __LINE__
#endif
#if IS_ENV_SEP(\';\')
n : 29 : __LINE__
#else
y : 31 : __LINE__
#endif'
		OUTPUT - $'# 1 ""\n\n\n\ny : 4 : 4\n\n\n\n\ny : 9 : 9\n\n\n\n
y : 14 : 14\n\n\n\n\ny : 19 : 19\n\n\n\n\ny : 24 : 24\n\n\n\n\n\n\ny : 31 : 31'

	EXEC -I-D
		INPUT - $'#if 0x10 == 020
__LINE__
#endif
#if 0x10 == 16
__LINE__
#endif'
		OUTPUT - $'# 1 ""\n\n2\n\n\n5'

TEST 22 '-D-d'
	EXEC -I-D -D-d
		INPUT - $'#define A B
#define C D \\
	E \\
	F
A
C'
		OUTPUT - $'# 1 ""
#define A B


#define C D E F
B
D E F'

TEST 23 'empty macro actuals'
	EXEC -I-D
		INPUT - $'#define f(a) F[a]
0 : f();
1 : f(1);
2 : f(1,2);'
		OUTPUT - $'# 1 ""

0 : F[ ];
1 : F[ 1];
2 : F[ 1];'
		ERROR - $'cpp: line 4: warning: f: 1 actual argument expected'
	EXEC -I-D
		INPUT - $'#define f(a,b) F[a][b]
0 : f();
1 : f(1);
2 : f(1,2);
3 : f(1,2,3);'
		OUTPUT - $'# 1 ""

0 : F[ ][ ];
1 : F[ 1][ ];
2 : F[ 1][ 2];
3 : F[ 1][ 2];'
		ERROR - $'cpp: line 2: warning: f: 2 actual arguments expected
cpp: line 3: warning: f: 2 actual arguments expected
cpp: line 5: warning: f: 2 actual arguments expected'
	EXEC -I-D
		INPUT - $'#define f(a,b) F[a][b]
f(1);
f();
f(1,2);
f(1,);
f(,2);
f(,);
f(1,2,3);
f(1,2,);
f(1,,3);
f(1,,);
f(,2,3);
f(,2,);
f(,,3);
f(,,);'
		OUTPUT - $'# 1 ""

F[ 1][ ];
F[ ][ ];
F[ 1][ 2];
F[ 1][ ];
F[ ][ 2];
F[ ][ ];
F[ 1][ 2];
F[ 1][ 2];
F[ 1][ ];
F[ 1][ ];
F[ ][ 2];
F[ ][ 2];
F[ ][ ];
F[ ][ ];'
		ERROR - $'cpp: line 2: warning: f: 2 actual arguments expected
cpp: line 3: warning: f: 2 actual arguments expected
cpp: line 8: warning: f: 2 actual arguments expected
cpp: line 9: warning: f: 2 actual arguments expected
cpp: line 10: warning: f: 2 actual arguments expected
cpp: line 11: warning: f: 2 actual arguments expected
cpp: line 12: warning: f: 2 actual arguments expected
cpp: line 13: warning: f: 2 actual arguments expected
cpp: line 14: warning: f: 2 actual arguments expected
cpp: line 15: warning: f: 2 actual arguments expected'

TEST 24 '#(define|undef) extern proto intercepts'

	EXEC -I-D -D-C -D__EXPORT__='__declspec(dllexport)'
		INPUT - $'#pragma prototyped
#define extern __EXPORT__
extern void fun(int);
#undef extern
extern void fun(int arg) { return arg; }'
		OUTPUT - $'# 1 ""

# 2

# 2

extern  __declspec(dllexport) int fun ();
# 4

extern  int fun (arg) int arg;
# 5
{ return arg; }'

	EXEC -I-D -D__cplusplus -D__EXPORT__='__declspec(dllexport)'
		OUTPUT - $'# 1 ""\n\n# 2\n\n# 2\n
extern "C" __declspec(dllexport) void fun (int) ;
# 4\n\nextern "C" void fun (int arg)  \n# 5\n{ return arg; }'

	EXEC -I-D -D-C -D__IMPORT__='__declspec(dllimport)'
		INPUT - $'#pragma prototyped
#define extern extern __IMPORT__
extern void fun(int);
#undef extern
extern void fun(int arg) { return arg; }'
		OUTPUT - $'# 1 ""

# 2

# 2

extern  __declspec(dllimport) int fun ();
# 4

extern  int fun (arg) int arg;
# 5
{ return arg; }'

	EXEC -I-D -D__cplusplus -D__IMPORT__='__declspec(dllimport)'
		OUTPUT - $'# 1 ""\n\n# 2\n\n# 2\n
extern "C" __declspec(dllimport) void fun (int) ;
# 4\n\nextern "C" void fun (int arg)  \n# 5\n{ return arg; }'

TEST 25 '#include next'

	EXEC -I-D -Itop -Imid -Ibot all.c
		INPUT all.c $'extern int beg_src;
#include <flt.h>
#include <mth.h>
extern int end_src;'
		INPUT top/flt.h $'extern int beg_top_flt;
extern int end_top_flt;'
		INPUT mid/flt.h $'#ifndef _MID_FLT_H
extern int beg_mid_flt;
#include <.../flt.h>
extern int end_mid_flt;
#ifndef _MID_FLT_H
#define _MID_FLT_H
#endif
#endif'
		INPUT mid/mth.h $'#ifndef _MID_MTH_H
extern int beg_mid_mth;
#include <.../mth.h>
#include <.../flt.h>
extern int end_mid_mth;
#ifndef _MID_MTH_H
#define _MID_MTH_H
#endif
#endif'
		INPUT bot/flt.h $'#ifndef _BOT_FLT_H
#define _BOT_FLT_H
extern int beg_bot_flt;
extern int end_bot_flt;
#endif'
		INPUT bot/mth.h $'#ifndef _BOT_MTH_H
#define _BOT_MTH_H
extern int beg_bot_mth;
extern int end_bot_mth;
#endif'
		OUTPUT - $'# 1 "all.c"
extern int beg_src;
# 1 "top/flt.h"
extern int beg_top_flt;
extern int end_top_flt;
# 3 "all.c"

# 1 "mid/mth.h"

extern int beg_mid_mth;
# 1 "bot/mth.h"


extern int beg_bot_mth;
extern int end_bot_mth;
# 4 "mid/mth.h"

# 1 "mid/flt.h"

extern int beg_mid_flt;
# 1 "bot/flt.h"


extern int beg_bot_flt;
extern int end_bot_flt;
# 4 "mid/flt.h"
extern int end_mid_flt;
# 5 "mid/mth.h"
extern int end_mid_mth;
# 4 "all.c"
extern int end_src;'

	EXEC -I-D -Itop -Imid -Ibot
		INPUT - $'#include <flt.h>'
		OUTPUT - $'# 1 ""

# 1 "top/flt.h"
extern int beg_top_flt;
extern int end_top_flt;
# 2 ""'

	EXEC -I-D -Itop -Imid -Ibot
		INPUT - $'#include <.../flt.h>'
		OUTPUT - $'# 1 ""

# 1 "mid/flt.h"

extern int beg_mid_flt;
# 1 "bot/flt.h"


extern int beg_bot_flt;
extern int end_bot_flt;
# 4 "mid/flt.h"
extern int end_mid_flt;
# 2 ""'

	EXEC -I-D -Itop -Imid -Ibot
		INPUT - $'#include <.../.../flt.h>'
		OUTPUT - $'# 1 ""

# 1 "bot/flt.h"


extern int beg_bot_flt;
extern int end_bot_flt;
# 2 ""'

	EXEC -I-D -Itop -Imid -Ibot
		INPUT - $'#include <aha.h>'
		INPUT top/aha.h $'beg_top_aha
#include <...>
end_top_aha'
		INPUT mid/aha.h $'beg_mid_aha
#include <...>
end_mid_aha'
		INPUT bot/aha.h $'beg_bot_aha
end_bot_aha'
		OUTPUT - $'# 1 ""

# 1 "top/aha.h"
beg_top_aha
# 1 "mid/aha.h"
beg_mid_aha
# 1 "bot/aha.h"
beg_bot_aha
end_bot_aha
# 3 "mid/aha.h"
end_mid_aha
# 3 "top/aha.h"
end_top_aha
# 2 ""'

	EXEC -I-D -Itop -Imid -Ibot
		INPUT - $'#include <aha.h>'
		INPUT top/aha.h $'beg_top_aha
#include <.../...>
end_top_aha'
		OUTPUT - $'# 1 ""

# 1 "top/aha.h"
beg_top_aha
# 1 "bot/aha.h"
beg_bot_aha
end_bot_aha
# 3 "top/aha.h"
end_top_aha
# 2 ""'

TEST 26 '#pragma pp:pragmaflags'

	EXEC -I-D
		INPUT - $'#pragma system_header'
		OUTPUT - $'# 1 ""
#pragma system_header'

	EXEC -I-D
		INPUT - $'#pragma pp:pragmaflags system_header
#pragma system_header'
		OUTPUT - $'# 1 ""

#pragma system_header'

	EXEC -I-D
		INPUT - $'#pragma pp:pragmaflags -system_header
#pragma system_header'
		OUTPUT - $'# 1 ""'

TEST 27 'misc pragmas'

	EXEC -I-D
		INPUT - $'#pragma pp:note abc'
		OUTPUT - $'# 1 ""'

TEST 28 'line syncs'

	EXEC -I-D
		INPUT - $'__LINE__
#pragma pass it on
__LINE__
#define a b
__LINE__
#define b c
__LINE__'
		OUTPUT - $'# 1 ""
1
#pragma pass it on
3

5

7'

	EXEC -I-D
		INPUT - $'#pragma foo bar
2 : __LINE__ : "a" : a ;
#pragma bar foo
4 : __LINE__ : a ;'
		OUTPUT - $'# 1 ""
#pragma foo bar
2 : 2 : "a": a ;
#pragma bar foo
4 : 4 : a ;'

	EXEC -I-D
		INPUT - $'1 : __LINE__
#pragma foo bar
3 : __LINE__ : "a" : a ;
#pragma bar foo
5 : __LINE__ : a ;'
		OUTPUT - $'# 1 ""
1 : 1
#pragma foo bar
3 : 3 : "a": a ;
#pragma bar foo
5 : 5 : a ;'

	EXEC -I-D
		INPUT - $'#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
32 : __LINE__'
		OUTPUT - $'# 1 ""\n\n# 32\n32 : 32'

	EXEC -I-D
		INPUT - $'#if 0
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#endif
32 : __LINE__'

	EXEC -I-D
		INPUT - $'#if 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#endif
32 : __LINE__'

	EXEC -I-D
		INPUT - $'\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n32 : __LINE__'

	EXEC -I-D
		INPUT - $'/*efine a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a*/
32 : __LINE__'
		OUTPUT - $'# 1 ""\n \n# 32\n32 : 32'

	EXEC -I-D
		INPUT - $'#pragma pass it on
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
32 : __LINE__'
		OUTPUT - $'# 1 ""\n#pragma pass it on\n# 32\n32 : 32'

	EXEC -I-D
		INPUT - $'#pragma pass it on
#if 0
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#endif
32 : __LINE__'

	EXEC -I-D
		INPUT - $'#pragma pass it on
#if 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#endif
32 : __LINE__'

	EXEC -I-D
		INPUT - $'#pragma pass it on\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n32 : __LINE__'

	EXEC -I-D
		INPUT - $'#pragma pass it on
/*efine a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a*/
32 : __LINE__'
		OUTPUT - $'# 1 ""\n#pragma pass it on\n \n# 32\n32 : 32'

	EXEC -I-D
		INPUT - $'1 : __LINE__
#pragma pass it on
3 : __LINE__
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
32 : __LINE__'
		OUTPUT - $'# 1 ""\n1 : 1\n#pragma pass it on\n3 : 3\n# 32\n32 : 32'

	EXEC -I-D
		INPUT - $'1 : __LINE__
#pragma pass it on
3 : __LINE__
#if 0
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#endif
32 : __LINE__'

	EXEC -I-D
		INPUT - $'1 : __LINE__
#pragma pass it on
3 : __LINE__
#if 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#endif
32 : __LINE__'

	EXEC -I-D
		INPUT - $'1 : __LINE__
#pragma pass it on\n3 : __LINE__\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n32 : __LINE__'

	EXEC -I-D
		INPUT - $'1 : __LINE__
#pragma pass it on
3 : __LINE__
/*efine a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a 1
#define a*/
32 : __LINE__'
		OUTPUT - $'# 1 ""\n1 : 1\n#pragma pass it on\n3 : 3\n \n# 32\n32 : 32'

	EXEC -I-D -DN
		INPUT - $'1 : __LINE__
#if N
3 : __LINE__
#else
5 : __LINE__
6 : __LINE__
#endif
8 : __LINE__
#if !N
10 : __LINE__
#else
12 : __LINE__
13 : __LINE__
#endif
15 : __LINE__'
		OUTPUT - $'# 1 ""
1 : 1

3 : 3




8 : 8



12 : 12
13 : 13

15 : 15'
	EXEC -I-D
		OUTPUT - $'# 1 ""
1 : 1



5 : 5
6 : 6

8 : 8

10 : 10




15 : 15'

TEST 29 'more hidden lines'

	EXEC -I-D
		INPUT - $'a\nb\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\ny\nz'
		OUTPUT - $'# 1 ""
a
b
# 31
y
z'

	EXEC -I-D
		INPUT - $'aa\nbb\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nyy\nzz'
		OUTPUT - $'# 1 ""
aa
bb
# 31
yy
zz'

	EXEC -I-D
		INPUT - $'int fun(void)\na\nb\n\treturn 0\nc\nd'
		OUTPUT - $'# 1 ""\nint fun(void)\na\nb\n\treturn 0\nc\nd'

	EXEC -I-D
		INPUT - $'a\n\na\n\n\na\n\n\n\na\n\n\n\n\na\n\n\n\n\n\na\n\n\n\n\n\n\na\n\n\n\n\n\n\n\na\n\n\n\n\n\n\n\n\na\n\n\n\n\n\n\n\n\n\na\n\n\n\n\n\n\n\n\n\n\na\n\n\n\n\n\n\n\n\n\n\n\na\n\n\n\n\n\n\n\n\n\n\n\n\na'
		OUTPUT - $'# 1 ""\na\n\na\n\n\na\n\n\n\na\n\n\n\n\na\n\n\n\n\n\na\n\n\n\n\n\n\na\n\n\n\n\n\n\n\na\n# 45\na\n# 55\na\n# 66\na\n# 78\na\n# 91\na'

	EXEC -I-D
		INPUT - $'aa\n\naa\n\n\naa\n\n\n\naa\n\n\n\n\naa\n\n\n\n\n\naa\n\n\n\n\n\n\naa\n\n\n\n\n\n\n\naa\n\n\n\n\n\n\n\n\naa\n\n\n\n\n\n\n\n\n\naa\n\n\n\n\n\n\n\n\n\n\naa\n\n\n\n\n\n\n\n\n\n\n\naa\n\n\n\n\n\n\n\n\n\n\n\n\naa'
		OUTPUT - $'# 1 ""\naa\n\naa\n\n\naa\n\n\n\naa\n\n\n\n\naa\n\n\n\n\n\naa\n\n\n\n\n\n\naa\n\n\n\n\n\n\n\naa\n# 45\naa\n# 55\naa\n# 66\naa\n# 78\naa\n# 91\naa'

	EXEC -I-D
		INPUT - $'1\n\n3\n\n\n6\n\n\n\n10\n\n\n\n\n15\n\n\n\n\n\n21\n\n\n\n\n\n\n28\n\n\n\n\n\n\n\n36\n\n\n\n\n\n\n\n\n45\n\n\n\n\n\n\n\n\n\n55\n\n\n\n\n\n\n\n\n\n\n66\n\n\n\n\n\n\n\n\n\n\n\n78\n\n\n\n\n\n\n\n\n\n\n\n\n91'
		OUTPUT - $'# 1 ""\n1\n\n3\n\n\n6\n\n\n\n10\n\n\n\n\n15\n\n\n\n\n\n21\n\n\n\n\n\n\n28\n\n\n\n\n\n\n\n36\n# 45\n45\n# 55\n55\n# 66\n66\n# 78\n78\n# 91\n91'

	EXEC -I-D
		INPUT - $'1 __LINE__\n\n3 __LINE__\n\n\n6 __LINE__\n\n\n\n10 __LINE__\n\n\n\n\n15 __LINE__\n\n\n\n\n\n21 __LINE__\n\n\n\n\n\n\n28 __LINE__\n\n\n\n\n\n\n\n36 __LINE__\n\n\n\n\n\n\n\n\n45 __LINE__\n\n\n\n\n\n\n\n\n\n55 __LINE__\n\n\n\n\n\n\n\n\n\n\n66 __LINE__\n\n\n\n\n\n\n\n\n\n\n\n78 __LINE__\n\n\n\n\n\n\n\n\n\n\n\n\n91 __LINE__'
		OUTPUT - $'# 1 ""\n1 1\n\n3 3\n\n\n6 6\n\n\n\n10 10\n\n\n\n\n15 15\n\n\n\n\n\n21 21\n\n\n\n\n\n\n28 28\n\n\n\n\n\n\n\n36 36\n# 45\n45 45\n# 55\n55 55\n# 66\n66 66\n# 78\n78 78\n# 91\n91 91'

TEST 30 'weird splices'

	EXEC -I-D
		INPUT - $'1 __LINE__ \\\n\n3 __LINE__'
		OUTPUT - $'# 1 ""\n1 1 \n\n3 3'

	EXEC -I-D
		INPUT - $'1 __LINE__\\\n\n3 __LINE__'
		OUTPUT - $'# 1 ""\n1 1\n\n3 3'

	EXEC -I-D
		INPUT - $'a\n\n@'
		OUTPUT - $'# 1 ""\na\n\n@'

	EXEC -I-D
		INPUT - $'a\n\n\\a'
		OUTPUT - $'# 1 ""\na\n\n\\a'

TEST 31 'convoluted function-like macros'

	EXEC -I-D
		INPUT - $'#define A(n,t)	B(n,t)
#define B(s,t)	C##s t
#define C2(x,y)	(x,(y,NIL))
A(2,(a,b))'
		OUTPUT - $'# 1 ""\n\n\n\n( a,( b,NIL))'

TEST 32 'catliteral vs stringize'

	EXEC -I-D
		INPUT - $'#define F(s)	"a" "b" #s
F(c);'
		OUTPUT - $'# 1 ""\n\n"abc";'

TEST 33 'passthrough vs comment threat'

	EXEC -I-D
		INPUT - $'#ifdef UseInstalled
        IMAKE_CMD = $(IMAKE) -DUseInstalled -I$(IRULESRC) $(IMAKE_DEFINES) \\
                $(IMAKE_WARNINGS)
#else
         IRULESRC = $(CONFIGSRC)/cf
        IMAKE_CMD = $(IMAKE) -I$(IRULESRC) $(IMAKE_DEFINES) $(IMAKE_WARNINGS)
#endif'
		OUTPUT - $'# 1 ""

         IRULESRC = $(CONFIGSRC)


/cf
        IMAKE_CMD = $(IMAKE) -I$(IRULESRC) $(IMAKE_DEFINES) $(IMAKE_WARNINGS)'

	EXEC -D-P -D-L
		OUTPUT - $'#line 1 ""

         IRULESRC = $(CONFIGSRC)/cf
        IMAKE_CMD = $(IMAKE) -I$(IRULESRC) $(IMAKE_DEFINES) $(IMAKE_WARNINGS)


'

TEST 34 'more passthrough ala rpcgen'

	EXEC -D-P -D-L
		INPUT - $'/* comment */
%#include <line/two>
int line = 3;'
		OUTPUT - $'#line 1 ""

%#include <line/two>
int line = 3;'

	EXEC -D-P -D-L -C
		OUTPUT - $'#line 1 ""
/* comment */
%#include <line/two>
int line = 3;'

	EXEC -D-P -D-L
		INPUT - $'/*
 * hidden
 */
%#include <line/four>
int line = 5;'
		OUTPUT - $'#line 1 ""
 


%#include <line/four>
int line = 5;'

	EXEC -D-P -D-L -C
		OUTPUT - $'#line 1 ""
/*
 * hidden
 */
%#include <line/four>
int line = 5;'

	EXEC -D-P -D-L
		INPUT - $'#ifndef YADAYADA
%#include <sys/seinfeld.h>
%#ifndef lint
%/*static char sccsid[] = "george";*/
%/*static char sccsid[] = "elaine";*/
%__RCSID("kramer");
%#endif /* not lint */
#endif'
		OUTPUT - $'#line 1 ""
%#include <sys/seinfeld.h>

%#ifndef lint
%
%
%__RCSID("kramer");
%#endif '

	EXEC -D-P -D-L -C
		OUTPUT - $'#line 1 ""
%#include <sys/seinfeld.h>

%#ifndef lint
%/*static char sccsid[] = "george";*/
%/*static char sccsid[] = "elaine";*/
%__RCSID("kramer");
%#endif /* not lint */'

TEST 35 'externalize'

	EXEC -I-D
		INPUT - $'#pragma prototyped
static int fun(int);
static int beg;
static int fun(int arg) { return !arg; }
static int end;'
		OUTPUT - $'# 1 ""

static int fun(int);
static int beg;
static int fun(int arg) { return !arg; }
static int end;'

	EXEC -I-D -D:externalize
		OUTPUT - $'# 1 ""

# 2
extern int fun (int);
static int beg;
extern int fun (int arg) 
# 4
{ return !arg; }
static int end;'

	EXEC -I-D -D:compatibility -D:externalize
		OUTPUT - $'# 1 ""

# 2
extern int fun ();
static int beg;
extern int fun (arg) int arg;
# 4
{ return !arg; }
static int end;'

TEST 36 'UL qualifier compatibility'

	EXEC -I-D
		INPUT - $'#define UL 1234UL
int i = 1234;
unsigned int j = 1234U;
unsigned int k = 1234L;
unsigned int l = 1234UL;
unsigned int m = UL;'
		OUTPUT - $'# 1 ""

int i = 1234;
unsigned int j = 1234U;
unsigned int k = 1234L;
unsigned int l = 1234UL;
unsigned int m = 1234UL;'

	EXEC -I-D -D-C
		OUTPUT - $'# 1 ""

int i = 1234;
unsigned int j = 1234;
unsigned int k = 1234L;
unsigned int l = 1234L;
unsigned int m = 1234L;'

	EXEC -I-D -D:compatibility

TEST 37 'a convoluted one from msvc 2008 express <sal.h> -- is all that maintained by hand?'

	EXEC -I-D
		INPUT - $'#define a(x) b(c(x))
#define b(x) x
#define c(x) =#x
a(X)'
		OUTPUT - $'# 1 ""



="X"'

TEST 38 'multiple include'

	EXEC -I-D
		INPUT - $'#pragma pp:map "/#pragma once/" ",#pragma once,#pragma pp:nomultiple,"
int b;
#include "m.h"
#include "n.h"
#include "o.h"
#include "m.h"
#include "n.h"
#include "o.h"
#include "m.h"
int e;'
		INPUT m.h $'extern int m;'
		INPUT n.h $'#pragma pp:nomultiple
int n;'
		INPUT o.h $'#pragma once
int o;'
		OUTPUT - $'# 1 ""

int b;
# 1 "m.h"
extern int m;
# 4 ""

# 1 "n.h"

int n;
# 5 ""

# 1 "o.h"
int o;
# 6 ""

# 1 "m.h"
extern int m;
# 7 ""

# 1 "m.h"
extern int m;
# 10 ""
int e;'

	EXEC -I-D -D:noallmultiple
		OUTPUT - $'# 1 ""

int b;
# 1 "m.h"
extern int m;
# 4 ""

# 1 "n.h"

int n;
# 5 ""

# 1 "o.h"
int o;
# 6 ""




int e;'

	EXEC -I-D
		INPUT - $'#include "h1.h"
#include "h1.h"'
		INPUT h1.h $'#include "h2.h"
#ifndef _H1_H
#define _H1_H
int h1_once;
#endif'
		INPUT h2.h $'extern int h2_multiple;
#define _H2_H'
		OUTPUT - $'# 1 ""

# 1 "h1.h"

# 1 "h2.h"
extern int h2_multiple;
# 2 "h1.h"


int h1_once;
# 2 ""

# 1 "h1.h"

# 1 "h2.h"
extern int h2_multiple;
# 2 "h1.h"

# 3 ""'

TEST 39 'macro definition overwrite -- I know'

	EXEC -I-D
		INPUT - $'#define FOO(x)	BAR x
#define BAR(x)
FOO(ONE)
FOO(TWO)
FOO(TEN)'
		OUTPUT - $'# 1 ""


BAR ONE
BAR TWO
BAR TEN'

TEST 40 'implicit prefix include with -I-'

	DO	DATA pfx

	EXEC -I-D -Istd t.c
		INPUT t.c '#include <a.h>'
		OUTPUT - '# 1 "t.c"

# 1 "std/a.h"

# 1 "std/pfx/c.h"

# 1 "std/pfx/d.h"
int d = 0;
# 2 "std/pfx/c.h"

# 2 "std/a.h"

# 2 "t.c"'

	EXEC -I-D -I- -Istd t.c
