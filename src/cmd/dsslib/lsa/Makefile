:PACKAGE: ast

LICENSE = since=2002,author=ashaikh

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

lsa plugin=dss :LIBRARY: \
		ospf_can_lsa.c ospf_lsa.c ospf_lsag_lsa.c \
		ospf_lsar_lsa.c ospf_pmcoa_lsa.c ospf_text_lsa.c \
		lib.c -ldss

:TEST: dss

:: RELEASE \
	data/lsa.dat \
	data/lsa-be.dat \
	data/lsa-le.dat
