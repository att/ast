# regression tests for the sortsync sort discipline library

TITLE + sync

unset SORTIN SORTOUT SORTOF01 SORTOF02 SORTOF03

export LC_ALL=C

VIEW data

TEST 01 'basics'

	EXEC -lsync,control=$data/s.ss,list
		OUTPUT - $'FILE COPY out
  KEY -k.1.3 -k.5.1r
  SORT
        0    3 a 0
        4    1 a 1
  SUM
        4    3 D 0
FILE size=4 01
  OUT
        0    3 v 0
        3    1 v 0 0a
FILE size=4 02
  INCLUDE=([6,1,a]!=\'31\'&&[1,2,b]!=\'bead\')
  OUT
        4    3 v 0
        3    1 v 0 0a
FILE size=15 03
  OUT
        0    3 v 0
        3    2 v 0 0000
        0    3 v 0 202020
        0    1 v 0 00
        0    1 v 0 20
        0    4 v 0 ffffffff
        0    1 v 0 0a'

	EXEC -lsync,control=$data/t.ss,list
		OUTPUT - $'FILE COPY out
  KEY -k.1.3
  SORT
        0    3 a 0
  SUM
        4    3 D 0
FILE size=4 01
  OUT
        0    3 v 0
        3    1 v 0 0a
FILE size=4 02
  INCLUDE=([6,1,a]!=\'31\'&&[1,2,b]!=\'bead\')
  OUT
        4    3 v 0
        3    1 v 0 0a'

	EXEC -lsync,control=$data/t.ss,out01=1.dat,out02=2.dat $data/t.dat
		OUTPUT - $'aaa 111 001
bbb 333 002
ccc 555 003'
		OUTPUT 1.dat $'aaa
bbb
ccc'
		OUTPUT 2.dat $'333
555'

	EXPORT SORTOF01=1.bad SORTOF02=2.bad

	EXEC -lsync,control=$data/t.ss,out01=1.1.dat,out02=1.2.dat $data/t.dat
		SAME 1.1.dat 1.dat
		SAME 1.2.dat 2.dat

	EXPORT SORTOF01=2.1.dat SORTOF02=2.2.dat

	EXEC -lsync,control=$data/t.ss $data/t.dat
		SAME 2.1.dat 1.dat
		SAME 2.2.dat 2.dat

	EXPORT SORTOF01=3.1.dat SORTOF02=3.2.dat

	EXEC -lsync $data/t.dat
		SAME INPUT $data/t.ss
		SAME 3.1.dat 1.dat
		SAME 3.2.dat 2.dat

	EXEC -lsync,out01=01.dat $data/t.dat
		INPUT - $'SORT FIELDS=COPY
  OUTFIL FILES=01,
         OUTREC=(1,3,4,A\'\\n\')'
		OUTPUT - $'aaa 001 001
bbb 003 002
ccc 005 003
aaa 010 004
bbb 030 005
ccc 050 006
aaa 100 007
bbb 300 008
ccc 500 009'
		OUTPUT 01.dat $'aaa
bbb
ccc
aaa
bbb
ccc
aaa
bbb
ccc'

	EXEC -lsync,out01=4.1.dat,out02=4.2.dat $data/t.dat
		INPUT - $'SORT FIELDS=COPY
  OUTFIL FILES=01,
         OUTREC=(1,3,4,A\'\\n\')
  OUTFIL FILES=02,
         INCLUDE=(7,1,AC,NE,A\'1\',AND,2,2,NE,B\'1011111010101101\'),
         OUTREC=(5,3,4,A\'\\n\')'
		OUTPUT - $'aaa 001 001
bbb 003 002
ccc 005 003
aaa 010 004
bbb 030 005
ccc 050 006
aaa 100 007
bbb 300 008
ccc 500 009'
		OUTPUT 4.1.dat $'aaa
bbb
ccc
aaa
bbb
ccc
aaa
bbb
ccc'
		OUTPUT 4.2.dat $'003
005
010
030
050
100
300
500'

	EXEC -lsync,out01=5.1.dat,out02=5.2.dat $data/t.dat
		INPUT - $'SORT
  OUTFIL FILES=01,
         OUTREC=(1,3,4,A\'\\n\')
  OUTFIL FILES=02,
         INCLUDE=(7,1,AC,NE,A\'1\',AND,2,2,NE,B\'1011111010101101\'),
         OUTREC=(5,3,4,A\'\\n\')'
		OUTPUT - $'aaa 001 001
aaa 010 004
aaa 100 007
bbb 003 002
bbb 030 005
bbb 300 008
ccc 005 003
ccc 050 006
ccc 500 009'
		OUTPUT 5.1.dat $'aaa
aaa
aaa
bbb
bbb
bbb
ccc
ccc
ccc'
		OUTPUT 5.2.dat $'010
100
003
030
300
005
050
500'

TEST 02 'sum'

	EXEC -lsync,control=$data/u.ss $data/t.dat
		OUTPUT - $'999'

	EXEC -lsync,control=$data/u.ss $data/t.dat -o u.out
		OUTPUT -
		OUTPUT u.out $'999'

	EXPORT SORTXSUM=xsum.out

	EXEC -lsync,control=$data/v.ss $data/t.dat -o v.out
		OUTPUT -
		OUTPUT v.out $'999'
		OUTPUT xsum.out $'bbb 003 002
ccc 005 003
aaa 010 004
bbb 030 005
ccc 050 006
aaa 100 007
bbb 300 008
ccc 500 009'

TEST 03 'ebcdic'

	EXEC -R12 -lsync,control=$data/e.ss,out1=e.out,out2=s.out $data/e.dat
		OUTPUT -n - $'\x81\x81\x81\x40\xf0\xf0\xf1\x40\xa7\xa7\xa7\x15\x82\x82\x82\x40\xf0\xf0\xf3\x40\xa7\xa7\xa7\x15\x83\x83\x83\x40\xf0\xf0\xf5\x40\xa7\xa7\xa7\x15'
		OUTPUT e%4.out
		OUTPUT -n s%4.out $'\xf0\xf0\xf1\x15\xf0\xf0\xf3\x15\xf0\xf0\xf5\x15'

	EXEC -lsync"
			lrecl=12
			code=ebcdic-o
			control=$data/e.ss
			out1=e.1.out
			out2=s.1.out
		" $data/e.dat
		SAME e.1%4.out e%4.out
		SAME s.1%4.out s%4.out

	EXEC -lsync"
			control=$data/x.ss
			out1=e.2.out
			out2=s.2.out
		" $data/e.dat
		SAME e.2%4.out e%4.out
		SAME s.2%4.out s%4.out

	EXEC -lsync,control=$data/f.ss,out-int=e.3.out $data/e.dat
		SAME e.3%4.out e%4.out

	EXEC -R12 -lsync,control=$data/e.ss,out1=e.4.out,out2=s.4.out -o e1.out $data/e.dat
		OUTPUT -
		OUTPUT -n e1%12.out $'\x81\x81\x81\x40\xf0\xf0\xf1\x40\xa7\xa7\xa7\x15\x82\x82\x82\x40\xf0\xf0\xf3\x40\xa7\xa7\xa7\x15\x83\x83\x83\x40\xf0\xf0\xf5\x40\xa7\xa7\xa7\x15'
		OUTPUT e%4.out
		SAME e.4%4.out e%4.out
		SAME s.4%4.out s%4.out

	EXEC -R12 -lsync,control=$data/e.ss,out1=e.5.out,out2=s.5.out -o /dev/null $data/e.dat
		SAME e.5%4.out e%4.out
		SAME s.5%4.out s%4.out

	EXEC -lsync,control=$data/g.ss $data/e.dat

	EXEC -lsync,control=$data/h.ss $data/e.dat

	EXEC -lsync,control=$data/k.ss $data/e.dat

TEST 04 'ebcdic with marked input'

	EXEC -R% -lsync,control=$data/e.ss,out1=e.out,out2=s.out $data/e%12.dat
		OUTPUT -n - $'\x81\x81\x81\x40\xf0\xf0\xf1\x40\xa7\xa7\xa7\x15\x82\x82\x82\x40\xf0\xf0\xf3\x40\xa7\xa7\xa7\x15\x83\x83\x83\x40\xf0\xf0\xf5\x40\xa7\xa7\xa7\x15'
		OUTPUT e%4.out
		OUTPUT -n s%4.out $'\xf0\xf0\xf1\x15\xf0\xf0\xf3\x15\xf0\xf0\xf5\x15'

	EXEC -R% -lsync"
			code=ebcdic-o
			control=$data/e.ss
			out1=e.1.out
			out2=s.1.out
		" $data/e%12.dat
		SAME e.1%4.out e%4.out
		SAME s.1%4.out s%4.out

	EXEC -R% -lsync"
			control=$data/x.ss
			out1=e.2%4.out
			out2=s.2%4.out
		" $data/e%12.dat
		SAME e.2%4.out e%4.out
		SAME s.2%4.out s%4.out

	EXEC -R% -lsync,control=$data/f.ss,out-int=e.3.out $data/e%12.dat
		SAME e.3%4.out e%4.out

	EXEC -R% -lsync,control=$data/f.ss,out-int=e.3.out $data/e.f.dat
		SAME e.3%4.out e%4.out

	EXEC -R% -lsync,control=$data/e.ss,out1=e.4%4.out,out2=s.4%4.out -o e1%12.out $data/e%12.dat
		OUTPUT -
		OUTPUT -n e1%12.out $'\x81\x81\x81\x40\xf0\xf0\xf1\x40\xa7\xa7\xa7\x15\x82\x82\x82\x40\xf0\xf0\xf3\x40\xa7\xa7\xa7\x15\x83\x83\x83\x40\xf0\xf0\xf5\x40\xa7\xa7\xa7\x15'
		OUTPUT e%4.out
		SAME e.4%4.out e%4.out
		SAME s.4%4.out s%4.out

	EXEC -R% -lsync,control=$data/e.ss,out1=e.5.out,out2=s.5.out -o /dev/null $data/e%12.dat
		SAME e.5%4.out e%4.out
		SAME s.5%4.out s%4.out

	EXEC -R% -lsync,control=$data/g.ss $data/e%12.dat

	EXEC -R% -lsync,control=$data/h.ss $data/e%12.dat

	EXEC -R% -lsync,control=$data/k.ss $data/e%12.dat

TEST 05 'detailed selection'

	EXEC -lsync,control=$data/i.ss,out1=1,out2=2,out3=3,out4=4 -o 0 $data/i.dat
		OUTPUT -
		OUTPUT 0 $'aO0YX44\nbO0NX55\ncO0YX55\ndI1XY31\neI0XY31\nfOABN72\ngUABY72'
		OUTPUT 1 $'cO0\neI0\ngUA'
		OUTPUT 2 $'dI1\neI0'
		OUTPUT 3 $'aO0\nbO0\ncO0\nfOA'
		OUTPUT 4 $'gUA'

TEST 06 'junk report'

	EXEC -lsync,control=$data/t.ss,junk=j.out $data/t.dat
		OUTPUT - $'aaa 111 001
bbb 333 002
ccc 555 003'
		OUTPUT j.out $'  11        6'

TEST 07 'packed decimal'

	EXEC -lsync,control=$data/p.ss $data/p.dat
		OUTPUT -n - $'\x01\x17\x7c\x30\x31\x01\x17\x7d\x30\x32'

TEST 08 'zoned decimal'

	EXEC -lsync,control=$data/z.ss $data/z.dat
		OUTPUT -n - $'\xf0\xf1\xf1\xf7\xc7\x30\x31\xf0\xf1\xf1\xf7\xd7\x30\x32'

TEST 09 'selection expressions'

	EXEC -lsync,control=lt.ss
		INPUT - $'9\n8\n7\n6\n5\n4\n3\n2\n1\n0'
		INPUT lt.ss $'RECORD TYPE=F,LENGTH=2\nSORT FIELDS=(1,1,AC,A)\nINCLUDE=(1,1,AC,LT,A\'3\')'
		OUTPUT - $'0\n1\n2'

	EXEC -lsync,control=le.ss
		INPUT le.ss $'RECORD TYPE=F,LENGTH=2\nSORT FIELDS=(1,1,AC,A)\nINCLUDE=(1,1,AC,LE,A\'2\')'

	EXEC -lsync,control=gt.ss
		INPUT gt.ss $'RECORD TYPE=F,LENGTH=2\nSORT FIELDS=(1,1,AC,A)\nINCLUDE=(1,1,AC,GT,A\'6\')'
		OUTPUT - $'7\n8\n9'

	EXEC -lsync,control=ge.ss
		INPUT ge.ss $'RECORD TYPE=F,LENGTH=2\nSORT FIELDS=(1,1,AC,A)\nINCLUDE=(1,1,AC,GE,A\'7\')'

	EXEC -lsync,control=eq.ss
		INPUT eq.ss $'RECORD TYPE=F,LENGTH=2\nSORT FIELDS=(1,1,AC,A)\nINCLUDE=(1,1,AC,EQ,A\'5\')'
		OUTPUT - $'5'

	EXEC -lsync,control=ne.ss
		INPUT ne.ss $'RECORD TYPE=F,LENGTH=2\nSORT FIELDS=(1,1,AC,A)\nINCLUDE=(1,1,AC,NE,A\'5\')'
		OUTPUT - $'0\n1\n2\n3\n4\n6\n7\n8\n9'

	EXEC -lsync,control=and.ss
		INPUT and.ss $'RECORD TYPE=F,LENGTH=2\nSORT FIELDS=(1,1,AC,A)\nINCLUDE=(1,1,AC,NE,A\'0\',AND,1,1,AC,NE,A\'9\')'
		OUTPUT - $'1\n2\n3\n4\n5\n6\n7\n8'

	EXEC -lsync,control=nor.ss
		INPUT nor.ss $'RECORD TYPE=F,LENGTH=2\nSORT FIELDS=(1,1,AC,A)\nINCLUDE=(NOT,(1,1,AC,EQ,A\'0\',OR,1,1,AC,EQ,A\'9\'))'

	EXEC -lsync,control=andor.ss
		INPUT andor.ss $'RECORD TYPE=F,LENGTH=2\nSORT FIELDS=(1,1,AC,A)\nINCLUDE=(1,1,AC,GT,A\'0\',AND,1,1,AC,LT,A\'2\',OR,\n1,1,AC,GT,A\'7\',AND,1,1,AC,LT,A\'9\')'
		OUTPUT - $'1\n8'

	EXEC -lsync,control=andor1.ss
		INPUT andor1.ss $'RECORD TYPE=F,LENGTH=2\nSORT FIELDS=(1,1,AC,A)\nINCLUDE=(1,1,AC,GT,A\'0\',AND,1,1,AC,LT,A\'2\',\nOR,1,1,AC,GT,A\'7\',AND,1,1,AC,LT,A\'9\')'

	EXEC -lsync,control=andor2.ss
		INPUT andor2.ss $'RECORD TYPE=F,LENGTH=2\nSORT FIELDS=(1,1,AC,A)\nINCLUDE=(1,1,AC,GT,A\'0\',AND,1,1,AC,LT,A\'2\',\nOR,\n1,1,AC,GT,A\'7\',AND,1,1,AC,LT,A\'9\')'

	EXEC -lsync,control=andor3.ss
		INPUT andor3.ss $'RECORD TYPE=F,LENGTH=2\nSORT FIELDS=(1,1,AC,A)\nINCLUDE=(1,1,AC,GT,A\'0\',AND,1,1,AC,LT,A\'2\',OR,\n1,1,AC,GT,A\'7\',AND,1,1,AC,LT,A\'9\')'

	EXEC -lsync,control=pandora.ss
		INPUT pandora.ss $'RECORD TYPE=F,LENGTH=2\nSORT FIELDS=(1,1,AC,A)\nINCLUDE=((1,1,AC,GT,A\'0\',AND,1,1,AC,LT,A\'2\'),\nOR,\n(1,1,AC,GT,A\'7\',AND,1,1,AC,LT,A\'9\'))'

	EXEC -lsync,control=pandorb.ss
		INPUT pandorb.ss $'RECORD TYPE=F,LENGTH=2\nSORT FIELDS=(1,1,AC,A)\nINCLUDE=(1,1,AC,GT,A\'0\',AND,(1,1,AC,LT,A\'2\',\nOR,\n1,1,AC,GT,A\'7\'),AND,1,1,AC,LT,A\'9\')'

TEST 10 '/dev/null => RS_IGNORE'

	EXEC -lsync,control=$data/c.ss -Xdump -za16ki -zi16ki -zm16 -zo16ki -o /dev/null /dev/null
		ERROR - $'main
	intermediates=16
state
	method=rasp
	insize=16384 outsize=16384
	alignsize=8192 procsize=16384 recsize=0
	merge=0 reverse=0 stable=1 uniq=0 ignore=1 verbose=0
	tab=\' \' keys= maxfield=0
	record format d data 0x0000000a key -1
field[0]
	begin field = 0
	 begin char = 0
	  end field = 2147483647
	   end char = 0
	      ccode = 0
	      coder = t
	       keep = all
	      trans = ident
	      bflag = 0
	      eflag = 0
	      rflag = 0
	      style = obsolete'

TEST 11 'catenation'

	EXEC -lsync c b a
		INPUT - $'SORT FIELDS=COPY'
		INPUT a $'aaa'
		INPUT b $'bbb'
		INPUT c $'ccc'
		OUTPUT - $'ccc\nbbb\naaa'

TEST 12 'INREC reformat'

	EXEC -lsync,control=$data/r.ss,list
		OUTPUT - $'FILE size=8 in
  INREC
        2    8 v 0
FILE COPY out
  COPY
  STOPAFT 5
  INCLUDE=[0,1,c]==\'f1\''

	EXEC -lsync $data/t.dat
		INPUT - $'MERGE FIELDS=COPY'
		SAME OUTPUT $data/t.dat

	EXEC -lsync $data/t.dat
		INPUT - $'SORT FIELDS=(1,3),SKIPREC=2,STOPAFT=4\nINREC FIELDS=(5,8)'
		OUTPUT - $'005 003
010 004
030 005
050 006'

	EXEC -lsync $data/s.dat
		INPUT - $'OPTION EQUALS\nSORT FIELDS=(1,3)\nINREC FIELDS=(1,4,9,4)'
		OUTPUT - $'aaa 001
aaa 004
aaa 007
bbb 002
bbb 005
bbb 008
ccc 003
ccc 006
ccc 009'

	EXEC -lsync $data/s.dat
		INPUT - $'SORT FIELDS=(1,3)\nINREC FIELDS=(1,4,9,4)'

	EXEC -lsync $data/c.dat
		INPUT - $'
  INCLUDE COND=(5,1,GE,A\'M\'),FORMAT=CH
    INREC FIELDS=(10,3,20,8,33,11,5,1,80,1)
     SORT FIELDS=(4,8,CH,A,1,3,FI,A)
      SUM FIELDS=(17,4,BI)'
		OUTPUT - $'M11M12mmmmmM13mmmmmmmmM
M11M22mmmmmM23mmmmmmmmM
N21N12nnnnnN13nnnnnnnnN
N21N22nnnnnN23nnnnnnnnN
Z11Z12zzzzzZ13zzzzzzzzZ
Z21Z22zzzzzZ23zzzzzzzzZ'

	EXEC -lsync $data/c.dat
		INPUT - $'
  INCLUDE COND=(5,1,GE,A\'M\'),FORMAT=CH
   OUTREC FIELDS=(10,3,20,8,33,11,5,1,80,1)
     SORT FIELDS=(20,8,CH,A,10,3,FI,A)
      SUM FIELDS=(38,4,BI)'

TEST 13 'merge with E35 exit'

	EXPORT SORT_E35_STATUS=AAAC
	EXEC -lsync,control=merge.ss c d b a
		INPUT merge.ss $'MERGE FIELDS=(1,2,CH,A)\nMODS E35=(E35,100,test)'
		INPUT a $'aa\nzz'
		INPUT b $'bb\nyy'
		INPUT c $'cc\nxx'
		INPUT d $'dd\nww'
		OUTPUT - $'aa\nbb\ncc\ndd\nww\nxx\nyy\nzz'
		ERROR - $'sort exit E35 1 0 A [3] "aa"
sort exit E35 2 4 A [3] "bb"
sort exit E35 3 4 A [3] "cc"
sort exit E35 4 4 C [3] "dd"
sort exit E35 5 4 C [3] "ww"
sort exit E35 6 4 C [3] "xx"
sort exit E35 7 4 C [3] "yy"
sort exit E35 8 4 C [3] "zz"'

	EXPORT SORT_E35_STATUS=DADADADA
	EXEC
		OUTPUT - $'bb\ndd\nxx\nzz'

		ERROR - $'sort exit E35 1 0 D [3] "aa"
sort exit E35 2 4 A [3] "bb"
sort exit E35 3 4 D [3] "cc"
sort exit E35 4 4 A [3] "dd"
sort exit E35 5 4 D [3] "ww"
sort exit E35 6 4 A [3] "xx"
sort exit E35 7 4 D [3] "yy"
sort exit E35 8 4 A [3] "zz"'

	EXPORT SORT_E35_STATUS=A
	EXEC -lsync,control=merge.ss y z
		INPUT merge.ss $'MERGE FIELDS=(1,2,CH,A)\nMODS E35=(E35,100,test)'
		INPUT y $'aa\nbb\ncc\ndd'
		INPUT z $'ww\nxx\nyy\nzz'
		OUTPUT - $'aa\nbb\ncc\ndd\nww\nxx\nyy\nzz'
		ERROR - $'sort exit E35 1 0 A [3] "aa"
sort exit E35 2 4 A [3] "bb"
sort exit E35 3 4 A [3] "cc"
sort exit E35 4 4 A [3] "dd"
sort exit E35 5 4 A [3] "ww"
sort exit E35 6 4 A [3] "xx"
sort exit E35 7 4 A [3] "yy"
sort exit E35 8 4 A [3] "zz"'

	EXEC -lsync,control=merge.ss z y

	EXEC -lsync,control=merge.ss z y z
		OUTPUT - $'aa\nbb\ncc\ndd\nww\nww\nxx\nxx\nyy\nyy\nzz\nzz'
		ERROR - $'sort exit E35 1 0 A [3] "aa"
sort exit E35 2 4 A [3] "bb"
sort exit E35 3 4 A [3] "cc"
sort exit E35 4 4 A [3] "dd"
sort exit E35 5 4 A [3] "ww"
sort exit E35 6 4 A [3] "ww"
sort exit E35 7 4 A [3] "xx"
sort exit E35 8 4 A [3] "xx"
sort exit E35 9 4 A [3] "yy"
sort exit E35 10 4 A [3] "yy"
sort exit E35 11 4 A [3] "zz"
sort exit E35 12 4 A [3] "zz"'

	EXEC -lsync,control=exit.ss z y z
		INPUT exit.ss $'SORT FIELDS=(1,2,CH,A)\nMODS E35=(E35,100,test)'

TEST 14 'sort with E15 exit'

	EXPORT SORT_E15_STATUS=MMMMMMMMC
	EXEC -lsync,control=sort.ss c d b a
		INPUT sort.ss $'SORT FIELDS=(1,2,CH,A)\nMODS E15=(E15,100,test)'
		INPUT a $'aa\nzz'
		INPUT b $'bb\nyy'
		INPUT c $'cc\nxx'
		INPUT d $'dd\nww'
		OUTPUT - $'az\nbb\ncx\nww\nxx\nyy\nzz'
		ERROR - $'sort exit E15 1 0 M [3] "cx"
sort exit E15 2 4 M [3] "xx"
sort exit E15 3 4 M [3] "dd"
sort exit E15 4 4 M [3] "ww"
sort exit E15 5 4 M [3] "bb"
sort exit E15 6 4 M [3] "yy"
sort exit E15 7 4 M [3] "az"
sort exit E15 8 8 M [3] "zz"'

	EXPORT SORT_E15_STATUS=MMMMMMMMC
	EXEC -lsync,control=sort.ss c d b a
		INPUT sort.ss $'SORT FIELDS=(1,1,CH,A,2,1,CH,D)\nMODS E15=(E15,100,test)'
		INPUT a $'aa\nab'
		OUTPUT - $'az\nab\nbb\ncx\nww\nxx\nyy'
		ERROR - $'sort exit E15 1 0 M [3] "cx"
sort exit E15 2 4 M [3] "xx"
sort exit E15 3 4 M [3] "dd"
sort exit E15 4 4 M [3] "ww"
sort exit E15 5 4 M [3] "bb"
sort exit E15 6 4 M [3] "yy"
sort exit E15 7 4 M [3] "az"
sort exit E15 8 8 M [3] "ab"'

TEST 15 'duplicate key diagnostic'

	EXEC -lsync,control=empty.ss,duplicates c d b a
		INPUT empty.ss
		INPUT a $'aa\ndd'
		INPUT b $'bb\nyy'
		INPUT c $'cc\naa'
		INPUT d $'dd\nww'
		OUTPUT - $'aa\naa\nbb\ncc\ndd\ndd\nww\nyy'
		ERROR - $'2 duplicate keys'

	EXEC -u -lsync,control=empty.ss,duplicates c d b a
		OUTPUT - $'aa\nbb\ncc\ndd\nww\nyy'

TEST 16 'base name size/suffix mapping'

	EXEC -lsync,control=empty.ss a b.b c.c.c
		INPUT empty.ss
		INPUT a%123.foo $'a123foo'
		INPUT b%123.0z $'b0z'
		INPUT b.b%123.1z $'b1z'
		INPUT c%123.0z $'c0z'
		INPUT c.c%123.1z $'c1z'
		INPUT c.c.c%123.2z $'c2z'
		OUTPUT - $'a123foo\nb1z\nc2z'

	EXEC -lsync,control=empty.ss a e.e
		INPUT e.e%789.1z $'b1z'
		OUTPUT -
		ERROR - $'sort: e.e%789.1z: format f789 incompatible with a%123.foo format f123'
		EXIT 1
