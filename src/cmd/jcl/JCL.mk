/*
 * jcl control-m rules
 */

JCL = jcl
JCLROOT = $(PACKAGEROOT)
JCLINCLUDE = $(JCLROOT)/jcl
JCLRUN = $(JCLROOT)/run
JCLFLAGS = -sv -I$(JCLINCLUDE)
JCLEVENTS = $(JCLRUN)/events-$(JCLROOT:B:S)

if ! JCL_AUTO_ampersand
JCL_AUTO_ampersand	= 01
end
if ! JCL_AUTO_at
JCL_AUTO_at		= 01
end
if ! JCL_AUTO_pound
JCL_AUTO_pound		= 01
end

":JCL:" : .MAKE .OPERATOR
	.INIT : .JCL.INIT
	.JCL.INIT : .MAKE .VIRTUAL .FORCE
		if COSHELL == "$(SHELL)"
			COSHELL =
		end
		:COSHELL: service=event db=$(JCLEVENTS) coshell $(JCLGROUP:+group=$(JCLGROUP))

.SOURCE : $(JCLINCLUDE)

.JOB : .USE .VIRTUAL
	$(~:N=*=*) $(~:T=E) $(JCL) $(JCLFLAGS) $(*)

.EVENT.WAIT : .USE .VIRTUAL .FORCE .IGNORE
	event wait $(<)

.EVENT.RAISE : .USE .VIRTUAL .FORCE .IGNORE
	event raise $(<:/.RAISE$//)$(JCL_AUTO_FRAGMENT_INDEX:+$(~$(<<):N=\(JCL_AUTO_FRAG\):+-$(JCL_AUTO_FRAG)))

.EVENT.DELETE : .USE .VIRTUAL .FORCE .IGNORE
	event clear $(<:/.DELETE$//)

.binding. =

.BIND.% : .FUNCTION
	local X
	if ! "$(.binding.)" && "$(%)" != ".*"
		.binding. = 1
		if "$(%:B:S)" == "*[[:lower:]]*"
			X := $(%:D:B=$(%:B:S:F=%(upper)s))
			if X = "$(X:T=F)"
				.binding. =
				return $(X)
			end
		end
		if "$(%:B:S)" == "*[[:upper:]]*"
			X := $(%:D:B=$(%:B:S:F=%(lower)s))
			if X = "$(X:T=F)"
				.binding. =
				return $(X)
			end
		end
		.binding. =
	end

.INTERRUPT.URG : .FUNCTION
	query
	return continue

.INTERRUPT.USR1 : .FUNCTION
	query - jobs
	return continue

.INTERRUPT.USR2 : .FUNCTION
	query - blocked
	return continue
