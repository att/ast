/*
 * package debug support
 *
 * @(#)debug-package (AT&T Research) 2011-09-07
 *
 * list package references and .SOURCE.* directories
 */

.MAKEINIT : .DEBUG.PACKAGE.INIT

.DEBUG.PACKAGE.ONLY = $(.PACKAGE.)

.DEBUG.PACKAGE.INIT : .MAKE .BEFORE
	if "$(~.ARGS)"
		.DEBUG.PACKAGE.ONLY := $(~.ARGS)
		.ARGS : .CLEAR
		:PACKAGE: - $(.DEBUG.PACKAGE.ONLY)
	end
	.ARGS : .DEBUG.PACKAGE

.DEBUG.PACKAGE : .MAKE
	local P S
	for P $(.DEBUG.PACKAGE.ONLY)
		if "$(PACKAGE_$(P))"
			print -f "%8s %-6s %s" $(P) "$(PACKAGE_$(P)_VERSION)" $(PACKAGE_$(P))
			print -f "                %s" -- $(PACKAGE_$(P)_INCLUDE)
			print -f "                %s" -- $(PACKAGE_$(P)_LIB)
			print -f "                %s" -- $("-l$(P)":T=F:P=A:B:S)
			if "$(CC.REQUIRE.$(P))"
				print -f "                %s" -- "$(CC.REQUIRE.$(P))"
			end
			print
		end
	end
	for S a h
		print .SOURCE.$(S)
		for P $(*.SOURCE.$(S):U:N!=.:T=F)
			print -f "                %s" -- $(P)
		end
		print
	end
