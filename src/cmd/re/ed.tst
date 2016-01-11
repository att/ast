# regression tests for the POSIX ed utility

KEEP "*.dat"

function DATA
{
	typeset f x=0123456789
	integer i
	typeset -i8 n
	for f
	do	test -f $f && continue
		case $f in
		big.dat)for ((i = 0; i <= 10000; i++))
			do	print $i:$x$x$x$x$x$x$x$x$x$x
			done
			;;
		dem.dat)for ((i = 512; i >= 0; i--))
			do	print $i
			done
			;;
		med.dat)for ((i = 0; i <= 512; i++))
			do	print $i
			done
			;;
		esac > $f
	done
}

TEST 01 'file args'
	DO	DATA big.dat
	EXEC
		NOTE 'no input file'
	EXEC file
		NOTE 'empty input file'
		INPUT file
		OUTPUT - 0
	EXEC big.dat
		NOTE 'big input file'
		INPUT - $'v/^10*:/d\n1,$s/:.*//\n,p\nQ'
		OUTPUT - $'1058997\n1\n10\n100\n1000\n10000'
	EXEC not_a_file
		NOTE 'non-existent input file'
		OUTPUT - '?'
		EXIT '[12]'

TEST 02 '{ = # a c } commands'
	EXEC file
		NOTE '='
		INPUT - g/./.=
		INPUT file $'a\nb\nc'
		OUTPUT - $'6\n1\n2\n3'
	EXEC file
		NOTE '# comments'
		INPUT - $'#n\ng/./.=\n#comment'
	EXEC file
		NOTE 'a top'
		INPUT - $'0a\nz\n.\nw\nq'
		OUTPUT - $'6\n8'
		OUTPUT file $'z\na\nb\nc'
	EXEC file
		NOTE 'a mid'
		INPUT - $'2a\nz\n.\nw\nq'
		OUTPUT file $'a\nb\nz\nc'
	EXEC file
		NOTE 'a bot'
		INPUT - $'$a\nz\n.\nw\nq'
		OUTPUT file $'a\nb\nc\nz'
	EXEC file
		NOTE 'c top'
		INPUT - $'1c\nz\n.\nw\nq'
		OUTPUT - $'6\n6'
		OUTPUT file $'z\nb\nc'
	EXEC file
		NOTE 'c mid'
		INPUT - $'2c\nz\n.\nw\nq'
		OUTPUT file $'a\nz\nc'
	EXEC file
		NOTE 'c bot'
		INPUT - $'$c\nz\n.\nw\nq'
		OUTPUT file $'a\nb\nz'

TEST 03 '{ s } commands'
	EXEC file
		NOTE 'substitute'
		INPUT file $'a\nb\nc'
		INPUT - $'1,$s/./&&/\n1,$s/./&x/\n1,$s/./&y/2\n1,$s/./&z/4\nw\nq'
		OUTPUT file $'axyaz\nbxybz\ncxycz'
		OUTPUT - $'6\n18'
	EXEC file
		NOTE 'substitute g'
		INPUT file $'abcabcabc\nabcabcabc\nabcabcabc\nabcabcabc\nabcabcabc\nabcabcabc\nabcabcabc\nabcabcabc\nabcabcabc'
		INPUT - $'1s/a//g\n2s/b//g\n3s/c//g\n4s/a/X/g\n5s/b/X/g\n6s/c/X/g\n7s/a/XX/g\n8s/b/XX/g\n9s/c/XX/g\nw\nq'
		OUTPUT file $'bcbcbc\nacacac\nababab\nXbcXbcXbc\naXcaXcaXc\nabXabXabX\nXXbcXXbcXXbc\naXXcaXXcaXXc\nabXXabXXabXX'
		OUTPUT - $'90\n90'
	EXEC file
		NOTE 'substitute count'
		INPUT - $'1s/a//2\n2s/b//2\n3s/c//2\n4s/a/X/2\n5s/b/X/2\n6s/c/X/2\n7s/a/XX/2\n8s/b/XX/2\n9s/c/XX/2\nw\nq'
		OUTPUT file $'abcbcabc\nabcacabc\nabcababc\nabcXbcabc\nabcaXcabc\nabcabXabc\nabcXXbcabc\nabcaXXcabc\nabcabXXabc'
		OUTPUT - $'90\n90'
	EXEC file
		NOTE 'substitute null g'
		INPUT file $'123\naaa'
		INPUT - $'1,$s/a*/x/g\nw\nq'
		OUTPUT file $'x1x2x3x\nx'
		OUTPUT - $'8\n10'
	EXEC file
		NOTE 'substitute splice'
		INPUT file $'123\naaa'
		INPUT - $'1,$s/a/&\\\nx/\nw\nq'
		OUTPUT file $'123\na\nxaa'
		OUTPUT - $'8\n10'
	EXEC file
		NOTE 'substitute previous'
		INPUT file $'123\naaa'
		INPUT - $'/aa/p\ns//zz/\nw\nq'
		OUTPUT file $'123\nzza'
		OUTPUT - $'8\naaa\n8'

TEST 04 'global commands'
	EXEC file
		NOTE 'global substitute splice'
		INPUT file $'yyabcyy\n123yyxx\nfoo\nbaryy\nfooyybar'
		INPUT - $'g/yy/s//ex/g\nw\nq'
		OUTPUT file $'exabcex\n123exxx\nfoo\nbarex\nfooexbar'
		OUTPUT - $'35\n35'

TEST 05 '{ t } commands'
	DO	DATA med.dat dem.dat
	EXEC file
		NOTE 'copy'
		INPUT file $'1\n2\n3\n4'
		INPUT - $'-2\n.t.\nw\nq'
		OUTPUT file $'1\n2\n2\n3\n4'
		OUTPUT - $'8\n2\n10'
	EXEC med.dat
		NOTE 'copy reverse'
		INPUT - $'g/./.t0\n514,$d\nw file\nq'
		SAME file dem.dat
		OUTPUT - $'1942\n1942'

TEST 06 '! command'
	EXEC -h file
		NOTE 'no saved command'
		INPUT file
		OUTPUT - $'"file" 0 lines, 0 characters'
		INPUT - $'!!'
		ERROR - $'ed: no saved shell command'
		EXIT 1
	EXEC
		NOTE 'empty command'
		INPUT - $'!'
		ERROR - $'ed: empty shell command'
	EXEC
		NOTE 'to stdout'
		INPUT - $'!echo stdout\n!!'
		OUTPUT - $'"file" 0 lines, 0 characters\nstdout\n!\necho stdout\nstdout\n!'
		ERROR -
		EXIT 0
	EXEC
		NOTE 'to stderr'
		INPUT - $'!echo stderr >&2\n!!'
		OUTPUT - $'"file" 0 lines, 0 characters\n!\necho stderr >&2\n!'
		ERROR - $'stderr\nstderr'

TEST 07 'REs'
	EXEC file
		NOTE 'remembered RE'
		INPUT file $'foo 1\nfoo 2\nfoo 3'
		INPUT - $'/foo\n//\n/'
		OUTPUT - $'18\nfoo 1\nfoo 2\nfoo 3'

TEST 08 '.'
	EXEC file
		NOTE '. before and after'
		INPUT file $'1\n2\n3'
		INPUT - $'.=\n1,$s/^/x/\n.=\n1\n1,$s/^/x/\n.=\ng/[12]/s/^/z/\n.=\nQ'
		OUTPUT - $'6\n3\n3\nx1\n3\n2'
