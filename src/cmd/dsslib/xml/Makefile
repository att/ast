:PACKAGE: ast

LICENSE = since=2010,author=gsf

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

xml plugin=dss :LIBRARY: \
		xml.c lib.c \
		-ldss

:TEST: dss

:: RELEASE \
	data/followers.xml data/friends.xml data/info.xml \
	data/basics.json data/array.json data/test.json data/twit.json
