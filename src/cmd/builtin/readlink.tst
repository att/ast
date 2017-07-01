# : : generated from /home/gsf/src/cmd/builtin/readlink.rt by mktest : : #

# regression tests for the readlink command

UNIT readlink

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
		OUTPUT -
		ERROR -n -
		EXIT 1

	EXEC	t/d

	EXEC	t/z

	EXEC	t/dd/g

	EXEC	t/z/g

	EXEC	t/f

	EXEC	t/dd/g

	EXEC	t/dd/eee

	EXEC	t/a
		OUTPUT - b
		EXIT 0

	EXEC	t/b
		OUTPUT - c

	EXEC	t/d/e
		OUTPUT - f

	EXEC	t/d/dd
		OUTPUT - dd

	EXEC	t/c
		OUTPUT - d/e

	EXEC	t/xxx
		OUTPUT -
		EXIT 1

	EXEC	--verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--verbose t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--verbose t/b
		OUTPUT - c

	EXEC	--verbose t/d/e
		OUTPUT - f

	EXEC	--verbose t/d/dd
		OUTPUT - dd

	EXEC	--verbose t/c
		OUTPUT - d/e

	EXEC	--verbose t/xxx
		OUTPUT -
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--readlink t/dd
		ERROR -n -

	EXEC	--readlink t/d

	EXEC	--readlink t/z

	EXEC	--readlink t/dd/g

	EXEC	--readlink t/z/g

	EXEC	--readlink t/f

	EXEC	--readlink t/dd/g

	EXEC	--readlink t/dd/eee

	EXEC	--readlink t/a
		OUTPUT - b
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
		EXIT 1

	EXEC	--readlink --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--readlink --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--readlink --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--readlink --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--readlink --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--readlink --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--readlink --verbose t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--readlink --verbose t/b
		OUTPUT - c

	EXEC	--readlink --verbose t/d/e
		OUTPUT - f

	EXEC	--readlink --verbose t/d/dd
		OUTPUT - dd

	EXEC	--readlink --verbose t/c
		OUTPUT - d/e

	EXEC	--readlink --verbose t/xxx
		OUTPUT -
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--relative t/dd
		ERROR -n -

	EXEC	--relative t/d

	EXEC	--relative t/z

	EXEC	--relative t/dd/g

	EXEC	--relative t/z/g

	EXEC	--relative t/f

	EXEC	--relative t/dd/g

	EXEC	--relative t/dd/eee

	EXEC	--relative t/a
		OUTPUT - b
		EXIT 0

	EXEC	--relative t/b
		OUTPUT - c

	EXEC	--relative t/d/e
		OUTPUT - f

	EXEC	--relative t/d/dd
		OUTPUT - dd

	EXEC	--relative t/c
		OUTPUT - d/e

	EXEC	--relative t/xxx
		OUTPUT -
		EXIT 1

	EXEC	--relative --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--relative --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--relative --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--relative --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--relative --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--relative --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--relative --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--relative --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--relative --verbose t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--relative --verbose t/b
		OUTPUT - c

	EXEC	--relative --verbose t/d/e
		OUTPUT - f

	EXEC	--relative --verbose t/d/dd
		OUTPUT - dd

	EXEC	--relative --verbose t/c
		OUTPUT - d/e

	EXEC	--relative --verbose t/xxx
		OUTPUT -
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--relative --readlink t/dd
		ERROR -n -

	EXEC	--relative --readlink t/d

	EXEC	--relative --readlink t/z

	EXEC	--relative --readlink t/dd/g

	EXEC	--relative --readlink t/z/g

	EXEC	--relative --readlink t/f

	EXEC	--relative --readlink t/dd/g

	EXEC	--relative --readlink t/dd/eee

	EXEC	--relative --readlink t/a
		OUTPUT - b
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
		EXIT 1

	EXEC	--relative --readlink --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--relative --readlink --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--relative --readlink --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--relative --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--relative --readlink --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--relative --readlink --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--relative --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--relative --readlink --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--relative --readlink --verbose t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--relative --readlink --verbose t/b
		OUTPUT - c

	EXEC	--relative --readlink --verbose t/d/e
		OUTPUT - f

	EXEC	--relative --readlink --verbose t/d/dd
		OUTPUT - dd

	EXEC	--relative --readlink --verbose t/c
		OUTPUT - d/e

	EXEC	--relative --readlink --verbose t/xxx
		OUTPUT -
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--logical t/dd
		ERROR -n -

	EXEC	--logical t/d

	EXEC	--logical t/z

	EXEC	--logical t/dd/g

	EXEC	--logical t/z/g

	EXEC	--logical t/f

	EXEC	--logical t/dd/g

	EXEC	--logical t/dd/eee

	EXEC	--logical t/a
		OUTPUT - b
		EXIT 0

	EXEC	--logical t/b
		OUTPUT - c

	EXEC	--logical t/d/e
		OUTPUT - f

	EXEC	--logical t/d/dd
		OUTPUT - dd

	EXEC	--logical t/c
		OUTPUT - d/e

	EXEC	--logical t/xxx
		OUTPUT -
		EXIT 1

	EXEC	--logical --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--logical --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--logical --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--logical --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--logical --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--logical --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--logical --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--logical --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--logical --verbose t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--logical --verbose t/b
		OUTPUT - c

	EXEC	--logical --verbose t/d/e
		OUTPUT - f

	EXEC	--logical --verbose t/d/dd
		OUTPUT - dd

	EXEC	--logical --verbose t/c
		OUTPUT - d/e

	EXEC	--logical --verbose t/xxx
		OUTPUT -
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--logical --readlink t/dd
		ERROR -n -

	EXEC	--logical --readlink t/d

	EXEC	--logical --readlink t/z

	EXEC	--logical --readlink t/dd/g

	EXEC	--logical --readlink t/z/g

	EXEC	--logical --readlink t/f

	EXEC	--logical --readlink t/dd/g

	EXEC	--logical --readlink t/dd/eee

	EXEC	--logical --readlink t/a
		OUTPUT - b
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
		EXIT 1

	EXEC	--logical --readlink --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--logical --readlink --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--logical --readlink --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--logical --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--logical --readlink --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--logical --readlink --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--logical --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--logical --readlink --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--logical --readlink --verbose t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--logical --readlink --verbose t/b
		OUTPUT - c

	EXEC	--logical --readlink --verbose t/d/e
		OUTPUT - f

	EXEC	--logical --readlink --verbose t/d/dd
		OUTPUT - dd

	EXEC	--logical --readlink --verbose t/c
		OUTPUT - d/e

	EXEC	--logical --readlink --verbose t/xxx
		OUTPUT -
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--logical --relative t/dd
		ERROR -n -

	EXEC	--logical --relative t/d

	EXEC	--logical --relative t/z

	EXEC	--logical --relative t/dd/g

	EXEC	--logical --relative t/z/g

	EXEC	--logical --relative t/f

	EXEC	--logical --relative t/dd/g

	EXEC	--logical --relative t/dd/eee

	EXEC	--logical --relative t/a
		OUTPUT - b
		EXIT 0

	EXEC	--logical --relative t/b
		OUTPUT - c

	EXEC	--logical --relative t/d/e
		OUTPUT - f

	EXEC	--logical --relative t/d/dd
		OUTPUT - dd

	EXEC	--logical --relative t/c
		OUTPUT - d/e

	EXEC	--logical --relative t/xxx
		OUTPUT -
		EXIT 1

	EXEC	--logical --relative --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--logical --relative --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--logical --relative --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--logical --relative --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--logical --relative --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--logical --relative --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--logical --relative --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--logical --relative --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--logical --relative --verbose t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--logical --relative --verbose t/b
		OUTPUT - c

	EXEC	--logical --relative --verbose t/d/e
		OUTPUT - f

	EXEC	--logical --relative --verbose t/d/dd
		OUTPUT - dd

	EXEC	--logical --relative --verbose t/c
		OUTPUT - d/e

	EXEC	--logical --relative --verbose t/xxx
		OUTPUT -
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--logical --relative --readlink t/dd
		ERROR -n -

	EXEC	--logical --relative --readlink t/d

	EXEC	--logical --relative --readlink t/z

	EXEC	--logical --relative --readlink t/dd/g

	EXEC	--logical --relative --readlink t/z/g

	EXEC	--logical --relative --readlink t/f

	EXEC	--logical --relative --readlink t/dd/g

	EXEC	--logical --relative --readlink t/dd/eee

	EXEC	--logical --relative --readlink t/a
		OUTPUT - b
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
		EXIT 1

	EXEC	--logical --relative --readlink --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--logical --relative --readlink --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--logical --relative --readlink --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--logical --relative --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--logical --relative --readlink --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--logical --relative --readlink --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--logical --relative --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--logical --relative --readlink --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--logical --relative --readlink --verbose t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--logical --relative --readlink --verbose t/b
		OUTPUT - c

	EXEC	--logical --relative --readlink --verbose t/d/e
		OUTPUT - f

	EXEC	--logical --relative --readlink --verbose t/d/dd
		OUTPUT - dd

	EXEC	--logical --relative --readlink --verbose t/c
		OUTPUT - d/e

	EXEC	--logical --relative --readlink --verbose t/xxx
		OUTPUT -
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--physical t/dd
		ERROR -n -

	EXEC	--physical t/d

	EXEC	--physical t/z

	EXEC	--physical t/dd/g

	EXEC	--physical t/z/g

	EXEC	--physical t/f

	EXEC	--physical t/dd/g

	EXEC	--physical t/dd/eee

	EXEC	--physical t/a
		OUTPUT - b
		EXIT 0

	EXEC	--physical t/b
		OUTPUT - c

	EXEC	--physical t/d/e
		OUTPUT - f

	EXEC	--physical t/d/dd
		OUTPUT - dd

	EXEC	--physical t/c
		OUTPUT - d/e

	EXEC	--physical t/xxx
		OUTPUT -
		EXIT 1

	EXEC	--physical --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--physical --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--physical --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--physical --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--physical --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--physical --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--physical --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--physical --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--physical --verbose t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--physical --verbose t/b
		OUTPUT - c

	EXEC	--physical --verbose t/d/e
		OUTPUT - f

	EXEC	--physical --verbose t/d/dd
		OUTPUT - dd

	EXEC	--physical --verbose t/c
		OUTPUT - d/e

	EXEC	--physical --verbose t/xxx
		OUTPUT -
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--physical --readlink t/dd
		ERROR -n -

	EXEC	--physical --readlink t/d

	EXEC	--physical --readlink t/z

	EXEC	--physical --readlink t/dd/g

	EXEC	--physical --readlink t/z/g

	EXEC	--physical --readlink t/f

	EXEC	--physical --readlink t/dd/g

	EXEC	--physical --readlink t/dd/eee

	EXEC	--physical --readlink t/a
		OUTPUT - b
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
		EXIT 1

	EXEC	--physical --readlink --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--physical --readlink --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--physical --readlink --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--physical --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--physical --readlink --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--physical --readlink --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--physical --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--physical --readlink --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--physical --readlink --verbose t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--physical --readlink --verbose t/b
		OUTPUT - c

	EXEC	--physical --readlink --verbose t/d/e
		OUTPUT - f

	EXEC	--physical --readlink --verbose t/d/dd
		OUTPUT - dd

	EXEC	--physical --readlink --verbose t/c
		OUTPUT - d/e

	EXEC	--physical --readlink --verbose t/xxx
		OUTPUT -
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--physical --relative t/dd
		ERROR -n -

	EXEC	--physical --relative t/d

	EXEC	--physical --relative t/z

	EXEC	--physical --relative t/dd/g

	EXEC	--physical --relative t/z/g

	EXEC	--physical --relative t/f

	EXEC	--physical --relative t/dd/g

	EXEC	--physical --relative t/dd/eee

	EXEC	--physical --relative t/a
		OUTPUT - b
		EXIT 0

	EXEC	--physical --relative t/b
		OUTPUT - c

	EXEC	--physical --relative t/d/e
		OUTPUT - f

	EXEC	--physical --relative t/d/dd
		OUTPUT - dd

	EXEC	--physical --relative t/c
		OUTPUT - d/e

	EXEC	--physical --relative t/xxx
		OUTPUT -
		EXIT 1

	EXEC	--physical --relative --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--physical --relative --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--physical --relative --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--physical --relative --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--physical --relative --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--physical --relative --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--physical --relative --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--physical --relative --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--physical --relative --verbose t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--physical --relative --verbose t/b
		OUTPUT - c

	EXEC	--physical --relative --verbose t/d/e
		OUTPUT - f

	EXEC	--physical --relative --verbose t/d/dd
		OUTPUT - dd

	EXEC	--physical --relative --verbose t/c
		OUTPUT - d/e

	EXEC	--physical --relative --verbose t/xxx
		OUTPUT -
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--physical --relative --readlink t/dd
		ERROR -n -

	EXEC	--physical --relative --readlink t/d

	EXEC	--physical --relative --readlink t/z

	EXEC	--physical --relative --readlink t/dd/g

	EXEC	--physical --relative --readlink t/z/g

	EXEC	--physical --relative --readlink t/f

	EXEC	--physical --relative --readlink t/dd/g

	EXEC	--physical --relative --readlink t/dd/eee

	EXEC	--physical --relative --readlink t/a
		OUTPUT - b
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
		EXIT 1

	EXEC	--physical --relative --readlink --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--physical --relative --readlink --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--physical --relative --readlink --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--physical --relative --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--physical --relative --readlink --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--physical --relative --readlink --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--physical --relative --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--physical --relative --readlink --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--physical --relative --readlink --verbose t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--physical --relative --readlink --verbose t/b
		OUTPUT - c

	EXEC	--physical --relative --readlink --verbose t/d/e
		OUTPUT - f

	EXEC	--physical --relative --readlink --verbose t/d/dd
		OUTPUT - dd

	EXEC	--physical --relative --readlink --verbose t/c
		OUTPUT - d/e

	EXEC	--physical --relative --readlink --verbose t/xxx
		OUTPUT -
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d

	EXEC	--canonicalize t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z

	EXEC	--canonicalize t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z/g

	EXEC	--canonicalize t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/f

	EXEC	--canonicalize t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/eee

	EXEC	--canonicalize t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/f

	EXEC	--canonicalize t/b

	EXEC	--canonicalize t/d/e

	EXEC	--canonicalize t/d/dd
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/f
		EXIT 0

	EXEC	--canonicalize t/xxx
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/xxx

	EXEC	--canonicalize --verbose t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd

	EXEC	--canonicalize --verbose t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d

	EXEC	--canonicalize --verbose t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z

	EXEC	--canonicalize --verbose t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize --verbose t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z/g

	EXEC	--canonicalize --verbose t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/f

	EXEC	--canonicalize --verbose t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize --verbose t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/eee

	EXEC	--canonicalize --verbose t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/f

	EXEC	--canonicalize --verbose t/b

	EXEC	--canonicalize --verbose t/d/e

	EXEC	--canonicalize --verbose t/d/dd
		OUTPUT -
		ERROR - 'readlink: t/d/dd: canonicalization error [Too many levels of symbolic links]'
		EXIT 1

	EXEC	--canonicalize --verbose t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/f
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize --verbose t/xxx
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/xxx

	EXEC	--canonicalize --readlink t/dd
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize --readlink t/d

	EXEC	--canonicalize --readlink t/z

	EXEC	--canonicalize --readlink t/dd/g

	EXEC	--canonicalize --readlink t/z/g

	EXEC	--canonicalize --readlink t/f

	EXEC	--canonicalize --readlink t/dd/g

	EXEC	--canonicalize --readlink t/dd/eee

	EXEC	--canonicalize --readlink t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/b
		EXIT 0

	EXEC	--canonicalize --readlink t/b
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/c

	EXEC	--canonicalize --readlink t/d/e
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/f

	EXEC	--canonicalize --readlink t/d/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/dd

	EXEC	--canonicalize --readlink t/c
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize --readlink t/xxx

	EXEC	--canonicalize --readlink --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --readlink --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --readlink --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --readlink --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --readlink --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --readlink --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --readlink --verbose t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/b
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize --readlink --verbose t/b
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/c

	EXEC	--canonicalize --readlink --verbose t/d/e
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/f

	EXEC	--canonicalize --readlink --verbose t/d/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/dd

	EXEC	--canonicalize --readlink --verbose t/c
		OUTPUT -
		ERROR - 'readlink: t/c: canonicalization error at e [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize --readlink --verbose t/xxx
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'

	EXEC	--canonicalize --relative t/dd
		OUTPUT - t/dd
		ERROR -n -
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
		EXIT 1

	EXEC	--canonicalize --relative t/c
		OUTPUT - t/d/f
		EXIT 0

	EXEC	--canonicalize --relative t/xxx
		OUTPUT - t/xxx

	EXEC	--canonicalize --relative --verbose t/dd
		OUTPUT - t/dd

	EXEC	--canonicalize --relative --verbose t/d
		OUTPUT - t/d

	EXEC	--canonicalize --relative --verbose t/z
		OUTPUT - t/z

	EXEC	--canonicalize --relative --verbose t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize --relative --verbose t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize --relative --verbose t/f
		OUTPUT - t/f

	EXEC	--canonicalize --relative --verbose t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize --relative --verbose t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize --relative --verbose t/a
		OUTPUT - t/d/f

	EXEC	--canonicalize --relative --verbose t/b

	EXEC	--canonicalize --relative --verbose t/d/e

	EXEC	--canonicalize --relative --verbose t/d/dd
		OUTPUT -
		ERROR - 'readlink: t/d/dd: canonicalization error [Too many levels of symbolic links]'
		EXIT 1

	EXEC	--canonicalize --relative --verbose t/c
		OUTPUT - t/d/f
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize --relative --verbose t/xxx
		OUTPUT - t/xxx

	EXEC	--canonicalize --relative --readlink t/dd
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize --relative --readlink t/d

	EXEC	--canonicalize --relative --readlink t/z

	EXEC	--canonicalize --relative --readlink t/dd/g

	EXEC	--canonicalize --relative --readlink t/z/g

	EXEC	--canonicalize --relative --readlink t/f

	EXEC	--canonicalize --relative --readlink t/dd/g

	EXEC	--canonicalize --relative --readlink t/dd/eee

	EXEC	--canonicalize --relative --readlink t/a
		OUTPUT - b
		EXIT 0

	EXEC	--canonicalize --relative --readlink t/b
		OUTPUT - c

	EXEC	--canonicalize --relative --readlink t/d/e
		OUTPUT - f

	EXEC	--canonicalize --relative --readlink t/d/dd
		OUTPUT - dd

	EXEC	--canonicalize --relative --readlink t/c
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize --relative --readlink t/xxx

	EXEC	--canonicalize --relative --readlink --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --relative --readlink --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --relative --readlink --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --relative --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --relative --readlink --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --relative --readlink --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --relative --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --relative --readlink --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --relative --readlink --verbose t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize --relative --readlink --verbose t/b
		OUTPUT - c

	EXEC	--canonicalize --relative --readlink --verbose t/d/e
		OUTPUT - f

	EXEC	--canonicalize --relative --readlink --verbose t/d/dd
		OUTPUT - dd

	EXEC	--canonicalize --relative --readlink --verbose t/c
		OUTPUT -
		ERROR - 'readlink: t/c: canonicalization error at e [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize --relative --readlink --verbose t/xxx
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'

	EXEC	--canonicalize --logical t/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize --logical t/d
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d

	EXEC	--canonicalize --logical t/z
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z

	EXEC	--canonicalize --logical t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize --logical t/z/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z/g

	EXEC	--canonicalize --logical t/f
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/f

	EXEC	--canonicalize --logical t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize --logical t/dd/eee
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/eee

	EXEC	--canonicalize --logical t/a
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/a

	EXEC	--canonicalize --logical t/b
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/b

	EXEC	--canonicalize --logical t/d/e
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/e

	EXEC	--canonicalize --logical t/d/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/dd

	EXEC	--canonicalize --logical t/c
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/c

	EXEC	--canonicalize --logical t/xxx
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize --logical --verbose t/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd
		EXIT 0

	EXEC	--canonicalize --logical --verbose t/d
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d

	EXEC	--canonicalize --logical --verbose t/z
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z

	EXEC	--canonicalize --logical --verbose t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize --logical --verbose t/z/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z/g

	EXEC	--canonicalize --logical --verbose t/f
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/f

	EXEC	--canonicalize --logical --verbose t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize --logical --verbose t/dd/eee
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/eee

	EXEC	--canonicalize --logical --verbose t/a
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/a

	EXEC	--canonicalize --logical --verbose t/b
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/b

	EXEC	--canonicalize --logical --verbose t/d/e
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/e

	EXEC	--canonicalize --logical --verbose t/d/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/dd

	EXEC	--canonicalize --logical --verbose t/c
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/c

	EXEC	--canonicalize --logical --verbose t/xxx
		OUTPUT -
		ERROR - 'readlink: t/xxx: canonicalization error [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize --logical --readlink t/dd
		ERROR -n -

	EXEC	--canonicalize --logical --readlink t/d

	EXEC	--canonicalize --logical --readlink t/z

	EXEC	--canonicalize --logical --readlink t/dd/g

	EXEC	--canonicalize --logical --readlink t/z/g

	EXEC	--canonicalize --logical --readlink t/f

	EXEC	--canonicalize --logical --readlink t/dd/g

	EXEC	--canonicalize --logical --readlink t/dd/eee

	EXEC	--canonicalize --logical --readlink t/a

	EXEC	--canonicalize --logical --readlink t/b

	EXEC	--canonicalize --logical --readlink t/d/e

	EXEC	--canonicalize --logical --readlink t/d/dd

	EXEC	--canonicalize --logical --readlink t/c

	EXEC	--canonicalize --logical --readlink t/xxx

	EXEC	--canonicalize --logical --readlink --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --readlink --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --readlink --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --readlink --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --readlink --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --readlink --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --readlink --verbose t/a
		ERROR - 'readlink: t/a: canonicalization error [No such file or directory]'

	EXEC	--canonicalize --logical --readlink --verbose t/b
		ERROR - 'readlink: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize --logical --readlink --verbose t/d/e
		ERROR - 'readlink: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize --logical --readlink --verbose t/d/dd
		ERROR - 'readlink: t/d/dd: canonicalization error [No such file or directory]'

	EXEC	--canonicalize --logical --readlink --verbose t/c
		ERROR - 'readlink: t/c: canonicalization error at e [No such file or directory]'

	EXEC	--canonicalize --logical --readlink --verbose t/xxx
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'

	EXEC	--canonicalize --logical --relative t/dd
		OUTPUT - t/dd
		ERROR -n -
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
		EXIT 1

	EXEC	--canonicalize --logical --relative --verbose t/dd
		OUTPUT - t/dd
		EXIT 0

	EXEC	--canonicalize --logical --relative --verbose t/d
		OUTPUT - t/d

	EXEC	--canonicalize --logical --relative --verbose t/z
		OUTPUT - t/z

	EXEC	--canonicalize --logical --relative --verbose t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize --logical --relative --verbose t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize --logical --relative --verbose t/f
		OUTPUT - t/f

	EXEC	--canonicalize --logical --relative --verbose t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize --logical --relative --verbose t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize --logical --relative --verbose t/a
		OUTPUT - t/a

	EXEC	--canonicalize --logical --relative --verbose t/b
		OUTPUT - t/b

	EXEC	--canonicalize --logical --relative --verbose t/d/e
		OUTPUT - t/d/e

	EXEC	--canonicalize --logical --relative --verbose t/d/dd
		OUTPUT - t/d/dd

	EXEC	--canonicalize --logical --relative --verbose t/c
		OUTPUT - t/c

	EXEC	--canonicalize --logical --relative --verbose t/xxx
		OUTPUT -
		ERROR - 'readlink: t/xxx: canonicalization error [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize --logical --relative --readlink t/dd
		ERROR -n -

	EXEC	--canonicalize --logical --relative --readlink t/d

	EXEC	--canonicalize --logical --relative --readlink t/z

	EXEC	--canonicalize --logical --relative --readlink t/dd/g

	EXEC	--canonicalize --logical --relative --readlink t/z/g

	EXEC	--canonicalize --logical --relative --readlink t/f

	EXEC	--canonicalize --logical --relative --readlink t/dd/g

	EXEC	--canonicalize --logical --relative --readlink t/dd/eee

	EXEC	--canonicalize --logical --relative --readlink t/a

	EXEC	--canonicalize --logical --relative --readlink t/b

	EXEC	--canonicalize --logical --relative --readlink t/d/e

	EXEC	--canonicalize --logical --relative --readlink t/d/dd

	EXEC	--canonicalize --logical --relative --readlink t/c

	EXEC	--canonicalize --logical --relative --readlink t/xxx

	EXEC	--canonicalize --logical --relative --readlink --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --relative --readlink --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --relative --readlink --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --relative --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --relative --readlink --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --relative --readlink --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --relative --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --relative --readlink --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --logical --relative --readlink --verbose t/a
		ERROR - 'readlink: t/a: canonicalization error [No such file or directory]'

	EXEC	--canonicalize --logical --relative --readlink --verbose t/b
		ERROR - 'readlink: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize --logical --relative --readlink --verbose t/d/e
		ERROR - 'readlink: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize --logical --relative --readlink --verbose t/d/dd
		ERROR - 'readlink: t/d/dd: canonicalization error [No such file or directory]'

	EXEC	--canonicalize --logical --relative --readlink --verbose t/c
		ERROR - 'readlink: t/c: canonicalization error at e [No such file or directory]'

	EXEC	--canonicalize --logical --relative --readlink --verbose t/xxx
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'

	EXEC	--canonicalize --physical t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize --physical t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d

	EXEC	--canonicalize --physical t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z

	EXEC	--canonicalize --physical t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize --physical t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z/g

	EXEC	--canonicalize --physical t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/f

	EXEC	--canonicalize --physical t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize --physical t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/eee

	EXEC	--canonicalize --physical t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/f

	EXEC	--canonicalize --physical t/b

	EXEC	--canonicalize --physical t/d/e

	EXEC	--canonicalize --physical t/d/dd
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize --physical t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/f
		EXIT 0

	EXEC	--canonicalize --physical t/xxx
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/xxx

	EXEC	--canonicalize --physical --verbose t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd

	EXEC	--canonicalize --physical --verbose t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d

	EXEC	--canonicalize --physical --verbose t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z

	EXEC	--canonicalize --physical --verbose t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize --physical --verbose t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z/g

	EXEC	--canonicalize --physical --verbose t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/f

	EXEC	--canonicalize --physical --verbose t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize --physical --verbose t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/eee

	EXEC	--canonicalize --physical --verbose t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/f

	EXEC	--canonicalize --physical --verbose t/b

	EXEC	--canonicalize --physical --verbose t/d/e

	EXEC	--canonicalize --physical --verbose t/d/dd
		OUTPUT -
		ERROR - 'readlink: t/d/dd: canonicalization error [Too many levels of symbolic links]'
		EXIT 1

	EXEC	--canonicalize --physical --verbose t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/f
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize --physical --verbose t/xxx
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/xxx

	EXEC	--canonicalize --physical --readlink t/dd
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize --physical --readlink t/d

	EXEC	--canonicalize --physical --readlink t/z

	EXEC	--canonicalize --physical --readlink t/dd/g

	EXEC	--canonicalize --physical --readlink t/z/g

	EXEC	--canonicalize --physical --readlink t/f

	EXEC	--canonicalize --physical --readlink t/dd/g

	EXEC	--canonicalize --physical --readlink t/dd/eee

	EXEC	--canonicalize --physical --readlink t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/b
		EXIT 0

	EXEC	--canonicalize --physical --readlink t/b
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/c

	EXEC	--canonicalize --physical --readlink t/d/e
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/f

	EXEC	--canonicalize --physical --readlink t/d/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/dd

	EXEC	--canonicalize --physical --readlink t/c
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize --physical --readlink t/xxx

	EXEC	--canonicalize --physical --readlink --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --readlink --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --readlink --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --readlink --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --readlink --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --readlink --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --readlink --verbose t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/b
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize --physical --readlink --verbose t/b
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/c

	EXEC	--canonicalize --physical --readlink --verbose t/d/e
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/f

	EXEC	--canonicalize --physical --readlink --verbose t/d/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/dd

	EXEC	--canonicalize --physical --readlink --verbose t/c
		OUTPUT -
		ERROR - 'readlink: t/c: canonicalization error at e [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize --physical --readlink --verbose t/xxx
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'

	EXEC	--canonicalize --physical --relative t/dd
		OUTPUT - t/dd
		ERROR -n -
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
		EXIT 1

	EXEC	--canonicalize --physical --relative t/c
		OUTPUT - t/d/f
		EXIT 0

	EXEC	--canonicalize --physical --relative t/xxx
		OUTPUT - t/xxx

	EXEC	--canonicalize --physical --relative --verbose t/dd
		OUTPUT - t/dd

	EXEC	--canonicalize --physical --relative --verbose t/d
		OUTPUT - t/d

	EXEC	--canonicalize --physical --relative --verbose t/z
		OUTPUT - t/z

	EXEC	--canonicalize --physical --relative --verbose t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize --physical --relative --verbose t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize --physical --relative --verbose t/f
		OUTPUT - t/f

	EXEC	--canonicalize --physical --relative --verbose t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize --physical --relative --verbose t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize --physical --relative --verbose t/a
		OUTPUT - t/d/f

	EXEC	--canonicalize --physical --relative --verbose t/b

	EXEC	--canonicalize --physical --relative --verbose t/d/e

	EXEC	--canonicalize --physical --relative --verbose t/d/dd
		OUTPUT -
		ERROR - 'readlink: t/d/dd: canonicalization error [Too many levels of symbolic links]'
		EXIT 1

	EXEC	--canonicalize --physical --relative --verbose t/c
		OUTPUT - t/d/f
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize --physical --relative --verbose t/xxx
		OUTPUT - t/xxx

	EXEC	--canonicalize --physical --relative --readlink t/dd
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize --physical --relative --readlink t/d

	EXEC	--canonicalize --physical --relative --readlink t/z

	EXEC	--canonicalize --physical --relative --readlink t/dd/g

	EXEC	--canonicalize --physical --relative --readlink t/z/g

	EXEC	--canonicalize --physical --relative --readlink t/f

	EXEC	--canonicalize --physical --relative --readlink t/dd/g

	EXEC	--canonicalize --physical --relative --readlink t/dd/eee

	EXEC	--canonicalize --physical --relative --readlink t/a
		OUTPUT - b
		EXIT 0

	EXEC	--canonicalize --physical --relative --readlink t/b
		OUTPUT - c

	EXEC	--canonicalize --physical --relative --readlink t/d/e
		OUTPUT - f

	EXEC	--canonicalize --physical --relative --readlink t/d/dd
		OUTPUT - dd

	EXEC	--canonicalize --physical --relative --readlink t/c
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize --physical --relative --readlink t/xxx

	EXEC	--canonicalize --physical --relative --readlink --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --relative --readlink --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --relative --readlink --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --relative --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --relative --readlink --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --relative --readlink --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --relative --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --relative --readlink --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize --physical --relative --readlink --verbose t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize --physical --relative --readlink --verbose t/b
		OUTPUT - c

	EXEC	--canonicalize --physical --relative --readlink --verbose t/d/e
		OUTPUT - f

	EXEC	--canonicalize --physical --relative --readlink --verbose t/d/dd
		OUTPUT - dd

	EXEC	--canonicalize --physical --relative --readlink --verbose t/c
		OUTPUT -
		ERROR - 'readlink: t/c: canonicalization error at e [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize --physical --relative --readlink --verbose t/xxx
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'

	EXEC	--canonicalize-existing t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize-existing t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d

	EXEC	--canonicalize-existing t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z

	EXEC	--canonicalize-existing t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize-existing t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z/g

	EXEC	--canonicalize-existing t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/f

	EXEC	--canonicalize-existing t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize-existing t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/eee

	EXEC	--canonicalize-existing t/a
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize-existing t/b

	EXEC	--canonicalize-existing t/d/e

	EXEC	--canonicalize-existing t/d/dd

	EXEC	--canonicalize-existing t/c

	EXEC	--canonicalize-existing t/xxx

	EXEC	--canonicalize-existing --verbose t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd
		EXIT 0

	EXEC	--canonicalize-existing --verbose t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d

	EXEC	--canonicalize-existing --verbose t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z

	EXEC	--canonicalize-existing --verbose t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize-existing --verbose t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z/g

	EXEC	--canonicalize-existing --verbose t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/f

	EXEC	--canonicalize-existing --verbose t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize-existing --verbose t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/eee

	EXEC	--canonicalize-existing --verbose t/a
		OUTPUT -
		ERROR - 'readlink: t/a: canonicalization error [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize-existing --verbose t/b
		ERROR - 'readlink: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --verbose t/d/e
		ERROR - 'readlink: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --verbose t/d/dd
		ERROR - 'readlink: t/d/dd: canonicalization error [Too many levels of symbolic links]'

	EXEC	--canonicalize-existing --verbose t/c
		ERROR - 'readlink: t/c: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --verbose t/xxx
		ERROR - 'readlink: t/xxx: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --readlink t/dd
		ERROR -n -

	EXEC	--canonicalize-existing --readlink t/d

	EXEC	--canonicalize-existing --readlink t/z

	EXEC	--canonicalize-existing --readlink t/dd/g

	EXEC	--canonicalize-existing --readlink t/z/g

	EXEC	--canonicalize-existing --readlink t/f

	EXEC	--canonicalize-existing --readlink t/dd/g

	EXEC	--canonicalize-existing --readlink t/dd/eee

	EXEC	--canonicalize-existing --readlink t/a

	EXEC	--canonicalize-existing --readlink t/b

	EXEC	--canonicalize-existing --readlink t/d/e

	EXEC	--canonicalize-existing --readlink t/d/dd

	EXEC	--canonicalize-existing --readlink t/c

	EXEC	--canonicalize-existing --readlink t/xxx

	EXEC	--canonicalize-existing --readlink --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --readlink --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --readlink --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --readlink --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --readlink --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --readlink --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --readlink --verbose t/a
		ERROR - 'readlink: t/a: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --readlink --verbose t/b
		ERROR - 'readlink: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --readlink --verbose t/d/e
		ERROR - 'readlink: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --readlink --verbose t/d/dd
		ERROR - 'readlink: t/d/dd: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --readlink --verbose t/c
		ERROR - 'readlink: t/c: canonicalization error at e [No such file or directory]'

	EXEC	--canonicalize-existing --readlink --verbose t/xxx
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'

	EXEC	--canonicalize-existing --relative t/dd
		OUTPUT - t/dd
		ERROR -n -
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
		EXIT 1

	EXEC	--canonicalize-existing --relative t/b

	EXEC	--canonicalize-existing --relative t/d/e

	EXEC	--canonicalize-existing --relative t/d/dd

	EXEC	--canonicalize-existing --relative t/c

	EXEC	--canonicalize-existing --relative t/xxx

	EXEC	--canonicalize-existing --relative --verbose t/dd
		OUTPUT - t/dd
		EXIT 0

	EXEC	--canonicalize-existing --relative --verbose t/d
		OUTPUT - t/d

	EXEC	--canonicalize-existing --relative --verbose t/z
		OUTPUT - t/z

	EXEC	--canonicalize-existing --relative --verbose t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-existing --relative --verbose t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize-existing --relative --verbose t/f
		OUTPUT - t/f

	EXEC	--canonicalize-existing --relative --verbose t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-existing --relative --verbose t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize-existing --relative --verbose t/a
		OUTPUT -
		ERROR - 'readlink: t/a: canonicalization error [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize-existing --relative --verbose t/b
		ERROR - 'readlink: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --relative --verbose t/d/e
		ERROR - 'readlink: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --relative --verbose t/d/dd
		ERROR - 'readlink: t/d/dd: canonicalization error [Too many levels of symbolic links]'

	EXEC	--canonicalize-existing --relative --verbose t/c
		ERROR - 'readlink: t/c: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --relative --verbose t/xxx
		ERROR - 'readlink: t/xxx: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --relative --readlink t/dd
		ERROR -n -

	EXEC	--canonicalize-existing --relative --readlink t/d

	EXEC	--canonicalize-existing --relative --readlink t/z

	EXEC	--canonicalize-existing --relative --readlink t/dd/g

	EXEC	--canonicalize-existing --relative --readlink t/z/g

	EXEC	--canonicalize-existing --relative --readlink t/f

	EXEC	--canonicalize-existing --relative --readlink t/dd/g

	EXEC	--canonicalize-existing --relative --readlink t/dd/eee

	EXEC	--canonicalize-existing --relative --readlink t/a

	EXEC	--canonicalize-existing --relative --readlink t/b

	EXEC	--canonicalize-existing --relative --readlink t/d/e

	EXEC	--canonicalize-existing --relative --readlink t/d/dd

	EXEC	--canonicalize-existing --relative --readlink t/c

	EXEC	--canonicalize-existing --relative --readlink t/xxx

	EXEC	--canonicalize-existing --relative --readlink --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --relative --readlink --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --relative --readlink --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --relative --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --relative --readlink --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --relative --readlink --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --relative --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --relative --readlink --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --relative --readlink --verbose t/a
		ERROR - 'readlink: t/a: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --relative --readlink --verbose t/b
		ERROR - 'readlink: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --relative --readlink --verbose t/d/e
		ERROR - 'readlink: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --relative --readlink --verbose t/d/dd
		ERROR - 'readlink: t/d/dd: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --relative --readlink --verbose t/c
		ERROR - 'readlink: t/c: canonicalization error at e [No such file or directory]'

	EXEC	--canonicalize-existing --relative --readlink --verbose t/xxx
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'

	EXEC	--canonicalize-existing --logical t/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize-existing --logical t/d
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d

	EXEC	--canonicalize-existing --logical t/z
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z

	EXEC	--canonicalize-existing --logical t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize-existing --logical t/z/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z/g

	EXEC	--canonicalize-existing --logical t/f
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/f

	EXEC	--canonicalize-existing --logical t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize-existing --logical t/dd/eee
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/eee

	EXEC	--canonicalize-existing --logical t/a
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize-existing --logical t/b

	EXEC	--canonicalize-existing --logical t/d/e

	EXEC	--canonicalize-existing --logical t/d/dd

	EXEC	--canonicalize-existing --logical t/c

	EXEC	--canonicalize-existing --logical t/xxx

	EXEC	--canonicalize-existing --logical --verbose t/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd
		EXIT 0

	EXEC	--canonicalize-existing --logical --verbose t/d
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d

	EXEC	--canonicalize-existing --logical --verbose t/z
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z

	EXEC	--canonicalize-existing --logical --verbose t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize-existing --logical --verbose t/z/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z/g

	EXEC	--canonicalize-existing --logical --verbose t/f
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/f

	EXEC	--canonicalize-existing --logical --verbose t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize-existing --logical --verbose t/dd/eee
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/eee

	EXEC	--canonicalize-existing --logical --verbose t/a
		OUTPUT -
		ERROR - 'readlink: t/a: canonicalization error [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize-existing --logical --verbose t/b
		ERROR - 'readlink: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --verbose t/d/e
		ERROR - 'readlink: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --verbose t/d/dd
		ERROR - 'readlink: t/d/dd: canonicalization error [Too many levels of symbolic links]'

	EXEC	--canonicalize-existing --logical --verbose t/c
		ERROR - 'readlink: t/c: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --verbose t/xxx
		ERROR - 'readlink: t/xxx: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --readlink t/dd
		ERROR -n -

	EXEC	--canonicalize-existing --logical --readlink t/d

	EXEC	--canonicalize-existing --logical --readlink t/z

	EXEC	--canonicalize-existing --logical --readlink t/dd/g

	EXEC	--canonicalize-existing --logical --readlink t/z/g

	EXEC	--canonicalize-existing --logical --readlink t/f

	EXEC	--canonicalize-existing --logical --readlink t/dd/g

	EXEC	--canonicalize-existing --logical --readlink t/dd/eee

	EXEC	--canonicalize-existing --logical --readlink t/a

	EXEC	--canonicalize-existing --logical --readlink t/b

	EXEC	--canonicalize-existing --logical --readlink t/d/e

	EXEC	--canonicalize-existing --logical --readlink t/d/dd

	EXEC	--canonicalize-existing --logical --readlink t/c

	EXEC	--canonicalize-existing --logical --readlink t/xxx

	EXEC	--canonicalize-existing --logical --readlink --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --readlink --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --readlink --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --readlink --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --readlink --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --readlink --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --readlink --verbose t/a
		ERROR - 'readlink: t/a: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --readlink --verbose t/b
		ERROR - 'readlink: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --readlink --verbose t/d/e
		ERROR - 'readlink: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --readlink --verbose t/d/dd
		ERROR - 'readlink: t/d/dd: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --readlink --verbose t/c
		ERROR - 'readlink: t/c: canonicalization error at e [No such file or directory]'

	EXEC	--canonicalize-existing --logical --readlink --verbose t/xxx
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'

	EXEC	--canonicalize-existing --logical --relative t/dd
		OUTPUT - t/dd
		ERROR -n -
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
		EXIT 1

	EXEC	--canonicalize-existing --logical --relative t/b

	EXEC	--canonicalize-existing --logical --relative t/d/e

	EXEC	--canonicalize-existing --logical --relative t/d/dd

	EXEC	--canonicalize-existing --logical --relative t/c

	EXEC	--canonicalize-existing --logical --relative t/xxx

	EXEC	--canonicalize-existing --logical --relative --verbose t/dd
		OUTPUT - t/dd
		EXIT 0

	EXEC	--canonicalize-existing --logical --relative --verbose t/d
		OUTPUT - t/d

	EXEC	--canonicalize-existing --logical --relative --verbose t/z
		OUTPUT - t/z

	EXEC	--canonicalize-existing --logical --relative --verbose t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-existing --logical --relative --verbose t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize-existing --logical --relative --verbose t/f
		OUTPUT - t/f

	EXEC	--canonicalize-existing --logical --relative --verbose t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-existing --logical --relative --verbose t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize-existing --logical --relative --verbose t/a
		OUTPUT -
		ERROR - 'readlink: t/a: canonicalization error [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize-existing --logical --relative --verbose t/b
		ERROR - 'readlink: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --relative --verbose t/d/e
		ERROR - 'readlink: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --relative --verbose t/d/dd
		ERROR - 'readlink: t/d/dd: canonicalization error [Too many levels of symbolic links]'

	EXEC	--canonicalize-existing --logical --relative --verbose t/c
		ERROR - 'readlink: t/c: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --relative --verbose t/xxx
		ERROR - 'readlink: t/xxx: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --relative --readlink t/dd
		ERROR -n -

	EXEC	--canonicalize-existing --logical --relative --readlink t/d

	EXEC	--canonicalize-existing --logical --relative --readlink t/z

	EXEC	--canonicalize-existing --logical --relative --readlink t/dd/g

	EXEC	--canonicalize-existing --logical --relative --readlink t/z/g

	EXEC	--canonicalize-existing --logical --relative --readlink t/f

	EXEC	--canonicalize-existing --logical --relative --readlink t/dd/g

	EXEC	--canonicalize-existing --logical --relative --readlink t/dd/eee

	EXEC	--canonicalize-existing --logical --relative --readlink t/a

	EXEC	--canonicalize-existing --logical --relative --readlink t/b

	EXEC	--canonicalize-existing --logical --relative --readlink t/d/e

	EXEC	--canonicalize-existing --logical --relative --readlink t/d/dd

	EXEC	--canonicalize-existing --logical --relative --readlink t/c

	EXEC	--canonicalize-existing --logical --relative --readlink t/xxx

	EXEC	--canonicalize-existing --logical --relative --readlink --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --relative --readlink --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --relative --readlink --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --relative --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --relative --readlink --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --relative --readlink --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --relative --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --relative --readlink --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --logical --relative --readlink --verbose t/a
		ERROR - 'readlink: t/a: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --relative --readlink --verbose t/b
		ERROR - 'readlink: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --relative --readlink --verbose t/d/e
		ERROR - 'readlink: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --relative --readlink --verbose t/d/dd
		ERROR - 'readlink: t/d/dd: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --logical --relative --readlink --verbose t/c
		ERROR - 'readlink: t/c: canonicalization error at e [No such file or directory]'

	EXEC	--canonicalize-existing --logical --relative --readlink --verbose t/xxx
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'

	EXEC	--canonicalize-existing --physical t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize-existing --physical t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d

	EXEC	--canonicalize-existing --physical t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z

	EXEC	--canonicalize-existing --physical t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize-existing --physical t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z/g

	EXEC	--canonicalize-existing --physical t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/f

	EXEC	--canonicalize-existing --physical t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize-existing --physical t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/eee

	EXEC	--canonicalize-existing --physical t/a
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize-existing --physical t/b

	EXEC	--canonicalize-existing --physical t/d/e

	EXEC	--canonicalize-existing --physical t/d/dd

	EXEC	--canonicalize-existing --physical t/c

	EXEC	--canonicalize-existing --physical t/xxx

	EXEC	--canonicalize-existing --physical --verbose t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd
		EXIT 0

	EXEC	--canonicalize-existing --physical --verbose t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d

	EXEC	--canonicalize-existing --physical --verbose t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z

	EXEC	--canonicalize-existing --physical --verbose t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize-existing --physical --verbose t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z/g

	EXEC	--canonicalize-existing --physical --verbose t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/f

	EXEC	--canonicalize-existing --physical --verbose t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize-existing --physical --verbose t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/eee

	EXEC	--canonicalize-existing --physical --verbose t/a
		OUTPUT -
		ERROR - 'readlink: t/a: canonicalization error [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize-existing --physical --verbose t/b
		ERROR - 'readlink: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --verbose t/d/e
		ERROR - 'readlink: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --verbose t/d/dd
		ERROR - 'readlink: t/d/dd: canonicalization error [Too many levels of symbolic links]'

	EXEC	--canonicalize-existing --physical --verbose t/c
		ERROR - 'readlink: t/c: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --verbose t/xxx
		ERROR - 'readlink: t/xxx: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --readlink t/dd
		ERROR -n -

	EXEC	--canonicalize-existing --physical --readlink t/d

	EXEC	--canonicalize-existing --physical --readlink t/z

	EXEC	--canonicalize-existing --physical --readlink t/dd/g

	EXEC	--canonicalize-existing --physical --readlink t/z/g

	EXEC	--canonicalize-existing --physical --readlink t/f

	EXEC	--canonicalize-existing --physical --readlink t/dd/g

	EXEC	--canonicalize-existing --physical --readlink t/dd/eee

	EXEC	--canonicalize-existing --physical --readlink t/a

	EXEC	--canonicalize-existing --physical --readlink t/b

	EXEC	--canonicalize-existing --physical --readlink t/d/e

	EXEC	--canonicalize-existing --physical --readlink t/d/dd

	EXEC	--canonicalize-existing --physical --readlink t/c

	EXEC	--canonicalize-existing --physical --readlink t/xxx

	EXEC	--canonicalize-existing --physical --readlink --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --readlink --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --readlink --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --readlink --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --readlink --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --readlink --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --readlink --verbose t/a
		ERROR - 'readlink: t/a: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --readlink --verbose t/b
		ERROR - 'readlink: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --readlink --verbose t/d/e
		ERROR - 'readlink: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --readlink --verbose t/d/dd
		ERROR - 'readlink: t/d/dd: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --readlink --verbose t/c
		ERROR - 'readlink: t/c: canonicalization error at e [No such file or directory]'

	EXEC	--canonicalize-existing --physical --readlink --verbose t/xxx
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'

	EXEC	--canonicalize-existing --physical --relative t/dd
		OUTPUT - t/dd
		ERROR -n -
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
		EXIT 1

	EXEC	--canonicalize-existing --physical --relative t/b

	EXEC	--canonicalize-existing --physical --relative t/d/e

	EXEC	--canonicalize-existing --physical --relative t/d/dd

	EXEC	--canonicalize-existing --physical --relative t/c

	EXEC	--canonicalize-existing --physical --relative t/xxx

	EXEC	--canonicalize-existing --physical --relative --verbose t/dd
		OUTPUT - t/dd
		EXIT 0

	EXEC	--canonicalize-existing --physical --relative --verbose t/d
		OUTPUT - t/d

	EXEC	--canonicalize-existing --physical --relative --verbose t/z
		OUTPUT - t/z

	EXEC	--canonicalize-existing --physical --relative --verbose t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-existing --physical --relative --verbose t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize-existing --physical --relative --verbose t/f
		OUTPUT - t/f

	EXEC	--canonicalize-existing --physical --relative --verbose t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-existing --physical --relative --verbose t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize-existing --physical --relative --verbose t/a
		OUTPUT -
		ERROR - 'readlink: t/a: canonicalization error [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize-existing --physical --relative --verbose t/b
		ERROR - 'readlink: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --relative --verbose t/d/e
		ERROR - 'readlink: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --relative --verbose t/d/dd
		ERROR - 'readlink: t/d/dd: canonicalization error [Too many levels of symbolic links]'

	EXEC	--canonicalize-existing --physical --relative --verbose t/c
		ERROR - 'readlink: t/c: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --relative --verbose t/xxx
		ERROR - 'readlink: t/xxx: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --relative --readlink t/dd
		ERROR -n -

	EXEC	--canonicalize-existing --physical --relative --readlink t/d

	EXEC	--canonicalize-existing --physical --relative --readlink t/z

	EXEC	--canonicalize-existing --physical --relative --readlink t/dd/g

	EXEC	--canonicalize-existing --physical --relative --readlink t/z/g

	EXEC	--canonicalize-existing --physical --relative --readlink t/f

	EXEC	--canonicalize-existing --physical --relative --readlink t/dd/g

	EXEC	--canonicalize-existing --physical --relative --readlink t/dd/eee

	EXEC	--canonicalize-existing --physical --relative --readlink t/a

	EXEC	--canonicalize-existing --physical --relative --readlink t/b

	EXEC	--canonicalize-existing --physical --relative --readlink t/d/e

	EXEC	--canonicalize-existing --physical --relative --readlink t/d/dd

	EXEC	--canonicalize-existing --physical --relative --readlink t/c

	EXEC	--canonicalize-existing --physical --relative --readlink t/xxx

	EXEC	--canonicalize-existing --physical --relative --readlink --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --relative --readlink --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --relative --readlink --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --relative --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --relative --readlink --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --relative --readlink --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --relative --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --relative --readlink --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-existing --physical --relative --readlink --verbose t/a
		ERROR - 'readlink: t/a: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --relative --readlink --verbose t/b
		ERROR - 'readlink: t/b: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --relative --readlink --verbose t/d/e
		ERROR - 'readlink: t/d/e: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --relative --readlink --verbose t/d/dd
		ERROR - 'readlink: t/d/dd: canonicalization error [No such file or directory]'

	EXEC	--canonicalize-existing --physical --relative --readlink --verbose t/c
		ERROR - 'readlink: t/c: canonicalization error at e [No such file or directory]'

	EXEC	--canonicalize-existing --physical --relative --readlink --verbose t/xxx
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'

	EXEC	--canonicalize-missing t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize-missing t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d

	EXEC	--canonicalize-missing t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z

	EXEC	--canonicalize-missing t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize-missing t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z/g

	EXEC	--canonicalize-missing t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/f

	EXEC	--canonicalize-missing t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize-missing t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/eee

	EXEC	--canonicalize-missing t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/f

	EXEC	--canonicalize-missing t/b

	EXEC	--canonicalize-missing t/d/e

	EXEC	--canonicalize-missing t/d/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/dd

	EXEC	--canonicalize-missing t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/f

	EXEC	--canonicalize-missing t/xxx
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/xxx

	EXEC	--canonicalize-missing --verbose t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd

	EXEC	--canonicalize-missing --verbose t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d

	EXEC	--canonicalize-missing --verbose t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z

	EXEC	--canonicalize-missing --verbose t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize-missing --verbose t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z/g

	EXEC	--canonicalize-missing --verbose t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/f

	EXEC	--canonicalize-missing --verbose t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize-missing --verbose t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/eee

	EXEC	--canonicalize-missing --verbose t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/f

	EXEC	--canonicalize-missing --verbose t/b

	EXEC	--canonicalize-missing --verbose t/d/e

	EXEC	--canonicalize-missing --verbose t/d/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/dd

	EXEC	--canonicalize-missing --verbose t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/f

	EXEC	--canonicalize-missing --verbose t/xxx
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/xxx

	EXEC	--canonicalize-missing --readlink t/dd
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize-missing --readlink t/d

	EXEC	--canonicalize-missing --readlink t/z

	EXEC	--canonicalize-missing --readlink t/dd/g

	EXEC	--canonicalize-missing --readlink t/z/g

	EXEC	--canonicalize-missing --readlink t/f

	EXEC	--canonicalize-missing --readlink t/dd/g

	EXEC	--canonicalize-missing --readlink t/dd/eee

	EXEC	--canonicalize-missing --readlink t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/b
		EXIT 0

	EXEC	--canonicalize-missing --readlink t/b
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/c

	EXEC	--canonicalize-missing --readlink t/d/e
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/f

	EXEC	--canonicalize-missing --readlink t/d/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/dd

	EXEC	--canonicalize-missing --readlink t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/d/e

	EXEC	--canonicalize-missing --readlink t/xxx
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize-missing --readlink --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --readlink --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --readlink --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --readlink --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --readlink --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --readlink --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --readlink --verbose t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/b
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize-missing --readlink --verbose t/b
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/c

	EXEC	--canonicalize-missing --readlink --verbose t/d/e
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/f

	EXEC	--canonicalize-missing --readlink --verbose t/d/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/dd

	EXEC	--canonicalize-missing --readlink --verbose t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/d/e

	EXEC	--canonicalize-missing --readlink --verbose t/xxx
		OUTPUT -
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize-missing --relative t/dd
		OUTPUT - t/dd
		ERROR -n -
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

	EXEC	--canonicalize-missing --relative --verbose t/dd
		OUTPUT - t/dd

	EXEC	--canonicalize-missing --relative --verbose t/d
		OUTPUT - t/d

	EXEC	--canonicalize-missing --relative --verbose t/z
		OUTPUT - t/z

	EXEC	--canonicalize-missing --relative --verbose t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-missing --relative --verbose t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize-missing --relative --verbose t/f
		OUTPUT - t/f

	EXEC	--canonicalize-missing --relative --verbose t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-missing --relative --verbose t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize-missing --relative --verbose t/a
		OUTPUT - t/d/f

	EXEC	--canonicalize-missing --relative --verbose t/b

	EXEC	--canonicalize-missing --relative --verbose t/d/e

	EXEC	--canonicalize-missing --relative --verbose t/d/dd
		OUTPUT - t/d/dd

	EXEC	--canonicalize-missing --relative --verbose t/c
		OUTPUT - t/d/f

	EXEC	--canonicalize-missing --relative --verbose t/xxx
		OUTPUT - t/xxx

	EXEC	--canonicalize-missing --relative --readlink t/dd
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize-missing --relative --readlink t/d

	EXEC	--canonicalize-missing --relative --readlink t/z

	EXEC	--canonicalize-missing --relative --readlink t/dd/g

	EXEC	--canonicalize-missing --relative --readlink t/z/g

	EXEC	--canonicalize-missing --relative --readlink t/f

	EXEC	--canonicalize-missing --relative --readlink t/dd/g

	EXEC	--canonicalize-missing --relative --readlink t/dd/eee

	EXEC	--canonicalize-missing --relative --readlink t/a
		OUTPUT - b
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
		EXIT 1

	EXEC	--canonicalize-missing --relative --readlink --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --relative --readlink --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --relative --readlink --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --relative --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --relative --readlink --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --relative --readlink --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --relative --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --relative --readlink --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --relative --readlink --verbose t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize-missing --relative --readlink --verbose t/b
		OUTPUT - c

	EXEC	--canonicalize-missing --relative --readlink --verbose t/d/e
		OUTPUT - f

	EXEC	--canonicalize-missing --relative --readlink --verbose t/d/dd
		OUTPUT - dd

	EXEC	--canonicalize-missing --relative --readlink --verbose t/c
		OUTPUT - d/e

	EXEC	--canonicalize-missing --relative --readlink --verbose t/xxx
		OUTPUT -
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize-missing --logical t/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize-missing --logical t/d
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d

	EXEC	--canonicalize-missing --logical t/z
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z

	EXEC	--canonicalize-missing --logical t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize-missing --logical t/z/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z/g

	EXEC	--canonicalize-missing --logical t/f
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/f

	EXEC	--canonicalize-missing --logical t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize-missing --logical t/dd/eee
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/eee

	EXEC	--canonicalize-missing --logical t/a
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/a

	EXEC	--canonicalize-missing --logical t/b
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/b

	EXEC	--canonicalize-missing --logical t/d/e
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/e

	EXEC	--canonicalize-missing --logical t/d/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/dd

	EXEC	--canonicalize-missing --logical t/c
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/c

	EXEC	--canonicalize-missing --logical t/xxx
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/xxx

	EXEC	--canonicalize-missing --logical --verbose t/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd

	EXEC	--canonicalize-missing --logical --verbose t/d
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d

	EXEC	--canonicalize-missing --logical --verbose t/z
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z

	EXEC	--canonicalize-missing --logical --verbose t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize-missing --logical --verbose t/z/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z/g

	EXEC	--canonicalize-missing --logical --verbose t/f
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/f

	EXEC	--canonicalize-missing --logical --verbose t/dd/g
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize-missing --logical --verbose t/dd/eee
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/eee

	EXEC	--canonicalize-missing --logical --verbose t/a
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/a

	EXEC	--canonicalize-missing --logical --verbose t/b
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/b

	EXEC	--canonicalize-missing --logical --verbose t/d/e
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/e

	EXEC	--canonicalize-missing --logical --verbose t/d/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/dd

	EXEC	--canonicalize-missing --logical --verbose t/c
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/c

	EXEC	--canonicalize-missing --logical --verbose t/xxx
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/xxx

	EXEC	--canonicalize-missing --logical --readlink t/dd
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize-missing --logical --readlink t/d

	EXEC	--canonicalize-missing --logical --readlink t/z

	EXEC	--canonicalize-missing --logical --readlink t/dd/g

	EXEC	--canonicalize-missing --logical --readlink t/z/g

	EXEC	--canonicalize-missing --logical --readlink t/f

	EXEC	--canonicalize-missing --logical --readlink t/dd/g

	EXEC	--canonicalize-missing --logical --readlink t/dd/eee

	EXEC	--canonicalize-missing --logical --readlink t/a
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/b
		EXIT 0

	EXEC	--canonicalize-missing --logical --readlink t/b
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/c

	EXEC	--canonicalize-missing --logical --readlink t/d/e
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/f

	EXEC	--canonicalize-missing --logical --readlink t/d/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/dd

	EXEC	--canonicalize-missing --logical --readlink t/c
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/d/e

	EXEC	--canonicalize-missing --logical --readlink t/xxx
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize-missing --logical --readlink --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --readlink --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --readlink --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --readlink --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --readlink --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --readlink --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --readlink --verbose t/a
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/b
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize-missing --logical --readlink --verbose t/b
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/c

	EXEC	--canonicalize-missing --logical --readlink --verbose t/d/e
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/f

	EXEC	--canonicalize-missing --logical --readlink --verbose t/d/dd
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/dd

	EXEC	--canonicalize-missing --logical --readlink --verbose t/c
		OUTPUT - /home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/d/e

	EXEC	--canonicalize-missing --logical --readlink --verbose t/xxx
		OUTPUT -
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize-missing --logical --relative t/dd
		OUTPUT - t/dd
		ERROR -n -
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

	EXEC	--canonicalize-missing --logical --relative --verbose t/dd
		OUTPUT - t/dd

	EXEC	--canonicalize-missing --logical --relative --verbose t/d
		OUTPUT - t/d

	EXEC	--canonicalize-missing --logical --relative --verbose t/z
		OUTPUT - t/z

	EXEC	--canonicalize-missing --logical --relative --verbose t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-missing --logical --relative --verbose t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize-missing --logical --relative --verbose t/f
		OUTPUT - t/f

	EXEC	--canonicalize-missing --logical --relative --verbose t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-missing --logical --relative --verbose t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize-missing --logical --relative --verbose t/a
		OUTPUT - t/a

	EXEC	--canonicalize-missing --logical --relative --verbose t/b
		OUTPUT - t/b

	EXEC	--canonicalize-missing --logical --relative --verbose t/d/e
		OUTPUT - t/d/e

	EXEC	--canonicalize-missing --logical --relative --verbose t/d/dd
		OUTPUT - t/d/dd

	EXEC	--canonicalize-missing --logical --relative --verbose t/c
		OUTPUT - t/c

	EXEC	--canonicalize-missing --logical --relative --verbose t/xxx
		OUTPUT - t/xxx

	EXEC	--canonicalize-missing --logical --relative --readlink t/dd
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize-missing --logical --relative --readlink t/d

	EXEC	--canonicalize-missing --logical --relative --readlink t/z

	EXEC	--canonicalize-missing --logical --relative --readlink t/dd/g

	EXEC	--canonicalize-missing --logical --relative --readlink t/z/g

	EXEC	--canonicalize-missing --logical --relative --readlink t/f

	EXEC	--canonicalize-missing --logical --relative --readlink t/dd/g

	EXEC	--canonicalize-missing --logical --relative --readlink t/dd/eee

	EXEC	--canonicalize-missing --logical --relative --readlink t/a
		OUTPUT - b
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
		EXIT 1

	EXEC	--canonicalize-missing --logical --relative --readlink --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --relative --readlink --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --relative --readlink --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --relative --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --relative --readlink --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --relative --readlink --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --relative --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --relative --readlink --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --logical --relative --readlink --verbose t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize-missing --logical --relative --readlink --verbose t/b
		OUTPUT - c

	EXEC	--canonicalize-missing --logical --relative --readlink --verbose t/d/e
		OUTPUT - f

	EXEC	--canonicalize-missing --logical --relative --readlink --verbose t/d/dd
		OUTPUT - dd

	EXEC	--canonicalize-missing --logical --relative --readlink --verbose t/c
		OUTPUT - d/e

	EXEC	--canonicalize-missing --logical --relative --readlink --verbose t/xxx
		OUTPUT -
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize-missing --physical t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize-missing --physical t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d

	EXEC	--canonicalize-missing --physical t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z

	EXEC	--canonicalize-missing --physical t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize-missing --physical t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z/g

	EXEC	--canonicalize-missing --physical t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/f

	EXEC	--canonicalize-missing --physical t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize-missing --physical t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/eee

	EXEC	--canonicalize-missing --physical t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/f

	EXEC	--canonicalize-missing --physical t/b

	EXEC	--canonicalize-missing --physical t/d/e

	EXEC	--canonicalize-missing --physical t/d/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/dd

	EXEC	--canonicalize-missing --physical t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/f

	EXEC	--canonicalize-missing --physical t/xxx
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/xxx

	EXEC	--canonicalize-missing --physical --verbose t/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd

	EXEC	--canonicalize-missing --physical --verbose t/d
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d

	EXEC	--canonicalize-missing --physical --verbose t/z
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z

	EXEC	--canonicalize-missing --physical --verbose t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize-missing --physical --verbose t/z/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/z/g

	EXEC	--canonicalize-missing --physical --verbose t/f
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/f

	EXEC	--canonicalize-missing --physical --verbose t/dd/g
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/g

	EXEC	--canonicalize-missing --physical --verbose t/dd/eee
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/dd/eee

	EXEC	--canonicalize-missing --physical --verbose t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/f

	EXEC	--canonicalize-missing --physical --verbose t/b

	EXEC	--canonicalize-missing --physical --verbose t/d/e

	EXEC	--canonicalize-missing --physical --verbose t/d/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/dd

	EXEC	--canonicalize-missing --physical --verbose t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/d/f

	EXEC	--canonicalize-missing --physical --verbose t/xxx
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/t/xxx

	EXEC	--canonicalize-missing --physical --readlink t/dd
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize-missing --physical --readlink t/d

	EXEC	--canonicalize-missing --physical --readlink t/z

	EXEC	--canonicalize-missing --physical --readlink t/dd/g

	EXEC	--canonicalize-missing --physical --readlink t/z/g

	EXEC	--canonicalize-missing --physical --readlink t/f

	EXEC	--canonicalize-missing --physical --readlink t/dd/g

	EXEC	--canonicalize-missing --physical --readlink t/dd/eee

	EXEC	--canonicalize-missing --physical --readlink t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/b
		EXIT 0

	EXEC	--canonicalize-missing --physical --readlink t/b
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/c

	EXEC	--canonicalize-missing --physical --readlink t/d/e
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/f

	EXEC	--canonicalize-missing --physical --readlink t/d/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/dd

	EXEC	--canonicalize-missing --physical --readlink t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/d/e

	EXEC	--canonicalize-missing --physical --readlink t/xxx
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize-missing --physical --readlink --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --readlink --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --readlink --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --readlink --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --readlink --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --readlink --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --readlink --verbose t/a
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/b
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize-missing --physical --readlink --verbose t/b
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/c

	EXEC	--canonicalize-missing --physical --readlink --verbose t/d/e
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/f

	EXEC	--canonicalize-missing --physical --readlink --verbose t/d/dd
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/dd

	EXEC	--canonicalize-missing --physical --readlink --verbose t/c
		OUTPUT - /data/home/gsf/arch/linux.i386-64/src/cmd/builtin/readlink.tmp/d/e

	EXEC	--canonicalize-missing --physical --readlink --verbose t/xxx
		OUTPUT -
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1

	EXEC	--canonicalize-missing --physical --relative t/dd
		OUTPUT - t/dd
		ERROR -n -
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

	EXEC	--canonicalize-missing --physical --relative --verbose t/dd
		OUTPUT - t/dd

	EXEC	--canonicalize-missing --physical --relative --verbose t/d
		OUTPUT - t/d

	EXEC	--canonicalize-missing --physical --relative --verbose t/z
		OUTPUT - t/z

	EXEC	--canonicalize-missing --physical --relative --verbose t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-missing --physical --relative --verbose t/z/g
		OUTPUT - t/z/g

	EXEC	--canonicalize-missing --physical --relative --verbose t/f
		OUTPUT - t/f

	EXEC	--canonicalize-missing --physical --relative --verbose t/dd/g
		OUTPUT - t/dd/g

	EXEC	--canonicalize-missing --physical --relative --verbose t/dd/eee
		OUTPUT - t/dd/eee

	EXEC	--canonicalize-missing --physical --relative --verbose t/a
		OUTPUT - t/d/f

	EXEC	--canonicalize-missing --physical --relative --verbose t/b

	EXEC	--canonicalize-missing --physical --relative --verbose t/d/e

	EXEC	--canonicalize-missing --physical --relative --verbose t/d/dd
		OUTPUT - t/d/dd

	EXEC	--canonicalize-missing --physical --relative --verbose t/c
		OUTPUT - t/d/f

	EXEC	--canonicalize-missing --physical --relative --verbose t/xxx
		OUTPUT - t/xxx

	EXEC	--canonicalize-missing --physical --relative --readlink t/dd
		OUTPUT -
		EXIT 1

	EXEC	--canonicalize-missing --physical --relative --readlink t/d

	EXEC	--canonicalize-missing --physical --relative --readlink t/z

	EXEC	--canonicalize-missing --physical --relative --readlink t/dd/g

	EXEC	--canonicalize-missing --physical --relative --readlink t/z/g

	EXEC	--canonicalize-missing --physical --relative --readlink t/f

	EXEC	--canonicalize-missing --physical --relative --readlink t/dd/g

	EXEC	--canonicalize-missing --physical --relative --readlink t/dd/eee

	EXEC	--canonicalize-missing --physical --relative --readlink t/a
		OUTPUT - b
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
		EXIT 1

	EXEC	--canonicalize-missing --physical --relative --readlink --verbose t/dd
		ERROR - 'readlink: t/dd: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --relative --readlink --verbose t/d
		ERROR - 'readlink: t/d: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --relative --readlink --verbose t/z
		ERROR - 'readlink: t/z: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --relative --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --relative --readlink --verbose t/z/g
		ERROR - 'readlink: t/z/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --relative --readlink --verbose t/f
		ERROR - 'readlink: t/f: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --relative --readlink --verbose t/dd/g
		ERROR - 'readlink: t/dd/g: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --relative --readlink --verbose t/dd/eee
		ERROR - 'readlink: t/dd/eee: cannot read link text [Invalid argument]'

	EXEC	--canonicalize-missing --physical --relative --readlink --verbose t/a
		OUTPUT - b
		ERROR -n -
		EXIT 0

	EXEC	--canonicalize-missing --physical --relative --readlink --verbose t/b
		OUTPUT - c

	EXEC	--canonicalize-missing --physical --relative --readlink --verbose t/d/e
		OUTPUT - f

	EXEC	--canonicalize-missing --physical --relative --readlink --verbose t/d/dd
		OUTPUT - dd

	EXEC	--canonicalize-missing --physical --relative --readlink --verbose t/c
		OUTPUT - d/e

	EXEC	--canonicalize-missing --physical --relative --readlink --verbose t/xxx
		OUTPUT -
		ERROR - 'readlink: t/xxx: cannot read link text [No such file or directory]'
		EXIT 1
