/*
 * yacc %include file support
 *
 * @(#)yacc (AT&T Research) 1996-12-25
 *
 * %include in *.yy produces yacc output file *.y
 * %include suffix .yacc
 */

":yacc:" : .MAKE .OPERATOR

.SOURCE.%.yacc : $$(*.SOURCE.%.LCL.INCLUDE)

.SCAN.yy : .SCAN
	Q|/*|*/||C|
	Q|//||\\|LC|
	Q|"|"|\\|LQ|
	Q|'|'|\\|LQ|
	Q|\\|||CS|
	I| \% include "%"|A.LCL.INCLUDE|

.ATTRIBUTE.%.yy : .SCAN.yy

%.y : %.yy
	$(.YACC.INCLUDE.) $(>) > $(<)

.YACC.INCLUDE. : .FUNCTION
	local T V
	for T $(!$(>>))
		eval
		V += -e '/^[ 	]*%[ 	]*include[ 	][ 	]*"$$(T:P=U:C%/%\\/%G)"/ {$$("\n")r '$(T)'$$("\n")d$$("\n")}'
		end
	end
	return $(V:@?$$(SED) $$(V)?cat?)

if CC.HOSTTYPE != "win32.*"

.ATTRIBUTE.%.Y : .SCAN.yy

%.y : %.Y
	$(.YACC.INCLUDE.) $(>) > $(<)

end
