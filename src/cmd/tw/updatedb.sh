########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1989-2011 AT&T Intellectual Property          #
#                      and is licensed under the                       #
#                 Eclipse Public License, Version 1.0                  #
#                    by AT&T Intellectual Property                     #
#                                                                      #
#                A copy of the License is available at                 #
#          http://www.eclipse.org/org/documents/epl-v10.html           #
#         (with md5 checksum b35adb5213ca9657e911e9befb180842)         #
#                                                                      #
#              Information and Software Systems Research               #
#                            AT&T Research                             #
#                           Florham Park NJ                            #
#                                                                      #
#               Glenn Fowler <glenn.s.fowler@gmail.com>                #
#                                                                      #
########################################################################
: gnu updatedb wrapper for tw

codes_default="lib/find/codes"
dirs_default="/"
drop_default="/afs|/backup|/dev|/proc|/tmp|/usr/tmp|/var/tmp"
keep_default="/home|/usr/local|/usr/common"

COMMAND=updatedb
case `(getopts '[-][123:xyz]' opt --xyz; echo 0$opt) 2>/dev/null` in
0123)	ARGV0="-a $COMMAND"
	USAGE=$'
[-?
@(#)$Id: updatedb (AT&T Labs Research) 2003-11-14 $
]
'$USAGE_LICENSE$'
[+NAME?updatedb - generate locate pathname database]
[+DESCRIPTION?\bupdatedb\b generates the locate pathname database that is used
	by \blocate\b(1), \bfind\b(1), and \btw\b(1). Sufficient privilege
	is required to change the system locate database.]
[+?This implemenation is a script that generates a \btw\b(1) command that
	does all the work.]
[a:auto-home?Include the \bnis\b(1) \aauto.home\a auto mounter home directories
	in \b/home\b. NOTE: this causes the home directories to be mounted
	and may swamp the mount table.]
[d:directory|localpaths?Local directories to include in the database. If the
	first \adir\a is \b+\b then the default list is appended.]:
	["dir1 dir2 ...":='${dirs_default}$']
[i:ignore-errors?Omit inaccessible files and directory error messages.]
[k:keep?Directories to retain in the database; used to override
	\b--nocrossdevice\b. If any of the paths are symbolic links then they
	are followed. If the first \adir\a is \b+\b then the default list
	is appended.]:["dir1 dir2 ...":='${keep_default//\|/\ }$']
[l:local?Do not descend into non-local filesystem directories.]
[r:netpaths?Network directories to include in the database. Currently
	equivalent to \b--localpaths\b.]:["dir1 dir2 ..."]
[p:prunepaths|drop?Directories to exclude from the database. If the first \adir\a
	is \b+\b then the default list is appended.]:
	["dir1 dir2 ...":='${drop_default//\|/\ }$']
[o:output|codes?The path of the generated database.]:[dbfile:='${codes_default}$']
[P:public?Omit files that are not publicly readable and directories that
	are not publicly searchable.]
[u:user|netuser?The user id used to search directories.]:[user]
[m:dir-format?Generate a database similar to \b--gnu-format\b, except that
	directories are marked for efficient implementations of \bfind\b(1)
	and \btw\b(1). This is the default database format.]
[g:gnu-format?Generate a machine independent gnu \blocate\b(1) compatible
	database.]
[O:old-format?Generate a database compatible with the obsolete
	\bfastfind\b(1). This format has a machine dependent encoding.]
[D:depth?Limit the directory traversal depth to \alevel\a.]#[level]
[X!:crossdevice?Retain subdirectories that cross device boundaries.]
[n:show?Show the underlying the \btw\b(1) command but do not execute.]
[+FILES]{[+'${codes_default}$'?Default locate database on \b\$PATH\b.]}
[+CAVEATS?If you run \bupdatedb\b as root then protected directory
	contents may be visible to everyone via the database. Use the
	\b--public\b option to limit the database to publically visible
	files and directories.]
[+SEE ALSO?\blocate\b(1), \bfastfind\b(1), \bfind\b(1), \bnis\b(1), \btw\b(1)]
'
	;;
*)	ARGV0=""
	USAGE="aiglnOPXd:['dir1 dir2 ...']k:['dir1 dir2 ...']o:[dbfile]p:['dir1 dir2 ...']r:['dir1 dir2 ...']u:[user]D:[level]"
	;;
esac

usage()
{
	OPTIND=0
	getopts $ARGV0 "$USAGE" OPT '-?'
	exit 2
}

cross=1
depth=
dirs=
drop=
format="-"
keep=
options="-P"
output="-"
public=
show=
su=
while	getopts $ARGV0 "$USAGE" OPT
do	case $OPT in
	a)	dirs="$dirs `{
			ypcat auto.home | sed 's,.*/,-d /home/,'
			ls /home | sed 's,^,-d /home/,'
		} | sort -u`"
		;;
	d|r)	case $OPTARG in
		"+"|"+ "*)	;;
		*)		dirs_default= ;;
		esac
		for dir in $OPTARG
		do	case $dir in
			+)	;;
			*)	dirs="$dirs -d $dir" ;;
			esac
		done
		;;
	g)	format=gnu
		;;
	i)	options="$options -i"
		;;
	k)	case $OPTARG in
		"+"|"+ "*)	;;
		*)		keep_default= ;;
		esac
		for dir in $OPTARG
		do	case $dir in
			+)	;;
			*)	case $keep in
				?*)	keep="$keep|$dir" ;;
				*)	keep="$dir" ;;
				esac
				;;
			esac
		done
		;;
	l)	options="$options -l"
		;;
	m)	format="-"
		;;
	o)	output=$OPTARG
		;;
	n)	show="print --"
		;;
	p)	case $OPTARG in
		"+"|"+ "*)	;;
		*)		drop_default= ;;
		esac
		for dir in $OPTARG
		do	case $dir in
			+)	;;
			*)	case $drop in
				?*)	drop="$drop|$dir" ;;
				*)	drop="$dir" ;;
				esac
				;;
			esac
		done
		;;
	u)	su="su $OPTARG"
		;;
	D)	depth=$OPTARG
		;;
	O)	format=old
		;;
	P)	public=1
		;;
	X)	cross=
		;;
	*)	usage
		;;
	esac
done
shift $OPTIND-1
case $# in
0)	;;
*)	usage ;;
esac

for dir in $dirs_default
do	dirs="$dirs -d $dir"
done
for dir in $drop_default
do	case $drop in
	"")	drop="$dir" ;;
	*)	drop="$drop|$dir" ;;
	esac
done
for dir in $keep_default
do	case $keep in
	"")	keep="$dir" ;;
	*)	keep="$keep|$dir" ;;
	esac
done

skip=
follow=
alt="
		"
sep="
	"
case $public in
?*)	case $skip in
	?*)	skip="$skip ||" ;;
	esac
	skip="${skip}${alt}!(mode & 'o+r') ||${alt}type == DIR &&${alt}!(mode & 'o+x')"
	;;
esac
case $depth in
?*)	case $skip in
	?*)	skip="$skip ||" ;;
	esac
	skip="${skip}${alt}level > $depth"
	;;
esac
case $cross in
"")	case $skip in
	?*)	skip="$skip ||" ;;
	esac
	skip="${skip}${alt}fstype != '/'.fstype"
	case $keep in
	?*)	skip="${skip} &&${alt}path != '($keep)?(/*)'"
	esac
	;;
esac
case $drop in
?*)	case $skip in
	?*)	skip="$skip ||" ;;
	esac
	skip="${skip}${alt}path == '($drop)?(/*)'"
	;;
esac
expr=
case $skip in
?*)	expr="if (${skip}${alt})${alt}	status = SKIP;" ;;
esac
case $keep in
?*)	case $expr in
	?*)	expr="${expr}${sep}else " ;;
	esac
	expr="${expr}if (${alt}type == LNK &&${alt}level < 3 &&${alt}path == '($keep)?(/*)'${alt})${alt}	status = FOLLOW;"
	;;
esac
case $expr in
?*)	case $show in
	"")	expr="-e${sep}${expr}${sep}" ;;
	*)	expr="-e \"${sep}${expr}\"${sep}" ;;
	esac
	;;
esac
$show $su tw $options -F "$output" -G "$format" "$expr" $dirs
