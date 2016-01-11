# : : generated from jcl.rt by mktest : : #

# regression tests for the jcl command

UNIT jcl

TEST 01 basics

	EXEC	-nvw -m-
		INPUT - $'//TEST     JOB    TEST                   job comment
//SET1     SET    A=123,B=456,C=789      default values
//STEP1    EXEC   PGM=IEBGENER           exec comment
//SYSIN    DD     TEST.&A..&B..&C..TEST, dd comment
//                LRECL=80               continuation comment
//STEP2    EXEC   PGM=IEFBR14            pgm comment
//SYSIN    DD     *
here
//*                                      comment comment
//STEP3    EXEC   PGM=TEST               pgm comment'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: STEP1
STEP=STEP1 \\
DDIN=\'\' \\
DDOUT=\'\' \\
IEBGENER < TEST.123.456.789.TEST
code=$?
: STEP2
STEP=STEP2 \\
DDIN=\'\' \\
DDOUT=\'\' \\
IEFBR14 <<\'/*\'
here
/*
code=$?
: STEP3
STEP=STEP3 \\
DDIN=\'\' \\
DDOUT=\'\' \\
TEST
code=$?'

	EXEC	-nvw -m-
		INPUT - $'//NNNNNNNN JOB \'#BOZO\',CLASS=C,
//      MSGCLASS=8,REGION=4096K
//MMMMMMMM EXEC PGM=TEST
//SYSIN    DD   DSN=IN,
//            DISP=SHR                                   * XXX XXX XXXX
//SYSOUT   DD   DSN=FOO$OUT,
//            DISP=(NEW,CATLG),                          * YYYYYYYY
//            DCB=(RECFM=FB,BLKSIZE=0,LRECL=74),UNIT=(DISK,16)
//TEMP1    DD   DSN=FOO$TMP,
//            DISP=(NEW,CATLG)                           * ZZZZZZZZ
//TEMP2    DD   DSN=\'BAR TMP\',
//            DISP=(MOD,CATLG)                           * ZZZZZZZZ
//SYSERR   DD   DSNAME=ERR'
		OUTPUT - $': JOB NNNNNNNN
export JCL_AUTO_JOBNAME=NNNNNNNN
code=0
: MMMMMMMM
STEP=MMMMMMMM \\
TEMP1=FOO\\$TMP \\
TEMP2=+"BAR TMP" \\
DDIN=\'\' \\
DDOUT=\'TEMP1 TEMP2\' \\
TEST < IN > FOO\\$OUT 2> ERR
code=$?'

	EXEC	-nvw -m-
		INPUT - $'//NNNNNNNN JOB \'#BOZO\',CLASS=C,
//      MSGCLASS=8,REGION=4096K
//MMMMMMMM EXEC PGM=TEST
//SYSIN    DD   DSN=IN,
//            DISP=SHR                                   * XXX XXX XXXX
//OUTPUT   DD   DSN=OUT,
//            DISP=(MOD,CATLG),                          * YYYYYYYY
//            DCB=(RECFM=FB,BLKSIZE=0,LRECL=74),UNIT=(DISK,16)
//SYSERR   DD   DSNAME=ERR'
		OUTPUT - $': JOB NNNNNNNN
export JCL_AUTO_JOBNAME=NNNNNNNN
code=0
: MMMMMMMM
STEP=MMMMMMMM \\
OUTPUT=+OUT \\
DDIN=\'\' \\
DDOUT=\'OUTPUT\' \\
TEST < IN 2> ERR
code=$?'

TEST 02 continuation

	EXEC	-nvw -m-
		INPUT - $'//TEST     JOB
//S1       EXEC PGM=TEST
//D1       DD   DSNAME=SWITCH.LEVEL18.GROUP12,UNIT=3350,
//         VOLUME=335023,SPACE=(TRK,(80,15)),DISP=(,PASS)
//D2       DD   DSNAME=INDS,DISP=OLD,CHKPT=EOV,   MY INPUT DATA SET
//         UNIT=SYSSQ,VOLUME=SER=(TAPE01,TAPE02,TAPE03)
//S2       EXEC PGM=BILLING,COND.PAID=((20,LT),EVEN),
//         COND.LATE=(60,GT,FIND),
//         COND.BILL=((20,GE),(30,LT,CHGE))  THIS STATEMENT CALLS      X
//         THE BILLING PROCEDURE AND SPECIFIES RETURN CODE TESTS       X
//         FOR THREE PROCEDURE STEPS
//*     10        20        30        40        50        60        70
//*4567890123456789012345678901234567890123456789012345678901234567890123456'\
$'7890
//S3       EXEC PGM=IEFBR14,PARM=\'THIS IS A LONG PARAMETER WITHIN APOST
//              ROPHES, CONTINUED IN COLUMN 16 OF THE NEXT RECORD\''
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: S1
STEP=S1 \\
D1=SWITCH.LEVEL18.GROUP12 \\
D2=INDS \\
DDIN=\'D1 D2\' \\
DDOUT=\'\' \\
TEST
code=$?
: S2
STEP=S2 \\
DDIN=\'\' \\
DDOUT=\'\' \\
BILLING COND.BILL=\'((20,GE),(30,LT,CHGE))\' COND.LATE=\'(60,GT,FIND)\' COND'\
$'.PAID=\'((20,LT),EVEN)\'
code=$?
: S3
STEP=S3 \\
DDIN=\'\' \\
DDOUT=\'\' \\
IEFBR14 \'THIS IS A LONG PARAMETER WITHIN APOSTROPHES, CONTINUED IN COLUMN 1'\
$'6 OF THE NEXT RECORD\'
code=$?'

	EXEC	-nvw -m-
		INPUT - $'//TEST     JOB
//*     10        20        30        40        50        60        70
//*4567890123456789012345678901234567890123456789012345678901234567890123456'\
$'7890
// SET  AAA=                        XXXXXXXX YYYYYYY                  #
// SET  BBB=bbb                     XXXXXXXX YYYYYYY                  #
// SET  CCC=ccc                     XXXXXXXX YYYYYYY                   #
// SET  DDD=ddd                     XXXXXXXX YYYYYYY                   #
// SET  EEE=eee                     XXXXXXXX YYYYYYY                  #
//      EXEC  PGM=ECHO,PARM=&AAA.&BBB.&CCC.&DDD.&EEE.'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO \'bbbccc&DDD.&EEE.\'
code=$?'
		ERROR - $'jcl: line 9: warning: &DDD: undefined variable
jcl: line 9: warning: &EEE: undefined variable'

TEST 03 quoting

	EXEC	-nvw -m-
		INPUT - $'//* TEST
//TEST     PROC   A=,B=\'\',C=\'\'\'\'\'c\'\'\'\'\',D=\'o\'\'\'\'k\',E=O\'\''\
$'\'\'\'\'K
//STEP     EXEC   PGM=TEST
//SYSIN    DD     \':A=&A:B=&B:C=&C:D=&D:E=&E:&&A:&&Z:\''
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: STEP
STEP=STEP \\
DDIN=\'\' \\
DDOUT=\'\' \\
TEST < ":A=:B=:C=\'c\':D=o\'k:E=O\'K:&&A:&&Z:"
code=$?'

	EXEC	-nvw -m-
		INPUT - $'//TEST JOB\n//STEP     EXEC   PGM=TEST,PARM=(\'FILE=AAA,LIST=Y,,EMPTY\')'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: STEP
STEP=STEP \\
DDIN=\'\' \\
DDOUT=\'\' \\
TEST FILE=AAA,LIST=Y,,EMPTY
code=$?'

	EXEC	-nvw -m-
		INPUT - $'//ABC JOB
//    SET  VAR=value
//PDQ EXEC PGM=XYZ,COND=(4,LT),
//         PARM=(\'$SKIP=-1,VAR=&VAR\')'
		OUTPUT - $': JOB ABC
export JCL_AUTO_JOBNAME=ABC
code=0
: PDQ
STEP=PDQ \\
DDIN=\'\' \\
DDOUT=\'\' \\
XYZ \'$SKIP=-1,VAR=value\'
code=$?'

TEST 04 'PROC variable scope'

	EXEC	-nvw -m-
		INPUT - $'//EXAMPLE   PROC SYM1=\'What\'\'\'\'s up, Doc?\',SYM2=(DEF),SYM3=&&&&TEMP1,
//       SYM4=\'&&TEMP2\',SYM5=&&TEMP3,TEMP3=TEMPNAME,
//       SYM6=&TEMP3
//S1        EXEC PGM=WTO,PARM=\'&SYM1\',ACCT=&SYM2
//DD1       DD   DSN=&SYM3,UNIT=SYSDA,SPACE=(TRK,(1,1))
//DD2       DD   DSN=&SYM4,UNIT=SYSDA,SPACE=(TRK,(1,1))
//DD3       DD   DSN=&SYM5,UNIT=SYSDA,SPACE=(TRK,(1,1))
//DD4       DD   DSN=&SYM6,UNIT=SYSDA,SPACE=(TRK,(1,1))
//          PEND'
		OUTPUT - $': JOB EXAMPLE
export JCL_AUTO_JOBNAME=EXAMPLE
code=0
trap \'code=$?; rm -rf ${TMPDIR:-/tmp}/job.EXAMPLE.$$.*; exit $code\' 0 1 2
: S1
STEP=S1 \\
DD1=${TMPDIR:-/tmp}/job.EXAMPLE.$$.TEMP1 \\
DD2=${TMPDIR:-/tmp}/job.EXAMPLE.$$.TEMP2 \\
DD3=${TMPDIR:-/tmp}/job.EXAMPLE.$$.TEMP3 \\
DD4=${TMPDIR:-/tmp}/job.EXAMPLE.$$.TEMP3 \\
DDIN=\'DD1 DD2 DD3 DD4\' \\
DDOUT=\'\' \\
WTO $\'What\\\'s up, Doc?\' ACCT=\'(DEF)\'
code=$?'
		ERROR - 'jcl: line 3: warning: &TEMP3: undefined variable'

	EXEC	-nvw -m-
		INPUT - $'//TEST01   JOB    TEST
//SET01    SET    A=A0,B=B0
//*
//STEP01   EXEC   PGM=TEST01
//SYSIN    DD     TEST.&A..&B..&C..TEST01
//*
//PROC1    PROC   A=A1,C=C1
//STEP11   EXEC   PROC=PROC2,A=A2,B=B2
//STEP12   EXEC   PGM=TEST12
//SYSIN    DD     TEST.&A..&B..&C..TEST12
//         PEND
//*
//PROC2    PROC
//STEP21   EXEC   PGM=TEST21
//SYSIN    DD     TEST.&A..&B..&C..TEST21
//         PEND
//*
//STEP02   EXEC   PROC=PROC1,A=,C=C4
//*
//STEP03   EXEC   PGM=TEST03
//SYSIN    DD     TEST.&A..&B..&C..TEST02
//*'
		OUTPUT - $': JOB TEST01
export JCL_AUTO_JOBNAME=TEST01
code=0
: STEP01
STEP=STEP01 \\
DDIN=\'\' \\
DDOUT=\'\' \\
TEST01 < "TEST.A0.B0.&C..TEST01"
code=$?
: STEP02 PROC PROC1
: STEP11 PROC PROC2
: STEP21
STEP=STEP21 \\
DDIN=\'\' \\
DDOUT=\'\' \\
TEST21 < TEST.A2.B2.C4.TEST21
code=$?
: STEP12
STEP=STEP12 \\
DDIN=\'\' \\
DDOUT=\'\' \\
TEST12 < TEST..B0.C4.TEST12
code=$?
: STEP03
STEP=STEP03 \\
DDIN=\'\' \\
DDOUT=\'\' \\
TEST03 < "TEST.A0.B0.&C..TEST02"
code=$?'
		ERROR - $'jcl: line 5: warning: &C: undefined variable
jcl: line 21: warning: &C: undefined variable'

TEST 05 references

	EXEC	-nvw -m-
		INPUT - $'//JOB1    JOB   TEST
//STEPA   EXEC  PGM=TEST
//DD1     DD    DSNAME=REPORT
//DD2     DD    DSN=TABLE
//DD3     DD    DSNAME=*.DD1'
		OUTPUT - $': JOB JOB1
export JCL_AUTO_JOBNAME=JOB1
code=0
: STEPA
STEP=STEPA \\
DD1=REPORT \\
DD2=TABLE \\
DD3=REPORT \\
DDIN=\'DD1 DD2 DD3\' \\
DDOUT=\'\' \\
TEST
code=$?'

	EXEC	-nvw -m- REF=X
		INPUT - $'//JOB5    JOB   TEST
//AHA     EXEC  PGM=TEST,PARM=AHA
//PROC1   PROC  N=1
//PSTEP1  EXEC  PGM=TEST,PARM=01
//DS1     DD    DSNAME=DATA1
//PSTEP2  EXEC  PGM=TEST,PARM=02
//DS2     DD    DSNAME=DATA2
//        PEND
//CALLER  EXEC  PROC=PROC1
//REF1    DD    DSNAME=&REF..CALLER.PSTEP2.DS2
//NEXT    EXEC  PGM=TEST,PARM=03
//REF2    DD    DSNAME=&REF..CALLER.PSTEP1.DS1'
		OUTPUT - $'export REF=X
: JOB JOB5
export JCL_AUTO_JOBNAME=JOB5
code=0
: AHA
STEP=AHA \\
DDIN=\'\' \\
DDOUT=\'\' \\
TEST AHA
code=$?
: CALLER PROC PROC1
: PSTEP1
STEP=PSTEP1 \\
DS1=DATA1 \\
DDIN=\'DS1\' \\
DDOUT=\'\' \\
TEST 01
code=$?
: PSTEP2
STEP=PSTEP2 \\
DS2=DATA2 \\
DDIN=\'DS2\' \\
DDOUT=\'\' \\
TEST 02
code=$?
: NEXT
STEP=NEXT \\
REF2=X.CALLER.PSTEP1.DS1 \\
DDIN=\'REF2\' \\
DDOUT=\'\' \\
TEST 03
code=$?'

	EXEC	-nvw -m- REF='*'
		OUTPUT - $'export REF="*"
: JOB JOB5
export JCL_AUTO_JOBNAME=JOB5
code=0
: AHA
STEP=AHA \\
DDIN=\'\' \\
DDOUT=\'\' \\
TEST AHA
code=$?
: CALLER PROC PROC1
: PSTEP1
STEP=PSTEP1 \\
DS1=DATA1 \\
DDIN=\'DS1\' \\
DDOUT=\'\' \\
TEST 01
code=$?
: PSTEP2
STEP=PSTEP2 \\
DS2=DATA2 \\
DDIN=\'DS2\' \\
DDOUT=\'\' \\
TEST 02
code=$?'
		ERROR - $'jcl: line 10: warning: PSTEP2.DS2: DD not defined
jcl: line 12: CALLER: step back reference not supported'
		EXIT 1

	EXEC	-nvw -m-
		INPUT - $'//JOB2    JOB   TEST
//STEP1   EXEC  PGM=TEST
//DDA     DD    DSNAME=D58.POK.PUBS01
//STEP2   EXEC  PGM=TEST
//DDB     DD    DSNAME=*.STEP1.DDA'
		OUTPUT - $': JOB JOB2
export JCL_AUTO_JOBNAME=JOB2
code=0
: STEP1
STEP=STEP1 \\
DDA=D58.POK.PUBS01 \\
DDIN=\'DDA\' \\
DDOUT=\'\' \\
TEST
code=$?'
		ERROR - 'jcl: line 5: STEP1: step back reference not supported'

TEST 06 'nested procedure calls'

	EXEC	-nvw -m-
		INPUT - $'//JOB1    JOB   NEST,TEST
//C       PROC
//CS1     EXEC  PGM=GHI
//        PEND
//B       PROC
//BS1     EXEC  PROC=C
//BS2     EXEC  PGM=DEF
//        PEND
//A       PROC
//AS1     EXEC  PROC=B
//AS2     EXEC  PGM=ABC
//        PEND
//STEP1   EXEC  PROC=A
//STEP2   EXEC  PGM=JKL'
		OUTPUT - $': JOB JOB1
export JCL_AUTO_JOBNAME=JOB1
code=0
: STEP1 PROC A
: AS1 PROC B
: BS1 PROC C
: CS1
STEP=CS1 \\
DDIN=\'\' \\
DDOUT=\'\' \\
GHI
code=$?
: BS2
STEP=BS2 \\
DDIN=\'\' \\
DDOUT=\'\' \\
DEF
code=$?
: AS2
STEP=AS2 \\
DDIN=\'\' \\
DDOUT=\'\' \\
ABC
code=$?
: STEP2
STEP=STEP2 \\
DDIN=\'\' \\
DDOUT=\'\' \\
JKL
code=$?'

	EXEC	-nvw -m-
		INPUT - $'//JOBA     JOB  ...
//INSTREAM PROC LOC=POK
//PSTEP    EXEC PGM=WRITER
//DSA      DD   SYSOUT=A,DEST=&LOC
//         PEND
//CALLER   EXEC INSTREAM,LOC=NYC'
		OUTPUT - $': JOB JOBA
export JCL_AUTO_JOBNAME=JOBA
code=0
: CALLER PROC INSTREAM
: PSTEP
STEP=PSTEP \\
DDIN=\'\' \\
DDOUT=\'\' \\
WRITER
code=$?'

	EXEC	-nvw -m-
		INPUT - $'//JOBB      JOB  ...
//INSTREAM  PROC LOC=POK,NUMBER=3350
//PSTEP     EXEC PGM=TEST
//PIN       DD   DSNAME=REPORT,DISP=(OLD,KEEP),UNIT=&NUMBER
//POUT      DD   SYSOUT=A,DEST=&LOC
//          PEND
//CALLER    EXEC PROC=INSTREAM,NUMBER=,LOC=STL
//PSTEP.INDATA  DD   *
(data)
/*'
		OUTPUT - $': JOB JOBB
export JCL_AUTO_JOBNAME=JOBB
code=0
: CALLER PROC INSTREAM
: PSTEP
STEP=PSTEP \\
PIN=REPORT \\
DDIN=\'PIN\' \\
DDOUT=\'\' \\
TEST
code=$?'

	EXEC	-nvw -m-
		INPUT - $'//TEST     JOB
//TESTJCL   PROC
//STEP1     EXEC  TESTJCL1
//          PEND
//TESTJCL1  PROC
//STEP2     EXEC  PGM=IEFBR14,PARM=&PVAL
//SYSUDUMP  DD    SYSOUT=A
//          PEND
//RUNIT     EXEC  TESTJCL,PVAL=EXEC0'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: RUNIT PROC TESTJCL
: STEP1 PROC TESTJCL1
: STEP2
STEP=STEP2 \\
DDIN=\'\' \\
DDOUT=\'\' \\
IEFBR14 EXEC0
code=$?'

TEST 07 'DD override'

	EXEC	-nvw -m-
		INPUT - $'//JOB1    JOB
//* Procedure C:
//C       PROC
//CS1     EXEC  PGM=CCC
//CS1DD1  DD    DSNAME=A.B.C,DISP=SHR
//CS1DD2  DD    SYSOUT=A
//        PEND
//* Procedure B:
//B       PROC
//BS1     EXEC  PROC=C
//CS1.CS1DD1  DD  DSNAME=X.Y.Z       This statement is a valid
//*                                  override of procedure C, stepCS1
//*                                  for DD CS1DD1
//*
//CS1.CS1DD3  DD  SYSOUT=A           This statement is a valid
//*                                  addition to procedure C, step CS1
//BS2     EXEC  PGM=BBB
//BS2DD1  DD    DSNAME=E,DISP=SHR
//        PEND
//* Procedure A:
//A       PROC
//AS1     EXEC  PROC=B
//BS2.BS2DD2  DD  DSNAME=G,DISP=SHR  This statement is a valid
//*                                  addition to procedure B, step BS2
//AS2     EXEC  PGM=AAA
//AS2DD1  DD    DSNAME=E,DISP=SHR
//        PEND
//* Job Stream:
//STEP1   EXEC  PROC=A
//AS2.AS2DD2  DD  DSNAME=G,DISP=SHR  This statement is a valid
//*                                  addition to procedure A, step AS2
//STEP2   EXEC  PGM=IEFBR14'
		OUTPUT - $': JOB JOB1
export JCL_AUTO_JOBNAME=JOB1
code=0
: STEP1 PROC A
: AS1 PROC B
: BS1 PROC C
: CS1
STEP=CS1 \\
CS1DD1=X.Y.Z \\
DDIN=\'CS1DD1\' \\
DDOUT=\'\' \\
CCC
code=$?
: BS2
STEP=BS2 \\
BS2DD1=E \\
BS2DD2=G \\
DDIN=\'BS2DD1 BS2DD2\' \\
DDOUT=\'\' \\
BBB
code=$?
: AS2
STEP=AS2 \\
AS2DD1=E \\
AS2DD2=G \\
DDIN=\'AS2DD1 AS2DD2\' \\
DDOUT=\'\' \\
AAA
code=$?
: STEP2
STEP=STEP2 \\
DDIN=\'\' \\
DDOUT=\'\' \\
IEFBR14
code=$?'

	EXEC	-nvw -m-
		INPUT - $'//JOB1    JOB
//CS1     EXEC  PGM=CCC
//CS1DD1  DD    DSNAME=X.Y.Z,DISP=SHR
//*
//CS1DD2  DD    SYSOUT=A
//CS1DD3  DD    SYSOUT=A
//*
//BS2     EXEC  PGM=BBB
//BS2DD1  DD    DSNAME=E,DISP=SHR
//BS2DD2  DD    DSNAME=G,DISP=SHR
//*
//AS2     EXEC  PGM=AAA
//AS2DD1  DD    DSNAME=E,DISP=SHR
//AS2DD2  DD    DSNAME=G,DISP=SHR
//STEP2   EXEC  PGM=IEFBR14'
		OUTPUT - $': JOB JOB1
export JCL_AUTO_JOBNAME=JOB1
code=0
: CS1
STEP=CS1 \\
CS1DD1=X.Y.Z \\
DDIN=\'CS1DD1\' \\
DDOUT=\'\' \\
CCC
code=$?
: BS2
STEP=BS2 \\
BS2DD1=E \\
BS2DD2=G \\
DDIN=\'BS2DD1 BS2DD2\' \\
DDOUT=\'\' \\
BBB
code=$?
: AS2
STEP=AS2 \\
AS2DD1=E \\
AS2DD2=G \\
DDIN=\'AS2DD1 AS2DD2\' \\
DDOUT=\'\' \\
AAA
code=$?
: STEP2
STEP=STEP2 \\
DDIN=\'\' \\
DDOUT=\'\' \\
IEFBR14
code=$?'

	EXEC	-nvw -m-
		INPUT - $'//TEST    JOB VER=0
//SUB     PROC VER=1
//S1      EXEC  PGM=TEST
//S1D1    DD    DSN=O1.&VER..DAT
//S1D2    DD    DSN=O2.&VER..DAT
//        PEND
//MAIN1   EXEC  PROC=SUB
//S1.S1D1 DD    DSN=N1.&VER..DAT
//S1.S1D3 DD    DSN=N3.&VER..DAT'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: MAIN1 PROC SUB
: S1
STEP=S1 \\
S1D1=N1.1.DAT \\
S1D2=O2.1.DAT \\
S1D3=N3.1.DAT \\
DDIN=\'S1D1 S1D2 S1D3\' \\
DDOUT=\'\' \\
TEST
code=$?'

	EXEC	-nvw -m-
		INPUT - $'//TEST    JOB
//SUB     PROC VER=1
//S1      EXEC  PGM=TEST
//S1D1    DD    DSN=O1.&VER..DAT
//S1D2    DD    DSN=O2.&VER..DAT
//        PEND
//MAIN1   EXEC  PROC=SUB
//S1.S1D1 DD    DSN=N1.&VER..DAT
//S1.S1D3 DD    DSN=N3.&VER..DAT'

TEST 08 syntax

	EXEC	-nvw -m-
		INPUT - $'//TEST   JOB  BAD
//SET1   SET  VAL1=\'ABC,\',VAL2=DEF,NULLSYM=\'\'
//S1     EXEC PGM=IEFBR14,PARM=&VAL1
//         TIME=30
//S2     EXEC PGM=IEFBR14,PARM=&VAL2
//         TIME=60
//S3     EXEC PGM=IEFBR14,PARM=&VAL1
//         &NULLSYM'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: S1
STEP=S1 \\
DDIN=\'\' \\
DDOUT=\'\' \\
IEFBR14 ABC TIME=30
code=$?
: S2
STEP=S2 \\
DDIN=\'\' \\
DDOUT=\'\' \\
IEFBR14 DEF
code=$?'
		ERROR - $'jcl: line 6: warning: TIME: unknown OP
jcl: line 8: unexpected EOF in continuation'
		EXIT 1

	EXEC	-nvw -m-
		INPUT - $'//TEST JOB
//     SET  QUOTE=\'\'\'\'
//S1   EXEC PGM=IEFBR14,PARM=&QUOTE.ABC   DEF&QUOTE
//DD1  DD   DUMMY'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: S1
STEP=S1 \\
DD1=/dev/null \\
DDIN=\'DD1\' \\
DDOUT=\'\' \\
IEFBR14
code=$?'
		ERROR - $'jcl: line 3: unbalanced \'...\''

	EXEC	-nvw -m-
		INPUT - $'//TEST JOB  NOPARM
//    SET  CONT=\' \',T=\'(30,0)\'
//S1  EXEC PGM=IEFBR14&CONT.PARM=\'ABC  DEF\',TIME=&T'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: S1
STEP=S1 \\
DDIN=\'\' \\
DDOUT=\'\' \\
IEFBR14
code=$?'
		ERROR -
		EXIT 0

	EXEC	-nvw -m-
		INPUT - $'//TEST JOB  PARM
//    SET  CONT=\',\',T=\'(30,0)\'
//S1  EXEC PGM=IEFBR14&CONT.PARM=\'ABC  DEF\',TIME=&T'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: S1
STEP=S1 \\
DDIN=\'\' \\
DDOUT=\'\' \\
IEFBR14 \'ABC  DEF\' TIME=\'(30,0)\'
code=$?'

	EXEC	-nvw -m-
		INPUT - $'//TEST  JOB  PARM
//TPROC PROC
//STEP1 EXEC PGM=IEFBR14,PARM=&INPUT&XXX
//      PEND
//STEP2 EXEC PROC=TPROC,XXX=VALUE'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: STEP2 PROC TPROC
: STEP1
STEP=STEP1 \\
DDIN=\'\' \\
DDOUT=\'\' \\
IEFBR14 \'&INPUTVALUE\'
code=$?'
		ERROR - 'jcl: "TPROC", line 2: warning: &INPUT: undefined variable'

TEST 09 'substitution subterfuge'

	EXEC	-nvw -m-
		INPUT - $'//TEST     JOB
//TESTPROC PROC A=IMB406,B=ABLE,C=3330,D=WXYZ1,
//           E=OLD,F=TRK,G=\'10,10,1\'
//STEP     EXEC PGM=&A
//DD1      DD   DSNAME=&B,UNIT=&C,VOLUME=SER=&D,DISP=&E,
//           SPACE=(&F,(&G))
//         PEND
//CALLER1 EXEC PROC=TESTPROC,A=IEFBR14,B=BAKER,E=(NEW,KEEP)
//CALLER2 EXEC PROC=TESTPROC,A=IEFBR14,B=,C=3350,D=,E='
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: CALLER1 PROC TESTPROC
: STEP
[[ ! -d BAKER && ! -f BAKER ]] && mkdir -p BAKER
STEP=STEP \\
DD1=BAKER \\
DDIN=\'\' \\
DDOUT=\'DD1\' \\
IEFBR14
code=$?
: CALLER2 PROC TESTPROC
: STEP
STEP=STEP \\
DD1= \\
DDIN=\'DD1\' \\
DDOUT=\'\' \\
IEFBR14
code=$?'

TEST 10 'path prefix maps'

	EXEC	-r -m- -m t.map AAA.BBB.CCC.DDD02 P.D.CCC.DDD ..CCC.DDD.1 A.B.C.D
		INPUT t.map $'map\t*.*.CCC.DDD\t\tHIT-'
		OUTPUT - $'HIT-02\nHIT-\nHIT-.1\nA.B.C.D'

	EXEC	-r -m- -m t.map AAA.BBB.CCC.DDD02 P.D.CCC.DDD ..CCC.DDD.1 A.B.C.D
		INPUT t.map $'set\t%%CYCLE=123\nmap\t*.*.CCC.DDD\t\tHIT-${%%CYCLE}.${JCL_AUTO_CYCLE}'
		OUTPUT - $'HIT-123.12302\nHIT-123.123\nHIT-123.123.1\nA.B.C.D'

	EXEC	-r -m- -m t.map %%CYCLE=789 AAA.BBB.CCC.DDD02 P.D.CCC.DDD ..CCC.DDD.1 A.B.C.D
		OUTPUT - $'HIT-789.78902\nHIT-789.789\nHIT-789.789.1\nA.B.C.D'

EXPORT JCL_AUTO_CYCLE=789

	EXEC	-r -m- -m t.map AAA.BBB.CCC.DDD02 P.D.CCC.DDD ..CCC.DDD.1 A.B.C.D
		OUTPUT - $'HIT-123.12302\nHIT-123.123\nHIT-123.123.1\nA.B.C.D'

	EXEC	-i -r -m- -m t.map AAA.BBB.CCC.DDD02 P.D.CCC.DDD ..CCC.DDD.1 A.B.C.D
		OUTPUT - $'HIT-789.78902\nHIT-789.789\nHIT-789.789.1\nA.B.C.D'

	EXEC	-r -m- -m t.map AAA.BBB.CCC.DDD02 P.D.CCC.DDD ..CCC.DDD.1 A.B.C.D
		INPUT t.map $'set\t--import\nset\t%%CYCLE=123\nmap\t*.*.CCC.DDD\t\tHIT-${%%CYCLE}.${JCL_AUTO_CYCLE}'

	EXEC	-nvw -m- -m t.map
		INPUT - $'//TEST   JOB  MAP TEST
//S1     EXEC PGM=IEFBR14
//S1D1   DD   A.B.C.Z    abc.Z
//S1D2   DD   A.B.Z      ab.Z
//S1D3   DD   A.C.Z      a.C.Z
//S1D4   DD   A.Z        a.Z
//S1D5   DD   Z          Z
//S1D6   DD   A.Z        a.Z
//S1D7   DD   A.C.Z      a.C.Z
//S1D8   DD   A.B.Z      ab.Z
//S1D9   DD   A.B.C.Z    abc.Z'
		INPUT t.map $'map\tA.B.C\t\tabc\nmap\tA.B\t\tab\t%2\nmap\tA\t\ta\t%1\nmap\t""\t\tx'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: S1
STEP=S1 \\
S1D1=abc.Z \\
S1D2=ab.Z%2 \\
S1D3=a.C.Z%1 \\
S1D4=a.Z%1 \\
S1D5=xZ \\
S1D6=a.Z%1 \\
S1D7=a.C.Z%1 \\
S1D8=ab.Z%2 \\
S1D9=abc.Z \\
DDIN=\'S1D1 S1D2 S1D3 S1D4 S1D5 S1D6 S1D7 S1D8 S1D9\' \\
DDOUT=\'\' \\
IEFBR14
code=$?'

	EXEC	-nvw -m- -m t.map
		INPUT t.map $'map\tA.B.C\t\tabc\nmap\tA.B\t\tab\t%2\nmap\tA\t\ta\t%1\nmap\t*\t\t/dev/null'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: S1
STEP=S1 \\
S1D1=abc.Z \\
S1D2=ab.Z%2 \\
S1D3=a.C.Z%1 \\
S1D4=a.Z%1 \\
S1D5=/dev/null \\
S1D6=a.Z%1 \\
S1D7=a.C.Z%1 \\
S1D8=ab.Z%2 \\
S1D9=abc.Z \\
DDIN=\'S1D1 S1D2 S1D3 S1D4 S1D5 S1D6 S1D7 S1D8 S1D9\' \\
DDOUT=\'\' \\
IEFBR14
code=$?'

	EXEC	-nvw -m- -k
		INPUT - $'//TEST   JOB  TEST
//S1     EXEC PGM=IEFBR14
//S1D1   DD   DSN=A.B.C,DCB=(RECFM=FB,LRECL=123)
//S1D2   DD   DSN=A.B.Q,DISP=(NEW,CATLG,CATLG),DCB=(RECFM=FB,LRECL=6001)
//S2     EXEC PGM=IEFBR14
//S2D1   DD   DSN=A.B.C'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: S1
[[ -e A.B.Q%6001 ]] || > A.B.Q%6001
STEP=S1 \\
S1D1=A.B.C%123 \\
S1D2=A.B.Q%6001 \\
DDIN=\'S1D1\' \\
DDOUT=\'S1D2\' \\
IEFBR14
code=$?
: S2
STEP=S2 \\
S2D1=A.B.C%123 \\
DDIN=\'S2D1\' \\
DDOUT=\'\' \\
IEFBR14
code=$?'

	EXEC	-nvw -m- -k -m t.map
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: S1
[[ -e ab.Q%2 ]] || > ab.Q%2
STEP=S1 \\
S1D1=abc%123 \\
S1D2=ab.Q%2 \\
DDIN=\'S1D1\' \\
DDOUT=\'S1D2\' \\
IEFBR14
code=$?
: S2
STEP=S2 \\
S2D1=abc%123 \\
DDIN=\'S2D1\' \\
DDOUT=\'\' \\
IEFBR14
code=$?'

	EXEC	-nvw -m- -m k.map
		INPUT k.map $'set\t--marklength\nmap\tA.B.C\t\tabc\nmap\tA.B\t\tab\t%2\nmap\tA\t\ta\t%1\nmap\t""\t\tx'

	EXEC	-nvw -m- -m k.map --nomarklength
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: S1
STEP=S1 \\
S1D1=abc \\
S1D2=ab.Q%2 \\
DDIN=\'S1D1\' \\
DDOUT=\'S1D2\' \\
IEFBR14
code=$?
: S2
STEP=S2 \\
S2D1=abc \\
DDIN=\'S2D1\' \\
DDOUT=\'\' \\
IEFBR14
code=$?'

	EXEC	-nvw -m- -m q.map
		INPUT q.map $'set\t--marklength
map\tA.B.Q\t\tabq\t.qz
map\tA.B.C\t\tabc\t%3.qz
map\tA.B\t\tab\t%2
map\tA\t\ta\t%1
map\t""\t\tx'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: S1
[[ -e abq%6001.qz ]] || > abq%6001.qz
STEP=S1 \\
S1D1=abc%3.qz \\
S1D2=abq%6001.qz \\
DDIN=\'S1D1\' \\
DDOUT=\'S1D2\' \\
IEFBR14
code=$?
: S2
STEP=S2 \\
S2D1=abc%3.qz \\
DDIN=\'S2D1\' \\
DDOUT=\'\' \\
IEFBR14
code=$?'

	EXEC	-nvw -m- -m q.map
		INPUT q.map $'set\t--marklength
map\tA.B.Q\t\tabq\t.qz
map\tA.B.C\t\tabc\t%v3.qz
map\tA.B\t\tab\t%v2
map\tA\t\ta\t%v1
map\t""\t\tx'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: S1
[[ -e abq%6001.qz ]] || > abq%6001.qz
STEP=S1 \\
S1D1=abc%v3.qz \\
S1D2=abq%6001.qz \\
DDIN=\'S1D1\' \\
DDOUT=\'S1D2\' \\
IEFBR14
code=$?
: S2
STEP=S2 \\
S2D1=abc%v3.qz \\
DDIN=\'S2D1\' \\
DDOUT=\'\' \\
IEFBR14
code=$?'

	EXEC	-nvw -m- -m q.map
		INPUT q.map $'set\t--marklength
map\tA.B.Q\t\tabq\t.qz
map\tA.B.C\t\tabc.qz\t%3
map\tA.B\t\tab\t%2
map\tA\t\ta\t%1
map\t""\t\tx'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: S1
[[ -e abq%6001.qz ]] || > abq%6001.qz
STEP=S1 \\
S1D1=abc%3.qz \\
S1D2=abq%6001.qz \\
DDIN=\'S1D1\' \\
DDOUT=\'S1D2\' \\
IEFBR14
code=$?
: S2
STEP=S2 \\
S2D1=abc%3.qz \\
DDIN=\'S2D1\' \\
DDOUT=\'\' \\
IEFBR14
code=$?'

	EXEC	-nvw -m- -m q.map
		INPUT q.map $'set\t--marklength
map\tA.B.Q\t\tabq\t.qz
map\tA.B.C\t\tabc.qz\t%v3
map\tA.B\t\tab\t%v2
map\tA\t\ta\t%v1
map\t""\t\tx'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: S1
[[ -e abq%6001.qz ]] || > abq%6001.qz
STEP=S1 \\
S1D1=abc%v3.qz \\
S1D2=abq%6001.qz \\
DDIN=\'S1D1\' \\
DDOUT=\'S1D2\' \\
IEFBR14
code=$?
: S2
STEP=S2 \\
S2D1=abc%v3.qz \\
DDIN=\'S2D1\' \\
DDOUT=\'\' \\
IEFBR14
code=$?'

	EXEC	-nvw -m- -m q.map
		INPUT q.map $'set\t--marklength
suf\t.qz
map\tA.B.Q\t\tabq\t.qz
map\tA.B.C\t\tabc.qz\t%3
map\tA.B\t\tab\t%2
map\tA\t\ta\t%1
map\t""\t\tx'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: S1
[[ -e abq%6001.qz ]] || > abq%6001.qz
STEP=S1 \\
S1D1=abc%3.qz \\
S1D2=abq%6001.qz \\
DDIN=\'S1D1\' \\
DDOUT=\'S1D2\' \\
IEFBR14
code=$?
: S2
STEP=S2 \\
S2D1=abc%3.qz \\
DDIN=\'S2D1\' \\
DDOUT=\'\' \\
IEFBR14
code=$?'

	EXEC	-nvw -m- -m q.map
		INPUT q.map $'set\t--marklength
suf\t.qz
map\tA.B.Q\t\tabq\t.qz
map\tA.B.C\t\tabc.qz\t%v3
map\tA.B\t\tab\t%v2
map\tA\t\ta\t%v1
map\t""\t\tx'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: S1
[[ -e abq%6001.qz ]] || > abq%6001.qz
STEP=S1 \\
S1D1=abc%v3.qz \\
S1D2=abq%6001.qz \\
DDIN=\'S1D1\' \\
DDOUT=\'S1D2\' \\
IEFBR14
code=$?
: S2
STEP=S2 \\
S2D1=abc%v3.qz \\
DDIN=\'S2D1\' \\
DDOUT=\'\' \\
IEFBR14
code=$?'

	EXEC	-nvw -m- -k -m t.map
		INPUT - $'//TEST    JOB  TEST
//SORT1   EXEC PGM=SORT
//SYSOUT  DD SYSOUT=*
//SYSIN   DD *
 SORT FIELDS=COPY
/*
//SORTIN  DD   DSN=A.B.C,DCB=(RECFM=FB,LRECL=62)
//        DD   DSN=A.B.C.X
//        DD   DSN=A.B.C.Y
//SORTOUT DD   DSN=A.B.C.Z,DCB=(*.SORTIN)'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: SORT1
STEP=SORT1 \\
SORTIN=abc%62\'
\t\'abc.X%62\'
\t\'abc.Y%62 \\
SORTOUT=abc.Z%62 \\
DDIN=\'SORTIN SORTOUT\' \\
DDOUT=\'\' \\
SORT <<\'/*\'
 SORT FIELDS=COPY
/*
code=$?'

	EXEC	-nvw -m- -m k.map

	EXEC	-nvw -m- -k -m t.map
		INPUT - $'//TEST    JOB  TEST
//SORT1   EXEC PGM=SORT
//SYSOUT  DD SYSOUT=*
//SYSIN   DD *
 SORT FIELDS=COPY
/*
//SORTIN  DD   DUMMY,DSN=A.B.C,DCB=(RECFM=FB,LRECL=62)
//        DD   DSN=A.B.C.X
//        DD   DSN=A.B.C.Y
//SORTOUT DD   DSN=A.B.C.Z,DCB=(*.SORTIN)'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: SORT1
STEP=SORT1 \\
SORTIN=/dev/null \\
SORTOUT=abc.Z%62 \\
DDIN=\'SORTIN SORTOUT\' \\
DDOUT=\'\' \\
SORT <<\'/*\'
 SORT FIELDS=COPY
/*
code=$?'

	EXEC	-nvw -m- -m k.map

	EXEC	-nvw -m- -m m.map
		INPUT - $'//TEST   JOB  MAP TEST
//S1     EXEC PGM=IEFBR14
//S1D1   DD   DFUB1IW.TPHR212.C040609.XIVAI01.FIIP1800    abc.Z'
		INPUT m.map $'map\tDFUB1IW.TPHR212.C040609.\t\t${TESTROOT}/@data1/C040609/'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: S1
STEP=S1 \\
S1D1="${TESTROOT}/@data1/C040609/XIVAI01.FIIP1800" \\
DDIN=\'S1D1\' \\
DDOUT=\'\' \\
IEFBR14
code=$?'

	EXEC	-nvw -m- -m n.map
		INPUT n.map $'map\tDFUB1IW.TPHR212.C040609\t\t${TESTROOT}/@data1/C040609\'
//TEST   JOB  MAP TEST
//S1     EXEC PGM=IEFBR14
//S1D1   DD   DFUB1IW.TPHR212.C040609.XIVAI01.FIIP1800    abc.Z'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: S1
STEP=S1 \\
S1D1="${TESTROOT}/@data1/C040609.XIVAI01.FIIP1800" \\
DDIN=\'S1D1\' \\
DDOUT=\'\' \\
IEFBR14
code=$?'
		ERROR - $'jcl: "n.map", line 2: warning: //TEST: unknown op
jcl: "n.map", line 3: warning: //S1: unknown op
jcl: "n.map", line 4: warning: //S1D1: unknown op'

	EXEC	-nvw -m- -m p.map
		INPUT - $'//TEST    JOB  TEST
//TEST1   EXEC PGM=TEST
//TESTIN  DD   DSN=A.B.C,DCB=(RECFM=FB,LRECL=62)
//        DD   DSN=A.B.C.X,DCB=(RECFM=VB,LRECL=123)
//        DD   DSN=D.E.F.Y
//        DD   DSN=G.H.I.Z'
		INPUT p.map $'set\t--marklength
map\tA.B.C\t\tabc\t\t.qz
map\t*.E.F\t\t${1}/E/F\t%321
map\t*.*.I\t\t${2}/${1}.I
map\t""\t\tx'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: TEST1
STEP=TEST1 \\
TESTIN=abc%v123.qz\'
\t\'abc.X%123.qz\'
\t\'D/E/F.Y%321\'
\t\'H/G.I.Z%123 \\
DDIN=\'TESTIN\' \\
DDOUT=\'\' \\
TEST
code=$?'
		ERROR -

	EXEC	-nvw -m- -m v.map
		INPUT - $'//TEST    JOB  TEST
//TEST1   EXEC PGM=TEST
//TESTIN  DD   DSN=A.B.C.D.X.F
//        DD   DSN=A.B.C.D.Y.F
//        DD   DSN=A.B.C.D.E.F.G
//        DD   DSN=A.B.C.D.E.F
//        DD   DSN=A.B.C.D.E
//        DD   DSN=A.B.C.D
//        DD   DSN=A.B.C
//        DD   DSN=A.B
//        DD   DSN=A'
		INPUT v.map $'set\t--marklength
map\t*.*.*.*.X.\t/X/${1}/${2}/${3}/${4}/
map\t*.*.*.*.Y\t/Y/${1}/${2}/${3}/${4}
map\t*.*.*.*.*\t/5/${1}/${2}/${3}/${4}/${5}
map\t*.*.*.*\t\t/4/${1}/${2}/${3}/${4}
map\t*.*.*\t\t/3/${1}/${2}/${3}
map\t*.*\t\t/2/${1}/${2}
map\t*\t\t/1/${1}'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: TEST1
STEP=TEST1 \\
TESTIN=/X/A/B/C/D/F\'
\t\'/Y/A/B/C/D.F\'
\t\'/5/A/B/C/D/E.F.G\'
\t\'/5/A/B/C/D/E.F\'
\t\'/5/A/B/C/D/E\'
\t\'/4/A/B/C/D\'
\t\'/3/A/B/C\'
\t\'/2/A/B\'
\t\'/1/A \\
DDIN=\'TESTIN\' \\
DDOUT=\'\' \\
TEST
code=$?'

	EXEC	-nvw -m- -m v.map
		INPUT - $'//TEST    JOB  TEST
//TEST1   EXEC PGM=TEST
//TESTIN  DD   DSN=AAAAAAA.BBBBBBB.CCCCCCC.DDDDDDD.XXXXXXX
//        DD   DSN=AAAAAAA.BBBBBBB.CCCCCCC.DDDDDDD.YYYYYYY
//        DD   DSN=AAAAAAA.BBBBBBB.CCCCCCC.DDDDDDD.ZZZZZZZ
//        DD   DSN=AAAAAAA.BBBBBBB.RRRRRRR.FFFFFFF.SSSSSSS
//        DD   DSN=AAAAAAA.BBBBBBB.ERROR.FFFFFFF.SSSSSSS
//        DD   DSN=AAAAAAA.BBBBBBB.QQQQQQQ.DDDDDDD.EEEEEEE'
		INPUT v.map $'set\t--marklength
export\tROOT=/root
set\tDATA=${ROOT}/data
map\t*.*.*.*.XXXXXXX\t\t${DATA}/${3}/${4}.XXXXXXX\t%789.qz
map\t*.*.*.*.YYYYYYY\t\t${DATA}/${3}/${4}.YYYYYYY\t%v1234.qz
map\t*.*.QQQQQQQ.*.*\t\t${DATA}/QQQQQQQ/${4}.${3}
map\t*.*.RRRRRRR.*.SSSSSSS\t${DATA}/SSSSSSS/${3}/${2}/${1}
map\t*.*.ERROR.*\t\t${DATA}/SSSSSSS/${4}
map\t*.*.*.*.*\t\t${DATA}/${3}/${4}.${5}'
		OUTPUT - $'export ROOT=/root
: JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: TEST1
STEP=TEST1 \\
TESTIN=/root/data/CCCCCCC/DDDDDDD.XXXXXXX%789.qz\'
\t\'/root/data/CCCCCCC/DDDDDDD.YYYYYYY%v1234.qz\'
\t\'/root/data/CCCCCCC/DDDDDDD.ZZZZZZZ\'
\t\'/root/data/SSSSSSS/FFFFFFF/BBBBBBB/AAAAAAA\'
\t\'/root/data/SSSSSSS/\'
\t\'/root/data/QQQQQQQ/EEEEEEE.DDDDDDD \\
DDIN=\'TESTIN\' \\
DDOUT=\'\' \\
TEST
code=$?'
		ERROR - 'jcl: line 7: ${4}: not defined for this pattern'
		EXIT 1

	EXEC	-nvw -m- -m v.map
		INPUT v.map $'set\t--marklength
set\tROOT=/root
set\tDATA=${ROOT}/data
map\t*.*.*.*.XXXXXXX\t\t${DATA}/${3}/${4}.XXXXXXX\t%789.qz
map\t*.*.*.*.YYYYYYY\t\t${DATA}/${3}/${4}.YYYYYYY\t%v1234.qz
map\t*.*.QQQQQQQ.*.*\t\t${DATA}/QQQQQQQ/${4}.${3}
map\t*.*.RRRRRRR.*.SSSSSSS\t${DATA}/SSSSSSS/${3}/${2}/${1}
map\t*.*.ERROR.*\t\t${DATA}/SSSSSSS/${4}
map\t*.*.*.*.*\t\t${DATA}/${3}/${4}.${5}'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: TEST1
STEP=TEST1 \\
TESTIN=/root/data/CCCCCCC/DDDDDDD.XXXXXXX%789.qz\'
\t\'/root/data/CCCCCCC/DDDDDDD.YYYYYYY%v1234.qz\'
\t\'/root/data/CCCCCCC/DDDDDDD.ZZZZZZZ\'
\t\'/root/data/SSSSSSS/FFFFFFF/BBBBBBB/AAAAAAA\'
\t\'/root/data/SSSSSSS/\'
\t\'/root/data/QQQQQQQ/EEEEEEE.DDDDDDD \\
DDIN=\'TESTIN\' \\
DDOUT=\'\' \\
TEST
code=$?'

	EXEC	-r -m- -m v.map AAAAAAA.BBBBBBB.CCCCCCC.DDDDDDD.XXXXXXX AAAAAAA.BBBBBBB.CCCCCCC.DDDDDDD.YYYYYYY AAAAAAA.BBBBBBB.CCCCCCC.DDDDDDD.ZZZZZZZ AAAAAAA.BBBBBBB.RRRRRRR.FFFFFFF.SSSSSSS AAAAAAA.BBBBBBB.ERROR.FFFFFFF.SSSSSSS AAAAAAA.BBBBBBB.QQQQQQQ.DDDDDDD.EEEEEEE
		OUTPUT - $'/root/data/CCCCCCC/DDDDDDD.XXXXXXX%789.qz
/root/data/CCCCCCC/DDDDDDD.YYYYYYY%v1234.qz
/root/data/CCCCCCC/DDDDDDD.ZZZZZZZ
/root/data/SSSSSSS/FFFFFFF/BBBBBBB/AAAAAAA
/root/data/SSSSSSS/
/root/data/QQQQQQQ/EEEEEEE.DDDDDDD'
		ERROR - 'jcl: ${4}: not defined for this pattern'

	EXEC	-nvw -m- -m e.map
		INPUT -
		INPUT e.map $'set\t--marklength
map\tXXXXXXX\t\t\txxxxxxx
map\tXXXXXXX.*\t\tyyyyyyy/${1}
map\tXXXXXXX.*.*\t\tyyyyyyy/${1}/${2}
map\t*.*.*.*.XXXXXXX\t\t${3}/XXXXXXX
map\t*.*.*.*.XXXXXXX.*\t${3}/YYYYYYY/${5}
map\t*.*.*.*.XXXXXXX.*.*\t${3}/YYYYYYY/${5}/${6}
map\t*.*.XXXXXXX.*.YYYYYYY\t${3}/YYYYYYY
map\t*.*.XXXXXXX.*.ZZZZZZZ\t${3}/ZZZZZZZ'
		OUTPUT -
		ERROR - $'jcl: "e.map", line 4: XXXXXXX.*.*: duplicate map prefix
jcl: "e.map", line 7: *.*.*.*.XXXXXXX.*.*: duplicate map prefix
jcl: "e.map", line 9: *.*.XXXXXXX.*.ZZZZZZZ: duplicate map prefix'

	EXEC	-nvw -m- -m a.map
		INPUT - $'//TEST    JOB  TEST
//TEST1   EXEC PGM=TEST
//TESTIN  DD   DSN=AAAAAAA.BBBBBBB.CCCCCCC
//        DD   DSN=AAAAAAA.BBBBBBB.CCCCCCC.DDDDDDD.XXXXXXX
//        DD   DSN=AAAAAAA.BBBBBBB.CCCCCCC.DDDDDDD.YYYYYYY
//        DD   DSN=AAAAAAA.BBBBBBB.CCCCCCC.DDDDDDD.ZZZZZZZ
//        DD   DSN=AAAAAAA.BBBBBBB.RRRRRRR.FFFFFFF.SSSSSSS
//        DD   DSN=AAAAAAA.BBBBBBB.ERROR.FFFFFFF.SSSSSSS
//        DD   DSN=AAAAAAA.BBBBBBB.QQQQQQQ.DDDDDDD.EEEEEEE
//        DD   DSN=AAAAAAA.ZZZZZZZ(CCCCCCC)
//        DD   DSN=AAAAAAA.ZZZZZZZ(CCCCCCC.DDDDDDD.XXXXXXX)
//        DD   DSN=AAAAAAA.ZZZZZZZ(CCCCCCC.DDDDDDD.YYYYYYY)
//        DD   DSN=AAAAAAA.ZZZZZZZ(CCCCCCC.DDDDDDD.ZZZZZZZ)
//        DD   DSN=AAAAAAA.ZZZZZZZ(RRRRRRR.FFFFFFF.SSSSSSS)
//        DD   DSN=AAAAAAA.ZZZZZZZ(ERROR.FFFFFFF.SSSSSSS)
//        DD   DSN=AAAAAAA.ZZZZZZZ(QQQQQQQ.DDDDDDD.EEEEEEE)'
		INPUT a.map $'map\t*.BBBBBBB.*        \tdir/${1}/${2}\nmap\t*.ZZZZZZZ/*        \tpds/${1}/${2}'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: TEST1
STEP=TEST1 \\
TESTIN=dir/AAAAAAA/CCCCCCC\'
\t\'dir/AAAAAAA/CCCCCCC.DDDDDDD.XXXXXXX\'
\t\'dir/AAAAAAA/CCCCCCC.DDDDDDD.YYYYYYY\'
\t\'dir/AAAAAAA/CCCCCCC.DDDDDDD.ZZZZZZZ\'
\t\'dir/AAAAAAA/RRRRRRR.FFFFFFF.SSSSSSS\'
\t\'dir/AAAAAAA/ERROR.FFFFFFF.SSSSSSS\'
\t\'dir/AAAAAAA/QQQQQQQ.DDDDDDD.EEEEEEE\'
\t\'AAAAAAA.ZZZZZZZ/CCCCCCC\'
\t\'AAAAAAA.ZZZZZZZ/CCCCCCC.DDDDDDD.XXXXXXX\'
\t\'AAAAAAA.ZZZZZZZ/CCCCCCC.DDDDDDD.YYYYYYY\'
\t\'AAAAAAA.ZZZZZZZ/CCCCCCC.DDDDDDD.ZZZZZZZ\'
\t\'AAAAAAA.ZZZZZZZ/RRRRRRR.FFFFFFF.SSSSSSS\'
\t\'AAAAAAA.ZZZZZZZ/ERROR.FFFFFFF.SSSSSSS\'
\t\'AAAAAAA.ZZZZZZZ/QQQQQQQ.DDDDDDD.EEEEEEE \\
DDIN=\'TESTIN\' \\
DDOUT=\'\' \\
TEST
code=$?'
		ERROR -
		EXIT 0

TEST 11 'syntax round II'

	EXEC	-nvw -m- -k
		INPUT - $'//TEST    JOB  TEST\n//SORT1   EXEC PGM=SORT\n//SYSIN   DD *\n SORT FIELDS=COPY'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: SORT1
STEP=SORT1 \\
DDIN=\'\' \\
DDOUT=\'\' \\
SORT <<\'/*\'
 SORT FIELDS=COPY
/*
code=$?'

	EXEC	-nvw -m- -k
		INPUT - $'//TEST    JOB  TEST
//STEP01  EXEC PGM=HAL9000
//HAL_IN  DD DSN=A.B.C.D
//        DD DSN=A.B.C.E'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: STEP01
STEP=STEP01 \\
HAL_IN=A.B.C.D\'
\t\'A.B.C.E \\
DDIN=\'HAL_IN\' \\
DDOUT=\'\' \\
HAL9000
code=$?'

	EXEC	-nvw -m- -k
		INPUT - $'//TEST    JOB  TEST
//STEP01  EXEC PGM=HAL9000
//HAL.IN  DD DSN=A.B.C.D
//        DD DSN=A.B.C.E'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: STEP01
STEP=STEP01 \\
DDIN=\'\' \\
DDOUT=\'\' \\
HAL9000
code=$?'

TEST 12 IF/ELSE/ENDIF

	EXEC	-nvw -m- -k
		INPUT - $'//TEST  JOB   TEST
//      SET   ?RC?=2
//      IF    (RC LT 4) THEN
//      EXEC  PGM=ECHO,PARM=TRUE-1
//      ELSE
//      EXEC  PGM=ECHO,PARM=FALSE-1
//      ENDIF
//      IF    (RC < 4) THEN
//      EXEC  PGM=ECHO,PARM=TRUE-2
//      ELSE
//      EXEC  PGM=ECHO,PARM=FALSE-2
//      ENDIF
//      SET   ?RC?=16
//      IF    (RC LT 4) THEN
//      EXEC  PGM=ECHO,PARM=TRUE-3
//      ELSE
//      EXEC  PGM=ECHO,PARM=FALSE-3
//      ENDIF
//      IF    (RC < 4) THEN
//      EXEC  PGM=ECHO,PARM=TRUE-4
//      ELSE
//      EXEC  PGM=ECHO,PARM=FALSE-4
//      ENDIF
//      SET   ?RC?=2
//      IF    (RC LT 4 | RC = 16) THEN
//      EXEC  PGM=ECHO,PARM=TRUE-5
//      ELSE
//      EXEC  PGM=ECHO,PARM=FALSE-5
//      ENDIF
//      IF    (RC < 4 | RC = 16) THEN
//      EXEC  PGM=ECHO,PARM=TRUE-6
//      ELSE
//      EXEC  PGM=ECHO,PARM=FALSE-6
//      ENDIF
//      SET   ?RC?=16
//      IF    (RC LT 4 | RC = 16) THEN
//      EXEC  PGM=ECHO,PARM=TRUE-7
//      ELSE
//      EXEC  PGM=ECHO,PARM=FALSE-7
//      ENDIF
//      IF    (RC < 4 | RC = 16) THEN
//      EXEC  PGM=ECHO,PARM=TRUE-8
//      ELSE
//      EXEC  PGM=ECHO,PARM=FALSE-8
//      ENDIF
//      SET   ?RC?=2
//      IF    (RC LT 4 | RC = 16) THEN
//      EXEC  PGM=ECHO,PARM=TRUE-9
//      ELSE
//      EXEC  PGM=ECHO,PARM=FALSE-9
//      ENDIF
//      IF    ((RC LT 4 & RC LT 12) | RC = 16) THEN
//      EXEC  PGM=ECHO,PARM=TRUE-10
//      ELSE
//      EXEC  PGM=ECHO,PARM=FALSE-10
//      ENDIF
//      SET   ?RC?=16
//      IF    (RC LT 4 | RC = 16) THEN
//      EXEC  PGM=ECHO,PARM=TRUE-11
//      ELSE
//      EXEC  PGM=ECHO,PARM=FALSE-11
//      ENDIF
//      IF    ((RC LT 4 & RC LT 12) | RC = 16) THEN
//      EXEC  PGM=ECHO,PARM=TRUE-12
//      ELSE
//      EXEC  PGM=ECHO,PARM=FALSE-12
//      ENDIF'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO TRUE-1
code=$?
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO TRUE-2
code=$?
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO FALSE-3
code=$?
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO FALSE-4
code=$?
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO TRUE-5
code=$?
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO TRUE-6
code=$?
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO TRUE-7
code=$?
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO TRUE-8
code=$?
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO TRUE-9
code=$?
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO TRUE-10
code=$?
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO TRUE-11
code=$?
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO TRUE-12
code=$?'
		EXIT 16

	EXEC	-nvw -m- -k
		INPUT - $'//TEST  JOB   TEST
//      SET  ?ABEND?=1
//STEP1 EXEC PGM=TRUE
//      IF  (ABEND | STEP1.RC > 8) THEN
//      EXEC PGM=ECHO,PARM=TRUE-1
//      ENDIF
//      IF  (ABEND OR STEP1.RC GT 8) THEN
//      EXEC PGM=ECHO,PARM=TRUE-2
//      ENDIF
//      SET  ?RC?=6
//      IF  (RC > 4 & RC < 8) THEN
//      EXEC PGM=ECHO,PARM=TRUE-3
//      ENDIF
//      IF  (RC GT 4 AND RC LT 8) THEN
//      EXEC PGM=ECHO,PARM=TRUE-4
//      ENDIF'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: STEP1
STEP=STEP1 \\
DDIN=\'\' \\
DDOUT=\'\' \\
TRUE
code=$?
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO TRUE-1
code=$?
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO TRUE-2
code=$?
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO TRUE-3
code=$?
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO TRUE-4
code=$?'
		EXIT 6

	EXEC	-nvw -m- -k
		INPUT - $'//TEST  JOB   TEST
//      SET  ?ABEND?=0
//STEP1 EXEC PGM=TRUE
//      IF  (ABEND | STEP1.RC > 8) THEN
//      EXEC PGM=ECHO,PARM=TRUE-1
//      ENDIF
//      IF  (ABEND OR STEP1.RC GT 8) THEN
//      EXEC PGM=ECHO,PARM=TRUE-2
//      ENDIF
//      SET  ?RC?=2
//      IF  (RC > 4 & RC < 8) THEN
//      EXEC PGM=ECHO,PARM=TRUE-3
//      ENDIF
//      IF  (RC GT 4 AND RC LT 8) THEN
//      EXEC PGM=ECHO,PARM=TRUE-4
//      ENDIF'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: STEP1
STEP=STEP1 \\
DDIN=\'\' \\
DDOUT=\'\' \\
TRUE
code=$?'
		EXIT 2

	EXEC	-nvw -m- -k
		INPUT - $'//JOBA      JOB   TEST
//STEP1     EXEC  PGM=BEFORE
//IFBAD     IF  (ABEND | STEP1.RC > 8) THEN
//TRUE      EXEC  PGM=ERROR
//IFBADEND  ENDIF
//NEXTSTEP  EXEC  PGM=AFTER'
		OUTPUT - $': JOB JOBA
export JCL_AUTO_JOBNAME=JOBA
code=0
: STEP1
STEP=STEP1 \\
DDIN=\'\' \\
DDOUT=\'\' \\
BEFORE
code=$?
: NEXTSTEP
STEP=NEXTSTEP \\
DDIN=\'\' \\
DDOUT=\'\' \\
AFTER
code=$?'
		EXIT 0

	EXEC	-nvw -m- -k
		INPUT - $'//JOBA      JOB   TEST
//STEP1     EXEC  PGM=BEFORE
//IFBAD     IF  (!ABEND & STEP1.RC < 8) THEN
//TRUE      EXEC  PGM=OK
//IFBADEND  ENDIF
//NEXTSTEP  EXEC  PGM=AFTER'
		OUTPUT - $': JOB JOBA
export JCL_AUTO_JOBNAME=JOBA
code=0
: STEP1
STEP=STEP1 \\
DDIN=\'\' \\
DDOUT=\'\' \\
BEFORE
code=$?
: TRUE
STEP=TRUE \\
DDIN=\'\' \\
DDOUT=\'\' \\
OK
code=$?
: NEXTSTEP
STEP=NEXTSTEP \\
DDIN=\'\' \\
DDOUT=\'\' \\
AFTER
code=$?'

	EXEC	-nvw -m- -k
		INPUT - $'//JOBA      JOB   TEST
//STEP1     EXEC  PGM=BEFORE
//IFBAD     IF  (ABEND | STEP1.RC > 8) THEN
//TRUE      EXEC  PGM=ERROR
//          ELSE
//IFBADEND  ENDIF
//NEXTSTEP  EXEC  PGM=AFTER'
		OUTPUT - $': JOB JOBA
export JCL_AUTO_JOBNAME=JOBA
code=0
: STEP1
STEP=STEP1 \\
DDIN=\'\' \\
DDOUT=\'\' \\
BEFORE
code=$?
: NEXTSTEP
STEP=NEXTSTEP \\
DDIN=\'\' \\
DDOUT=\'\' \\
AFTER
code=$?'

	EXEC	-nvw -m- -k
		INPUT - $'//JOBA      JOB   TEST
//STEP1     EXEC  PGM=BEFORE
//IFBAD     IF  (!ABEND & STEP1.RC < 8) THEN
//TRUE      EXEC  PGM=OK
//          ELSE
//IFBADEND  ENDIF
//NEXTSTEP  EXEC  PGM=AFTER'
		OUTPUT - $': JOB JOBA
export JCL_AUTO_JOBNAME=JOBA
code=0
: STEP1
STEP=STEP1 \\
DDIN=\'\' \\
DDOUT=\'\' \\
BEFORE
code=$?
: TRUE
STEP=TRUE \\
DDIN=\'\' \\
DDOUT=\'\' \\
OK
code=$?
: NEXTSTEP
STEP=NEXTSTEP \\
DDIN=\'\' \\
DDOUT=\'\' \\
AFTER
code=$?'

	EXEC	-nvw -m- -k
		INPUT - $'//JOBA      JOB   TEST
//STEP1     EXEC  PGM=BEFORE
//IFBAD     IF  (ABEND | STEP1.RC > 8) THEN
//TRUE      EXEC  PGM=ERROR
//          ELSE
//YES       EXEC  PGM=TRUE
//IFBADEND  ENDIF
//NEXTSTEP  EXEC  PGM=AFTER'
		OUTPUT - $': JOB JOBA
export JCL_AUTO_JOBNAME=JOBA
code=0
: STEP1
STEP=STEP1 \\
DDIN=\'\' \\
DDOUT=\'\' \\
BEFORE
code=$?
: YES
STEP=YES \\
DDIN=\'\' \\
DDOUT=\'\' \\
TRUE
code=$?
: NEXTSTEP
STEP=NEXTSTEP \\
DDIN=\'\' \\
DDOUT=\'\' \\
AFTER
code=$?'

	EXEC	-nvw -m- -k
		INPUT - $'//JOBA      JOB   TEST
//STEP1     EXEC  PGM=BEFORE
//IFBAD     IF  (!ABEND & STEP1.RC < 8) THEN
//TRUE      EXEC  PGM=OK
//          ELSE
//          EXEC  PGM=TRUE
//IFBADEND  ENDIF
//NEXTSTEP  EXEC  PGM=AFTER'
		OUTPUT - $': JOB JOBA
export JCL_AUTO_JOBNAME=JOBA
code=0
: STEP1
STEP=STEP1 \\
DDIN=\'\' \\
DDOUT=\'\' \\
BEFORE
code=$?
: TRUE
STEP=TRUE \\
DDIN=\'\' \\
DDOUT=\'\' \\
OK
code=$?
: NEXTSTEP
STEP=NEXTSTEP \\
DDIN=\'\' \\
DDOUT=\'\' \\
AFTER
code=$?'

	EXEC	-nvw -m- -k RC=3
		INPUT - $'//TEST JOB   TEST
//     SET ?RC?=&RC
//     EXEC  PGM=ECHO,PARM=1
//      IF  (RC > 5) THEN
//       IF  (RC > 7) THEN
//        EXEC  PGM=ECHO,PARM=2
//       ELSE
//        EXEC  PGM=ECHO,PARM=3
//       ENDIF
//      ELSE
//       IF  (RC > 3) THEN
//        EXEC  PGM=ECHO,PARM=4
//       ELSE
//        EXEC  PGM=ECHO,PARM=5
//       ENDIF
//      ENDIF
//     EXEC  PGM=ECHO,PARM=(E,O,F)'
		OUTPUT - $'export RC=3
: JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO 1
code=$?
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO 5
code=$?
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO E,O,F
code=$?'
		EXIT 3

	EXEC	-nvw -m- -k RC=5
		OUTPUT - $'export RC=5
: JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO 1
code=$?
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO 4
code=$?
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO E,O,F
code=$?'
		EXIT 5

	EXEC	-nvw -m- -k RC=7
		OUTPUT - $'export RC=7
: JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO 1
code=$?
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO 3
code=$?
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO E,O,F
code=$?'
		EXIT 7

	EXEC	-nvw -m- -k RC=9
		OUTPUT - $'export RC=9
: JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO 1
code=$?
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO 2
code=$?
: 
STEP= \\
DDIN=\'\' \\
DDOUT=\'\' \\
ECHO E,O,F
code=$?'
		EXIT 9

TEST 13 INCLUDE

	EXEC	-nvw -m-
		INPUT - $'//TEST     JOB   TEST
//LIBSRCH  JCLLIB  ORDER=TEST.SYSOUT.JCL
//STEP1    EXEC    PGM=OUTRTN
//OUTPUT1  INCLUDE MEMBER=include.jcl
//STEP2    EXEC    PGM=IEFBR14'
		INPUT include.jcl '//SYSOUT2  DD      DSN=A'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: STEP1
STEP=STEP1 \\
SYSOUT2=A \\
DDIN=\'SYSOUT2\' \\
DDOUT=\'\' \\
OUTRTN
code=$?
: STEP2
STEP=STEP2 \\
DDIN=\'\' \\
DDOUT=\'\' \\
IEFBR14
code=$?'

	EXEC	-nvw -m-
		INPUT - $'//TEST     JOB   TEST
//LIBSRCH  JCLLIB  ORDER=(TEST.SYSOUT.JCL,MY.JCL.NOT.YOURS)
//STEP1    EXEC    PGM=OUTRTN
//OUTPUT1  INCLUDE MEMBER=include.jcl
//STEP2    EXEC    PGM=IEFBR14'
		INPUT include.jcl '//SYSOUT2  DD      DSN=A'

TEST 14 DD

	EXEC	-nvw -m-
		INPUT - $'//SPACY     PROC SYM1=\'What\'\'\'\'s up, Doc?\',SYM2=(DEF),SYM3=&&&&TEMP1,
//       SYM4=\'&&TEMP2\',SYM5=TEMP3,
//       SYM6=TEMP4
//S1        EXEC PGM=WTO,PARM=\'&SYM1\',ACCT=&SYM2
//DD1       DD   DSN=&SYM3,UNIT=SYSDA,
//               SPACE=(TRK,(1,,1),2),DISP=(NEW,KEEP)
//DD2       DD   DSN=&SYM4,UNIT=SYSDA,
//               SPACE=(TRK,(1,,1),2),DISP=(OLD,DELETE)
//DD3       DD   DSN=&SYM5,UNIT=SYSDA,
//               SPACE=(TRK,(1,,1),2),DISP=MOD
//DD4       DD   DSN=&SYM6,UNIT=SYSDA,
//               SPACE=(TRK,(1,,1),2),DISP=NEW
//          PEND'
		OUTPUT - $': JOB SPACY
export JCL_AUTO_JOBNAME=SPACY
code=0
trap \'code=$?; rm -rf ${TMPDIR:-/tmp}/job.SPACY.$$.*; exit $code\' 0 1 2
: S1
[[ ! -d ${TMPDIR:-/tmp}/job.SPACY.$$.TEMP1 && ! -f ${TMPDIR:-/tmp}/job.SPACY'\
$'.$$.TEMP1 ]] && mkdir -p ${TMPDIR:-/tmp}/job.SPACY.$$.TEMP1
[[ ! -d TEMP3 && ! -f TEMP3 ]] && mkdir -p TEMP3
[[ ! -d TEMP4 && ! -f TEMP4 ]] && mkdir -p TEMP4
STEP=S1 \\
DD1=${TMPDIR:-/tmp}/job.SPACY.$$.TEMP1 \\
DD2=${TMPDIR:-/tmp}/job.SPACY.$$.TEMP2 \\
DD3=TEMP3 \\
DD4=TEMP4 \\
DDIN=\'DD2\' \\
DDOUT=\'DD1 DD3 DD4\' \\
WTO $\'What\\\'s up, Doc?\' ACCT=\'(DEF)\'
code=$?
if (( ! $code ))
then
\trm -rf ${TMPDIR:-/tmp}/job.SPACY.$$.TEMP2
fi'

TEST 15 OUTPUT

	EXEC	-nvw -m-
		INPUT - $'//TESTOUT PROC    OUTTEST
//OUT1    OUTPUT  DEST=STLNODE.WMSMITH
//OUT2    OUTPUT  CONTROL=DOUBLE
//TEST    EXEC    PGM=TEST
//DS      DD      SYSOUT=C,OUTPUT=(*.OUT1,*.OUT2)'
		OUTPUT - $': JOB TESTOUT
export JCL_AUTO_JOBNAME=TESTOUT
code=0
: TEST
STEP=TEST \\
DDIN=\'\' \\
DDOUT=\'\' \\
TEST
code=$?'

	EXEC	-nsvw -m-
		OUTPUT - $': JOB TESTOUT
export JCL_AUTO_JOBNAME=TESTOUT
code=0
n=0
t=$(date +%y-%m-%d)
while\t:
do\td=TESTOUT.$t.$((++n))
\t[[ -d $d ]] || break
done
mkdir $d && cd $d || exit 1
exec > SYSOUT 2> SYSERR
TIMEFORMAT=\'USAGE CPU=%P%% REAL=%R USR=%U SYS=%S\'
time {
date +\'STARTED AT %K\'
: TEST
STEP=TEST \\
DDIN=\'\' \\
DDOUT=\'\' \\
TEST
code=$?
date +\'COMPLETED AT %K\'
}'

TEST 16 'delayed references'

	EXEC	-nvw -m-
		INPUT - $'//JOB1    JOB   TEST
//STEPA   EXEC  PGM=TEST
//DD1     DD    DSNAME=REPORT
//DD2     DD    DSN=TABLE
//DD3     DD    DSNAME=*.DD1
//DD4     DD    DDNAME=DD1'
		OUTPUT - $': JOB JOB1
export JCL_AUTO_JOBNAME=JOB1
code=0
: STEPA
STEP=STEPA \\
DD1=REPORT \\
DD2=TABLE \\
DD3=REPORT \\
DD4=REPORT \\
DDIN=\'DD1 DD2 DD3 DD4\' \\
DDOUT=\'\' \\
TEST
code=$?'

	EXEC	-nvw -m-
		INPUT - $'//JOB1     JOB  TEST
//STEP1    EXEC PGM=IEBGENER
//SYSPRINT DD   SYSOUT=*
//SYSUT1   DD   DDNAME=INPUT
//INPUT    DD   DSN=TSTDATA1,DISP=SHR
//         DD   DSN=TSTDATA2,DISP=SHR
//SYSUT2   DD   SYSOUT=*
//SYSIN    DD   DUMMY'
		OUTPUT - $': JOB JOB1
export JCL_AUTO_JOBNAME=JOB1
code=0
: STEP1
STEP=STEP1 \\
INPUT=TSTDATA1\'
\t\'TSTDATA2 \\
SYSUT1=TSTDATA1\'
\t\'TSTDATA2 \\
DDIN=\'INPUT SYSUT1\' \\
DDOUT=\'\' \\
IEBGENER < /dev/null
code=$?'

	EXEC	-nvw -m-
		INPUT - $'//JOB1     JOB  TEST
//TPROC    PROC
//S1       EXEC PGM=PROG1
//DD1      DD   DDNAME=INPUT
//DD2      DD   DSN=MYDSN2,DISP=SHR
//DD3      DD   DSN=MYDSN3,DISP=SHR
//S2       EXEC PGM=PROG2
//DDA      DD   DDNAME=INPUT
//DDB      DD   DSN=MINE2,DISP=SHR
//DDC      DD   DSN=MINE3,DISP=SHR
//         PEND
//STEP1    EXEC TPROC
//INPUT    DD   DSN=MYDSN1,DISP=SHR
//         DD   DSN=MYDSN4,DISP=SHR
//S2.INPUT DD   DSN=MINE1,DISP=SHR
//         DD   DSN=MINE4,DISP=SHR'
		OUTPUT - $': JOB JOB1
export JCL_AUTO_JOBNAME=JOB1
code=0
: STEP1 PROC TPROC
: S1
STEP=S1 \\
DD1=MYDSN1\'
\t\'MYDSN4 \\
DD2=MYDSN2 \\
DD3=MYDSN3 \\
DDIN=\'DD1 DD2 DD3\' \\
DDOUT=\'\' \\
PROG1
code=$?
: S2
STEP=S2 \\
DDA=MINE1\'
\t\'MINE4 \\
DDB=MINE2 \\
DDC=MINE3 \\
INPUT=MINE1\'
\t\'MINE4 \\
DDIN=\'DDA DDB DDC INPUT\' \\
DDOUT=\'\' \\
PROG2
code=$?'

	EXEC	-nvw -m-
		INPUT - $'//JOB1     JOB  TEST
//TPROC    PROC
//S1       EXEC PGM=PROG1
//DD1      DD   DDNAME=INPUT
//DD2      DD   DSN=MYDSN2,DISP=SHR
//DD3      DD   DSN=MYDSN3,DISP=SHR
//S2       EXEC PGM=PROG2
//DDA      DD   SUBSYS=(BLSR,\'DDNAME=INPUT\')
//DDB      DD   DSN=MINE2,DISP=SHR
//DDC      DD   DSN=MINE3,DISP=SHR
//         PEND
//STEP1    EXEC TPROC
//INPUT    DD   DSN=MYDSN1,DISP=SHR
//         DD   DSN=MYDSN4,DISP=SHR
//S2.INPUT DD   DSN=MINE1,DISP=SHR
//         DD   DSN=MINE4,DISP=SHR'

TEST 17 'control-m auto edit expansion'

	EXEC	-d4 -n -m- -O 1998-06-03
		INPUT - $'//EJ%%ODATE JOB (0,15)\n//          EXEC PGM=ACCOUNTS,DAY=%%ODAY,MONTH=%%OMONTH'
		ERROR - $'jcl: debug-4: record     1  //EJ980603 JOB (0,15)
jcl: debug-4: record     2  // EXEC PGM=ACCOUNTS,DAY=03,MONTH=06
jcl: debug-2: set       DAY=03
jcl: debug-2: set       MONTH=06'

	EXEC	-d4 -n -m-
		INPUT - $'//J1       JOB TEST
//*  %%SET %%A  = 1
//*  %%SET %%B  = 2
//*  %%SET %%A2 = 100
//P1       EXEC PGM=P1,A=%%A,B=%%B,AB=%%A%%B,A.B=%%A.%%B'
		ERROR - $'jcl: debug-4: record     1  //J1 JOB TEST
jcl: debug-2: set       JCL_AUTO_A=1
jcl: debug-2: set       JCL_AUTO_B=2
jcl: debug-2: set       JCL_AUTO_A2=100
jcl: debug-4: record     5  //P1 EXEC PGM=P1,A=1,B=2,AB=100,A.B=12
jcl: debug-2: set       A=1
jcl: debug-2: set       B=2
jcl: debug-2: set       AB=100
jcl: debug-2: set       A.B=12'

	EXEC	-d4 -n -m- -O 2000-09-24
		INPUT - $'//J1       JOB    TEST
//S1       EXEC   PGM=T1,$OYEAR=%%$OYEAR,OYEAR=%%OYEAR,
//                OMONTH=%%OMONTH,ODAY=%%ODAY,OWDAY=%%OWDAY
//* %%SET  %%BACKUP_UNIT = TAPE
//F1       DD %%BACKUP_UNIT
//* %%SET  %%BACKUP_UNIT_%%OWDAY = EE%%OMONTH.%%ODAY
//F2       DD %%BACKUP_UNIT_%%OWDAY
//F3       DD %%BACKUP_UNIT_7'
		ERROR - $'jcl: debug-4: record     1  //J1 JOB TEST
jcl: debug-4: record     2  //S1 EXEC PGM=T1,$OYEAR=2000,OYEAR=00,OMONTH=09,'\
$'ODAY=24,OWDAY=7
jcl: debug-2: set       $OYEAR=2000
jcl: debug-2: set       OYEAR=00
jcl: debug-2: set       OMONTH=09
jcl: debug-2: set       ODAY=24
jcl: debug-2: set       OWDAY=7
jcl: debug-2: set       JCL_AUTO_BACKUP_UNIT=TAPE
jcl: debug-4: record     5  //F1 DD TAPE
jcl: debug-2: set       JCL_AUTO_BACKUP_UNIT_7=EE0924
jcl: debug-4: record     7  //F2 DD EE0924
jcl: debug-4: record     8  //F3 DD EE0924'

TEST 18 'parsalyze this'

	EXEC	-nvw -m-
		INPUT - $'//TEST    JOB (0,15)
//TEST01  EXEC PGM=TEST02,PARM=(\'(12345,\'\' \'\',\'\'TEST03\'\',FOO BAR)\''\
$')
//TEST04  EXEC PGM=TEST05,PARM=(\'FOO = \'\'BAR\'\';\')'
		OUTPUT - $': JOB TEST
export JCL_AUTO_JOBNAME=TEST
code=0
: TEST01
STEP=TEST01 \\
DDIN=\'\' \\
DDOUT=\'\' \\
TEST02 $\'(12345,\\\' \\\',\\\'TEST03\\\',FOO BAR)\'
code=$?
: TEST04
STEP=TEST04 \\
DDIN=\'\' \\
DDOUT=\'\' \\
TEST05 $\'FOO = \\\'BAR\\\';\'
code=$?'
