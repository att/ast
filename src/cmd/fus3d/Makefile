:PACKAGE: ast

LICENSE = since=2012,author=ek

FUSE_VERSION_MIN = 2.8.0
FUSE_VERSION = $(CC.STDLIB:L=libfuse.so.*.*.*:C/libfuse.so.//) $(FUSE_VERSION_MIN)

FUSE_USE_VERSION == 26
_FILE_OFFSET_BITS == 64

if CC.HOSTTYPE == "*"
	if "$(FUSE_VERSION:H=V:O=1)" != "$(FUSE_VERSION_MIN)"
		fus3d :: fus3d.c ndfsops.c ndfsutils.c -lfuse -lpthread

		:: RELEASE
	else
		:NOTHING:

		:: RELEASE fus3d.c ndfsops.c ndfsutils.c
	end
end
