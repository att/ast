typeset -A Keytable
trap 'eval "${Keytable[${.sh.edchar}]}"' KEYBD
function emacs_keybind
{
	keybind $'\E[A' $'\020'	# Up key
	keybind $'\E[B' $'\016'	# Down key
	keybind $'\E[C' $'\06'	# Right key
	keybind $'\E[D' $'\02'	# Left key
	keybind $'\E[H' $'\01'	# Home key
	keybind $'\E[Y' $'\05'	# End key
	keybind $'\t' $'\E\E'	# Tab for command-line completion	
}
