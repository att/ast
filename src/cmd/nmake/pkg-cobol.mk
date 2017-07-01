/*
 * generic cobol package setup
 * specific setup by pkg-cobol-$(COBOL:B:S=.mk)
 */

.PACKAGE.cobol.dontcare := 1

COBOL = cobc
COBOLDIALECT =
COBOLFLAGS = $(COBOLDIALECT)
COBOLLIBRARIES =

COBOL.PLUGIN.LIBRARIES =

freeze COBOL

.SUFFIX.cob = .cob .COB .cbl .CBL
.SUFFIX.HEADER.cob = .cpy .CPY

.SCAN.cob : .SCAN
	I|COPY % |M$$(.INCLUDE.SUFFIX. cob)|
	I|\T COPY % |M$$(.INCLUDE.SUFFIX. cob)|
	I|\T \D COPY % |M$$(.INCLUDE.SUFFIX. cob)|
	#I|\D *. COPY % |M$$(.INCLUDE.SUFFIX. cob)| # this one is trouble for ^[[:space:]]*COPY sqlca.cbl$

$(.SUFFIX.cob:/^/.ATTRIBUTE.%/) : .SCAN.cob

.SOURCE.%.SCAN.cob : . $$(*.SOURCE$$(.SUFFIX.HEADER.cob:O=1)) $$(*.SOURCE.cob) $$(*.SOURCE)

.PKG.COBOL.INIT : .MAKE .VIRTUAL .FORCE .AFTER
	local F
	if ! "$(PATH:/:/ /G:X=$(COBOL):P=X)"
		error 3 $(COBOL): cobol compiler not found -- required to build $(.RWD.:-$(PWD:B))
	end
	F = pkg-cobol-$(COBOL:B:S=.mk)
	if ( F = "$(F:T=F)" )
		include + $(F)
	end
	COBOLFLAGS &= $(COBOLFLAGS:VA:V) $$(.INCLUDE. cob -I)
	if "$(-debug-symbols)" && ! "$(COBOLFLAGS:N=$(CC.DEBUG)|-g)"
		COBOLFLAGS := $(COBOLFLAGS:V:N!=$(CC.OPTIMIZE)|-O*)
	end

.PKG.COBOL.TEST : .MAKE .VIRTUAL .FORCE .IMMEDIATE
	if "$(".PROBE.INIT":A=.TRIGGERED)"
		make .PKG.COBOL.INIT
	else
		.PROBE.INIT : .PKG.COBOL.INIT
	end
