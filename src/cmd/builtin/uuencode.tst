# regression tests for the uuencode/uudecode utilitiy

umask 022

KEEP "*.dat"

export LC_ALL=C

function DATA
{
	typeset f
	integer i
	typeset -i8 n
	for f
	do	test -f $f && continue
		case $f in
		chars.dat)
			typeset -i8 o
			for ((o = 0; o < 256; o++))
			do	print -f "\\${o#8#}"
			done
			;;
		1.dat)	print -f $'\001'
			;;
		2.dat)	print -f $'\001\002'
			;;
		3.dat)	print -f $'\001\002\003'
			;;
		abc.dat)print $'abc'
			;;
		esac > $f
		chmod u=rw,go=r $f
	done
}

TEST 01 'encode and decode ascii range'
	DO	DATA chars.dat
	EXEC	-x posix chars.dat test.dat
		OUTPUT - $'begin 644 test.dat
M  $" P0%!@<("0H+# T.#Q 1$A,4%187&!D:&QP=\'A\\@(2(C)"4F)R@I*BLL
M+2XO,#$R,S0U-C<X.3H[/#T^/T!!0D-$149\'2$E*2TQ-3D]045)35%565UA9
M6EM<75Y?8&%B8V1E9F=H:6IK;&UN;W!Q<G-T=79W>\'EZ>WQ]?G^ @8*#A(6&
MAXB)BHN,C8Z/D)&2DY25EI>8F9J;G)V>GZ"AHJ.DI::GJ*FJJZRMKJ^PL;*S
MM+6VM[BYNKN\\O;Z_P,\'"P\\3%QL?(R<K+S,W.S]#1TM/4U=;7V-G:V]S=WM_@
?X>+CY.7FY^CIZNOL[>[O\\/\'R\\_3U]O?X^?K[_/W^_P  \n \nend'
	EXEC
		MOVE OUTPUT t
	PROG	uudecode t -
		SAME OUTPUT chars.dat
	EXEC	-x ucb chars.dat test.dat
		OUTPUT - $'begin 644 test.dat
M``$"`P0%!@<("0H+#`T.#Q`1$A,4%187&!D:&QP=\'A\\@(2(C)"4F)R@I*BLL
M+2XO,#$R,S0U-C<X.3H[/#T^/T!!0D-$149\'2$E*2TQ-3D]045)35%565UA9
M6EM<75Y?8&%B8V1E9F=H:6IK;&UN;W!Q<G-T=79W>\'EZ>WQ]?G^`@8*#A(6&
MAXB)BHN,C8Z/D)&2DY25EI>8F9J;G)V>GZ"AHJ.DI::GJ*FJJZRMKJ^PL;*S
MM+6VM[BYNKN\\O;Z_P,\'"P\\3%QL?(R<K+S,W.S]#1TM/4U=;7V-G:V]S=WM_@
?X>+CY.7FY^CIZNOL[>[O\\/\'R\\_3U]O?X^?K[_/W^_VYN
`
end'
	EXEC
		MOVE OUTPUT t
	PROG	uudecode t -
		SAME OUTPUT chars.dat
	EXEC	-x mime chars.dat test.dat
		OUTPUT - $'begin-base64 644 test.dat
AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmJygpKiss
LS4vMDEyMzQ1Njc4OTo7PD0+P0BBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZ
WltcXV5fYGFiY2RlZmdoaWprbG1ub3BxcnN0dXZ3eHl6e3x9fn+AgYKDhIWG
h4iJiouMjY6PkJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmqq6ytrq+wsbKz
tLW2t7i5uru8vb6/wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g
4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7/P3+/w==
===='
	EXEC
		MOVE OUTPUT t
	PROG	uudecode t -
		SAME OUTPUT chars.dat
	EXEC	-x quoted-printable chars.dat test.dat
		OUTPUT - $'=00=01=02=03=04=05=06=07=08=09
=0B=0C=0D=0E=0F=10=11=12=13=14=15=16=17=18=19=1A=1B=1C=1D=1E=1F !"#$%&\'(=
)*+,-./0123456789:;<=3D>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmn=
opqrstuvwxyz{|}~=7F=80=81=82=83=84=85=86=87=88=89=8A=8B=8C=8D=8E=8F=90=91=
=92=93=94=95=96=97=98=99=9A=9B=9C=9D=9E=9F=A0=A1=A2=A3=A4=A5=A6=A7=A8=A9=
=AA=AB=AC=AD=AE=AF=B0=B1=B2=B3=B4=B5=B6=B7=B8=B9=BA=BB=BC=BD=BE=BF=C0=C1=
=C2=C3=C4=C5=C6=C7=C8=C9=CA=CB=CC=CD=CE=CF=D0=D1=D2=D3=D4=D5=D6=D7=D8=D9=
=DA=DB=DC=DD=DE=DF=E0=E1=E2=E3=E4=E5=E6=E7=E8=E9=EA=EB=EC=ED=EE=EF=F0=F1=
=F2=F3=F4=F5=F6=F7=F8=F9=FA=FB=FC=FD=FE=FF='
	EXEC
		MOVE OUTPUT t
	PROG	uudecode -x quoted-printable t -
		SAME OUTPUT chars.dat
	EXEC	-x binhex chars.dat test.dat
		OUTPUT - $'(This file must be converted with BinHex 4.0)
:#(4PFh3ZC\'&d!!#3#2J!!*!#!3!!N!46qJ!"!J-%"3B(#!N+#``0$Jm3%4)6&"8@
&aJC\'KXF(4iI)#%L)b3P*LFS+5SV,#dZ,c!a-M-d06Bh1$Nk1c`p2Mp!38*$4%9\'
4dK*5NY-68j28&&58e499PGB@9TEA&eHAf"KBQ0NC@CRD\'PUDfaYEQp`FA*cG(9f
GhKjHRYmIAjrJ)\'#Ji5&KSH)LBU,M)f1Mj!!NC+6P*@@PjLCQTZFRCkIS+\'LSk5P
TUHSUDUVV+fZVl#aXV1dYEDhZ,QkZlbp[Vr!`F,$a-A\'amM*bX[-cFl2d0(5dp69
eYIBfGVEh0hHhq$KiZ2NjHERk1RUkqcYlZr`mI,cp2AfprMjq[[mrIlrIP8!!*$r
:'
	EXEC
		MOVE OUTPUT t
	PROG	uudecode -x binhex t -
		SAME OUTPUT chars.dat

TEST 02 'encode and decode 1 char'
	DO	DATA 1.dat
	EXEC	-x posix 1.dat test.dat
		OUTPUT - $'begin 644 test.dat\n! 0  \n \nend'
	EXEC
		MOVE OUTPUT t
	PROG	uudecode t -
		SAME OUTPUT 1.dat
	EXEC	-x ucb 1.dat test.dat
		OUTPUT - $'begin 644 test.dat\n!`6YN\n`\nend'
	EXEC
		MOVE OUTPUT t
	PROG	uudecode t -
		SAME OUTPUT 1.dat
	EXEC	-x mime 1.dat test.dat
		OUTPUT - $'begin-base64 644 test.dat\nAQ==\n===='
	EXEC
		MOVE OUTPUT t
	PROG	uudecode t -
		SAME OUTPUT 1.dat
	EXEC	-x quoted-printable 1.dat test.dat
		OUTPUT - $'=01='
	EXEC
		MOVE OUTPUT t
	PROG	uudecode -x quoted-printable t -
		SAME OUTPUT 1.dat
	EXEC	-x binhex 1.dat test.dat
		OUTPUT - $'(This file must be converted with BinHex 4.0)
:#(4PFh3ZC\'&d!!#3#2J!!*!$!3!!N!1m#`%3)3!!N2m!:'
	EXEC
		MOVE OUTPUT t
	PROG	uudecode -x binhex t -
		SAME OUTPUT 1.dat

TEST 03 'encode and decode 2 chars'
	DO	DATA 2.dat
	EXEC	-x posix 2.dat test.dat
		OUTPUT - $'begin 644 test.dat\n" 0( \n \nend'
	EXEC
		MOVE OUTPUT t
	PROG	uudecode t -
		SAME OUTPUT 2.dat
	EXEC	-x ucb 2.dat test.dat
		OUTPUT - $'begin 644 test.dat\n"`0)N\n`\nend'
	EXEC
		MOVE OUTPUT t
	PROG	uudecode t -
		SAME OUTPUT 2.dat
	EXEC	-x mime 2.dat test.dat
		OUTPUT - $'begin-base64 644 test.dat\nAQI=\n===='
	EXEC
		MOVE OUTPUT t
	PROG	uudecode t -
		SAME OUTPUT 2.dat
	EXEC	-x quoted-printable 2.dat test.dat
		OUTPUT - $'=01=02='
	EXEC
		MOVE OUTPUT t
	PROG	uudecode -x quoted-printable t -
		SAME OUTPUT 2.dat
	EXEC	-x binhex 2.dat test.dat
		OUTPUT - $'(This file must be converted with BinHex 4.0)
:#(4PFh3ZC\'&d!!#3#2J!!*!$!J!!N!05f3%#%h-!!*$r:'
	EXEC
		MOVE OUTPUT t
	PROG	uudecode -x binhex t -
		SAME OUTPUT 2.dat

TEST 04 'encode and decode 3 chars'
	DO	DATA 3.dat
	EXEC	-x posix 3.dat test.dat
		OUTPUT - $'begin 644 test.dat\n# 0(#\n \nend'
	EXEC
		MOVE OUTPUT t
	PROG	uudecode t -
		SAME OUTPUT 3.dat
	EXEC	-x ucb 3.dat test.dat
		OUTPUT - $'begin 644 test.dat\n#`0(#\n`\nend'
	EXEC
		MOVE OUTPUT t
	PROG	uudecode t -
		SAME OUTPUT 3.dat
	EXEC	-x mime 3.dat test.dat
		OUTPUT - $'begin-base64 644 test.dat\nAQID\n===='
	EXEC
		MOVE OUTPUT t
	PROG	uudecode t -
		SAME OUTPUT 3.dat
	EXEC	-x quoted-printable 3.dat test.dat
		OUTPUT - $'=01=02=03='
	EXEC
		MOVE OUTPUT t
	PROG	uudecode -x quoted-printable t -
		SAME OUTPUT 3.dat
	EXEC	-x binhex 3.dat test.dat
		OUTPUT - $'(This file must be converted with BinHex 4.0)
:#(4PFh3ZC\'&d!!#3#2J!!*!$!`!!N!2iL!%#!f%a!!#3:'
	EXEC
		MOVE OUTPUT t
	PROG	uudecode -x binhex t -
		SAME OUTPUT 3.dat

TEST 05 'file name shell game'
	DO	DATA abc.dat
	EXEC	-x mime game.dat
		INPUT - $'abc'
		OUTPUT - $'begin-base64 644 game.dat\nYWJjCg==\n===='
	EXEC	-o o -x mime game.dat
		OUTPUT -
	PROG	uudecode o
		SAME game.dat abc.dat
	PROG	uudecode -o o.dat o
		SAME o.dat abc.dat
	PROG	uudecode o -
		OUTPUT - $'abc'
	PROG	uudecode -o -
		INPUT - $'begin-base64 644 foo.dat\nYWJjCg==\n===='
	PROG	uudecode
		OUTPUT -
		SAME foo.dat abc.dat
