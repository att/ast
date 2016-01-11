/*
 * S specific metarules -- S source must have suffix .S
 */

SHOME := $(ADDDIRS:/:/ /G:X=S/.:T=F:O=1)
SINCLUDE = $(SHOME)/newfun/include

RATFOR = ratfor

.SOURCE.m : $(SINCLUDE)

.ATTRIBUTE.%.S : .SCAN.m4

(RATFOR) (RATFORFLAGS) : .PARAMETER

%.o : %.S (M4) (M4FLAGS) (RATFOR) (RATFORFLAGS) (F77) (F77FLAGS) (SHOME)
	$(RM) $(RMFLAGS) $(<:D=$(TMPDIR):B:S=.*)
	$(M4) $(M4FLAGS) $(SINCLUDE)/u/mach.m $(SINCLUDE)/ratfor.m $(>) > $(<:D=$(TMPDIR):B:S=.r)
	$(RATFOR) $(RATFORFLAGS) "-6&" < $(<:D=$(TMPDIR):B:S=.r) > $(<:D=$(TMPDIR):B:S=.f)
	$(F77) $(F77FLAGS) -c $(<:D=$(TMPDIR):B:S=.f)
	$(RM) $(RMFLAGS) $(<:D=$(TMPDIR):B:S=.*)

.HEADER.S : .USE $$(<:B:S=.m)
	$(RM) $(RMFLAGS) $(<)
	$(BINED) - $(*) <<'!'
	g/#.*/s///
	g/[' 	`']/s///g
	v/^define([A-Z][A-Z0-9]*,-*[0xX]*[0-9][0-9]*)$/d
	1,$s/define(\(.*\),\(.*\))/#define \1	(\2)/
	0a
	$("/")* definitions translated from $(*) *$("/")
	.
	w $(<)
	q
	!
