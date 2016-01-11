/*
 * coshell support
 *
 * @(#)coshell (AT&T Research) 2006-08-11
 *
 * :COSHELL: name=value ...
 */

":COSHELL:" : .MAKE .OPERATOR
	local A AX C D I R S SX H=local P=fdp
	for I $(>)
		if I == "/*"
			C := $(I)
		else if I == "connect=*"
			C := $(I:/connect=//)
		else if I == "coshell?(=*)"
			R := 1
		else if I == "db=*"
			D := $(I)
		else if I == "fdp?(=*)"
			P := fdp
		else if I == "group=*" || I == "other" || I == "user?(=*)"
			A := $(I)
		else if I == "host=*"
			H := $(I:/host=//)
		else if I == "local?(=*)"
			H := local
		else if I == "service=*"
			S := $(I)
		else if I == "share?(=*)"
			H := share
		else if I == "tcp?(=*)"
			P := tcp
		else if I == "trust?(=*)"
			AX += $(I:/=.*//)
		else
			SX += $(I)
		end
	end
	if ! COSHELL
		if C || ! A && R
			COSHELL := coshell $(C) $(CX)
		elif A
			if AX
				AX := /$(AX:@C, ,/,G)
			end
			COSHELL := coshell /dev/$(P)/$(H)/coshell/$(A)$(AX) $(CX)
		end
	end
	if S
		if D
			S := $(S):$(D)
		end
		COSHELL_OPTIONS += $(S) $(SX)
	end
