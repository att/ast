/*
 * debug support
 *
 * @(#)debug (AT&T Research) 2010-02-14
 *
 * *.i from *.c
 */

set --nonativepp

CCIFLAGS = $(CC.ALTPP.FLAGS) $(CCFLAGS:N=-[DIU]*) $(&$(<:B:S=.o):T=D)
CCISCOPE =  $(~$(<:B:S=.o):N=*=*:Q)

.CCDEFINITIONS. : .FUNCTION
	if ! $(-nativepp:-0) || "$(CC.DIALECT:N=LIBPP)"
		return -D-d
	end
	return $(CC.DIALECT:N=-dD)

for .S. $(.SUFFIX.c) $(.SUFFIX.C)

	%.i : %$(.S.) .ALWAYS $$(CCISCOPE)
		$(CC) $(CCIFLAGS) -E $(.CCDEFINITIONS.) $(>) > $(<)

	%.inc : %$(.S.) .ALWAYS $$(CCISCOPE)
		$(CPP) $(CCIFLAGS) -H $(>) > /dev/null 2> $(<)

	%.s : %$(.S.) .ALWAYS $$(CCISCOPE)
		$(CC) $(CCIFLAGS) -S $(>)

end
