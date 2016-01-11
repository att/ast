/*
 * -lcs service support
 *
 * @(#)service (AT&T Research) 1997-11-11
 *
 * service [ type [ mode ] [ dir ] ] :SERVICE: prerequisites
 *
 * type defaults to tcp, mode defaults to null
 */

if ! SERVICEDIR
	SERVICEDIR = $(LIBDIR)/cs
end

SERVICESUFFIX = .svc

":SERVICE:" : .MAKE .OPERATOR
	local opt service type mode dir
	eval
	service = $(<:O=1)
	if ! ( type = "$(<:O=2:/,/ /G)" )
		type = tcp
	end
	for opt $(<:O>=3)
		if "$(opt:N=-*|*=*)"
			mode += $(opt)
		else
			dir := $(opt)
		end
	end
	$$(SERVICEDIR) :INSTALLDIR:
	if dir
		$(dir)/$(service) :INSTALL: preserve=1 $(service)$(SERVICESUFFIX)
	else
		for dir $(type)
			$$(SERVICEDIR)/$(dir)/$(service)/$(service)$(SERVICESUFFIX) :INSTALL: preserve=1 $(mode) $(service)$(SERVICESUFFIX)
		end
	end
	for dir $(type)
		$$(SERVICEDIR)/$(dir)/$(service) :INSTALLDIR: $(>:N=export|hosts)
	end
	:INSTALLDIR: $(service)$(SERVICESUFFIX)
	$(service)$(SERVICESUFFIX) :: $(>) -l$(SERVICEDIR:B)
	:ALL: $(service)$(SERVICESUFFIX)
	end
