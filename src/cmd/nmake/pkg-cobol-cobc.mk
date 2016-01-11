/*
 * open source cobol package setup
 */

COBOLFLAGS = -static $(COBOLDIALECT) -C $(-debug-symbols|"$(CCFLAGS:N=$(CC.DEBUG)|-g)":??$(CC.OPTIMIZE)?)
COBOLLIBRARIES = -lcob

.COBOL.INIT : .MAKE .VIRTUAL .FORCE .IGNORE
	$(COBOLLIBRARIES) : .DONTCARE
	LDFLAGS += $$(!:A=.SCAN.cob|.SCAN.sqb:@?$$(CC.EXPORT.DYNAMIC)??)
	LDLIBRARIES += $$(!:A=.SCAN.cob|.SCAN.sqb:@?$$(COBOLLIBRARIES)??)

for .S. $(.SUFFIX.cob)
	%.c %.c.h : %$(.S.) (COBOL) (COBOLDIALECT) (COBOLFLAGS) .COBOL.INIT
		$(COBOL) $(COBOLFLAGS) $(>)
end

":VARARGS:" : .MAKE .OPERATOR /* mfcobc specific */
