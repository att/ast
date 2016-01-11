 RECORD TYPE=F,LENGTH=8,CODESET=ascii
 SORT FIELDS=(1,1,CH,A)
  OUTFIL FILES=1,
         INCLUDE=((02,01,CH,EQ,C'O',AND,
                   06,02,CH,EQ,C'55',AND,
                   04,01,CH,EQ,C'Y',AND,
                   03,01,CH,EQ,C'0'),OR,
                  (02,01,CH,EQ,C'I',AND,
                   05,01,CH,EQ,C'Y',AND,
                   03,01,CH,EQ,C'0'),OR,
                  (02,01,CH,EQ,C'U',AND,
                   05,01,CH,EQ,C'Y')),
         OUTREC=(1,3,C'\n')
  OUTFIL FILES=2,
         INCLUDE=(02,01,CH,EQ,C'I'),
         OUTREC=(1,3,C'\n')
  OUTFIL FILES=3,
         INCLUDE=(02,01,CH,EQ,C'O'),
         OUTREC=(1,3,C'\n')
  OUTFIL FILES=4,
         INCLUDE=(02,01,CH,EQ,C'U'),
         OUTREC=(1,3,C'\n')
