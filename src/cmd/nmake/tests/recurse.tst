# ast nmake recursion order tests

INCLUDE test.def

UNIT nmake

TEST 01 'recursion order basics'

	EXEC	-n --recurse=list
		INPUT Makefile $':MAKE:'
		INPUT cmd/Makefile $':MAKE:'
		INPUT cmd/tstutil/Makefile $':PACKAGE: ast libtstinc:order'
		INPUT cmd/genutil/Makefile $'genutil :: genutil.c -lnet -ldb -ltstasm -lsys'
		INPUT cmd/fe/Makefile $':PACKAGE: tstdata:order\nfe :: fe.c -ltstasm'
		INPUT cmd/be/Makefile $':PACKAGE: tstdata:order\nbe :: be.c -lnetgen -lsys'
		INPUT cmd/tstdata/Makefile $':PACKAGE: tstutil:order'
		INPUT lib/Makefile $':MAKE:'
		INPUT lib/libtstasm/Makefile $'tstasm :LIBRARY: tstasm.c +ljcl -lsys'
		INPUT lib/libusr/Makefile $'usr :LIBRARY: usr.c'
		INPUT lib/libtstgen/Makefile $'tstgen :LIBRARY: tstgen.c -ltstasm'
		INPUT lib/libnet/Makefile $':PACKAGE: be:order\nnet :LIBRARY: net.c -lnetgen'
		INPUT lib/libnetgen/Makefile $':PACKAGE: fe:order\nnetgen :LIBRARY: netgen.c -ldb -lsys'
		INPUT lib/libdb/Makefile $'db :LIBRARY: db.c -lusr -ltstgen -lzip'
		INPUT lib/libsort/Makefile $'sort :LIBRARY: sort.c -lsys'
		INPUT lib/libtstinc/Makefile $':PACKAGE: tst'
		OUTPUT - $'lib/libtstinc
-
cmd/tstutil
-
cmd/tstdata
-
lib/libtstasm
-
cmd/fe
lib/libusr
lib/libtstgen
-
lib/libdb
-
lib/libnetgen
-
cmd/be
-
-
lib/libnet
-
cmd/genutil
lib/libsort
+
lib
cmd'

	EXEC	-n --recurse=prereqs
		OUTPUT - $'cmd/tstdata : cmd/tstutil
cmd/tstutil : lib/libtstinc
cmd/be : cmd/tstdata lib/libnetgen libsys
lib/libnetgen : cmd/fe lib/libdb libsys
cmd/fe : cmd/tstdata lib/libtstasm
lib/libtstasm : libjcl libsys
lib/libdb : lib/libusr lib/libtstgen libzip
lib/libtstgen : lib/libtstasm
cmd/genutil : lib/libnet lib/libdb lib/libtstasm libsys
lib/libnet : cmd/be lib/libnetgen
lib/libsort : libsys
all : lib/libsort cmd/genutil'

TEST 02 'common action recursion'

	EXEC
		INPUT Makefile $':MAKE: t1.mk t2.mk'
		INPUT t1.mk $'all :\n\t: $(MAKEFILE) :'
		INPUT t2.mk $'all :\n\t: $(MAKEFILE) :'
		ERROR - $'t1.mk:
+ : t1.mk :
t2.mk:
+ : t2.mk :'

	EXEC -n clobber
		OUTPUT - $'+ ignore rm -f -r  t1.mo t1.ms
+ ignore rm -f -r  t2.mo t2.ms
+ ignore rm -f -r  Makefile.mo Makefile.ms'
		ERROR - $'t1.mk:
t2.mk:'

	EXEC clobber
		OUTPUT -
		ERROR - $'t1.mk:
+ ignore rm -f -r t1.mo t1.ms
t2.mk:
+ ignore rm -f -r t2.mo t2.ms
+ ignore rm -f -r Makefile.mo Makefile.ms'

	EXEC -n clobber
		OUTPUT - $'+ ignore rm -f -r  
+ ignore rm -f -r  
+ ignore rm -f -r  '
		ERROR - $'t1.mk:
t2.mk:'

	EXEC clobber
		OUTPUT -
		ERROR - $'t1.mk:
+ ignore rm -f -r t1.mo
t2.mk:
+ ignore rm -f -r t2.mo
+ ignore rm -f -r Makefile.mo'
