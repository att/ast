# regression tests for the tr utilitiy

KEEP "*.dat"

export LC_ALL=C

function DATA
{
	typeset f
	integer i
	typeset -i8 n
	for f
	do	test -f $f && continue
		case $f in
		chars.dat)
			typeset -i8 o
			for ((o = 0; o < 256; o++))
			do	print -f "\\${o#8#}"
			done
			;;
		lower.dat)
			for a in '[' a b c d e f g h i j k l m n o p q r s t u v w x y z ']'
			do	print "$a"
			done
			;;
		nul.dat)
			print -- '[a\000b\007c\014d\015e\016f\n\000x\015]'
			;;
		nul1.dat)
			print -- '[a\000b\007c\014de\016f\n\000x]'
			;;
		nul2.dat)
			print -- '[ab\007c\014de\016f\nx]'
			;;
		nul3.dat)
			print -- '[a\nb\007c\fd\ne\016f\n\nx\n]'
			;;
		nul4.dat)
			print -- '[a\nb\nc\nd\ne\016f\n\nx\n]'
			;;
		nul5.dat)
			print -- '\0bc123'
			;;
		upper.dat)
			for a in '[' A B C D E F G H I J K L M N O P Q R S T U V W X Y Z ']'
			do	print "$a"
			done
			;;
		zero.dat)
			for ((n = 0; n < 256; n++))
			do	print -f '\0'
			done
			;;
		esac > $f
	done
}

TEST 01 'simple translation'
	EXEC	aaa xyz
		INPUT - $'abcxyz'
		OUTPUT - $'zbcxyz'
	EXEC	'a-a' 'z'
	EXEC	'a-b' 'z'
		OUTPUT - $'zzcxyz'
	EXEC	'abc' 'AB-'
		OUTPUT - $'AB-xyz'
	EXEC	'ab-' 'ABC'
		OUTPUT - $'ABcxyz'
	EXEC	'ab-' 'ABC'
		INPUT - $'ab-xyz'
		OUTPUT - $'ABCxyz'
	EXEC	'[a-a]' 'z'
		INPUT - $'abcxyz'
		OUTPUT - $'zbcxyz'
	EXEC	'[a-b]' 'z'
		OUTPUT - $'zzcxyz'
	EXEC	'[abc]' '[AB-]'
		OUTPUT - $'ABBxyz'
	EXEC	AAA XYZ
		INPUT - $'ABCXYZ'
		OUTPUT - $'ZBCXYZ'
	EXEC	'A-A' 'Z'
	EXEC	'A-B' 'Z'
		OUTPUT - $'ZZCXYZ'
	EXEC	'ABC' 'ab-'
		OUTPUT - $'ab-XYZ'
	EXEC	'AB-' 'abc'
		OUTPUT - $'abCXYZ'
	EXEC	'AB-' 'abc'
		INPUT - $'AB-XYZ'
		OUTPUT - $'abcXYZ'
	EXEC	'[A-A]' 'Z'
		INPUT - $'ABCXYZ'
		OUTPUT - $'ZBCXYZ'
	EXEC	'[A-B]' 'Z'
		OUTPUT - $'ZZCXYZ'
	EXEC	'[ABC]' '[ab-]'
		OUTPUT - $'abbXYZ'
	EXEC	'[ab-]' '[ABC]'
		INPUT - $'abcxyz'
		OUTPUT -
		ERROR - $'tr: [ab-]: invalid source string'
		EXIT 1
	EXEC	'[ab-]' '[ABC]'
	EXEC	'[ab-]' '[AB-]'
	EXEC	'[AB-]' '[abc]'
		ERROR - $'tr: [AB-]: invalid source string'
	EXEC	'[AB-]' '[abc]'
	EXEC	'[AB-]' '[ab-]'

TEST 02 'case conversion'
	DO	DATA lower.dat upper.dat
	EXEC
		INPUT lower.dat
		SAME OUTPUT lower.dat
	EXEC
		INPUT upper.dat
		SAME OUTPUT upper.dat
	EXEC	abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ
	EXEC	a-z A-Z
	EXEC	'[a-z]' '[A-Z]'
	EXEC	'[:lower:]' '[:upper:]'
	EXEC	'[[:lower:]]' '[[:upper:]]'
	EXEC	abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ
		INPUT lower.dat
	EXEC	abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ
	EXEC	a-z A-Z
	EXEC	'[a-z]' '[A-Z]'
	EXEC	'[:lower:]' '[:upper:]'
	EXEC	'[[:lower:]]' '[[:upper:]]'
	EXEC	ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz
		SAME OUTPUT lower.dat
	EXEC	A-Z a-z
	EXEC	'[A-Z]' '[a-z]'
	EXEC	'[:upper:]' '[:lower:]'
	EXEC	'[[:upper:]]' '[[:lower:]]'
	EXEC	ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz
		INPUT upper.dat
	EXEC	A-Z a-z
	EXEC	'[A-Z]' '[a-z]'
	EXEC	'[:upper:]' '[:lower:]'
	EXEC	'[[:upper:]]' '[[:lower:]]'

TEST 03 'NUL combinations'
	DO	DATA zero.dat nul.dat nul1.dat nul2.dat nul3.dat nul4.dat
	EXEC	-d '\000'
		SAME INPUT zero.dat
		OUTPUT -
	EXEC	-d '[\015]'
		SAME INPUT nul.dat
		SAME OUTPUT nul1.dat
	EXEC	-d '[\015\000]'
		SAME OUTPUT nul2.dat
	EXEC	-d '[\000\015]'
	EXEC	'[\015\000]' '[\n]'
		SAME OUTPUT nul3.dat
	EXEC	'[\000\015]' '[\n]'
	EXEC	'\000-\015' '\n'
		SAME OUTPUT nul4.dat
	EXEC	'[\000-\015]' '[\n]'
	EXEC	'[\000-\015]' '\n'
	EXEC	'\000-\015' '[\n]'
		SAME OUTPUT nul4.dat
	EXEC	'[\000-\015]' '[\n]'

TEST 04 'squeeze'
	EXEC	-cs '[a-zA-Z0-9]' '[\n*]'
		INPUT - $'one  two\nthree-four!five\n six\rseven\t\t\n---eight---'
		OUTPUT - $'one\ntwo\nthree\nfour\nfive\nsix\nseven\neight'
	EXEC	-cs '[a-zA-Z0-9]' '\n'
	EXEC	-cs '[a-zA-Z0-9]' '[\n*]'
		INPUT - $'@@@one  two\nthree-four!five\n six\rseven\t\t\n---eight---'
		OUTPUT - $'\none\ntwo\nthree\nfour\nfive\nsix\nseven\neight'
	EXEC	-cs '[a-zA-Z0-9]' '\n'
	EXEC	-s '\n' '\n'
		INPUT - $'a\n\n\nb\nc\n\n\nd\n\ne'
		OUTPUT - $'a\nb\nc\nd\ne'
	EXEC	-s '\n'
	EXEC	-s '\n' '\n'
		INPUT - $'\n\n\na\n\n\nb\nc\n\n\nd\n\ne'
		OUTPUT - $'\na\nb\nc\nd\ne'
	EXEC	-s '\n'
	EXEC abc '[%*]xyz'
		INPUT - $'abc'
		OUTPUT - $'xyz'
	EXEC abc '[%*0]xyz'
	EXEC abc '[%*1]xyz'
		OUTPUT - $'%xy'
	EXEC abc '[%*2]xyz'
		OUTPUT - $'%%x'
	EXEC abc '[%*3]xyz'
		OUTPUT - $'%%%'
	EXEC abc '[%*4]xyz'
		OUTPUT - $'%%%'

TEST 05 'diagnostics'
	IGNORE OUTPUT ERROR
	EXEC	'[a-z'
		INPUT -
		EXIT [12]
	EXEC	'z-a'
	EXEC	'b-a'
	EXEC	'[:foo:]'

TEST 06 'gnu tr tests'
	EXEC	'abcd' '[@*]'
		INPUT - $'abcd'
		OUTPUT - $'@@@@'
	EXEC	'abcd' '[]*]'
		INPUT - $'abcd'
		OUTPUT - $']]]]'
	EXEC	'abc' '[%*]yz'
		INPUT - $'aabbcc'
		OUTPUT - $'%%yyzz'
	EXEC	'abc' '[%*]yzx'
		INPUT - $'aabbcc'
		OUTPUT - $'yyzzxx'
	EXEC	'abc' 'xy[%*]'
		INPUT - $'aabbcc'
		OUTPUT - $'xxyy%%'
	EXEC	'abc' 'xy[%*]z'
		INPUT - $'aabbcc'
		OUTPUT - $'xxyyzz'
	EXEC	'' '[.*]'
		INPUT - $'abc'
		OUTPUT - $'abc'
	EXEC	-t 'abcd' 'xy'
		INPUT - $'abcde'
		OUTPUT - $'xycde'
	EXEC	'abcd' 'xy'
		OUTPUT - $'xyyye'
	EXEC	'abcd' 'x[y*]'

TEST 07 '-s'
	EXEC	-s 'a-p' '%[.*]'
		INPUT - $'abcdefghijklmnop'
		OUTPUT - $'%.'
	EXEC	-s 'a-p' '%[.*]$'
		OUTPUT - $'%.$'
	EXEC	-s 'a-p' '[.*]$'
		OUTPUT - $'.$'
	EXEC	-s 'a-p' '%[.*]'
		OUTPUT - $'%.'
	EXEC	-s '[a-z]'
		INPUT - $'aabbcc'
		OUTPUT - $'abc'
	EXEC	-s '[a-c]'
	EXEC	-s '[a-b]'
		OUTPUT - $'abcc'
	EXEC	-s '[b-c]'
		OUTPUT - $'aabc'
	EXEC	-s '[\0-\5]'
		INPUT - $'\0\0a\1\1b\2\2\2c\3\3\3d\4\4\4\4e\5\5'
		OUTPUT - $'\0a\1b\2c\3d\4e\5'

TEST 08	'-d'
	EXEC	-d '[=[=]'
		INPUT - $'[[[[[[[]]]]]]]]'
		OUTPUT - $']]]]]]]]'
	EXEC	-d '[=]=]'
		OUTPUT - $'[[[[[[['
	EXEC	-d '[:digit:]'
		INPUT - $'a0b1c2d3e4f5g6h7i8j9k'
		OUTPUT - $'abcdefghijk'
	EXEC	-d '[:xdigit:]'
		INPUT - $'w0x1y2z3456789acbdefABCDEFz'
		OUTPUT - $'wxyzz'
	EXEC	-d '[:xdigit:]'
		INPUT - $'0123456789acbdefABCDEF'
		OUTPUT - $''
	EXEC	-d '[:digit:]'
		INPUT - $'0123456789'
	EXEC	-d '[:lower:]'
		INPUT - $'abcdefghijklmnopqrstuvwxyz'
	EXEC	-d '[:upper:]'
		INPUT - $'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
	EXEC	-d '[:lower:][:upper:]'
		INPUT - $'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ'
	EXEC	-d '[:alpha:]'
		INPUT - $'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ'
	EXEC	-d '[:alnum:]'
		INPUT - $'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'
	EXEC	-d '[:alnum:]'
		INPUT - $'.abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.'
		OUTPUT - $'..'
	EXEC	-ds '[:alnum:]' '.'
		OUTPUT - $'.'

TEST 09 '-s combinations'
	EXEC	-cs '[:alnum:]' '\n'
		INPUT - $'The big black fox jumped over the fence.'
		OUTPUT - $'The\nbig\nblack\nfox\njumped\nover\nthe\nfence'
	EXEC	-cs '[:alnum:]' '[\n*]'
	EXEC	-ds 'b' 'a'
		INPUT - $'aabbaa'
		OUTPUT - $'a'
	EXEC	-ds '[:xdigit:]' 'Z'
		INPUT - $'ZZ0123456789acbdefABCDEFZZ'
		OUTPUT - $'Z'
	EXEC	-ds '\350' '\345'
		INPUT -n - $'\300\301\377\345\345\350\345'
		OUTPUT -n - $'\300\301\377\345'
	EXEC	-s 'abcdefghijklmn' '[:*016]'
		INPUT - $'abcdefghijklmnop'
		OUTPUT - $':op'
	EXEC	-d 'a-z'
		INPUT - $'abc $code'
		OUTPUT - $' $'
	EXEC	-ds 'a-z' '$.'
		INPUT - $'a.b.c $$$$code\\'
		OUTPUT - $'. $\\'

TEST 10	'ranges and classes'
	EXEC	'a-a' 'z'
		INPUT - $'abc'
		OUTPUT - $'zbc'
	EXEC	'[:lower:]' '[:upper:]'
		INPUT - $'abcxyzABCXYZ'
		OUTPUT - $'ABCXYZABCXYZ'
	EXEC	'[:upper:]' '[:lower:]'
		OUTPUT - $'abcxyzabcxyz'
	EXEC	'a[=*2][=c=]' 'xyyz'
		INPUT - $'a=c'
		OUTPUT - $'xyz'
	EXEC	'[:*3][:digit:]' 'a-m'
		INPUT - $':1239'
		OUTPUT - $'aefgm'
	EXEC	':*3[:digit:]' 'a-m'
		INPUT - $':1239'
		OUTPUT - $'aefgm'
	EXEC	'a[b*512]c' '1[x*]2'
		INPUT - $'abc'
		OUTPUT - $'1x2'
	EXEC	'a[b*513]c' '1[x*]2'
	EXEC	'a\-z' 'A-Z'
		INPUT - $'abc-z'
		OUTPUT - $'AbcBC'
	EXEC	0-4 _
		INPUT - $'a1-q2589z'
		OUTPUT - $'a_-q_589z'
	EXEC	0-4- _
		INPUT - $'a1-q2589z'
		OUTPUT - $'a__q_589z'
	EXEC	0-4-9 _
		INPUT - $'a1-q2589z'
		OUTPUT - $'a__q_58_z'

TEST 11	'from ross'
	EXEC	-cs '[:upper:][:digit:]' '[Z*]'
	EXEC	-dcs '[:alnum:]' '[:digit:]'
	EXEC	-dc '[:lower:]'
	EXEC	-dc '[:upper:]'
	EXEC	-cs '[:upper:]' '[X*]'
		INPUT -n - $'AMZamz123.-+AMZ'
		OUTPUT -n - $'AMZXAMZ'
	EXEC	-dcs '[:lower:]' 'n-rs-z'
		INPUT -n - $'amzAMZ123.-+amz'
		OUTPUT -n - $'amzamz'
	EXEC	-ds '[:xdigit:]' '[:alnum:]'
		INPUT - $'.ZABCDEFGzabcdefg.0123456788899.GG'
		OUTPUT - $'.ZGzg..G'

TEST 12	'gnu complains about these'
	DO	DATA nul5.dat
	EXEC	'a' ''
		INPUT - $'abc123'
		SAME OUTPUT nul5.dat
	EXEC	-cs '[:upper:]' 'X[Y*]'
		OUTPUT -n - $'Y'
	EXEC	-cs '[:cntrl:]' 'X[Y*]'
		OUTPUT - $'Y'

TEST 13 'unlucky char sign extension?'
	EXEC	$'\x8d' $'\n'
		INPUT -n - $'x\x8d'
		OUTPUT - $'x'

TEST 14 'multibyte basics'
	EXPORT	LC_CTYPE=C.UTF-8
	export	LC_CTYPE=C.UTF-8
	EXEC	-c $'[:alpha:]\n' 'X'
		INPUT - $'\u[20ac]\u[20ac]'
		OUTPUT - $'XX'
	EXEC	-C $'[:alpha:]\n' 'X'
	EXEC	-cq $'[:alpha:]\n' 'X'
		INPUT - $'\u[20ac]\xac\u[20ac]'
		OUTPUT - $'X\xacX'
	EXEC	-Cq $'[:alpha:]\n' 'X'
	EXEC	-c $'[:alpha:]\n' 'X'
		INPUT - $'\u[20ac]\xac\u[20ac]'
		OUTPUT - $'X\xacX'
		ERROR - $'tr: line 1: \\xac: invalid multibyte character byte'
		EXIT	1
	EXEC	-C $'[:alpha:]\n' 'X'
