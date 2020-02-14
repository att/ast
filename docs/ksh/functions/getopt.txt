function getopt
{
	typeset c optstring=$1 options= sep=
	shift
	while	getopts $optstring c
	do	case $c in
		[:?])	
			exit 2
			;;
		*)
			options="$options$sep-$c"
			sep=' '
			if	[[ $optstring == *$c:* ]]
			then	options=" $options $OPTARG"
			fi
			#then	print -rn -- " -$c" "$OPTARG"
			#else	print -rn -- " -$c"
			;;
		esac
	done
	print -rn -- "$options"
	if	[[ ${@:$OPTIND-1} != -- ]]
	then	print -rn -- " --"
	fi
	if	[[ -n ${@:$OPTIND} ]]
	then	print -r -- " ${@:$OPTIND}"
	fi
}
