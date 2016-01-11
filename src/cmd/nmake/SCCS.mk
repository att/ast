/*
 * sccs metarule support
 */

":SCCS:" : .MAKE .OPERATOR

GET = get
GETFLAGS =

.SCCS.UNGET =

.SCCS.get : .FUNCTION
	local ( I O ) $(%)
	if "$(O:P=X)"
		return : $(O) ok
	end
	if ! .SCCS.UNGET
		.DONE : .SCCS.DONE
		.SCCS.DONE :
			$(RM) $(RMFLAGS) $(.SCCS.UNGET)
	end
	.SCCS.UNGET += $(O)
	return $(GET) $(GETFLAGS) $(I)

% : .TERMINAL s.%
	$(.SCCS.get $(>) $(<))
