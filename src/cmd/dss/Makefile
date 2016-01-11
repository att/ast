:PACKAGE: ast

LICENSE = since=2002,author=gsf

dss :: dss.c -ldss -ldll

:: RELEASE dss.mm

:MSGKEY: dss
	for method in $( dlls --base dss | grep -v '_t$' )
	do	dss -x $method,keys 2>&1 || true
	done
