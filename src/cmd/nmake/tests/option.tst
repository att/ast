# ast nmake option tests

INCLUDE cc.def

TEST 01 'option basics'

	EXEC	-n -f - . 'print -- $(-?:/ /$("\n")/G)'
		OUTPUT - $'--all-static:=1
--archive-output:=\'option\'
--cctype:=cc
--compare:=1
--lib-type:=1
--official-output:=OFFICIAL
--preserve:=\'lib*.so.*|\'
--recurse:=1
--separate-include:=1'

	EXEC	-n -f - . 'print -- $(-+:/ /$("\n")/G)'
		OUTPUT - $'--noexec
--regress=message'

	EXEC	-n -f - . 'print -- $(-:/ /$("\n")/G)'
		OUTPUT - $'--noexec
--regress=message
--native-pp=-1
--noprefix-include
--novirtual'

	EXEC	-n -f - . 'print -- $(--:/ /$("\n")/G)'
		OUTPUT - $'--noaccept
--alias
--nobase
--nobelieve
--nocompatibility
--nocompile
--nocorrupt
--nocross
--nodebug
--noerrorid
--noexec
--noexpandview
--noexplain
--file=-
--noforce
--noglobal
--noignore
--noignorelock
--noinclude
--intermediate
--jobs=1
--nokeepgoing
--nolist
--nomam
--nonever
--nooption
--nooverride
--noquestionable
--noreadonly
--readstate=32
--regress=message
--noreread
--noruledump
--scan
--noserialize
--nosilent
--nostrictview
--notarget-context
--notarget-prefix
--notest
--notolerance
--notouch
--novardump
--nowarn
--writeobject=-
--writestate=-
--nobyname
--nodefine
--nopreprocess
--noundef
--all-static:=1
--noancestor
--noancestor-source
--noarchive-clean
--archive-output:=\'option\'
--cctype:=cc
--noclean-ignore
--noclobber
--compare:=1
--nodebug-symbols
--noforce-shared
--noinstrument
--nold-script
--lib-type:=1
--nolink
--nolocal-static
--native-pp=-1
--official-output:=OFFICIAL
--noprefix-include
--preserve:=\'lib*.so.*|\'
--noprofile
--recurse:=1
--norecurse-enter
--norecurse-leave
--noselect
--separate-include:=1
--noshared
--nostatic-link
--nostrip-symbols
--nothreads
--novariants
--noview-verify
--novirtual
--all-static
--lib-type
--native-pp=-1'

	EXEC	--clobber -n -f - . 'print -- $(-?:/ /$("\n")/G)'
		OUTPUT - $'--all-static:=1
--archive-output:=\'option\'
--cctype:=cc
--compare:=1
--lib-type:=1
--official-output:=OFFICIAL
--preserve:=\'lib*.so.*|\'
--recurse:=1
--separate-include:=1'

	EXEC	--clobber -n -f - . 'print -- $(-+:/ /$("\n")/G)'
		OUTPUT - $'--noexec
--regress=message'

	EXEC	--clobber -n -f - . 'print -- $(-:/ /$("\n")/G)'
		OUTPUT - $'--noexec
--regress=message
--clobber=\'*\'
--native-pp=-1
--noprefix-include
--novirtual'

	EXEC	--clobber -n -f - . 'print -- $(--:/ /$("\n")/G)'
		OUTPUT - $'--noaccept
--alias
--nobase
--nobelieve
--nocompatibility
--nocompile
--nocorrupt
--nocross
--nodebug
--noerrorid
--noexec
--noexpandview
--noexplain
--file=-
--noforce
--noglobal
--noignore
--noignorelock
--noinclude
--intermediate
--jobs=1
--nokeepgoing
--nolist
--nomam
--nonever
--nooption
--nooverride
--noquestionable
--noreadonly
--readstate=32
--regress=message
--noreread
--noruledump
--scan
--noserialize
--nosilent
--nostrictview
--notarget-context
--notarget-prefix
--notest
--notolerance
--notouch
--novardump
--nowarn
--writeobject=-
--writestate=-
--nobyname
--nodefine
--nopreprocess
--noundef
--all-static:=1
--noancestor
--noancestor-source
--noarchive-clean
--archive-output:=\'option\'
--cctype:=cc
--noclean-ignore
--clobber=\'*\'
--compare:=1
--nodebug-symbols
--noforce-shared
--noinstrument
--nold-script
--lib-type:=1
--nolink
--nolocal-static
--native-pp=-1
--official-output:=OFFICIAL
--noprefix-include
--preserve:=\'lib*.so.*|\'
--noprofile
--recurse:=1
--norecurse-enter
--norecurse-leave
--noselect
--separate-include:=1
--noshared
--nostatic-link
--nostrip-symbols
--nothreads
--novariants
--noview-verify
--novirtual
--all-static
--lib-type
--native-pp=-1'

	EXEC	-n -f - -A --writeobject=test.mo --nowritestate . 'print -- $(--:/ /$("\n")/G)'
		OUTPUT - $'--accept
--alias
--nobase
--nobelieve
--nocompatibility
--nocompile
--nocorrupt
--nocross
--nodebug
--noerrorid
--noexec
--noexpandview
--noexplain
--file=-
--noforce
--noglobal
--noignore
--noignorelock
--noinclude
--intermediate
--jobs=1
--nokeepgoing
--nolist
--nomam
--nonever
--nooption
--nooverride
--noquestionable
--noreadonly
--readstate=32
--regress=message
--noreread
--noruledump
--scan
--noserialize
--nosilent
--nostrictview
--notarget-context
--notarget-prefix
--notest
--notolerance
--notouch
--novardump
--nowarn
--writeobject=test.mo
--nowritestate
--nobyname
--nodefine
--nopreprocess
--noundef
--all-static:=1
--noancestor
--noancestor-source
--noarchive-clean
--archive-output:=\'option\'
--cctype:=cc
--noclean-ignore
--noclobber
--compare:=1
--nodebug-symbols
--noforce-shared
--noinstrument
--nold-script
--lib-type:=1
--nolink
--nolocal-static
--native-pp=-1
--official-output:=OFFICIAL
--noprefix-include
--preserve:=\'lib*.so.*|\'
--noprofile
--recurse:=1
--norecurse-enter
--norecurse-leave
--noselect
--separate-include:=1
--noshared
--nostatic-link
--nostrip-symbols
--nothreads
--novariants
--noview-verify
--novirtual
--all-static
--lib-type
--native-pp=-1'

	EXEC	-n -f - . 'print -- $(-force)'
		OUTPUT - $''

	EXEC	-n -f - +F . 'print -- $(-force)'

	EXEC	-n -f - --noforce . 'print -- $(-force)'

	EXEC	-n -f - --force=0 . 'print -- $(-force)'

	EXEC	-n -f - --force . 'print -- $(-force)'
		OUTPUT - $'1'

	EXEC	-n -f - --force=1 . 'print -- $(-force)'

	EXEC	-n -f - -F . 'print -- $(-force)'

	EXEC	-n -f - . 'print -- $(--force)'
		OUTPUT - $'--noforce'

	EXEC	-n -f - +F . 'print -- $(--force)'

	EXEC	-n -f - --noforce . 'print -- $(--force)'

	EXEC	-n -f - --force=0 . 'print -- $(--force)'

	EXEC	-n -f - -o noforce . 'print -- $(--force)'

	EXEC	-n -f - -o force=0 . 'print -- $(--force)'

	EXEC	-n -f - -o +F . 'print -- $(--force)'

	EXEC	-n -f - --force . 'print -- $(--force)'
		OUTPUT - $'--force'

	EXEC	-n -f - --force=1 . 'print -- $(--force)'

	EXEC	-n -f - -F . 'print -- $(--force)'

	EXEC	-n -f - -o force . 'print -- $(--force)'

	EXEC	-n -f - -o force=1 . 'print -- $(--force)'

	EXEC	-n -f - -o -F . 'print -- $(--force)'

TEST 02 'undefined options'

	EXEC	--silent -f - --foobar
		ERROR - $'make: foobar: unknown option'
		EXIT 2

	EXEC	--silent -f - -o --foobar

	EXEC	--silent -f - -o foobar

	EXEC	--silent -f - ++foobar
		ERROR - $'make: ++foobar: section not found'

	EXEC	--silent -f - -Y
		ERROR - $'make: -Y: unknown option'

	EXEC	--silent -f - -o -Y

	EXEC	--silent -f - +Y
		ERROR - $'make: +Y: unknown option'

	EXEC	--silent -f - -o +Y

TEST 03 'user defined options'

	EXEC	--silent
		INPUT Makefile $'.set.unconditional : .FUNCTION
	set $(%:N=1:??no?)force
set option=u;unconditional;b;.set.unconditional;\'Equivalent to \\b--force\\b.\'
set unconditional
all : .MAKE
	print force=$(-force) unconditional=$(-unconditional) -F=$(-F) -u=$(-u)
	print : $(-) :
	print : $(--) :'
		OUTPUT - $'force=1 unconditional=1 -F=1 -u=1
: --force --regress=message --silent --native-pp=-1 --noprefix-include --novirtual --unconditional :
: --noaccept --alias --nobase --nobelieve --nocompatibility --nocompile --nocorrupt --nocross --debug=-2 --noerrorid --exec --noexpandview --noexplain --nofile --force --noglobal --noignore --noignorelock --noinclude --intermediate --jobs=1 --nokeepgoing --nolist --nomam --nonever --nooption --nooverride --noquestionable --noreadonly --readstate=32 --regress=message --noreread --noruledump --scan --noserialize --silent --nostrictview --notarget-context --notarget-prefix --notest --notolerance --notouch --novardump --nowarn --writeobject=- --writestate=- --nobyname --nodefine --nopreprocess --noundef --all-static:=1 --noancestor --noancestor-source --noarchive-clean --archive-output:=\'option\' --cctype:=cc --noclean-ignore --noclobber --compare:=1 --nodebug-symbols --noforce-shared --noinstrument --nold-script --lib-type:=1 --nolink --nolocal-static --native-pp=-1 --official-output:=OFFICIAL --noprefix-include --preserve:=\'lib*.so.*|\' --noprofile --recurse:=1 --norecurse-enter --norecurse-leave --noselect --separate-include:=1 --noshared --nostatic-link --nostrip-symbols --nothreads --novariants --noview-verify --novirtual --unconditional --all-static --lib-type --native-pp=-1 :'

	EXEC --silent
		OUTPUT - $'force=1 unconditional=1 -F=1 -u=1
: --force --regress=message --silent --native-pp=-1 --noprefix-include --novirtual --unconditional :
: --noaccept --alias --nobase --nobelieve --nocompatibility --nocompile --nocorrupt --nocross --debug=-2 --noerrorid --exec --noexpandview --noexplain --nofile --force --noglobal --noignore --noignorelock --noinclude --intermediate --jobs=1 --nokeepgoing --nolist --nomam --nonever --nooption --nooverride --noquestionable --noreadonly --readstate=32 --regress=message --noreread --noruledump --scan --noserialize --silent --nostrictview --notarget-context --notarget-prefix --notest --notolerance --notouch --novardump --nowarn --writeobject=- --writestate=- --nobyname --nodefine --nopreprocess --noundef --all-static:=1 --noancestor --noancestor-source --noarchive-clean --archive-output:=\'option\' --cctype:=cc --noclean-ignore --noclobber --compare:=1 --nodebug-symbols --noforce-shared --noinstrument --nold-script --lib-type:=1 --nolink --nolocal-static --native-pp=-1 --official-output:=OFFICIAL --noprefix-include --preserve:=\'lib*.so.*|\' --noprofile --recurse:=1 --norecurse-enter --norecurse-leave --noselect --separate-include:=1 --noshared --nostatic-link --nostrip-symbols --nothreads --novariants --noview-verify --novirtual --unconditional --all-static --lib-type --native-pp=-1 :'

	EXEC --silent clobber
		OUTPUT -

	EXEC --silent -u
		INPUT Makefile $'.set.unconditional : .FUNCTION
	set $(%:N=1:??no?)force
set option=u;unconditional;b;.set.unconditional;\'Equivalent to \\b--force\\b.;\'
set unconditional
all : .MAKE
	print force=$(-force) unconditional=$(-unconditional) -F=$(-F) -u=$(-u) : $(-) :'
		OUTPUT - $'force=1 unconditional=1 -F=1 -u=1 : --force --regress=message --silent --native-pp=-1 --noprefix-include --novirtual --unconditional :'

	EXEC --silent -u

	EXEC --silent

	EXEC --silent +u
		OUTPUT - $'force= unconditional= -F= -u= : --noforce --regress=message --silent --native-pp=-1 --noprefix-include --novirtual --nounconditional :'

	EXEC --silent
		OUTPUT - $'force=1 unconditional=1 -F=1 -u=1 : --force --regress=message --silent --native-pp=-1 --noprefix-include --novirtual --unconditional :'

	EXEC --silent clobber
		OUTPUT -

	EXEC --silent +u
		OUTPUT - $'force= unconditional= -F= -u= : --noforce --regress=message --silent --native-pp=-1 --noprefix-include --novirtual --nounconditional :'

	EXEC --silent +u

	EXEC --silent

	EXEC --silent -u
		OUTPUT - $'force=1 unconditional=1 -F=1 -u=1 : --force --regress=message --silent --native-pp=-1 --noprefix-include --novirtual --unconditional :'

	EXEC --silent
		OUTPUT - $'force= unconditional= -F= -u= : --noforce --regress=message --silent --native-pp=-1 --noprefix-include --novirtual --nounconditional :'

	EXEC --silent clobber
		OUTPUT -

	EXEC --silent -o unconditional
		OUTPUT - $'force=1 unconditional=1 -F=1 -u=1 : --force --regress=message --silent --native-pp=-1 --noprefix-include --novirtual --unconditional :'

	EXEC --silent

	EXEC --silent -o nounconditional
		OUTPUT - $'force= unconditional= -F= -u= : --noforce --regress=message --silent --native-pp=-1 --noprefix-include --novirtual --nounconditional :'

	EXEC --silent
		OUTPUT - $'force=1 unconditional=1 -F=1 -u=1 : --force --regress=message --silent --native-pp=-1 --noprefix-include --novirtual --unconditional :'

	EXEC --silent clobber
		OUTPUT -

	EXEC --silent --unconditional
		OUTPUT - $'force=1 unconditional=1 -F=1 -u=1 : --force --regress=message --silent --native-pp=-1 --noprefix-include --novirtual --unconditional :'

	EXEC --silent

	EXEC --silent --nounconditional
		OUTPUT - $'force= unconditional= -F= -u= : --noforce --regress=message --silent --native-pp=-1 --noprefix-include --novirtual --nounconditional :'

	EXEC --silent
		OUTPUT - $'force=1 unconditional=1 -F=1 -u=1 : --force --regress=message --silent --native-pp=-1 --noprefix-include --novirtual --unconditional :'

TEST 04 'command line definition'

	EXEC	-n --option='Z;Ztest;b;-;Z test.' --Ztest
		INPUT Makefile $'tst :
	: $(-) : $(-Z) : $(-Ztest) :'
		OUTPUT - $'+ : --noexec --regress=message --Ztest --native-pp=-1 --noprefix-include --novirtual : 1 : 1 :'

	EXEC	-n --option='Z;Ztest;b;-;Z test.' -o Ztest

	EXEC	-n --option='Z;Ztest;b;-;Z test.' -o --Ztest

	EXEC	-n --option='Z;Ztest;b;-;Z test.' -o -Ztest

	EXEC	-n --option='Z;Ztest;b;-;Z test.' -Z

	EXEC	-n --option='Z;Ztest;b;-;Z test.' -o -Z

	EXEC	-n --option='Z;Ztest;b;-;Z test.' -o Z

	EXEC	-n --option='Z;Ztest;b;-;Z test.' --noZtest
		OUTPUT - $'+ : --noexec --regress=message --noZtest --native-pp=-1 --noprefix-include --novirtual :  :  :'

	EXEC	-n --option='Z;Ztest;b;-;Z test.' -o noZtest

	EXEC	-n --option='Z;Ztest;b;-;Z test.' -o --noZtest

	EXEC	-n --option='Z;Ztest;b;-;Z test.' -o -noZtest
		OUTPUT - $'+ : --noexec --regress=message --native-pp=-1 --noprefix-include --novirtual :  :  :'

	EXEC	-n --option='Z;Ztest;b;-;Z test.' +Z
		OUTPUT - $'+ : --noexec --regress=message --noZtest --native-pp=-1 --noprefix-include --novirtual :  :  :'

	EXEC	-n --option='Z;Ztest;b;-;Z test.' -o +Z

	EXEC	-n --option='Z;Ztest;b;-;Z test.' -o Z
		OUTPUT - $'+ : --noexec --regress=message --Ztest --native-pp=-1 --noprefix-include --novirtual : 1 : 1 :'

TEST 05 '.mo interaction'

	EXEC	-n
		INPUT Makefile $'all : status
status : .MAKE .VIRTUAL .FORCE .REPEAT
	print : $(-) : keepgoing=$(-keepgoing)'
		OUTPUT - $': --noexec --regress=message --native-pp=-1 --noprefix-include --novirtual : keepgoing='

	EXEC	-n -k
		OUTPUT - $': --noexec --keepgoing --regress=message --native-pp=-1 --noprefix-include --novirtual : keepgoing=1'

	EXEC	-k
		OUTPUT - $': --keepgoing --regress=message --native-pp=-1 --noprefix-include --novirtual : keepgoing=1'

	EXEC	--
		OUTPUT - $': --regress=message --native-pp=-1 --noprefix-include --novirtual : keepgoing='

TEST 06 'jobs'

	EXEC	--silent --jobs=4
		INPUT Makefile $'SLEEP = sleep
LONG =
test : a b c d
	echo $(<) done
a :
	$(SLEEP) 4$(LONG)
	echo $(<) done
b :
	$(SLEEP) 3$(LONG)
	echo $(<) done
c :
	$(SLEEP) 2$(LONG)
	echo $(<) done
d :
	$(SLEEP) 1$(LONG)
	echo $(<) done'
		OUTPUT - $'d done
c done
b done
a done
test done'

TEST 07 'old jobs deadlock' && {

	mkdir -p root && cd root || FATAL cannot cd root

	EXEC	--jobs=4
		INPUT Makefile $'
a : d
	: making $(<)
	sleep 5
	echo $(<) : *
	touch $(<)
b : c a
	: making $(<)
	sleep 1
	echo $(<) : *
	touch $(<)
c : c.x
	: making $(<)
	sleep 1
	echo $(<) : *
	touch $(<)
d : b d.x
	: making $(<)
	sleep 5
	echo $(<) : *
	touch $(<)'
		INPUT c.x
		INPUT d.x
		OUTPUT - $'c : Makefile Makefile.ml Makefile.mo c.x d.x
a : Makefile Makefile.ml Makefile.mo c c.x d.x
b : Makefile Makefile.ml Makefile.mo a c c.x d.x
d : Makefile Makefile.ml Makefile.mo a b c c.x d.x'
		ERROR - $'+ : making c
+ sleep 1
+ echo c : Makefile Makefile.ml Makefile.mo c.x d.x
+ touch c
+ : making a
+ sleep 5
+ echo a : Makefile Makefile.ml Makefile.mo c c.x d.x
+ touch a
+ : making b
+ sleep 1
+ echo b : Makefile Makefile.ml Makefile.mo a c c.x d.x
+ touch b
+ : making d
+ sleep 5
+ echo d : Makefile Makefile.ml Makefile.mo a b c c.x d.x
+ touch d'

}

TEST 08 'concurrency vs. errors'

	EXEC	--jobs=1 --silent
		INPUT Makefile $'all : 1 2 3 4
1 3 :
	sleep $(<); echo $(<) >&2; false
2 4 :
	sleep $(<); echo $(<) >&2'
		ERROR - $'1\nmake: *** exit code 1 making 1'
		EXIT 1

	EXEC	--jobs=2 --silent
		ERROR - $'1\nmake: *** exit code 1 making 1\n2'

	EXEC	--jobs=3 --silent
		ERROR - $'1\nmake: *** exit code 1 making 1\n2\n3\nmake: *** exit code 1 making 3'

	EXEC	--jobs=4 --silent
		ERROR - $'1\nmake: *** exit code 1 making 1\n2\n3\nmake: *** exit code 1 making 3\n4'

TEST 09 '--writeobject and --writestate with readonly pwd'

	DO	{ mkdir ro wr || FATAL cannot initialize directories ;}

	CD	ro

	EXEC	--silent
		INPUT Makefile $'all :\n\tls ../wr'
		OUTPUT -

	DO	chmod ugo-w .

	EXEC	--silent --nowriteobject --nowritestate

	EXEC	--silent --writeobject=../wr --nowritestate
		OUTPUT - $'Makefile.mo'

	EXEC	--silent --writeobject=../wr --writestate=../wr
		OUTPUT - $'Makefile.ml\nMakefile.mo'

	EXEC	--silent --writeobject=../wr/Testfile.mo --nowritestate
		OUTPUT - $'Makefile.mo\nMakefile.ms\nTestfile.mo'

	EXEC	--silent --writeobject=../wr/Testfile.mo --writestate=../wr/Testfile.ms
		OUTPUT - $'Makefile.mo\nMakefile.ms\nTestfile.ml\nTestfile.mo'

	EXEC	--silent --writeobject=../wr/Testfile.mo --writestate=../wr/Testfile.ms
		OUTPUT - $'Makefile.mo\nMakefile.ms\nTestfile.ml\nTestfile.mo\nTestfile.ms'

TEST 10 'self documenting external options'

	EXEC	-bcf base.mk
		INPUT base.mk $'set option=";aha-base;b;-;Test base rules option;; superfluous.;"'

	EXEC	'--?aha' -g global.mk MAKERULES=base
		INPUT global.mk $'set option=";aha-global;n;-;Test global rules option.;estimate:=true"'
		INPUT Makefile $'set option=";aha-user;sv;-;Test makefile option.;pattern:!*:=*.exe"
set option=";aha-two"
set option=";aha-three;s"
set option=";aha-four;s;-;Blah;; blah."
set option=";aha-five;s;-;Blah;; blah.;path"
set option=";aha-six;s;-;Blah;; blah.;path;"
set option=";aha-seven;n"
set option=";aha-eight;n;-;Blah;; blah."
set option=";aha-nine;n;-;Blah;; blah.;level"
set option=";aha-ten;n;-;Blah;; blah.;level;"
set option=";aha-var-three;sv"
set option=";aha-var-four;sv;-;Blah;; blah."
set option=";aha-var-five;sv;-;Blah;; blah.;path"
set option=";aha-var-six;sv;-;Blah;; blah.;path;"
set option=";aha-var-seven;nv"
set option=";aha-var-eight;nv;-;Blah;; blah."
set option=";aha-var-nine;nv;-;Blah;; blah.;level"
set option=";aha-var-ten;nv;-;Blah;; blah.;level"'
		EXIT 2
		ERROR - $'make: [ options ] [ script ... ] [ target ... ]
OPTIONS
  --aha-base      (base) Test base rules option; superfluous.
  --aha-global=estimate
                  (global) Test global rules option. The default value is true.
  --aha-user[=pattern]
                  (Makefile) Test makefile option. If the option value is
                  omitted then * is assumed. The default value is *.exe.
  --aha-two       (Makefile) option.
  --aha-three=string
                  (Makefile) option.
  --aha-four=string
                  (Makefile) Blah; blah.
  --aha-five=path (Makefile) Blah; blah.
  --aha-six=path  (Makefile) Blah; blah.
  --aha-seven=number
                  (Makefile) option.
  --aha-eight=number
                  (Makefile) Blah; blah.
  --aha-nine=level
                  (Makefile) Blah; blah.
  --aha-ten=level (Makefile) Blah; blah.
  --aha-var-three[=string]
                  (Makefile) option. The option value may be omitted.
  --aha-var-four[=string]
                  (Makefile) Blah; blah. The option value may be omitted.
  --aha-var-five[=path]
                  (Makefile) Blah; blah. The option value may be omitted.
  --aha-var-six[=path]
                  (Makefile) Blah; blah. The option value may be omitted.
  --aha-var-seven[=number]
                  (Makefile) option. The option value may be omitted.
  --aha-var-eight[=number]
                  (Makefile) Blah; blah. The option value may be omitted.
  --aha-var-nine[=level]
                  (Makefile) Blah; blah. The option value may be omitted.
  --aha-var-ten[=level]
                  (Makefile) Blah; blah. The option value may be omitted.'

	EXEC	'--?aha' -g global.mk MAKERULES=base
		INPUT Makefile $'set option=";aha-user;sv;-;Test makefile option.;pattern:!*:=*.exe"
set option="+aha-two"
set option="+aha-three+s"
set option="+aha-four+s+-+Blah; blah."
set option="+aha-five+s+-+Blah; blah.+path"
set option="+aha-six+s+-+Blah; blah.+path+"
set option="+aha-seven+n"
set option="+aha-eight+n+-+Blah; blah."
set option="+aha-nine+n+-+Blah; blah.+level"
set option="+aha-ten+n+-+Blah; blah.+level+"
set option="+aha-var-three+sv"
set option="+aha-var-four+sv+-+Blah; blah."
set option="+aha-var-five+sv+-+Blah; blah.+path"
set option="+aha-var-six+sv+-+Blah; blah.+path+"
set option="+aha-var-seven+nv"
set option="+aha-var-eight+nv+-+Blah; blah."
set option="+aha-var-nine+nv+-+Blah; blah.+level"
set option="+aha-var-ten+nv+-+Blah; blah.+level"'

TEST 11 'rules obsolete "name=value" compatibility'

	EXEC	-n
		INPUT Makefile $'all :\n\t: $(-clobber) : $(clobber) :'
		OUTPUT - $'+ :  :  :'

	EXEC	-n --noclobber

	EXEC	-n --clobber=0

	EXEC	-n --noclobber clobber=1
		OUTPUT - $'+ :  : 1 :'

	EXEC	-n clobber=1 --noclobber

	EXEC	-n clobber=0
		OUTPUT - $'+ :  : 0 :'
		ERROR - $'make: warning: clobber=0: obsolete: use --noclobber'

	EXEC	-n --clobber
		OUTPUT - $'+ : * :  :'
		ERROR -

	EXEC	-n --clobber=1

	EXEC	-n --clobber clobber=0
		OUTPUT - $'+ : * : 0 :'

	EXEC	-n clobber=0 --clobber

	EXPORT	MAKE_OPTIONS="clobber $MAKE_OPTIONS"

	EXEC	-n
		OUTPUT - $'+ : * :  :'

	EXEC	-n --clobber=2
		OUTPUT - $'+ : 2 :  :'

	EXEC	-n --noclobber
		OUTPUT - $'+ :  :  :'

	EXEC	-n --noclobber clobber=1
		OUTPUT - $'+ :  : 1 :'

	EXEC	-n clobber=1 --noclobber

	EXEC	-n clobber=0
		OUTPUT - $'+ : * : 0 :'

	EXEC	-n --clobber
		OUTPUT - $'+ : * :  :'
		ERROR -

	EXEC	-n --clobber=1

	EXEC	-n --clobber clobber=0
		OUTPUT - $'+ : * : 0 :'

	EXEC	-n clobber=0 --clobber

	EXEC	-n
		INPUT Makefile $'clobber = 0
all :\n\t: $(-clobber) : $(clobber) :'
		OUTPUT - $'+ : * : 0 :'

	EXEC	-n
		INPUT Makefile $'set noclobber
all :\n\t: $(-clobber) : $(clobber) :'
		OUTPUT - $'+ :  :  :'
		ERROR -

	EXEC	-n
		INPUT Makefile $'clobber = 1
all :\n\t: $(-clobber) : $(clobber) :'
		OUTPUT - $'+ : * : 1 :'

	EXEC	-n
		INPUT Makefile $'set clobber
all :\n\t: $(-clobber) : $(clobber) :'
		OUTPUT - $'+ : * :  :'
		ERROR -

TEST 12 '--serialize'

	EXEC	--jobs=2 --serialize
		INPUT Makefile $'all : one two
one :
	echo begin $(<)
	sleep 1
	echo end $(<)
two :
	echo begin $(<)
	sleep 3
	echo end $(<)'
		OUTPUT - $'begin one
end one
begin two
end two'
		ERROR - $'+ echo begin one
+ sleep 1
+ echo end one
+ echo begin two
+ sleep 3
+ echo end two'

TEST 13 '--name[-+&|^]=value'

	EXEC	-n
		INPUT Makefile $'all : tst
tst : .MAKE
	print $(-test:F=0x%08x)
	set test=0x1
	print $(-test:F=0x%08x)
	set test=0
	print $(-test:F=0x%08x)
	set test|=0x100
	print $(-test:F=0x%08x)
	set test|=0x011
	print $(-test:F=0x%08x)
	set test&=0x101
	print $(-test:F=0x%08x)
	set test^=0x111
	print $(-test:F=0x%08x)
	set test+=0x030
	print $(-test:F=0x%08x)
	set test-=0x010
	print $(-test:F=0x%08x)'
		OUTPUT - $'0x00000000\n0x00000001\n0x00000000\n0x00000100\n0x00000111\n0x00000101\n0x00000010\n0x00000040\n0x00000030'

TEST 14 '$(-option) variations'

	EXEC	-n --noforce --nojobs
		INPUT Makefile $'all : tst
tst :
	:$(-F):$(-force):$(-noforce):$(-j):$(-jobs):$(-nojobs):
	:$(--F):$(--force):$(--noforce):$(--j):$(--jobs):$(--nojobs):'
		OUTPUT - $'+ :::1:::1:
+ :--noforce:--noforce:--noforce:--nojobs:--nojobs:--nojobs:'

	EXEC	-n --force --jobs=4
		OUTPUT - $'+ :1:1::4:4::
+ :--force:--force:--force:--jobs=4:--jobs=4:--jobs=4:'
