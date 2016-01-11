# regression tests for the dss flat method

TITLE + flat

export TZ=EST5EDT

VIEW data

TEST 01 'flat simple types'

	EXEC -I $data -x pwd-txt 'password==""&&uid==0' $data/pwd-txt.dat
		OUTPUT - $'aha::0:0::/:/bin/sh'

	EXEC -I $data -x pwd-txt 'password==""' $data/pwd-txt.dat
		OUTPUT - $'aha::0:0::/:/bin/sh
he::30002:30002:Hammond Egger:/home/he:/bin/ksh'

	EXEC -I $data -x pwd-txt 'shell=="/bin/ksh"' $data/pwd-txt.dat
		OUTPUT - $'as:*:30001:30001:Alan Smithee:/home/as:/bin/ksh
he::30002:30002:Hammond Egger:/home/he:/bin/ksh'

	EXEC -I $data -x pwd-txt '(shell=="/bin/ksh")|{print "%(dss.record)u %(uid)u"}' $data/pwd-txt.dat
		OUTPUT - $'15 30001
16 30002'

	EXEC -c -I $data -x pwd-txt 'shell=~".*/ksh"' $data/pwd-txt.dat
		OUTPUT - $'2/18'

	EXEC -c -I $data -x pwd-txt 'shell!~".*/ksh"' $data/pwd-txt.dat
		OUTPUT - $'16/18'

TEST 02 'flat with field quotes'

	EXEC -I $data -x httplog -p '%(time)s %(addr)s' 'addr=~"192.31/16"' $data/httplog.dat
		OUTPUT - $'1997-09-30+03:00:00 192.31.231.63
1997-09-30+03:00:01 192.31.231.63
1997-09-30+03:00:04 192.31.231.63
1997-09-30+03:00:08 192.31.231.63'

TEST 03 'flat fixed width field with record terminator'

	EXEC -I $data -x fixed - $data/fixed.dat
		OUTPUT - $'aabbccdd
eeffgghh
iijjkkll
mmnnoopp'

	EXEC -I $data -x fixed '{print "<%(s1)s> <%(s2)s> <%(s3)s> <%(s4)s>"}' $data/fixed.dat
		OUTPUT - $'<aa> <bb> <cc> <dd>
<ee> <ff> <gg> <hh>
<ii> <jj> <kk> <ll>
<mm> <nn> <oo> <pp>'

	EXEC -I $data -x variable -
		INPUT -n - $'a:b2:c23:d234:e2345:f23456:g234567:h2345678:i23456789:j2345678:k234567:l23456:m2345:n234:o23:p2:q:'
		SAME OUTPUT INPUT

	EXEC -I $data -x variable '{print "<%(s1)s> <%(s2)s> <%(s3)s> <%(s4)s>"}'
		OUTPUT - '<a> <> <> <>
<b2> <> <> <>
<c2> <3> <> <>
<d2> <34> <> <>
<e2> <34> <5> <>
<f2> <34> <56> <>
<g2> <34> <56> <7>
<h2> <34> <56> <78>
<i2> <34> <56> <78>
<j2> <34> <56> <78>
<k2> <34> <56> <7>
<l2> <34> <56> <>
<m2> <34> <5> <>
<n2> <34> <> <>
<o2> <3> <> <>
<p2> <> <> <>
<q> <> <> <>'

	EXEC -I $data -x fixed -
		SAME INPUT $data/fixed-bad.dat
		OUTPUT - $'aabbccdd
eeff
gghhi
jjkkll
mmnnoop
qqrrsstt
uuvvwwxxy
zzaabbccdd
eeffgghhiij
kkllmmnnoopp
wwxxyyzz'

	EXEC -I $data -x variable '{print "<%(s1)s> <%(s2)s> <%(s3)s> <%(s4)s>"}'
		INPUT -n - $'abcdefghijklmnop'
		OUTPUT - '<ab> <cd> <ef> <gh>
<ij> <kl> <mn> <op>'
		ERROR - $'dss::scan: warning: stdin, record 1, offset 0: flat record terminator ignored'

TEST 04 'flat conversion'

	EXEC -I $data -x pwd-txt '{flatten pwd-bin}' $data/pwd-txt.dat
		SAME OUTPUT $data/pwd-bin.dat
		ERROR - $'dss::flatten: warning: pad: field not in source record -- default value will be output'

	EXEC -I $data -x pwd-bin '{flatten pwd-txt}' $data/pwd-bin.dat
		SAME OUTPUT $data/pwd-txt.dat
		ERROR -

	EXEC -I $data -x pwd-bin '{print "%(name)s %(uid)d"}' $data/pwd-bin.dat
		OUTPUT - $'root 0
sysadm 0
diag 0
daemon 1
bin 2
uucp 3
sys 4
adm 5
aha 0
lp 9
auditor 11
ftp 112
demos 993
guest 998
as 30001
he 30002
noaccess 60002
nobody 60001'

	for typ in txt bin
	do

	EXEC -p '%(name)s' -I $data -x pwd-$typ 'password==""&&uid==0' $data/pwd-$typ.dat
		OUTPUT - $'aha'

	EXEC -p '%(name)s' -I $data -x pwd-$typ 'password==""' $data/pwd-$typ.dat
		OUTPUT - $'aha\nhe'

	EXEC -p '%(name)s' -I $data -x pwd-$typ 'shell=="/bin/ksh"' $data/pwd-$typ.dat
		OUTPUT - $'as\nhe'

	EXEC -p '%(comment)s' -I $data -x pwd-$typ 'shell=~".*/ksh"' $data/pwd-$typ.dat
		OUTPUT - $'Alan Smithee\nHammond Egger'

	EXEC -p '%(comment)s' -I $data -x pwd-$typ 'shell!~".*/ksh"' $data/pwd-$typ.dat
		OUTPUT - $'Super-User
System V Administration
Hardware Diagnostics
daemons
System Tools Owner
UUCP Owner
System Activity Owner
Accounting Files Owner

Print Spooler Owner
Audit Activity Owner
File Transfer
Demonstration User
Guest Account
uid no access
original nobody uid'
	done

TEST 05 'keyed data'

	EXEC -I $data -x key-1 - /dev/null
		ERROR - "dss: \"$data/key-1.dss\", line 28: warning: id: id: field key is ambiguous -- qualification required
dss::scan: warning: ID: id: field key is ambiguous -- qualification required"

	EXEC -I $data -x key-2 - /dev/null
		ERROR - "dss::scan: warning: ID: id: field key is ambiguous -- qualification required"

	EXEC -I $data -x key-3 - /dev/null
		ERROR - "dss: \"$data/key-3.dss\", line 29: warning: id: id: field key is ambiguous -- qualification required"

TEST 06 'copybook conversion'

	EXEC -I $data -x cpy-txt '{flatten cpy-bin}' $data/cpy-txt.dat
		SAME OUTPUT $data/cpy-bin.dat

	EXEC -I $data -x cpy-bin '{flatten cpy-txt}' $data/cpy-bin.dat
		SAME OUTPUT $data/cpy-txt.dat

	EXEC -I $data -x cpy-bin '{flatten --emptyspace cpy-txt}' $data/cpy-bin.dat
		OUTPUT - $'12345678|MOE     |1|0|1|01/02/2003|03/02/2001|X|00000000|RUN  |L|1|Q|00000000|Q|        |95|00000004|00000000|Q| |01/01/1900|USD|Q|ENG|2004-05-06-07.08.09.123456|6|HOWARD
12345678|LARRY   |1|0|1|04/05/2006|06/05/2004|X|00000000|DUCK |R|2|Q|00000000|Q|        |12.1234567890123|00000005|00000000|Q| |01/01/1900|USD|Q|ENG|2004-05-06-07.08.10.789012|4|FINE
87654321|CURLY   |1|0|2|07/08/2009|09/08/2007|X|00000000|HYDE |L|3|Q|00000000|Q|        |0|00000006|00000000|Q| |01/01/1900|USD|Q|ENG|2004-05-06-07.08.11.345678|0| '

	EXEC -I $data -x cpy-str-txt '{flatten cpy-str-bin}' $data/cpy-txt.dat
		SAME OUTPUT $data/cpy-bin.dat

	EXEC -I $data -x cpy-str-bin '{flatten cpy-str-txt}' $data/cpy-bin.dat
		SAME OUTPUT $data/cpy-txt.dat

TEST 07 'fixed with variable sized but bounded fields'

	EXEC -I $data -x fixedvar '{print "%(TEST_FIELD_03)s"}' $data/fixedvar.dat
		OUTPUT - $'BBBB
DDDD
FFFF'

	EXEC -I $data -x fixedmax '{print "%(TEST_FIELD_01)s %(TEST_FIELD_03)s %(TEST_FIELD_02_SIZE)02d %(TEST_FIELD_02_DATA)s"}' $data/fixedmax.dat
		OUTPUT - $'AAAA BBBB 10 abcdefghij
CCCC DDDD 03 xyz
EEEE FFFF 00 '

	EXEC -I $data -x fixedmaxvar '{print "%(TEST_FIELD_01)s %(TEST_FIELD_03)s %(TEST_FIELD_02_SIZE)02d %(TEST_FIELD_02_DATA)s"}' $data/fixedmaxvar.dat
		OUTPUT - $'AAAA BBBB 10 abcdefghij
CCCC DDDD 03 xyz
EEEE FFFF 00 '

TEST 08 'syntax diagnostics'

	EXEC -x bad.dss
		INPUT bad.dss $'<METHOD>flat</>
<FLAT>
 <FIELD>
  <NAME>A</>
  <FIELD>
   <NAME>B</>
   <FIELD>
    <NAME>C</>
    <TYPE>string>
    <PHYSICAL>
     <WIDTH>16</>
    </>
   </>
  </>
 </>
</>'
		ERROR - $'dss: "bad.dss", line 10: <FLAT><FIELD><FIELD><FIELD><TYPE>: invalid input
dss: bad.dss: unknown method'
		EXIT 1

	EXEC -x bad.dss
		INPUT bad.dss $'<METHOD>flat</>
<FLAT>
 <FIELD>
 </>'
		ERROR - $'dss: "bad.dss", line 4: field name must be specified
dss: bad.dss: unknown method'

	EXEC -x bad.dss
		INPUT bad.dss $'<METHOD>flat</>
<FLAT>
 <FIELD>
  <NAME>A</>
 </>'
		ERROR - $'dss: "bad.dss", line 6: <FLAT>: no closing tag
dss: bad.dss: unknown method'

TEST 09 'schema info options'

	EXEC -I $data -x flat,header:fixedvar -
		OUTPUT - $'/*
 * TEST dynamic interface
 * converted by cpy2dss from copybook standard input
 */

#define TEST_RECORD(data)	(_TEST_record_=(TEST_record_t*)DSSDATA(data))

typedef Cxvalue_t* (*TEST_get_f)(void*,int);

typedef struct TEST_field_s		/* record field			*/
{
	Cxvalue_t	value;		/* value (first for dynamic Q)	*/
	void*		field;		/* static field info		*/
	size_t		off;		/* record data offset		*/
	size_t		siz;		/* record data size		*/
	unsigned int	serial;		/* read serial number		*/
	unsigned int	keyed;		/* keyed serial number		*/
} TEST_field_t;

typedef struct TEST_record_s		/* current record info		*/
{
	TEST_field_t*	fields;		/* fields (first for dynamic Q)	*/
	TEST_get_f	getf;		/* getf (second for dynamic Q)	*/
} TEST_record_t;

static TEST_record_t*	_TEST_record_;

#define TEST_TEST_FIELD_01	((*_TEST_record_->getf)(_TEST_record_,0)->string.data)
#define TEST_TEST_FIELD_01_size	((*_TEST_record_->getf)(_TEST_record_,0)->string.size)
#define TEST_TEST_FIELD_02_SIZE	((*_TEST_record_->getf)(_TEST_record_,2)->number)
#define TEST_TEST_FIELD_02_DATA	((*_TEST_record_->getf)(_TEST_record_,3)->string.data)
#define TEST_TEST_FIELD_02_DATA_size	((*_TEST_record_->getf)(_TEST_record_,3)->string.size)
#define TEST_TEST_FIELD_03	((*_TEST_record_->getf)(_TEST_record_,4)->string.data)
#define TEST_TEST_FIELD_03_size	((*_TEST_record_->getf)(_TEST_record_,4)->string.size)'

	EXEC -I $data -x flat,prototype:fixedvar -
		OUTPUT - $'TEST_FIELD_01||TEST_FIELD_02_SIZE|TEST_FIELD_02_DATA|TEST_FIELD_03'

	EXEC -I $data -x flat,struct:fixedmax -
		OUTPUT - $'/* converted by cpy2dss from copybook standard input */
struct _flat_TEST_s
{
/*01*/char		TEST_FIELD_01[4];
struct
{
/*02*/char		TEST_FIELD_02_SIZE[2];
/*02*/char		TEST_FIELD_02_DATA[1024];
} TEST_FIELD_02;
/*01*/char		TEST_FIELD_03[4];
};
/* sizeof(struct _flat_TEST_s)==1034 */'

	EXEC -I $data -x flat,struct:fixedmaxvar -
		OUTPUT - $'/* converted by cpy2dss from copybook standard input */
struct _flat_TEST_s
{
/*01*/char		TEST_FIELD_01[4];
struct
{
/*02*/char		TEST_FIELD_02_SIZE[2];
/*02*/char		TEST_FIELD_02_DATA[16];
} TEST_FIELD_02;
/*01*/char		TEST_FIELD_03[4];
char			_delimiter_1;	/* delimiter \'\\n\' */
};
/* sizeof(struct _flat_TEST_s)==27 */'

TEST 10 'quote + escape vs. delimiters and terminators'

	EXEC -I $data -x qe-bin '{flatten q-txt.dss}' qe-bin.dat
		OUTPUT - $'AB"|"D|wxyz
a"|"cd|HI""""K
LM\\O|p""""""s
""""""""""|\\\\\\\\'
		COPY OUTPUT qe.dat

	EXEC -I $data -x q-txt '{flatten q-txt.dss}' qe.dat

	EXEC -I $data -x qe-bin '{flatten qa-txt.dss}' qe-bin.dat
		OUTPUT - $'"AB|D"|"wxyz"
"a|cd"|"HI""K"
"LM\\O"|"p""""s"
""""""""""|"\\\\\\\\"'
		COPY OUTPUT qe.dat

	EXEC -I $data -x qa-txt '{flatten qa-txt.dss}' qe.dat

	EXEC -I $data -x qe-bin '{flatten e-txt.dss}' qe-bin.dat
		OUTPUT - $'AB\\|D|wxyz
a\\|cd|HI"K
LM\\\\O|p""s
""""|\\\\\\\\\\\\\\\\'
		COPY OUTPUT qe.dat

	EXEC -I $data -x e-txt '{flatten e-txt.dss}' qe.dat

	EXEC -I $data -x qe-bin '{flatten qe-txt.dss}' qe-bin.dat
		OUTPUT - $'AB\\|D|wxyz
a\\|cd|HI\\"K
LM\\\\O|p\\"\\"s
\\"\\"\\"\\"|\\\\\\\\\\\\\\\\'
		COPY OUTPUT qe.dat

	EXEC -I $data -x qe-txt '{flatten qe-txt.dss}' qe.dat

	EXEC -I $data -x ff-bin '{flatten --emptyspace ff-txt.dss}' ff-bin.dat
		OUTPUT - $'"00"|"\xff*"
"01"|"\xff*"
"02"|"\xff*"
"03"|"\xff*"
"04"|"\xff*"
"05"|"\xff*"
"06"|"\xff*"
"07"|"\xff*"
"08"|"\xff*"
"09"|"\xff*"
"10"|"\xff*"
"11"|"\xff*"
"12"|"\xff*"
"13"|"\xff*"
"14"|"\xff*"
"15"|"\xff*"
"16"|"\xff*"
"17"|"\xff*"
"18"|"\xff*"
"19"|"\xff*"
"20"|"\xff*"
"21"|"\xff*"
"22"|"\xff*"
"23"|"\xff*"
"24"|"\xff*"
"25"|"\xff*"
"26"|"\xff*"
"27"|"\xff*"
"28"|"\xff*"
"29"|"\xff*"
"30"|"\xff*"
"31"|"\xff*"
"32"|"\xff*"
"33"|"\xff*"
"34"|"\xff*"
"35"|"\xff*"
"36"|"\xff*"
"37"|"\xff*"
"38"|"\xff*"
"39"|"\xff*"
"40"|"\xff*"
"41"|"\xff*"
"42"|"\xff*"
"43"|"\xff*"
"44"|"\xff*"
"45"|"\xff*"
"46"|"\xff*"
"47"|"\xff*"
"48"|"\xff*"
"49"|"\xff*"
"50"|"\xff*"
"51"|"\xff*"
"52"|"\xff*"
"53"|"\xff*"
"54"|"\xff*"
"55"|"\xff*"
"56"|"\xff*"
"57"|"\xff*"
"58"|"\xff*"
"59"|"\xff*"
"60"|"\xff*"
"61"|"\xff*"
"62"|"\xff*"
"63"|"\xff*"
"64"|"\xff*"
"65"|"\xff*"
"66"|"\xff*"
"67"|"\xff*"
"68"|"\xff*"
"69"|"\xff*"
"70"|"\xff*"
"71"|"\xff*"
"72"|"\xff*"
"73"|"\xff*"
"74"|"\xff*"
"75"|"\xff*"
"76"|"\xff*"
"77"|"\xff*"
"78"|"\xff*"
"79"|"\xff*"
"80"|"\xff*"
"81"|"\xff*"
"82"|"\xff*"
"83"|"\xff*"
"84"|"\xff*"
"85"|"\xff*"
"86"|"\xff*"
"87"|"\xff*"
"88"|"\xff*"
"89"|"\xff*"
"90"|"\xff*"
"91"|"\xff*"
"92"|"\xff*"
"93"|"\xff*"
"94"|"\xff*"
"95"|"\xff*"
"96"|"\xff*"
"97"|"\xff*"
"98"|"\xff*"
"99"|"\xff*"'

TEST 11 'file suffix conflicts'

	EXEC -I $data -x suffix-txt 'password==""&&uid==0' ./suffix-txt
		INPUT suffix-txt $'aha::0:0::/:/bin/sh'
		OUTPUT - $'aha::0:0::/:/bin/sh'

	EXEC -I $data -x suffix-txt 'password==""&&uid==0' suffix-txt
		INPUT suffix-txt $'aha::0:0::/:/bin/sh'
		OUTPUT - $'aha::0:0::/:/bin/sh'
