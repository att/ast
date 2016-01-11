:PACKAGE: - X11 tcl:install ast libtksh:force

ID			=	tk
LIB			=	tk
TK_VERSION		=	4.2
TCL_VERSION		= 	7.6

LIB_DIR			==	"lib/$(ID)$(TK_VERSION)"

HAVE_LIMITS_H		==	1
HAVE_UNISTD_H		==	1

if CC.HOSTTYPE == "win32.*"
CCFLAGS			=	$(CC.OPTIMIZE) $(CC.DLL)
else
CCFLAGS			=	$(CC.OPTIMIZE)
end

.SOURCE : generic unix
.SOURCE.h : include bitmaps xlib generic tcl

$(LIB) $(TK_VERSION) :LIBRARY: \
	tkUnixSelect.c tkUnixInit.c tkUnixEvent.c tkUnixXId.c \
	tkUnixCursor.c tkUnixDialog.c tkUnixDraw.c tkUnixWm.c tkUnix.c \
	tk3d.c tkArgv.c tkAtom.c tkBind.c tkBitmap.c tkButton.c \
	tkCanvArc.c tkCanvBmap.c tkCanvImg.c tkCanvLine.c tkCanvPoly.c \
	tkCanvPs.c tkCanvText.c tkCanvUtil.c tkCanvWind.c tkCanvas.c \
	tkClipboard.c tkCmds.c tkColor.c tkConfig.c tkConsole.c tkCursor.c \
	tkEntry.c tkError.c tkEvent.c tkFileFilter.c tkFocus.c tkFont.c \
	tkFrame.c tkGC.c tkGeometry.c tkGet.c tkGrab.c tkGrid.c tkImage.c \
	tkImgBmap.c tkImgGIF.c tkImgPPM.c tkImgPhoto.c tkImgUtil.c \
	tkListbox.c tkMenu.c tkMenubutton.c tkMessage.c tkOption.c tkPack.c \
	tkPlace.c tkRectOval.c tkScale.c tkScrollbar.c tkSelect.c tkSend.c \
	tkSquare.c tkTest.c tkText.c tkTextBTree.c tkTextDisp.c \
	tkTextIndex.c tkTextMark.c tkTextTag.c tkTextWind.c tkTrig.c \
	tkUtil.c tkVisual.c tkWindow.c tkMain.c \
	tkDefault.h tkFileFilter.h tkSelect.h tkNnames.h tkInt.h \
	tkText.h tk.h tkPatch.h tkCanvas.h tkPort.h tkUnixDef.h \
	tkUnixInt.h tkUnixPort.h

$(INCLUDEDIR) :INSTALLPROTO: tk.h

:: README RELEASE ToDo changes license.terms tkAppInit.c

/* Generated with: ksh gen library '/$(ID)$(TK_VERSION)' */

$(LIBDIR)/$(ID)$(TK_VERSION):INSTALLDIR: \
	library/bgerror.tcl \
	library/button.tcl \
	library/clrpick.tcl \
	library/comdlg.tcl \
	library/console.tcl \
	library/dialog.tcl \
	library/entry.tcl \
	library/focus.tcl \
	library/folder.gif \
	library/license.terms \
	library/listbox.tcl \
	library/menu.tcl \
	library/msgbox.tcl \
	library/obsolete.tcl \
	library/optMenu.tcl \
	library/palette.tcl \
	library/prolog.ps \
	library/scale.tcl \
	library/scrlbar.tcl \
	library/tclIndex \
	library/tearoff.tcl \
	library/text.tcl \
	library/textfile.gif \
	library/tk.tcl \
	library/tkfbox.tcl \
	library/updir.xbm \
	library/xmfbox.tcl

$(LIBDIR)/$(ID)$(TK_VERSION)/demos:INSTALLDIR: \
	library/demos/README \
	library/demos/arrow.tcl \
	library/demos/bind.tcl \
	library/demos/bitmap.tcl \
	library/demos/browse \
	library/demos/button.tcl \
	library/demos/check.tcl \
	library/demos/clrpick.tcl \
	library/demos/colors.tcl \
	library/demos/cscroll.tcl \
	library/demos/ctext.tcl \
	library/demos/dialog1.tcl \
	library/demos/dialog2.tcl \
	library/demos/entry1.tcl \
	library/demos/entry2.tcl \
	library/demos/filebox.tcl \
	library/demos/floor.tcl \
	library/demos/form.tcl \
	library/demos/hello \
	library/demos/hscale.tcl \
	library/demos/icon.tcl \
	library/demos/image1.tcl \
	library/demos/image2.tcl \
	library/demos/items.tcl \
	library/demos/ixset \
	library/demos/label.tcl \
	library/demos/license.terms \
	library/demos/menu.tcl \
	library/demos/msgbox.tcl \
	library/demos/plot.tcl \
	library/demos/puzzle.tcl \
	library/demos/radio.tcl \
	library/demos/rmt \
	library/demos/rolodex \
	library/demos/ruler.tcl \
	library/demos/sayings.tcl \
	library/demos/search.tcl \
	library/demos/square \
	library/demos/states.tcl \
	library/demos/style.tcl \
	library/demos/tclIndex \
	library/demos/tcolor \
	library/demos/text.tcl \
	library/demos/timer \
	library/demos/twind.tcl \
	library/demos/vscale.tcl \
	library/demos/widget

$(LIBDIR)/$(ID)$(TK_VERSION)/demos/images:INSTALLDIR: \
	library/demos/images/earth.gif \
	library/demos/images/earthris.gif \
	library/demos/images/face.bmp \
	library/demos/images/flagdown.bmp \
	library/demos/images/flagup.bmp \
	library/demos/images/gray25.bmp \
	library/demos/images/letters.bmp \
	library/demos/images/noletter.bmp \
	library/demos/images/pattern.bmp \
	library/demos/images/tcllogo.gif \
	library/demos/images/teapot.ppm

/* Generated with: ksh gen library */

::	\
	library/dialog.tcl \
	library/textfile.gif \
	library/optMenu.tcl \
	library/scale.tcl \
	library/text.tcl \
	library/folder.gif \
	library/updir.xbm \
	library/focus.tcl \
	library/menu.tcl \
	library/msgbox.tcl \
	library/listbox.tcl \
	library/tearoff.tcl \
	library/prolog.ps \
	library/button.tcl \
	library/comdlg.tcl \
	library/tclIndex \
	library/license.terms \
	library/tk.tcl \
	library/palette.tcl \
	library/obsolete.tcl \
	library/console.tcl \
	library/bgerror.tcl \
	library/clrpick.tcl \
	library/scrlbar.tcl \
	library/xmfbox.tcl \
	library/tkfbox.tcl \
	library/entry.tcl \
	library/demos/rmt \
	library/demos/arrow.tcl \
	library/demos/square \
	library/demos/puzzle.tcl \
	library/demos/vscale.tcl \
	library/demos/hscale.tcl \
	library/demos/ctext.tcl \
	library/demos/style.tcl \
	library/demos/cscroll.tcl \
	library/demos/browse \
	library/demos/twind.tcl \
	library/demos/tcolor \
	library/demos/floor.tcl \
	library/demos/bind.tcl \
	library/demos/text.tcl \
	library/demos/plot.tcl \
	library/demos/timer \
	library/demos/menu.tcl \
	library/demos/ruler.tcl \
	library/demos/README \
	library/demos/items.tcl \
	library/demos/search.tcl \
	library/demos/filebox.tcl \
	library/demos/bitmap.tcl \
	library/demos/msgbox.tcl \
	library/demos/radio.tcl \
	library/demos/hello \
	library/demos/button.tcl \
	library/demos/widget \
	library/demos/tclIndex \
	library/demos/ixset \
	library/demos/license.terms \
	library/demos/label.tcl \
	library/demos/states.tcl \
	library/demos/dialog2.tcl \
	library/demos/clrpick.tcl \
	library/demos/image2.tcl \
	library/demos/entry2.tcl \
	library/demos/sayings.tcl \
	library/demos/rolodex \
	library/demos/form.tcl \
	library/demos/check.tcl \
	library/demos/colors.tcl \
	library/demos/icon.tcl \
	library/demos/dialog1.tcl \
	library/demos/image1.tcl \
	library/demos/entry1.tcl \
	library/demos/images/flagdown.bmp \
	library/demos/images/pattern.bmp \
	library/demos/images/tcllogo.gif \
	library/demos/images/teapot.ppm \
	library/demos/images/noletter.bmp \
	library/demos/images/earthris.gif \
	library/demos/images/face.bmp \
	library/demos/images/flagup.bmp \
	library/demos/images/gray25.bmp \
	library/demos/images/letters.bmp \
	library/demos/images/earth.gif 


/* Generated with: ksh gen doc */

::	\
	doc/CrtPhImgFmt.3 \
	doc/toplevel.n \
	doc/CrtErrHdlr.3 \
	doc/GetScroll.3 \
	doc/CrtGenHdlr.3 \
	doc/GetCapStyl.3 \
	doc/tk.n \
	doc/popup.n \
	doc/wm.n \
	doc/FindPhoto.3 \
	doc/GetVRoot.3 \
	doc/MainLoop.3 \
	doc/place.n \
	doc/pack.n \
	doc/image.n \
	doc/scale.n \
	doc/frame.n \
	doc/Name.3 \
	doc/tkwait.n \
	doc/CrtSelHdlr.3 \
	doc/tkvars.n \
	doc/IdToWindow.3 \
	doc/GetColor.3 \
	doc/InternAtom.3 \
	doc/GetPixmap.3 \
	doc/GetBitmap.3 \
	doc/GetClrmap.3 \
	doc/grab.n \
	doc/label.n \
	doc/grid.n \
	doc/ClrSelect.3 \
	doc/MainWin.3 \
	doc/optionMenu.n \
	doc/HandleEvent.3 \
	doc/chooseColor.n \
	doc/menubutton.n \
	doc/CrtWindow.3 \
	doc/ManageGeom.3 \
	doc/palette.n \
	doc/focus.n \
	doc/listbox.n \
	doc/menubar.n \
	doc/GetGC.3 \
	doc/destroy.n \
	doc/Tk_Main.3 \
	doc/button.n \
	doc/CanvPsY.3 \
	doc/entry.n \
	doc/Restack.3 \
	doc/tkerror.n \
	doc/SetAppName.3 \
	doc/bell.n \
	doc/GetUid.3 \
	doc/send.n \
	doc/event.n \
	doc/menu.n \
	doc/text.n \
	doc/GetRelief.3 \
	doc/GetOption.3 \
	doc/getOpenFile.n \
	doc/GeomReq.3 \
	doc/GetRootCrd.3 \
	doc/canvas.n \
	doc/CrtItemType.3 \
	doc/NameOfImg.3 \
	doc/selection.n \
	doc/message.n \
	doc/DeleteImg.3 \
	doc/GetJoinStl.3 \
	doc/bindtags.n \
	doc/clipboard.n \
	doc/Clipboard.3 \
	doc/GetFontStr.3 \
	doc/GetAnchor.3 \
	doc/DrawFocHlt.3 \
	doc/lower.n \
	doc/SetGrid.3 \
	doc/3DBorder.3 \
	doc/CoordToWin.3 \
	doc/WindowId.3 \
	doc/options.n \
	doc/CanvTkwin.3 \
	doc/pack-old.n \
	doc/option.n \
	doc/bind.n \
	doc/raise.n \
	doc/wish.1 \
	doc/checkbutton.n \
	doc/EventHndlr.3 \
	doc/license.terms \
	doc/radiobutton.n \
	doc/GetVisual.3 \
	doc/SetVisual.3 \
	doc/GetSelect.3 \
	doc/MoveToplev.3 \
	doc/GetPixels.3 \
	doc/StrictMotif.3 \
	doc/OwnSelect.3 \
	doc/QWinEvent.3 \
	doc/MaintGeom.3 \
	doc/Tk_Init.3 \
	doc/CanvTxtInfo.3 \
	doc/RestrictEv.3 \
	doc/focusNext.n \
	doc/MapWindow.3 \
	doc/ImgChanged.3 \
	doc/BindTable.3 \
	doc/GetCursor.3 \
	doc/dialog.n \
	doc/FreeXId.3 \
	doc/GetJustify.3 \
	doc/CrtImgType.3 \
	doc/bitmap.n \
	doc/ParseArgv.3 \
	doc/messageBox.n \
	doc/scrollbar.n \
	doc/tk4.0.ps \
	doc/winfo.n \
	doc/GetImage.3 \
	doc/ConfigWidg.3 \
	doc/ConfigWind.3 \
	doc/SetClass.3 \
	doc/man.macros \
	doc/photo.n 
