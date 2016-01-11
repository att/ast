/*
 * Glenn Fowler
 * AT&T Research
 *
 * scan strategies -- oh no, another little language
 *
 * except for .SCAN.c, which must differentiate "..." from <...>,
 * .SOURCE.%.SCAN.<lang> should specify the binding dirs
 */

.SCANRULES.ID. = "@(#)$Id: Scanrules (AT&T Research) 2010-07-22 $"

/*
 * $(.INCLUDE. <lang> [<flag>])
 *
 * return <flag> option list for .SCAN.<lang>
 * .SCAN.c is handled in .MAKEINIT
 */

.INCLUDE. : .FUNCTION
	return $(*.SOURCE.%.SCAN.$(%:O=1):I=$$(!$(**:O=1):P=D):$(%:O=2:?/^/$$(%:O=2)/))

.LCL.INCLUDE : .ATTRIBUTE
.PFX.INCLUDE : .ATTRIBUTE .TERMINAL
.STD.INCLUDE : .ATTRIBUTE

.ACCEPT .IGNORE .RETAIN : .LCL.INCLUDE .PFX.INCLUDE .STD.INCLUDE

/*
 * .PFX.INCLUDE stuff needed for C #include "..." compatibility
 * --noprefix-include to turn off the hack
 */

.BIND.%.PFX.INCLUDE : .FUNCTION
	if "$(%:A=.DONTCARE)"
		$(%:B:S) : .DONTCARE
	end
	return $(%:B:S)

.PREFIX.INCLUDE. : .FUNCTION
	if "$(%%)" != "/*" && "$(<<:O=1:P=U)" != "." && ( "$(-target-context)" || "$(-recurse)" == "combine" && "$(%%:P=U)" != "*/*" || "$(<<:A:A=.SCAN:O=1)" == "$(<<<:A:A=.SCAN:O=1)" && "$(%%:P=U:D)" == ".|../*" )
		$(%%) : .LCL.INCLUDE
		if ! "$(%%:T=XG)"
			local B
			B := $(<<`;O=1;P=U;D;B=$$(%%);P=C)
			if B == "../*"
				B := $(<<<:O=1:P=U:D)/$(B)
				B := $(B:P=C)
			end
			$(B) : .PFX.INCLUDE
			if "$(%%:A=.DONTCARE)"
				$(B) : .DONTCARE
			end
			return $(B)
		end
	end
	return $(%%)

.PREFIX.SOURCE. : .FUNCTION
	if ! "$(-prefix-include)" && ! "$(-target-context)"
		local P S
		P := $(!$(%):A=.PFX.INCLUDE)
		if S = "$(P:P=U:D:U:N!=.)"
			return $(P:P=D:X=$(S):T>FD:U)
		end
	end

.SOURCE.%.LCL.INCLUDE : .FORCE $$(*.SOURCE.c) $$(*.SOURCE) $$(*.SOURCE.h)

.SOURCE.%.STD.INCLUDE : .FORCE $$(*.SOURCE.h)

.SCAN.c : .SCAN
	O|S|
	Q|/*|*/||C|
	Q|//||\\|LC|
	Q|"|"|\\|LQ|
	Q|'|'|\\|LQ|
	Q|\\|||CS|
	D| \# define %|
	B| \# if|
	E| \# endif|
	I| \# include <%>|A.STD.INCLUDE|R$$(%:P=U:D:S:N=.:?$$$(%)??):.TERMINAL|
	I| \# include "%"|A.LCL.INCLUDE|R$$(%:P=U:D:S:N=.:?$$$(%)??):.TERMINAL|M$$(.PREFIX.INCLUDE.)|
	I| \# pragma library "%"|A.VIRTUAL|A.ACCEPT|M.LIBRARY.$$(%)|

$("$(.SUFFIX.c) $(.SUFFIX.C) .h .S":/^/.ATTRIBUTE.%/) : .SCAN.c

.SCAN.f : .SCAN
	I| include '%'|
	I| INCLUDE '%'|

$(.SUFFIX.f:/^/.ATTRIBUTE.%/) : .SCAN.f

.SOURCE.%.SCAN.m4 : $$(*.SOURCE.m4) $$(*.SOURCE) $$(*.SOURCE.h)

.SCAN.m4 : .SCAN
	O|M|
	I|%|

.SCAN.mk : .SCAN
	Q|/*|*/||CW|
	Q|"|"|\\|LQ|
	Q|'|'|\\|LQ|
	Q|\\|||CS|
	B|if|
	B| \# if|
	E|end|
	E| \# endif|
	I|include - "%"|A.DONTCARE|M$$(%)|
	I|include - %|A.DONTCARE|M$$(%)|
	I|include "%"|M$$(%)|
	I|include %|M$$(%)|
	I| \# include <%>|A.STD.INCLUDE|
	I| \# include "%"|A.LCL.INCLUDE|M$$(.PREFIX.INCLUDE.)|

.ATTRIBUTE.%.mk : .SCAN.mk

.SCAN.nroff : .SCAN
	I|.so % |A.DONTCARE|
	I|,so % |A.DONTCARE|
	I|.BP % |M$$(%:/\([\-,0-9]*\)$//)|A.DONTCARE|
	I|,BP % |M$$(%:/\([\-,0-9]*\)$//)|A.DONTCARE|

.SCAN.p : .SCAN
	Q|(*|*)||C|
	Q|{|}||C|
	Q|"|"|\\|LQ|
	I| \# include '%'|

.SCAN.r : .SCAN
	I| include '%'|
	I| include "%"|
	I| include % |
	I| INCLUDE '%'|
	I| INCLUDE "%"|
	I| INCLUDE % |

$(.SUFFIX.r:/^/.ATTRIBUTE.%/) : .SCAN.r

.SCAN.sh : .SCAN
	O|S|
	Q|#||\\|LCW|
	Q|'|'||Q|
	Q|$'|'|\\|Q|
	Q|\\|||QS|

.ATTRIBUTE.%.sh : .SCAN.sh

.BIND.%.SCAN.sql : .FUNCTION
	if "$(%:N!=*.h)"
		return $(%).h
	end

.SOURCE.%.SCAN.sql : .FORCE $$(*.SOURCE.sql) $$(*.SOURCE.h)

.SCAN.sql : .SCAN
	Q|/*|*/||C|
	Q|//||\\|LC|
	Q|"|"|\\|LQ|
	Q|'|'|\\|LQ|
	Q|\\|||CS|
	I| EXEC SQL include "%"|
	I| EXEC SQL include % |M$$(%:/;.*//)|
	I| \## include "%"|
	I| \## include % |M$$(%:/;.*//)|
	I| $ include <%> |
	I| $ include "%" |
	I| $ include '%' |
	I| $ include % |M$$(%:/;.*//)|

.SCAN.exec.sh : .SCAN
	$(@.SCAN.sh)
	I| : include@ % |OX|
	I| . % |A.DONTCARE|M$$(%:C%.*[`$&].*%%:C%['"]%%:C%["']$%%)|
	I| *< % |A.DONTCARE|M$$(%:C%.*[`$&].*%%:C%['"]%%:C%["']$%%)|

.ATTRIBUTE.features/%.sh : .SCAN.exec.sh

.INCLUDE.SUFFIX. : .FUNCTION
	local F L Q S T ignorecase
	for L $(%)
		if L == "--ignorecase"
			ignorecase = 1
		elif L != "--*"
			break
		end
	end
	F := $(%%:/ .*//:/\.$//:/'\(.*\)'/\1/)
	Q := $("\"")
	if F == "$(Q)*$(Q)"
		F := $(F:/^$(Q)//:/$(Q)$//)
	end
	if ! "$(F:S)"
		for S $(.SUFFIX.HEADER.$(L)) $(<<:S)
			T := $(F)$(S)
			$(T) : .SCAN.$(L)
			if "$(T:T=F)"
				F := $(T)
				break
			end
			if ignorecase
				if T != "*[[:lower:]]*"
					Q := $(T:F=%(lower)s)
					$(Q) : .SCAN.$(L)
					if "$(Q:T=F)"
						F := $(Q)
						break
					end
				elif T != "[*[[:upper:]]*"
					Q := $(T:F=%(upper)s)
					$(Q) : .SCAN.$(L)
					if "$(Q:T=F)"
						F := $(Q)
						break
					end
				end
			end
		end
	end
	return $(F)

.SCAN.iffe : .SCAN
	I| include % |
	I| inc % |
	I| set stdio % |

.ATTRIBUTE.features/% : .SCAN.iffe

.ATTRIBUTE.%.iffe : .SCAN.iffe
