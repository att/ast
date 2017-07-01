# regression tests for codex(1) and codex(3)

UMASK 022

KEEP "*.dat"

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
		0.dat)	;;
		1.dat)	print -f $'\001'
			;;
		2.dat)	print -f $'\001\002'
			;;
		3.dat)	print -f $'\001\002\003'
			;;
		abc.dat)print $'abc'
			;;
		big.dat)integer i
			echo 'This is a test.' > t
			for ((i = 0; i < 5; i++))
			do	cat t t > u
				cat u u > t
			done
			cat t
			rm t u
			;;
		esac > $f
		chmod u=rw,go=r $f
	done
}

TEST 01 'encode and decode ascii range'
	DO	DATA chars.dat

	EXEC	--encode=uu-posix
		SAME INPUT chars.dat
		OUTPUT - $'M  $" P0%!@<("0H+# T.#Q 1$A,4%187&!D:&QP=\'A\\@(2(C)"4F)R@I*BLL
M+2XO,#$R,S0U-C<X.3H[/#T^/T!!0D-$149\'2$E*2TQ-3D]045)35%565UA9
M6EM<75Y?8&%B8V1E9F=H:6IK;&UN;W!Q<G-T=79W>\'EZ>WQ]?G^ @8*#A(6&
MAXB)BHN,C8Z/D)&2DY25EI>8F9J;G)V>GZ"AHJ.DI::GJ*FJJZRMKJ^PL;*S
MM+6VM[BYNKN\\O;Z_P,\'"P\\3%QL?(R<K+S,W.S]#1TM/4U=;7V-G:V]S=WM_@
?X>+CY.7FY^CIZNOL[>[O\\/\'R\\_3U]O?X^?K[_/W^_P  '
		MOVE OUTPUT test.dat
	EXEC	--decode=uu-posix
		SAME INPUT test.dat
		SAME OUTPUT chars.dat

	EXEC	--encode=uu-ucb
		SAME INPUT chars.dat
		OUTPUT - $'M``$"`P0%!@<("0H+#`T.#Q`1$A,4%187&!D:&QP=\'A\\@(2(C)"4F)R@I*BLL
M+2XO,#$R,S0U-C<X.3H[/#T^/T!!0D-$149\'2$E*2TQ-3D]045)35%565UA9
M6EM<75Y?8&%B8V1E9F=H:6IK;&UN;W!Q<G-T=79W>\'EZ>WQ]?G^`@8*#A(6&
MAXB)BHN,C8Z/D)&2DY25EI>8F9J;G)V>GZ"AHJ.DI::GJ*FJJZRMKJ^PL;*S
MM+6VM[BYNKN\\O;Z_P,\'"P\\3%QL?(R<K+S,W.S]#1TM/4U=;7V-G:V]S=WM_@
?X>+CY.7FY^CIZNOL[>[O\\/\'R\\_3U]O?X^?K[_/W^_VYN'
		MOVE OUTPUT test.dat
	EXEC	--decode=uu-ucb
		SAME INPUT test.dat
		SAME OUTPUT chars.dat

	EXEC	--encode=uu-mime
		SAME INPUT chars.dat
		OUTPUT - $'AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmJygpKiss
LS4vMDEyMzQ1Njc4OTo7PD0+P0BBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZ
WltcXV5fYGFiY2RlZmdoaWprbG1ub3BxcnN0dXZ3eHl6e3x9fn+AgYKDhIWG
h4iJiouMjY6PkJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmqq6ytrq+wsbKz
tLW2t7i5uru8vb6/wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g
4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7/P3+/w=='
		MOVE OUTPUT test.dat
	EXEC	--decode=uu-mime
		SAME INPUT test.dat
		SAME OUTPUT chars.dat

	EXEC	--encode=qp
		SAME INPUT chars.dat
		OUTPUT - $'=00=01=02=03=04=05=06=07=08=09
=0B=0C=0D=0E=0F=10=11=12=13=14=15=16=17=18=19=1A=1B=1C=1D=1E=1F !"#$%&\'(=
)*+,-./0123456789:;<=3D>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmn=
opqrstuvwxyz{|}~=7F=80=81=82=83=84=85=86=87=88=89=8A=8B=8C=8D=8E=8F=90=91=
=92=93=94=95=96=97=98=99=9A=9B=9C=9D=9E=9F=A0=A1=A2=A3=A4=A5=A6=A7=A8=A9=
=AA=AB=AC=AD=AE=AF=B0=B1=B2=B3=B4=B5=B6=B7=B8=B9=BA=BB=BC=BD=BE=BF=C0=C1=
=C2=C3=C4=C5=C6=C7=C8=C9=CA=CB=CC=CD=CE=CF=D0=D1=D2=D3=D4=D5=D6=D7=D8=D9=
=DA=DB=DC=DD=DE=DF=E0=E1=E2=E3=E4=E5=E6=E7=E8=E9=EA=EB=EC=ED=EE=EF=F0=F1=
=F2=F3=F4=F5=F6=F7=F8=F9=FA=FB=FC=FD=FE=FF='
		MOVE OUTPUT test.dat
	EXEC	--decode=qp
		SAME INPUT test.dat
		SAME OUTPUT chars.dat

TEST 02 'method composition'
	DO	DATA chars.dat

	EXEC	--encode=sum-md5^uu-posix
		SAME INPUT chars.dat
		OUTPUT - $'M  $" P0%!@<("0H+# T.#Q 1$A,4%187&!D:&QP=\'A\\@(2(C)"4F)R@I*BLL
M+2XO,#$R,S0U-C<X.3H[/#T^/T!!0D-$149\'2$E*2TQ-3D]045)35%565UA9
M6EM<75Y?8&%B8V1E9F=H:6IK;&UN;W!Q<G-T=79W>\'EZ>WQ]?G^ @8*#A(6&
MAXB)BHN,C8Z/D)&2DY25EI>8F9J;G)V>GZ"AHJ.DI::GJ*FJJZRMKJ^PL;*S
MM+6VM[BYNKN\\O;Z_P,\'"P\\3%QL?(R<K+S,W.S]#1TM/4U=;7V-G:V]S=WM_@
?X>+CY.7FY^CIZNOL[>[O\\/\'R\\_3U]O?X^?K[_/W^_P  '
		MOVE OUTPUT test.dat
		ERROR - $'e2c865db4162bed963bfaa9ef6ac18f0'
	EXEC	--decode=sum-md5^uu-posix
		SAME INPUT test.dat
		SAME OUTPUT chars.dat

	EXEC	--encode=uu-ucb^sum-md5
		SAME INPUT chars.dat
		OUTPUT - $'M``$"`P0%!@<("0H+#`T.#Q`1$A,4%187&!D:&QP=\'A\\@(2(C)"4F)R@I*BLL
M+2XO,#$R,S0U-C<X.3H[/#T^/T!!0D-$149\'2$E*2TQ-3D]045)35%565UA9
M6EM<75Y?8&%B8V1E9F=H:6IK;&UN;W!Q<G-T=79W>\'EZ>WQ]?G^`@8*#A(6&
MAXB)BHN,C8Z/D)&2DY25EI>8F9J;G)V>GZ"AHJ.DI::GJ*FJJZRMKJ^PL;*S
MM+6VM[BYNKN\\O;Z_P,\'"P\\3%QL?(R<K+S,W.S]#1TM/4U=;7V-G:V]S=WM_@
?X>+CY.7FY^CIZNOL[>[O\\/\'R\\_3U]O?X^?K[_/W^_VYN'
		MOVE OUTPUT test.dat
		ERROR - $'0bcb8785cd4eb877aa9e246512e2b38e'
	EXEC	--decode=uu-ucb^sum-md5
		SAME INPUT test.dat
		SAME OUTPUT chars.dat

	EXEC	--encode=uu-mime^sum-md5
		SAME INPUT chars.dat
		OUTPUT - $'AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmJygpKiss
LS4vMDEyMzQ1Njc4OTo7PD0+P0BBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZ
WltcXV5fYGFiY2RlZmdoaWprbG1ub3BxcnN0dXZ3eHl6e3x9fn+AgYKDhIWG
h4iJiouMjY6PkJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmqq6ytrq+wsbKz
tLW2t7i5uru8vb6/wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g
4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7/P3+/w=='
		MOVE OUTPUT test.dat
		ERROR - $'1731d6ec5da5343fcf7e1138a4d78fba'
	EXEC	--decode=uu-mime^sum-md5
		SAME INPUT test.dat
		SAME OUTPUT chars.dat

	EXEC	--encode=qp^sum-md5
		SAME INPUT chars.dat
		OUTPUT - $'=00=01=02=03=04=05=06=07=08=09
=0B=0C=0D=0E=0F=10=11=12=13=14=15=16=17=18=19=1A=1B=1C=1D=1E=1F !"#$%&\'(=
)*+,-./0123456789:;<=3D>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmn=
opqrstuvwxyz{|}~=7F=80=81=82=83=84=85=86=87=88=89=8A=8B=8C=8D=8E=8F=90=91=
=92=93=94=95=96=97=98=99=9A=9B=9C=9D=9E=9F=A0=A1=A2=A3=A4=A5=A6=A7=A8=A9=
=AA=AB=AC=AD=AE=AF=B0=B1=B2=B3=B4=B5=B6=B7=B8=B9=BA=BB=BC=BD=BE=BF=C0=C1=
=C2=C3=C4=C5=C6=C7=C8=C9=CA=CB=CC=CD=CE=CF=D0=D1=D2=D3=D4=D5=D6=D7=D8=D9=
=DA=DB=DC=DD=DE=DF=E0=E1=E2=E3=E4=E5=E6=E7=E8=E9=EA=EB=EC=ED=EE=EF=F0=F1=
=F2=F3=F4=F5=F6=F7=F8=F9=FA=FB=FC=FD=FE=FF='
		MOVE OUTPUT test.dat
		ERROR - $'0e9c18ae6849579ac9882ca9d56adc8b'
	EXEC	--decode=qp^sum-md5
		SAME INPUT test.dat
		SAME OUTPUT chars.dat

TEST 03 'uu boundaries'
	DO	DATA 0.dat 1.dat 2.dat 3.dat abc.dat

	for u in posix mime bsd
	do	for t in '' -text
		do	for f in 0 1 2 3 abc
			do

	EXEC	--encode=uu-$u$t
		INPUT $f.dat
		MOVE OUTPUT test.dat
	EXEC	--decode=uu-$u$t
		SAME INPUT test.dat
		SAME OUTPUT $f.dat

	done
	done
	done
