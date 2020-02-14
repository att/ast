typeset -A Keytable
trap 'eval "${Keytable[${.sh.edchar}]}"' KEYBD
function vi_keybind
{
	keybind $'\E[A' k # Up key
	keybind $'\E[B' j # Down key
	keybind $'\E[C' l # Right key
	keybind $'\E[D' h # Left key
	keybind $'\t' '\' # Tab for command-line completion
}
