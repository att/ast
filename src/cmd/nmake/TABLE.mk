/*
 * table ... :TABLE: [class=name] [format={lower,upper}]
 *		[ignore=pattern] [map[=map]] [name=name]
 *		[novariables] [options] [state]
 *
 * if map==1 then do $(.TABLE.MAP. $(table) [name=name]) otherwise
 * define variables in table name of the form [(class|table)_]name=1[value]
 * format=upper[lower] converts var names to upper[lower] case
 * ignore=pattern to ignore matching .TABLE.SET. generated variables
 * set TABLE.list=1 or TABLE.list.name=1 to list the table variables
 * if state==1 then define state variables rather than normal variables
 * if noassign then variables not defined
 */

":TABLE:" : .MAKE .OPERATOR
	local I
	for I $(<)
		local table=$(I) class format ignore map name novariables options list state $(~$(I)) $(>)
		if "$(@$(I):V)"
			if name
				name := name=$(name)
			end
			if map == "1"
				: $(.TABLE.MAP. $(table) $(name))
			else
				if TABLE.list || "$(TABLE.list.$(table))"
					list = 1
				end
				if class == "1"
					class := $(table)_
				else if class
					class := $(class)_
				end
				if format
					format := F=%($(format))s
				end
				format := :$(format)
				if options
					options := options
				end
				if novariables
					state = -
				else if state
					state = ==
				else
					state = =
				end
				table := $(.TABLE.INIT. $(table) $(options) $(name))
				if map
					map := $(.TABLE.MAP. $(map) $(name))
				end
				: $(.TABLE.SET. $(table) "$(class)" $(format) "$(state)" "$(map)" "$(ignore)" "$(list)")
			end
		else if name
			if map
				map = MAP.
			end
			.TABLE.$(map)$(table) := $(name)
		end
	end

/*
 * :TABLE: inner loop
 */

.TABLE.SET. :FUNCTION:
	local I J M N V (row name format assign map ignore list) $(%)
	local class $($(row).)
	if class
		name := $(class)_
	end
	for I $($(row))
		if I == "*=*"
			V := $(I:/[^=]*=//)
			I := $(I:/=.*)
		else
			V = 1
		end
		if I == "-"
			continue
		end
		N := $("$(name)$(I)":/[^a-zA-Z_0-9]/_/G:$(format))
		if N != "$(ignore)"
			if map && ( M = "$($(map).$(N))" )
				M := $(M:/,/ /G)
			end
			if assign != "-"
				for J $(M|N)
					if J != "-"
						eval
							$(J) $(assign) $(V)
							if list
								print $(J) $(assign) $(V)
							end
						end
					end
				end
			end
			: $(.TABLE.SET. $(row).$(I) $(N)_ "$(format)" "$(assign)" "$(map)" "$(ignore)" "$(list)")
		end
	end

/*
 * $(.TABLE.INIT. table [name=name] [options])
 *
 * name {
 *	[no|NO]op1[=value]
 *	[no|NO]op2[=value] {
 *		...
 *	}
 * }
 *
 * convert the table in action to name[name]=value
 * if options then op names matching no* are placed in noname[name]=value
 * if options then op names matching NO* are placed in NOname[name]=value
 * TABLE.all=1 or TABLE.all.name=1 enables all no* entries
 * table data name returned
 */

.TABLE.INIT. :FUNCTION:
	local T name options table=$(%)
	if ( T = "$(.TABLE.$(table))" )
		return $(T)
	end
	if "$(@$(table):V)"
		local I K P X all level=0 no=
		if TABLE.all || "$(TABLE.all.$(table))"
			all = 1
		end
		T := $(name|table)
		.TABLE.$(table) := $(T)
		.TABLE.options.$(level). := $(options)
		.TABLE.no.$(level). :=
		for I $(@$(table))
			if I == "{"
				let level = level + 1
				.TABLE.options.$(level). := $(options)
				.TABLE.no.$(level). := $(no)
				if options
					if P == "no*"
						P := $(P:/..//)
						if ! all
							no = no
							$(no)$(T) += $(P)
						end
					else if P == "NO*"
						P := $(P:/..//)
						no = NO
						$(no)$(T) += $(P)
					end
				end
				T := $(T).$(P:/=.*//)
				P :=
			else if I == "}"
				if T == "*."
					eval
					$(P:N=options=*)
					end
					let level = level - 1
					.TABLE.options.$(level). := $(options)
					.TABLE.no.$(level). := $(no)
					let level = level + 1
					if ! options && no
						T := $(T:/\.\([^.]*\)\.$/.$(no)\1./)
						no :=
					end
				else if no
					$(T) :=
					K := 0
				else
					if options
						if P == "-|no*"
							P := $(P:/..//)
							no$(T) += $(P)
							if ! all && P != "-"
								P :=
							end
						else if P == "NO*"
							P := $(P:/..//)
							NO$(T) += $(P)
							P :=
						end
					end
					$(T) += $(P)
					K := 1
				end
				P := $(T)
				T := $(T:/\.[^.]*$//)
				if K
					$(T) += $(P:/.*\.//)
				end
				P :=
				let level = level - 1
				options := $(.TABLE.options.$(level).)
				no := $(.TABLE.no.$(level).)
			else
				if options
					if P == "-|no*"
						P := $(P:/..//)
						no$(T) += $(P)
						if ! all && P != "-"
							P :=
						end
					else if P == "NO*"
						P := $(P:/..//)
						NO$(T) += $(P)
						P :=
					end
				end
				$(T) += $(P)
				P := $(I)
			end
		end
		if options
			if P == "no*"
				P := $(P:/..//)
				no$(T) += $(P)
				if ! all
					P :=
				end
			else if P == "NO*"
				P := $(P:/..//)
				NO$(T) += $(P)
				P :=
			end
		end
		$(T) += $(P)
		clear $(table)
	end
	return $(T)

/*
 * $(.TABLE.MAP. map [name=name])
 *
 * name {
 *	old	new[,new...]|-
 *	...
 * }
 *
 * convert the map in name's action to name[name].old=new
 * map data name returned
 */

.TABLE.MAP. :FUNCTION:
	local T name map=$(%)
	if ( T = "$(.TABLE.MAP.$(map))" )
		return $(T)
	end
	if "$(@$(map):V)"
		local I P
		T := $(name|map)
		.TABLE.MAP.$(map) := $(T)
		for I $(@$(map))
			if P
				$(T).$(P) := $(I)
				P =
			else
				P := $(I)
			end
		end
		clear $(map)
	end
	return $(T)
