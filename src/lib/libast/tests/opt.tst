# regression tests for the ast optget() and opthelp() functions

export LC_ALL=C LC_MESSAGES=C

TEST 01 'compatibility'
	EXEC	net 'a-' -a b
		OUTPUT - $'return=a option=-a name=-a arg=(null) num=1
argument=1 value="b"'
	EXEC	net 'a-:' -a b
	EXEC	net 'a-:' -a --help b
		OUTPUT - $'return=a option=-a name=-a arg=(null) num=1
return=- option=-- name=-- arg=help num=1
argument=1 value="b"'
	EXEC	cmd $'a file ...' -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: cmd [-a] file ...'
		EXIT 2
	EXEC	cmd $'a file ...' --??api
		OUTPUT - $'return=? option=-? name=--??api num=0'
		ERROR - $'.SH SYNOPSIS\ncmd [ options ] file ...\n.SH OPTIONS\n.OP a - flag -'
	EXEC	cmd $'[ab] file' -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: cmd [ -a | -b ] file'
	EXEC	cmd $'[ab] file' --??api
		OUTPUT - $'return=? option=-? name=--??api num=0'
		ERROR - $'.SH SYNOPSIS\ncmd [ options ] file\n.SH OPTIONS\n.OP - - oneof\n.OP a - flag -\n.OP b - flag -\n.OP - - anyof'
	EXEC	cmd $'[a:b] file' -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: cmd [ -a arg | -b ] file'
	EXEC	cmd $'[a:b] file' --??api
		OUTPUT - $'return=? option=-? name=--??api num=0'
		ERROR - $'.SH SYNOPSIS\ncmd [ options ] file\n.SH OPTIONS\n.OP - - oneof\n.OP a - string arg\n.OP b - flag -\n.OP - - anyof'
	EXEC	wc $'lw[cm] [ file ... ]' -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: wc [-lw] [ -c | -m ] [ file ... ]'
	EXEC	wc $'lw[cm] [ file ... ]' --??api
		OUTPUT - $'return=? option=-? name=--??api num=0'
		ERROR - $'.SH SYNOPSIS\nwc [ options ] [ file ... ]\n.SH OPTIONS\n.OP l - flag -\n.OP w - flag -\n.OP - - oneof\n.OP c - flag -\n.OP m - flag -\n.OP - - anyof'

TEST 02 'long options'
	usage=$'[-?\n@(#)testopt (AT&T Research) 1999-02-02\n][-author?Glenn Fowler <gsf@research.att.com>][--dictionary?tests:opt][x:method?One of]:?[algorithm:oneof:ignorecase]{[+a?method a][+b?method b][+c?method c]}[\n[y=10:yes?Yes.][20:no?No.]\n][d:database?File database path.]:[path][z:size?Important size.]#[sizeX][V:vernum?List program version and exit.][v:verbose?Enable verbose trace.][n:show?Show but don\'t execute.]\n\npattern ...'
	EXEC	cmd "$usage" -y
		OUTPUT - 'return=-10 option=-10 name=-y arg=(null) num=1'
	EXEC	cmd "$usage" --y
		OUTPUT - 'return=-10 option=-10 name=--yes arg=(null) num=1'
	EXEC	cmd "$usage" -x
		OUTPUT - 'return=x option=-x name=-x arg=(null) num=1'
	EXEC	cmd "$usage" -xabc
		OUTPUT - 'return=x option=-x name=-x arg=abc num=1'
	EXEC	cmd "$usage" -x abc
	EXEC	cmd "$usage" --method
		OUTPUT - 'return=x option=-x name=--method arg=(null) num=1'
	EXEC	cmd "$usage" --method=abc
		OUTPUT - 'return=x option=-x name=--method arg=abc num=1'
	EXEC	cmd "$usage" --method abc
		OUTPUT - $'return=x option=-x name=--method arg=(null) num=1\nargument=1 value="abc"'
	EXEC	cmd "$usage" -V -x --si 10k --size=1m
		OUTPUT - $'return=V option=-V name=-V arg=(null) num=1\nreturn=x option=-x name=-x arg=(null) num=0\nreturn=z option=-z name=--size arg=10k num=10000\nreturn=z option=-z name=--size arg=1m num=1000000'
	EXEC	cmd "$usage" -V -x --si 10ki --size=1mi
		OUTPUT - $'return=V option=-V name=-V arg=(null) num=1\nreturn=x option=-x name=-x arg=(null) num=0\nreturn=z option=-z name=--size arg=10ki num=10240\nreturn=z option=-z name=--size arg=1mi num=1048576'
	EXEC	cmd "$usage" --yes --no --noyes --no-yes --yes=1 --yes=0
		OUTPUT - $'return=-10 option=-10 name=--yes arg=(null) num=1\nreturn=-20 option=-20 name=--no arg=(null) num=1\nreturn=-10 option=-10 name=--yes arg=(null) num=0\nreturn=-10 option=-10 name=--yes arg=(null) num=0\nreturn=-10 option=-10 name=--yes arg=(null) num=1\nreturn=-10 option=-10 name=--yes arg=(null) num=0'
	EXEC	cmd "$usage" --vern
		OUTPUT - $'return=V option=-V name=--vernum arg=(null) num=1'
	EXEC	cmd "$usage" --ver
		OUTPUT - $'return=: option=-V name=--ver num=0'
		ERROR - $'cmd: --ver: ambiguous option'
		EXIT 1
	EXEC	cmd "$usage" --ve
		OUTPUT - $'return=: option=-V name=--ve num=0'
		ERROR - $'cmd: --ve: ambiguous option'
	EXEC	cmd "$usage" --v
		OUTPUT - $'return=: option=-V name=--v num=0'
		ERROR - $'cmd: --v: ambiguous option'
	EXEC	cmd "$usage" -z
		OUTPUT - $'return=: option=-z name=-z num=0'
		ERROR - $'cmd: -z: numeric sizeX argument expected'
	EXEC	cmd "$usage" --size
		OUTPUT - $'return=: option=-z name=--size num=0'
		ERROR - $'cmd: --size: numeric sizeX value expected'
	EXEC	cmd "$usage" --foo
		OUTPUT - $'return=: option= name=--foo num=0 str=--foo'
		ERROR - $'cmd: --foo: unknown option'
	EXEC	cmd "$usage" --foo=bar
		OUTPUT - $'return=: option= name=--foo num=0 str=--foo=bar'
		ERROR - $'cmd: --foo: unknown option'
	EXEC	cmd "$usage" --version
		OUTPUT - $'return=? option=- name=--version num=0'
		ERROR - $'  version         testopt (AT&T Research) 1999-02-02'
		EXIT 2
	EXEC	cmd "$usage" '--?-'
		OUTPUT - $'return=? option=-? name=--?- num=0'
		ERROR - $'  version         testopt (AT&T Research) 1999-02-02\n  author          Glenn Fowler <gsf@research.att.com>'
	EXEC	cmd "$usage" '--??about'
		OUTPUT - $'return=? option=-? name=--??about num=0'
	EXEC	cmd "$usage" '--about'
		OUTPUT - $'return=? option=- name=--about num=0'
	EXEC	cmd "$usage" '--?--'
		OUTPUT - $'return=? option=-? name=--?-- num=0'
		ERROR - $'  version         testopt (AT&T Research) 1999-02-02\n  author          Glenn Fowler <gsf@research.att.com>\n  dictionary      tests:opt'
	EXEC	cmd "$usage" -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: cmd [-Vvn] [-x[algorithm]] [ -y ] [-d path] [-z sizeX] pattern ...'
	EXEC	cmd "$usage" --?
		OUTPUT - $'return=? option=-? name=--? num=0'
		ERROR - $'Usage: cmd [ options ] pattern ...
OPTIONS
  -x, --method[=algorithm]
                  One of
                    a     method a
                    b     method b
                    c     method c
                  The option value may be omitted.
  -y, --yes       Yes.
  --no            No.
  -d, --database=path
                  File database path.
  -z, --size=sizeX
                  Important size.
  -V, --vernum    List program version and exit.
  -v, --verbose   Enable verbose trace.
  -n, --show      Show but don\'t execute.'
	EXEC	cmd "$usage" --??api
		OUTPUT - $'return=? option=-? name=--??api num=0'
		ERROR - $'.SH SYNOPSIS\ncmd [ options ] pattern ...\n.SH OPTIONS\n.OP x method string:ignorecase:oneof:optional algorithm\nOne of\n.H2 a\nmethod a\n.H2 b\nmethod b\n.H2 c\nmethod c\n.SP\nThe option value may be omitted.\n.OP - - oneof\n.OP y yes flag -\nYes.\n.OP - no flag -\nNo.\n.OP - - anyof\n.OP d database string path\nFile database path.\n.OP z size number sizeX\nImportant size.\n.OP V vernum flag -\nList program version and exit.\n.OP v verbose flag -\nEnable verbose trace.\n.OP n show flag -\nShow but don\'t execute.\n.SH IMPLEMENTATION\n.H0 version\ntestopt (AT&T Research) 1999-02-02\n.H0 author\nGlenn Fowler <gsf@research.att.com>'
	EXEC	cmd "$usage" --?help
		OUTPUT - $'return=? option=-? name=--?help num=0'
		ERROR - $'Usage: cmd [ options ]
OPTIONS
  --help          List detailed help option info.'
	EXEC	cmd "$usage" --??help
		OUTPUT - $'return=? option=-? name=--??help num=0'
		ERROR - $'NAME
  options available to all ast commands

SYNOPSIS
  cmd [ options ]

DESCRIPTION
  -? and --?* options are the same for all ast commands. For any item below, if
  --item is not supported by a given command then it is equivalent to --??item.
  The --?? form should be used for portability. All output is written to the
  standard error.

OPTIONS
  --about         List all implementation info.
  --api           List detailed info in program readable form.
  --help          List detailed help option info.
  --html          List detailed info in html.
  --keys          List the usage translation key strings with C style escapes.
  --long          List long option usage.
  --man           List detailed info in displayed man page form.
  --nroff         List detailed info in nroff.
  --options       List short and long option details.
  --posix         List posix getopt usage.
  --short         List short option usage.
  --usage         List the usage string with C style escapes.
  --?-label       List implementation info matching label*.
  --?name         Equivalent to --help=name.
  --?             Equivalent to --??options.
  --??            Equivalent to --??man.
  --???           Equivalent to --??help.
  --???item       If the next argument is --option then list the option output
                  in the item style. Otherwise print version=n where n>0 if
                  --??item is supported, 0 if not.
  --???ESC        Emit escape codes even if output is not a terminal.
  --???MAN[=section]
                  List the man(1) section title for section
                  [the current command].
  --???SECTION    List the man(1) section number for the current command.
  --???TEST       Massage the output for regression testing.'
	EXEC	cmd "$usage" --???
		OUTPUT - $'return=? option=-? name=--??? num=0'
	EXEC	cmd "$usage" --??usage
		OUTPUT - $'return=? option=-? name=--??usage num=0'
		ERROR - $'[-?\\n@(#)testopt (AT&T Research) 1999-02-02\\n][-author?Glenn Fowler <gsf@research.att.com>][--dictionary?tests:opt][x:method?One of]:?[algorithm:oneof:ignorecase]{[+a?method a][+b?method b][+c?method c]}[\\n[y=10:yes?Yes.][20:no?No.]\\n][d:database?File database path.]:[path][z:size?Important size.]#[sizeX][V:vernum?List program version and exit.][v:verbose?Enable verbose trace.][n:show?Show but don\\\'t execute.]\\n\\npattern ...'
	EXEC	head $'[-n?\n@(#)$Id: head (AT&T Research) 2010-04-22 $\n][-author?bozo]' --version
		OUTPUT - $'return=? option=- name=--version num=0'
		ERROR - $'  version         head (AT&T Research) 2010-04-22'

TEST 03 'info callback'
	usage=$'[+][x:method?One of \fmethods\f.]:?[algorithm][m:meta?Enable metachars.]\n\npattern ...'
	EXEC	cmd "$usage" --?met
		OUTPUT - $'return=? option=-? name=--?met num=0'
		ERROR - $'Usage: cmd [ options ] pattern ...
OPTIONS
  -x, --method[=algorithm]
                  One of <* methods info ok *>. The option value may be
                  omitted.
  -m, --meta      Enable metachars.'
		EXIT 2
	usage=$'[-][+NAME?stty][a:aaa?AAA][z:zzz?ZZZ]:[path][+\fone\f?\ftwo\f]{[+?\fthree\f]}'
	EXEC	cmd "$usage" --man
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - $'NAME
  stty

SYNOPSIS
  stty [ options ]

OPTIONS
  -a, --aaa       AAA
  -z, --zzz=path  ZZZ

<* one info ok *>
  <* two info ok *>
          <* three info ok *>'
	usage=$'[-][+NAME?stty][z:zzz?ZZZ]:[path]{\fzero\f}[a:aaa?AAA][+\fone\f?\ftwo\f]{[+?\fthree\f]}'
	EXEC	cmd "$usage" --man
		ERROR - $'NAME
  stty

SYNOPSIS
  stty [ options ]

OPTIONS
  -z, --zzz=path  ZZZ
                    yabba dabba
                    doo   aroni
  -a, --aaa       AAA

<* one info ok *>
  <* two info ok *>
          <* three info ok *>'
	usage=$'[-][+NAME?aha - just do it][+DESCRIPTION?Bla bla.]{\fzero\f}\n\n[ dialect ]\n\n[+SEE ALSO?Bla.]'
	EXEC aha "$usage" --man
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - $'NAME
  aha - just do it

SYNOPSIS
  aha [ options ] [ dialect ]

DESCRIPTION
  Bla bla.
    yabba dabba
    doo   aroni

SEE ALSO
  Bla.'
	usage=$'[-][+NAME?aha - just do it][+DESCRIPTION?Bla bla.]{[+before?insert]\fzero\f[+after?append]}'
	EXEC	aha "$usage" --man
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - $'NAME
  aha - just do it

SYNOPSIS
  aha [ options ]

DESCRIPTION
  Bla bla.
    before
          insert
    yabba dabba
    doo   aroni
    after append'
	usage=$'[-][+NAME?aha - just do it][+DESCRIPTION?Bla bla.][o]:?[option?bla bla bla:]{[+before?insert]\fzero\f[+after?append]}'
	EXEC	aha "$usage" --man
		ERROR - $'NAME
  aha - just do it

SYNOPSIS
  aha [ options ]

DESCRIPTION
  Bla bla.

OPTIONS
  -o[option]      bla bla bla:
                    before
                          insert
                    yabba dabba
                    doo   aroni
                    after append
                  The option value may be omitted.'
	usage=$'[-][+NAME?Color_t - Color enum type][+Color_t?Values are \fmore#5\f.  The default value is \fmore#6\f.]'
	EXEC	Color_t "$usage" --man
		ERROR - $'NAME
  Color_t - Color enum type

SYNOPSIS
  Color_t [ options ]

Color_t
  Values are red, orange, yellow, green, blue, indigo, violet. The default
  value is red.'

TEST 04 'oh man'
	usage=$'
	[-?@(#)sum (AT&T Research) 1999-01-23\n]
	[-author?Glenn Fowler <gsf@research.att.com>]
	[-author?David Korn <dgk@research.att.com>]
	[-copyright?Copyright (c) 1989-1999 AT&T Corp.]
	[-license?http://www.research.att.com/sw/license/ast-open.html]
	[+NAME?\f?\f - write file checksums and sizes]
	[+DESCRIPTION?\b\f?\f\b calculates and writes to the standard
		output a checksum and the number of bytes in each file.
		Alternate algorithms can be specified as an option.
		The default algorithm is POSIX CRC.]
	[+?This should start another paragraph under DESCRIPTION.]
	[v:v*erbose?List verbose information on the algorithm. This
		can get really long and really wordy.]
	[V:vernum?List the program version and exit.]
	[a:each?List sum for each file when used with --total.]
	[l:file-list?Each file is a list of file names to sum.]
	[r:recursive?Recursively sum the files in directories.]
	[10:furby?Use fuzzy logic.]
	[t:total?Print the sum of the concatenation of all files.]
	[m:magic?Magic incantation.]#?[yyxzy]
	[x:algorithm|method?The checksum algorithm to apply.]:
		[algorithm:=att:oneof:ignorecase]
		{
			[+att|sys5|s5?The traditional system 5 sum.]
			{
				[+one?Test sub sub 1.]
				[+two?Test sub sub 2.]
			}
			[+ast?The ast pseudo-random number generator sum.]
			[+md5?The message digest checksum.]
		}
	[:woo?Short test 0.]:[junk-0]
	[A?Short test 1.]:[junk-1]
	[B:?Short test 2.]:[junk-2]
	[C:junk?Short test 3.]:[junk-3]
	[D:?Short test 4.]:[junk-4]
	[E]:[junk-5]
	[F:]:[junk-6]
	[G?]:[junk-7]
	[H:?]:[junk-8]
	[Q:test?Enable test code -- could be buggy.]

	file file
	file [ ... dir ]

	[+STANDARD OUTPUT FORMAT?"%u %d %s\\n" <\achecksum\a> <\apathname\a>]
	[+?If no \afile\a operand is specified, the pathname and its leading
		space are omitted.]
	[+FILES?These files are referenced whether [you]] like it [or not.]]]
	{
		[+lib/file/magic?Magic number table.]
		[+lib/find/find.codes?Fast find database.]
	}
	[+JUNK]
	{
		[+lib/file/magic?Magic number table.]
		[+lib/locate/locate.db?GNU fast find database.]
	}
	[+AND ALSO MORE JUNK TOO?First paragraph.]
	[+?Next paragraph.]
'
	EXEC	sum "$usage" '-?'
		EXIT 2
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: sum [-vValrtQ] [-m[yyxzy]] [-x algorithm] [-A junk-1] [-B junk-2]
           [-C junk-3] [-D junk-4] [-E junk-5] [-F junk-6] [-G junk-7]
           [-H junk-8] file file
   Or: sum [ options ] file [ ... dir ]'
	EXEC	sum "$usage" '--?'
		OUTPUT - $'return=? option=-? name=--? num=0'
		ERROR - $'Usage: sum [ options ] file file
   Or: sum [ options ] file [ ... dir ]
OPTIONS
  -v, --v*erbose  List verbose information on the algorithm. This can get
                  really long and really wordy.
  -V, --vernum    List the program version and exit.
  -a, --each      List sum for each file when used with --total.
  -l, --file-list Each file is a list of file names to sum.
  -r, --recursive Recursively sum the files in directories.
  --furby         Use fuzzy logic.
  -t, --total     Print the sum of the concatenation of all files.
  -m, --magic[=yyxzy]
                  Magic incantation. The option value may be omitted.
  -x, --algorithm|method=algorithm
                  The checksum algorithm to apply.
                    att|sys5|s5
                          The traditional system 5 sum.
                            one   Test sub sub 1.
                            two   Test sub sub 2.
                    ast   The ast pseudo-random number generator sum.
                    md5   The message digest checksum.
                  The default value is att.
  --woo=junk-0    Short test 0.
  -A junk-1       Short test 1.
  -B junk-2       Short test 2.
  -C, --junk=junk-3
                  Short test 3.
  -D junk-4       Short test 4.
  -E junk-5
  -F junk-6
  -G junk-7
  -H junk-8
  -Q, --test      Enable test code -- could be buggy.'
	EXEC	sum "$usage" '--??'
		OUTPUT - $'return=? option=-? name=--?? num=0'
		ERROR - $'NAME
  sum - write file checksums and sizes

SYNOPSIS
  sum [ options ] file file
  sum [ options ] file [ ... dir ]

DESCRIPTION
  sum calculates and writes to the standard output a checksum and the number of
  bytes in each file. Alternate algorithms can be specified as an option. The
  default algorithm is POSIX CRC.

  This should start another paragraph under DESCRIPTION.

OPTIONS
  -v, --v*erbose  List verbose information on the algorithm. This can get
                  really long and really wordy.
  -V, --vernum    List the program version and exit.
  -a, --each      List sum for each file when used with --total.
  -l, --file-list Each file is a list of file names to sum.
  -r, --recursive Recursively sum the files in directories.
  --furby         Use fuzzy logic.
  -t, --total     Print the sum of the concatenation of all files.
  -m, --magic[=yyxzy]
                  Magic incantation. The option value may be omitted.
  -x, --algorithm|method=algorithm
                  The checksum algorithm to apply.
                    att|sys5|s5
                          The traditional system 5 sum.
                            one   Test sub sub 1.
                            two   Test sub sub 2.
                    ast   The ast pseudo-random number generator sum.
                    md5   The message digest checksum.
                  The default value is att.
  --woo=junk-0    Short test 0.
  -A junk-1       Short test 1.
  -B junk-2       Short test 2.
  -C, --junk=junk-3
                  Short test 3.
  -D junk-4       Short test 4.
  -E junk-5
  -F junk-6
  -G junk-7
  -H junk-8
  -Q, --test      Enable test code -- could be buggy.

STANDARD OUTPUT FORMAT
  "%u %d %s\\n" <checksum> <pathname>

  If no file operand is specified, the pathname and its leading space are
  omitted.

FILES
  These files are referenced whether [you] like it [or not.]
    lib/file/magic
          Magic number table.
    lib/find/find.codes
          Fast find database.

JUNK
    lib/file/magic
          Magic number table.
    lib/locate/locate.db
          GNU fast find database.

AND ALSO MORE JUNK TOO
  First paragraph.

  Next paragraph.

IMPLEMENTATION
  version         sum (AT&T Research) 1999-01-23
  author          Glenn Fowler <gsf@research.att.com>
  author          David Korn <dgk@research.att.com>
  copyright       Copyright (c) 1989-1999 AT&T Corp.
  license         http://www.research.att.com/sw/license/ast-open.html'
	EXEC	sum "$usage" '--?' -
		OUTPUT - $'return=? option=-? name=--? num=0'
		ERROR - $'  version         sum (AT&T Research) 1999-01-23
  author          Glenn Fowler <gsf@research.att.com>
  author          David Korn <dgk@research.att.com>
  copyright       Copyright (c) 1989-1999 AT&T Corp.
  license         http://www.research.att.com/sw/license/ast-open.html'
	EXEC	sum "$usage" '--?' -ver
		OUTPUT - $'return=? option=-? name=--? num=0'
		ERROR - $'  version         sum (AT&T Research) 1999-01-23'
	EXEC	sum "$usage" '--?' -copyright
		OUTPUT - $'return=? option=-? name=--? num=0'
		ERROR - $'  copyright       Copyright (c) 1989-1999 AT&T Corp.'
	EXEC	sum "$usage" '--?' -license
		OUTPUT - $'return=? option=-? name=--? num=0'
		ERROR - $'  license         http://www.research.att.com/sw/license/ast-open.html'
	EXEC	sum "$usage" '--?' -author
		OUTPUT - $'return=? option=-? name=--? num=0'
		ERROR - $'  author          Glenn Fowler <gsf@research.att.com>
  author          David Korn <dgk@research.att.com>'
	EXEC	sum "$usage" '--?' -foo
		EXIT 1
		OUTPUT - $'return=: option=-? name=--? num=0'
		ERROR - $'sum: -foo: unknown option'
	EXEC	sum "$usage" -m A
		ERROR -
		OUTPUT - $'return=m option=-m name=-m arg=(null) num=0\nargument=1 value="A"'
		EXIT 0
	EXEC	sum "$usage" -m L A
		OUTPUT - $'return=m option=-m name=-m arg=(null) num=0\nargument=1 value="L"\nargument=2 value="A"'
	EXEC	sum "$usage" -m 0 A
		OUTPUT - $'return=m option=-m name=-m arg=0 num=0\nargument=1 value="A"'

TEST 05 'should at least generate its own man page'
	usage=$'[-?@(#)getopts (AT&T Research) 1999-02-02\n]
[+NAME?\f?\f - parse utility options]
[a:command]:[name?Use \aname\a instead of the command name in usage messages.]

opstring name [args...]

[+DESCRIPTION?The \bgetopts\b utility can be used to retrieve options and
arguments from a list of arguments give by \aargs\a or the positional
parameters if \aargs\a is omitted.  It can also generate usage messages
and a man page for the command based on the information in \aoptstring\a.]
[+?The \aoptstring\a string consists of alpha-numeric characters,
the special characters +, -, ?, :, and <space>, or character groups
enclosed in [...]].  Character groups may be nested in {...}.
Outside of a [...]] group, a single new-line followed by zero or
more blanks is ignored.  One or more blank lines separates the
options from the command argument synopsis.]
[+?Each [...]] group consists of an optional label,
optional attributes separated by :, and an
optional description string following ?.  The characters from the ?
to the end of the next ]] are ignored for option parsing and short
usage messages.  They are used for generating verbose help or man pages.
The : character may not appear in the label.
The ? character must be specified as ?? in label and the ]] character
must be specified as ]]]] in the description string.
Text between two \\b (backspace) characters indicates
that the text should be emboldened when displayed.
Text between two \\a (bell) characters indicates that the text should
be emphasised or italicised when displayed.]
[+?There are four types of groups:]
{
	[+1.?An option specifiation of the form \aoption\a:\alongname\a.
	In this case the first field is the option character.  If there
	is no option character, then a two digit number should be specified
	that corresponds to the long options.  This negative of this number
	will be returned as the value of \aname\a by \bgetopts\b if the long
	option is matched. A longname is matched with \b--\b\alongname\a.  A
	* in the \alongname\a field indicates that only characters up that
	point need to match provided any additional characters match the option.
	The [ and ]] can be omitted for options that don\'t have longnames
	or descriptive text.]
	[+2.?A string option argument specification.
	Options that take arguments can be followed by : or # and an option
	group specification.  An option group specification consists
	of a name for the option argument as field 1.   The remaining
	fields are a typename and zero or more of the special attribute words
	\blistof\b, \boneof\b, and \bignorecase\b.
	The option specification can be followed
	by a list of option value descriptions enclosed in parenthesis.]
	[+3.?A option value description.]
	[+4.?A argument specification. A list of valid option argument values
		can be specified by enclosing them inside a {...} following
		the option argument specification.  Each of the permitted
		values can be specified with a [...]] containing the
		value followed by a description.]
}
[+?If the leading character of \aoptstring\a is +, then arguments
beginning with + will also be considered options.]
[+?A leading : character or a : following a leading + in \aoptstring\a
affects the way errors are handled.  If an option character or longname
argument not specified in \aoptstring\a is encountered when processing
options, the shell variable whose name is \aname\a will be set to the ?
character.  The shell variable \bOPTARG\b will be set to
the character found.  If an option argument is missing or has an invalid
value, then \aname\a will be set to the : character and the shell variable
\bOPTARG\b will be set to the option character found.
Without the leading :, \aname\a will be set to the ? character, \bOPTARG\b
will be unset, and an error message will be written to standard error
when errors are encountered.]
[+?The end of options occurs when:]
{
	[+1.?The special argument \b--\b.]
	[+2.?An argument that does not beging with a \b-\b.]
	[+3.?A help argument is specified.]
	[+4.?An error is encountered.]
}
[+?If \bOPTARG\b is set to the value \b1\b, a new set of arguments
can be used.]
[+?\bgetopts\b can also be used to generate help messages containing command
usage and detailed descriptions.  Specify \aargs\a as:]
{
	[+-???To generate a usage synopsis.]
	[+--?????To generate a verbose usage message.]
	[+--????man?To generate a formatted man page.]
	[+--????api?To generate an easy to parse usage message.]
	[+--????html?To generate a man page in \bhtml\b format.]
}
[+?When the end of options is encountered, \bgetopts\b exits with a
non-zero return value and the variable \bOPTIND\b is set to the
index of the first non-option argument.]
[+EXIT STATUS]
{
	[+0?An option specified was found.]
	[+1?An end of options was encountered.]
	[+2?A usage or information message was generated.]
}
'
	getopts_usage=$usage
	EXEC	getopts "$usage" '-?'
		EXIT 2
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: getopts [-a name] opstring name [args...]'
	EXEC	getopts "$usage" '--?'
		OUTPUT - $'return=? option=-? name=--? num=0'
		ERROR - $'Usage: getopts [ options ] opstring name [args...]
OPTIONS
  -a, --command=name
                  Use name instead of the command name in usage messages.'
	EXEC	getopts "$usage" '--??'
		OUTPUT - $'return=? option=-? name=--?? num=0'
		ERROR - $'NAME
  getopts - parse utility options

SYNOPSIS
  getopts [ options ] opstring name [args...]

OPTIONS
  -a, --command=name
                  Use name instead of the command name in usage messages.

DESCRIPTION
  The getopts utility can be used to retrieve options and arguments from a list
  of arguments give by args or the positional parameters if args is omitted. It
  can also generate usage messages and a man page for the command based on the
  information in optstring.

  The optstring string consists of alpha-numeric characters, the special
  characters +, -, ?, :, and <space>, or character groups enclosed in [...].
  Character groups may be nested in {...}. Outside of a [...] group, a single
  new-line followed by zero or more blanks is ignored. One or more blank lines
  separates the options from the command argument synopsis.

  Each [...] group consists of an optional label, optional attributes separated
  by :, and an optional description string following ?. The characters from the
  ? to the end of the next ] are ignored for option parsing and short usage
  messages. They are used for generating verbose help or man pages. The :
  character may not appear in the label. The ? character must be specified as
  ?? in label and the ] character must be specified as ]] in the description
  string. Text between two \\b (backspace) characters indicates that the text
  should be emboldened when displayed. Text between two \\a (bell) characters
  indicates that the text should be emphasised or italicised when displayed.

  There are four types of groups:
    1.    An option specifiation of the form option:longname. In this case the
          first field is the option character. If there is no option character,
          then a two digit number should be specified that corresponds to the
          long options. This negative of this number will be returned as the
          value of name by getopts if the long option is matched. A longname is
          matched with --longname. A * in the longname field indicates that
          only characters up that point need to match provided any additional
          characters match the option. The [ and ] can be omitted for options
          that don\'t have longnames or descriptive text.
    2.    A string option argument specification. Options that take arguments
          can be followed by : or # and an option group specification. An
          option group specification consists of a name for the option argument
          as field 1. The remaining fields are a typename and zero or more of
          the special attribute words listof, oneof, and ignorecase. The option
          specification can be followed by a list of option value descriptions
          enclosed in parenthesis.
    3.    A option value description.
    4.    A argument specification. A list of valid option argument values can
          be specified by enclosing them inside a {...} following the option
          argument specification. Each of the permitted values can be specified
          with a [...] containing the value followed by a description.

  If the leading character of optstring is +, then arguments beginning with +
  will also be considered options.

  A leading : character or a : following a leading + in optstring affects the
  way errors are handled. If an option character or longname argument not
  specified in optstring is encountered when processing options, the shell
  variable whose name is name will be set to the ? character. The shell
  variable OPTARG will be set to the character found. If an option argument is
  missing or has an invalid value, then name will be set to the : character and
  the shell variable OPTARG will be set to the option character found. Without
  the leading :, name will be set to the ? character, OPTARG will be unset, and
  an error message will be written to standard error when errors are
  encountered.

  The end of options occurs when:
    1.    The special argument --.
    2.    An argument that does not beging with a -.
    3.    A help argument is specified.
    4.    An error is encountered.

  If OPTARG is set to the value 1, a new set of arguments can be used.

  getopts can also be used to generate help messages containing command usage
  and detailed descriptions. Specify args as:
    -?    To generate a usage synopsis.
    --??  To generate a verbose usage message.
    --??man
          To generate a formatted man page.
    --??api
          To generate an easy to parse usage message.
    --??html
          To generate a man page in html format.

  When the end of options is encountered, getopts exits with a non-zero return
  value and the variable OPTIND is set to the index of the first non-option
  argument.

EXIT STATUS
    0     An option specified was found.
    1     An end of options was encountered.
    2     A usage or information message was generated.

IMPLEMENTATION
  version         getopts (AT&T Research) 1999-02-02'
	EXEC	getopts "$usage" '--??api'
		OUTPUT - $'return=? option=-? name=--??api num=0'
		ERROR - $'.SH NAME
getopts - parse utility options
.SH SYNOPSIS
getopts [ options ] opstring name [args...]
.SH OPTIONS
.OP a command string name
Use name instead of the command name in usage messages.
.SH DESCRIPTION
The getopts utility can be used to retrieve options and arguments from a list of arguments give by args or the positional parameters if args is omitted. It can also generate usage messages and a man page for the command based on the information in optstring.
.PP
The optstring string consists of alpha-numeric characters, the special characters +, -, ?, :, and <space>, or character groups enclosed in [...]. Character groups may be nested in {...}. Outside of a [...] group, a single new-line followed by zero or more blanks is ignored. One or more blank lines separates the options from the command argument synopsis.
.PP
Each [...] group consists of an optional label, optional attributes separated by :, and an optional description string following ?. The characters from the ? to the end of the next ] are ignored for option parsing and short usage messages. They are used for generating verbose help or man pages. The : character may not appear in the label. The ? character must be specified as ?? in label and the ] character must be specified as ]] in the description string. Text between two \\b (backspace) characters indicates that the text should be emboldened when displayed. Text between two \\a (bell) characters indicates that the text should be emphasised or italicised when displayed.
.PP
There are four types of groups:
.H1 1.
An option specifiation of the form option:longname. In this case the first field is the option character. If there is no option character, then a two digit number should be specified that corresponds to the long options. This negative of this number will be returned as the value of name by getopts if the long option is matched. A longname is matched with --longname. A * in the longname field indicates that only characters up that point need to match provided any additional characters match the option. The [ and ] can be omitted for options that don\'t have longnames or descriptive text.
.H1 2.
A string option argument specification. Options that take arguments can be followed by : or # and an option group specification. An option group specification consists of a name for the option argument as field 1. The remaining fields are a typename and zero or more of the special attribute words listof, oneof, and ignorecase. The option specification can be followed by a list of option value descriptions enclosed in parenthesis.
.H1 3.
A option value description.
.H1 4.
A argument specification. A list of valid option argument values can be specified by enclosing them inside a {...} following the option argument specification. Each of the permitted values can be specified with a [...] containing the value followed by a description.
.PP
If the leading character of optstring is +, then arguments beginning with + will also be considered options.
.PP
A leading : character or a : following a leading + in optstring affects the way errors are handled. If an option character or longname argument not specified in optstring is encountered when processing options, the shell variable whose name is name will be set to the ? character. The shell variable OPTARG will be set to the character found. If an option argument is missing or has an invalid value, then name will be set to the : character and the shell variable OPTARG will be set to the option character found. Without the leading :, name will be set to the ? character, OPTARG will be unset, and an error message will be written to standard error when errors are encountered.
.PP
The end of options occurs when:
.H1 1.
The special argument --.
.H1 2.
An argument that does not beging with a -.
.H1 3.
A help argument is specified.
.H1 4.
An error is encountered.
.PP
If OPTARG is set to the value 1, a new set of arguments can be used.
.PP
getopts can also be used to generate help messages containing command usage and detailed descriptions. Specify args as:
.H1 -?
To generate a usage synopsis.
.H1 --??
To generate a verbose usage message.
.H1 --??man
To generate a formatted man page.
.H1 --??api
To generate an easy to parse usage message.
.H1 --??html
To generate a man page in html format.
.PP
When the end of options is encountered, getopts exits with a non-zero return value and the variable OPTIND is set to the index of the first non-option argument.
.SH EXIT STATUS
.H1 0
An option specified was found.
.H1 1
An end of options was encountered.
.H1 2
A usage or information message was generated.
.SH IMPLEMENTATION
.H0 version
getopts (AT&T Research) 1999-02-02'
	EXEC	getopts "$usage" '--??html'
		OUTPUT - $'return=? option=-? name=--??html num=0'
		ERROR - $'<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<HEAD>
<META name="generator" content="optget (AT&T Research) 2011-11-11">
<TITLE>getopts man document</TITLE>
<STYLE type="text/css">
div.SH { padding-left:2em; text-indent:0em; }
div.SY { padding-left:4em; text-indent:-2em; }
dt { float:left; clear:both; }
dd { margin-left:3em; }
</STYLE>
</HEAD>
<BODY bgcolor=white>
<H4><TABLE width=100%><TR><TH align=left>GETOPTS&nbsp;(&nbsp;1&nbsp;)&nbsp;<TH align=center><A href="." title="Index">USER COMMANDS</A><TH align=right>GETOPTS&nbsp;(&nbsp;1&nbsp;)</TR></TABLE></H4>
<HR>
<H4><A name="NAME">NAME</A></H4>
<DIV class=SH>
<!--MAN-INDEX-->getopts - parse utility options
<P>
</DIV>
<H4><A name="SYNOPSIS">SYNOPSIS</A></H4>
<DIV class=SY>
<B>getopts</B> &#0091; <I>options</I> &#0093; opstring name &#0091;args...&#0093;
<P>
</DIV>
<H4><A name="OPTIONS">OPTIONS</A></H4>
<DIV class=SH>
<DL>
<DT>-<B>a</B>, --<B>command</B>=<I>name</I><DD><BR>Use <I>name</I> instead of the
command name in usage messages.
</DL>
</DIV>
<H4><A name="DESCRIPTION">DESCRIPTION</A></H4>
<DIV class=SH>
The <B>getopts</B> utility can be used to retrieve options and arguments from a
list of arguments give by <I>args</I> or the positional parameters if <I>args
</I> is omitted. It can also generate usage messages and a man page for the
command based on the information in <I>optstring</I>.
<P>
The <I>optstring</I> string consists of alpha-numeric characters, the special
characters +, -, ?, :, and &lt;space&gt;, or character groups enclosed in
&#0091;...&#0093;. Character groups may be nested in {...}. Outside of a &#0091;...&#0093; group, a
single new-line followed by zero or more blanks is ignored. One or more blank
lines separates the options from the command argument synopsis.
<P>
Each &#0091;...&#0093; group consists of an optional label, optional attributes separated
by :, and an optional description string following ?. The characters from the ?
to the end of the next &#0093; are ignored for option parsing and short usage
messages. They are used for generating verbose help or man pages. The :
character may not appear in the label. The ? character must be specified as ??
in label and the &#0093; character must be specified as &#0093;&#0093; in the description string.
Text between two \\b (backspace) characters indicates that the text should be
emboldened when displayed. Text between two \\a (bell) characters indicates that
the text should be emphasised or italicised when displayed.
<P>
There are four types of groups:
<DL>
<DT><A name="1."><B>1.</B></A><DD>An option specifiation of the form <I>option</I>:
<I>longname</I>. In this case the first field is the option character. If there
is no option character, then a two digit number should be specified that
corresponds to the long options. This negative of this number will be returned
as the value of <I>name</I> by <B>getopts</B> if the long option is matched. A
longname is matched with <B>--</B><I>longname</I>. A * in the <I>longname</I>
field indicates that only characters up that point need to match provided any
additional characters match the option. The &#0091; and &#0093; can be omitted for options
that don\'t have longnames or descriptive text.
<DT><A name="2."><B>2.</B></A><DD>A string option argument specification. Options
that take arguments can be followed by : or # and an option group
specification. An option group specification consists of a name for the option
argument as field 1. The remaining fields are a typename and zero or more of
the special attribute words <B>listof</B>, <B>oneof</B>, and <B>ignorecase</B>.
The option specification can be followed by a list of option value descriptions
enclosed in parenthesis.
<DT><A name="3."><B>3.</B></A><DD>A option value description.
<DT><A name="4."><B>4.</B></A><DD>A argument specification. A list of valid option
argument values can be specified by enclosing them inside a {...} following the
option argument specification. Each of the permitted values can be specified
with a &#0091;...&#0093; containing the value followed by a description.
<P>
</DL>
If the leading character of <I>optstring</I> is +, then arguments beginning
with + will also be considered options.
<P>
A leading : character or a : following a leading + in <I>optstring</I> affects
the way errors are handled. If an option character or longname argument not
specified in <I>optstring</I> is encountered when processing options, the shell
variable whose name is <I>name</I> will be set to the ? character. The shell
variable <B>OPTARG</B> will be set to the character found. If an option
argument is missing or has an invalid value, then <I>name</I> will be set to
the : character and the shell variable <B>OPTARG</B> will be set to the option
character found. Without the leading :, <I>name</I> will be set to the ?
character, <B>OPTARG</B> will be unset, and an error message will be written to
standard error when errors are encountered.
<P>
The end of options occurs when:
<DL>
<DT><A name="1."><B>1.</B></A><DD>The special argument <B>--</B>.
<DT><A name="2."><B>2.</B></A><DD>An argument that does not beging with a <B>-</B>.
<DT><A name="3."><B>3.</B></A><DD>A help argument is specified.
<DT><A name="4."><B>4.</B></A><DD>An error is encountered.
<P>
</DL>
If <B>OPTARG</B> is set to the value <B>1</B>, a new set of arguments can be
used.
<P>
<B>getopts</B> can also be used to generate help messages containing command
usage and detailed descriptions. Specify <I>args</I> as:
<DL>
<DT><A name="-?"><B>-?</B></A><DD>To generate a usage synopsis.
<DT><A name="--??"><B>--??</B></A><DD>To generate a verbose usage message.
<DT><A name="--??man"><B>--??man</B></A><DD><BR>To generate a formatted man page.
<DT><A name="--??api"><B>--??api</B></A><DD><BR>To generate an easy to parse usage
message.
<DT><A name="--??html"><B>--??html</B></A><DD><BR>To generate a man page in <B>html
</B> format.
<P>
</DL>
When the end of options is encountered, <B>getopts</B> exits with a non-zero
return value and the variable <B>OPTIND</B> is set to the index of the first
non-option argument.
</DIV>
<H4><A name="EXIT STATUS">EXIT STATUS</A></H4>
<DIV class=SH>
<DL>
<DT><A name="0"><B>0</B></A><DD>An option specified was found.
<DT><A name="1"><B>1</B></A><DD>An end of options was encountered.
<DT><A name="2"><B>2</B></A><DD>A usage or information message was generated.
</DL>
</DIV>
<H4><A name="IMPLEMENTATION">IMPLEMENTATION</A></H4>
<DIV class=SH>
<DL>
<DT><A name="version"><B>version</B></A><DD><BR>getopts (AT&amp;T Research)
1999-02-02
</DL>
</DIV>
</BODY>
</HTML>'
	EXEC	getopts "$usage" '--version'
		OUTPUT - $'return=? option=- name=--version num=0'
		ERROR - $'  version         getopts (AT&T Research) 1999-02-02'
	usage=$'[-][a:aaa?AAA]:[delimiter:=\\::][b:bbb?BBB]:[string:=??][c]:[ccc:==??::=]'
	EXEC	tooalso "$usage" --man
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - $'SYNOPSIS
  tooalso [ options ]

OPTIONS
  -a, --aaa=delimiter
                  AAA The default value is \\:.
  -b, --bbb=string
                  BBB The default value is ?.
  -c ccc The default value is =?:=.'

TEST 06 'bugs of omission'
	usage=$'[-][+NAME?locate][a:again?Look again.][b:noback?Don\'t look back.][n:notnow?Don\'t look now.][t:twice?Look twice.][+FOO]{[+foo?aha]}[+BAR?bahah]'
	EXEC	locate "$usage" '-?'
		EXIT 2
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: locate [-abnt]'
	EXEC	locate "$usage" '--?'
		OUTPUT - $'return=? option=-? name=--? num=0'
		ERROR - $'Usage: locate [ options ]
OPTIONS
  -a, --again     Look again.
  -b, --noback    Don\'t look back.
  -n, --notnow    Don\'t look now.
  -t, --twice     Look twice.'
	EXEC	locate "$usage" '--man'
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - $'NAME
  locate

SYNOPSIS
  locate [ options ]

OPTIONS
  -a, --again     Look again.
  -b, --noback    Don\'t look back.
  -n, --notnow    Don\'t look now.
  -t, --twice     Look twice.

FOO
    foo   aha

BAR
  bahah'

TEST 07 'return value tests'
	usage=$'[-][a=1111:aaa]\t[b=2:bbb]\t[=3:ccc][44:ddd][i!:iii][j!:jjj?Yada yada.]'
	EXEC tst "$usage" -a
		OUTPUT - $'return=-1111 option=-1111 name=-a arg=(null) num=1'
	EXEC tst "$usage" --a
		OUTPUT - $'return=-1111 option=-1111 name=--aaa arg=(null) num=1'
	EXEC tst "$usage" -b
		OUTPUT - $'return=-2 option=-2 name=-b arg=(null) num=1'
	EXEC tst "$usage" --b
		OUTPUT - $'return=-2 option=-2 name=--bbb arg=(null) num=1'
	EXEC tst "$usage" --c
		OUTPUT - $'return=-3 option=-3 name=--ccc arg=(null) num=1'
	EXEC tst "$usage" --d
		OUTPUT - $'return=-44 option=-44 name=--ddd arg=(null) num=1'
	EXEC tst "$usage" -a -b --a --b --c --d -i --i --noi
		OUTPUT - $'return=-1111 option=-1111 name=-a arg=(null) num=1
return=-2 option=-2 name=-b arg=(null) num=1
return=-1111 option=-1111 name=--aaa arg=(null) num=1
return=-2 option=-2 name=--bbb arg=(null) num=1
return=-3 option=-3 name=--ccc arg=(null) num=1
return=-44 option=-44 name=--ddd arg=(null) num=1
return=i option=-i name=-i arg=(null) num=0
return=i option=-i name=--iii arg=(null) num=1
return=i option=-i name=--iii arg=(null) num=0'
	EXEC tst "$usage" '-?'
		EXIT 2
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: tst [-abij]'
	EXEC tst "$usage" '--?'
		OUTPUT - $'return=? option=-? name=--? num=0'
		ERROR - $'Usage: tst [ options ]\nOPTIONS\n  -a, --aaa\n  -b, --bbb\n  --ccc\n  --ddd\n  -i, --iii\n  -j, --jjj       Yada yada. On by default; -j means --nojjj.'
	EXEC tst "$usage" '--??api'
		OUTPUT - $'return=? option=-? name=--??api num=0'
		ERROR - $'.SH SYNOPSIS
tst [ options ]
.SH OPTIONS
.OP a aaa flag -
.OP b bbb flag -
.OP - ccc flag -
.OP - ddd flag -
.OP i iii flag:invert -
.OP j jjj flag:invert -
Yada yada.
 On by default; -j means --nojjj.
.SH IMPLEMENTATION
.SP'

TEST 08 'optstr() tests'
	usage=$'[-][a:aaa?AAA][v:vvv?VVV]:[xxx]'
	EXEC	- dll "$usage" 'vvv=zzz foo: aaa bar: aaa'
		OUTPUT - $'return=v option=-v name=vvv arg=zzz num=1
return=# option=: name=foo arg=(null) num=0
return=a option=-a name=aaa arg=(null) num=1
return=# option=: name=bar arg=(null) num=0
return=a option=-a name=aaa arg=(null) num=1'
	EXEC	- dll "$usage" '???TEST,???NOEMPHASIS,man'
		EXIT 2
		OUTPUT - $'return=? option=-? name=man num=0'
		ERROR - $'SYNOPSIS
  dll [ options ]

OPTIONS
  -a, --aaa       AAA
  -v, --vvv=xxx   VVV'
	usage=$'[-?@(#)pax (AT&T Research) 1999-02-14\n]
[a:append?Append to end of archive.]
[101:atime?Preserve or set access time.]:?[time]
[z:base?Delta base archive name. - ignores base on input, compresses on output.]:[archive]
[b:blocksize?Input/output block size. The default is format specific.]#[size]
[102:blok?Input/output BLOK format for tapes on file.]:?[i|o]
[103:charset|codeset?Header data character set name.]:[name]

[ file ... ]

[-author?Glenn Fowler <gsf@research.att.com>]
[-license?http://www.research.att.com/sw/tools/reuse]
'
	EXEC	- pax "$usage" 'append base="aaa zzz" charset=us'
		EXIT 0
		OUTPUT - $'return=a option=-a name=append arg=(null) num=1
return=z option=-z name=base arg=aaa zzz num=1
return=-103 option=-103 name=charset arg=us num=1'
		ERROR -
	EXEC	- pax "$usage" '--append --base="aaa zzz" --charset=us'
	EXEC	- pax "$usage" '-a -zaaa'
		OUTPUT - $'return=a option=-a name=-a arg=(null) num=1
return=z option=-z name=-z arg=aaa num=1'

	EXEC	- pax "$usage" 'app,base=aaa\,zzz,ch:=us\ ascii,block+=77777777777'
		OUTPUT - $'return=a option=-a name=append arg=(null) num=1
return=z option=-z name=base arg=aaa,zzz num=1
return=-103 option=-103 name=charset arg:=us ascii num=1
return=b option=-b name=blocksize arg+=77777777777 num=77777777777'
	EXEC	- pax "$usage" $'6 app\n16 base=aaa,zzz\n21 charset:=us ascii\n22 block+=77777777777\n'
	EXEC	- pax "$usage" $'6 app\n16 base=aaa,zzz\n21 charset:=us ascii\n22 block+=77777777777'
	EXEC	- pax "$usage" '--app,--base=aaa\,zzz,--ch:=us\ ascii,--block+=77777777777'
	EXEC	- pax "$usage" '-a,-zaaa,-b77777777777'
		OUTPUT - $'return=a option=-a name=-a arg=(null) num=1
return=z option=-z name=-z arg=aaa num=1
return=b option=-b name=-b arg=77777777777 num=77777777777'

	EXEC	- pax "$usage" 'app,base=aaa\,zzz,ch:=us\ ascii,block+=077777777777'
		OUTPUT - $'return=a option=-a name=append arg=(null) num=1
return=z option=-z name=base arg=aaa,zzz num=1
return=-103 option=-103 name=charset arg:=us ascii num=1
return=b option=-b name=blocksize arg+=077777777777 num=77777777777'
	EXEC	- pax "$usage" $'6 app\n16 base=aaa,zzz\n21 charset:=us ascii\n23 block+=077777777777\n'
	EXEC	- pax "$usage" $'6 app\n16 base=aaa,zzz\n21 charset:=us ascii\n23 block+=077777777777'
	EXEC	- pax "$usage" '--app,--base=aaa\,zzz,--ch:=us\ ascii,--block+=077777777777'
	EXEC	- pax "$usage" '-a,-zaaa,-b077777777777'
		OUTPUT - $'return=a option=-a name=-a arg=(null) num=1
return=z option=-z name=-z arg=aaa num=1
return=b option=-b name=-b arg=077777777777 num=77777777777'

	EXEC	- pax "$usage" 'app,base=aaa\,zzz,ch:=us\ ascii,block+=8#77777777777'
		OUTPUT - $'return=a option=-a name=append arg=(null) num=1
return=z option=-z name=base arg=aaa,zzz num=1
return=-103 option=-103 name=charset arg:=us ascii num=1
return=b option=-b name=blocksize arg+=8#77777777777 num=8589934591'
	EXEC	- pax "$usage" $'6 app\n16 base=aaa,zzz\n21 charset:=us ascii\n24 block+=8#77777777777\n'
	EXEC	- pax "$usage" $'6 app\n16 base=aaa,zzz\n21 charset:=us ascii\n24 block+=8#77777777777'
	EXEC	- pax "$usage" '--app,--base=aaa\,zzz,--ch:=us\ ascii,--block+=8#77777777777'
	EXEC	- pax "$usage" '-a,-zaaa,-b8#77777777777'
		OUTPUT - $'return=a option=-a name=-a arg=(null) num=1
return=z option=-z name=-z arg=aaa num=1
return=b option=-b name=-b arg=8#77777777777 num=8589934591'

	EXEC	- pax "$usage" 'app,base=aaa\,zzz,ch:=us\ ascii,block+=0x77777777777'
		OUTPUT - $'return=a option=-a name=append arg=(null) num=1
return=z option=-z name=base arg=aaa,zzz num=1
return=-103 option=-103 name=charset arg:=us ascii num=1
return=b option=-b name=blocksize arg+=0x77777777777 num=8209686820727'
	EXEC	- pax "$usage" $'6 app\n16 base=aaa,zzz\n21 charset:=us ascii\n24 block+=0x77777777777\n'
	EXEC	- pax "$usage" $'6 app\n16 base=aaa,zzz\n21 charset:=us ascii\n24 block+=0x77777777777'
	EXEC	- pax "$usage" '--app,--base=aaa\,zzz,--ch:=us\ ascii,--block+=0x77777777777'
	EXEC	- pax "$usage" '-a,-zaaa,-b0x77777777777'
		OUTPUT - $'return=a option=-a name=-a arg=(null) num=1
return=z option=-z name=-z arg=aaa num=1
return=b option=-b name=-b arg=0x77777777777 num=8209686820727'

	EXEC	- pax "$usage" 'app,base=aaa\,zzz,ch:=us\ ascii,block+=16#77777777777'
		OUTPUT - $'return=a option=-a name=append arg=(null) num=1
return=z option=-z name=base arg=aaa,zzz num=1
return=-103 option=-103 name=charset arg:=us ascii num=1
return=b option=-b name=blocksize arg+=16#77777777777 num=8209686820727'
	EXEC	- pax "$usage" $'6 app\n16 base=aaa,zzz\n21 charset:=us ascii\n25 block+=16#77777777777\n'
	EXEC	- pax "$usage" $'6 app\n16 base=aaa,zzz\n21 charset:=us ascii\n25 block+=16#77777777777'
	EXEC	- pax "$usage" '--app,--base=aaa\,zzz,--ch:=us\ ascii,--block+=16#77777777777'
	EXEC	- pax "$usage" '-a,-zaaa,-b16#77777777777'
		OUTPUT - $'return=a option=-a name=-a arg=(null) num=1
return=z option=-z name=-z arg=aaa num=1
return=b option=-b name=-b arg=16#77777777777 num=8209686820727'

	EXEC	- pax "$usage" '14 foo'
		EXIT 1
		OUTPUT - $'return=: option= name=14 num=0 str=14 foo\nreturn=: option= name=foo num=0 str=14 foo'
		ERROR - $'pax: 14: unknown option\npax: foo: unknown option'

TEST 09 'hidden options'
	usage=$'ab?cd [ file ... ]'
	EXEC tst "$usage" -a -b -c -d
		OUTPUT - $'return=a option=-a name=-a arg=(null) num=1
return=b option=-b name=-b arg=(null) num=1
return=c option=-c name=-c arg=(null) num=1
return=d option=-d name=-d arg=(null) num=1'
	EXEC tst "$usage" -?
		EXIT 2
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: tst [-ab] [ file ... ]'
	EXEC tst "$usage" --?
		OUTPUT - $'return=? option=-? name=--? num=0'
	EXEC tst "$usage" --api
		OUTPUT - $'return=? option=- name=--api num=0'
		ERROR - $'.SH SYNOPSIS
tst [ options ] [ file ... ]
.SH OPTIONS
.OP a - flag -
.OP b - flag -
.OP c - flag:hidden -
.OP d - flag:hidden -'
	usage=$'ab:[arg]?c'
	EXEC tst "$usage" -a -b z -c
		EXIT 0
		OUTPUT - $'return=a option=-a name=-a arg=(null) num=1
return=b option=-b name=-b arg=z num=1
return=c option=-c name=-c arg=(null) num=1'
		ERROR -
	EXEC tst "$usage" -?
		EXIT 2
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: tst [-a] [-b arg]'
	EXEC tst "$usage" --?
		OUTPUT - $'return=? option=-? name=--? num=0'
	EXEC tst "$usage" --api
		OUTPUT - $'return=? option=- name=--api num=0'
		ERROR - $'.SH SYNOPSIS
tst [ options ]
.SH OPTIONS
.OP a - flag -
.OP b - string arg
.OP c - flag:hidden -'

TEST 10 'numeric options'
	new=$'[-n][b:ignoreblanks][K:oldkey?Obsolete]#[old-key]'
	alt=$'[-n][b:ignoreblanks][K|X:oldkey?Obsolete]#[old-key]'
	old=$'bK#[old-key]'
	EXEC sort "$new" -1 +2 file
		OUTPUT - $'return=K option=-K name=-1 arg=1 num=1\nreturn=K option=+K name=+2 arg=2 num=2\nargument=1 value="file"'
	EXEC sort "$alt" -1 +2 file
	EXEC sort "$old" -1 +2 file
		OUTPUT - $'return=: option= name=-1 num=0 str=bK#[old-key]\nargument=1 value="+2"\nargument=2 value="file"'
		ERROR - $'sort: -1: unknown option'
		EXIT 1

	usage=$'[-][x:trace?Trace.][m:max?Max.]#[num]'
	EXEC set "$usage" -x
		OUTPUT - $'return=x option=-x name=-x arg=(null) num=1'
		ERROR -
		EXIT 0
	EXEC set "$usage" +x
		OUTPUT - $'argument=1 value="+x"'
	EXEC set "$usage" -4
		OUTPUT - $'return=: option= name=-4 num=0 str=[-][x:trace?Trace.][m:max?Max.]#[num]'
		ERROR - $'set: -4: unknown option'
		EXIT 1
	EXEC set "$usage" +4
		OUTPUT - $'argument=1 value="+4"'
		ERROR -
		EXIT 0

	usage=$'+[-][x:trace?Trace.][m:max?Max.]#[num]'
	EXEC set "$usage" -x
		OUTPUT - $'return=x option=-x name=-x arg=(null) num=1'
	EXEC set "$usage" +x
		OUTPUT - $'return=x option=+x name=+x arg=(null) num=1'
		ERROR -
	EXEC set "$usage" -4
		OUTPUT - $'return=: option= name=-4 num=0 str=+[-][x:trace?Trace.][m:max?Max.]#[num]'
		ERROR - $'set: -4: unknown option'
		EXIT 1
	EXEC set "$usage" +4
		OUTPUT - $'return=: option= name=+4 num=0 str=+[-][x:trace?Trace.][m:max?Max.]#[num]'
		ERROR - $'set: +4: unknown option'

	usage=$'[-+][x:trace?Trace.][m:max?Max.]#[num]'
	EXEC set "$usage" -x
		OUTPUT - $'return=x option=-x name=-x arg=(null) num=1'
		ERROR -
		EXIT 0
	EXEC set "$usage" +x
		OUTPUT - $'return=x option=+x name=+x arg=(null) num=1'
	EXEC set "$usage" -4
		OUTPUT - $'return=: option= name=-4 num=0 str=[-+][x:trace?Trace.][m:max?Max.]#[num]'
		ERROR - $'set: -4: unknown option'
		EXIT 1
	EXEC set "$usage" +4
		OUTPUT - $'return=: option= name=+4 num=0 str=[-+][x:trace?Trace.][m:max?Max.]#[num]'
		ERROR - $'set: +4: unknown option'

	usage=$'+[-n][x:trace?Trace.][m:max?Max.]#[num]'
	EXEC set "$usage" -x
		OUTPUT - $'return=x option=-x name=-x arg=(null) num=1'
		ERROR -
		EXIT 0
	EXEC set "$usage" +x
		OUTPUT - $'return=x option=+x name=+x arg=(null) num=1'
	EXEC set "$usage" -4
		OUTPUT - $'return=m option=-m name=-4 arg=4 num=4'
	EXEC set "$usage" +4
		OUTPUT - $'return=m option=+m name=+4 arg=4 num=4'

	usage=$'[-+n][x:trace?Trace.][m:max?Max.]#[num]'
	EXEC set "$usage" -x
		OUTPUT - $'return=x option=-x name=-x arg=(null) num=1'
		ERROR -
		EXIT 0
	EXEC set "$usage" +x
		OUTPUT - $'return=x option=+x name=+x arg=(null) num=1'
	EXEC set "$usage" -4
		OUTPUT - $'return=m option=-m name=-4 arg=4 num=4'
	EXEC set "$usage" +4
		OUTPUT - $'return=m option=+m name=+4 arg=4 num=4'

TEST 11 'find style!'
	find=$'[-1p1?@(#)find (AT&T Research) 2010-04-22][+NAME?\bfind\b - find files][13:amin?File was last accessed \aminutes\a minutes ago.]#[minutes][17:chop?Chop leading \b./\b from printed pathnames.]\n\n[ path ... ] [ option ]\n\n[+SEE ALSO?cpio(1), file(1), ls(1), sh(1), test(1), tw(1), stat(2)]'
	EXEC	find "$find" -amin 1 -chop
		OUTPUT - $'return=-13 option=-13 name=-amin arg=1 num=1\nreturn=-17 option=-17 name=-chop arg=(null) num=1'
	EXEC	find "$find" -amin=1 --chop
	EXEC	find "$find" --amin 1 --chop
	EXEC	find "$find" -amin
		OUTPUT - $'return=: option=-13 name=-amin num=0'
		ERROR - $'find: -amin: numeric minutes value expected'
		EXIT 1
	EXEC	find "$find" -foo
		OUTPUT - $'return=: option= name=-foo num=0 str=-foo'
		ERROR - $'find: -foo: unknown option'
	EXEC	find "$find" --foo
		OUTPUT - $'return=: option= name=-foo num=0 str=--foo'
		ERROR - $'find: -foo: unknown option'
	EXEC	find "$find" -foo=bar
		OUTPUT - $'return=: option= name=-foo num=0 str=-foo=bar'
		ERROR - $'find: -foo: unknown option'
	EXEC	find "$find" --foo=bar
		OUTPUT - $'return=: option= name=-foo num=0 str=--foo=bar'
		ERROR - $'find: -foo: unknown option'
	EXEC	find "$find" -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: find [-amin minutes] [-chop] [ path ... ] [ option ]'
		EXIT 2
	EXEC	find "$find" --?
		ERROR - $'Usage: find [ options ] [ path ... ] [ option ]\nOPTIONS\n  -amin minutes   File was last accessed minutes minutes ago.\n  -chop           Chop leading ./ from printed pathnames.'
	EXEC	find "$find" --man
		OUTPUT - $'return=? option=- name=-man num=0'
		ERROR - $'NAME\n  find - find files\n\nSYNOPSIS\n  find [ options ] [ path ... ] [ option ]\n\nOPTIONS\n  -amin minutes   File was last accessed minutes minutes ago.\n  -chop           Chop leading ./ from printed pathnames.\n\nSEE ALSO\n  cpio(1), file(1), ls(1), sh(1), test(1), tw(1), stat(2)\n\nIMPLEMENTATION\n  version         find (AT&T Research) 2010-04-22'
	EXEC	find "$find" --nroff
		OUTPUT - $'return=? option=- name=-nroff num=0'
		ERROR - $'.\\" format with nroff|troff|groff -man
.TH FIND 1 2010-04-22
.fp 5 CW
.nr mH 5
.de H0
.nr mH 0
.in 5n
\\fB\\\\$1\\fP
.in 7n
..
.de H1
.nr mH 1
.in 7n
\\fB\\\\$1\\fP
.in 9n
..
.de H2
.nr mH 2
.in 11n
\\fB\\\\$1\\fP
.in 13n
..
.de H3
.nr mH 3
.in 15n
\\fB\\\\$1\\fP
.in 17n
..
.de H4
.nr mH 4
.in 19n
\\fB\\\\$1\\fP
.in 21n
..
.de OP
.nr mH 0
.ie !\'\\\\$1\'-\' \\{
.ds mO \\\\fB\\\\-\\\\$1\\\\fP
.ds mS ,\\\\0
.\\}
.el \\{
.ds mO \\\\&
.ds mS \\\\&
.\\}
.ie \'\\\\$2\'-\' \\{
.if !\'\\\\$4\'-\' .as mO \\\\0\\\\fI\\\\$4\\\\fP
.\\}
.el \\{
.as mO \\\\*(mS\\\\fB\\\\-\\\\$2\\\\fP
.if !\'\\\\$4\'-\' .as mO =\\\\fI\\\\$4\\\\fP
.\\}
.in 5n
\\\\*(mO
.in 9n
..
.de SP
.if \\\\n(mH==2 .in 9n
.if \\\\n(mH==3 .in 13n
.if \\\\n(mH==4 .in 17n
..
.de FN
.nr mH 0
.in 5n
\\\\$1 \\\\$2
.in 9n
..
.de DS
.in +3n
.ft 5
.nf
..
.de DE
.fi
.ft R
.in -3n
..
.SH NAME
\\fBfind\\fP \\- find files
.SH SYNOPSIS
\\fBfind\\fP\\ [\\ \\fIoptions\\fP\\ ]\\ [\\ path\\ \\&.\\&.\\&.\\ ]\\ [\\ option\\ ]
.SH OPTIONS
.OP - amin number minutes
File was last accessed \\fIminutes\\fP minutes ago\\&.
.OP - chop flag -
Chop leading \\fB\\&./\\fP from printed pathnames\\&.
.SH SEE\\ ALSO
cpio(1), file(1), ls(1), sh(1), test(1), tw(1), stat(2)
.SH IMPLEMENTATION
.H0 version
find (AT&T Research) 2010\\-04\\-22'

TEST 12 'dd style!'
	dd=$'[-1p0][+NAME?\bdd\b - copy and convert file][10:if?Input file name.]:[file][11:conv?Conversion option.]:[conversion][+SEE ALSO?cp(1), pax(1), tr(1), seek(2)]'
	EXEC	dd "$dd" if=foo conv=ascii
		OUTPUT - $'return=-10 option=-10 name=if arg=foo num=1\nreturn=-11 option=-11 name=conv arg=ascii num=1'
	EXEC	dd "$dd" if foo conv ascii
	EXEC	dd "$dd" -if=foo --conv=ascii
	EXEC	dd "$dd" if
		OUTPUT - $'return=: option=-10 name=if num=0'
		ERROR - $'dd: if: file argument expected'
		EXIT 1
	EXEC	dd "$dd" --if=bar foo
		OUTPUT - $'return=-10 option=-10 name=if arg=bar num=1\nreturn=: option= name=foo num=0 str=foo'
		ERROR - $'dd: foo: unknown option'
	EXEC	dd "$dd" -foo
		OUTPUT - $'return=: option= name=foo num=0 str=-foo'
	EXEC	dd "$dd" --foo
		OUTPUT - $'return=: option= name=foo num=0 str=--foo'
	EXEC	dd "$dd" foo=bar
		OUTPUT - $'return=: option= name=foo num=0 str=foo=bar'
	EXEC	dd "$dd" -foo=bar
		OUTPUT - $'return=: option= name=foo num=0 str=-foo=bar'
	EXEC	dd "$dd" --foo=bar
		OUTPUT - $'return=: option= name=foo num=0 str=--foo=bar'
	EXEC	dd "$dd" -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: dd [if=file] [conv=conversion]'
		EXIT 2
	EXEC	dd "$dd" --?
		OUTPUT - $'return=? option=-? name=? num=0'
		ERROR - $'Usage: dd [ options ]\nOPTIONS\n  if=file         Input file name.\n  conv=conversion Conversion option.'
	EXEC	dd "$dd" --man
		OUTPUT - $'return=? option=- name=man num=0'
		ERROR - $'NAME\n  dd - copy and convert file\n\nSYNOPSIS\n  dd [ options ]\n\nOPTIONS\n  if=file         Input file name.\n  conv=conversion Conversion option.\n\nSEE ALSO\n  cp(1), pax(1), tr(1), seek(2)'

TEST 13 'unknown options'
	usage=$'[-][l:library]:[dll][G!:gzip] [ file ]'
	EXEC	pzip "$usage" --library=mps --checksum --nogzip data.db
		OUTPUT - $'return=l option=-l name=--library arg=mps num=1\nreturn=: option= name=--checksum num=0 str=--checksum\nreturn=G option=-G name=--gzip arg=(null) num=0\nargument=1 value="data.db"'
		ERROR - $'pzip: --checksum: unknown option'
		EXIT 1
	EXEC	pzip "$usage" --library=mps --checksum=1 --nogzip data.db
		OUTPUT - $'return=l option=-l name=--library arg=mps num=1\nreturn=: option= name=--checksum num=0 str=--checksum=1\nreturn=G option=-G name=--gzip arg=(null) num=0\nargument=1 value="data.db"'
	EXEC	pzip "$usage" --library=mps --nochecksum --nogzip data.db
		OUTPUT - $'return=l option=-l name=--library arg=mps num=1\nreturn=: option= name=--nochecksum num=0 str=--nochecksum\nreturn=G option=-G name=--gzip arg=(null) num=0\nargument=1 value="data.db"'
		ERROR - $'pzip: --nochecksum: unknown option'

TEST 14 'interface queries'
	usage=$'[-][l:library]:[dll][G!:gzip] [ file ]'
	EXEC	command "$usage" --???api
		OUTPUT - $'return=? option=-? name=--???api num=0'
		ERROR - $'version=1'
		EXIT 2
	EXEC	command "$usage" --???html
		OUTPUT - $'return=? option=-? name=--???html num=0'
	EXEC	command "$usage" --???man
		OUTPUT - $'return=? option=-? name=--???man num=0'
	usage=$'[-p2][l:library]:[dll][G!:gzip] [ file ]'
	EXEC	command "$usage" --???api
		OUTPUT - $'return=? option=-? name=--???api num=0'
		ERROR - $'version=1'
		EXIT 2
	usage=$'[-2p2][l:library]:[dll][G!:gzip] [ file ]'
	EXEC	command "$usage" --???api
		OUTPUT - $'return=? option=-? name=--???api num=0'
		ERROR - $'version=2'
		EXIT 2
	usage=$'l:[dll]G [ file ]'
	EXEC	command "$usage" --???api
		OUTPUT - $'return=? option=-? name=--???api num=0'
		ERROR - $'version=0'
		EXIT 2
	EXEC	command "$usage" --???html
		OUTPUT - $'return=? option=-? name=--???html num=0'
	EXEC	command "$usage" --???man
		OUTPUT - $'return=? option=-? name=--???man num=0'
	EXEC	command "$usage" --???MAN=3S
		OUTPUT - $'return=? option=-? name=--???MAN num=0'
		ERROR - $'STANDARD I/O FUNCTIONS'

TEST 15 'required vs. optional arguments'
	usage=$'[-][r:required]:[value][o:optional]:?[value][2:aha]'
	EXEC	cmd "$usage" --req=1 --req -2 --req 3 4 5
		OUTPUT - $'return=r option=-r name=--required arg=1 num=1\nreturn=r option=-r name=--required arg=-2 num=1\nreturn=r option=-r name=--required arg=3 num=1\nargument=1 value="4"\nargument=2 value="5"'
	EXEC	cmd "$usage" -r1 -r -2 -r 3 4 5
		OUTPUT - $'return=r option=-r name=-r arg=1 num=1\nreturn=r option=-r name=-r arg=-2 num=1\nreturn=r option=-r name=-r arg=3 num=1\nargument=1 value="4"\nargument=2 value="5"'
	EXEC	cmd "$usage" --opt=1 --opt -2 --opt 3 4 5
		OUTPUT - $'return=o option=-o name=--optional arg=1 num=1\nreturn=o option=-o name=--optional arg=(null) num=1\nreturn=2 option=-2 name=-2 arg=(null) num=1\nreturn=o option=-o name=--optional arg=(null) num=1\nargument=1 value="3"\nargument=2 value="4"\nargument=3 value="5"'
	EXEC	cmd "$usage" -o1 -o -2 -o 3 4 5
		OUTPUT - $'return=o option=-o name=-o arg=1 num=1\nreturn=o option=-o name=-o arg=(null) num=0\nreturn=2 option=-2 name=-2 arg=(null) num=1\nreturn=o option=-o name=-o arg=3 num=1\nargument=1 value="4"\nargument=2 value="5"'
	usage=$'[-][y:sort?sort by key]:?[key]{[a?A]}'
	EXEC	ls "$usage" -y
		OUTPUT - $'return=y option=-y name=-y arg=(null) num=1'

TEST 16 'detailed man'
	usage=$'[-][+NAME?ah][+DESCRIPTION?\abla\a does bla and bla. The blas are:]{[+\aaha\a?bla bla aha][+\bbwaha?not bold][+bold?yes it is]}[+?Next paragraph]{[+aaa?aaa][+bbb?bbb]}'
	EXEC	cmd "$usage" --html
		OUTPUT - $'return=? option=- name=--html num=0'
		ERROR - $'<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<HEAD>
<META name="generator" content="optget (AT&T Research) 2011-11-11">
<TITLE>ah man document</TITLE>
<STYLE type="text/css">
div.SH { padding-left:2em; text-indent:0em; }
div.SY { padding-left:4em; text-indent:-2em; }
dt { float:left; clear:both; }
dd { margin-left:3em; }
</STYLE>
</HEAD>
<BODY bgcolor=white>
<H4><TABLE width=100%><TR><TH align=left>AH&nbsp;(&nbsp;1&nbsp;)&nbsp;<TH align=center><A href="." title="Index">USER COMMANDS</A><TH align=right>AH&nbsp;(&nbsp;1&nbsp;)</TR></TABLE></H4>
<HR>
<H4><A name="NAME">NAME</A></H4>
<DIV class=SH>
<!--MAN-INDEX-->ah
<P>
</DIV>
<H4><A name="SYNOPSIS">SYNOPSIS</A></H4>
<DIV class=SY>
<B>ah</B> &#0091; <I>options</I> &#0093;
</DIV>
<H4><A name="DESCRIPTION">DESCRIPTION</A></H4>
<DIV class=SH>
<I>bla</I> does bla and bla. The blas are:
<DL>
<DT><A name="aha"><I>aha</I></A><DD>bla bla aha
<DT><A name="bwaha">bwaha</A><DD><BR>not bold
<DT><A name="bold"><B>bold</B></A><DD>yes it is
<P>
</DL>
Next paragraph
<DL>
<DT><A name="aaa"><B>aaa</B></A><DD>aaa
<DT><A name="bbb"><B>bbb</B></A><DD>bbb
</DL>
</DIV>
</BODY>
</HTML>'
		EXIT 2

TEST 17 'literal : ? ] makess you ssee double'
	usage=$'[-][a:aha?magic\\sesame]:[yyzzy][x?foo]:[bar]{[+a[b::c[d[,e]]]]??f[::g]]]]?A?B:C[]]][+p \'d\' "q"?duh]}[z:zoom]'
	EXEC	zwei "$usage" -a o
		OUTPUT - $'return=a option=-a name=-a arg=o num=1'
	EXEC	zwei "$usage" -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: zwei [-z] [-a yyzzy] [-x bar]'
		EXIT 2
	EXEC	zwei "$usage" --api
		OUTPUT - $'return=? option=- name=--api num=0'
		ERROR - $'.SH SYNOPSIS
zwei [ options ]
.SH OPTIONS
.OP a aha string yyzzy
magic\\sesame
.OP x - string bar
foo
.H2 a[b:c[d[,e]]?f[:g]]
A?B:C[]
.H2 p \'d\' "q"
duh
.OP z zoom flag -
.SH IMPLEMENTATION
.SP'
	EXEC	zwei "$usage" --nroff
		OUTPUT - $'return=? option=- name=--nroff num=0'
		ERROR - $'.\\" format with nroff|troff|groff -man
.TH ZWEI 1
.fp 5 CW
.nr mH 5
.de H0
.nr mH 0
.in 5n
\\fB\\\\$1\\fP
.in 7n
..
.de H1
.nr mH 1
.in 7n
\\fB\\\\$1\\fP
.in 9n
..
.de H2
.nr mH 2
.in 11n
\\fB\\\\$1\\fP
.in 13n
..
.de H3
.nr mH 3
.in 15n
\\fB\\\\$1\\fP
.in 17n
..
.de H4
.nr mH 4
.in 19n
\\fB\\\\$1\\fP
.in 21n
..
.de OP
.nr mH 0
.ie !\'\\\\$1\'-\' \\{
.ds mO \\\\fB\\\\-\\\\$1\\\\fP
.ds mS ,\\\\0
.\\}
.el \\{
.ds mO \\\\&
.ds mS \\\\&
.\\}
.ie \'\\\\$2\'-\' \\{
.if !\'\\\\$4\'-\' .as mO \\\\0\\\\fI\\\\$4\\\\fP
.\\}
.el \\{
.as mO \\\\*(mS\\\\fB\\\\-\\\\-\\\\$2\\\\fP
.if !\'\\\\$4\'-\' .as mO =\\\\fI\\\\$4\\\\fP
.\\}
.in 5n
\\\\*(mO
.in 9n
..
.de SP
.if \\\\n(mH==2 .in 9n
.if \\\\n(mH==3 .in 13n
.if \\\\n(mH==4 .in 17n
..
.de FN
.nr mH 0
.in 5n
\\\\$1 \\\\$2
.in 9n
..
.de DS
.in +3n
.ft 5
.nf
..
.de DE
.fi
.ft R
.in -3n
..
.SH SYNOPSIS
\\fBzwei\\fP\\ [\\ \\fIoptions\\fP\\ ]
.SH OPTIONS
.OP a aha string yyzzy
magic\\esesame
.OP x - string bar
foo
.H2 a[b:c[d[,e]]?f[:g]]
A?B:C[]
.H2 p\\ \'d\'\\ "q"
duh
.OP z zoom flag -
.SH IMPLEMENTATION
.SP'

TEST 18 'more compatibility'
	usage=$'CD:[macro[=value]]EI:?[dir]MPU:[macro]V?A:[assertion]HTX:[dialect]Y:[stdinclude] [input [output]]'
	EXEC	cpp "$usage" -Da=b -Ic -I -Id
		OUTPUT - $'return=D option=-D name=-D arg=a=b num=1\nreturn=I option=-I name=-I arg=c num=1\nreturn=I option=-I name=-I arg=(null) num=0\nreturn=I option=-I name=-I arg=d num=1'
	EXEC	cpp "$usage" -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: cpp [-CEMPV] [-D macro[=value]] [-I[dir]] [-U macro] [input [output]]'
		EXIT 2
	usage=$'abc'
	EXEC	typeset "$usage" -
		OUTPUT - $'argument=1 value="-"'
		ERROR -
		EXIT 0
	EXEC	typeset "+$usage" -
	EXEC	typeset "$usage" +
		OUTPUT - $'argument=1 value="+"'
	EXEC	typeset "+$usage" +
	usage=$' file'
	EXEC	none "$usage" -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: none [ options ] file'
		EXIT 2
	EXEC	none "$usage" --api
		OUTPUT - $'return=? option=- name=--api num=0'
		ERROR - $'.SH SYNOPSIS\nnone [ options ] file'
	usage=$'[-]\n\nfile'
	EXEC	none "$usage" -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: none [ options ] file'
	EXEC	none "$usage" --api
		OUTPUT - $'return=? option=- name=--api num=0'
		ERROR - $'.SH SYNOPSIS\nnone [ options ] file\n.SH IMPLEMENTATION\n.SP'

TEST 19 'mutual exclusion'
	usage=$'[-][\t[a:A][b:B][c:C]\t][d:D]\n\n[ file ... ]'
	EXEC	mutex "$usage" -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: mutex [-d] [ -a | -b | -c ] [ file ... ]'
		EXIT 2
	usage=$'[-] [ [a:A][b:B][c:C] ] [d:D]\n\n[ file ... ]'
	EXEC	mutex "$usage" -?
	usage=$'[abc]d [ file ... ]'
	EXEC	mutex "$usage" -?

TEST 20 'flag alternation'
	usage=$'[-][a|A:aha][b|q|z:foo][n|m=10:meth]:[meth][x:bar]\n\nfile ...'
	EXEC	alt "$usage" -a
		OUTPUT - $'return=a option=-a name=-a arg=(null) num=1'
	EXEC	alt "$usage" -A
		OUTPUT - $'return=a option=-A name=-A arg=(null) num=1'
	EXEC	alt "$usage" --aha
		OUTPUT - $'return=a option=-a name=--aha arg=(null) num=1'
	EXEC	alt "$usage" -b
		OUTPUT - $'return=b option=-b name=-b arg=(null) num=1'
	EXEC	alt "$usage" -q
		OUTPUT - $'return=b option=-q name=-q arg=(null) num=1'
	EXEC	alt "$usage" -z
		OUTPUT - $'return=b option=-z name=-z arg=(null) num=1'
	EXEC	alt "$usage" --foo
		OUTPUT - $'return=b option=-b name=--foo arg=(null) num=1'
	EXEC	alt "$usage" -n nnn
		OUTPUT - $'return=-10 option=-10 name=-n arg=nnn num=1'
	EXEC	alt "$usage" -m mmm
		OUTPUT - $'return=-10 option=-10 name=-m arg=mmm num=1'
	EXEC	alt "$usage" --meth=zzz
		OUTPUT - $'return=-10 option=-10 name=--meth arg=zzz num=1'
	EXEC	alt "$usage" -x
		OUTPUT - $'return=x option=-x name=-x arg=(null) num=1'
	EXEC	alt "$usage" --bar
		OUTPUT - $'return=x option=-x name=--bar arg=(null) num=1'
	EXEC	alt "$usage" -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: alt [-aAbqzx] [-n|m meth] file ...'
		EXIT 2
	EXEC	alt "$usage" --api
		OUTPUT - $'return=? option=- name=--api num=0'
		ERROR - $'.SH SYNOPSIS
alt [ options ] file ...
.SH OPTIONS
.OP a|A aha flag -
.OP b|q|z foo flag -
.OP n|m meth string meth
.OP x bar flag -
.SH IMPLEMENTATION
.SP'

TEST 21 'justification and emphasis'
	usage=$'[-][w!:warn?Warn about invalid \b--check\b lines.][f:format?hours:minutes:seconds.\t\aid\a may be  followed  by [maybe]] \b:case:\b\ap1\a:\as1\a:...:\apn\a:\asn\a which expands to \asi\a if]\n\n[ file ... ]'
	EXEC esc "$usage" --???TEST --man
		OUTPUT - $'return=? option=-? name=--man num=0'
		ERROR - $'\E[1mSYNOPSIS\E[0m
  \E[1mesc\E[0m [ \E[1;4moptions\E[0m ] [ file ... ]

\E[1mOPTIONS\E[0m
  -\E[1mw\E[0m, --\E[1mwarn\E[0m      Warn about invalid \E[1m--check\E[0m lines. On by default; -\E[1mw\E[0m means
                  --\E[1mnowarn\E[0m.
  -\E[1mf\E[0m, --\E[1mformat\E[0m    hours:minutes:seconds. \E[1;4mid\E[0m may be followed by [maybe]
                  \E[1m:case:\E[0m\E[1;4mp1\E[0m:\E[1;4ms1\E[0m:...:\E[1;4mpn\E[0m:\E[1;4msn\E[0m which expands to \E[1;4msi\E[0m if'
		EXIT 2
	EXEC esc "$usage" --keys
		OUTPUT - $'return=? option=- name=--keys num=0'
		ERROR - $'"warn"
"Warn about invalid \\b--check\\b lines."
"format"
"hours:minutes:seconds.\\t\\aid\\a may be  followed  by [maybe] \\b:case:\\b\\ap1\\a:\\as1\\a:...:\\apn\\a:\\asn\\a which expands to \\asi\\a if"
"[ file ... ]"'

TEST 22 'no with values'
	usage=$'[-][j:jobs?Job concurrency level.]#[level]'
	extra=$'[n!:exec?Execute shell actions.]'
	EXEC	make "$usage" -j1 -j 2 --jobs=3 --jobs 4 --nojobs 5 6
		OUTPUT - $'return=j option=-j name=-j arg=1 num=1
return=j option=-j name=-j arg=2 num=2
return=j option=-j name=--jobs arg=3 num=3
return=j option=-j name=--jobs arg=4 num=4
return=j option=-j name=--jobs arg=(null) num=0
argument=1 value="5"
argument=2 value="6"'
	EXEC	make "$usage$extra" -j1 -j 2 --jobs=3 --jobs 4 --nojobs 5 6
	EXEC	make "$usage" --nojobs=1 2
		OUTPUT - $'return=: option=-j name=--jobs num=0\nargument=1 value="2"'
		ERROR - $'make: --nojobs: value not expected'
		EXIT 1
	EXEC	make "$usage$extra" --nojobs=1 2

TEST 23 'weird help'
	usage=$'[-][j:jobs?Job concurrency level.]#[level]'
	EXEC	test "$usage" --man=-
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - ''
		EXIT 2
	EXEC	test "$usage" --man -
		ERROR - $'SYNOPSIS
  test [ options ]

OPTIONS
  -j, --jobs=level
                  Job concurrency level.'
	EXEC	test "$usage" --man --
	EXEC	test "$usage" --man
	usage=$'[-][!:expand?Compress to 32 byte record format.][!:sort?Sort detail records.]'
	EXEC	pzip "$usage" --man
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - 'SYNOPSIS
  pzip [ options ]

OPTIONS
  --expand        Compress to 32 byte record format. On by default; use
                  --noexpand to turn off.
  --sort          Sort detail records. On by default; use --nosort to turn off.'
		EXIT 2

TEST 24 'detailed html'
	usage=$'[-][-author?Glenn Fowler <gsf@research.att.com>][-copyright?Copyright (c) 1989-1999 AT&T Corp.][-license?http://www.research.att.com/sw/license/ast-proprietary.html][+NAME?\bdd\b - copy and convert file][10:if?Input file name (see \aintro\a(2)).]:[file\a (see \bstat\b(2))][11:conv?Conversion option \abegin[-end]]=value\a passed to \bmain\b().]:[conversion][+SEE ALSO?\bcp\b(1), \bpax\b(1), \btr\b(1), \bseek\b(2), \bdd::plugin\b(5P)]'
	EXEC	test "$usage" --html
		EXIT 2
		OUTPUT - $'return=? option=- name=--html num=0'
		ERROR - $'<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<HEAD>
<META name="generator" content="optget (AT&T Research) 2011-11-11">
<!--INTERNAL-->
<TITLE>dd man document</TITLE>
<STYLE type="text/css">
div.SH { padding-left:2em; text-indent:0em; }
div.SY { padding-left:4em; text-indent:-2em; }
dt { float:left; clear:both; }
dd { margin-left:3em; }
</STYLE>
</HEAD>
<BODY bgcolor=white>
<H4><TABLE width=100%><TR><TH align=left>DD&nbsp;(&nbsp;1&nbsp;)&nbsp;<TH align=center><A href="." title="Index">USER COMMANDS</A><TH align=right>DD&nbsp;(&nbsp;1&nbsp;)</TR></TABLE></H4>
<HR>
<H4><A name="NAME">NAME</A></H4>
<DIV class=SH>
<!--MAN-INDEX--><B>dd</B> - copy and convert file
<P>
</DIV>
<H4><A name="SYNOPSIS">SYNOPSIS</A></H4>
<DIV class=SY>
<B>dd</B> &#0091; <I>options</I> &#0093;
<P>
</DIV>
<H4><A name="OPTIONS">OPTIONS</A></H4>
<DIV class=SH>
<DL>
<DT>--<B>if</B>=<I>file</I> (see
<NOBR><A href="../man2/stat.html"><B>stat</B></A>(2))</NOBR><DD><BR>Input file
name (see <NOBR><A href="../man2/intro.html"><I>intro</I></A>(2)).</NOBR>
<DT>--<B>conv</B>=<I>conversion</I><DD><BR>Conversion option <I>begin&#0091;-end&#0093;=value
</I> passed to <B>main</B>().
</DL>
</DIV>
<H4><A name="SEE ALSO">SEE ALSO</A></H4>
<DIV class=SH>
<NOBR><A href="../man1/cp.html"><B>cp</B></A>(1),</NOBR>
<NOBR><A href="../man1/pax.html"><B>pax</B></A>(1),</NOBR>
<NOBR><A href="../man1/tr.html"><B>tr</B></A>(1),</NOBR>
<NOBR><A href="../man2/seek.html"><B>seek</B></A>(2),</NOBR>
<NOBR><A href="../man5P/dd-plugin.html"><B>dd::plugin</B></A>(5P)</NOBR>
</DIV>
<H4><A name="IMPLEMENTATION">IMPLEMENTATION</A></H4>
<DIV class=SH>

<DL>
<DT><A name="author"><B>author</B></A><DD><BR>Glenn Fowler &lt;<A
href="mailto:gsf@research.att.com">gsf@research.att.com</A>&gt;
<DT><A name="copyright"><B>copyright</B></A><DD><BR>Copyright &copy; 1989-1999 AT&amp;T Corp.
<DT><A name="license"><B>license</B></A><DD><BR><A href="http://www.research.att.com/sw/license/ast-proprietary.html">http://www.research.att.com/sw/license/ast-proprietary.html</A>
</DL>
</DIV>
</BODY>
</HTML>'

TEST 25 'extra args after help options'
	usage=$'[-][a:aha][b:foo][n=10:meth]:[meth][x:bar]\n\nfile ...'
	EXEC	extra "$usage" xxx
		OUTPUT - $'argument=1 value="xxx"'
	EXEC	extra "$usage" --short
		OUTPUT - $'return=? option=- name=--short num=0'
		ERROR - $'Usage: extra [-abx] [-n meth] file ...'
		EXIT 2
	EXEC	extra "$usage" --short xxx
	EXEC	extra "$usage" --??short
		OUTPUT - $'return=? option=-? name=--??short num=0'
	EXEC	extra "$usage" --??short xxx

TEST 26 'usage combinations'
	lib1=$'[-1l?library][--catalog?nyuk][-author?Dewey Cheatham][+LIBRARY?\b-loser\b - user library][b:bbb?BBB][+EXIT STATUS?]{[+0?Spread out.][+>0?Why you.]}[+SEE ALSO?\bfoo\b(1)]'
	lib2=$'[-1i?implicit][-author?Andy Howe][+LIBRARY?implicit library][z:zzz?ZZZ]'
	usage=$'[-?main][-author?Bea Taylor][+NAME?\baha\b - bwoohahahahah][a:aha?AHA] [ file ... ]'
	EXEC	+ "$lib1" + "$lib2" combo "$usage" --aha
		OUTPUT - $'return=a option=-a name=--aha arg=(null) num=1'
	EXEC	+ "$lib1" + "$lib2" combo "$usage" --bbb
		EXIT 1
		OUTPUT - $'return=: option= name=--bbb num=0 str=--bbb'
		ERROR - $'combo: --bbb: unknown option'
	EXEC	+ "$lib1" + "$lib2" combo "$usage" --aha --man
		EXIT 2
		OUTPUT - $'return=a option=-a name=--aha arg=(null) num=1\nreturn=? option=-a name=--man num=1'
		ERROR - $'NAME
  aha - bwoohahahahah

SYNOPSIS
  aha [ options ]

OPTIONS
  -a, --aha       AHA

IMPLEMENTATION
  version         main
  author          Bea Taylor

LIBRARY
  -loser - user library

OPTIONS
  --bbb           BBB

EXIT STATUS
    0     Spread out.
    >0    Why you.

SEE ALSO
  foo(1)

IMPLEMENTATION
  version         library
  author          Dewey Cheatham'
	EXEC	combo "$lib1" --bbb --?bbb
		OUTPUT - $'return=b option=-b name=--bbb arg=(null) num=1\nreturn=? option=-? name=--?bbb num=1'
		ERROR - $'Usage: combo [ options ]\nOPTIONS\n  --bbb           BBB'
	EXEC	+ "$lib1" + "$lib2" combo "$usage" --aha --?bbb
		OUTPUT - $'return=a option=-a name=--aha arg=(null) num=1\nreturn=? option=-? name=--?bbb num=1'
	EXEC	+ "$lib1" + "$lib2" combo "$usage" --aha --usage
		OUTPUT - $'return=a option=-a name=--aha arg=(null) num=1\nreturn=? option=-a name=--usage num=1'
		ERROR - $'[-1l?library][--catalog?nyuk][-author?Dewey Cheatham][+LIBRARY?\\b-loser\\b - user library][b:bbb?BBB][+EXIT STATUS?]{[+0?Spread out.][+>0?Why you.]}[+SEE ALSO?\\bfoo\\b(1)]'
	EXEC	+ "$lib1" + "$lib2" combo "$usage" --aha --keys
		OUTPUT - $'return=a option=-a name=--aha arg=(null) num=1\nreturn=? option=-a name=--keys num=1'
		ERROR - $'"\\b-loser\\b - user library"
"bbb"
"BBB"
"Spread out."
"Why you."
"\\bfoo\\b(1)"'
	usage=$'[-?main][-author?Opie][+NAME?\baha\b - bwoohahahahah][ [b:bar?BAR][f:foo?FOO] ]\n\n[ file ... ]\n\n[+EXIT STATUS?]{[+0?okeedokee][+!=0?eekodeeko]}[+SEE ALSO?\bslingshot\b(1)]'
	EXEC	rfd "$usage" --keys
		OUTPUT - $'return=? option=- name=--keys num=0'
		ERROR - $'"\\baha\\b - bwoohahahahah"
"bar"
"BAR"
"foo"
"FOO"
"[ file ... ]"
"okeedokee"
"eekodeeko"
"\\bslingshot\\b(1)"'
	usage=$'[-] \t\n[+NAME?bar][f:flag]'
	EXEC	foo "$usage" --short
		OUTPUT - $'return=? option=- name=--short num=0'
		ERROR - $'Usage: foo [-f]'
	EXEC	foo "$usage" --long
		OUTPUT - $'return=? option=- name=--long num=0'
		ERROR - $'Usage: foo [--flag]'
	EXEC	foo "$usage" --usage
		OUTPUT - $'return=? option=- name=--usage num=0'
		ERROR - $'[-] \\t\\n[+NAME?bar][f:flag]'
	EXEC	foo "$usage" --man
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - $'NAME
  bar

SYNOPSIS
  bar [ options ]

OPTIONS
  -f, --flag'
	EXEC	foo "$usage" --html
		OUTPUT - $'return=? option=- name=--html num=0'
		ERROR - $'<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<HEAD>
<META name="generator" content="optget (AT&T Research) 2011-11-11">
<TITLE>bar man document</TITLE>
<STYLE type="text/css">
div.SH { padding-left:2em; text-indent:0em; }
div.SY { padding-left:4em; text-indent:-2em; }
dt { float:left; clear:both; }
dd { margin-left:3em; }
</STYLE>
</HEAD>
<BODY bgcolor=white>
<H4><TABLE width=100%><TR><TH align=left>BAR&nbsp;(&nbsp;1&nbsp;)&nbsp;<TH align=center><A href="." title="Index">USER COMMANDS</A><TH align=right>BAR&nbsp;(&nbsp;1&nbsp;)</TR></TABLE></H4>
<HR>
<H4><A name="NAME">NAME</A></H4>
<DIV class=SH>
<!--MAN-INDEX-->bar
<P>
</DIV>
<H4><A name="SYNOPSIS">SYNOPSIS</A></H4>
<DIV class=SY>
<B>bar</B> &#0091; <I>options</I> &#0093;
<P>
</DIV>
<H4><A name="OPTIONS">OPTIONS</A></H4>
<DIV class=SH>
-<B>f</B>, --<B>flag</B>
</DIV>
</BODY>
</HTML>'

TEST 27 'opt_info.num with opt_info.arg'
	usage=$'[-][f:flag][m:must]:[yes][o:optional]:?[maybe]'
	EXEC	huh "$usage" --noflag
		OUTPUT - $'return=f option=-f name=--flag arg=(null) num=0'
	EXEC	huh "$usage" --flag
		OUTPUT - $'return=f option=-f name=--flag arg=(null) num=1'
	EXEC	huh "$usage" -f
		OUTPUT - $'return=f option=-f name=-f arg=(null) num=1'
	EXEC	huh "$usage" --nomust
		OUTPUT - $'return=m option=-m name=--must arg=(null) num=0'
	EXEC	huh "$usage" --must=ok
		OUTPUT - $'return=m option=-m name=--must arg=ok num=1'
	EXEC	huh "$usage" --must ok
	EXEC	huh "$usage" -mok
		OUTPUT - $'return=m option=-m name=-m arg=ok num=1'
	EXEC	huh "$usage" -m ok
		OUTPUT - $'return=m option=-m name=-m arg=ok num=1'
	EXEC	huh "$usage" --nooptional
		OUTPUT - $'return=o option=-o name=--optional arg=(null) num=0'
	EXEC	huh "$usage" --optional
		OUTPUT - $'return=o option=-o name=--optional arg=(null) num=1'
	EXEC	huh "$usage" --optional -f
		OUTPUT - $'return=o option=-o name=--optional arg=(null) num=1\nreturn=f option=-f name=-f arg=(null) num=1'
	EXEC	huh "$usage" -o
		OUTPUT - $'return=o option=-o name=-o arg=(null) num=1'
	EXEC	huh "$usage" -o -f
		OUTPUT - $'return=o option=-o name=-o arg=(null) num=0\nreturn=f option=-f name=-f arg=(null) num=1'
	EXEC	huh "$usage" --optional=ok
		OUTPUT - $'return=o option=-o name=--optional arg=ok num=1'
	EXEC	huh "$usage" --optional ok
		OUTPUT - $'return=o option=-o name=--optional arg=(null) num=1\nargument=1 value="ok"'
	EXEC	huh "$usage" -ook
		OUTPUT - $'return=o option=-o name=-o arg=ok num=1'
	EXEC	huh "$usage" -o ok
		OUTPUT - $'return=o option=-o name=-o arg=ok num=1'
	EXEC	huh "$usage" --must
		OUTPUT - $'return=: option=-m name=--must num=0'
		ERROR - $'huh: --must: yes value expected'
		EXIT 1

TEST 28 'user defined optget return value'
	usage=$'[-n][n:count]#[number][234:ZZZ]'
	EXEC	num "$usage" -123
		OUTPUT - $'return=n option=-n name=-1 arg=123 num=123'
	EXEC	num "$usage" -234
		OUTPUT - $'return=n option=-n name=-2 arg=234 num=234'

TEST 29 'usage stack'
	usage=$'[-?main][-author?Barney Fife][+NAME?\baha\b - bwoohahahahah][a:aha?AHA.]\foptions\f[q:what?Explain in detail.] [ file ... ]'
	EXEC	info "$usage" --aha --zoom --boom=junk --what
		OUTPUT - $'return=a option=-a name=--aha arg=(null) num=1
return=Z option=-Z name=--zoom arg=(null) num=1
return=B option=-B name=--boom arg=junk num=1
return=q option=-q name=--what arg=(null) num=1'
	EXEC	info "$usage" --man
		EXIT 2
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - $'NAME
  aha - bwoohahahahah

SYNOPSIS
  aha [ options ]

OPTIONS
  -a, --aha       AHA.
  -Z, --zoom      Do it as fast as possible.
  -C, --cram      Cram as much as possible.
  -K, --kill      kill all processes.
  -F, --fudge     Fudge the statistics to satisfy everyone.
  -D, --dump      Dump as much as possible.
  -B, --boom=file Dump into file.
  -q, --what      Explain in detail.

IMPLEMENTATION
  version         main
  author          Barney Fife'

TEST 30 'library interfaces'
	USAGE_LICENSE="[-author?Glenn Fowler <gsf@research.att.com>][-copyright?Copyright (c) 1995-1999 AT&T Corp.][-license?http://www.research.att.com/sw/license/ast-open.html]"
	usage=$'
[-1s3S?@(#)sum (AT&T Research) 1999-12-11]'$USAGE_LICENSE$'
[+NAME?sum - checksum library]
[+DESCRIPTION?\bsum\b is a checksum library.]
[Sum_t*:sumopen(const char* \amethod\a)?Open a sum handle for \amethod\a.]
[int:sumclose(Sum_t* \asum\a)?Close a sum handle \asum\a previously returned
	by \bsumopen\b.]

#include <sum.h>

[+SEE ALSO?\bcksum\b(1)]
'
	EXEC	sum "$usage" --html
		EXIT 2
		OUTPUT - $'return=? option=- name=html num=0'
		ERROR - $'<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<HEAD>
<META name="generator" content="optget (AT&T Research) 2011-11-11">
<TITLE>sum man document</TITLE>
<STYLE type="text/css">
div.SH { padding-left:2em; text-indent:0em; }
div.SY { padding-left:4em; text-indent:-2em; }
dt { float:left; clear:both; }
dd { margin-left:3em; }
</STYLE>
</HEAD>
<BODY bgcolor=white>
<H4><TABLE width=100%><TR><TH align=left>SUM&nbsp;(&nbsp;3S&nbsp;)&nbsp;<TH align=center><A href="." title="Index">STANDARD I/O FUNCTIONS</A><TH align=right>SUM&nbsp;(&nbsp;3S&nbsp;)</TR></TABLE></H4>
<HR>
<H4><A name="NAME">NAME</A></H4>
<DIV class=SH>
<!--MAN-INDEX-->sum - checksum library
<P>
</DIV>
<H4><A name="SYNOPSIS">SYNOPSIS</A></H4>
<DIV class=SY>
#include &lt;sum.h&gt;
</DIV>
<H4><A name="DESCRIPTION">DESCRIPTION</A></H4>
<DIV class=SH>
<B>sum</B> is a checksum library.
<P>
</DIV>
<H4><A name="FUNCTIONS">FUNCTIONS</A></H4>
<DIV class=SH>
<DL>
<DT>Sum_t* <B>sumopen</B>(const char* <I>method</I>)<DD><BR>Open a sum handle for
<I>method</I>.
<DT>int <B>sumclose</B>(Sum_t* <I>sum</I>)<DD><BR>Close a sum handle <I>sum</I>
previously returned by <B>sumopen</B>.
</DL>
</DIV>
<H4><A name="SEE ALSO">SEE ALSO</A></H4>
<DIV class=SH>
<NOBR><A href="../man1/cksum.html"><B>cksum</B></A>(1)</NOBR>
</DIV>
<H4><A name="IMPLEMENTATION">IMPLEMENTATION</A></H4>
<DIV class=SH>
<DL>
<DT><A name="version"><B>version</B></A><DD><BR>sum (AT&amp;T Research) 1999-12-11
<DT><A name="author"><B>author</B></A><DD><BR>Glenn Fowler &lt;<A
href="mailto:gsf@research.att.com">gsf@research.att.com</A>&gt;
<DT><A name="copyright"><B>copyright</B></A><DD><BR>Copyright &copy; 1995-1999 AT&amp;T Corp.
<DT><A name="license"><B>license</B></A><DD><BR><A href="http://www.research.att.com/sw/license/ast-open.html">http://www.research.att.com/sw/license/ast-open.html</A>
</DL>
</DIV>
</BODY>
</HTML>'
	EXEC	sum "$usage" --nroff
		OUTPUT - $'return=? option=- name=nroff num=0'
		ERROR - $'.\\" format with nroff|troff|groff -man
.TH SUM 3S 1999-12-11
.fp 5 CW
.nr mH 5
.de H0
.nr mH 0
.in 5n
\\fB\\\\$1\\fP
.in 7n
..
.de H1
.nr mH 1
.in 7n
\\fB\\\\$1\\fP
.in 9n
..
.de H2
.nr mH 2
.in 11n
\\fB\\\\$1\\fP
.in 13n
..
.de H3
.nr mH 3
.in 15n
\\fB\\\\$1\\fP
.in 17n
..
.de H4
.nr mH 4
.in 19n
\\fB\\\\$1\\fP
.in 21n
..
.de OP
.nr mH 0
.ie !\'\\\\$1\'-\' \\{
.ds mO \\\\fB\\\\-\\\\$1\\\\fP
.ds mS ,\\\\0
.\\}
.el \\{
.ds mO \\\\&
.ds mS \\\\&
.\\}
.ie \'\\\\$2\'-\' \\{
.if !\'\\\\$4\'-\' .as mO \\\\0\\\\fI\\\\$4\\\\fP
.\\}
.el \\{
.as mO \\\\*(mS\\\\fB\\\\$2\\\\fP
.if !\'\\\\$4\'-\' .as mO =\\\\fI\\\\$4\\\\fP
.\\}
.in 5n
\\\\*(mO
.in 9n
..
.de SP
.if \\\\n(mH==2 .in 9n
.if \\\\n(mH==3 .in 13n
.if \\\\n(mH==4 .in 17n
..
.de FN
.nr mH 0
.in 5n
\\\\$1 \\\\$2
.in 9n
..
.de DS
.in +3n
.ft 5
.nf
..
.de DE
.fi
.ft R
.in -3n
..
.SH NAME
sum \\- checksum library
.SH SYNOPSIS
#include\\ <sum\\&.h>
.SH DESCRIPTION
\\fBsum\\fP is a checksum library\\&.
.SH FUNCTIONS
.FN Sum_t*\\ \\fBsumopen\\fP(const\\ char*\\ \\fImethod\\fP)
Open a sum handle for \\fImethod\\fP\\&.
.FN int\\ \\fBsumclose\\fP(Sum_t*\\ \\fIsum\\fP)
Close a sum handle \\fIsum\\fP previously returned by \\fBsumopen\\fP\\&.
.SH SEE\\ ALSO
\\fBcksum\\fP(1)
.SH IMPLEMENTATION
.H0 version
sum (AT&T Research) 1999\\-12\\-11
.H0 author
Glenn Fowler <gsf@research\\&.att\\&.com>
.H0 copyright
Copyright (c) 1995\\-1999 AT&T Corp\\&.
.H0 license
http://www\\&.research\\&.att\\&.com/sw/license/ast\\-open\\&.html'

TEST 31 'off by one -- my epitaph'
	short_usage=$'[-n][n]#[s]:\n\npid ...'
	long_usage=$'[-n][n:number]#[s:name]:\n\npid ...'
	EXEC	kill "$short_usage" -0 123
		OUTPUT - $'return=n option=-n name=-0 arg=0 num=0\nargument=1 value="123"'
	EXEC	kill "$long_usage" -0 123
	EXEC	kill "$short_usage" -T 123
		OUTPUT - $'return=: option= name=-T num=0 str=[-n][n]#[s]:\n\npid ...\nargument=1 value="123"'
		ERROR - $'kill: -T: unknown option'
		EXIT 1
	EXEC	kill "$long_usage" -T 123
		OUTPUT - $'return=: option= name=-T num=0 str=[-n][n:number]#[s:name]:\n\npid ...\nargument=1 value="123"'

TEST 32 'miscellaneous'
	usage=$'[-][+NAME?wow - zowee][a|b:aORb?A or B.][y|z:yORz?Y or Z.]\n\n[ file ... ]'
	EXEC	wow "$usage" --nroff
		EXIT 2
		OUTPUT - $'return=? option=- name=--nroff num=0'
		ERROR - $'.\\" format with nroff|troff|groff -man
.TH WOW 1
.fp 5 CW
.nr mH 5
.de H0
.nr mH 0
.in 5n
\\fB\\\\$1\\fP
.in 7n
..
.de H1
.nr mH 1
.in 7n
\\fB\\\\$1\\fP
.in 9n
..
.de H2
.nr mH 2
.in 11n
\\fB\\\\$1\\fP
.in 13n
..
.de H3
.nr mH 3
.in 15n
\\fB\\\\$1\\fP
.in 17n
..
.de H4
.nr mH 4
.in 19n
\\fB\\\\$1\\fP
.in 21n
..
.de OP
.nr mH 0
.ie !\'\\\\$1\'-\' \\{
.ds mO \\\\fB\\\\-\\\\$1\\\\fP
.ds mS ,\\\\0
.\\}
.el \\{
.ds mO \\\\&
.ds mS \\\\&
.\\}
.ie \'\\\\$2\'-\' \\{
.if !\'\\\\$4\'-\' .as mO \\\\0\\\\fI\\\\$4\\\\fP
.\\}
.el \\{
.as mO \\\\*(mS\\\\fB\\\\-\\\\-\\\\$2\\\\fP
.if !\'\\\\$4\'-\' .as mO =\\\\fI\\\\$4\\\\fP
.\\}
.in 5n
\\\\*(mO
.in 9n
..
.de SP
.if \\\\n(mH==2 .in 9n
.if \\\\n(mH==3 .in 13n
.if \\\\n(mH==4 .in 17n
..
.de FN
.nr mH 0
.in 5n
\\\\$1 \\\\$2
.in 9n
..
.de DS
.in +3n
.ft 5
.nf
..
.de DE
.fi
.ft R
.in -3n
..
.SH NAME
wow \\- zowee
.SH SYNOPSIS
\\fBwow\\fP\\ [\\ \\fIoptions\\fP\\ ]\\ [\\ file\\ \\&.\\&.\\&.\\ ]
.SH OPTIONS
.OP a|b aORb flag -
A or B\\&.
.OP y|z yORz flag -
Y or Z\\&.
.SH IMPLEMENTATION
.SP'

TEST 33 'never thought of that'
	usage=$'n: path'
	EXEC	printf "$usage" +1
		EXIT 0
		OUTPUT - $'argument=1 value="+1"'
		ERROR -
	usage=$'[-][+NAME?printf][n:num?number]:[val]\n\npath'
	EXEC	printf "$usage" +1
	usage=$'[-][+NAME?printf][n:num?number]:[#]\n\npath'
	EXEC	printf "$usage" +1
	usage=$'n# path'
	EXEC	printf "$usage" +1
	usage=$'n#[val] path'
	EXEC	printf "$usage" +1
	usage=$'[-][+NAME?printf][n:num?number]#[#]\n\npath'
	EXEC	printf "$usage" +1
	usage=$'[-n][+NAME?printf][n:num?number]#[#]\n\npath'
	EXEC	printf "$usage" +1
		OUTPUT - $'return=n option=+n name=+1 arg=1 num=1'
	EXEC	printf "$usage" -1
		OUTPUT - $'return=n option=-n name=-1 arg=1 num=1'
	EXEC	printf "$usage" +1 -1
		OUTPUT - $'return=n option=+n name=+1 arg=1 num=1\nreturn=n option=-n name=-1 arg=1 num=1'
	usage=$'[-][x:xxx]#[yyy?could be:]{[+seeing?double]}'
	EXEC	kill "$usage" --man
		EXIT 2
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - $'SYNOPSIS
  kill [ options ]

OPTIONS
  -x, --xxx=yyy   could be:
                    seeing
                          double'
	usage=$'[-][+NAME?find][71:printf]:[format]{[+foo? ][+a?alert]}'
	EXEC find "$usage" --man
		ERROR - $'NAME
  find

SYNOPSIS
  find [ options ]

OPTIONS
  --printf=format
                    foo
                    a     alert'
	usage=$'[-][+NAME?find][71:printf]:[format]{[+foo?][+a?alert]}'
	EXEC find "$usage" --man
	usage=$'[-][+NAME?find][71:printf]:[format]{[+foo?][+a?alert]}'
	EXEC find "$usage" --man

TEST 35 'alternate version ids'
	EXEC id $'[-?\n@(#)id (spamco) 2000-12-01\n]' --?-version
		OUTPUT - $'return=? option=-? name=--?-version num=0'
		ERROR - $'  version         id (spamco) 2000-12-01'
		EXIT 2
	EXEC id $'[-?\n@(#)  \t  id (spamco) 2000-12-01\n]' --?-version
	EXEC id $'[-?\n$Id: id (spamco) 2000-12-01 $\n]' --?-version
		ERROR - $'  version         id (spamco) 2000-12-01'
	EXEC id $'[-?\n@(#)$Id: id (spamco) 2000-12-01 $\n]' --?-version
	EXEC id $'[-?\n$Id: @(#)id (spamco) 2000-12-01 $\n]' --?-version

TEST 36 'enumerated option argument values'
	usage=$'[-?\naha\n][Q:quote?Quote names according to \astyle\a:]:[style:=question]{\n[c:C?C style.][A:always?Always shell style.][101:shell?Shell quote if necessary.][q:question?Replace unknown chars with ?.]\n}'
	EXEC ls "$usage" --man
		EXIT 2
		OUTPUT - 'return=? option=- name=--man num=0'
		ERROR - 'SYNOPSIS
  ls [ options ]

OPTIONS
  -Q, --quote=style
                  Quote names according to style:
                    c|C   C style.
                    A|always
                          Always shell style.
                    shell Shell quote if necessary.
                    question
                          Replace unknown chars with ?.
                  The default value is question.

IMPLEMENTATION
  version         aha'
	EXEC ls "$usage" --quote
		EXIT 1
		OUTPUT - 'return=: option=-Q name=--quote num=0'
		ERROR - 'ls: --quote: style value expected'
	EXEC ls "$usage" --quote=alx
		ERROR - 'ls: --quote: alx: unknown option argument value'
	EXEC ls "$usage" --quote=c
		EXIT 0
		OUTPUT - 'return=Q option=-Q name=--quote arg=c num=99'
		ERROR -
	EXEC ls "$usage" --quote=C
		OUTPUT - 'return=Q option=-Q name=--quote arg=C num=99'
	EXEC ls "$usage" -Q shell
		OUTPUT - 'return=Q option=-Q name=-Q arg=shell num=-101'
	EXEC ls "$usage" -Qs
		OUTPUT - 'return=Q option=-Q name=-Q arg=s num=-101'

TEST 37 'stealth bugs'
	usage=$'[-?\naha\n][-catalog?SpamCo][h:html?Read html from \afile\a.]:[file[??name=value;...]]]'
	EXEC m2h "$usage" -h
		EXIT 1
		OUTPUT - 'return=: option=-h name=-h num=0'
		ERROR - 'm2h: -h: file[?name=value;...] argument expected'
	EXEC m2h "$usage" --man
		EXIT 2
		OUTPUT - 'return=? option=- name=--man num=0'
		ERROR - 'SYNOPSIS
  m2h [ options ]

OPTIONS
  -h, --html=file[?name=value;...]
                  Read html from file.

IMPLEMENTATION
  version         aha
  catalog         SpamCo'
	EXEC m2h "$usage" --keys
		OUTPUT - 'return=? option=- name=--keys num=0'
		ERROR - '"catalog"
"html"
"Read html from \afile\a."
"file[?name=value;...]"'
	EXEC ls $'[-][w:width]#[screen-width]' -wx
		OUTPUT - $'return=: option=-w name=-w num=0'
		ERROR - $'ls: -w: numeric screen-width argument expected'
		EXIT 1

TEST 38 'ancient compatibility for modern implementations -- ok, I still use vi'
	usage=$'[-1o][a:all][f:full][l:long][u:user]:[uid]\n\n[ pid ... ]\n\n'
	EXEC ps "$usage" --man
		EXIT 2
		OUTPUT - 'return=? option=- name=--man num=0'
		ERROR - 'SYNOPSIS
  ps [ options ] [ pid ... ]

OPTIONS
  -a, --all
  -f, --full
  -l, --long
  -u, --user=uid'
	EXEC ps "$usage" a 123
		EXIT 0
		OUTPUT - 'return=a option=-a name=-a arg=(null) num=1
argument=1 value="123"'
		ERROR -
	EXEC ps "$usage" -a 123
	EXEC ps "$usage" al 123
		OUTPUT - 'return=a option=-a name=-a arg=(null) num=1
return=l option=-l name=-l arg=(null) num=1
argument=1 value="123"'
	EXEC ps "$usage" -al 123
	EXEC ps "$usage" a l 123
	EXEC ps "$usage" a -l 123
	EXEC ps "$usage" u bozo l 123
		OUTPUT - 'return=u option=-u name=-u arg=bozo num=1
return=l option=-l name=-l arg=(null) num=1
argument=1 value="123"'
	EXEC ps "$usage" -u bozo -l 123
	EXEC ps "$usage" a u bozo l 123
		OUTPUT - 'return=a option=-a name=-a arg=(null) num=1
return=u option=-u name=-u arg=bozo num=1
return=l option=-l name=-l arg=(null) num=1
argument=1 value="123"'

TEST 39 'local suboptions'
	usage=$'[-?\naha\n][+NAME?search][x:method?The algorithm:]:[method:=linear]{[linear?Fast.][quadratic?Slower. Sub-options:]{[-][e:edges?Limit edges.]#[n:=50]}[np?Slow.][heuristic?Almost works. Sub-options:]{[-][n:nodes?Limit nodes.]#?[n:=100][f:foo?Bar.]:[huh][d:dump?Dump tables.][l:label?Set label.]:[string]}}[D:debug?Debug level.]#[level][+SEE ALSO?\afoo\a(1)]'
	EXEC search "$usage" --man
		EXIT 2
		OUTPUT - 'return=? option=- name=--man num=0'
		ERROR - 'NAME
  search

SYNOPSIS
  search [ options ]

OPTIONS
  -x, --method=method
                  The algorithm:
                    linear
                          Fast.
                    quadratic
                          Slower. Sub-options:
                            edges=n
                                  Limit edges. The default value is 50.
                    np    Slow.
                    heuristic
                          Almost works. Sub-options:
                            nodes[=n]
                                  Limit nodes. The option value may be omitted.
                                  The default value is 100.
                            foo=huh
                                  Bar.
                            dump  Dump tables.
                            label=string
                                  Set label.
                  The default value is linear.
  -D, --debug=level
                  Debug level.

SEE ALSO
  foo(1)

IMPLEMENTATION
  version         aha'

TEST 40 'examples of example examples'
	usage=$'[-?\naha\n][-catalog?SpamCo][+NAME?eg - test example examples][Q:quote?Quote names according to \astyle\a:]:[style:=question][+EXAMPLES]{[+\none\ntwo][+\n\vthree\nfour][+\n\afive\nsix]}[+SEE ALSO?\begman\b(1)]'
	EXEC eg "$usage" --man
		EXIT 2
		OUTPUT - 'return=? option=- name=--man num=0'
		ERROR - $'NAME
  eg - test example examples

SYNOPSIS
  eg [ options ]

OPTIONS
  -Q, --quote=style
                  Quote names according to style: The default value is
                  question.

EXAMPLES
    one
    two

    three
    four

    five
    six

SEE ALSO
  egman(1)

IMPLEMENTATION
  version         aha
  catalog         SpamCo'
	EXEC eg "$usage" --nroff
		OUTPUT - 'return=? option=- name=--nroff num=0'
		ERROR - $'.\\" format with nroff|troff|groff -man
.TH EG 1
.fp 5 CW
.nr mH 5
.de H0
.nr mH 0
.in 5n
\\fB\\\\$1\\fP
.in 7n
..
.de H1
.nr mH 1
.in 7n
\\fB\\\\$1\\fP
.in 9n
..
.de H2
.nr mH 2
.in 11n
\\fB\\\\$1\\fP
.in 13n
..
.de H3
.nr mH 3
.in 15n
\\fB\\\\$1\\fP
.in 17n
..
.de H4
.nr mH 4
.in 19n
\\fB\\\\$1\\fP
.in 21n
..
.de OP
.nr mH 0
.ie !\'\\\\$1\'-\' \\{
.ds mO \\\\fB\\\\-\\\\$1\\\\fP
.ds mS ,\\\\0
.\\}
.el \\{
.ds mO \\\\&
.ds mS \\\\&
.\\}
.ie \'\\\\$2\'-\' \\{
.if !\'\\\\$4\'-\' .as mO \\\\0\\\\fI\\\\$4\\\\fP
.\\}
.el \\{
.as mO \\\\*(mS\\\\fB\\\\-\\\\-\\\\$2\\\\fP
.if !\'\\\\$4\'-\' .as mO =\\\\fI\\\\$4\\\\fP
.\\}
.in 5n
\\\\*(mO
.in 9n
..
.de SP
.if \\\\n(mH==2 .in 9n
.if \\\\n(mH==3 .in 13n
.if \\\\n(mH==4 .in 17n
..
.de FN
.nr mH 0
.in 5n
\\\\$1 \\\\$2
.in 9n
..
.de DS
.in +3n
.ft 5
.nf
..
.de DE
.fi
.ft R
.in -3n
..
.SH NAME
eg \\- test example examples
.SH SYNOPSIS
\\fBeg\\fP\\ [\\ \\fIoptions\\fP\\ ]
.SH OPTIONS
.OP Q quote string style question
Quote names according to \\fIstyle\\fP:
The default value is \\fBquestion\\fP\\&.
.SH EXAMPLES
.DS
\\f5one
two\\fP
.DE
.DS
three
four
.DE
.DS
\\fIfive
six\\fP
.DE
.SH SEE\\ ALSO
\\fBegman\\fP(1)
.SH IMPLEMENTATION
.H0 version
aha
.H0 catalog
SpamCo'

	EXEC eg "$usage" --html
		OUTPUT - 'return=? option=- name=--html num=0'
		ERROR - $'<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<HEAD>
<META name="generator" content="optget (AT&T Research) 2011-11-11">
<TITLE>eg man document</TITLE>
<STYLE type="text/css">
div.SH { padding-left:2em; text-indent:0em; }
div.SY { padding-left:4em; text-indent:-2em; }
dt { float:left; clear:both; }
dd { margin-left:3em; }
</STYLE>
</HEAD>
<BODY bgcolor=white>
<H4><TABLE width=100%><TR><TH align=left>EG&nbsp;(&nbsp;1&nbsp;)&nbsp;<TH align=center><A href="." title="Index">USER COMMANDS</A><TH align=right>EG&nbsp;(&nbsp;1&nbsp;)</TR></TABLE></H4>
<HR>
<H4><A name="NAME">NAME</A></H4>
<DIV class=SH>
<!--MAN-INDEX-->eg - test example examples
<P>
</DIV>
<H4><A name="SYNOPSIS">SYNOPSIS</A></H4>
<DIV class=SY>
<B>eg</B> &#0091; <I>options</I> &#0093;
<P>
</DIV>
<H4><A name="OPTIONS">OPTIONS</A></H4>
<DIV class=SH>
<DL>
<DT>-<B>Q</B>, --<B>quote</B>=<I>style</I><DD><BR>Quote names according to <I>style
</I>: The default value is <B>question</B>.
</DL>
</DIV>
<H4><A name="EXAMPLES">EXAMPLES</A></H4>
<DIV class=SH>
<P>
<PRE>
<TT>one
two</TT>
</PRE>
<PRE>
three
four
</PRE>
<PRE>
<I>five
six</I>
</PRE>
</DIV>
<H4><A name="SEE ALSO">SEE ALSO</A></H4>
<DIV class=SH>
<NOBR><A href="../man1/egman.html"><B>egman</B></A>(1)</NOBR>
</DIV>
<H4><A name="IMPLEMENTATION">IMPLEMENTATION</A></H4>
<DIV class=SH>
<DL>
<DT><A name="version"><B>version</B></A><DD><BR>aha
<DT><A name="catalog"><B>catalog</B></A><DD><BR>SpamCo
</DL>
</DIV>
</BODY>
</HTML>'
	usage=$'[-?\naha\n][-catalog?SpamCo][+NAME?eg - test example examples][Q:quote?Quote names according to \astyle\a:]:[style:=question][+EXAMPLES]{[+dss -x bgp \'(type==\"A\")??{write table > a}::{write cisco > b}\' mrt.dat?Write the announce records from \bmrt.dat\b to the file \ba\b in the \btable\b format and all other records to the file \bb\b in the \bcisco\b format.]}[+SEE ALSO?\begman\b(1), \bwalrus\b(8)]'
	EXEC eg "$usage" --html
		EXIT 2
		OUTPUT - 'return=? option=- name=--html num=0'
		ERROR - $'<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<HEAD>
<META name="generator" content="optget (AT&T Research) 2011-11-11">
<TITLE>eg man document</TITLE>
<STYLE type="text/css">
div.SH { padding-left:2em; text-indent:0em; }
div.SY { padding-left:4em; text-indent:-2em; }
dt { float:left; clear:both; }
dd { margin-left:3em; }
</STYLE>
</HEAD>
<BODY bgcolor=white>
<H4><TABLE width=100%><TR><TH align=left>EG&nbsp;(&nbsp;1&nbsp;)&nbsp;<TH align=center><A href="." title="Index">USER COMMANDS</A><TH align=right>EG&nbsp;(&nbsp;1&nbsp;)</TR></TABLE></H4>
<HR>
<H4><A name="NAME">NAME</A></H4>
<DIV class=SH>
<!--MAN-INDEX-->eg - test example examples
<P>
</DIV>
<H4><A name="SYNOPSIS">SYNOPSIS</A></H4>
<DIV class=SY>
<B>eg</B> &#0091; <I>options</I> &#0093;
<P>
</DIV>
<H4><A name="OPTIONS">OPTIONS</A></H4>
<DIV class=SH>
<DL>
<DT>-<B>Q</B>, --<B>quote</B>=<I>style</I><DD><BR>Quote names according to <I>style
</I>: The default value is <B>question</B>.
</DL>
</DIV>
<H4><A name="EXAMPLES">EXAMPLES</A></H4>
<DIV class=SH>
<DL>
<DT><A name="dss -x bgp \'(type==&quot;A&quot;)?{write table &gt; a}:{write cisco
&gt; b}\' mrt.dat"><B>dss -x bgp \'</B>(type==&quot;A&quot;)?{write table &gt;
a}:{write cisco &gt; b}\' mrt.dat</A><DD><BR>Write the announce records from <B>mrt.dat
</B> to the file <B>a</B> in the <B>table</B> format and all other records to
the file <B>b</B> in the <B>cisco</B> format.
</DL>
</DIV>
<H4><A name="SEE ALSO">SEE ALSO</A></H4>
<DIV class=SH>
<NOBR><A href="../man1/egman.html"><B>egman</B></A>(1),</NOBR>
<NOBR><A href="../man8/walrus.html"><B>walrus</B></A>(8)</NOBR>
</DIV>
<H4><A name="IMPLEMENTATION">IMPLEMENTATION</A></H4>
<DIV class=SH>
<DL>
<DT><A name="version"><B>version</B></A><DD><BR>aha
<DT><A name="catalog"><B>catalog</B></A><DD><BR>SpamCo
</DL>
</DIV>
</BODY>
</HTML>'

TEST 41 'cache exercises'
	usage=$'[-1c][a:aaa?AAA][b:bbb?BBB]:[bv][c:ccc?CCC]:?[cv]'
	EXEC typeset "$usage" -a -b1
		OUTPUT - $'[3] return=a option=-a name=-a arg=(null) num=1
[3] return=b option=-b name=-b arg=1 num=1
[2] return=a option=-a name=-a arg=(null) num=1
[2] return=b option=-b name=-b arg=1 num=1
[1] return=a option=-a name=-a arg=(null) num=1
[1] return=b option=-b name=-b arg=1 num=1'
	EXEC typeset "$usage" -c2 -a
		OUTPUT - $'[3] return=c option=-c name=-c arg=2 num=1
[3] return=a option=-a name=-a arg=(null) num=1
[2] return=c option=-c name=-c arg=2 num=1
[2] return=a option=-a name=-a arg=(null) num=1
[1] return=c option=-c name=-c arg=2 num=1
[1] return=a option=-a name=-a arg=(null) num=1'
	EXEC typeset "$usage" -c -a
		OUTPUT - $'[3] return=c option=-c name=-c arg=(null) num=0
[3] return=a option=-a name=-a arg=(null) num=1
[2] return=c option=-c name=-c arg=(null) num=0
[2] return=a option=-a name=-a arg=(null) num=1
[1] return=c option=-c name=-c arg=(null) num=0
[1] return=a option=-a name=-a arg=(null) num=1'
	EXEC typeset "$usage" -a -b1 -c -c2
		OUTPUT - $'[3] return=a option=-a name=-a arg=(null) num=1
[3] return=b option=-b name=-b arg=1 num=1
[3] return=c option=-c name=-c arg=(null) num=0
[3] return=c option=-c name=-c arg=2 num=1
[2] return=a option=-a name=-a arg=(null) num=1
[2] return=b option=-b name=-b arg=1 num=1
[2] return=c option=-c name=-c arg=(null) num=0
[2] return=c option=-c name=-c arg=2 num=1
[1] return=a option=-a name=-a arg=(null) num=1
[1] return=b option=-b name=-b arg=1 num=1
[1] return=c option=-c name=-c arg=(null) num=0
[1] return=c option=-c name=-c arg=2 num=1'
	usage=$'[-1c]a:[command][n:number]#?[number][s:string]:?[string]'
	EXEC typeset "$usage" -n
		OUTPUT - $'[3] return=n option=-n name=-n arg=(null) num=1
[2] return=n option=-n name=-n arg=(null) num=1
[1] return=n option=-n name=-n arg=(null) num=1'
	EXEC typeset "$usage" -n12
		OUTPUT - $'[3] return=n option=-n name=-n arg=12 num=12
[2] return=n option=-n name=-n arg=12 num=12
[1] return=n option=-n name=-n arg=12 num=12'
	EXEC typeset "$usage" -s
		OUTPUT - $'[3] return=s option=-s name=-s arg=(null) num=1
[2] return=s option=-s name=-s arg=(null) num=1
[1] return=s option=-s name=-s arg=(null) num=1'
	EXEC typeset "$usage" -s12
		OUTPUT - $'[3] return=s option=-s name=-s arg=12 num=1
[2] return=s option=-s name=-s arg=12 num=1
[1] return=s option=-s name=-s arg=12 num=1'
	EXEC typeset "$usage" OPT --foo
		OUTPUT - $'[3] argument=1 value="OPT"
[3] argument=2 value="--foo"
[2] argument=1 value="OPT"
[2] argument=2 value="--foo"
[1] argument=1 value="OPT"
[1] argument=2 value="--foo"'
	EXEC typeset "$usage" -a locate OPT --foo
		OUTPUT - $'[3] return=a option=-a name=-a arg=locate num=1
[3] argument=1 value="OPT"
[3] argument=2 value="--foo"
[2] return=a option=-a name=-a arg=locate num=1
[2] argument=1 value="OPT"
[2] argument=2 value="--foo"
[1] return=a option=-a name=-a arg=locate num=1
[1] argument=1 value="OPT"
[1] argument=2 value="--foo"'
	usage=$'[-1c][L]#?[n?Left justify.][Z]#?[n?Zero fill.]'
	EXEC typeset "$usage" -Z -L -L -Z -L foo=bar
		OUTPUT - '[3] return=Z option=-Z name=-Z arg=(null) num=0
[3] return=L option=-L name=-L arg=(null) num=0
[3] return=L option=-L name=-L arg=(null) num=0
[3] return=Z option=-Z name=-Z arg=(null) num=0
[3] return=L option=-L name=-L arg=(null) num=0
[3] argument=1 value="foo=bar"
[2] return=Z option=-Z name=-Z arg=(null) num=0
[2] return=L option=-L name=-L arg=(null) num=0
[2] return=L option=-L name=-L arg=(null) num=0
[2] return=Z option=-Z name=-Z arg=(null) num=0
[2] return=L option=-L name=-L arg=(null) num=0
[2] argument=1 value="foo=bar"
[1] return=Z option=-Z name=-Z arg=(null) num=0
[1] return=L option=-L name=-L arg=(null) num=0
[1] return=L option=-L name=-L arg=(null) num=0
[1] return=Z option=-Z name=-Z arg=(null) num=0
[1] return=L option=-L name=-L arg=(null) num=0
[1] argument=1 value="foo=bar"'

TEST 42 'optional long names'
	usage=$'[-][a:aaa][b:bbb?BBB][c?CCC][d:?DDD][e\f:n:eee\f][f\f:y:fff\f?FFF][g:ggg?GGG]'
	EXEC tst "$usage" -a --aaa -b --bbb -c -d -e --eee -f --fff -g -ggg
		OUTPUT - $'return=a option=-a name=-a arg=(null) num=1
return=a option=-a name=--aaa arg=(null) num=1
return=b option=-b name=-b arg=(null) num=1
return=b option=-b name=--bbb arg=(null) num=1
return=c option=-c name=-c arg=(null) num=1
return=d option=-d name=-d arg=(null) num=1
return=e option=-e name=-e arg=(null) num=1
return=e option=-e name=--eee arg=(null) num=1
return=f option=-f name=-f arg=(null) num=1
return=f option=-f name=--fff arg=(null) num=1
return=g option=-g name=-g arg=(null) num=1
return=g option=-g name=-g arg=(null) num=1
return=g option=-g name=-g arg=(null) num=1
return=g option=-g name=-g arg=(null) num=1'
	EXEC tst "$usage" --man
		EXIT 2
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - $'SYNOPSIS
  tst [ options ]

OPTIONS
  -a, --aaa
  -b, --bbb       BBB
  -c              CCC
  -d              DDD
  -e
  -f, --fff       FFF
  -g, --ggg       GGG'
	usage=$'[-][a:aaa]:[av][b:bbb?BBB]:[bv][c?CCC]:[cv][d:?DDD]:[dv][e\f:n:eee\f]:[ev][f\f:y:fff\f?FFF]:[fv][g:ggg?GGG]:[gv]'
	EXEC tst "$usage" -a AV --aaa=LAV -bBV --bbb=LBV -c CV -dDV -e EV --eee LEV -f FV --fff=LFV -gGV -ggg LGV
		EXIT 0
		OUTPUT - $'return=a option=-a name=-a arg=AV num=1
return=a option=-a name=--aaa arg=LAV num=1
return=b option=-b name=-b arg=BV num=1
return=b option=-b name=--bbb arg=LBV num=1
return=c option=-c name=-c arg=CV num=1
return=d option=-d name=-d arg=DV num=1
return=e option=-e name=-e arg=EV num=1
return=e option=-e name=--eee arg=LEV num=1
return=f option=-f name=-f arg=FV num=1
return=f option=-f name=--fff arg=LFV num=1
return=g option=-g name=-g arg=GV num=1
return=g option=-g name=-g arg=gg num=1
argument=1 value="LGV"'
		ERROR -
	EXEC tst "$usage" --man
		EXIT 2
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - $'SYNOPSIS
  tst [ options ]

OPTIONS
  -a, --aaa=av
  -b, --bbb=bv    BBB
  -c cv           CCC
  -d dv           DDD
  -e ev
  -f, --fff=fv    FFF
  -g, --ggg=gv    GGG'

TEST 43 'trailing wild card'
	usage=$'[-][z:zzz]:[style][A:a*][B:b*][C:c*]'
	EXEC wild "$usage" -A -B -C --a --b --c --axx --bxx --cxx
		OUTPUT - $'return=A option=-A name=-A arg=(null) num=1
return=B option=-B name=-B arg=(null) num=1
return=C option=-C name=-C arg=(null) num=1
return=A option=-A name=--a arg=(null) num=1
return=B option=-B name=--b arg=(null) num=1
return=C option=-C name=--c arg=(null) num=1
return=A option=-A name=--axx arg=(null) num=1
return=B option=-B name=--bxx arg=(null) num=1
return=C option=-C name=--cxx arg=(null) num=1'

	(
		export LC_ALL=en_US
		EXEC wild "$usage" -A -B -C --a --b --c --axx --bxx --cxx
	)

	usage=$'[-][z:zzz]:[style]{[A:a*][B:b*][C:c*]}'
	EXEC wild "$usage" -z A -z B -z C -z a -z b -z c -z axx -z bxx -z cxx
		OUTPUT - $'return=z option=-z name=-z arg=A num=65
return=z option=-z name=-z arg=B num=66
return=z option=-z name=-z arg=C num=67
return=z option=-z name=-z arg=a num=65
return=z option=-z name=-z arg=b num=66
return=z option=-z name=-z arg=c num=67
return=z option=-z name=-z arg=axx num=65
return=z option=-z name=-z arg=bxx num=66
return=z option=-z name=-z arg=cxx num=67'
	usage=$'[-][z:zzz]:[style]{[A:a*?aaa][B:b*?bbb][C:c*?ccc]}'
	EXEC wild "$usage" -z A -z B -z C -z a -z b -z c -z axx -z bxx -z cxx
	EXEC wild "$usage" -z
		EXIT 1
		OUTPUT - $'return=: option=-z name=-z num=0'
		ERROR - $'wild: -z: style argument expected'
	EXEC wild "$usage" --zzz
		OUTPUT - $'return=: option=-z name=--zzz num=0'
		ERROR - $'wild: --zzz: style value expected'
	usage=$'[-][z:zzz]:[style]{[A:a\aget\a?aaa][B:b\aoutta\a?bbb][C:c\atown\a?ccc]}'
	EXEC wild "$usage" -z A -z B -z C -z a -z b -z c -z axx -z bxx -z cxx
		EXIT 0
		OUTPUT - $'return=z option=-z name=-z arg=A num=65
return=z option=-z name=-z arg=B num=66
return=z option=-z name=-z arg=C num=67
return=z option=-z name=-z arg=a num=65
return=z option=-z name=-z arg=b num=66
return=z option=-z name=-z arg=c num=67
return=z option=-z name=-z arg=axx num=65
return=z option=-z name=-z arg=bxx num=66
return=z option=-z name=-z arg=cxx num=67'
		ERROR -
	EXEC wild "$usage" -z
		EXIT 1
		OUTPUT - $'return=: option=-z name=-z num=0'
		ERROR - $'wild: -z: style argument expected'
	EXEC wild "$usage" --zzz
		OUTPUT - $'return=: option=-z name=--zzz num=0'
		ERROR - $'wild: --zzz: style value expected'
	EXEC wild "$usage" --man
		EXIT 2
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - $'SYNOPSIS
  wild [ options ]

OPTIONS
  -z, --zzz=style
                    A|aget
                          aaa
                    B|boutta
                          bbb
                    C|ctown
                          ccc'

TEST 44 'getopt_long() compatibility'
	usage=$'[-1p1]\n[h:help]\n[V:version]\n[v:verbose]\n[X]\n[259:save-temps]\n[s:std]:[string]\n[d:debug]\n[262:static]\n[263:dynamic]\n[264:free]\n[265:fixed]\n[266:column]:[string]\n[267:MT]:[string]\n[268:MF]:[string]\n[269:fmain]\n[270:fno-main]\n[W:Wall]\n[272:Wobsolete]\n[273:Wno-obsolete]\n[274:Warchaic]\n[275:Wno-archaic]\n[276:Wcolumn-overflow]\n[277:Wno-column-overflow]\n[278:Wconstant]\n[279:Wno-constant]\n[280:Wparentheses]\n[281:Wno-parentheses]\n[282:Wimplicit-terminator]\n[283:Wno-implicit-terminator]\n[284:Wstrict-typing]\n[285:Wno-strict-typing]\n[?]\n[E]\n[P]\n[C]\n[S]\n[c]\n[m]\n[g]\n[o]:[]\n[I]:[]\n'
	EXEC cobcc "$usage" -static -I foo -Ibar -debug -C -Wparen tst.cob
		OUTPUT - $'return=-262 option=-262 name=-static arg=(null) num=1
return=I option=-262 name=-I arg=foo num=1
return=I option=-262 name=-I arg=bar num=1
return=d option=-d name=-debug arg=(null) num=1
return=C option=-d name=-C arg=(null) num=1
return=-280 option=-280 name=-Wparentheses arg=(null) num=1
argument=1 value="tst.cob"'

TEST 45 'n=v vs. n:=v'
	usage=$'[-][a:aaa?AAA]:[vvv]'
	EXEC pax "$usage" -a 1 --a=xx --a:=yy
		OUTPUT - $'return=a option=-a name=-a arg=1 num=1
return=a option=-a name=--aaa arg=xx num=1
return=a option=-a name=--aaa arg:=yy num=1'
	EXEC pax "$usage" -a 1 --aaa=xx --aaa:=yy

TEST 46 'html escapism'
	usage=$'[-][+NAME?codex - encode/decode filter][+?Methods:]{[+and?things]{[+of?this <= 64.]:[nature][+govern?ator]}}\n\n[ [ <,> ] method [ <,>,| method ... ] ]\n\n[+SEE ALSO?\bcodex\b(3), \bvcodex\b(3)]'
	EXEC codex "$usage" --html
		EXIT 2
		OUTPUT - $'return=? option=- name=--html num=0'
		ERROR - $'<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<HEAD>
<META name="generator" content="optget (AT&T Research) 2011-11-11">
<TITLE>codex man document</TITLE>
<STYLE type="text/css">
div.SH { padding-left:2em; text-indent:0em; }
div.SY { padding-left:4em; text-indent:-2em; }
dt { float:left; clear:both; }
dd { margin-left:3em; }
</STYLE>
</HEAD>
<BODY bgcolor=white>
<H4><TABLE width=100%><TR><TH align=left>CODEX&nbsp;(&nbsp;1&nbsp;)&nbsp;<TH align=center><A href="." title="Index">USER COMMANDS</A><TH align=right>CODEX&nbsp;(&nbsp;1&nbsp;)</TR></TABLE></H4>
<HR>
<H4><A name="NAME">NAME</A></H4>
<DIV class=SH>
<!--MAN-INDEX-->codex - encode/decode filter
<P>
</DIV>
<H4><A name="SYNOPSIS">SYNOPSIS</A></H4>
<DIV class=SY>
<B>codex</B> &#0091; <I>options</I> &#0093; &#0091; &#0091; &lt;,&gt; &#0093; method &#0091; &lt;,&gt;,| method ...
&#0093; &#0093;
<P>
Methods:
<DL>
<DT><A name="and"><B>and</B></A><DD>things
<DL>
<DT><A name="of=nature"><B>of=<I>nature</I></B></A><DD><BR>this &lt;= 64.
<DT><A name="govern"><B>govern</B></A><DD><BR>ator
</DL>
</DL>
</DIV>
<H4><A name="SEE ALSO">SEE ALSO</A></H4>
<DIV class=SH>
<NOBR><A href="../man3/codex.html"><B>codex</B></A>(3),</NOBR>
<NOBR><A href="../man3/vcodex.html"><B>vcodex</B></A>(3)</NOBR>
</DIV>
</BODY>
</HTML>'

TEST 47 'omitted optional arg default'
	usage1=$'[-][101:clobber?Clobber pattern.]:?[pattern:=*.exe:!*][102:select?Select pattern.]:[pattern=*.[ch]]]'
	usage2=$'[-][101:clobber?Clobber pattern.]:?[pattern:!*:=*.exe][102:select?Select pattern.]:[pattern=*.[ch]]]'
	EXEC make "$usage1" --clobber foo
		OUTPUT - $'return=-101 option=-101 name=--clobber arg=* num=1\nargument=1 value="foo"'
	EXEC make "$usage2" --clobber foo
	EXEC make "$usage1" --clobber=1 bar
		OUTPUT - $'return=-101 option=-101 name=--clobber arg=* num=1\nargument=1 value="bar"'
	EXEC make "$usage2" --clobber=1 bar
	EXEC make "$usage1" --noclobber foo
		OUTPUT - $'return=-101 option=-101 name=--clobber arg=(null) num=0\nargument=1 value="foo"'
	EXEC make "$usage2" --noclobber foo
	EXEC make "$usage1" --clobber=0 bar
		OUTPUT - $'return=-101 option=-101 name=--clobber arg=(null) num=0\nargument=1 value="bar"'
	EXEC make "$usage2" --clobber=0 bar
	EXEC make "$usage1" --clobber=foo bar
		OUTPUT - $'return=-101 option=-101 name=--clobber arg=foo num=1\nargument=1 value="bar"'
	EXEC make "$usage2" --clobber=foo bar
	EXEC make "$usage1" --select foo bar
		OUTPUT - $'return=-102 option=-102 name=--select arg=foo num=1\nargument=1 value="bar"'
	EXEC make "$usage2" --select foo bar
	EXEC make "$usage1" --select=foo bar
	EXEC make "$usage2" --select=foo bar
	EXEC make "$usage1" --?clobber
		OUTPUT - $'return=? option=-? name=--?clobber num=0'
		ERROR - $'Usage: make [ options ]
OPTIONS
  --clobber[=pattern]
                  Clobber pattern. If the option value is omitted then * is
                  assumed. The default value is *.exe.'
		  EXIT 2
	EXEC make "$usage2" --?clobber
	EXEC make "$usage1" --?select
		OUTPUT - $'return=? option=-? name=--?select num=0'
		ERROR - $'Usage: make [ options ]
OPTIONS
  --select=pattern=*.[ch]
                  Select pattern.'
	EXEC make "$usage2" --?select
	EXEC make "$usage1" --man
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - $'SYNOPSIS
  make [ options ]

OPTIONS
  --clobber[=pattern]
                  Clobber pattern. If the option value is omitted then * is
                  assumed. The default value is *.exe.
  --select=pattern=*.[ch]
                  Select pattern.'
	EXEC make "$usage2" --man
	EXEC make "$usage1" --html
		OUTPUT - $'return=? option=- name=--html num=0'
		ERROR - $'<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<HEAD>
<META name="generator" content="optget (AT&T Research) 2011-11-11">
<TITLE>make man document</TITLE>
<STYLE type="text/css">
div.SH { padding-left:2em; text-indent:0em; }
div.SY { padding-left:4em; text-indent:-2em; }
dt { float:left; clear:both; }
dd { margin-left:3em; }
</STYLE>
</HEAD>
<BODY bgcolor=white>
<H4><TABLE width=100%><TR><TH align=left>MAKE&nbsp;(&nbsp;1&nbsp;)&nbsp;<TH align=center><A href="." title="Index">USER COMMANDS</A><TH align=right>MAKE&nbsp;(&nbsp;1&nbsp;)</TR></TABLE></H4>
<HR>
<H4><A name="SYNOPSIS">SYNOPSIS</A></H4>
<DIV class=SY>
<B>make</B> &#0091; <I>options</I> &#0093;
</DIV>
<P>
<H4><A name="OPTIONS">OPTIONS</A></H4>
<DIV class=SH>
<DL>
<DT>--<B>clobber</B>&#0091;=<I>pattern</I>&#0093;<DD><BR>Clobber pattern. If the option value
is omitted then <B>*</B> is assumed. The default value is <B>*.exe</B>.
<DT>--<B>select</B>=<I>pattern=*.&#0091;ch&#0093;</I><DD><BR>Select pattern.
</DL>
</DIV>
</BODY>
</HTML>'
	EXEC make "$usage2" --html

TEST 48 'ambiguous trio'
	usage=$'[-][x:aha?AHA.][y:aha1?AHA1][z:aha2?AHA2]'
	EXEC huh "$usage" --aha
		OUTPUT - $'return=x option=-x name=--aha arg=(null) num=1'
	EXEC huh "$usage" --noaha
		OUTPUT - $'return=x option=-x name=--aha arg=(null) num=0'
	EXEC huh "$usage" --aha1
		OUTPUT - $'return=y option=-y name=--aha1 arg=(null) num=1'
	EXEC huh "$usage" --noaha1
		OUTPUT - $'return=y option=-y name=--aha1 arg=(null) num=0'
	EXEC huh "$usage" --ah
		OUTPUT - $'return=: option=-x name=--ah num=0'
		ERROR - $'huh: --ah: ambiguous option'
		EXIT 1
	EXEC huh "$usage" --noah
		OUTPUT - $'return=: option=-x name=--noah num=0'
		ERROR - $'huh: --noah: ambiguous option'

TEST 49 'the long and short of it'
	usage=$'[-][a:archive-clean][b:archive-clobber]'
	EXEC make "$usage" --archive-clean --acle --aclo --ar-clobber --arcle --arclo
		OUTPUT - $'return=a option=-a name=--archive-clean arg=(null) num=1
return=a option=-a name=--archive-clean arg=(null) num=1
return=b option=-b name=--archive-clobber arg=(null) num=1
return=b option=-b name=--archive-clobber arg=(null) num=1
return=a option=-a name=--archive-clean arg=(null) num=1
return=b option=-b name=--archive-clobber arg=(null) num=1'
	EXEC make "$usage" --archive_clean --acle --aclo --ar_clobber --arcle --arclo
	EXEC make "$usage" --ac --ac --a-c --ar-c --arc --arcl
		OUTPUT - $'return=: option=-a name=--ac num=0
return=: option=-a name=--ac num=0
return=: option=-a name=--a-c num=0
return=: option=-a name=--ar-c num=0
return=: option=-a name=--arc num=0
return=: option=-a name=--arcl num=0'
		ERROR - $'make: --ac: ambiguous option
make: --ac: ambiguous option
make: --a-c: ambiguous option
make: --ar-c: ambiguous option
make: --arc: ambiguous option
make: --arcl: ambiguous option'
		EXIT 1
	EXEC make "$usage" --ac --ac --a_c --ar_c --arc --arcl
		OUTPUT - $'return=: option=-a name=--ac num=0
return=: option=-a name=--ac num=0
return=: option=-a name=--a_c num=0
return=: option=-a name=--ar_c num=0
return=: option=-a name=--arc num=0
return=: option=-a name=--arcl num=0'
		ERROR - $'make: --ac: ambiguous option
make: --ac: ambiguous option
make: --a_c: ambiguous option
make: --ar_c: ambiguous option
make: --arc: ambiguous option
make: --arcl: ambiguous option'
	usage=$'[-][a:archive-clean]'
	EXEC make "$usage" --archive-clean --ac --a-c --ar-c --arc --ar-clean --arclean
		OUTPUT - $'return=a option=-a name=--archive-clean arg=(null) num=1
return=a option=-a name=--archive-clean arg=(null) num=1
return=a option=-a name=--archive-clean arg=(null) num=1
return=a option=-a name=--archive-clean arg=(null) num=1
return=a option=-a name=--archive-clean arg=(null) num=1
return=a option=-a name=--archive-clean arg=(null) num=1
return=a option=-a name=--archive-clean arg=(null) num=1'
		ERROR -
		EXIT 0
	EXEC make "$usage" --archive_clean --ac --a_c --ar_c --arc --ar_clean --arclean
	EXEC make "$usage" --?archive-clean --?a-c --?ac --?ar-c --?arc --?ar-clean --?arclean
		OUTPUT - $'return=? option=-? name=--?archive-clean num=0'
		ERROR - $'Usage: make [ options ]
OPTIONS
  -a, --archive-clean'
		EXIT 2
	EXEC make "$usage" --?archive_clean --?a_c --?ac --?ar_c --?arc --?ar_clean --?arclean
		OUTPUT - $'return=? option=-? name=--?archive_clean num=0'
	EXEC make "$usage" --?a-c
		OUTPUT - $'return=? option=-? name=--?a-c num=0'
	EXEC make "$usage" --?a_c
		OUTPUT - $'return=? option=-? name=--?a_c num=0'
	EXEC make "$usage" --?ac
		OUTPUT - $'return=? option=-? name=--?ac num=0'
	EXEC make "$usage" --?ar-c
		OUTPUT - $'return=? option=-? name=--?ar-c num=0'
	EXEC make "$usage" --?ar_c
		OUTPUT - $'return=? option=-? name=--?ar_c num=0'
	EXEC make "$usage" --?arc
		OUTPUT - $'return=? option=-? name=--?arc num=0'
	EXEC make "$usage" --?ar-clean
		OUTPUT - $'return=? option=-? name=--?ar-clean num=0'
	EXEC make "$usage" --?ar_clean
		OUTPUT - $'return=? option=-? name=--?ar_clean num=0'
	EXEC make "$usage" --?arclean
		OUTPUT - $'return=? option=-? name=--?arclean num=0'
	usage=$'[-][a:ArchiveClean]'
	EXEC make "$usage" --?ArchiveClean --?AC --?Arc --?ArC --?ArClean --?ArcClean
		OUTPUT - $'return=? option=-? name=--?ArchiveClean num=0'
		ERROR - $'Usage: make [ options ]
OPTIONS
  -a, --ArchiveClean'
	EXEC make "$usage" --?AC
		OUTPUT - $'return=? option=-? name=--?AC num=0'
	EXEC make "$usage" --?Arc
		OUTPUT - $'return=? option=-? name=--?Arc num=0'
	EXEC make "$usage" --?ArC
		OUTPUT - $'return=? option=-? name=--?ArC num=0'
	EXEC make "$usage" --?ArClean
		OUTPUT - $'return=? option=-? name=--?ArClean num=0'
	EXEC make "$usage" --?ArcClean
		OUTPUT - $'return=? option=-? name=--?ArcClean num=0'

TEST 50 'the underscored long and short of it'
	usage=$'[-][a:archive_clean][b:archive_clobber]'
	EXEC make "$usage" --archive-clean --acle --aclo --ar-clobber --arcle --arclo
		OUTPUT - $'return=a option=-a name=--archive_clean arg=(null) num=1
return=a option=-a name=--archive_clean arg=(null) num=1
return=b option=-b name=--archive_clobber arg=(null) num=1
return=b option=-b name=--archive_clobber arg=(null) num=1
return=a option=-a name=--archive_clean arg=(null) num=1
return=b option=-b name=--archive_clobber arg=(null) num=1'
	EXEC make "$usage" --archive_clean --acle --aclo --ar_clobber --arcle --arclo
	EXEC make "$usage" --ac --ac --a-c --ar-c --arc --arcl
		OUTPUT - $'return=: option=-a name=--ac num=0
return=: option=-a name=--ac num=0
return=: option=-a name=--a-c num=0
return=: option=-a name=--ar-c num=0
return=: option=-a name=--arc num=0
return=: option=-a name=--arcl num=0'
		ERROR - $'make: --ac: ambiguous option
make: --ac: ambiguous option
make: --a-c: ambiguous option
make: --ar-c: ambiguous option
make: --arc: ambiguous option
make: --arcl: ambiguous option'
		EXIT 1
	EXEC make "$usage" --ac --ac --a_c --ar_c --arc --arcl
		OUTPUT - $'return=: option=-a name=--ac num=0
return=: option=-a name=--ac num=0
return=: option=-a name=--a_c num=0
return=: option=-a name=--ar_c num=0
return=: option=-a name=--arc num=0
return=: option=-a name=--arcl num=0'
		ERROR - $'make: --ac: ambiguous option
make: --ac: ambiguous option
make: --a_c: ambiguous option
make: --ar_c: ambiguous option
make: --arc: ambiguous option
make: --arcl: ambiguous option'
	usage=$'[-][a:archive_clean]'
	EXEC make "$usage" --archive-clean --ac --a-c --ar-c --arc --ar-clean --arclean
		OUTPUT - $'return=a option=-a name=--archive_clean arg=(null) num=1
return=a option=-a name=--archive_clean arg=(null) num=1
return=a option=-a name=--archive_clean arg=(null) num=1
return=a option=-a name=--archive_clean arg=(null) num=1
return=a option=-a name=--archive_clean arg=(null) num=1
return=a option=-a name=--archive_clean arg=(null) num=1
return=a option=-a name=--archive_clean arg=(null) num=1'
		ERROR -
		EXIT 0
	EXEC make "$usage" --archive_clean --ac --a_c --ar_c --arc --ar_clean --arclean
	EXEC make "$usage" --?archive-clean --?a-c --?ac --?ar-c --?arc --?ar-clean --?arclean
		OUTPUT - $'return=? option=-? name=--?archive-clean num=0'
		ERROR - $'Usage: make [ options ]
OPTIONS
  -a, --archive_clean'
		EXIT 2
	EXEC make "$usage" --?archive_clean --?a_c --?ac --?ar_c --?arc --?ar_clean --?arclean
		OUTPUT - $'return=? option=-? name=--?archive_clean num=0'
	EXEC make "$usage" --?a-c
		OUTPUT - $'return=? option=-? name=--?a-c num=0'
	EXEC make "$usage" --?a_c
		OUTPUT - $'return=? option=-? name=--?a_c num=0'
	EXEC make "$usage" --?ac
		OUTPUT - $'return=? option=-? name=--?ac num=0'
	EXEC make "$usage" --?ar-c
		OUTPUT - $'return=? option=-? name=--?ar-c num=0'
	EXEC make "$usage" --?ar_c
		OUTPUT - $'return=? option=-? name=--?ar_c num=0'
	EXEC make "$usage" --?arc
		OUTPUT - $'return=? option=-? name=--?arc num=0'
	EXEC make "$usage" --?ar-clean
		OUTPUT - $'return=? option=-? name=--?ar-clean num=0'
	EXEC make "$usage" --?ar_clean
		OUTPUT - $'return=? option=-? name=--?ar_clean num=0'
	EXEC make "$usage" --?arclean
		OUTPUT - $'return=? option=-? name=--?arclean num=0'

TEST 51 'the suboption long and short of it'
	usage=$'[-][z:time-style?Time style.]:[style]{[11:iso?Iso.][12:posix-iso?Posix iso.][13:full-iso?Full iso.][14:full-numeric?Full numeric.]}'
	EXEC make "$usage" --time-style=iso --ts=i --time-style=full-iso --ts=fi --ts=fn --time-style=full-numeric -z fnumeric -z fulnumeric -z fuln -z fulln --timestyle=fullnumeric
		OUTPUT - $'return=z option=-z name=--time-style arg=iso num=-11
return=z option=-z name=--time-style arg=i num=-11
return=z option=-z name=--time-style arg=full-iso num=-13
return=z option=-z name=--time-style arg=fi num=-13
return=z option=-z name=--time-style arg=fn num=-14
return=z option=-z name=--time-style arg=full-numeric num=-14
return=z option=-z name=-z arg=fnumeric num=-14
return=z option=-z name=-z arg=fulnumeric num=-14
return=z option=-z name=-z arg=fuln num=-14
return=z option=-z name=-z arg=fulln num=-14
return=z option=-z name=--time-style arg=fullnumeric num=-14'
	EXEC make "$usage" --time_style=iso --ts=i --time_style=full_iso --ts=fi --ts=fn --time_style=full_numeric -z fnumeric -z fulnumeric -z fuln -z fulln --timestyle=fullnumeric
		OUTPUT - $'return=z option=-z name=--time-style arg=iso num=-11
return=z option=-z name=--time-style arg=i num=-11
return=z option=-z name=--time-style arg=full_iso num=-13
return=z option=-z name=--time-style arg=fi num=-13
return=z option=-z name=--time-style arg=fn num=-14
return=z option=-z name=--time-style arg=full_numeric num=-14
return=z option=-z name=-z arg=fnumeric num=-14
return=z option=-z name=-z arg=fulnumeric num=-14
return=z option=-z name=-z arg=fuln num=-14
return=z option=-z name=-z arg=fulln num=-14
return=z option=-z name=--time-style arg=fullnumeric num=-14'

TEST 52 'the underscored suboption long and short of it'
	usage=$'[-][z:time_style?Time style.]:[style]{[11:iso?Iso.][12:posix_iso?Posix iso.][13:full_iso?Full iso.][14:full_numeric?Full numeric.]}'
	EXEC make "$usage" --time-style=iso --ts=i --time-style=full-iso --ts=fi --ts=fn --time-style=full-numeric -z fnumeric -z fulnumeric -z fuln -z fulln --timestyle=fullnumeric
		OUTPUT - $'return=z option=-z name=--time_style arg=iso num=-11
return=z option=-z name=--time_style arg=i num=-11
return=z option=-z name=--time_style arg=full-iso num=-13
return=z option=-z name=--time_style arg=fi num=-13
return=z option=-z name=--time_style arg=fn num=-14
return=z option=-z name=--time_style arg=full-numeric num=-14
return=z option=-z name=-z arg=fnumeric num=-14
return=z option=-z name=-z arg=fulnumeric num=-14
return=z option=-z name=-z arg=fuln num=-14
return=z option=-z name=-z arg=fulln num=-14
return=z option=-z name=--time_style arg=fullnumeric num=-14'
	EXEC make "$usage" --time_style=iso --ts=i --time_style=full_iso --ts=fi --ts=fn --time_style=full_numeric -z fnumeric -z fulnumeric -z fuln -z fulln --timestyle=fullnumeric
		OUTPUT - $'return=z option=-z name=--time_style arg=iso num=-11
return=z option=-z name=--time_style arg=i num=-11
return=z option=-z name=--time_style arg=full_iso num=-13
return=z option=-z name=--time_style arg=fi num=-13
return=z option=-z name=--time_style arg=fn num=-14
return=z option=-z name=--time_style arg=full_numeric num=-14
return=z option=-z name=-z arg=fnumeric num=-14
return=z option=-z name=-z arg=fulnumeric num=-14
return=z option=-z name=-z arg=fuln num=-14
return=z option=-z name=-z arg=fulln num=-14
return=z option=-z name=--time_style arg=fullnumeric num=-14'

TEST 53 'dashing long name input separator ingeniousness'
	usage=$'[-][p:prefixinclude?PREFIX-INCLUDE]'
	EXEC make "$usage" --prefixinclude --prefix-include --pre-fix-inc-lude
		OUTPUT - $'return=p option=-p name=--prefixinclude arg=(null) num=1
return=p option=-p name=--prefixinclude arg=(null) num=1
return=p option=-p name=--prefixinclude arg=(null) num=1'
	EXEC make "$usage" --prefixinclude --prefix_include --pre_fix_inc_lude
	EXEC make "$usage" --prefixinclude --prefix_include --pre_fix-inc_lude
	EXEC make "$usage" --?prefixinclude
		OUTPUT - $'return=? option=-? name=--?prefixinclude num=0'
		ERROR - $'Usage: make [ options ]
OPTIONS
  -p, --prefixinclude
                  PREFIX-INCLUDE'
		EXIT 2
	EXEC make "$usage" --?prefix-include
		OUTPUT - $'return=? option=-? name=--?prefix-include num=0'
	EXEC make "$usage" --?pre-fix-inc-lude
		OUTPUT - $'return=? option=-? name=--?pre-fix-inc-lude num=0'
	EXEC make "$usage" --?pre_fix_inc_lude
		OUTPUT - $'return=? option=-? name=--?pre_fix_inc_lude num=0'
	EXEC make "$usage" --?pre_fix-inc_lude
		OUTPUT - $'return=? option=-? name=--?pre_fix-inc_lude num=0'
	usage=$'[-][p:pre-fix_inc-lude?PREFIX-INCLUDE]'
	EXEC make "$usage" --prefixinclude --prefix-include --pre-fix-inc-lude
		OUTPUT - $'return=p option=-p name=--pre-fix_inc-lude arg=(null) num=1
return=p option=-p name=--pre-fix_inc-lude arg=(null) num=1
return=p option=-p name=--pre-fix_inc-lude arg=(null) num=1'
		ERROR -
		EXIT 0
	EXEC make "$usage" --prefixinclude --prefix_include --pre_fix_inc_lude
	EXEC make "$usage" --prefixinclude --prefix_include --pre_fix-inc_lude
	EXEC make "$usage" --?prefixinclude
		OUTPUT - $'return=? option=-? name=--?prefixinclude num=0'
		ERROR - $'Usage: make [ options ]
OPTIONS
  -p, --pre-fix_inc-lude
                  PREFIX-INCLUDE'
		EXIT 2
	EXEC make "$usage" --?prefix-include
		OUTPUT - $'return=? option=-? name=--?prefix-include num=0'
	EXEC make "$usage" --?pre-fix-inc-lude
		OUTPUT - $'return=? option=-? name=--?pre-fix-inc-lude num=0'
	EXEC make "$usage" --?pre_fix_inc_lude
		OUTPUT - $'return=? option=-? name=--?pre_fix_inc_lude num=0'
	EXEC make "$usage" --?pre_fix-inc_lude
		OUTPUT - $'return=? option=-? name=--?pre_fix-inc_lude num=0'

TEST 54 'boolean option with numeric value'
	usage=$'[-][103:reread]'
	EXEC make "$usage" --reread --noreread --reread=0 --reread=1 --reread=2
		OUTPUT - $'return=-103 option=-103 name=--reread arg=(null) num=1
return=-103 option=-103 name=--reread arg=(null) num=0
return=-103 option=-103 name=--reread arg=(null) num=0
return=-103 option=-103 name=--reread arg=(null) num=1
return=-103 option=-103 name=--reread arg=2 num=2'

TEST 55 'solaris old format long option name compatibility'
	usage=$'f:(file)(input-file)g(global)o:(output)(output-file)'
	EXEC oksh "$usage" --usage
		OUTPUT - $'return=? option=- name=--usage num=0'
		ERROR - $'[-][f:file|input-file]:[string][g:global][o:output|output-file]:[string]'
		EXIT 2
	EXEC oksh "$usage" -f in -g -o out
		OUTPUT - $'return=f option=-f name=-f arg=in num=1
return=g option=-g name=-g arg=(null) num=1
return=o option=-o name=-o arg=out num=1'
		ERROR -
		EXIT 0
	EXEC oksh "$usage" --file=in --glob --out=out
		OUTPUT - $'return=f option=-f name=--file arg=in num=1
return=g option=-g name=--global arg=(null) num=1
return=o option=-o name=--output arg=out num=1'
	usage=$'xf:(file)(input-file)g(global)o:(output)(output-file)'
	EXEC oksh "$usage" --usage
		OUTPUT - $'return=? option=- name=--usage num=0'
		ERROR - $'[-][x][f:file|input-file]:[string][g:global][o:output|output-file]:[string]'
		EXIT 2
	usage=$'x:f:(file)(input-file)g(global)o:(output)(output-file)'
	EXEC oksh "$usage" --usage
		ERROR - $'[-][x]:[string][f:file|input-file]:[string][g:global][o:output|output-file]:[string]'
	usage=$'f:(file)(input-file)g(global)xo:(output)(output-file)'
	EXEC oksh "$usage" --usage
		ERROR - $'[-][f:file|input-file]:[string][g:global][x][o:output|output-file]:[string]'
	usage=$'f:(file)(input-file)g(global)x:o:(output)(output-file)'
	EXEC oksh "$usage" --usage
		ERROR - $'[-][f:file|input-file]:[string][g:global][x]:[string][o:output|output-file]:[string]'

TEST 56 'old format does long options, well, with a lot of secondary work'
	usage=$'o:-:'
	EXEC oldisnew "$usage" -o old --boolean --name=value
		OUTPUT - $'return=o option=-o name=-o arg=old num=1
return=- option=-- name=-- arg=boolean num=1
return=- option=-- name=-- arg=name=value num=1'
	EXEC oldisnew "$usage" '--?'
		OUTPUT - $'return=? option=-? name=--? num=0'
		ERROR - $'Usage: oldisnew [-o arg] [--long-option[=value]]'
		EXIT 2

TEST 57 'and the new format does old options'
	usage=$'[-][f:FFF][n:NNN]#[nnn][s:SSS]:[sss]\n\nfile ...'
	EXEC newisold "$usage" --'??posix'
		OUTPUT - $'return=? option=-? name=--??posix num=0'
		ERROR - $'fn:s:'
		EXIT 2

TEST 58 'various --* combinations'
	usage=$'[-][f:format]:[format]'
	EXEC printf "$usage" - operand
		OUTPUT - $'argument=1 value="-"\nargument=2 value="operand"'
	EXEC printf "$usage" -- operand
		OUTPUT - $'argument=1 value="operand"'
	EXEC printf "$usage" -- --operand
		OUTPUT - $'argument=1 value="--operand"'
	EXEC printf "$usage" -- ---operand
		OUTPUT - $'argument=1 value="---operand"'
	EXEC printf "$usage" ---operand
		OUTPUT - $'argument=1 value="---operand"'
	EXEC

TEST 59 'optget options'
	usage=$'[-n?\n@(#)$Id: sort (AT&T Research) 2008-04-24 $\n]'
	EXEC sort "$usage" --man
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - $'SYNOPSIS
  sort [ options ]

IMPLEMENTATION
  version         sort (AT&T Research) 2008-04-24'
		EXIT 2

TEST 60 'about nested components'

	usage=$'[-][+NAME?boohoo][m:method?The methods are:]{[+hal?HAL]{[--hoo?HOO][-har?HAR][--huh?HUH]}[+bob?BOB]{[--boo?BOO][-bar?BAR][--buh?BUH]}}'
	EXEC sort "$usage" --man
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - $'NAME
  boohoo

SYNOPSIS
  boohoo [ options ]

OPTIONS
  -m, --method    The methods are:
                    hal   HAL
                            (har)           HAR
                            (huh)           HUH
                    bob   BOB
                            (bar)           BAR
                            (buh)           BUH'
		EXIT 2

	usage=$'[-][+NAME?boohoo][m:method?The methods are:]{[+hal?HAL]{[--hoo?HOO][-har?HAR]}[+bob?BOB]{[--boo?BOO][-bar?BAR]}}'
	EXEC sort "$usage" --man
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - $'NAME
  boohoo

SYNOPSIS
  boohoo [ options ]

OPTIONS
  -m, --method    The methods are:
                    hal   HAL
                            (har)           HAR
                    bob   BOB
                            (bar)           BAR'

	usage=$'[-][+NAME?boohoo][m:method?The methods are:]{[+hal?HAL]{[+hoo?HOO][-har?HAR]}[+bob?BOB]{[-boo?BOO][+bar?BAR]}}'
	EXEC sort "$usage" --man
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - $'NAME
  boohoo

SYNOPSIS
  boohoo [ options ]

OPTIONS
  -m, --method    The methods are:
                    hal   HAL
                            hoo   HOO
                            (har)           HAR
                    bob   BOB
                            (boo)           BOO
                            bar             BAR'

	usage=$'[-][+NAME?boohoo][m:method?The methods are:]{[+hal?HAL]{[--hoo?HOO][-har?HAR][--huh?HUH]}[+bob?BOB]{[--boo?BOO][-bar?BAR][--buh?BUH]}}'
	EXEC sort "$usage" --man
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - $'NAME
  boohoo

SYNOPSIS
  boohoo [ options ]

OPTIONS
  -m, --method    The methods are:
                    hal   HAL
                            (har)           HAR
                            (huh)           HUH
                    bob   BOB
                            (bar)           BAR
                            (buh)           BUH'

	info=$'[+hal?HAL]{[--hoo?HOO][-har?HAR][--huh?HUH]}[+bob?BOB]{[--boo?BOO][-bar?BAR][--buh?BUH]}'
	usage=$'[-][+NAME?boohoo][m:method?The methods are:]{\fTEST\f}'
	EXEC =TEST="$info" sort "$usage" --man

	info=$'[+hal?HAL]{[+hoo?HOO][-?\n@(#)$Id: zip::hoo (AT&T Research) 2003-01-01 $\n][-har?HAR][--xxx?yyy][--catalog?zip][+aha?AHA]}[+bob?BOB]{[-?\n@(#)$Id: zip::boo (AT&T Research) 2003-01-01 $\n][-bar?BAR]}'
	usage=$'[-][+NAME?boohoo][m:method?The methods are:]{\fTEST\f}'
	EXEC =TEST="$info" sort "$usage" --man
		ERROR - $'NAME
  boohoo

SYNOPSIS
  boohoo [ options ]

OPTIONS
  -m, --method    The methods are:
                    hal   HAL
                            hoo   HOO
                            (version)       zip::hoo (AT&T Research) 2003-01-01
                            (har)           HAR
                            (xxx)           yyy
                            (catalog)       zip
                            aha             AHA
                    bob   BOB
                            (version)       zip::boo (AT&T Research) 2003-01-01
                            (bar)           BAR'

	EXEC =TEST="$info" sort "$usage" --help
		OUTPUT - $'return=? option=- name=--help num=0'
		ERROR - $'Usage: sort [ options ]
OPTIONS
  -m, --method    The methods are:
                    hal   HAL
                            hoo   HOO
                            aha   AHA
                    bob   BOB'

	EXEC =TEST="$info" sort "$usage" --?method
		OUTPUT - $'return=? option=-? name=--?method num=0'
		ERROR - $'Usage: sort [ options ]
OPTIONS
  -m, --method    The methods are:
                    hal   HAL
                            hoo   HOO
                            aha   AHA
                    bob   BOB'

	usage=$'
[-?\n@(#)$Id: codex (AT&T Research) 2009-04-15 $\n]
[+NAME?codex - encode/decode filter]
[+?The supported methods are:]
    {
        [+zip\b?zip compression. The first parameter is the PKZIP
            compression method.]
            {
                [+copy|0?No compression.]
                [+shrink|1?Shrink compression.]
                [+reduce|2|3|4|5?Reduce compression.]
                [+implode|6?Implode compression.]
                [+deflate|8?Deflate compression.]
            }
    }
[d:decode?Apply the \amethod\a operand to the standard input only.]
[e:encode?Apply the \amethod\a operand to the standard output only.]
    
file ...

[+SEE ALSO?\bcodex\b(3), \bvcodex\b(3)]'
	EXEC codex "$usage" --man
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - $'NAME
  codex - encode/decode filter

SYNOPSIS
  codex [ options ] file ...

  The supported methods are:
    zip   zip compression. The first parameter is the PKZIP compression method.
            copy|0
                  No compression.
            shrink|1
                  Shrink compression.
            reduce|2|3|4|5
                  Reduce compression.
            implode|6
                  Implode compression.
            deflate|8
                  Deflate compression.

OPTIONS
  -d, --decode    Apply the method operand to the standard input only.
  -e, --encode    Apply the method operand to the standard output only.

SEE ALSO
  codex(3), vcodex(3)

IMPLEMENTATION
  version         codex (AT&T Research) 2009-04-15'

TEST 61 'man(1) section titles'

	usage=$'[-1s1M][+NAME?section]'
	EXEC section "$usage" '--???MAN'
		OUTPUT - 'return=? option=-? name=--???MAN num=0'
		ERROR - 'MAKE ASSERTION OPERATORS AND RULES'
		EXIT 2
	EXEC section "$usage" '--???MAN=5P'
		OUTPUT - 'return=? option=-? name=--???MAN num=0'
		ERROR - 'PLUGINS'
	EXEC section "$usage" '--???MAN=5PU'
		OUTPUT - 'return=? option=-? name=--???MAN num=0'
		ERROR - 'UWIN PLUGINS'
	EXEC section "$usage" '--???MAN=8U'
		OUTPUT - 'return=? option=-? name=--???MAN num=0'
		ERROR - 'UWIN ADMINISTRATIVE COMMANDS'

TEST 62 'conformance conditionals'

	usage=$'[-][a:all?ALL][(foo|ast)s:some?SOME][(foo|bar)n:never?NEVER][e:every?EVERY][(foo|bar)+?Specific text.]'
	EXEC conformance "$usage" -a
		OUTPUT - 'return=a option=-a name=-a arg=(null) num=1'
	EXEC conformance "$usage" --all
		OUTPUT - 'return=a option=-a name=--all arg=(null) num=1'
	EXEC conformance "$usage" -s
		OUTPUT - 'return=s option=-s name=-s arg=(null) num=1'
	EXEC conformance "$usage" --some
		OUTPUT - 'return=s option=-s name=--some arg=(null) num=1'
	EXEC conformance "$usage" -e
		OUTPUT - 'return=e option=-e name=-e arg=(null) num=1'
	EXEC conformance "$usage" --every
		OUTPUT - 'return=e option=-e name=--every arg=(null) num=1'
	EXEC conformance "$usage" -n
		EXIT 1
		OUTPUT - 'return=: option= name=-n num=0 str=[-][a:all?ALL][(foo|ast)s:some?SOME][(foo|bar)n:never?NEVER][e:every?EVERY][(foo|bar)+?Specific text.]'
		ERROR - $'conformance: -n: unknown option'
	EXEC conformance "$usage" --never
		OUTPUT - 'return=: option= name=--never num=0 str=--never'
		ERROR - 'conformance: --never: unknown option'
	EXEC conformance "$usage" --man
		EXIT 2
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - $'OPTIONS
  -a, --all       ALL
  -s, --some      [(foo|ast) conformance] SOME
  -n, --never     [(foo|bar) conformance] NEVER
  -e, --every     EVERY

  [(foo|bar) conformance] Specific text.

SYNOPSIS
  conformance [ options ]'

	EXEC conformance "$usage" --html
		OUTPUT - $'return=? option=- name=--html num=0'
		ERROR - '<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<HEAD>
<META name="generator" content="optget (AT&T Research) 2011-11-11">
<TITLE>conformance man document</TITLE>
<STYLE type="text/css">
div.SH { padding-left:2em; text-indent:0em; }
div.SY { padding-left:4em; text-indent:-2em; }
dt { float:left; clear:both; }
dd { margin-left:3em; }
</STYLE>
</HEAD>
<BODY bgcolor=white>
<H4><TABLE width=100%><TR><TH align=left>CONFORMANCE&nbsp;(&nbsp;1&nbsp;)&nbsp;<TH align=center><A href="." title="Index">USER COMMANDS</A><TH align=right>CONFORMANCE&nbsp;(&nbsp;1&nbsp;)</TR></TABLE></H4>
<HR>
<H4><A name="OPTIONS">OPTIONS</A></H4>
<DIV class=SH>
<DL>
<DT>-<B>a</B>, --<B>all</B><DD><BR>ALL
<DT>-<B>s</B>, --<B>some</B><DD><BR>&#0091;(foo|ast) conformance&#0093; SOME
<DT>-<B>n</B>, --<B>never</B><DD><BR>&#0091;(foo|bar) conformance&#0093; NEVER
<DT>-<B>e</B>, --<B>every</B><DD><BR>EVERY
<P>
<DT>&#0091;(foo|bar) conformance&#0093; Specific text.
<P>
</DL>
</DIV>
<H4><A name="SYNOPSIS">SYNOPSIS</A></H4>
<DIV class=SY>
<B>conformance</B> &#0091; <I>options</I> &#0093;
</DIV>
</BODY>
</HTML>'

# skip non-astsa (standalone ast) tests

[[ $($COMMAND -+ astsa '[-]' '--???about' 2>/dev/null) == *catalog=libast* ]] || exit

TEST 98 'translation'
	usage=$'[-][+NAME?aha - just do it][+DESCRIPTION?Bla bla.]{\fzero\f}\n\n[ dialect ]\n\n[+SEE ALSO?Bla.]'
	EXEC -+ aha "$usage" --man
		OUTPUT - $'id=aha catalog=libast text="about"
id=aha catalog=libast text="api"
id=aha catalog=libast text="help"
id=aha catalog=libast text="html"
id=aha catalog=libast text="keys"
id=aha catalog=libast text="long"
id=aha catalog=libast text="man"
id=aha catalog=libast text="about"
id=aha catalog=libast text="api"
id=aha catalog=libast text="help"
id=aha catalog=libast text="html"
id=aha catalog=libast text="keys"
id=aha catalog=libast text="long"
id=aha catalog=libast text="man"
id=aha catalog=libast text="NAME"
id=aha catalog=libast text="aha - just do it"
id=aha catalog=libast text="DESCRIPTION"
id=aha catalog=libast text="Bla bla."
id=aha catalog=libast text="dabba"
id=aha catalog=libast text="aroni"
id=aha catalog=libast text="SEE ALSO"
id=aha catalog=libast text="Bla."
id=aha catalog=libast text="SYNOPSIS"
id=aha catalog=libast text="options"
id=aha catalog=libast text="[ dialect ]"
return=? option=- name=--man num=0
id=aha catalog=libast text="Usage"'
		ERROR - $'ANZR
  nun - whfg qb vg

FLABCFVF
  aha [ bcgvbaf ] [ qvnyrpg ]

QRFPEVCGVBA
  Oyn oyn.
    yabba qnoon
    doo   nebav

FRR NYFB
  Oyn.'
		EXIT 2
	usage=$getopts_usage
	EXEC	-+ getopts "$usage" '--man'
		OUTPUT - $'id=getopts catalog=libast text="command"
id=getopts catalog=libast text="about"
id=getopts catalog=libast text="api"
id=getopts catalog=libast text="help"
id=getopts catalog=libast text="html"
id=getopts catalog=libast text="keys"
id=getopts catalog=libast text="long"
id=getopts catalog=libast text="man"
id=getopts catalog=libast text="about"
id=getopts catalog=libast text="api"
id=getopts catalog=libast text="help"
id=getopts catalog=libast text="html"
id=getopts catalog=libast text="keys"
id=getopts catalog=libast text="long"
id=getopts catalog=libast text="man"
id=getopts catalog=libast text="version"
id=getopts catalog=libast text="getopts (AT&T Research) 1999-02-02
"
id=getopts catalog=libast text="NAME"
id=getopts catalog=libast text="? - parse utility options"
id=getopts catalog=libast text="OPTIONS"
id=getopts catalog=libast text="command"
id=getopts catalog=libast text="name"
id=getopts catalog=libast text="Use \aname\a instead of the command name in usage messages."
id=getopts catalog=libast text="DESCRIPTION"
id=getopts catalog=libast text="The \bgetopts\b utility can be used to retrieve options and
arguments from a list of arguments give by \aargs\a or the positional
parameters if \aargs\a is omitted.  It can also generate usage messages
and a man page for the command based on the information in \aoptstring\a."
id=getopts catalog=libast text="The \aoptstring\a string consists of alpha-numeric characters,
the special characters +, -, ?, :, and <space>, or character groups
enclosed in [...].  Character groups may be nested in {...}.
Outside of a [...] group, a single new-line followed by zero or
more blanks is ignored.  One or more blank lines separates the
options from the command argument synopsis."
id=getopts catalog=libast text="Each [...] group consists of an optional label,
optional attributes separated by :, and an
optional description string following ?.  The characters from the ?
to the end of the next ] are ignored for option parsing and short
usage messages.  They are used for generating verbose help or man pages.
The : character may not appear in the label.
The ? character must be specified as ?? in label and the ] character
must be specified as ]] in the description string.
Text between two \\b (backspace) characters indicates
that the text should be emboldened when displayed.
Text between two \\a (bell) characters indicates that the text should
be emphasised or italicised when displayed."
id=getopts catalog=libast text="There are four types of groups:"
id=getopts catalog=libast text="An option specifiation of the form \aoption\a:\alongname\a.
	In this case the first field is the option character.  If there
	is no option character, then a two digit number should be specified
	that corresponds to the long options.  This negative of this number
	will be returned as the value of \aname\a by \bgetopts\b if the long
	option is matched. A longname is matched with \b--\b\alongname\a.  A
	* in the \alongname\a field indicates that only characters up that
	point need to match provided any additional characters match the option.
	The [ and ] can be omitted for options that don\'t have longnames
	or descriptive text."
id=getopts catalog=libast text="A string option argument specification.
	Options that take arguments can be followed by : or # and an option
	group specification.  An option group specification consists
	of a name for the option argument as field 1.   The remaining
	fields are a typename and zero or more of the special attribute words
	\blistof\b, \boneof\b, and \bignorecase\b.
	The option specification can be followed
	by a list of option value descriptions enclosed in parenthesis."
id=getopts catalog=libast text="A option value description."
id=getopts catalog=libast text="A argument specification. A list of valid option argument values
		can be specified by enclosing them inside a {...} following
		the option argument specification.  Each of the permitted
		values can be specified with a [...] containing the
		value followed by a description."
id=getopts catalog=libast text="If the leading character of \aoptstring\a is +, then arguments
beginning with + will also be considered options."
id=getopts catalog=libast text="A leading : character or a : following a leading + in \aoptstring\a
affects the way errors are handled.  If an option character or longname
argument not specified in \aoptstring\a is encountered when processing
options, the shell variable whose name is \aname\a will be set to the ?
character.  The shell variable \bOPTARG\b will be set to
the character found.  If an option argument is missing or has an invalid
value, then \aname\a will be set to the : character and the shell variable
\bOPTARG\b will be set to the option character found.
Without the leading :, \aname\a will be set to the ? character, \bOPTARG\b
will be unset, and an error message will be written to standard error
when errors are encountered."
id=getopts catalog=libast text="The end of options occurs when:"
id=getopts catalog=libast text="The special argument \b--\b."
id=getopts catalog=libast text="An argument that does not beging with a \b-\b."
id=getopts catalog=libast text="A help argument is specified."
id=getopts catalog=libast text="An error is encountered."
id=getopts catalog=libast text="If \bOPTARG\b is set to the value \b1\b, a new set of arguments
can be used."
id=getopts catalog=libast text="\bgetopts\b can also be used to generate help messages containing command
usage and detailed descriptions.  Specify \aargs\a as:"
id=getopts catalog=libast text="To generate a usage synopsis."
id=getopts catalog=libast text="To generate a verbose usage message."
id=getopts catalog=libast text="To generate a formatted man page."
id=getopts catalog=libast text="To generate an easy to parse usage message."
id=getopts catalog=libast text="To generate a man page in \bhtml\b format."
id=getopts catalog=libast text="When the end of options is encountered, \bgetopts\b exits with a
non-zero return value and the variable \bOPTIND\b is set to the
index of the first non-option argument."
id=getopts catalog=libast text="EXIT STATUS"
id=getopts catalog=libast text="An option specified was found."
id=getopts catalog=libast text="An end of options was encountered."
id=getopts catalog=libast text="A usage or information message was generated."
id=getopts catalog=libast text="IMPLEMENTATION"
id=getopts catalog=libast text="SYNOPSIS"
id=getopts catalog=libast text="options"
id=getopts catalog=libast text="opstring name [args...]"
return=? option=- name=--man num=0
id=getopts catalog=libast text="Usage"'
		ERROR - $'ANZR
  getopts - cnefr hgvyvgl bcgvbaf

FLABCFVF
  getopts [ bcgvbaf ] bcfgevat anzr [netf...]

BCGVBAF
  -a, --pbzznaq|command=anzr
                  Hfr anzr vafgrnq bs gur pbzznaq anzr va hfntr zrffntrf.

QRFPEVCGVBA
  Gur getopts hgvyvgl pna or hfrq gb ergevrir bcgvbaf naq nethzragf sebz n yvfg
  bs nethzragf tvir ol netf be gur cbfvgvbany cnenzrgref vs netf vf bzvggrq. Vg
  pna nyfb trarengr hfntr zrffntrf naq n zna cntr sbe gur pbzznaq onfrq ba gur
  vasbezngvba va bcgfgevat.

  Gur bcgfgevat fgevat pbafvfgf bs nycun-ahzrevp punenpgref, gur fcrpvny
  punenpgref +, -, ?, :, naq <fcnpr>, be punenpgre tebhcf rapybfrq va [...].
  Punenpgre tebhcf znl or arfgrq va {...}. Bhgfvqr bs n [...] tebhc, n fvatyr
  arj-yvar sbyybjrq ol mreb be zber oynaxf vf vtaberq. Bar be zber oynax yvarf
  frcnengrf gur bcgvbaf sebz gur pbzznaq nethzrag flabcfvf.

  Rnpu [...] tebhc pbafvfgf bs na bcgvbany ynory, bcgvbany nggevohgrf frcnengrq
  ol :, naq na bcgvbany qrfpevcgvba fgevat sbyybjvat ?. Gur punenpgref sebz gur
  ? gb gur raq bs gur arkg ] ner vtaberq sbe bcgvba cnefvat naq fubeg hfntr
  zrffntrf. Gurl ner hfrq sbe trarengvat ireobfr uryc be zna cntrf. Gur :
  punenpgre znl abg nccrne va gur ynory. Gur ? punenpgre zhfg or fcrpvsvrq nf
  ?? va ynory naq gur ] punenpgre zhfg or fcrpvsvrq nf ]] va gur qrfpevcgvba
  fgevat. Grkg orgjrra gjb \\b (onpxfcnpr) punenpgref vaqvpngrf gung gur grkg
  fubhyq or rzobyqrarq jura qvfcynlrq. Grkg orgjrra gjb \\a (oryy) punenpgref
  vaqvpngrf gung gur grkg fubhyq or rzcunfvfrq be vgnyvpvfrq jura qvfcynlrq.

  Gurer ner sbhe glcrf bs tebhcf:
    1.    Na bcgvba fcrpvsvngvba bs gur sbez bcgvba:ybatanzr. Va guvf pnfr gur
          svefg svryq vf gur bcgvba punenpgre. Vs gurer vf ab bcgvba punenpgre,
          gura n gjb qvtvg ahzore fubhyq or fcrpvsvrq gung pbeerfcbaqf gb gur
          ybat bcgvbaf. Guvf artngvir bs guvf ahzore jvyy or erghearq nf gur
          inyhr bs anzr ol getopts vs gur ybat bcgvba vf zngpurq. N ybatanzr vf
          zngpurq jvgu --ybatanzr. N * va gur ybatanzr svryq vaqvpngrf gung
          bayl punenpgref hc gung cbvag arrq gb zngpu cebivqrq nal nqqvgvbany
          punenpgref zngpu gur bcgvba. Gur [ naq ] pna or bzvggrq sbe bcgvbaf
          gung qba\'g unir ybatanzrf be qrfpevcgvir grkg.
    2.    N fgevat bcgvba nethzrag fcrpvsvpngvba. Bcgvbaf gung gnxr nethzragf
          pna or sbyybjrq ol : be # naq na bcgvba tebhc fcrpvsvpngvba. Na
          bcgvba tebhc fcrpvsvpngvba pbafvfgf bs n anzr sbe gur bcgvba nethzrag
          nf svryq 1. Gur erznvavat svryqf ner n glcranzr naq mreb be zber bs
          gur fcrpvny nggevohgr jbeqf listof, oneof, naq ignorecase. Gur bcgvba
          fcrpvsvpngvba pna or sbyybjrq ol n yvfg bs bcgvba inyhr qrfpevcgvbaf
          rapybfrq va cneragurfvf.
    3.    N bcgvba inyhr qrfpevcgvba.
    4.    N nethzrag fcrpvsvpngvba. N yvfg bs inyvq bcgvba nethzrag inyhrf pna
          or fcrpvsvrq ol rapybfvat gurz vafvqr n {...} sbyybjvat gur bcgvba
          nethzrag fcrpvsvpngvba. Rnpu bs gur crezvggrq inyhrf pna or fcrpvsvrq
          jvgu n [...] pbagnvavat gur inyhr sbyybjrq ol n qrfpevcgvba.

  Vs gur yrnqvat punenpgre bs bcgfgevat vf +, gura nethzragf ortvaavat jvgu +
  jvyy nyfb or pbafvqrerq bcgvbaf.

  N yrnqvat : punenpgre be n : sbyybjvat n yrnqvat + va bcgfgevat nssrpgf gur
  jnl reebef ner unaqyrq. Vs na bcgvba punenpgre be ybatanzr nethzrag abg
  fcrpvsvrq va bcgfgevat vf rapbhagrerq jura cebprffvat bcgvbaf, gur furyy
  inevnoyr jubfr anzr vf anzr jvyy or frg gb gur ? punenpgre. Gur furyy
  inevnoyr OPTARG jvyy or frg gb gur punenpgre sbhaq. Vs na bcgvba nethzrag vf
  zvffvat be unf na vainyvq inyhr, gura anzr jvyy or frg gb gur : punenpgre naq
  gur furyy inevnoyr OPTARG jvyy or frg gb gur bcgvba punenpgre sbhaq. Jvgubhg
  gur yrnqvat :, anzr jvyy or frg gb gur ? punenpgre, OPTARG jvyy or hafrg, naq
  na reebe zrffntr jvyy or jevggra gb fgnaqneq reebe jura reebef ner
  rapbhagrerq.

  Gur raq bs bcgvbaf bpphef jura:
    1.    Gur fcrpvny nethzrag --.
    2.    Na nethzrag gung qbrf abg ortvat jvgu n -.
    3.    N uryc nethzrag vf fcrpvsvrq.
    4.    Na reebe vf rapbhagrerq.

  Vs OPTARG vf frg gb gur inyhr 1, n arj frg bs nethzragf pna or hfrq.

  getopts pna nyfb or hfrq gb trarengr uryc zrffntrf pbagnvavat pbzznaq hfntr
  naq qrgnvyrq qrfpevcgvbaf. Fcrpvsl netf nf:
    -?    Gb trarengr n hfntr flabcfvf.
    --??  Gb trarengr n ireobfr hfntr zrffntr.
    --??man
          Gb trarengr n sbeznggrq zna cntr.
    --??api
          Gb trarengr na rnfl gb cnefr hfntr zrffntr.
    --??html
          Gb trarengr n zna cntr va html sbezng.

  Jura gur raq bs bcgvbaf vf rapbhagrerq, getopts rkvgf jvgu n aba-mreb erghea
  inyhr naq gur inevnoyr OPTIND vf frg gb gur vaqrk bs gur svefg aba-bcgvba
  nethzrag.

RKVG FGNGHF
    0     Na bcgvba fcrpvsvrq jnf sbhaq.
    1     Na raq bs bcgvbaf jnf rapbhagrerq.
    2     N hfntr be vasbezngvba zrffntr jnf trarengrq.

VZCYRZRAGNGVBA
  irefvba         trgbcgf (NG&G Erfrnepu) 1999-02-02'
	usage=$'[-?\n@(#)xlate 1.0\n][-author?Col. Hyde][a:algorithm]:[method][b:again|back]'
	EXEC -+ xlate "$usage" --algorithm=xxx --again --back
		OUTPUT - $'id=xlate catalog=libast text="algorithm"
return=a option=-a name=--algorithm arg=xxx num=1
id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="again|back"
return=b option=-b name=--again arg=(null) num=1
id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="again|back"
return=b option=-b name=--back arg=(null) num=1'
		ERROR -
		EXIT 0
	EXEC -+ xlate "$usage" --nytbevguz=xxx --ntnva --onpx
		OUTPUT - $'id=xlate catalog=libast text="algorithm"
return=a option=-a name=--nytbevguz arg=xxx num=1
id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="again|back"
return=b option=-b name=--ntnva arg=(null) num=1
id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="again|back"
return=b option=-b name=--onpx arg=(null) num=1'
	EXEC -+ xlate "$usage" --algorithm
		EXIT 1
		OUTPUT - $'id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="method"
id=xlate catalog=libast text="value expected"
return=: option=-a name=--algorithm num=0
id=xlate catalog=libast text="%s"'
		ERROR - $'xlate: --algorithm: zrgubq inyhr rkcrpgrq'
	EXEC -+ xlate "$usage" --nytbevguz
		OUTPUT - $'id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="method"
id=xlate catalog=libast text="value expected"
return=: option=-a name=--nytbevguz num=0
id=xlate catalog=libast text="%s"'
		ERROR - $'xlate: --nytbevguz: zrgubq inyhr rkcrpgrq'
	EXEC -+ xlate "$usage" --man
		EXIT 2
		OUTPUT - $'id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="about"
id=xlate catalog=libast text="api"
id=xlate catalog=libast text="help"
id=xlate catalog=libast text="html"
id=xlate catalog=libast text="keys"
id=xlate catalog=libast text="long"
id=xlate catalog=libast text="man"
id=xlate catalog=libast text="about"
id=xlate catalog=libast text="api"
id=xlate catalog=libast text="help"
id=xlate catalog=libast text="html"
id=xlate catalog=libast text="keys"
id=xlate catalog=libast text="long"
id=xlate catalog=libast text="man"
id=xlate catalog=libast text="version"
id=xlate catalog=libast text="xlate 1.0
"
id=xlate catalog=libast text="author"
id=xlate catalog=libast text="Col. Hyde"
id=xlate catalog=libast text="OPTIONS"
id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="method"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="IMPLEMENTATION"
id=xlate catalog=libast text="SYNOPSIS"
id=xlate catalog=libast text="options"
return=? option=- name=--man num=0
id=xlate catalog=libast text="Usage"'
		ERROR - $'FLABCFVF
  xlate [ bcgvbaf ]

BCGVBAF
  -a, --nytbevguz|algorithm=zrgubq
  -b, --ntnva|onpx|again|back

VZCYRZRAGNGVBA
  irefvba         kyngr 1.0 
  nhgube          Pby. Ulqr'
	EXEC -+ xlate "$usage" -?
		OUTPUT - $'id=xlate catalog=libast text="method"
id=xlate catalog=libast text="options"
return=? option=-? name=-? num=0
id=xlate catalog=libast text="Usage"'
		ERROR - $'Hfntr: xlate [-b] [-a zrgubq]'
	EXEC -+ xlate "$usage" --?
		OUTPUT - $'id=xlate catalog=libast text="OPTIONS"
id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="method"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="options"
return=? option=-? name=--? num=0
id=xlate catalog=libast text="Usage"'
		ERROR - $'Hfntr: xlate [ bcgvbaf ]
BCGVBAF
  -a, --nytbevguz|algorithm=zrgubq
  -b, --ntnva|onpx|again|back'
	EXEC -+ xlate "$usage" --??
		OUTPUT - $'id=xlate catalog=libast text="version"
id=xlate catalog=libast text="xlate 1.0
"
id=xlate catalog=libast text="author"
id=xlate catalog=libast text="Col. Hyde"
id=xlate catalog=libast text="OPTIONS"
id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="method"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="IMPLEMENTATION"
id=xlate catalog=libast text="SYNOPSIS"
id=xlate catalog=libast text="options"
return=? option=-? name=--?? num=0
id=xlate catalog=libast text="Usage"'
		ERROR - $'FLABCFVF
  xlate [ bcgvbaf ]

BCGVBAF
  -a, --nytbevguz|algorithm=zrgubq
  -b, --ntnva|onpx|again|back

VZCYRZRAGNGVBA
  irefvba         kyngr 1.0 
  nhgube          Pby. Ulqr'
	EXEC -+ xlate "$usage" --???
		OUTPUT - $'id=xlate catalog=libast text="about"
id=xlate catalog=libast text="api"
id=xlate catalog=libast text="help"
id=xlate catalog=libast text="html"
id=xlate catalog=libast text="keys"
id=xlate catalog=libast text="long"
id=xlate catalog=libast text="man"
id=xlate catalog=libast text="nroff"
id=xlate catalog=libast text="options"
id=xlate catalog=libast text="posix"
id=xlate catalog=libast text="short"
id=xlate catalog=libast text="usage"
id=xlate catalog=libast text="NAME"
id=xlate catalog=libast text="options available to all \bast\b commands"
id=xlate catalog=libast text="DESCRIPTION"
id=xlate catalog=libast text="\b-?\b and \b--?\b* options are the same for all \bast\b commands. For any \aitem\a below, if \b--\b\aitem\a is not supported by a given command then it is equivalent to \b--??\b\aitem\a. The \b--??\b form should be used for portability. All output is written to the standard error."
id=xlate catalog=libast text="OPTIONS"
id=xlate catalog=libast text="about"
id=xlate catalog=libast text="List all implementation info."
id=xlate catalog=libast text="api"
id=xlate catalog=libast text="List detailed info in program readable form."
id=xlate catalog=libast text="help"
id=xlate catalog=libast text="List detailed help option info."
id=xlate catalog=libast text="html"
id=xlate catalog=libast text="List detailed info in html."
id=xlate catalog=libast text="keys"
id=xlate catalog=libast text="List the usage translation key strings with C style escapes."
id=xlate catalog=libast text="long"
id=xlate catalog=libast text="List long option usage."
id=xlate catalog=libast text="man"
id=xlate catalog=libast text="List detailed info in displayed man page form."
id=xlate catalog=libast text="nroff"
id=xlate catalog=libast text="List detailed info in nroff."
id=xlate catalog=libast text="options"
id=xlate catalog=libast text="List short and long option details."
id=xlate catalog=libast text="posix"
id=xlate catalog=libast text="List posix getopt usage."
id=xlate catalog=libast text="short"
id=xlate catalog=libast text="List short option usage."
id=xlate catalog=libast text="usage"
id=xlate catalog=libast text="List the usage string with C style escapes."
id=xlate catalog=libast text="?-\alabel\a"
id=xlate catalog=libast text="List implementation info matching \alabel\a*."
id=xlate catalog=libast text="?\aname\a"
id=xlate catalog=libast text="Equivalent to \b--help=\b\aname\a."
id=xlate catalog=libast text="?"
id=xlate catalog=libast text="Equivalent to \b--??options\b."
id=xlate catalog=libast text="??"
id=xlate catalog=libast text="Equivalent to \b--??man\b."
id=xlate catalog=libast text="???"
id=xlate catalog=libast text="Equivalent to \b--??help\b."
id=xlate catalog=libast text="???\aitem\a"
id=xlate catalog=libast text="If the next argument is \b--\b\aoption\a then list the \aoption\a output in the \aitem\a style. Otherwise print \bversion=\b\an\a where \an\a>0 if \b--??\b\aitem\a is supported, \b0\b if not."
id=xlate catalog=libast text="???ESC"
id=xlate catalog=libast text="Emit escape codes even if output is not a terminal."
id=xlate catalog=libast text="???MAN[=\asection\a]"
id=xlate catalog=libast text="List the \bman\b(1) section title for \asection\a [the current command]."
id=xlate catalog=libast text="???SECTION"
id=xlate catalog=libast text="List the \bman\b(1) section number for the current command."
id=xlate catalog=libast text="???TEST"
id=xlate catalog=libast text="Massage the output for regression testing."
id=xlate catalog=libast text="SYNOPSIS"
id=xlate catalog=libast text="options"
return=? option=-? name=--??? num=0
id=xlate catalog=libast text="Usage"'
		ERROR - $'ANZR
  bcgvbaf ninvynoyr gb nyy ast pbzznaqf

FLABCFVF
  xlate [ bcgvbaf ]

QRFPEVCGVBA
  -? naq --?* bcgvbaf ner gur fnzr sbe nyy ast pbzznaqf. Sbe nal vgrz orybj, vs
  --vgrz vf abg fhccbegrq ol n tvira pbzznaq gura vg vf rdhvinyrag gb --??vgrz.
  Gur --?? sbez fubhyq or hfrq sbe cbegnovyvgl. Nyy bhgchg vf jevggra gb gur
  fgnaqneq reebe.

BCGVBAF
  --nobhg|about   Yvfg nyy vzcyrzragngvba vasb.
  --ncv|api       Yvfg qrgnvyrq vasb va cebtenz ernqnoyr sbez.
  --uryc|help     Yvfg qrgnvyrq uryc bcgvba vasb.
  --ugzy|html     Yvfg qrgnvyrq vasb va ugzy.
  --xrlf|keys     Yvfg gur hfntr genafyngvba xrl fgevatf jvgu P fglyr rfpncrf.
  --ybat|long     Yvfg ybat bcgvba hfntr.
  --zna|man       Yvfg qrgnvyrq vasb va qvfcynlrq zna cntr sbez.
  --aebss|nroff   Yvfg qrgnvyrq vasb va aebss.
  --bcgvbaf|options
                  Yvfg fubeg naq ybat bcgvba qrgnvyf.
  --cbfvk|posix   Yvfg cbfvk trgbcg hfntr.
  --fubeg|short   Yvfg fubeg bcgvba hfntr.
  --hfntr|usage   Yvfg gur hfntr fgevat jvgu P fglyr rfpncrf.
  --?-ynory|?-label
                  Yvfg vzcyrzragngvba vasb zngpuvat ynory*.
  --?anzr|?name   Rdhvinyrag gb --help=anzr.
  --?             Rdhvinyrag gb --??options.
  --??            Rdhvinyrag gb --??man.
  --???           Rdhvinyrag gb --??help.
  --???vgrz|???item
                  Vs gur arkg nethzrag vf --bcgvba gura yvfg gur bcgvba bhgchg
                  va gur vgrz fglyr. Bgurejvfr cevag version=a jurer a>0 vs
                  --??vgrz vf fhccbegrq, 0 vs abg.
  --???RFP|???ESC Rzvg rfpncr pbqrf rira vs bhgchg vf abg n grezvany.
  --???ZNA[=frpgvba]|???MAN[=section]
                  Yvfg gur man(1) frpgvba gvgyr sbe frpgvba
                  [gur pheerag pbzznaq].
  --???FRPGVBA|???SECTION
                  Yvfg gur man(1) frpgvba ahzore sbe gur pheerag pbzznaq.
  --???GRFG|???TEST
                  Znffntr gur bhgchg sbe erterffvba grfgvat.'
	EXEC -+ xlate "$usage" --keys
		OUTPUT - $'id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="about"
id=xlate catalog=libast text="api"
id=xlate catalog=libast text="help"
id=xlate catalog=libast text="html"
id=xlate catalog=libast text="keys"
id=xlate catalog=libast text="about"
id=xlate catalog=libast text="api"
id=xlate catalog=libast text="help"
id=xlate catalog=libast text="html"
id=xlate catalog=libast text="keys"
return=? option=- name=--keys num=0
id=xlate catalog=libast text="Usage"'
		ERROR - $'"algorithm"
"method"
"again|back"'
	usage=$usage$'[+EXAMPLES?Examples.]{[+foo?Foo bar.][+\abar\a?Bar foo.]}[+?More e.g.]{[+aha?AHA]}'
	EXEC -+ xlate "$usage" --keys
		OUTPUT - $'id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="about"
id=xlate catalog=libast text="api"
id=xlate catalog=libast text="help"
id=xlate catalog=libast text="html"
id=xlate catalog=libast text="keys"
id=xlate catalog=libast text="about"
id=xlate catalog=libast text="api"
id=xlate catalog=libast text="help"
id=xlate catalog=libast text="html"
id=xlate catalog=libast text="keys"
return=? option=- name=--keys num=0
id=xlate catalog=libast text="Usage"'
		ERROR - $'"algorithm"
"method"
"again|back"
"Examples."
"Foo bar."
"\\abar\\a"
"Bar foo."
"More e.g."
"AHA"'
	EXEC -+ xlate "$usage" --man
		OUTPUT - $'id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="about"
id=xlate catalog=libast text="api"
id=xlate catalog=libast text="help"
id=xlate catalog=libast text="html"
id=xlate catalog=libast text="keys"
id=xlate catalog=libast text="long"
id=xlate catalog=libast text="man"
id=xlate catalog=libast text="about"
id=xlate catalog=libast text="api"
id=xlate catalog=libast text="help"
id=xlate catalog=libast text="html"
id=xlate catalog=libast text="keys"
id=xlate catalog=libast text="long"
id=xlate catalog=libast text="man"
id=xlate catalog=libast text="version"
id=xlate catalog=libast text="xlate 1.0
"
id=xlate catalog=libast text="author"
id=xlate catalog=libast text="Col. Hyde"
id=xlate catalog=libast text="OPTIONS"
id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="method"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="EXAMPLES"
id=xlate catalog=libast text="Examples."
id=xlate catalog=libast text="Foo bar."
id=xlate catalog=libast text="\abar\a"
id=xlate catalog=libast text="Bar foo."
id=xlate catalog=libast text="More e.g."
id=xlate catalog=libast text="AHA"
id=xlate catalog=libast text="IMPLEMENTATION"
id=xlate catalog=libast text="SYNOPSIS"
id=xlate catalog=libast text="options"
return=? option=- name=--man num=0
id=xlate catalog=libast text="Usage"'
		ERROR - $'BCGVBAF
  -a, --nytbevguz|algorithm=zrgubq
  -b, --ntnva|onpx|again|back

RKNZCYRF
  Rknzcyrf.
    foo   Sbb one.
    one   One sbb.

  Zber r.t.
    aha   NUN

FLABCFVF
  xlate [ bcgvbaf ]

VZCYRZRAGNGVBA
  irefvba         kyngr 1.0 
  nhgube          Pby. Ulqr'
	EXEC -+ xlate "$usage" --?-auth
		OUTPUT - $'id=xlate catalog=libast text="version"
id=xlate catalog=libast text="version"
id=xlate catalog=libast text="author"
id=xlate catalog=libast text="author?Col. Hyde][a:algorithm]:[method][b:again|back][+EXAMPLES?Examples.]{[+foo?Foo bar.][+\abar\a?Bar foo.]}[+?More e.g.]{[+aha?AHA]}"
id=xlate catalog=libast text="author"
id=xlate catalog=libast text="Col. Hyde"
id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="algorithm]:[method][b:again|back][+EXAMPLES?Examples.]{[+foo?Foo bar.][+\abar\a?Bar foo.]}[+?More e.g.]{[+aha?AHA]}"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="again|back][+EXAMPLES?Examples.]{[+foo?Foo bar.][+\abar\a?Bar foo.]}[+?More e.g.]{[+aha?AHA]}"
return=? option=-? name=--?-auth num=0
id=xlate catalog=libast text="Usage"'
		ERROR - $'  nhgube          Pby. Ulqr'
		EXIT 2
	EXEC -+ xlate "$usage" --?-nhgu
		OUTPUT - $'id=xlate catalog=libast text="version"
id=xlate catalog=libast text="version"
id=xlate catalog=libast text="author"
id=xlate catalog=libast text="author?Col. Hyde][a:algorithm]:[method][b:again|back][+EXAMPLES?Examples.]{[+foo?Foo bar.][+\abar\a?Bar foo.]}[+?More e.g.]{[+aha?AHA]}"
id=xlate catalog=libast text="author"
id=xlate catalog=libast text="Col. Hyde"
id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="algorithm]:[method][b:again|back][+EXAMPLES?Examples.]{[+foo?Foo bar.][+\abar\a?Bar foo.]}[+?More e.g.]{[+aha?AHA]}"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="again|back][+EXAMPLES?Examples.]{[+foo?Foo bar.][+\abar\a?Bar foo.]}[+?More e.g.]{[+aha?AHA]}"
return=? option=-? name=--?-nhgu num=0
id=xlate catalog=libast text="Usage"'
		ERROR - $'  nhgube          Pby. Ulqr'
	EXEC -+ xlate "$usage" --zna
		OUTPUT - $'id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="about"
id=xlate catalog=libast text="api"
id=xlate catalog=libast text="help"
id=xlate catalog=libast text="html"
id=xlate catalog=libast text="keys"
id=xlate catalog=libast text="long"
id=xlate catalog=libast text="man"
id=xlate catalog=libast text="about"
id=xlate catalog=libast text="api"
id=xlate catalog=libast text="help"
id=xlate catalog=libast text="html"
id=xlate catalog=libast text="keys"
id=xlate catalog=libast text="long"
id=xlate catalog=libast text="man"
id=xlate catalog=libast text="version"
id=xlate catalog=libast text="xlate 1.0
"
id=xlate catalog=libast text="author"
id=xlate catalog=libast text="Col. Hyde"
id=xlate catalog=libast text="OPTIONS"
id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="method"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="EXAMPLES"
id=xlate catalog=libast text="Examples."
id=xlate catalog=libast text="Foo bar."
id=xlate catalog=libast text="\abar\a"
id=xlate catalog=libast text="Bar foo."
id=xlate catalog=libast text="More e.g."
id=xlate catalog=libast text="AHA"
id=xlate catalog=libast text="IMPLEMENTATION"
id=xlate catalog=libast text="SYNOPSIS"
id=xlate catalog=libast text="options"
return=? option=- name=--zna num=0
id=xlate catalog=libast text="Usage"'
		ERROR - $'BCGVBAF
  -a, --nytbevguz|algorithm=zrgubq
  -b, --ntnva|onpx|again|back

RKNZCYRF
  Rknzcyrf.
    foo   Sbb one.
    one   One sbb.

  Zber r.t.
    aha   NUN

FLABCFVF
  xlate [ bcgvbaf ]

VZCYRZRAGNGVBA
  irefvba         kyngr 1.0 
  nhgube          Pby. Ulqr'
		EXIT 2
	EXEC -+ xlate "$usage" --??zna
		OUTPUT - $'id=xlate catalog=libast text="about"
id=xlate catalog=libast text="api"
id=xlate catalog=libast text="help"
id=xlate catalog=libast text="html"
id=xlate catalog=libast text="keys"
id=xlate catalog=libast text="long"
id=xlate catalog=libast text="man"
id=xlate catalog=libast text="version"
id=xlate catalog=libast text="xlate 1.0
"
id=xlate catalog=libast text="author"
id=xlate catalog=libast text="Col. Hyde"
id=xlate catalog=libast text="OPTIONS"
id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="method"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="EXAMPLES"
id=xlate catalog=libast text="Examples."
id=xlate catalog=libast text="Foo bar."
id=xlate catalog=libast text="\abar\a"
id=xlate catalog=libast text="Bar foo."
id=xlate catalog=libast text="More e.g."
id=xlate catalog=libast text="AHA"
id=xlate catalog=libast text="IMPLEMENTATION"
id=xlate catalog=libast text="SYNOPSIS"
id=xlate catalog=libast text="options"
return=? option=-? name=--??zna num=0
id=xlate catalog=libast text="Usage"'
	EXEC -+ xlate "$usage" --?again
		OUTPUT - $'id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="algorithm]:[method][b:again|back][+EXAMPLES?Examples.]{[+foo?Foo bar.][+\abar\a?Bar foo.]}[+?More e.g.]{[+aha?AHA]}"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="again|back][+EXAMPLES?Examples.]{[+foo?Foo bar.][+\abar\a?Bar foo.]}[+?More e.g.]{[+aha?AHA]}"
id=xlate catalog=libast text="OPTIONS"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="options"
return=? option=-? name=--?again num=0
id=xlate catalog=libast text="Usage"'
		ERROR - $'Hfntr: xlate [ bcgvbaf ]
BCGVBAF
  -b, --ntnva|onpx|again|back'
		EXIT 2
	EXEC -+ xlate "$usage" --?ntnva
		OUTPUT - $'id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="algorithm]:[method][b:again|back][+EXAMPLES?Examples.]{[+foo?Foo bar.][+\abar\a?Bar foo.]}[+?More e.g.]{[+aha?AHA]}"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="again|back][+EXAMPLES?Examples.]{[+foo?Foo bar.][+\abar\a?Bar foo.]}[+?More e.g.]{[+aha?AHA]}"
id=xlate catalog=libast text="OPTIONS"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="options"
return=? option=-? name=--?ntnva num=0
id=xlate catalog=libast text="Usage"'
		ERROR - $'Hfntr: xlate [ bcgvbaf ]
BCGVBAF
  -b, --ntnva|onpx|again|back'
	EXEC -+ xlate "$usage" --?back
		OUTPUT - $'id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="algorithm]:[method][b:again|back][+EXAMPLES?Examples.]{[+foo?Foo bar.][+\abar\a?Bar foo.]}[+?More e.g.]{[+aha?AHA]}"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="again|back][+EXAMPLES?Examples.]{[+foo?Foo bar.][+\abar\a?Bar foo.]}[+?More e.g.]{[+aha?AHA]}"
id=xlate catalog=libast text="OPTIONS"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="options"
return=? option=-? name=--?back num=0
id=xlate catalog=libast text="Usage"'
	 	ERROR - $'Hfntr: xlate [ bcgvbaf ]
BCGVBAF
  -b, --ntnva|onpx|again|back'
	EXEC -+ xlate "$usage" --?onpx
		OUTPUT - $'id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="algorithm]:[method][b:again|back][+EXAMPLES?Examples.]{[+foo?Foo bar.][+\abar\a?Bar foo.]}[+?More e.g.]{[+aha?AHA]}"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="again|back][+EXAMPLES?Examples.]{[+foo?Foo bar.][+\abar\a?Bar foo.]}[+?More e.g.]{[+aha?AHA]}"
id=xlate catalog=libast text="OPTIONS"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="options"
return=? option=-? name=--?onpx num=0
id=xlate catalog=libast text="Usage"'
		ERROR - $'Hfntr: xlate [ bcgvbaf ]
BCGVBAF
  -b, --ntnva|onpx|again|back'
	EXEC -+ xlate "$usage" --?+EXAMPLES
		OUTPUT - $'id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="algorithm]:[method][b:again|back][+EXAMPLES?Examples.]{[+foo?Foo bar.][+\abar\a?Bar foo.]}[+?More e.g.]{[+aha?AHA]}"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="again|back][+EXAMPLES?Examples.]{[+foo?Foo bar.][+\abar\a?Bar foo.]}[+?More e.g.]{[+aha?AHA]}"
id=xlate catalog=libast text="EXAMPLES"
id=xlate catalog=libast text="EXAMPLES?Examples.]{[+foo?Foo bar.][+\abar\a?Bar foo.]}[+?More e.g.]{[+aha?AHA]}"
id=xlate catalog=libast text="Examples."
id=xlate catalog=libast text="Foo bar."
id=xlate catalog=libast text="\abar\a"
id=xlate catalog=libast text="Bar foo."
id=xlate catalog=libast text="More e.g."
id=xlate catalog=libast text="AHA"
return=? option=-? name=--?+EXAMPLES num=0
id=xlate catalog=libast text="Usage"'
		ERROR - $'  EXAMPLES        Rknzcyrf.
                    foo   Sbb one.
                    one   One sbb.
                  Zber r.t.
                    aha   NUN'
	EXEC -+ xlate "$usage" --?+EX
		OUTPUT - $'id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="algorithm]:[method][b:again|back][+EXAMPLES?Examples.]{[+foo?Foo bar.][+\abar\a?Bar foo.]}[+?More e.g.]{[+aha?AHA]}"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="again|back][+EXAMPLES?Examples.]{[+foo?Foo bar.][+\abar\a?Bar foo.]}[+?More e.g.]{[+aha?AHA]}"
id=xlate catalog=libast text="EXAMPLES"
id=xlate catalog=libast text="EXAMPLES?Examples.]{[+foo?Foo bar.][+\abar\a?Bar foo.]}[+?More e.g.]{[+aha?AHA]}"
id=xlate catalog=libast text="Examples."
id=xlate catalog=libast text="Foo bar."
id=xlate catalog=libast text="\abar\a"
id=xlate catalog=libast text="Bar foo."
id=xlate catalog=libast text="More e.g."
id=xlate catalog=libast text="AHA"
return=? option=-? name=--?+EX num=0
id=xlate catalog=libast text="Usage"'
		ERROR - $'  EXAMPLES        Rknzcyrf.
                    foo   Sbb one.
                    one   One sbb.
                  Zber r.t.
                    aha   NUN'
	EXEC -+ xlate "$usage" --?+RKNZCYRF
		OUTPUT - $'id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="algorithm]:[method][b:again|back][+EXAMPLES?Examples.]{[+foo?Foo bar.][+\abar\a?Bar foo.]}[+?More e.g.]{[+aha?AHA]}"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="again|back][+EXAMPLES?Examples.]{[+foo?Foo bar.][+\abar\a?Bar foo.]}[+?More e.g.]{[+aha?AHA]}"
id=xlate catalog=libast text="EXAMPLES"
id=xlate catalog=libast text="EXAMPLES?Examples.]{[+foo?Foo bar.][+\abar\a?Bar foo.]}[+?More e.g.]{[+aha?AHA]}"
id=xlate catalog=libast text="Examples."
id=xlate catalog=libast text="Foo bar."
id=xlate catalog=libast text="\abar\a"
id=xlate catalog=libast text="Bar foo."
id=xlate catalog=libast text="More e.g."
id=xlate catalog=libast text="AHA"
return=? option=-? name=--?+RKNZCYRF num=0
id=xlate catalog=libast text="Usage"'
		ERROR - $'  EXAMPLES        Rknzcyrf.
                    foo   Sbb one.
                    one   One sbb.
                  Zber r.t.
                    aha   NUN'
	usage=$'[-?\n@(#)xlate 1.0\n][-author?Col. Hyde][a:algorithm?\fone\f]:[method]{[+?\fthree\f]}[b:again|back?\ftwo\f]'
	EXEC -+ xlate "$usage" --keys
		OUTPUT - $'id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="about"
id=xlate catalog=libast text="api"
id=xlate catalog=libast text="help"
id=xlate catalog=libast text="html"
id=xlate catalog=libast text="keys"
id=xlate catalog=libast text="about"
id=xlate catalog=libast text="api"
id=xlate catalog=libast text="help"
id=xlate catalog=libast text="html"
id=xlate catalog=libast text="keys"
return=? option=- name=--keys num=0
id=xlate catalog=libast text="Usage"'
		ERROR - $'"algorithm"
"\\fone\\f"
"method"
"\\fthree\\f"
"again|back"
"\\ftwo\\f"'
		EXIT 2
	EXEC -+ xlate "$usage" --usage
		OUTPUT - $'id=xlate catalog=libast text="algorithm"
id=xlate catalog=libast text="again|back"
id=xlate catalog=libast text="about"
id=xlate catalog=libast text="api"
id=xlate catalog=libast text="help"
id=xlate catalog=libast text="html"
id=xlate catalog=libast text="keys"
id=xlate catalog=libast text="long"
id=xlate catalog=libast text="man"
id=xlate catalog=libast text="nroff"
id=xlate catalog=libast text="options"
id=xlate catalog=libast text="posix"
id=xlate catalog=libast text="short"
id=xlate catalog=libast text="usage"
id=xlate catalog=libast text="about"
id=xlate catalog=libast text="api"
id=xlate catalog=libast text="help"
id=xlate catalog=libast text="html"
id=xlate catalog=libast text="keys"
id=xlate catalog=libast text="long"
id=xlate catalog=libast text="man"
id=xlate catalog=libast text="nroff"
id=xlate catalog=libast text="options"
id=xlate catalog=libast text="posix"
id=xlate catalog=libast text="short"
id=xlate catalog=libast text="usage"
return=? option=- name=--usage num=0
id=xlate catalog=libast text="Usage"'
		ERROR - $'[-?\\n@(#)xlate 1.0\\n][-author?Col. Hyde][a:algorithm?<* one info ok *>]:[method]{[+?<* three info ok *>]}[b:again|back?<* two info ok *>]'

TEST 99 'detailed key strings' # this test must be last
	usage=$'[-?\naha\n][-catalog?SpamCo][Q:quote?Quote names according to \astyle\a:]:[style:=question]{\n\t[c:C?C "..." style.]\t[e:escape?\b\\\b escape if necessary.]\t[A:always?Always shell style.]\t[101:shell?Shell quote if necessary.]\t[q:question|huh?Replace unknown chars with ?.]\n}[x:exec|run?Just do it.]:?[action:=default]'
	EXEC ls "$usage" --man
		EXIT 2
		OUTPUT - 'return=? option=- name=--man num=0'
		ERROR - 'SYNOPSIS
  ls [ options ]

OPTIONS
  -Q, --quote=style
                  Quote names according to style:
                    c|C   C "..." style.
                    escape
                          \ escape if necessary.
                    A|always
                          Always shell style.
                    shell Shell quote if necessary.
                    question|huh
                          Replace unknown chars with ?.
                  The default value is question.
  -x, --exec|run[=action]
                  Just do it. The option value may be omitted. The default
                  value is default.

IMPLEMENTATION
  version         aha
  catalog         SpamCo'
	EXEC ls "$usage" --keys
		OUTPUT - 'return=? option=- name=--keys num=0'
		ERROR - '"catalog"
"quote"
"Quote names according to \astyle\a:"
"style:=question"
"C \"...\" style."
"escape"
"\b\\\b escape if necessary."
"always"
"Always shell style."
"shell"
"Shell quote if necessary."
"question|huh"
"Replace unknown chars with ?."
"exec|run"
"Just do it."
"action:=default"'

	EXPORT	LC_ALL=debug LC_MESSAGES=debug

	IF '[[ $(LC_ALL=debug $COMMAND query "[-][+NAME]" --man 2>&1 >/dev/null) == "(libast,3,325)"* ]]'

	EXEC ls "$usage" --man
		OUTPUT - 'return=? option=- name=--man num=0'
		ERROR - '(libast,3,372)
  ls [ (libast,3,709) ]

(libast,3,333)
  -Q, --(debug,ls,SpamCo,quote)|quote=(debug,ls,SpamCo,style:=question)
                  (debug,ls,SpamCo,Quote names according to style:)
                    (debug,ls,SpamCo,C)
                          (debug,ls,SpamCo,C "..." style.)
                    (debug,ls,SpamCo,escape)
                          (debug,ls,SpamCo,\ escape if necessary.)
                    (debug,ls,SpamCo,always)
                          (debug,ls,SpamCo,Always shell style.)
                    (debug,ls,SpamCo,shell)
                          (debug,ls,SpamCo,Shell quote if necessary.)
                    (debug,ls,SpamCo,question|huh)
                          (debug,ls,SpamCo,Replace unknown chars with ?.)
                  (debug,ls,SpamCo,(libast,3,400) question.)
  -x, --(debug,ls,SpamCo,exec|run)|exec|run[=(debug,ls,SpamCo,action:=default)]
                  (debug,ls,SpamCo,Just do it.) (libast,3,401) (libast,3,400)
                  default.

(libast,3,238)
  (libast,3,812)  (debug,ls,SpamCo,aha )
  (libast,3,499)  (debug,ls,SpamCo,SpamCo)'

	usage=$'[-][+NAME?small]\n\nfile\n\n[+SEE?\btbig\b(1)]'
	EXEC small "$usage" --man
		EXIT 2
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - '(libast,3,325)
  (debug,small,libast,small)

(libast,3,372)
  small [ (libast,3,709) ] (debug,small,libast,file)

(debug,small,libast,SEE)
  (debug,small,libast,tbig(1))'

	usage=$'[-][Y:layout?Listing layout \akey\a:]:[key]{\n[a:across|horizontal?Multi-column across the page.][1:single-column?One column down the page.]\n}'
	EXEC ls "$usage" --man
		ERROR - '(libast,3,372)
  ls [ (libast,3,709) ]

(libast,3,333)
  -Y, --(debug,ls,libast,layout)|layout=(debug,ls,libast,key)
                  (debug,ls,libast,Listing layout key:)
                    (debug,ls,libast,across|horizontal)
                          (debug,ls,libast,Multi-column across the page.)
                    (debug,ls,libast,single-column)
                          (debug,ls,libast,One column down the page.)'
	ELSE '$INSTALLROOT/share/lib/locale/C/LC_MESSAGES/libast not installed'

	EXEC ls "$usage" --man
		OUTPUT - 'return=? option=- name=--man num=0'
		ERROR - '(debug,ls,libast,"SYNOPSIS")
  ls [ (debug,ls,libast,"options") ]

(debug,ls,libast,"OPTIONS")
  -Y, --(debug,ls,libast,"layout")|layout=(debug,ls,libast,"key")
                  (debug,ls,libast,"Listing layout key:")
                    (debug,ls,libast,"across|horizontal")
                          (debug,ls,libast,"Multi-column across the page.")
                    (debug,ls,libast,"single-column")
                          (debug,ls,libast,"One column down the page.")'

	usage=$'[-][+NAME?small]\n\nfile\n\n[+SEE?\btbig\b(1)]'
	EXEC small "$usage" --man
		EXIT 2
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - $'(debug,small,libast,"NAME")
  (debug,small,libast,"small")

(debug,small,libast,"SYNOPSIS")
  small [ (debug,small,libast,"options") ] (debug,small,libast,"file")

(debug,small,libast,"SEE")
  (debug,small,libast,"tbig(1)")'

	usage=$'[-][Y:layout?Listing layout \akey\a:]:[key]{\n[a:across|horizontal?Multi-column across the page.][1:single-column?One column down the page.]\n}'
	EXEC ls "$usage" --man
		ERROR - $'(debug,ls,libast,"SYNOPSIS")
  ls [ (debug,ls,libast,"options") ]

(debug,ls,libast,"OPTIONS")
  -Y, --(debug,ls,libast,"layout")|layout=(debug,ls,libast,"key")
                  (debug,ls,libast,"Listing layout key:")
                    (debug,ls,libast,"across|horizontal")
                          (debug,ls,libast,"Multi-column across the page.")
                    (debug,ls,libast,"single-column")
                          (debug,ls,libast,"One column down the page.")'

	FI
