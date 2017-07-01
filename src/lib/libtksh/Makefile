:PACKAGE: - X11 tcl:install shell:20120101 ast

ID			=	tksh
LIB			=	tcl

TCL_VERSION		=	7.6
TK_VERSION		=	4.2

LIB_DIR			==	"lib/$(ID)$(TCL_VERSION)"
TKSH_LIBRARY    	==      "$(LIBDIR)/$(ID)$(TCL_VERSION)"

if CC.HOSTTYPE == "win32.*"
CCFLAGS			=	$(CC.OPTIMIZE) $(CC.DLLBIG)
else
CCFLAGS			=	$(CC.OPTIMIZE)
end

.SOURCE : src tcl
.SOURCE.h : include

$(LIB) $(TCL_VERSION) :LIBRARY: \
	float.h nvextra.h shcompat.h tclInt.h \
	tclPort.h tclcmd.h tksh.h \
	panic.c regexp.c tclHash.c tclGet.c tclAsync.c tclLink.c tclExpr.c \
	tclCmdAH.c tclCmdMZ.c tclCmdIL.c tclParse.c tclGlob.c tclUtil.c \
	tclEvent.c tclPreserve.c tclNotify.c tclClock.c tclDate.c tclInterp.c \
	tclUnixFile.c tclUnixTime.c tclLoad.c tclLoadDl.c tclPkg.c \
	tclPosixStr.c tclFHandle.c tclIOCmd.c tclUnixNotfy.c init.c debug.c \
	var.c eval.c list.c shcompat.c tcleval.c tclproc.c commands.c \
	varcmd.c tclMain.c basic.c filecmd.c tclIO.c \
	tclUnixChan.c tclIOUtil.c \
	$(CC.HOSTTYPE:N=win32.*:?-lshell?+lshell?)

$(INCLUDEDIR) :INSTALLPROTO: tksh.h tcl.h

::	tcl/README tcl/license.terms gen tksh.mk widget \
	README RELEASE scripts/mailfile scripts/watchdir

$(LIBDIR)/$(ID)$(TCL_VERSION):INSTALLDIR: \
	lib/tksh7.6/init.ksh \
	lib/tksh7.6/init.tcl \
	lib/tksh7.6/ldAout.tcl \
	lib/tksh7.6/license.terms \
	lib/tksh7.6/parray.tcl \
	lib/tksh7.6/tclIndex

:: \
	lib/tksh7.6/init.ksh \
	lib/tksh7.6/init.tcl \
	lib/tksh7.6/ldAout.tcl \
	lib/tksh7.6/license.terms \
	lib/tksh7.6/parray.tcl \
	lib/tksh7.6/tclIndex

::	\
	desksh/desksh \
	desksh/desksh.1 \
	desksh/lib/routines-desksh \
	desksh/lib/routines-file-browser \
	desksh/lib/routines-fman \
	desksh/lib/routines-menu-file \
	desksh/lib/routines-menu-help \
	desksh/lib/routines-menu-util \
	desksh/lib/routines-pman \
	desksh/lib/routines-utilities \
	desksh/lib/routines-windows \
	desksh/lib/deskshrc \
	desksh/lib/directory_dialog \
	desksh/lib/find_popup \
	desksh/lib/generate \
	desksh/lib/utils \
	desksh/lib/twrc \
	desksh/lib/s1.c \
	desksh/lib/s2.c \
	desksh/lib/sample.c \
	desksh/lib/desksh-options \
	desksh/lib/imgs/blocks.gif \
	desksh/lib/imgs/const.gif \
	desksh/lib/imgs/copy.gif \
	desksh/lib/imgs/edit.gif \
	desksh/lib/imgs/exec.gif \
	desksh/lib/imgs/exit.gif \
	desksh/lib/imgs/key.gif \
	desksh/lib/imgs/move.gif \
	desksh/lib/imgs/search.gif \
	desksh/lib/imgs/tools.gif \
	desksh/lib/imgs/tools2.gif \
	desksh/lib/imgs/trash.gif \
	desksh/lib/imgs/write.gif 
