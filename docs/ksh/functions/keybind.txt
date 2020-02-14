typeset -A Keytable
trap 'eval "${Keytable[${.sh.edchar}]}"' KEYBD
function keybind # key action
{ 
	typeset key=$(print -f "%q" "$2")
	case $# in
	2)	Keytable[$1]='.sh.edchar=${.sh.edmode}'"$key"
		;;
	1)	unset Keytable[$1]
		;;
	*)	print -u2 "Usage: $0 key [action]"
		;;
	esac
}
