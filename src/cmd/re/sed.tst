# regression tests for the POSIX sed utility

function DATA
{
	for f
	do	test -f $f && continue
		case $f in
		[0-7][0-7][0-7])
			echo $f > $f
			chmod $f $f
			;;
		nul.in) print '\0AAAfooZZZ' > $f
			;;
		nul.out)print '\0AAAbarZZZ' > $f
			;;
		esac
	done
}

TEST 01 'empty script and passive command combinations'
	EXEC -f script -n
		NOTE '=, -n, blank lines'
		INPUT script $'\n=\n'
		INPUT - $'a\nb\nc'
		OUTPUT - $'1\n2\n3'
	EXEC -f script
		NOTE 'comments, including #n'
		INPUT script $'#n\n=\n#comment'
	EXEC -f script
		NOTE 'empty script'
		INPUT script
		OUTPUT - $'a\nb\nc'
	EXEC -f script
		NOTE 'do-nothing script'
		INPUT script $'# nothing\n'
	EXEC -f script
		NOTE 'substitution; regexp dot; &; substitution flag'
		INPUT script $'s/./&&/\n s/./&x/\n s/./&y/2\n s/./&z/5'
		OUTPUT - $'axya\nbxyb\ncxyc'

TEST 02 'line counting, ranges, overlaps'
	EXEC -f script
		NOTE 'd overlap'
		INPUT script $'1,2d\n 2,6d\n 4,5d\n 7,10d'
		INPUT - $'1\n2\n3\n4\n5\n6\n7\n8'
		OUTPUT - $'3\n6'
	EXEC -f script -e 1,2d -e 2,6d
		NOTE 'd disjoint range'
		INPUT script $'4,5d\n 7,10d'
	EXEC -f script -n
		NOTE 'd bypassing end of range'
		INPUT script $'2,5d\n 1,3p\n 1,6p'
		OUTPUT - $'1\n1\n6'
	EXEC -f script
		NOTE 'negation, address $, print'
		INPUT script $'3,$!d\n $d\n p\n 4,5d'
		OUTPUT - $'3\n3\n4\n5\n6\n6\n7\n7'

TEST 03 'regexp addresses, append, insert, change'
	EXEC -f script
		INPUT script $'/a/a\\\nA\\\nA\n/a/a\\\nB\n/d/i\\\nD\n/c/c\\\nC\\\nC\n$i\\\nE'
		INPUT - $'a\nb\nc\nd'
		OUTPUT - $'a\nA\nA\nB\nb\nC\nC\nD\nE\nd'

TEST 04 'braces'
	EXEC -f script
		INPUT script $'3,7{\n/a/s/a/&&/\n/b/,/./s/./&&&/\n}'
		INPUT - $'a\nb\na\nb\na\na\nb\na\nb'
		OUTPUT - $'a\nb\naa\nbbb\naaaa\naa\nbbb\na\nb'

TEST 05 'hold, get combinations'
	EXEC -f script
		NOTE 'hold, get'
		INPUT script $'$!H\n$!d\n$G'
		INPUT - $'1\n2\n3\n4'
		OUTPUT - $'4\n\n1\n2\n3'
	EXEC -f script
		NOTE 'hold, exchange, get, brace without newline'
		INPUT script $'1{h\nd\n}\n3x\n$g'
		OUTPUT - $'2\n1\n3'
	EXEC -f script
		NOTE 'quit'
		INPUT script $'a\\\nx\nq\n='
		OUTPUT - $'1\nx'

TEST 06 'next, regexp $'
	EXEC -f script
		INPUT script $'s/$/x/\nn\ns/./&y/\nN\ns/.../&z/\n='
		INPUT - $'a\nb\nc'
		OUTPUT - $'ax\n3\nby\nzc'

TEST 07 'substitute flag combinations'
	EXEC -f script -n
		NOTE 's newline p'
		INPUT script $'/../s/./&\\\n/\np\nP\ns/\\n//p'
		INPUT - $'a\nbc\nd'
		OUTPUT - $'a\na\nb\nc\nb\nbc\nd\nd'
	EXEC -f script -n
		NOTE 'write, flag w'
		INPUT script $'/../s/./&\\\n/\nw result\ns/\\n//w result'
		OUTPUT result $'a\nb\nc\nbc\nd'
		OUTPUT -
	EXEC -f script -n
		NOTE 's w file'
		INPUT script $'s/b/--/gw result'
		INPUT - $'abcabc'
		OUTPUT result $'a--ca--c'
	EXEC -f script -n
		NOTE 's pw file'
		INPUT script $'s/b/--/gpw result'
		OUTPUT result $'a--ca--c'
		OUTPUT - $'a--ca--c'
	EXEC -f script
		NOTE 's pw file'
		INPUT script $'s/b/--/gpw result'
		OUTPUT result $'a--ca--c'
		OUTPUT - $'a--ca--c\na--ca--c'

TEST 08 'character classes, flag g'
	EXEC -f script
		INPUT script $'1s/[a-z][a-z]*//\n2s/[[:digit:]][[:digit:]]*//\n3s/[^a-z]/X/g\n'
		INPUT - $'AZ`abyz{\n/0189:\na1b2c3d'
		OUTPUT - $'AZ`{\n/:\naXbXcXd'
	EXEC $'s/[\\]/X/g'
		INPUT - $'[]./:,\\'
		OUTPUT - $'[]./:,X'
	EXEC $'s/[^./]/X/g'
		OUTPUT - $'XX./XXX'
	EXEC $'s/[][]/X/g'
		OUTPUT - $'XX./:,\\'
	EXEC $'s.[.]\\..X.g'
		OUTPUT - $'[]X:,\\'

TEST 09 'null matches'
	EXEC -f script
		INPUT script $'1,2s/a*/x/g'
		INPUT - $'123\naaa'
		OUTPUT - $'x1x2x3x\nx'

TEST 10 'longest match, unmatched subexpressions'
	EXEC -f script
		INPUT script $'s/\\(...\\)*\\(..\\)*/:\\1:\\2:/'
		INPUT - $'abc\nabcd\nabcde\nabcdef\nabcdefg'
		OUTPUT - $':abc::\n::cd:\n:abc:de:\n:def::\n:abc:fg:'

TEST 11 'metacharacters in substition'
	EXEC -f script
		INPUT script $'1s/$/\\&/\n2s/$/\\b/\n3s/$/\\\\/\n4s/$/\\//\n5s&$&\\&&'
		INPUT - $'1\n2\n3\n4\n5'
		OUTPUT - $'1&\n2\b\n3\\\n4/\n5'

TEST 12 'branch combinations'
	EXEC -f script
		NOTE 'branches'
		INPUT script $':x\n/a/{\ns/a/x/\nbx\n}'
		INPUT - $'aaa\nb\nabca'
		OUTPUT - $'xxx\nb\nxbcx'
	EXEC -f script
		NOTE 'long labels may be truncated'
		INPUT script $':longlabel\n/a/s/a/x/\ntlonglabel'
	EXEC -f script
		NOTE 'jump to end of script'
		INPUT script $'3b\n/a/s/a/x/g'
		OUTPUT - $'xxx\nb\nabca'
	EXEC -f script -n
		NOTE 'end of bracket range'
		INPUT script $'/c/d\n/a/,/d/{\n\t/b/,/c/{\n\t\t=\n\t}\n}'
		INPUT - $'a\nb\nc\nd\na'
		OUTPUT - $'2\n4\n5'
	EXEC -f script
		NOTE 'end of change range'
		INPUT script $'/a/,/b/{\n\t/b/,/c/c\\\nx\n}'
		OUTPUT - $'a\nc\nd\nx'
	EXEC -f script
		NOTE 'end of change range'
		INPUT - $'a\nb\nc\na\nc\nb\nd'
		OUTPUT - $'a\nc\nx\nd'
	EXEC -f script
		NOTE 'end of change range'
		INPUT script $'/a/,/b/c\\\nc'
		INPUT - $'a\nb\na'
		OUTPUT - $'c\nc'

TEST 15 'weird delimiters, remembered expression'
	EXEC -f script
		INPUT script $'1s1\\(.\\)\\11\\11\n2s.\\(\\.\\)\\..\\.\\1.\n3s*\\(.\\)\\**\\*\\1*\n4s&\\(.\\)\\&&\\1\\&&\n\\1\\(\\1\\)\\11s//\\1/\n\\&\\(\\&\\)\\&&s//\\&b&/'
		INPUT - $'a1b\nabc\nabc\na&b\n11b\na&&'
		OUTPUT - $'1b\n.ac\n*c\naa&b\n1b\na&b&&'

TEST 16 '7680-char line, backreference'
	EXEC -f script
		INPUT script $'s/.*/&&&&&&&&/\ns//&&&&&&&&/\ns//&&&&&&&&/\nh\ns/[^8]//g\ns/^\\(.*\\)\\1\\1\\1$/\\1/\ns//\\1/\ns//\\1/\ns//\\1/p\ng\ns/^\\(.*\\)\\1\\1\\1$/\\1/\ns//\\1/\ns//\\1/\ns//\\1/\ns/\\(.*\\)\\1/\\1/'
		INPUT - $'123456787654321'
		OUTPUT - $'88\n123456787654321'

TEST 17 'r from w file, nonexistent r'
	EXEC -f script -n
		INPUT script $'r not_a_file\nr OUTPUT\nw OUTPUT'
		INPUT - $'1\n2\n3'
		OUTPUT - $'1\n1\n2\n1\n2\n3'

TEST 18 'eof combinations'
	EXEC -f script
		NOTE 'eof in n'
		INPUT script $'a\\\n1\nn'
		INPUT - $'a'
		OUTPUT - $'a\n1'
	EXEC -f script
		NOTE 'eof in N'
		INPUT script $'a\\\n1\nN'
		OUTPUT - $'1'

TEST 19 'transliterate'
	EXEC -f script
		INPUT script $'y/abc/ABC/\ny:/\\\\\\:.:\\/.\\::'
		INPUT - $'abcABCabcdef\n1/:2.'
		OUTPUT - $'ABCABCABCdef\n1\\.2:'
	EXEC -f script
		INPUT script $'y/(/\\n/;y/)/\\n/'
		INPUT - $'(a)(b)(c)'
		OUTPUT - $'\na\n\nb\n\nc\n'

TEST 20 'N, D, G combinations'
	EXEC -f script
		NOTE 'N, D'
		INPUT script $'=\nN\np\nD\ns/.*/x/'
		INPUT - $'a\nb\nc'
		OUTPUT - $'1\na\nb\n2\nb\nc\n3'
	EXEC -f script
		NOTE 'D, G initial states'
		INPUT script $'1D\nG\nh'
		OUTPUT - $'b\n\nc\nb\n'

TEST 21 'a,c,r interaction'
	EXEC -f script
		INPUT script $'$!a\\\nA\n$!r INPUT\n$!a\\\nB\n1,2c\\\nC'
		INPUT - $'a\nb\nc'
		OUTPUT - $'A\na\nb\nc\nB\nC\nA\na\nb\nc\nB\nc'

TEST 22 'substitution null string combinations'
	EXEC -f script
		NOTE 'global substitution for null string'
		INPUT script $'s/a*/b/g'
		INPUT - $'aaa\nccc'
		OUTPUT - $'b\nbcbcbcb'
		NOTE 'count 1 substitution for null string'
	EXEC -f script
		INPUT script $'s/a*/b/1'
		OUTPUT - $'b\nbccc'
		NOTE 'count 2 substitution for null string'
	EXEC -f script
		INPUT script $'s/a*/b/2'
		OUTPUT - $'aaa\ncbcc'
	EXEC -f script
		NOTE 'count 3 substitution for null string'
		INPUT script $'s/a*/b/3'
		OUTPUT - $'aaa\nccbc'

TEST 23 'perverse semicolons'
	EXEC $'s;\\;;x;'
		INPUT - $'a;b'
		OUTPUT - $'axb'

TEST 24 'multiple files'
	EXEC -n '$=' INPUT INPUT INPUT
		INPUT - $'a;\nb;\nc'
		OUTPUT - $'9'

TEST 25 'weird chars, line folding'
	EXEC -n $'s/.*/&&&&&&&&&&/\nH\nH\nH\nG\nl'
		INPUT - $'\ta\a\\\n'
		OUTPUT - $'\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\\n\\n\\n\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\\n\\a\\\\\\n\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\t\\\na\\a\\\\\\n\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\\n\\ta\\a\\\\$\n\\n\\n\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\\n\\a\\\\\\n\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\t\\\na\\a\\\\\\n\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\ta\\a\\\\\\\n\\ta\\a\\\\\\n\\n\\n$'
	EXEC -n $'s/.*/&&&&&&&&&&/\nH\nH\nH\nG\nl'
		INPUT - $'\ta\002\\\n'
		OUTPUT - $'\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\n\\\\\\ta\\002\\\\\\ta\\002\\\\\\n\\n\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\t\\\na\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\n\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\n\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\n\\ta\\\n\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\t\\\na\\002\\\\\\ta\\002\\\\$\n\\n\\n\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\\n\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\n\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\\n\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\n\\ta\\002\\\\\\ta\\002\\\\\\ta\\\n\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\ta\\002\\\\\\n\\\n\\n\\n$'

TEST 26 'multiple w files, multiple inputs, OUTPUT1 empty'
	EXEC -f script -n
		INPUT - $'1\n2\n3\n4\n5\n6\n7\n8\n9'
		INPUT script $'9,$d\nw out9\n8,$d\nw out8\n7,$d\nw out7\n6,$d\nw out6\n5,$d\nw out5\n4,$d\nw out4\n3,$d\nw out3\n2,$d\nw out2\n1,$d\nw out1'
		OUTPUT -n out1
		OUTPUT out2 $'1'
		OUTPUT out3 $'1\n2'
		OUTPUT out4 $'1\n2\n3'
		OUTPUT out5 $'1\n2\n3\n4'
		OUTPUT out6 $'1\n2\n3\n4\n5'
		OUTPUT out7 $'1\n2\n3\n4\n5\n6'
		OUTPUT out8 $'1\n2\n3\n4\n5\n6\n7'
		OUTPUT out9 $'1\n2\n3\n4\n5\n6\n7\n8'

TEST 27 'assorted warnings'
	DIAGNOSTICS
	EXEC -n /o/p
		NOTE 'trailing newline omitted'
		INPUT -n - $'hello\ngoodbye'
		OUTPUT - $'hello\ngoodbye'

TEST 28 'assorted diagnostics'
	DIAGNOSTICS
	EXEC = not_a_file
	EXEC -e :x -e :x
	EXEC r/dev/null
	EXEC -f not_a_file
	EXEC
	EXEC 0p
	EXEC //p
	EXEC bx
	EXEC /
	EXEC /a
	EXEC $'/\\'
	EXEC /a/
	EXEC 1,c
	EXEC 1,/
	EXEC 1,2,3p
	EXEC $'/\\(/p'
	EXEC $'/\\1/p'
	EXEC s/a/b
	EXEC s/a/b/q
	EXEC s/a/b/3g3
	EXEC s/a/b/gg
	EXEC s/a/b/pp
	EXEC s/a/b/wnot_a_file
	EXEC y/a
	EXEC y/a/
	EXEC y/a/b
	EXEC y/aa/bb/
	EXEC y/aa/ab/
	EXEC y/a/bb/
	EXEC y/aa/b/
	EXEC 1,2=
	EXEC 1,2q
	EXEC :
	EXEC $'\\'
	EXEC a
	EXEC c
	EXEC e
	EXEC f
	EXEC i
	EXEC j
	EXEC k
	EXEC m
	EXEC o
	EXEC r
	EXEC s
	EXEC u
	EXEC v
	EXEC w
	EXEC y
	EXEC z
	EXEC $'{'
	EXEC $'}'
	EXEC 1
	EXEC pq
	EXEC dq
	EXEC aq
	EXEC $'!!p'
	EXEC $'1#'
	EXEC $'a\\'
	EXEC $'w .'
	EXEC $'s /a/b/'
	EXEC $'s/a/b/w .'

TEST 29 'customary extensions'
BODY {
	COMMAND '=;=' </dev/null 2>/dev/null && a=yes || a=no
	INFO "$a" 'semicolon usable as newline'
	case $a in
	*y*)	a=$(print no | COMMAND ':x;s/no/yes/' 2>/dev/null)
		INFO "$a" 'semicolon terminates a label'
		COMMAND '=;  =' </dev/null 2>/dev/null && a=yes || a=no
		INFO "$a" 'space after semicolon'
		COMMAND '=  ;=' </dev/null 2>/dev/null && a=yes || a=no
		INFO "$a" 'space before semicolon'
		;;
	esac
	COMMAND '/a/s///' </dev/null 2>/dev/null && a=yes || a=no
	INFO "$a" 'previous regular expression can be abbreviated as //'
	case $a in
	*y*)	a=$(print ab | COMMAND '
			/a/bx
      				/b/=
			:x
			s///
			/a/s/.*/static/
			/b/s/.*/dynamic/
		')
		INFO "$a" 'label scope'
		;;
	esac
	COMMAND 'w/dev/null' </dev/null 2>/dev/null && a=yes || a=no
	INFO "$a" 'space optional in r and w commands'
	COMMAND '/\y/b' </dev/null 2>/dev/null && a=yes || a=no
	INFO "$a" '\ may precede a non-special character in regular expression'
	COMMAND 's/x/\y/' </dev/null 2>/dev/null && a=yes || a=no
	INFO "$a" '\ may precede a non-special character in substitution text'
	COMMAND '1 , 2p' </dev/null 2>/dev/null && a=yes || a=no
	INFO "$a" 'spaces allowed between addresses'
}

TEST 30 'hold buffer line join'
	EXEC -One '
/^Package:/{
s/^Package:[[:space:]]*\<\([[:alnum:].+-]*$1[[:alnum:].+-]*\).*/\1/
h
}
/^Description:/{
s/^Description:[[:space:]]*\(.*\)/\1/
H
g
s/\
/ - /
p
}
'
		INPUT - 'Package: grep
Essential: yes
Priority: required
Section: base
Installed-Size: 488
Debian-Maintainer: Wichert Akkerman <wakkerma@debian.org>
Maintainer: Carl Worth <cworth@handhelds.org>
Architecture: arm
Version: 2.4.2-1
Provides: rgrep
Pre-Depends: libc6 (>= 2.1.2)
Conflicts: rgrep
Filename: ./grep_2.4.2-1_arm.ipk
Size: 119438
MD5Sum: 67fa4cb756f951fda7b7a5d4da2ab523
Description: GNU grep, egrep and fgrep.
 The GNU family of grep utilities may be the "fastest grep in the west".
 GNU grep is based on a fast lazy-state deterministic matcher (about
 twice as fast as stock Unix egrep) hybridized with a Boyer-Moore-Gosper
 search for a fixed string that eliminates impossible text from being
 considered by the full regexp matcher without necessarily having to
 look at every character. The result is typically many times faster
 than Unix grep or egrep. (Regular expressions containing backreferencing
 will run more slowly, however.)

Package: sed
Priority: required
Section: base
Installed-Size: 180
Debian-Maintainer: Wichert Akkerman <wakkerma@debian.org>
Maintainer: Carl Worth <cworth@handhelds.org>
Architecture: arm
Version: 3.02-6
Pre-Depends: libc6 (>= 2.1.2)
Filename: ./sed_3.02-6_arm.ipk
Size: 12338
MD5Sum: c893daf6fef70813b566db8ed8c06950
Description: The GNU sed stream editor.
 sed reads the specified files or the standard input if no
 files are specified, makes editing changes according to a
 list of commands, and writes the results to the standard
 output.'
		OUTPUT - 'Package: grep - GNU grep, egrep and fgrep.
Package: sed - The GNU sed stream editor.'

TEST 31 'file access'
	DATA 000 644
	DIAGNOSTICS
	EXEC 's/./x/' 000 644
		OUTPUT - $'x44'
	EXEC 's/./x/' 644 000

TEST 32 'embedded \0'
	DATA nul.in nul.out
	EXEC 's/foo/bar/' nul.in
		SAME OUTPUT nul.out

TEST 33 'multibyte'
	EXPORT LC_CTYPE=C.UTF-8

	EXEC $'y/\303\240\303\242\303\251\303\250\303\252\303\253\303\256\303\257\303\264\303\266\303\271\303\273\303\247/aaeeeeiioouuc/'
		INPUT - $'chaine de caract\303\250res d\303\251finie en utf-8'
		OUTPUT - $'chaine de caracteres definie en utf-8'

	EXEC $'y\342\202\254\303\240\303\242\303\251\303\250\303\252\303\253\303\256\303\257\303\264\303\266\303\271\303\273\303\247\342\202\254aaeeeeiioouuc\342\202\254'
