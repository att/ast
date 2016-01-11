# : : generated from /home/gsf/src/cmd/builtin/realpath.rt by mktest : : #

# regression tests for the realpath command

UNIT realpath

function init
{
	typeset dir="dd dd/g dd/eee dd/h d z z/g z/eee z/h"
	typeset reg="f dd/g/g dd/eee/g dd/h/g z/g/g z/eee/g z/h/g"
	typeset lnk="eee dd/g h dd/eee b a c b f d/e dd d/dd dd z d/e c"
	typeset top=$1
	typeset f

	mkdir $top
	for f in $dir
	do	mkdir -p "$top/$f"
	done
	for f in $reg
	do	: > $top/$f
	done
	set $lnk
	while	(( $# >= 2 ))
	do	ln -s $1 $top/$2
		shift 2
	done
}


TEST 01 'kitchen sink'

	DO init t

	EXEC	t/dd
		INPUT -n -
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd
		ERROR -n -

	EXEC	t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d

	EXEC	t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z

	EXEC	t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z/g

	EXEC	t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/f

	EXEC	t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/eee

	EXEC	t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/f

	EXEC	t/b

	EXEC	t/d/e

	EXEC	t/d/dd
		OUTPUT -
		ERROR - 'realpath: t/d/dd: canonicalization error [Too many levels of symbolic links]'
		EXIT 1

	EXEC	t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/f
		ERROR -n -
		EXIT 0

	EXEC	t/xxx
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/xxx

	EXEC	--quiet t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd

	EXEC	--quiet t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d

	EXEC	--quiet t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z

	EXEC	--quiet t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--quiet t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z/g

	EXEC	--quiet t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/f

	EXEC	--quiet t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--quiet t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/eee

	EXEC	--quiet t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/f

	EXEC	--quiet t/b

	EXEC	--quiet t/d/e

	EXEC	--quiet t/d/dd
		OUTPUT -
		EXIT 1

	EXEC	--quiet t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/f
		EXIT 0

	EXEC	--quiet t/xxx
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/xxx

	EXEC	--readlink t/dd
		OUTPUT -
		ERROR - 'realpath: t/dd: cannot read link text [Invalid argument]'
		EXIT 1

	EXEC	--readlink t/d
		ERROR - 'realpath: t/d: cannot read link text [Invalid argument]'

	EXEC	--readlink t/z
		ERROR - 'realpath: t/z: cannot read link text [Invalid argument]'

	EXEC	--readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--readlink t/z/g
		ERROR - 'realpath: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--readlink t/f
		ERROR - 'realpath: t/f: cannot read link text [Invalid argument]'

	EXEC	--readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--readlink t/dd/eee
		ERROR - 'realpath: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--readlink t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--readlink t/b
		OUTPUT - c

	EXEC	--readlink t/d/e
		OUTPUT - f

	EXEC	--readlink t/d/dd
		OUTPUT - dd

	EXEC	--readlink t/c
		OUTPUT - d/e

	EXEC	--readlink t/xxx
		OUTPUT -
		ERROR - 'realpath: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--readlink --quiet t/dd
		ERROR -n -

	EXEC	--readlink --quiet t/d

	EXEC	--readlink --quiet t/z

	EXEC	--readlink --quiet t/dd/g

	EXEC	--readlink --quiet t/z/g

	EXEC	--readlink --quiet t/f

	EXEC	--readlink --quiet t/dd/g

	EXEC	--readlink --quiet t/dd/eee

	EXEC	--readlink --quiet t/a
		OUTPUT - b
		EXIT 0

	EXEC	--readlink --quiet t/b
		OUTPUT - c

	EXEC	--readlink --quiet t/d/e
		OUTPUT - f

	EXEC	--readlink --quiet t/d/dd
		OUTPUT - dd

	EXEC	--readlink --quiet t/c
		OUTPUT - d/e

	EXEC	--readlink --quiet t/xxx
		OUTPUT -
		EXIT 1

	EXEC	--relative t/dd
		OUTPUT - t/dd
		EXIT 0

	EXEC	--relative t/d
		OUTPUT - t/d

	EXEC	--relative t/z
		OUTPUT - t/z

	EXEC	--relative t/dd/g
		OUTPUT - t/dd/g

	EXEC	--relative t/z/g
		OUTPUT - t/z/g

	EXEC	--relative t/f
		OUTPUT - t/f

	EXEC	--relative t/dd/g
		OUTPUT - t/dd/g

	EXEC	--relative t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--relative t/a
		OUTPUT - t/d/f

	EXEC	--relative t/b

	EXEC	--relative t/d/e

	EXEC	--relative t/d/dd
		OUTPUT -
		ERROR - 'realpath: t/d/dd: canonicalization error [Too many levels of symbolic links]'
		EXIT 1

	EXEC	--relative t/c
		OUTPUT - t/d/f
		ERROR -n -
		EXIT 0

	EXEC	--relative t/xxx
		OUTPUT - t/xxx

	EXEC	--relative --quiet t/dd
		OUTPUT - t/dd

	EXEC	--relative --quiet t/d
		OUTPUT - t/d

	EXEC	--relative --quiet t/z
		OUTPUT - t/z

	EXEC	--relative --quiet t/dd/g
		OUTPUT - t/dd/g

	EXEC	--relative --quiet t/z/g
		OUTPUT - t/z/g

	EXEC	--relative --quiet t/f
		OUTPUT - t/f

	EXEC	--relative --quiet t/dd/g
		OUTPUT - t/dd/g

	EXEC	--relative --quiet t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--relative --quiet t/a
		OUTPUT - t/d/f

	EXEC	--relative --quiet t/b

	EXEC	--relative --quiet t/d/e

	EXEC	--relative --quiet t/d/dd
		OUTPUT -
		EXIT 1

	EXEC	--relative --quiet t/c
		OUTPUT - t/d/f
		EXIT 0

	EXEC	--relative --quiet t/xxx
		OUTPUT - t/xxx

	EXEC	--relative --readlink t/dd
		OUTPUT -
		ERROR - 'realpath: t/dd: cannot read link text [Invalid argument]'
		EXIT 1

	EXEC	--relative --readlink t/d
		ERROR - 'realpath: t/d: cannot read link text [Invalid argument]'

	EXEC	--relative --readlink t/z
		ERROR - 'realpath: t/z: cannot read link text [Invalid argument]'

	EXEC	--relative --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--relative --readlink t/z/g
		ERROR - 'realpath: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--relative --readlink t/f
		ERROR - 'realpath: t/f: cannot read link text [Invalid argument]'

	EXEC	--relative --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--relative --readlink t/dd/eee
		ERROR - 'realpath: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--relative --readlink t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--relative --readlink t/b
		OUTPUT - c

	EXEC	--relative --readlink t/d/e
		OUTPUT - f

	EXEC	--relative --readlink t/d/dd
		OUTPUT - dd

	EXEC	--relative --readlink t/c
		OUTPUT - d/e

	EXEC	--relative --readlink t/xxx
		OUTPUT -
		ERROR - 'realpath: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--relative --readlink --quiet t/dd
		ERROR -n -

	EXEC	--relative --readlink --quiet t/d

	EXEC	--relative --readlink --quiet t/z

	EXEC	--relative --readlink --quiet t/dd/g

	EXEC	--relative --readlink --quiet t/z/g

	EXEC	--relative --readlink --quiet t/f

	EXEC	--relative --readlink --quiet t/dd/g

	EXEC	--relative --readlink --quiet t/dd/eee

	EXEC	--relative --readlink --quiet t/a
		OUTPUT - b
		EXIT 0

	EXEC	--relative --readlink --quiet t/b
		OUTPUT - c

	EXEC	--relative --readlink --quiet t/d/e
		OUTPUT - f

	EXEC	--relative --readlink --quiet t/d/dd
		OUTPUT - dd

	EXEC	--relative --readlink --quiet t/c
		OUTPUT - d/e

	EXEC	--relative --readlink --quiet t/xxx
		OUTPUT -
		EXIT 1

	EXEC	--logical t/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd
		EXIT 0

	EXEC	--logical t/d
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d

	EXEC	--logical t/z
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z

	EXEC	--logical t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--logical t/z/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z/g

	EXEC	--logical t/f
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/f

	EXEC	--logical t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--logical t/dd/eee
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/eee

	EXEC	--logical t/a
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/a

	EXEC	--logical t/b
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/b

	EXEC	--logical t/d/e
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/e

	EXEC	--logical t/d/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/dd

	EXEC	--logical t/c
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/c

	EXEC	--logical t/xxx
		OUTPUT -
		ERROR - 'realpath: t/xxx: canonicalization error [No such file or directory]'
		EXIT 1

	EXEC	--logical --quiet t/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd
		ERROR -n -
		EXIT 0

	EXEC	--logical --quiet t/d
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d

	EXEC	--logical --quiet t/z
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z

	EXEC	--logical --quiet t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--logical --quiet t/z/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z/g

	EXEC	--logical --quiet t/f
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/f

	EXEC	--logical --quiet t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--logical --quiet t/dd/eee
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/eee

	EXEC	--logical --quiet t/a
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/a

	EXEC	--logical --quiet t/b
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/b

	EXEC	--logical --quiet t/d/e
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/e

	EXEC	--logical --quiet t/d/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/dd

	EXEC	--logical --quiet t/c
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/c

	EXEC	--logical --quiet t/xxx
		OUTPUT -
		EXIT 1

	EXEC	--logical --readlink t/dd
		ERROR - 'realpath: t/dd: cannot read link text [Invalid argument]'

	EXEC	--logical --readlink t/d
		ERROR - 'realpath: t/d: cannot read link text [Invalid argument]'

	EXEC	--logical --readlink t/z
		ERROR - 'realpath: t/z: cannot read link text [Invalid argument]'

	EXEC	--logical --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--logical --readlink t/z/g
		ERROR - 'realpath: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--logical --readlink t/f
		ERROR - 'realpath: t/f: cannot read link text [Invalid argument]'

	EXEC	--logical --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--logical --readlink t/dd/eee
		ERROR - 'realpath: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--logical --readlink t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--logical --readlink t/b
		OUTPUT - c

	EXEC	--logical --readlink t/d/e
		OUTPUT - f

	EXEC	--logical --readlink t/d/dd
		OUTPUT - dd

	EXEC	--logical --readlink t/c
		OUTPUT - d/e

	EXEC	--logical --readlink t/xxx
		OUTPUT -
		ERROR - 'realpath: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--logical --readlink --quiet t/dd
		ERROR -n -

	EXEC	--logical --readlink --quiet t/d

	EXEC	--logical --readlink --quiet t/z

	EXEC	--logical --readlink --quiet t/dd/g

	EXEC	--logical --readlink --quiet t/z/g

	EXEC	--logical --readlink --quiet t/f

	EXEC	--logical --readlink --quiet t/dd/g

	EXEC	--logical --readlink --quiet t/dd/eee

	EXEC	--logical --readlink --quiet t/a
		OUTPUT - b
		EXIT 0

	EXEC	--logical --readlink --quiet t/b
		OUTPUT - c

	EXEC	--logical --readlink --quiet t/d/e
		OUTPUT - f

	EXEC	--logical --readlink --quiet t/d/dd
		OUTPUT - dd

	EXEC	--logical --readlink --quiet t/c
		OUTPUT - d/e

	EXEC	--logical --readlink --quiet t/xxx
		OUTPUT -
		EXIT 1

	EXEC	--logical --relative t/dd
		OUTPUT - t/dd
		EXIT 0

	EXEC	--logical --relative t/d
		OUTPUT - t/d

	EXEC	--logical --relative t/z
		OUTPUT - t/z

	EXEC	--logical --relative t/dd/g
		OUTPUT - t/dd/g

	EXEC	--logical --relative t/z/g
		OUTPUT - t/z/g

	EXEC	--logical --relative t/f
		OUTPUT - t/f

	EXEC	--logical --relative t/dd/g
		OUTPUT - t/dd/g

	EXEC	--logical --relative t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--logical --relative t/a
		OUTPUT - t/a

	EXEC	--logical --relative t/b
		OUTPUT - t/b

	EXEC	--logical --relative t/d/e
		OUTPUT - t/d/e

	EXEC	--logical --relative t/d/dd
		OUTPUT - t/d/dd

	EXEC	--logical --relative t/c
		OUTPUT - t/c

	EXEC	--logical --relative t/xxx
		OUTPUT -
		ERROR - 'realpath: t/xxx: canonicalization error [No such file or directory]'
		EXIT 1

	EXEC	--logical --relative --quiet t/dd
		OUTPUT - t/dd
		ERROR -n -
		EXIT 0

	EXEC	--logical --relative --quiet t/d
		OUTPUT - t/d

	EXEC	--logical --relative --quiet t/z
		OUTPUT - t/z

	EXEC	--logical --relative --quiet t/dd/g
		OUTPUT - t/dd/g

	EXEC	--logical --relative --quiet t/z/g
		OUTPUT - t/z/g

	EXEC	--logical --relative --quiet t/f
		OUTPUT - t/f

	EXEC	--logical --relative --quiet t/dd/g
		OUTPUT - t/dd/g

	EXEC	--logical --relative --quiet t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--logical --relative --quiet t/a
		OUTPUT - t/a

	EXEC	--logical --relative --quiet t/b
		OUTPUT - t/b

	EXEC	--logical --relative --quiet t/d/e
		OUTPUT - t/d/e

	EXEC	--logical --relative --quiet t/d/dd
		OUTPUT - t/d/dd

	EXEC	--logical --relative --quiet t/c
		OUTPUT - t/c

	EXEC	--logical --relative --quiet t/xxx
		OUTPUT -
		EXIT 1

	EXEC	--logical --relative --readlink t/dd
		ERROR - 'realpath: t/dd: cannot read link text [Invalid argument]'

	EXEC	--logical --relative --readlink t/d
		ERROR - 'realpath: t/d: cannot read link text [Invalid argument]'

	EXEC	--logical --relative --readlink t/z
		ERROR - 'realpath: t/z: cannot read link text [Invalid argument]'

	EXEC	--logical --relative --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--logical --relative --readlink t/z/g
		ERROR - 'realpath: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--logical --relative --readlink t/f
		ERROR - 'realpath: t/f: cannot read link text [Invalid argument]'

	EXEC	--logical --relative --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--logical --relative --readlink t/dd/eee
		ERROR - 'realpath: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--logical --relative --readlink t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--logical --relative --readlink t/b
		OUTPUT - c

	EXEC	--logical --relative --readlink t/d/e
		OUTPUT - f

	EXEC	--logical --relative --readlink t/d/dd
		OUTPUT - dd

	EXEC	--logical --relative --readlink t/c
		OUTPUT - d/e

	EXEC	--logical --relative --readlink t/xxx
		OUTPUT -
		ERROR - 'realpath: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--logical --relative --readlink --quiet t/dd
		ERROR -n -

	EXEC	--logical --relative --readlink --quiet t/d

	EXEC	--logical --relative --readlink --quiet t/z

	EXEC	--logical --relative --readlink --quiet t/dd/g

	EXEC	--logical --relative --readlink --quiet t/z/g

	EXEC	--logical --relative --readlink --quiet t/f

	EXEC	--logical --relative --readlink --quiet t/dd/g

	EXEC	--logical --relative --readlink --quiet t/dd/eee

	EXEC	--logical --relative --readlink --quiet t/a
		OUTPUT - b
		EXIT 0

	EXEC	--logical --relative --readlink --quiet t/b
		OUTPUT - c

	EXEC	--logical --relative --readlink --quiet t/d/e
		OUTPUT - f

	EXEC	--logical --relative --readlink --quiet t/d/dd
		OUTPUT - dd

	EXEC	--logical --relative --readlink --quiet t/c
		OUTPUT - d/e

	EXEC	--logical --relative --readlink --quiet t/xxx
		OUTPUT -
		EXIT 1

	EXEC	--physical t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd
		EXIT 0

	EXEC	--physical t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d

	EXEC	--physical t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z

	EXEC	--physical t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--physical t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z/g

	EXEC	--physical t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/f

	EXEC	--physical t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--physical t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/eee

	EXEC	--physical t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/f

	EXEC	--physical t/b

	EXEC	--physical t/d/e

	EXEC	--physical t/d/dd
		OUTPUT -
		ERROR - 'realpath: t/d/dd: canonicalization error [Too many levels of symbolic links]'
		EXIT 1

	EXEC	--physical t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/f
		ERROR -n -
		EXIT 0

	EXEC	--physical t/xxx
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/xxx

	EXEC	--physical --quiet t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd

	EXEC	--physical --quiet t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d

	EXEC	--physical --quiet t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z

	EXEC	--physical --quiet t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--physical --quiet t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z/g

	EXEC	--physical --quiet t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/f

	EXEC	--physical --quiet t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--physical --quiet t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/eee

	EXEC	--physical --quiet t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/f

	EXEC	--physical --quiet t/b

	EXEC	--physical --quiet t/d/e

	EXEC	--physical --quiet t/d/dd
		OUTPUT -
		EXIT 1

	EXEC	--physical --quiet t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/f
		EXIT 0

	EXEC	--physical --quiet t/xxx
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/xxx

	EXEC	--physical --readlink t/dd
		OUTPUT -
		ERROR - 'realpath: t/dd: cannot read link text [Invalid argument]'
		EXIT 1

	EXEC	--physical --readlink t/d
		ERROR - 'realpath: t/d: cannot read link text [Invalid argument]'

	EXEC	--physical --readlink t/z
		ERROR - 'realpath: t/z: cannot read link text [Invalid argument]'

	EXEC	--physical --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--physical --readlink t/z/g
		ERROR - 'realpath: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--physical --readlink t/f
		ERROR - 'realpath: t/f: cannot read link text [Invalid argument]'

	EXEC	--physical --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--physical --readlink t/dd/eee
		ERROR - 'realpath: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--physical --readlink t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--physical --readlink t/b
		OUTPUT - c

	EXEC	--physical --readlink t/d/e
		OUTPUT - f

	EXEC	--physical --readlink t/d/dd
		OUTPUT - dd

	EXEC	--physical --readlink t/c
		OUTPUT - d/e

	EXEC	--physical --readlink t/xxx
		OUTPUT -
		ERROR - 'realpath: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--physical --readlink --quiet t/dd
		ERROR -n -

	EXEC	--physical --readlink --quiet t/d

	EXEC	--physical --readlink --quiet t/z

	EXEC	--physical --readlink --quiet t/dd/g

	EXEC	--physical --readlink --quiet t/z/g

	EXEC	--physical --readlink --quiet t/f

	EXEC	--physical --readlink --quiet t/dd/g

	EXEC	--physical --readlink --quiet t/dd/eee

	EXEC	--physical --readlink --quiet t/a
		OUTPUT - b
		EXIT 0

	EXEC	--physical --readlink --quiet t/b
		OUTPUT - c

	EXEC	--physical --readlink --quiet t/d/e
		OUTPUT - f

	EXEC	--physical --readlink --quiet t/d/dd
		OUTPUT - dd

	EXEC	--physical --readlink --quiet t/c
		OUTPUT - d/e

	EXEC	--physical --readlink --quiet t/xxx
		OUTPUT -
		EXIT 1

	EXEC	--physical --relative t/dd
		OUTPUT - t/dd
		EXIT 0

	EXEC	--physical --relative t/d
		OUTPUT - t/d

	EXEC	--physical --relative t/z
		OUTPUT - t/z

	EXEC	--physical --relative t/dd/g
		OUTPUT - t/dd/g

	EXEC	--physical --relative t/z/g
		OUTPUT - t/z/g

	EXEC	--physical --relative t/f
		OUTPUT - t/f

	EXEC	--physical --relative t/dd/g
		OUTPUT - t/dd/g

	EXEC	--physical --relative t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--physical --relative t/a
		OUTPUT - t/d/f

	EXEC	--physical --relative t/b

	EXEC	--physical --relative t/d/e

	EXEC	--physical --relative t/d/dd
		OUTPUT -
		ERROR - 'realpath: t/d/dd: canonicalization error [Too many levels of symbolic links]'
		EXIT 1

	EXEC	--physical --relative t/c
		OUTPUT - t/d/f
		ERROR -n -
		EXIT 0

	EXEC	--physical --relative t/xxx
		OUTPUT - t/xxx

	EXEC	--physical --relative --quiet t/dd
		OUTPUT - t/dd

	EXEC	--physical --relative --quiet t/d
		OUTPUT - t/d

	EXEC	--physical --relative --quiet t/z
		OUTPUT - t/z

	EXEC	--physical --relative --quiet t/dd/g
		OUTPUT - t/dd/g

	EXEC	--physical --relative --quiet t/z/g
		OUTPUT - t/z/g

	EXEC	--physical --relative --quiet t/f
		OUTPUT - t/f

	EXEC	--physical --relative --quiet t/dd/g
		OUTPUT - t/dd/g

	EXEC	--physical --relative --quiet t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--physical --relative --quiet t/a
		OUTPUT - t/d/f

	EXEC	--physical --relative --quiet t/b

	EXEC	--physical --relative --quiet t/d/e

	EXEC	--physical --relative --quiet t/d/dd
		OUTPUT -
		EXIT 1

	EXEC	--physical --relative --quiet t/c
		OUTPUT - t/d/f
		EXIT 0

	EXEC	--physical --relative --quiet t/xxx
		OUTPUT - t/xxx

	EXEC	--physical --relative --readlink t/dd
		OUTPUT -
		ERROR - 'realpath: t/dd: cannot read link text [Invalid argument]'
		EXIT 1

	EXEC	--physical --relative --readlink t/d
		ERROR - 'realpath: t/d: cannot read link text [Invalid argument]'

	EXEC	--physical --relative --readlink t/z
		ERROR - 'realpath: t/z: cannot read link text [Invalid argument]'

	EXEC	--physical --relative --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--physical --relative --readlink t/z/g
		ERROR - 'realpath: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--physical --relative --readlink t/f
		ERROR - 'realpath: t/f: cannot read link text [Invalid argument]'

	EXEC	--physical --relative --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--physical --relative --readlink t/dd/eee
		ERROR - 'realpath: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--physical --relative --readlink t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--physical --relative --readlink t/b
		OUTPUT - c

	EXEC	--physical --relative --readlink t/d/e
		OUTPUT - f

	EXEC	--physical --relative --readlink t/d/dd
		OUTPUT - dd

	EXEC	--physical --relative --readlink t/c
		OUTPUT - d/e

	EXEC	--physical --relative --readlink t/xxx
		OUTPUT -
		ERROR - 'realpath: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--physical --relative --readlink --quiet t/dd
		ERROR -n -

	EXEC	--physical --relative --readlink --quiet t/d

	EXEC	--physical --relative --readlink --quiet t/z

	EXEC	--physical --relative --readlink --quiet t/dd/g

	EXEC	--physical --relative --readlink --quiet t/z/g

	EXEC	--physical --relative --readlink --quiet t/f

	EXEC	--physical --relative --readlink --quiet t/dd/g

	EXEC	--physical --relative --readlink --quiet t/dd/eee

	EXEC	--physical --relative --readlink --quiet t/a
		OUTPUT - b
		EXIT 0

	EXEC	--physical --relative --readlink --quiet t/b
		OUTPUT - c

	EXEC	--physical --relative --readlink --quiet t/d/e
		OUTPUT - f

	EXEC	--physical --relative --readlink --quiet t/d/dd
		OUTPUT - dd

	EXEC	--physical --relative --readlink --quiet t/c
		OUTPUT - d/e

	EXEC	--physical --relative --readlink --quiet t/xxx
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd
		EXIT 0

	EXEC	--canonicalize t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d

	EXEC	--canonicalize t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z

	EXEC	--canonicalize t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z/g

	EXEC	--canonicalize t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/f

	EXEC	--canonicalize t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/eee

	EXEC	--canonicalize t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/f

	EXEC	--canonicalize t/b

	EXEC	--canonicalize t/d/e

	EXEC	--canonicalize t/d/dd
		OUTPUT -
		ERROR - 'realpath: t/d/dd: canonicalization error [Too many levels of symbolic links]'
		EXIT 1

	EXEC	--canonicalize t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/f
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize t/xxx
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/xxx

	EXEC	--canonicalize --quiet t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd

	EXEC	--canonicalize --quiet t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d

	EXEC	--canonicalize --quiet t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z

	EXEC	--canonicalize --quiet t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize --quiet t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z/g

	EXEC	--canonicalize --quiet t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/f

	EXEC	--canonicalize --quiet t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize --quiet t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/eee

	EXEC	--canonicalize --quiet t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/f

	EXEC	--canonicalize --quiet t/b

	EXEC	--canonicalize --quiet t/d/e

	EXEC	--canonicalize --quiet t/d/dd
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize --quiet t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/f
		EXIT 0

	EXEC	--canonicalize --quiet t/xxx
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/xxx

	EXEC	--canonicalize --readlink t/dd
		OUTPUT -
		ERROR - 'realpath: t/dd: cannot read link text [Invalid argument]'
		EXIT 1

	EXEC	--canonicalize --readlink t/d
		ERROR - 'realpath: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --readlink t/z
		ERROR - 'realpath: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --readlink t/z/g
		ERROR - 'realpath: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --readlink t/f
		ERROR - 'realpath: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --readlink t/dd/eee
		ERROR - 'realpath: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --readlink t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/b
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize --readlink t/b
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/c

	EXEC	--canonicalize --readlink t/d/e
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/f

	EXEC	--canonicalize --readlink t/d/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/dd

	EXEC	--canonicalize --readlink t/c
		OUTPUT -
		ERROR - 'realpath: t/c: canonicalization error at e [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize --readlink t/xxx
		ERROR - 'realpath: t/xxx: cannot read link text [No such file or directory]'

	EXEC	--canonicalize --readlink --quiet t/dd
		ERROR -n -

	EXEC	--canonicalize --readlink --quiet t/d

	EXEC	--canonicalize --readlink --quiet t/z

	EXEC	--canonicalize --readlink --quiet t/dd/g

	EXEC	--canonicalize --readlink --quiet t/z/g

	EXEC	--canonicalize --readlink --quiet t/f

	EXEC	--canonicalize --readlink --quiet t/dd/g

	EXEC	--canonicalize --readlink --quiet t/dd/eee

	EXEC	--canonicalize --readlink --quiet t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/b
		EXIT 0

	EXEC	--canonicalize --readlink --quiet t/b
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/c

	EXEC	--canonicalize --readlink --quiet t/d/e
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/f

	EXEC	--canonicalize --readlink --quiet t/d/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/dd

	EXEC	--canonicalize --readlink --quiet t/c
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize --readlink --quiet t/xxx

	EXEC	--canonicalize --relative t/dd
		OUTPUT - t/dd
		EXIT 0

	EXEC	--canonicalize --relative t/d
		OUTPUT - t/d

	EXEC	--canonicalize --relative t/z
		OUTPUT - t/z

	EXEC	--canonicalize --relative t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize --relative t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize --relative t/f
		OUTPUT - t/f

	EXEC	--canonicalize --relative t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize --relative t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize --relative t/a
		OUTPUT - t/d/f

	EXEC	--canonicalize --relative t/b

	EXEC	--canonicalize --relative t/d/e

	EXEC	--canonicalize --relative t/d/dd
		OUTPUT -
		ERROR - 'realpath: t/d/dd: canonicalization error [Too many levels of symbolic links]'
		EXIT 1

	EXEC	--canonicalize --relative t/c
		OUTPUT - t/d/f
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize --relative t/xxx
		OUTPUT - t/xxx

	EXEC	--canonicalize --relative --quiet t/dd
		OUTPUT - t/dd

	EXEC	--canonicalize --relative --quiet t/d
		OUTPUT - t/d

	EXEC	--canonicalize --relative --quiet t/z
		OUTPUT - t/z

	EXEC	--canonicalize --relative --quiet t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize --relative --quiet t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize --relative --quiet t/f
		OUTPUT - t/f

	EXEC	--canonicalize --relative --quiet t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize --relative --quiet t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize --relative --quiet t/a
		OUTPUT - t/d/f

	EXEC	--canonicalize --relative --quiet t/b

	EXEC	--canonicalize --relative --quiet t/d/e

	EXEC	--canonicalize --relative --quiet t/d/dd
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize --relative --quiet t/c
		OUTPUT - t/d/f
		EXIT 0

	EXEC	--canonicalize --relative --quiet t/xxx
		OUTPUT - t/xxx

	EXEC	--canonicalize --relative --readlink t/dd
		OUTPUT -
		ERROR - 'realpath: t/dd: cannot read link text [Invalid argument]'
		EXIT 1

	EXEC	--canonicalize --relative --readlink t/d
		ERROR - 'realpath: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --relative --readlink t/z
		ERROR - 'realpath: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --relative --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --relative --readlink t/z/g
		ERROR - 'realpath: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --relative --readlink t/f
		ERROR - 'realpath: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --relative --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --relative --readlink t/dd/eee
		ERROR - 'realpath: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --relative --readlink t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize --relative --readlink t/b
		OUTPUT - c

	EXEC	--canonicalize --relative --readlink t/d/e
		OUTPUT - f

	EXEC	--canonicalize --relative --readlink t/d/dd
		OUTPUT - dd

	EXEC	--canonicalize --relative --readlink t/c
		OUTPUT -
		ERROR - 'realpath: t/c: canonicalization error at e [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize --relative --readlink t/xxx
		ERROR - 'realpath: t/xxx: cannot read link text [No such file or directory]'

	EXEC	--canonicalize --relative --readlink --quiet t/dd
		ERROR -n -

	EXEC	--canonicalize --relative --readlink --quiet t/d

	EXEC	--canonicalize --relative --readlink --quiet t/z

	EXEC	--canonicalize --relative --readlink --quiet t/dd/g

	EXEC	--canonicalize --relative --readlink --quiet t/z/g

	EXEC	--canonicalize --relative --readlink --quiet t/f

	EXEC	--canonicalize --relative --readlink --quiet t/dd/g

	EXEC	--canonicalize --relative --readlink --quiet t/dd/eee

	EXEC	--canonicalize --relative --readlink --quiet t/a
		OUTPUT - b
		EXIT 0

	EXEC	--canonicalize --relative --readlink --quiet t/b
		OUTPUT - c

	EXEC	--canonicalize --relative --readlink --quiet t/d/e
		OUTPUT - f

	EXEC	--canonicalize --relative --readlink --quiet t/d/dd
		OUTPUT - dd

	EXEC	--canonicalize --relative --readlink --quiet t/c
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize --relative --readlink --quiet t/xxx

	EXEC	--canonicalize --logical t/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd
		EXIT 0

	EXEC	--canonicalize --logical t/d
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d

	EXEC	--canonicalize --logical t/z
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z

	EXEC	--canonicalize --logical t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize --logical t/z/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z/g

	EXEC	--canonicalize --logical t/f
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/f

	EXEC	--canonicalize --logical t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize --logical t/dd/eee
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/eee

	EXEC	--canonicalize --logical t/a
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/a

	EXEC	--canonicalize --logical t/b
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/b

	EXEC	--canonicalize --logical t/d/e
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/e

	EXEC	--canonicalize --logical t/d/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/dd

	EXEC	--canonicalize --logical t/c
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/c

	EXEC	--canonicalize --logical t/xxx
		OUTPUT -
		ERROR - 'realpath: t/xxx: canonicalization error [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize --logical --quiet t/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize --logical --quiet t/d
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d

	EXEC	--canonicalize --logical --quiet t/z
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z

	EXEC	--canonicalize --logical --quiet t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize --logical --quiet t/z/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z/g

	EXEC	--canonicalize --logical --quiet t/f
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/f

	EXEC	--canonicalize --logical --quiet t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize --logical --quiet t/dd/eee
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/eee

	EXEC	--canonicalize --logical --quiet t/a
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/a

	EXEC	--canonicalize --logical --quiet t/b
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/b

	EXEC	--canonicalize --logical --quiet t/d/e
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/e

	EXEC	--canonicalize --logical --quiet t/d/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/dd

	EXEC	--canonicalize --logical --quiet t/c
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/c

	EXEC	--canonicalize --logical --quiet t/xxx
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize --logical --readlink t/dd
		ERROR - 'realpath: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --readlink t/d
		ERROR - 'realpath: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --readlink t/z
		ERROR - 'realpath: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --readlink t/z/g
		ERROR - 'realpath: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --readlink t/f
		ERROR - 'realpath: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --readlink t/dd/eee
		ERROR - 'realpath: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --readlink t/a
		ERROR - 'realpath: t/a: canonicalization error [No such file or directory]'

	EXEC	--canonicalize --logical --readlink t/b
		ERROR - 'realpath: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize --logical --readlink t/d/e
		ERROR - 'realpath: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize --logical --readlink t/d/dd
		ERROR - 'realpath: t/d/dd: canonicalization error [No such file or directory]'

	EXEC	--canonicalize --logical --readlink t/c
		ERROR - 'realpath: t/c: canonicalization error at e [No such file or directory]'

	EXEC	--canonicalize --logical --readlink t/xxx
		ERROR - 'realpath: t/xxx: cannot read link text [No such file or directory]'

	EXEC	--canonicalize --logical --readlink --quiet t/dd
		ERROR -n -

	EXEC	--canonicalize --logical --readlink --quiet t/d

	EXEC	--canonicalize --logical --readlink --quiet t/z

	EXEC	--canonicalize --logical --readlink --quiet t/dd/g

	EXEC	--canonicalize --logical --readlink --quiet t/z/g

	EXEC	--canonicalize --logical --readlink --quiet t/f

	EXEC	--canonicalize --logical --readlink --quiet t/dd/g

	EXEC	--canonicalize --logical --readlink --quiet t/dd/eee

	EXEC	--canonicalize --logical --readlink --quiet t/a

	EXEC	--canonicalize --logical --readlink --quiet t/b

	EXEC	--canonicalize --logical --readlink --quiet t/d/e

	EXEC	--canonicalize --logical --readlink --quiet t/d/dd

	EXEC	--canonicalize --logical --readlink --quiet t/c

	EXEC	--canonicalize --logical --readlink --quiet t/xxx

	EXEC	--canonicalize --logical --relative t/dd
		OUTPUT - t/dd
		EXIT 0

	EXEC	--canonicalize --logical --relative t/d
		OUTPUT - t/d

	EXEC	--canonicalize --logical --relative t/z
		OUTPUT - t/z

	EXEC	--canonicalize --logical --relative t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize --logical --relative t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize --logical --relative t/f
		OUTPUT - t/f

	EXEC	--canonicalize --logical --relative t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize --logical --relative t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize --logical --relative t/a
		OUTPUT - t/a

	EXEC	--canonicalize --logical --relative t/b
		OUTPUT - t/b

	EXEC	--canonicalize --logical --relative t/d/e
		OUTPUT - t/d/e

	EXEC	--canonicalize --logical --relative t/d/dd
		OUTPUT - t/d/dd

	EXEC	--canonicalize --logical --relative t/c
		OUTPUT - t/c

	EXEC	--canonicalize --logical --relative t/xxx
		OUTPUT -
		ERROR - 'realpath: t/xxx: canonicalization error [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize --logical --relative --quiet t/dd
		OUTPUT - t/dd
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize --logical --relative --quiet t/d
		OUTPUT - t/d

	EXEC	--canonicalize --logical --relative --quiet t/z
		OUTPUT - t/z

	EXEC	--canonicalize --logical --relative --quiet t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize --logical --relative --quiet t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize --logical --relative --quiet t/f
		OUTPUT - t/f

	EXEC	--canonicalize --logical --relative --quiet t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize --logical --relative --quiet t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize --logical --relative --quiet t/a
		OUTPUT - t/a

	EXEC	--canonicalize --logical --relative --quiet t/b
		OUTPUT - t/b

	EXEC	--canonicalize --logical --relative --quiet t/d/e
		OUTPUT - t/d/e

	EXEC	--canonicalize --logical --relative --quiet t/d/dd
		OUTPUT - t/d/dd

	EXEC	--canonicalize --logical --relative --quiet t/c
		OUTPUT - t/c

	EXEC	--canonicalize --logical --relative --quiet t/xxx
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize --logical --relative --readlink t/dd
		ERROR - 'realpath: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --relative --readlink t/d
		ERROR - 'realpath: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --relative --readlink t/z
		ERROR - 'realpath: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --relative --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --relative --readlink t/z/g
		ERROR - 'realpath: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --relative --readlink t/f
		ERROR - 'realpath: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --relative --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --relative --readlink t/dd/eee
		ERROR - 'realpath: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --relative --readlink t/a
		ERROR - 'realpath: t/a: canonicalization error [No such file or directory]'

	EXEC	--canonicalize --logical --relative --readlink t/b
		ERROR - 'realpath: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize --logical --relative --readlink t/d/e
		ERROR - 'realpath: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize --logical --relative --readlink t/d/dd
		ERROR - 'realpath: t/d/dd: canonicalization error [No such file or directory]'

	EXEC	--canonicalize --logical --relative --readlink t/c
		ERROR - 'realpath: t/c: canonicalization error at e [No such file or directory]'

	EXEC	--canonicalize --logical --relative --readlink t/xxx
		ERROR - 'realpath: t/xxx: cannot read link text [No such file or directory]'

	EXEC	--canonicalize --logical --relative --readlink --quiet t/dd
		ERROR -n -

	EXEC	--canonicalize --logical --relative --readlink --quiet t/d

	EXEC	--canonicalize --logical --relative --readlink --quiet t/z

	EXEC	--canonicalize --logical --relative --readlink --quiet t/dd/g

	EXEC	--canonicalize --logical --relative --readlink --quiet t/z/g

	EXEC	--canonicalize --logical --relative --readlink --quiet t/f

	EXEC	--canonicalize --logical --relative --readlink --quiet t/dd/g

	EXEC	--canonicalize --logical --relative --readlink --quiet t/dd/eee

	EXEC	--canonicalize --logical --relative --readlink --quiet t/a

	EXEC	--canonicalize --logical --relative --readlink --quiet t/b

	EXEC	--canonicalize --logical --relative --readlink --quiet t/d/e

	EXEC	--canonicalize --logical --relative --readlink --quiet t/d/dd

	EXEC	--canonicalize --logical --relative --readlink --quiet t/c

	EXEC	--canonicalize --logical --relative --readlink --quiet t/xxx

	EXEC	--canonicalize --physical t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd
		EXIT 0

	EXEC	--canonicalize --physical t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d

	EXEC	--canonicalize --physical t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z

	EXEC	--canonicalize --physical t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize --physical t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z/g

	EXEC	--canonicalize --physical t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/f

	EXEC	--canonicalize --physical t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize --physical t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/eee

	EXEC	--canonicalize --physical t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/f

	EXEC	--canonicalize --physical t/b

	EXEC	--canonicalize --physical t/d/e

	EXEC	--canonicalize --physical t/d/dd
		OUTPUT -
		ERROR - 'realpath: t/d/dd: canonicalization error [Too many levels of symbolic links]'
		EXIT 1

	EXEC	--canonicalize --physical t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/f
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize --physical t/xxx
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/xxx

	EXEC	--canonicalize --physical --quiet t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd

	EXEC	--canonicalize --physical --quiet t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d

	EXEC	--canonicalize --physical --quiet t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z

	EXEC	--canonicalize --physical --quiet t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize --physical --quiet t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z/g

	EXEC	--canonicalize --physical --quiet t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/f

	EXEC	--canonicalize --physical --quiet t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize --physical --quiet t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/eee

	EXEC	--canonicalize --physical --quiet t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/f

	EXEC	--canonicalize --physical --quiet t/b

	EXEC	--canonicalize --physical --quiet t/d/e

	EXEC	--canonicalize --physical --quiet t/d/dd
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize --physical --quiet t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/f
		EXIT 0

	EXEC	--canonicalize --physical --quiet t/xxx
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/xxx

	EXEC	--canonicalize --physical --readlink t/dd
		OUTPUT -
		ERROR - 'realpath: t/dd: cannot read link text [Invalid argument]'
		EXIT 1

	EXEC	--canonicalize --physical --readlink t/d
		ERROR - 'realpath: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --readlink t/z
		ERROR - 'realpath: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --readlink t/z/g
		ERROR - 'realpath: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --readlink t/f
		ERROR - 'realpath: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --readlink t/dd/eee
		ERROR - 'realpath: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --readlink t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/b
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize --physical --readlink t/b
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/c

	EXEC	--canonicalize --physical --readlink t/d/e
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/f

	EXEC	--canonicalize --physical --readlink t/d/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/dd

	EXEC	--canonicalize --physical --readlink t/c
		OUTPUT -
		ERROR - 'realpath: t/c: canonicalization error at e [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize --physical --readlink t/xxx
		ERROR - 'realpath: t/xxx: cannot read link text [No such file or directory]'

	EXEC	--canonicalize --physical --readlink --quiet t/dd
		ERROR -n -

	EXEC	--canonicalize --physical --readlink --quiet t/d

	EXEC	--canonicalize --physical --readlink --quiet t/z

	EXEC	--canonicalize --physical --readlink --quiet t/dd/g

	EXEC	--canonicalize --physical --readlink --quiet t/z/g

	EXEC	--canonicalize --physical --readlink --quiet t/f

	EXEC	--canonicalize --physical --readlink --quiet t/dd/g

	EXEC	--canonicalize --physical --readlink --quiet t/dd/eee

	EXEC	--canonicalize --physical --readlink --quiet t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/b
		EXIT 0

	EXEC	--canonicalize --physical --readlink --quiet t/b
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/c

	EXEC	--canonicalize --physical --readlink --quiet t/d/e
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/f

	EXEC	--canonicalize --physical --readlink --quiet t/d/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/dd

	EXEC	--canonicalize --physical --readlink --quiet t/c
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize --physical --readlink --quiet t/xxx

	EXEC	--canonicalize --physical --relative t/dd
		OUTPUT - t/dd
		EXIT 0

	EXEC	--canonicalize --physical --relative t/d
		OUTPUT - t/d

	EXEC	--canonicalize --physical --relative t/z
		OUTPUT - t/z

	EXEC	--canonicalize --physical --relative t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize --physical --relative t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize --physical --relative t/f
		OUTPUT - t/f

	EXEC	--canonicalize --physical --relative t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize --physical --relative t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize --physical --relative t/a
		OUTPUT - t/d/f

	EXEC	--canonicalize --physical --relative t/b

	EXEC	--canonicalize --physical --relative t/d/e

	EXEC	--canonicalize --physical --relative t/d/dd
		OUTPUT -
		ERROR - 'realpath: t/d/dd: canonicalization error [Too many levels of symbolic links]'
		EXIT 1

	EXEC	--canonicalize --physical --relative t/c
		OUTPUT - t/d/f
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize --physical --relative t/xxx
		OUTPUT - t/xxx

	EXEC	--canonicalize --physical --relative --quiet t/dd
		OUTPUT - t/dd

	EXEC	--canonicalize --physical --relative --quiet t/d
		OUTPUT - t/d

	EXEC	--canonicalize --physical --relative --quiet t/z
		OUTPUT - t/z

	EXEC	--canonicalize --physical --relative --quiet t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize --physical --relative --quiet t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize --physical --relative --quiet t/f
		OUTPUT - t/f

	EXEC	--canonicalize --physical --relative --quiet t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize --physical --relative --quiet t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize --physical --relative --quiet t/a
		OUTPUT - t/d/f

	EXEC	--canonicalize --physical --relative --quiet t/b

	EXEC	--canonicalize --physical --relative --quiet t/d/e

	EXEC	--canonicalize --physical --relative --quiet t/d/dd
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize --physical --relative --quiet t/c
		OUTPUT - t/d/f
		EXIT 0

	EXEC	--canonicalize --physical --relative --quiet t/xxx
		OUTPUT - t/xxx

	EXEC	--canonicalize --physical --relative --readlink t/dd
		OUTPUT -
		ERROR - 'realpath: t/dd: cannot read link text [Invalid argument]'
		EXIT 1

	EXEC	--canonicalize --physical --relative --readlink t/d
		ERROR - 'realpath: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --relative --readlink t/z
		ERROR - 'realpath: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --relative --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --relative --readlink t/z/g
		ERROR - 'realpath: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --relative --readlink t/f
		ERROR - 'realpath: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --relative --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --relative --readlink t/dd/eee
		ERROR - 'realpath: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --relative --readlink t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize --physical --relative --readlink t/b
		OUTPUT - c

	EXEC	--canonicalize --physical --relative --readlink t/d/e
		OUTPUT - f

	EXEC	--canonicalize --physical --relative --readlink t/d/dd
		OUTPUT - dd

	EXEC	--canonicalize --physical --relative --readlink t/c
		OUTPUT -
		ERROR - 'realpath: t/c: canonicalization error at e [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize --physical --relative --readlink t/xxx
		ERROR - 'realpath: t/xxx: cannot read link text [No such file or directory]'

	EXEC	--canonicalize --physical --relative --readlink --quiet t/dd
		ERROR -n -

	EXEC	--canonicalize --physical --relative --readlink --quiet t/d

	EXEC	--canonicalize --physical --relative --readlink --quiet t/z

	EXEC	--canonicalize --physical --relative --readlink --quiet t/dd/g

	EXEC	--canonicalize --physical --relative --readlink --quiet t/z/g

	EXEC	--canonicalize --physical --relative --readlink --quiet t/f

	EXEC	--canonicalize --physical --relative --readlink --quiet t/dd/g

	EXEC	--canonicalize --physical --relative --readlink --quiet t/dd/eee

	EXEC	--canonicalize --physical --relative --readlink --quiet t/a
		OUTPUT - b
		EXIT 0

	EXEC	--canonicalize --physical --relative --readlink --quiet t/b
		OUTPUT - c

	EXEC	--canonicalize --physical --relative --readlink --quiet t/d/e
		OUTPUT - f

	EXEC	--canonicalize --physical --relative --readlink --quiet t/d/dd
		OUTPUT - dd

	EXEC	--canonicalize --physical --relative --readlink --quiet t/c
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize --physical --relative --readlink --quiet t/xxx

	EXEC	--canonicalize-existing t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd
		EXIT 0

	EXEC	--canonicalize-existing t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d

	EXEC	--canonicalize-existing t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z

	EXEC	--canonicalize-existing t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize-existing t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z/g

	EXEC	--canonicalize-existing t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/f

	EXEC	--canonicalize-existing t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize-existing t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/eee

	EXEC	--canonicalize-existing t/a
		OUTPUT -
		ERROR - 'realpath: t/a: canonicalization error [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize-existing t/b
		ERROR - 'realpath: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing t/d/e
		ERROR - 'realpath: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing t/d/dd
		ERROR - 'realpath: t/d/dd: canonicalization error [Too many levels of symbolic links]'

	EXEC	--canonicalize-existing t/c
		ERROR - 'realpath: t/c: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing t/xxx
		ERROR - 'realpath: t/xxx: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --quiet t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize-existing --quiet t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d

	EXEC	--canonicalize-existing --quiet t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z

	EXEC	--canonicalize-existing --quiet t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize-existing --quiet t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z/g

	EXEC	--canonicalize-existing --quiet t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/f

	EXEC	--canonicalize-existing --quiet t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize-existing --quiet t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/eee

	EXEC	--canonicalize-existing --quiet t/a
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize-existing --quiet t/b

	EXEC	--canonicalize-existing --quiet t/d/e

	EXEC	--canonicalize-existing --quiet t/d/dd

	EXEC	--canonicalize-existing --quiet t/c

	EXEC	--canonicalize-existing --quiet t/xxx

	EXEC	--canonicalize-existing --readlink t/dd
		ERROR - 'realpath: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --readlink t/d
		ERROR - 'realpath: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --readlink t/z
		ERROR - 'realpath: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --readlink t/z/g
		ERROR - 'realpath: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --readlink t/f
		ERROR - 'realpath: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --readlink t/dd/eee
		ERROR - 'realpath: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --readlink t/a
		ERROR - 'realpath: t/a: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --readlink t/b
		ERROR - 'realpath: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --readlink t/d/e
		ERROR - 'realpath: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --readlink t/d/dd
		ERROR - 'realpath: t/d/dd: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --readlink t/c
		ERROR - 'realpath: t/c: canonicalization error at e [No such file or directory]'

	EXEC	--canonicalize-existing --readlink t/xxx
		ERROR - 'realpath: t/xxx: cannot read link text [No such file or directory]'

	EXEC	--canonicalize-existing --readlink --quiet t/dd
		ERROR -n -

	EXEC	--canonicalize-existing --readlink --quiet t/d

	EXEC	--canonicalize-existing --readlink --quiet t/z

	EXEC	--canonicalize-existing --readlink --quiet t/dd/g

	EXEC	--canonicalize-existing --readlink --quiet t/z/g

	EXEC	--canonicalize-existing --readlink --quiet t/f

	EXEC	--canonicalize-existing --readlink --quiet t/dd/g

	EXEC	--canonicalize-existing --readlink --quiet t/dd/eee

	EXEC	--canonicalize-existing --readlink --quiet t/a

	EXEC	--canonicalize-existing --readlink --quiet t/b

	EXEC	--canonicalize-existing --readlink --quiet t/d/e

	EXEC	--canonicalize-existing --readlink --quiet t/d/dd

	EXEC	--canonicalize-existing --readlink --quiet t/c

	EXEC	--canonicalize-existing --readlink --quiet t/xxx

	EXEC	--canonicalize-existing --relative t/dd
		OUTPUT - t/dd
		EXIT 0

	EXEC	--canonicalize-existing --relative t/d
		OUTPUT - t/d

	EXEC	--canonicalize-existing --relative t/z
		OUTPUT - t/z

	EXEC	--canonicalize-existing --relative t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-existing --relative t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize-existing --relative t/f
		OUTPUT - t/f

	EXEC	--canonicalize-existing --relative t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-existing --relative t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize-existing --relative t/a
		OUTPUT -
		ERROR - 'realpath: t/a: canonicalization error [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize-existing --relative t/b
		ERROR - 'realpath: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --relative t/d/e
		ERROR - 'realpath: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --relative t/d/dd
		ERROR - 'realpath: t/d/dd: canonicalization error [Too many levels of symbolic links]'

	EXEC	--canonicalize-existing --relative t/c
		ERROR - 'realpath: t/c: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --relative t/xxx
		ERROR - 'realpath: t/xxx: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --relative --quiet t/dd
		OUTPUT - t/dd
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize-existing --relative --quiet t/d
		OUTPUT - t/d

	EXEC	--canonicalize-existing --relative --quiet t/z
		OUTPUT - t/z

	EXEC	--canonicalize-existing --relative --quiet t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-existing --relative --quiet t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize-existing --relative --quiet t/f
		OUTPUT - t/f

	EXEC	--canonicalize-existing --relative --quiet t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-existing --relative --quiet t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize-existing --relative --quiet t/a
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize-existing --relative --quiet t/b

	EXEC	--canonicalize-existing --relative --quiet t/d/e

	EXEC	--canonicalize-existing --relative --quiet t/d/dd

	EXEC	--canonicalize-existing --relative --quiet t/c

	EXEC	--canonicalize-existing --relative --quiet t/xxx

	EXEC	--canonicalize-existing --relative --readlink t/dd
		ERROR - 'realpath: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --relative --readlink t/d
		ERROR - 'realpath: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --relative --readlink t/z
		ERROR - 'realpath: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --relative --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --relative --readlink t/z/g
		ERROR - 'realpath: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --relative --readlink t/f
		ERROR - 'realpath: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --relative --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --relative --readlink t/dd/eee
		ERROR - 'realpath: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --relative --readlink t/a
		ERROR - 'realpath: t/a: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --relative --readlink t/b
		ERROR - 'realpath: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --relative --readlink t/d/e
		ERROR - 'realpath: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --relative --readlink t/d/dd
		ERROR - 'realpath: t/d/dd: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --relative --readlink t/c
		ERROR - 'realpath: t/c: canonicalization error at e [No such file or directory]'

	EXEC	--canonicalize-existing --relative --readlink t/xxx
		ERROR - 'realpath: t/xxx: cannot read link text [No such file or directory]'

	EXEC	--canonicalize-existing --relative --readlink --quiet t/dd
		ERROR -n -

	EXEC	--canonicalize-existing --relative --readlink --quiet t/d

	EXEC	--canonicalize-existing --relative --readlink --quiet t/z

	EXEC	--canonicalize-existing --relative --readlink --quiet t/dd/g

	EXEC	--canonicalize-existing --relative --readlink --quiet t/z/g

	EXEC	--canonicalize-existing --relative --readlink --quiet t/f

	EXEC	--canonicalize-existing --relative --readlink --quiet t/dd/g

	EXEC	--canonicalize-existing --relative --readlink --quiet t/dd/eee

	EXEC	--canonicalize-existing --relative --readlink --quiet t/a

	EXEC	--canonicalize-existing --relative --readlink --quiet t/b

	EXEC	--canonicalize-existing --relative --readlink --quiet t/d/e

	EXEC	--canonicalize-existing --relative --readlink --quiet t/d/dd

	EXEC	--canonicalize-existing --relative --readlink --quiet t/c

	EXEC	--canonicalize-existing --relative --readlink --quiet t/xxx

	EXEC	--canonicalize-existing --logical t/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd
		EXIT 0

	EXEC	--canonicalize-existing --logical t/d
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d

	EXEC	--canonicalize-existing --logical t/z
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z

	EXEC	--canonicalize-existing --logical t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize-existing --logical t/z/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z/g

	EXEC	--canonicalize-existing --logical t/f
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/f

	EXEC	--canonicalize-existing --logical t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize-existing --logical t/dd/eee
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/eee

	EXEC	--canonicalize-existing --logical t/a
		OUTPUT -
		ERROR - 'realpath: t/a: canonicalization error [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize-existing --logical t/b
		ERROR - 'realpath: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical t/d/e
		ERROR - 'realpath: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical t/d/dd
		ERROR - 'realpath: t/d/dd: canonicalization error [Too many levels of symbolic links]'

	EXEC	--canonicalize-existing --logical t/c
		ERROR - 'realpath: t/c: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical t/xxx
		ERROR - 'realpath: t/xxx: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --quiet t/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize-existing --logical --quiet t/d
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d

	EXEC	--canonicalize-existing --logical --quiet t/z
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z

	EXEC	--canonicalize-existing --logical --quiet t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize-existing --logical --quiet t/z/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z/g

	EXEC	--canonicalize-existing --logical --quiet t/f
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/f

	EXEC	--canonicalize-existing --logical --quiet t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize-existing --logical --quiet t/dd/eee
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/eee

	EXEC	--canonicalize-existing --logical --quiet t/a
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize-existing --logical --quiet t/b

	EXEC	--canonicalize-existing --logical --quiet t/d/e

	EXEC	--canonicalize-existing --logical --quiet t/d/dd

	EXEC	--canonicalize-existing --logical --quiet t/c

	EXEC	--canonicalize-existing --logical --quiet t/xxx

	EXEC	--canonicalize-existing --logical --readlink t/dd
		ERROR - 'realpath: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --readlink t/d
		ERROR - 'realpath: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --readlink t/z
		ERROR - 'realpath: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --readlink t/z/g
		ERROR - 'realpath: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --readlink t/f
		ERROR - 'realpath: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --readlink t/dd/eee
		ERROR - 'realpath: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --readlink t/a
		ERROR - 'realpath: t/a: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --readlink t/b
		ERROR - 'realpath: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --readlink t/d/e
		ERROR - 'realpath: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --readlink t/d/dd
		ERROR - 'realpath: t/d/dd: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --readlink t/c
		ERROR - 'realpath: t/c: canonicalization error at e [No such file or directory]'

	EXEC	--canonicalize-existing --logical --readlink t/xxx
		ERROR - 'realpath: t/xxx: cannot read link text [No such file or directory]'

	EXEC	--canonicalize-existing --logical --readlink --quiet t/dd
		ERROR -n -

	EXEC	--canonicalize-existing --logical --readlink --quiet t/d

	EXEC	--canonicalize-existing --logical --readlink --quiet t/z

	EXEC	--canonicalize-existing --logical --readlink --quiet t/dd/g

	EXEC	--canonicalize-existing --logical --readlink --quiet t/z/g

	EXEC	--canonicalize-existing --logical --readlink --quiet t/f

	EXEC	--canonicalize-existing --logical --readlink --quiet t/dd/g

	EXEC	--canonicalize-existing --logical --readlink --quiet t/dd/eee

	EXEC	--canonicalize-existing --logical --readlink --quiet t/a

	EXEC	--canonicalize-existing --logical --readlink --quiet t/b

	EXEC	--canonicalize-existing --logical --readlink --quiet t/d/e

	EXEC	--canonicalize-existing --logical --readlink --quiet t/d/dd

	EXEC	--canonicalize-existing --logical --readlink --quiet t/c

	EXEC	--canonicalize-existing --logical --readlink --quiet t/xxx

	EXEC	--canonicalize-existing --logical --relative t/dd
		OUTPUT - t/dd
		EXIT 0

	EXEC	--canonicalize-existing --logical --relative t/d
		OUTPUT - t/d

	EXEC	--canonicalize-existing --logical --relative t/z
		OUTPUT - t/z

	EXEC	--canonicalize-existing --logical --relative t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-existing --logical --relative t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize-existing --logical --relative t/f
		OUTPUT - t/f

	EXEC	--canonicalize-existing --logical --relative t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-existing --logical --relative t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize-existing --logical --relative t/a
		OUTPUT -
		ERROR - 'realpath: t/a: canonicalization error [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize-existing --logical --relative t/b
		ERROR - 'realpath: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --relative t/d/e
		ERROR - 'realpath: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --relative t/d/dd
		ERROR - 'realpath: t/d/dd: canonicalization error [Too many levels of symbolic links]'

	EXEC	--canonicalize-existing --logical --relative t/c
		ERROR - 'realpath: t/c: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --relative t/xxx
		ERROR - 'realpath: t/xxx: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --relative --quiet t/dd
		OUTPUT - t/dd
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize-existing --logical --relative --quiet t/d
		OUTPUT - t/d

	EXEC	--canonicalize-existing --logical --relative --quiet t/z
		OUTPUT - t/z

	EXEC	--canonicalize-existing --logical --relative --quiet t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-existing --logical --relative --quiet t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize-existing --logical --relative --quiet t/f
		OUTPUT - t/f

	EXEC	--canonicalize-existing --logical --relative --quiet t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-existing --logical --relative --quiet t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize-existing --logical --relative --quiet t/a
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize-existing --logical --relative --quiet t/b

	EXEC	--canonicalize-existing --logical --relative --quiet t/d/e

	EXEC	--canonicalize-existing --logical --relative --quiet t/d/dd

	EXEC	--canonicalize-existing --logical --relative --quiet t/c

	EXEC	--canonicalize-existing --logical --relative --quiet t/xxx

	EXEC	--canonicalize-existing --logical --relative --readlink t/dd
		ERROR - 'realpath: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --relative --readlink t/d
		ERROR - 'realpath: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --relative --readlink t/z
		ERROR - 'realpath: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --relative --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --relative --readlink t/z/g
		ERROR - 'realpath: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --relative --readlink t/f
		ERROR - 'realpath: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --relative --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --relative --readlink t/dd/eee
		ERROR - 'realpath: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --relative --readlink t/a
		ERROR - 'realpath: t/a: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --relative --readlink t/b
		ERROR - 'realpath: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --relative --readlink t/d/e
		ERROR - 'realpath: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --relative --readlink t/d/dd
		ERROR - 'realpath: t/d/dd: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --relative --readlink t/c
		ERROR - 'realpath: t/c: canonicalization error at e [No such file or directory]'

	EXEC	--canonicalize-existing --logical --relative --readlink t/xxx
		ERROR - 'realpath: t/xxx: cannot read link text [No such file or directory]'

	EXEC	--canonicalize-existing --logical --relative --readlink --quiet t/dd
		ERROR -n -

	EXEC	--canonicalize-existing --logical --relative --readlink --quiet t/d

	EXEC	--canonicalize-existing --logical --relative --readlink --quiet t/z

	EXEC	--canonicalize-existing --logical --relative --readlink --quiet t/dd/g

	EXEC	--canonicalize-existing --logical --relative --readlink --quiet t/z/g

	EXEC	--canonicalize-existing --logical --relative --readlink --quiet t/f

	EXEC	--canonicalize-existing --logical --relative --readlink --quiet t/dd/g

	EXEC	--canonicalize-existing --logical --relative --readlink --quiet t/dd/eee

	EXEC	--canonicalize-existing --logical --relative --readlink --quiet t/a

	EXEC	--canonicalize-existing --logical --relative --readlink --quiet t/b

	EXEC	--canonicalize-existing --logical --relative --readlink --quiet t/d/e

	EXEC	--canonicalize-existing --logical --relative --readlink --quiet t/d/dd

	EXEC	--canonicalize-existing --logical --relative --readlink --quiet t/c

	EXEC	--canonicalize-existing --logical --relative --readlink --quiet t/xxx

	EXEC	--canonicalize-existing --physical t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd
		EXIT 0

	EXEC	--canonicalize-existing --physical t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d

	EXEC	--canonicalize-existing --physical t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z

	EXEC	--canonicalize-existing --physical t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize-existing --physical t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z/g

	EXEC	--canonicalize-existing --physical t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/f

	EXEC	--canonicalize-existing --physical t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize-existing --physical t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/eee

	EXEC	--canonicalize-existing --physical t/a
		OUTPUT -
		ERROR - 'realpath: t/a: canonicalization error [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize-existing --physical t/b
		ERROR - 'realpath: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical t/d/e
		ERROR - 'realpath: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical t/d/dd
		ERROR - 'realpath: t/d/dd: canonicalization error [Too many levels of symbolic links]'

	EXEC	--canonicalize-existing --physical t/c
		ERROR - 'realpath: t/c: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical t/xxx
		ERROR - 'realpath: t/xxx: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --quiet t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize-existing --physical --quiet t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d

	EXEC	--canonicalize-existing --physical --quiet t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z

	EXEC	--canonicalize-existing --physical --quiet t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize-existing --physical --quiet t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z/g

	EXEC	--canonicalize-existing --physical --quiet t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/f

	EXEC	--canonicalize-existing --physical --quiet t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize-existing --physical --quiet t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/eee

	EXEC	--canonicalize-existing --physical --quiet t/a
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize-existing --physical --quiet t/b

	EXEC	--canonicalize-existing --physical --quiet t/d/e

	EXEC	--canonicalize-existing --physical --quiet t/d/dd

	EXEC	--canonicalize-existing --physical --quiet t/c

	EXEC	--canonicalize-existing --physical --quiet t/xxx

	EXEC	--canonicalize-existing --physical --readlink t/dd
		ERROR - 'realpath: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --readlink t/d
		ERROR - 'realpath: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --readlink t/z
		ERROR - 'realpath: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --readlink t/z/g
		ERROR - 'realpath: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --readlink t/f
		ERROR - 'realpath: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --readlink t/dd/eee
		ERROR - 'realpath: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --readlink t/a
		ERROR - 'realpath: t/a: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --readlink t/b
		ERROR - 'realpath: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --readlink t/d/e
		ERROR - 'realpath: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --readlink t/d/dd
		ERROR - 'realpath: t/d/dd: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --readlink t/c
		ERROR - 'realpath: t/c: canonicalization error at e [No such file or directory]'

	EXEC	--canonicalize-existing --physical --readlink t/xxx
		ERROR - 'realpath: t/xxx: cannot read link text [No such file or directory]'

	EXEC	--canonicalize-existing --physical --readlink --quiet t/dd
		ERROR -n -

	EXEC	--canonicalize-existing --physical --readlink --quiet t/d

	EXEC	--canonicalize-existing --physical --readlink --quiet t/z

	EXEC	--canonicalize-existing --physical --readlink --quiet t/dd/g

	EXEC	--canonicalize-existing --physical --readlink --quiet t/z/g

	EXEC	--canonicalize-existing --physical --readlink --quiet t/f

	EXEC	--canonicalize-existing --physical --readlink --quiet t/dd/g

	EXEC	--canonicalize-existing --physical --readlink --quiet t/dd/eee

	EXEC	--canonicalize-existing --physical --readlink --quiet t/a

	EXEC	--canonicalize-existing --physical --readlink --quiet t/b

	EXEC	--canonicalize-existing --physical --readlink --quiet t/d/e

	EXEC	--canonicalize-existing --physical --readlink --quiet t/d/dd

	EXEC	--canonicalize-existing --physical --readlink --quiet t/c

	EXEC	--canonicalize-existing --physical --readlink --quiet t/xxx

	EXEC	--canonicalize-existing --physical --relative t/dd
		OUTPUT - t/dd
		EXIT 0

	EXEC	--canonicalize-existing --physical --relative t/d
		OUTPUT - t/d

	EXEC	--canonicalize-existing --physical --relative t/z
		OUTPUT - t/z

	EXEC	--canonicalize-existing --physical --relative t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-existing --physical --relative t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize-existing --physical --relative t/f
		OUTPUT - t/f

	EXEC	--canonicalize-existing --physical --relative t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-existing --physical --relative t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize-existing --physical --relative t/a
		OUTPUT -
		ERROR - 'realpath: t/a: canonicalization error [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize-existing --physical --relative t/b
		ERROR - 'realpath: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --relative t/d/e
		ERROR - 'realpath: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --relative t/d/dd
		ERROR - 'realpath: t/d/dd: canonicalization error [Too many levels of symbolic links]'

	EXEC	--canonicalize-existing --physical --relative t/c
		ERROR - 'realpath: t/c: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --relative t/xxx
		ERROR - 'realpath: t/xxx: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --relative --quiet t/dd
		OUTPUT - t/dd
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize-existing --physical --relative --quiet t/d
		OUTPUT - t/d

	EXEC	--canonicalize-existing --physical --relative --quiet t/z
		OUTPUT - t/z

	EXEC	--canonicalize-existing --physical --relative --quiet t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-existing --physical --relative --quiet t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize-existing --physical --relative --quiet t/f
		OUTPUT - t/f

	EXEC	--canonicalize-existing --physical --relative --quiet t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-existing --physical --relative --quiet t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize-existing --physical --relative --quiet t/a
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize-existing --physical --relative --quiet t/b

	EXEC	--canonicalize-existing --physical --relative --quiet t/d/e

	EXEC	--canonicalize-existing --physical --relative --quiet t/d/dd

	EXEC	--canonicalize-existing --physical --relative --quiet t/c

	EXEC	--canonicalize-existing --physical --relative --quiet t/xxx

	EXEC	--canonicalize-existing --physical --relative --readlink t/dd
		ERROR - 'realpath: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --relative --readlink t/d
		ERROR - 'realpath: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --relative --readlink t/z
		ERROR - 'realpath: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --relative --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --relative --readlink t/z/g
		ERROR - 'realpath: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --relative --readlink t/f
		ERROR - 'realpath: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --relative --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --relative --readlink t/dd/eee
		ERROR - 'realpath: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --relative --readlink t/a
		ERROR - 'realpath: t/a: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --relative --readlink t/b
		ERROR - 'realpath: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --relative --readlink t/d/e
		ERROR - 'realpath: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --relative --readlink t/d/dd
		ERROR - 'realpath: t/d/dd: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --relative --readlink t/c
		ERROR - 'realpath: t/c: canonicalization error at e [No such file or directory]'

	EXEC	--canonicalize-existing --physical --relative --readlink t/xxx
		ERROR - 'realpath: t/xxx: cannot read link text [No such file or directory]'

	EXEC	--canonicalize-existing --physical --relative --readlink --quiet t/dd
		ERROR -n -

	EXEC	--canonicalize-existing --physical --relative --readlink --quiet t/d

	EXEC	--canonicalize-existing --physical --relative --readlink --quiet t/z

	EXEC	--canonicalize-existing --physical --relative --readlink --quiet t/dd/g

	EXEC	--canonicalize-existing --physical --relative --readlink --quiet t/z/g

	EXEC	--canonicalize-existing --physical --relative --readlink --quiet t/f

	EXEC	--canonicalize-existing --physical --relative --readlink --quiet t/dd/g

	EXEC	--canonicalize-existing --physical --relative --readlink --quiet t/dd/eee

	EXEC	--canonicalize-existing --physical --relative --readlink --quiet t/a

	EXEC	--canonicalize-existing --physical --relative --readlink --quiet t/b

	EXEC	--canonicalize-existing --physical --relative --readlink --quiet t/d/e

	EXEC	--canonicalize-existing --physical --relative --readlink --quiet t/d/dd

	EXEC	--canonicalize-existing --physical --relative --readlink --quiet t/c

	EXEC	--canonicalize-existing --physical --relative --readlink --quiet t/xxx

	EXEC	--canonicalize-missing t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd
		EXIT 0

	EXEC	--canonicalize-missing t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d

	EXEC	--canonicalize-missing t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z

	EXEC	--canonicalize-missing t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize-missing t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z/g

	EXEC	--canonicalize-missing t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/f

	EXEC	--canonicalize-missing t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize-missing t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/eee

	EXEC	--canonicalize-missing t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/f

	EXEC	--canonicalize-missing t/b

	EXEC	--canonicalize-missing t/d/e

	EXEC	--canonicalize-missing t/d/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/dd

	EXEC	--canonicalize-missing t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/f

	EXEC	--canonicalize-missing t/xxx
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/xxx

	EXEC	--canonicalize-missing --quiet t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd

	EXEC	--canonicalize-missing --quiet t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d

	EXEC	--canonicalize-missing --quiet t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z

	EXEC	--canonicalize-missing --quiet t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize-missing --quiet t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z/g

	EXEC	--canonicalize-missing --quiet t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/f

	EXEC	--canonicalize-missing --quiet t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize-missing --quiet t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/eee

	EXEC	--canonicalize-missing --quiet t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/f

	EXEC	--canonicalize-missing --quiet t/b

	EXEC	--canonicalize-missing --quiet t/d/e

	EXEC	--canonicalize-missing --quiet t/d/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/dd

	EXEC	--canonicalize-missing --quiet t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/f

	EXEC	--canonicalize-missing --quiet t/xxx
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/xxx

	EXEC	--canonicalize-missing --readlink t/dd
		OUTPUT -
		ERROR - 'realpath: t/dd: cannot read link text [Invalid argument]'
		EXIT 1

	EXEC	--canonicalize-missing --readlink t/d
		ERROR - 'realpath: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --readlink t/z
		ERROR - 'realpath: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --readlink t/z/g
		ERROR - 'realpath: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --readlink t/f
		ERROR - 'realpath: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --readlink t/dd/eee
		ERROR - 'realpath: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --readlink t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/b
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize-missing --readlink t/b
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/c

	EXEC	--canonicalize-missing --readlink t/d/e
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/f

	EXEC	--canonicalize-missing --readlink t/d/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/dd

	EXEC	--canonicalize-missing --readlink t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/d/e

	EXEC	--canonicalize-missing --readlink t/xxx
		OUTPUT -
		ERROR - 'realpath: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize-missing --readlink --quiet t/dd
		ERROR -n -

	EXEC	--canonicalize-missing --readlink --quiet t/d

	EXEC	--canonicalize-missing --readlink --quiet t/z

	EXEC	--canonicalize-missing --readlink --quiet t/dd/g

	EXEC	--canonicalize-missing --readlink --quiet t/z/g

	EXEC	--canonicalize-missing --readlink --quiet t/f

	EXEC	--canonicalize-missing --readlink --quiet t/dd/g

	EXEC	--canonicalize-missing --readlink --quiet t/dd/eee

	EXEC	--canonicalize-missing --readlink --quiet t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/b
		EXIT 0

	EXEC	--canonicalize-missing --readlink --quiet t/b
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/c

	EXEC	--canonicalize-missing --readlink --quiet t/d/e
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/f

	EXEC	--canonicalize-missing --readlink --quiet t/d/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/dd

	EXEC	--canonicalize-missing --readlink --quiet t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/d/e

	EXEC	--canonicalize-missing --readlink --quiet t/xxx
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize-missing --relative t/dd
		OUTPUT - t/dd
		EXIT 0

	EXEC	--canonicalize-missing --relative t/d
		OUTPUT - t/d

	EXEC	--canonicalize-missing --relative t/z
		OUTPUT - t/z

	EXEC	--canonicalize-missing --relative t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-missing --relative t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize-missing --relative t/f
		OUTPUT - t/f

	EXEC	--canonicalize-missing --relative t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-missing --relative t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize-missing --relative t/a
		OUTPUT - t/d/f

	EXEC	--canonicalize-missing --relative t/b

	EXEC	--canonicalize-missing --relative t/d/e

	EXEC	--canonicalize-missing --relative t/d/dd
		OUTPUT - t/d/dd

	EXEC	--canonicalize-missing --relative t/c
		OUTPUT - t/d/f

	EXEC	--canonicalize-missing --relative t/xxx
		OUTPUT - t/xxx

	EXEC	--canonicalize-missing --relative --quiet t/dd
		OUTPUT - t/dd

	EXEC	--canonicalize-missing --relative --quiet t/d
		OUTPUT - t/d

	EXEC	--canonicalize-missing --relative --quiet t/z
		OUTPUT - t/z

	EXEC	--canonicalize-missing --relative --quiet t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-missing --relative --quiet t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize-missing --relative --quiet t/f
		OUTPUT - t/f

	EXEC	--canonicalize-missing --relative --quiet t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-missing --relative --quiet t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize-missing --relative --quiet t/a
		OUTPUT - t/d/f

	EXEC	--canonicalize-missing --relative --quiet t/b

	EXEC	--canonicalize-missing --relative --quiet t/d/e

	EXEC	--canonicalize-missing --relative --quiet t/d/dd
		OUTPUT - t/d/dd

	EXEC	--canonicalize-missing --relative --quiet t/c
		OUTPUT - t/d/f

	EXEC	--canonicalize-missing --relative --quiet t/xxx
		OUTPUT - t/xxx

	EXEC	--canonicalize-missing --relative --readlink t/dd
		OUTPUT -
		ERROR - 'realpath: t/dd: cannot read link text [Invalid argument]'
		EXIT 1

	EXEC	--canonicalize-missing --relative --readlink t/d
		ERROR - 'realpath: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --relative --readlink t/z
		ERROR - 'realpath: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --relative --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --relative --readlink t/z/g
		ERROR - 'realpath: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --relative --readlink t/f
		ERROR - 'realpath: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --relative --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --relative --readlink t/dd/eee
		ERROR - 'realpath: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --relative --readlink t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize-missing --relative --readlink t/b
		OUTPUT - c

	EXEC	--canonicalize-missing --relative --readlink t/d/e
		OUTPUT - f

	EXEC	--canonicalize-missing --relative --readlink t/d/dd
		OUTPUT - dd

	EXEC	--canonicalize-missing --relative --readlink t/c
		OUTPUT - d/e

	EXEC	--canonicalize-missing --relative --readlink t/xxx
		OUTPUT -
		ERROR - 'realpath: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize-missing --relative --readlink --quiet t/dd
		ERROR -n -

	EXEC	--canonicalize-missing --relative --readlink --quiet t/d

	EXEC	--canonicalize-missing --relative --readlink --quiet t/z

	EXEC	--canonicalize-missing --relative --readlink --quiet t/dd/g

	EXEC	--canonicalize-missing --relative --readlink --quiet t/z/g

	EXEC	--canonicalize-missing --relative --readlink --quiet t/f

	EXEC	--canonicalize-missing --relative --readlink --quiet t/dd/g

	EXEC	--canonicalize-missing --relative --readlink --quiet t/dd/eee

	EXEC	--canonicalize-missing --relative --readlink --quiet t/a
		OUTPUT - b
		EXIT 0

	EXEC	--canonicalize-missing --relative --readlink --quiet t/b
		OUTPUT - c

	EXEC	--canonicalize-missing --relative --readlink --quiet t/d/e
		OUTPUT - f

	EXEC	--canonicalize-missing --relative --readlink --quiet t/d/dd
		OUTPUT - dd

	EXEC	--canonicalize-missing --relative --readlink --quiet t/c
		OUTPUT - d/e

	EXEC	--canonicalize-missing --relative --readlink --quiet t/xxx
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize-missing --logical t/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd
		EXIT 0

	EXEC	--canonicalize-missing --logical t/d
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d

	EXEC	--canonicalize-missing --logical t/z
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z

	EXEC	--canonicalize-missing --logical t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize-missing --logical t/z/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z/g

	EXEC	--canonicalize-missing --logical t/f
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/f

	EXEC	--canonicalize-missing --logical t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize-missing --logical t/dd/eee
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/eee

	EXEC	--canonicalize-missing --logical t/a
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/a

	EXEC	--canonicalize-missing --logical t/b
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/b

	EXEC	--canonicalize-missing --logical t/d/e
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/e

	EXEC	--canonicalize-missing --logical t/d/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/dd

	EXEC	--canonicalize-missing --logical t/c
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/c

	EXEC	--canonicalize-missing --logical t/xxx
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/xxx

	EXEC	--canonicalize-missing --logical --quiet t/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd

	EXEC	--canonicalize-missing --logical --quiet t/d
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d

	EXEC	--canonicalize-missing --logical --quiet t/z
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z

	EXEC	--canonicalize-missing --logical --quiet t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize-missing --logical --quiet t/z/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z/g

	EXEC	--canonicalize-missing --logical --quiet t/f
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/f

	EXEC	--canonicalize-missing --logical --quiet t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize-missing --logical --quiet t/dd/eee
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/eee

	EXEC	--canonicalize-missing --logical --quiet t/a
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/a

	EXEC	--canonicalize-missing --logical --quiet t/b
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/b

	EXEC	--canonicalize-missing --logical --quiet t/d/e
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/e

	EXEC	--canonicalize-missing --logical --quiet t/d/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/dd

	EXEC	--canonicalize-missing --logical --quiet t/c
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/c

	EXEC	--canonicalize-missing --logical --quiet t/xxx
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/xxx

	EXEC	--canonicalize-missing --logical --readlink t/dd
		OUTPUT -
		ERROR - 'realpath: t/dd: cannot read link text [Invalid argument]'
		EXIT 1

	EXEC	--canonicalize-missing --logical --readlink t/d
		ERROR - 'realpath: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --readlink t/z
		ERROR - 'realpath: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --readlink t/z/g
		ERROR - 'realpath: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --readlink t/f
		ERROR - 'realpath: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --readlink t/dd/eee
		ERROR - 'realpath: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --readlink t/a
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/b
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize-missing --logical --readlink t/b
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/c

	EXEC	--canonicalize-missing --logical --readlink t/d/e
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/f

	EXEC	--canonicalize-missing --logical --readlink t/d/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/dd

	EXEC	--canonicalize-missing --logical --readlink t/c
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/d/e

	EXEC	--canonicalize-missing --logical --readlink t/xxx
		OUTPUT -
		ERROR - 'realpath: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize-missing --logical --readlink --quiet t/dd
		ERROR -n -

	EXEC	--canonicalize-missing --logical --readlink --quiet t/d

	EXEC	--canonicalize-missing --logical --readlink --quiet t/z

	EXEC	--canonicalize-missing --logical --readlink --quiet t/dd/g

	EXEC	--canonicalize-missing --logical --readlink --quiet t/z/g

	EXEC	--canonicalize-missing --logical --readlink --quiet t/f

	EXEC	--canonicalize-missing --logical --readlink --quiet t/dd/g

	EXEC	--canonicalize-missing --logical --readlink --quiet t/dd/eee

	EXEC	--canonicalize-missing --logical --readlink --quiet t/a
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/b
		EXIT 0

	EXEC	--canonicalize-missing --logical --readlink --quiet t/b
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/c

	EXEC	--canonicalize-missing --logical --readlink --quiet t/d/e
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/f

	EXEC	--canonicalize-missing --logical --readlink --quiet t/d/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/dd

	EXEC	--canonicalize-missing --logical --readlink --quiet t/c
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/d/e

	EXEC	--canonicalize-missing --logical --readlink --quiet t/xxx
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize-missing --logical --relative t/dd
		OUTPUT - t/dd
		EXIT 0

	EXEC	--canonicalize-missing --logical --relative t/d
		OUTPUT - t/d

	EXEC	--canonicalize-missing --logical --relative t/z
		OUTPUT - t/z

	EXEC	--canonicalize-missing --logical --relative t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-missing --logical --relative t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize-missing --logical --relative t/f
		OUTPUT - t/f

	EXEC	--canonicalize-missing --logical --relative t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-missing --logical --relative t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize-missing --logical --relative t/a
		OUTPUT - t/a

	EXEC	--canonicalize-missing --logical --relative t/b
		OUTPUT - t/b

	EXEC	--canonicalize-missing --logical --relative t/d/e
		OUTPUT - t/d/e

	EXEC	--canonicalize-missing --logical --relative t/d/dd
		OUTPUT - t/d/dd

	EXEC	--canonicalize-missing --logical --relative t/c
		OUTPUT - t/c

	EXEC	--canonicalize-missing --logical --relative t/xxx
		OUTPUT - t/xxx

	EXEC	--canonicalize-missing --logical --relative --quiet t/dd
		OUTPUT - t/dd

	EXEC	--canonicalize-missing --logical --relative --quiet t/d
		OUTPUT - t/d

	EXEC	--canonicalize-missing --logical --relative --quiet t/z
		OUTPUT - t/z

	EXEC	--canonicalize-missing --logical --relative --quiet t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-missing --logical --relative --quiet t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize-missing --logical --relative --quiet t/f
		OUTPUT - t/f

	EXEC	--canonicalize-missing --logical --relative --quiet t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-missing --logical --relative --quiet t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize-missing --logical --relative --quiet t/a
		OUTPUT - t/a

	EXEC	--canonicalize-missing --logical --relative --quiet t/b
		OUTPUT - t/b

	EXEC	--canonicalize-missing --logical --relative --quiet t/d/e
		OUTPUT - t/d/e

	EXEC	--canonicalize-missing --logical --relative --quiet t/d/dd
		OUTPUT - t/d/dd

	EXEC	--canonicalize-missing --logical --relative --quiet t/c
		OUTPUT - t/c

	EXEC	--canonicalize-missing --logical --relative --quiet t/xxx
		OUTPUT - t/xxx

	EXEC	--canonicalize-missing --logical --relative --readlink t/dd
		OUTPUT -
		ERROR - 'realpath: t/dd: cannot read link text [Invalid argument]'
		EXIT 1

	EXEC	--canonicalize-missing --logical --relative --readlink t/d
		ERROR - 'realpath: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --relative --readlink t/z
		ERROR - 'realpath: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --relative --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --relative --readlink t/z/g
		ERROR - 'realpath: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --relative --readlink t/f
		ERROR - 'realpath: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --relative --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --relative --readlink t/dd/eee
		ERROR - 'realpath: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --relative --readlink t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize-missing --logical --relative --readlink t/b
		OUTPUT - c

	EXEC	--canonicalize-missing --logical --relative --readlink t/d/e
		OUTPUT - f

	EXEC	--canonicalize-missing --logical --relative --readlink t/d/dd
		OUTPUT - dd

	EXEC	--canonicalize-missing --logical --relative --readlink t/c
		OUTPUT - d/e

	EXEC	--canonicalize-missing --logical --relative --readlink t/xxx
		OUTPUT -
		ERROR - 'realpath: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize-missing --logical --relative --readlink --quiet t/dd
		ERROR -n -

	EXEC	--canonicalize-missing --logical --relative --readlink --quiet t/d

	EXEC	--canonicalize-missing --logical --relative --readlink --quiet t/z

	EXEC	--canonicalize-missing --logical --relative --readlink --quiet t/dd/g

	EXEC	--canonicalize-missing --logical --relative --readlink --quiet t/z/g

	EXEC	--canonicalize-missing --logical --relative --readlink --quiet t/f

	EXEC	--canonicalize-missing --logical --relative --readlink --quiet t/dd/g

	EXEC	--canonicalize-missing --logical --relative --readlink --quiet t/dd/eee

	EXEC	--canonicalize-missing --logical --relative --readlink --quiet t/a
		OUTPUT - b
		EXIT 0

	EXEC	--canonicalize-missing --logical --relative --readlink --quiet t/b
		OUTPUT - c

	EXEC	--canonicalize-missing --logical --relative --readlink --quiet t/d/e
		OUTPUT - f

	EXEC	--canonicalize-missing --logical --relative --readlink --quiet t/d/dd
		OUTPUT - dd

	EXEC	--canonicalize-missing --logical --relative --readlink --quiet t/c
		OUTPUT - d/e

	EXEC	--canonicalize-missing --logical --relative --readlink --quiet t/xxx
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize-missing --physical t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd
		EXIT 0

	EXEC	--canonicalize-missing --physical t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d

	EXEC	--canonicalize-missing --physical t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z

	EXEC	--canonicalize-missing --physical t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize-missing --physical t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z/g

	EXEC	--canonicalize-missing --physical t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/f

	EXEC	--canonicalize-missing --physical t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize-missing --physical t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/eee

	EXEC	--canonicalize-missing --physical t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/f

	EXEC	--canonicalize-missing --physical t/b

	EXEC	--canonicalize-missing --physical t/d/e

	EXEC	--canonicalize-missing --physical t/d/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/dd

	EXEC	--canonicalize-missing --physical t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/f

	EXEC	--canonicalize-missing --physical t/xxx
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/xxx

	EXEC	--canonicalize-missing --physical --quiet t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd

	EXEC	--canonicalize-missing --physical --quiet t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d

	EXEC	--canonicalize-missing --physical --quiet t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z

	EXEC	--canonicalize-missing --physical --quiet t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize-missing --physical --quiet t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/z/g

	EXEC	--canonicalize-missing --physical --quiet t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/f

	EXEC	--canonicalize-missing --physical --quiet t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/g

	EXEC	--canonicalize-missing --physical --quiet t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/dd/eee

	EXEC	--canonicalize-missing --physical --quiet t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/f

	EXEC	--canonicalize-missing --physical --quiet t/b

	EXEC	--canonicalize-missing --physical --quiet t/d/e

	EXEC	--canonicalize-missing --physical --quiet t/d/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/dd

	EXEC	--canonicalize-missing --physical --quiet t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/d/f

	EXEC	--canonicalize-missing --physical --quiet t/xxx
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/t/xxx

	EXEC	--canonicalize-missing --physical --readlink t/dd
		OUTPUT -
		ERROR - 'realpath: t/dd: cannot read link text [Invalid argument]'
		EXIT 1

	EXEC	--canonicalize-missing --physical --readlink t/d
		ERROR - 'realpath: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --readlink t/z
		ERROR - 'realpath: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --readlink t/z/g
		ERROR - 'realpath: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --readlink t/f
		ERROR - 'realpath: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --readlink t/dd/eee
		ERROR - 'realpath: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --readlink t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/b
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize-missing --physical --readlink t/b
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/c

	EXEC	--canonicalize-missing --physical --readlink t/d/e
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/f

	EXEC	--canonicalize-missing --physical --readlink t/d/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/dd

	EXEC	--canonicalize-missing --physical --readlink t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/d/e

	EXEC	--canonicalize-missing --physical --readlink t/xxx
		OUTPUT -
		ERROR - 'realpath: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize-missing --physical --readlink --quiet t/dd
		ERROR -n -

	EXEC	--canonicalize-missing --physical --readlink --quiet t/d

	EXEC	--canonicalize-missing --physical --readlink --quiet t/z

	EXEC	--canonicalize-missing --physical --readlink --quiet t/dd/g

	EXEC	--canonicalize-missing --physical --readlink --quiet t/z/g

	EXEC	--canonicalize-missing --physical --readlink --quiet t/f

	EXEC	--canonicalize-missing --physical --readlink --quiet t/dd/g

	EXEC	--canonicalize-missing --physical --readlink --quiet t/dd/eee

	EXEC	--canonicalize-missing --physical --readlink --quiet t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/b
		EXIT 0

	EXEC	--canonicalize-missing --physical --readlink --quiet t/b
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/c

	EXEC	--canonicalize-missing --physical --readlink --quiet t/d/e
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/f

	EXEC	--canonicalize-missing --physical --readlink --quiet t/d/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/dd

	EXEC	--canonicalize-missing --physical --readlink --quiet t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/realpath.tmp/d/e

	EXEC	--canonicalize-missing --physical --readlink --quiet t/xxx
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize-missing --physical --relative t/dd
		OUTPUT - t/dd
		EXIT 0

	EXEC	--canonicalize-missing --physical --relative t/d
		OUTPUT - t/d

	EXEC	--canonicalize-missing --physical --relative t/z
		OUTPUT - t/z

	EXEC	--canonicalize-missing --physical --relative t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-missing --physical --relative t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize-missing --physical --relative t/f
		OUTPUT - t/f

	EXEC	--canonicalize-missing --physical --relative t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-missing --physical --relative t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize-missing --physical --relative t/a
		OUTPUT - t/d/f

	EXEC	--canonicalize-missing --physical --relative t/b

	EXEC	--canonicalize-missing --physical --relative t/d/e

	EXEC	--canonicalize-missing --physical --relative t/d/dd
		OUTPUT - t/d/dd

	EXEC	--canonicalize-missing --physical --relative t/c
		OUTPUT - t/d/f

	EXEC	--canonicalize-missing --physical --relative t/xxx
		OUTPUT - t/xxx

	EXEC	--canonicalize-missing --physical --relative --quiet t/dd
		OUTPUT - t/dd

	EXEC	--canonicalize-missing --physical --relative --quiet t/d
		OUTPUT - t/d

	EXEC	--canonicalize-missing --physical --relative --quiet t/z
		OUTPUT - t/z

	EXEC	--canonicalize-missing --physical --relative --quiet t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-missing --physical --relative --quiet t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize-missing --physical --relative --quiet t/f
		OUTPUT - t/f

	EXEC	--canonicalize-missing --physical --relative --quiet t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-missing --physical --relative --quiet t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize-missing --physical --relative --quiet t/a
		OUTPUT - t/d/f

	EXEC	--canonicalize-missing --physical --relative --quiet t/b

	EXEC	--canonicalize-missing --physical --relative --quiet t/d/e

	EXEC	--canonicalize-missing --physical --relative --quiet t/d/dd
		OUTPUT - t/d/dd

	EXEC	--canonicalize-missing --physical --relative --quiet t/c
		OUTPUT - t/d/f

	EXEC	--canonicalize-missing --physical --relative --quiet t/xxx
		OUTPUT - t/xxx

	EXEC	--canonicalize-missing --physical --relative --readlink t/dd
		OUTPUT -
		ERROR - 'realpath: t/dd: cannot read link text [Invalid argument]'
		EXIT 1

	EXEC	--canonicalize-missing --physical --relative --readlink t/d
		ERROR - 'realpath: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --relative --readlink t/z
		ERROR - 'realpath: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --relative --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --relative --readlink t/z/g
		ERROR - 'realpath: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --relative --readlink t/f
		ERROR - 'realpath: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --relative --readlink t/dd/g
		ERROR - 'realpath: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --relative --readlink t/dd/eee
		ERROR - 'realpath: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --relative --readlink t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize-missing --physical --relative --readlink t/b
		OUTPUT - c

	EXEC	--canonicalize-missing --physical --relative --readlink t/d/e
		OUTPUT - f

	EXEC	--canonicalize-missing --physical --relative --readlink t/d/dd
		OUTPUT - dd

	EXEC	--canonicalize-missing --physical --relative --readlink t/c
		OUTPUT - d/e

	EXEC	--canonicalize-missing --physical --relative --readlink t/xxx
		OUTPUT -
		ERROR - 'realpath: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize-missing --physical --relative --readlink --quiet t/dd
		ERROR -n -

	EXEC	--canonicalize-missing --physical --relative --readlink --quiet t/d

	EXEC	--canonicalize-missing --physical --relative --readlink --quiet t/z

	EXEC	--canonicalize-missing --physical --relative --readlink --quiet t/dd/g

	EXEC	--canonicalize-missing --physical --relative --readlink --quiet t/z/g

	EXEC	--canonicalize-missing --physical --relative --readlink --quiet t/f

	EXEC	--canonicalize-missing --physical --relative --readlink --quiet t/dd/g

	EXEC	--canonicalize-missing --physical --relative --readlink --quiet t/dd/eee

	EXEC	--canonicalize-missing --physical --relative --readlink --quiet t/a
		OUTPUT - b
		EXIT 0

	EXEC	--canonicalize-missing --physical --relative --readlink --quiet t/b
		OUTPUT - c

	EXEC	--canonicalize-missing --physical --relative --readlink --quiet t/d/e
		OUTPUT - f

	EXEC	--canonicalize-missing --physical --relative --readlink --quiet t/d/dd
		OUTPUT - dd

	EXEC	--canonicalize-missing --physical --relative --readlink --quiet t/c
		OUTPUT - d/e

	EXEC	--canonicalize-missing --physical --relative --readlink --quiet t/xxx
		OUTPUT -
		EXIT 1
