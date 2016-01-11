/*
 * normalize local -l* library conventions
 *
 * L [ G11 ... G1n ... [ - Gg1 ... Ggn ] ] :MAPLIB: T1.c ... Tn.c
 *
 * if Giji not specified then G11 == L
 * the first Ti.c that compiles/links with group -lGi1 ... -lGin
 * but does not compile/link with no libraries maps
 * -lL to to require -lGi1 ... -lGin
 * otherwise -lL is not required and maps to "no library required"
 */

.MAP.SRC.OBJ. : .FUNCTION
	local I R
	for I $(%)
		R += $(I) $(I:B:S=$(CC.SUFFIX.OBJECT))
	end
	return $(R)

":MAPLIB:" : .MAKE .OPERATOR
	local L P
	L := $(<:B:O=1)
	if ! ( P = "$(<:B:O>1)" )
		P := $(L)
	end
	$(LIBDIR)/lib/$(L) :INSTALL: $(L).req
	eval
	$(L).req : (CC) $$(>)
		set -
		r='-'
		set '' '' $$(.MAP.SRC.OBJ. $$(*))
		while	:
		do	shift 2
			case $# in
			0)	break ;;
			esac
			src=$1
			obj=$2
			if	$$(CC) -c $src
			then	g=
				for p in $(P) -
				do	case $p in
					-)	if	$$(CC) -o $$(<:B:S=.exe) $obj $g
						then	$$(CC) -o $$(<:B:S=.exe) $obj || {
								r="$g"
								break 2
							}
						fi
						g=
						;;
					*)	g="$g -l$p"
						;;
					esac
				done
			fi
		done >/dev/null 2>&1
		echo " $r" > $$(<)
		rm -f $$(<:B:S=.exe) $$(*:B:S=$$(CC.SUFFIX.OBJECT))
	end
