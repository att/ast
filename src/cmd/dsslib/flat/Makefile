:PACKAGE: ast

LICENSE = since=2002,author=gsf

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

flat plugin=dss static :LIBRARY: flat.c lib.c -ldss

:TEST: dss

:: RELEASE \
	data/cpy-bin.dat      data/cpy-bin.dss      data/cpy-str-bin.dss \
	data/cpy-str-txt.dss  data/cpy-txt.dat      data/cpy-txt.dss	\
	data/e-txt.dss        data/ff-bin.dat       data/ff-bin.dss	\
	data/ff-txt.dss       data/fixed-bad.dat    data/fixed.dat	\
	data/fixed.dss        data/fixedmax.dat     data/fixedmax.dss	\
	data/fixedmaxvar.dat  data/fixedmaxvar.dss  data/fixedvar.dat	\
	data/fixedvar.dss     data/httplog.dat      data/httplog.dss	\
	data/key-1.dss        data/key-2.dss        data/key-3.dss	\
	data/pwd-bin.dat      data/pwd-bin.dss      data/pwd-txt.dat	\
	data/pwd-txt.dss      data/q-txt.dss        data/qa-txt.dss	\
	data/qe-bin.dat       data/qe-bin.dss       data/qe-txt.dss	\
	data/suffix-txt.dss   data/variable.dss
