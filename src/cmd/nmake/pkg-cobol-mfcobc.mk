/*
 * microfocus cobol package setup
 */

COBOLFLAGS = $(COBOLDIALECT) $(-debug-symbols|"$(CCFLAGS:N=-g|$(CC.DEBUG))":?$(CC.DEBUG)?$(CC.OPTIMIZE)?)
COBOLLIBRARIES = -lmfcob

COBOL.ARCH := $(HOSTTYPE:N=*-64*:+64)
COBOL.PLUGIN.LIBRARIES := -lcobrts$(COBOL.ARCH) -lcobcrtn$(COBOL.ARCH) -lcobmisc$(COBOL.ARCH)

if "$(COBDIR:P=X)"
.SOURCE.h : $(COBDIR)/include
end

/*
 * scope CC.* probe vars for cobol links
 */

.COBOL.LD. : .FUNCTION
	if "$(!!:A=.SCAN.cob|.SCAN.sqb:O=1)$(!!:N=?(*/)libcob.h:O=1)$(.PACKAGE.cobol.option.always:N!=0)"
		CC.REQUIRE.ast = -lc -last
		CC.DYNAMIC =
		CC.EXPORT.DYNAMIC =
		CC.SHARED = -Z
		return $(COBOL)
	else
		CC.DYNAMIC := $(.CC.DYNAMIC.ORIGINAL:V)
		CC.EXPORT.DYNAMIC := $(.CC.EXPORT.DYNAMIC.ORIGINAL:V)
		CC.SHARED := $(.CC.SHARED.ORIGINAL:V)
		return $(.$(%).ORIGINAL)
	end

if .PACKAGE.cobol.option.always
.MAKEINIT : .COBOL.INIT.AFTER
.COBOL.INIT.AFTER : .MAKE .VIRTUAL .FORCE .AFTER .COBOL.INIT
end

.COBOL.INIT : .MAKE .VIRTUAL .FORCE .IGNORE
	include - probe-mfcobc.mk
	.CCLD.ORIGINAL := $(CCLD:V)
	.CC.SHARED.LD.ORIGINAL := $(CC.SHARED.LD:V)
	.CC.DYNAMIC.ORIGINAL := $(CC.DYNAMIC:V)
	.CC.EXPORT.DYNAMIC.ORIGINAL := $(CC.EXPORT.DYNAMIC:V)
	.CC.SHARED.ORIGINAL := $(CC.SHARED:V)
	CCLD = $(.COBOL.LD. CCLD)
	CC.SHARED.LD = $(.COBOL.LD. CC.SHARED.LD)
	CCFLAGS += $(COBOL.CCFLAGS)
	LDFLAGS += $$("$$(!:A=.SCAN.cob|.SCAN.sqb:O=1)$$(!:N=?(*/)libcob.h:O=1)":@?$$(COBOLFLAGS:N=-[lLsO]*) $$(CC.EXPORT.DYNAMIC)??)
	LDLIBRARIES += $$("$$(!:A=.SCAN.cob|.SCAN.sqb:O=1)$$(!:N=?(*/)libcob.h:O=1)":@?$$(COBOLLIBRARIES)??)
	$(COBOLLIBRARIES) : .DONTCARE

for .S. $(.SUFFIX.cob)
	%.o : %$(.S.) (COBOL) (COBOLDIALECT) (COBOLFLAGS) .COBOL.INIT
		$(COBOL) -c $(COBOLFLAGS) $(>)
end

.COBTRACE : .MAKE .LOCAL
	COBOLFLAGS += -x

":COBTRACE:" : .MAKE .OPERATOR
	$(>:B:S=.o) : .COBTRACE

":COBWATCH:" : .MAKE .OPERATOR
	local T
	T := $(<:@/ /,/G)
	eval
		.COBWATCH.$(T) : .MAKE .LOCAL
			COBOLFLAGS += -w $(T)
	end
	$(>:B:S=.o) : .COBWATCH.$(T)

OBJCOPY = objcopy
OBJCOPYFLAGS =

.COBOL.DEMANGLE : .VIRTUAL .FORCE .REPEAT .AFTER
	$(OBJCOPY) $(OBJCOPYFLAGS) --redefine-sym  $(%%:B:/#/\\$23/)=$(%%:B) $(<<) 1.$(tmp).o &&
	$(MV) 1.$(tmp).o $(<<)

%.o : %.c $$(<:B:N=*#*:+.COBOL.DEMANGLE)

VACC = varargcc
VACCFLAGS = $(CC) $(CCFLAGS)

.VACC : .USE
	$(VACC) $(VACCFLAGS) -c $(*)

":VARARGS:" : .MAKE .OPERATOR
	local I
	for I $(>:G=%.o)
		$(I) : .VACC
	end
