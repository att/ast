if CC.HOSTTYPE == "no_build"
/*
 * 3d fs library
 *
 * NOTE: ld -r does not work with $(CC.DLL) so lib3d.o is not possible
 *       (unless we compile it in one file -- no thanks)
 *	 so a static link needs -u _exit -l3d other-libs
 */

:PACKAGE: ast:nolibrary

LICENSE = since=1989,author=gsf+dgk+ekrell

ID = 3d
VERSION = 4.0
CATALOG = 3d

CCFLAGS = -O $(CC.DLL) /* NOTE: gcc $(CC.OPTIMIZE) fails here! */
IFFEREFS = -last
TMP = 3.$(tmp)

DEBUG == 1
DEBUG_TABLE == 0
FS == 1
IDNAME == "$(ID)"
PATHCHECK == "$(ID)"
MIX ==
VCS == 1

if LITE
DEBUG = 0
DEBUG_TABLE = 0
FS = 0
VCS = 0
end

$(ID)$(VARIANTID) $(VERSION) id=$(ID) :LIBRARY: README RELEASE PROMO.mm \
	3d.h lib_3d.h dir_3d.h \
	init.c \
	bio.c copy.c error.c execv.c fs.c lib.c map.c \
	pathcanon.c pathnext.c pathreal.c peek.c \
	ast.c cs.c license.c \
	vcs_3d.h vcs.c \
	access3d.c brk3d.c chdir3d.c chmod3d.c chown3d.c close3d.c \
	creat3d.c dir3d.c dup23d.c dup3d.c execve3d.c exit3d.c \
	fchdir3d.c fchmod3d.c fchown3d.c fcntl3d.c fork3d.c fpathconf3d.c \
	fstat3d.c fstatfs3d.c fstatvfs3d.c fsync3d.c \
	ftruncate3d.c getdents3d.c kill3d.c link3d.c lseek3d.c \
	lstat3d.c mkdir3d.c mknod3d.c mount3d.c open3d.c pathconf3d.c \
	pipe3d.c read3d.c readlink3d.c rename3d.c rmdir3d.c sbrk3d.c \
	stat3d.c statfs3d.c statvfs3d.c symlink3d.c syscall3d.c \
	sysent3d.c truncate3d.c unlink3d.c utime3d.c utimes3d.c utimets3d.c \
	vfork3d.c write3d.c \
	acl3d.c facl3d.c \
	dll3d.c \
	utimensat3d.c \
	-ldl

if ! VARIANTID

3d :: 3d.sh

opaque :: opaque.sh

vex :: vex.sh

$(FUNDIR) :INSTALLDIR: mode=+x _2d

end

:VARIANT:

/*
 * -l<library> :lib: <subdir> ...
 *
 * snarf src from <library> and its <subdir>s
 */

NAME3D = lib_3d.h std_3d.h vcs_3d.h

":lib:" : .MAKE .OPERATOR
	local L
	L := $(<:/-l//)
	eval
	_BLD_$(L) == 1
	end
	:: $(L)_3d.h
	NAME3D += $(L)_3d.h
	L := ../../$(VARIANTID:?../)lib/lib$(L)
	.SOURCE.c : $(L:X=. $(>))
	.SOURCE.features/% : $(L)

-last :lib: comp dir hash misc path port string tm

-lcs :lib:

-ldll :lib:

terminal FEATURE/eaccess FEATURE/hack

3d :TEST: 3d.tst
	$(BINDIR)/3d $(SHELL) $(REGRESS) $(REGRESSFLAGS) $(*)

:: sys.tab

else

:NOTHING:

end
