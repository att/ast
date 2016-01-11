/*
 * make service support
 */

STALE = 5m

/* get id target ... */

.GET : .FUNCTION
	local I ID
	ID := $(%:O=1)
	for I $(%:O>1)
		if "$(I:A=.EXISTS|.FAILED)"
			print $(ID) done $(I)
		else
			eval
			.NOTIFY.$(ID) : .MAKE .VIRTUAL .FORCE .REPEAT .AFTER
				print $(ID) done $$(<<)
			end
			$(I) : .NOTIFY.$(ID)
			run $(I)
		end
	end

.MAKEINIT : .STALE

.STALE : .MAKE .VIRTUAL .FORCE .REPEAT
	reset
	alarm $(STALE) .STALE
