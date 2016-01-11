/*
 * system / subsystem support
 *
 * lhs is subsystem directory name
 * rhs is list of subsystem source file names relative to lhs
 * no* entries disable subsystem prerequisites
 * SS.MAIN is set to the first subsystem and its option table
 * controls all other subsystems
 *
 * rhs supports 2 { ... } grouping forms
 *
 *	{ lhs-identifier ... : rhs-file ... }
 *
 *		if any lhs-identifier is selected then rhs-file selected
 *
 *	{ rhs-file ... }
 *
 *		rhs-file always selected
 *
 * table options are controlled by specifiying no lhs
 * the default setting is in $(SS.TABLE)
 * to clear or reset a table option
 *
 *	:SYSTEM: nostate nomap format=lower
 *
 * no lhs and no rhs clears SS.MAIN
 */

set targetcontext

SS.MAIN =
SS.CURRENT =
SS.TABLE = state format=upper map=obsolete
SS.CALLER =
SS.EXCLUDE =
SS.INTERCEPT = : :: :LIBRARY:

":SYSTEM:" : .MAKE .OPERATOR .PROBEINIT
	if "$(<)"
		.ASSERT : ":_SS_ASSERT_:"
		.INIT : .SS.NOASSERT
		.SS.NOASSERT : .MAKE .FORCE .VIRTUAL
			.ASSERT : .DELETE ":_SS_ASSERT_:"
		if "$(>)" || "$(@:V)" || "$(<:N=*$(CC.SUFFIX.OBJECT))"
			SS.MAIN =
			eval
			$$(<) :SS: $$(>)
				$(@:V)
			end
		else
			SS.CURRENT := $(<)
		end
	else if "$(>)"
		local I N O P
		for I $(>)
			if I == "?(no)(format|map|state)?(=*)"
				O := $(I:/no//)
				SS.TABLE := $(SS.TABLE:N!=$(O)?(=*))
				if I != "no*"
					SS.TABLE += $(I)
				end
			else if I == "?(no)(caller|exclude|intercept)?([-+]=*)"
				O := $(I:/no//)
				N := SS.$(O:/[-+]*=.*//:F=%(upper)s)
				P := $(O:/[^-+=]*[-+]*=//)
				O := $(O:/$(P)//)
				T := $(O:/[-+]*=//)
				O := $(O:/$(T)//)
				if I == "no*"
					O = =
					P =
				else if ! O
					O = =
					P = 1
				end
				if O == "="
					$(N) := $(P)
				else if O == "+="
					if ! "$($(N):N=$$(P))"
						$(N) += $(P)
					end
				else
					$(N) := $($(N):N!=$$(P))
				end
			else
				error 1 $(I): invalid $(%) option
			end
		end
	else
		SS.MAIN =
		SS.CURRENT =
	end

":SS:" : .MAKE .OPERATOR .PROBEINIT
	local DD NN SS G I M N P R S T Y
	SS := $(<:O=1)
	DD := $(<:O=2)
	if ! DD
		DD = .
	else if DD == "-"
		DD := $(SS:D)
	else
		DD := $(DD:D)
	end
	if SS.MAIN
		SS.CURRENT := $(SS)
		if ! SS.ALL
			if "$(SS:D)" != "."
				Y := $($(SS.OPTIONS).$(SS:D:C,/,.,G))
				N := $(no$(SS.OPTIONS).$(SS:D:C,/,.,G)) $(NO$(SS.OPTIONS).$(SS:D:C,/,.,G))
			else
				Y := $($(SS.OPTIONS))
				N := $(no$(SS.OPTIONS)) $(NO$(SS.OPTIONS))
			end
			if ! "$(Y:N=$(SS:B))"
				if "$(N:N=$(SS:B))" || "$(Y:N=no$(SS:B))" || "$(Y:N=NO$(SS:B))"
					NN = 1
				end
			end
				N := $(no$(SS.OPTIONS).$(SS:C,/,.,G)) $(NO$(SS.OPTIONS).$(SS:C,/,.,G))
		end
		Y := $($(SS.OPTIONS).$(SS:C,/,.,G))
	else if "$(>)" || "$(@:V)" || "$(SS:N=*$(CC.SUFFIX.OBJECT))"
		SS.MAIN := $(SS)
		SS.CURRENT =
		if ( SS.OPTIONS = "$(.TABLE.INIT. $(SS.MAIN))" )
			SS.ALL =
		else
			SS.ALL = 1
		end
	else
		SS.CURRENT := $(SS)
	end
	if "$(@:V)" || "$(SS:N=*$(CC.SUFFIX.OBJECT))"
		G := $(SS)
		if ! "$(>)" && ! "$(@:V)"
			$(G) : .OBJECT$(CC.SUFFIX.OBJECT)
		end
	else
		G := $(SS)$(CC.SUFFIX.OBJECT)
	end
	if SS.EXCLUDE
		S = R
		for I $(N)
			if I == "-"
				break
			end
			if I == "{"
				if S != "R"
					error 3 "$(SS): nested {"
				end
				S = T
				T =
				P =
			else if I == ":"
				if S == "T"
					S = P
				else
					error 3 "$(SS): too many :'s"
				end
			else if I == "}"
				if S == "P"
					S = R
					if "$(N:N=$(T:/ /|/G))"
						R += $(P)
					end
				else if S == "T"
					S = R
				else
					error 3 "$(SS): unmatched }"
				end
			else
				$(S) += $(I)
			end
		end
		if S != "R"
			error 3 "$(SS): missing }"
		end
		P =
		for I $(R)
			if "$(I:A=.ATTRIBUTE)" || "$(I)" == "*=*"
				P += $(I)
			else
				S := $(I:D)
				if ! "$(I:A=.TARGET)"
					S := $(SS.CURRENT)/$(S)
				end
				if "$(I:S)"
					P += $(I:D=$(S):B:S)
				else
					P += $(I:D=$(S):B:S=$(CC.SUFFIX.OBJECT))
				end
			end
		end
		no$(SS.MAIN) : $(P)
		R =
		P =
	end
	S = R
	for I $(>) $(Y)
		if I == "-"
			break
		end
		if I == "{"
			if S != "R"
				error 3 "$(SS): nested {"
			end
			S = T
			T =
			P =
		else if I == ":"
			if S == "T"
				S = P
			else
				error 3 "$(SS): too many :'s"
			end
		else if I == "}"
			if S == "P"
				S = R
				I := $(SS.OPTIONS).$(SS:C,/,.,G)
				T := $(T:/ /|/G)
				while I == "*.*"
					if "$($(I):N=$(T))"
						R += $(P)
						break
					end
					I := $(I:B)
				end
			else if S == "T"
				S = R
				R += $(T)
				M += $(T)
			else
				error 3 "$(SS): unmatched }"
			end
		else
			$(S) += $(I)
		end
	end
	if S != "R"
		error 3 "$(SS): missing }"
	end
	if NN && ! SS.EXCLUDE
		if ! M
			return
		end
		R := $(M)
	end
	P =
	for I $(R)
		if "$(I:A=.ATTRIBUTE)" || "$(I)" == "*=*"
			P += $(I)
		else
			S := $(I:D)
			if ! "$(I:A=.TARGET)"
				S := $(SS.CURRENT)/$(S)
			end
			if "$(I:S)"
				P += $(I:D=$(S):B:S)
			else
				P += $(I:D=$(S):B:S=$(CC.SUFFIX.OBJECT))
			end
		end
	end
	eval
	$$(G) :: $$(P)
		$(@:V)
	end
	if SS.EXCLUDE && NN
		no$(SS.MAIN) : $(G)
	else
		if SS.CURRENT
			T := $(SS)
			while 1
				T := $(T:D)
				if T == "$(DD)"
					$(SS.MAIN) : $(G)
					break
				end
				T := $(T:D:B:S=$(CC.SUFFIX.OBJECT))
				$(T) : .OBJECT$(CC.SUFFIX.OBJECT) $(G)
				G := $(T)
			end
		end	
	end

":_SS_ASSERT_:" : .MAKE .OPERATOR
	if "$(SS.CURRENT)" && ! "$(SS:A=.IMMEDIATE)" && ( ! "$(%%:A=.MAKE|.IMMEDIATE)" || "$(%%)" == "$(SS.CALLER:/ /|/G)" ) && "$(%)" == "$(SS.INTERCEPT:/ /|/G))"
		local I T P
		for I $(<)
			if "$(I:A=.ATTRIBUTE|.TARGET)" && ! "$(I:D=$$(SS.CURRENT):B:S:A=.TARGET)"
				T += $(I)
			else
				T += $(I:D=$$(SS.CURRENT):B:S)
			end
		end
		for I $(>)
			if I == "*=*|/*" || "$(I:A=.ATTRIBUTE|.TARGET)"
				P += $(I)
			else
				P += $(I:D=$$(SS.CURRENT)/$$(I:D):B:S)
			end
		end
		eval
		$$(T) $(%) $$(P)
			$(@:V)
		end	
	else
		eval
		$$(<) $(%) $$(>)
			$(@:V)
		end
	end
