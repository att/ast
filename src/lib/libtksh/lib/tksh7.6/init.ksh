typeset -A env

function env.get 
{
	[[ ${.sh.subscript} == "" ]] && return
	nameref v=${.sh.subscript}
	env[${.sh.subscript}]="$v"
}

function env.set
{
	[[ ${.sh.subscript} == "" ]] && return
	nameref v=${.sh.subscript}
	v="${.sh.value}"
}

function env.unset
{
	[[ ${.sh.subscript} == "" ]] && return
	nameref v=${.sh.subscript}
	unset v
}

argv0="$0"
argv="$(tcl_list "$@")"

tksh_info mode tcl

source $tcl_library/init.tcl

# PS1="Tksh $PS1"
# export TK_LIBRARY=$tcl_library/tk4.2
# tk_library=$TK_LIBRARY

: "${env[PWD]}"		# not sure why this is needed

tksh_info sideeffects on
