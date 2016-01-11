: 3d regression tests
#
# 3d.tst (AT&T Research) 2004-07-01
#
# the first section defines the test harness
# the next section defines individual test functions
# the tests are in the last section
#

( vpath ) >/dev/null 2>&1 || {
	print -u2 $0: must be run from 3d shell
	exit 1
}

export LC_ALL=C

integer seconds=0

COMMAND=3d

FILE=
FORMAT="%K"
GROUP=
INIT=
NEW="new-and-improved"
NUKE=
OLD="original"
pwd=$PWD
PREFIX=
STAMP="2005-07-17+04:05:06"
VIRTUAL=

TWD

function ACTIVE
{
	DO return 0
}

function TEST
{
	case $INIT in
	"")	INIT=1
		mkdir -p $TWD/bottom || exit 1
		cd $TWD
		INTRO
		;;
	esac
	cd $TWD
	case $NUKE in
	?*)	rm -rf $NUKE; NUKE= ;;
	esac
	PREFIX=
	GROUP=$1
	ACTIVE || return 1
	vpath - -
	shift
	print "$GROUP	$*"
}

function FAIL # file message
{
	print -u2 "	FAIL $@"
	rm -rf $1
	(( ERRORS++ ))
	return 0
}

function VERIFY # command ...
{
	(( TESTS++ ))
	"$@" || FAIL "$@"
}

function PREFIX
{
	PREFIX=$1
	case $1 in
	/*)	NUKE="$NUKE $1" ;;
	*)	NUKE="$NUKE $PWD/$1" ;;
	esac
}

function VIRTUAL
{
	case $VIRTUAL in
	?*)	pwd=$PWD
		cd $TWD
		rm -rf $TWD/$VIRTUAL
		cd $pwd
		;;
	esac
	VIRTUAL=$1
}

function CD
{
	VERIFY cd $TWD/$1
}

function VPATH
{
	VERIFY vpath "$@"
}

function CP
{
	VERIFY cp "$@"
	shift $#-1
	NUKE="$NUKE $1"
}

function LN
{
	VERIFY ln "$@"
	shift $#-1
	NUKE="$NUKE $1"
}

function MV
{
	VERIFY mv "$@"
	shift $#-1
	NUKE="$NUKE $1"
}

function MKDIR
{
	VERIFY mkdir -p $*
	for i
	do	case $i in
		/*)	NUKE="$NUKE $i" ;;
		*)	NUKE="$NUKE $i" ;;
		esac
	done
}

function DATA
{
	VIRTUAL $VIRTUAL
	case $1 in
	-)	remove=1; shift ;;
	*)	remove=0 ;;
	esac
	case $# in
	0)	return 0 ;;
	1)	;;
	*)	return 1 ;;
	esac
	path=$1
	case $PREFIX in
	"")	FILE=$path ;;
	*)	FILE=$PREFIX/$path ;;
	esac
	file=bottom/$path
	if	[[ ! -f $TWD/$file ]]
	then	case $remove in
		0)	if	[[ $path == */* && ! -d $TWD/${file%/*} ]]
			then	mkdir -p $TWD/${file%/*} || FAIL $TWD/${file%/*} DATA mkdir
			fi
			print $OLD > $TWD/$file
			mode=${file%???}
			mode=${file#$mode}
			chmod $mode $TWD/$file || FAIL $TWD/$file DATA chmod
			;;
		esac
	else	case $remove in
		1)	rm -f $TWD/$file ;;
		esac
	fi
	(( TESTS++ ))
	return 0
}

#
# the remaining functions implement individiual parameterized tests
#

function APPEND
{
	DATA $*
	print "$NEW" >> $FILE || FAIL $FILE write error
	if	[[ $(<$FILE) != "$OLD"$'\n'"$NEW" ]]
	then	FAIL $FILE unchanged by $0
	elif	[[ -f $FILE/... && $(<$FILE/...) != "$OLD" ]]
	then	FAIL $FILE/... changed by $0
	fi
}

function MODE
{
	DATA $*
	chmod 000 $FILE || FAIL $FILE chmod error
	if	[[ -f $FILE/... && ! -r $FILE/... ]]
	then	FAIL $FILE/... changed by $0
	elif	[[ -r $FILE ]]
	then	FAIL $FILE unchanged by $0
	fi
}

function REMOVE
{
	DATA $*
	rm $FILE || FAIL $FILE rm error
	if	[[ ! -f $FILE/... ]]
	then	FAIL $FILE/... changed by $0
	fi
	print "$NEW" > $FILE || FAIL $FILE write error
	rm $FILE || FAIL $FILE rm error
	if	[[ $(<$FILE) != "$OLD" ]]
	then	FAIL $FILE unchanged by $0
	elif	[[ $(<$FILE/...) != "$OLD" ]]
	then	FAIL $FILE/... changed by $0
	fi
}

function TOUCH
{
	DATA $*
	touch -t "#$((seconds++))" $FILE/... || FAIL $FILE/... touch error
	touch -t "#$((seconds++))" $TWD/reference || FAIL $TWD/reference touch error
	touch -t "#$((seconds++))" $FILE || FAIL $FILE touch error
	if	[[ $FILE/... -nt $TWD/reference ]]
	then	FAIL $FILE/... changed by $0
	elif	[[ ! $FILE -nt $TWD/reference ]]
	then	FAIL $FILE unchanged by $0
	fi
	touch -t $STAMP $FILE
	if	[[ $(date -m -f $FORMAT $FILE) != "$STAMP" ]]
	then	FAIL $FILE modfiy time does not match $STAMP
	fi
}

function UPDATE
{
	DATA $*
	print "$NEW" 1<> $FILE || FAIL $FILE write error
	if	[[ $(<$FILE) != "$NEW" ]]
	then	FAIL $FILE unchanged by $0
	elif	[[ -f $FILE/... && $(<$FILE/...) != "$OLD" ]]
	then	FAIL $FILE/... changed by $0
	fi
}

function WRITE
{
	DATA $*
	print "$NEW" > $FILE || FAIL $FILE write error
	if	[[ $(<$FILE) != "$NEW" ]]
	then	FAIL $FILE unchanged by $0
	elif	[[ -f $FILE/... && $(<$FILE/...) != "$OLD" ]]
	then	FAIL $FILE/... changed by $0
	fi
}

function RUN
{
	[[ $1 == 3d ]] || return
	DATA
	WRITE	w666
	WRITE	w600
	TOUCH	t777
	MODE	m444
	WRITE	dir/w666
	WRITE	dir/w600
	TOUCH	dir/t777
	MODE	dir/m444
	UPDATE	u644
	UPDATE	u640
	APPEND	a644
	APPEND	a640
	UPDATE	dir/u644
	UPDATE	dir/u640
	APPEND	dir/a644
	APPEND	dir/a640
	VIRTUAL
	REMOVE	r644
	WRITE	r644
	REMOVE	r644
}

#
# finally the tests
#

TEST 01 PWD==top top exists &&
{
	VPATH top bottom
	MKDIR top
	CD top
	RUN 3d
}

TEST 02 PWD!=top top exists &&
{
	VPATH top bottom
	MKDIR top
	MKDIR junk
	CD junk
	PREFIX ../top
	RUN 3d
}

TEST 03 PWD==top top virtual &&
{
	VIRTUAL top
	VPATH top bottom
	CD top
	RUN 3d
}

TEST 04 PWD!=top top virtual &&
{
	VIRTUAL top
	VPATH top bottom
	MKDIR junk
	CD junk
	PREFIX ../top
	RUN 3d
}

TEST 05 top symlink &&
{
	if	LN -s text link
	then	[[ -L link ]] || FAIL lstat does stat
	fi
}

TEST 06 symlink spaghetti &&
{
	MKDIR usr/bin sbin
	echo : > sbin/cmd && chmod +x sbin/cmd
	LN -s usr/bin bin
	LN -s ../../sbin/cmd usr/bin/cmd
	CD bin
	PATH=:$PATH cmd
}

TEST 07 PWD==top top exists, bot virtual &&
{
	VPATH top bot
	MKDIR top
	CD top
	echo foo > foo && echo bar > bar
	CP foo ...
	MV bar ...
	CP foo ...
	[[ -d ... ]] && FAIL ... is a directory && return
	[[ $(cat foo 2>/dev/null) != foo ]] && FAIL top garbled -- $(cat foo 2>&1)
}
