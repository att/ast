# : : generated from jcm.rt by mktest : : #

# regression tests for the jcm command

UNIT jcm

TEST 01 basics

	EXEC	-c t.jcm
		INPUT -n -
		INPUT -n t.jcm 'DDC1000  05 M        YYYYYYYYYYYY                                           '\
'    W           W        YYYYYYYYYYYY                                       '\
'        MI~PXWAITDC-UB-UBWMBMRG      J2599999999                9999        '\
'            LMEMLIB.BASE                                                    '\
' 305            HWM: DUMMY JOB TO WAIT FOR ~DD-NAME FRAG SORTS              '\
'                    VDUMMY                                       DC-UB      '\
'                        T%%RUNMODE=I                                        '\
'                            T%%STREAM=*                                     '\
'                                T%%JOBPREFX=~PX                             '\
'                                    T%%INSTNC=~INSTNC8                      '\
'                                        T%%DDNAME=~DD-NAME                  '\
'                                            T%%CTN=~@                       '\
'                                                T%%CYCL=~#                  '\
'                                                    ZMERGE   DOCLIB.BASE    '\
'                                                V300    IDCJ-I~PS~FIN-OK-~@~'\
'#ODAT                                                       QDCQ-WLB        '\
'     0001XAQ-CTM-OK          0001                               ODCJ-I~PXWAI'\
'T-OK-~@~#ODAT+                                                      BYY10015'\
'Y                                                                       R   '\
'                                                          03                '\
'C,,,,ANYSTEP          001C0000 K%%%RUNMODE=R                                '\
'    C        ,,,,ANYSTEP          001EXERR %%%RUNMODE=R                     '\
'        C                                                                   '\
'            SNU-ECS           R45%%APPL %%JOBNAME NOTOK %%JOBCC %%ODATE %%TI'\
'ME              '
		OUTPUT - $'=== t.jcm ===
DDC1000  05 M        YYYYYYYYYYYY
W           W        YYYYYYYYYYYY
MI~PXWAITDC-UB-UBWMBMRG      J2599999999                9999
LMEMLIB.BASE                                                     305
HWM: DUMMY JOB TO WAIT FOR ~DD-NAME FRAG SORTS
VDUMMY                                       DC-UB
T%%RUNMODE=I
T%%STREAM=*
T%%JOBPREFX=~PX
T%%INSTNC=~INSTNC8
T%%DDNAME=~DD-NAME
T%%CTN=~@
T%%CYCL=~#
ZMERGE   DOCLIB.BASE                                                    V300
IDCJ-I~PS~FIN-OK-~@~#ODAT
QDCQ-WLB             0001XAQ-CTM-OK          0001
ODCJ-I~PXWAIT-OK-~@~#ODAT+
BYY10015Y
R                                                             03
C,,,,ANYSTEP          001C0000 K%%%RUNMODE=R        ,,,,ANYSTEP          001'\
$'EXERR %%%RUNMODE=R
SNU-ECS           R45%%APPL %%JOBNAME NOTOK %%JOBCC %%ODATE %%TIME'
		ERROR -n -

	EXEC	t.jcm
		OUTPUT - $':JCL:

JCL_AUTO_RUNMODE == I
JCL_AUTO_STREAM == *
JCL_AUTO_JOBPREFX == $(JCL_AUTO_PX)
JCL_AUTO_INSTNC == $(JCL_AUTO_INSTNC8)
JCL_AUTO_DDNAME == $(JCL_AUTO_DD-NAME)
JCL_AUTO_CTN == $(JCL_AUTO_at)
JCL_AUTO_CYCL == $(JCL_AUTO_pound)

all : .VIRTUAL t
t : .VIRTUAL JOB-I$(JCL_AUTO_PX)WAIT
JOB-I$(JCL_AUTO_PX)WAIT : .VIRTUAL .FOREGROUND .DO.NOTHING DCJ-I$(JCL_AUTO_P'\
$'S)$(JCL_AUTO_FIN)-OK-$(JCL_AUTO_at)$(JCL_AUTO_pound) DCJ-I$(JCL_AUTO_PX)WAI'\
$'T-OK-$(JCL_AUTO_at)$(JCL_AUTO_pound).RAISE
DCJ-I$(JCL_AUTO_PX)WAIT-OK-$(JCL_AUTO_at)$(JCL_AUTO_pound).RAISE : .AFTER .E'\
$'VENT.RAISE
DCJ-I$(JCL_AUTO_PS)$(JCL_AUTO_FIN)-OK-$(JCL_AUTO_at)$(JCL_AUTO_pound) : .VIR'\
'TUAL .EVENT.WAIT'

TEST 02 '--initialize=file'

	EXEC	-h u.jcm
		INPUT -n -
		INPUT u.ini $'JCL_AUTO_PX = 98\nJCL_AUTO_EVEN = 2\nJCL_AUTO_ODD = 3'
		INPUT -n u.jcm 'DDC1000  05 M        YYYYYYYYYYYY                                           '\
'    W           W        YYYYYYYYYYYY                                       '\
'        MI~PXWAITDC-UB-UBWMBMRG      J2599999999                9999        '\
'            LMEMLIB.BASE                                                    '\
' 305            HWM: DUMMY JOB TO WAIT FOR ~DD-NAME FRAG SORTS              '\
'                    VDUMMY                                       DC-UB      '\
'                        T%%RUNMODE=I                                        '\
'                            T%%STREAM=*                                     '\
'                                T%%INSTNC=~INSTNC8                          '\
'                                    T%%DDNAME=~DD-NAME                      '\
'                                        T%%CTN=~@                           '\
'                                            T%%CYCL=~#                      '\
'                                                T%%EVEN=~EVEN               '\
'                                                    T%%ODD=~ODD             '\
'                                                        ZMERGE   DOCLIB.BASE'\
'                                                    V300    IDCJ-I~PS~FIN-OK'\
'-~@~#ODAT                                                       QDCQ-WLB    '\
'         0001XAQ-CTM-OK          0001                               ODCJ-I~P'\
'XWAIT-OK-~@~#ODAT+                                                      BYY1'\
'0015Y                                                                       '\
'R                                                             03            '\
'    C,,,,ANYSTEP          001C0000 K%%%RUNMODE=R                            '\
'        C        ,,,,ANYSTEP          001EXERR %%%RUNMODE=R                 '\
'            C                                                               '\
'                SNU-ECS           R45%%APPL %%JOBNAME NOTOK %%JOBCC %%ODATE '\
'%%TIME              '
		OUTPUT - $':JCL:

# WM: DUMMY JOB TO WAIT FOR $(JCL_AUTO_DD-NAME) FRAG SORTS

JCL_AUTO_RUNMODE == I
JCL_AUTO_STREAM == *
JCL_AUTO_INSTNC == $(JCL_AUTO_INSTNC8)
JCL_AUTO_DDNAME == $(JCL_AUTO_DD-NAME)
JCL_AUTO_CTN == $(JCL_AUTO_at)
JCL_AUTO_CYCL == $(JCL_AUTO_pound)
JCL_AUTO_EVEN == 1
JCL_AUTO_ODD == 1

all : .VIRTUAL u
u : .VIRTUAL JOB-I$(JCL_AUTO_PXWAIT)
JOB-I$(JCL_AUTO_PXWAIT) : .VIRTUAL .FOREGROUND .DO.NOTHING DCJ-I$(JCL_AUTO_P'\
$'S)$(JCL_AUTO_FIN)-OK-$(JCL_AUTO_at)$(JCL_AUTO_pound) DCJ-I$(JCL_AUTO_PXWAIT'\
$')-OK-$(JCL_AUTO_at)$(JCL_AUTO_pound).RAISE
DCJ-I$(JCL_AUTO_PXWAIT)-OK-$(JCL_AUTO_at)$(JCL_AUTO_pound).RAISE : .AFTER .E'\
$'VENT.RAISE
DCJ-I$(JCL_AUTO_PS)$(JCL_AUTO_FIN)-OK-$(JCL_AUTO_at)$(JCL_AUTO_pound) : .VIR'\
'TUAL .EVENT.WAIT'
		ERROR -n -

	EXEC	-i u.ini u.jcm
		OUTPUT - $':JCL:

JCL_AUTO_EVEN == 2
JCL_AUTO_ODD == 3
JCL_AUTO_PX == 98

JCL_AUTO_RUNMODE == I
JCL_AUTO_STREAM == *
JCL_AUTO_INSTNC == $(JCL_AUTO_INSTNC8)
JCL_AUTO_DDNAME == $(JCL_AUTO_DD-NAME)
JCL_AUTO_CTN == $(JCL_AUTO_at)
JCL_AUTO_CYCL == $(JCL_AUTO_pound)

all : .VIRTUAL u
u : .VIRTUAL JOB-I$(JCL_AUTO_PX)WAIT
JOB-I$(JCL_AUTO_PX)WAIT : .VIRTUAL .FOREGROUND .DO.NOTHING DCJ-I$(JCL_AUTO_P'\
$'S)$(JCL_AUTO_FIN)-OK-$(JCL_AUTO_at)$(JCL_AUTO_pound) DCJ-I$(JCL_AUTO_PX)WAI'\
$'T-OK-$(JCL_AUTO_at)$(JCL_AUTO_pound).RAISE
DCJ-I$(JCL_AUTO_PX)WAIT-OK-$(JCL_AUTO_at)$(JCL_AUTO_pound).RAISE : .AFTER .E'\
$'VENT.RAISE
DCJ-I$(JCL_AUTO_PS)$(JCL_AUTO_FIN)-OK-$(JCL_AUTO_at)$(JCL_AUTO_pound) : .VIR'\
'TUAL .EVENT.WAIT'

	EXEC	-i u.ini -I 3 u.jcm
		OUTPUT - $':JCL:

JCL_AUTO_EVEN == 2
JCL_AUTO_ODD == 3
JCL_AUTO_PX == 98

JCL_AUTO_RUNMODE == I
JCL_AUTO_STREAM == *
JCL_AUTO_INSTNC == $(JCL_AUTO_INSTNC8)
JCL_AUTO_DDNAME == $(JCL_AUTO_DD-NAME)
JCL_AUTO_CTN == $(JCL_AUTO_at)
JCL_AUTO_CYCL == $(JCL_AUTO_pound)

all : .VIRTUAL u
u : .VIRTUAL JOB-I$(JCL_AUTO_PX)WAIT
JOB-I$(JCL_AUTO_PX)WAIT : .VIRTUAL .FOREGROUND .DO.NOTHING DCJ-I$(JCL_AUTO_P'\
$'S)$(JCL_AUTO_FIN)-OK-$(JCL_AUTO_at)$(JCL_AUTO_pound) DCJ-I$(JCL_AUTO_PS)$(J'\
$'CL_AUTO_FIN)-OK-$(JCL_AUTO_at)$(JCL_AUTO_pound)-01 DCJ-I$(JCL_AUTO_PS)$(JCL'\
$'_AUTO_FIN)-OK-$(JCL_AUTO_at)$(JCL_AUTO_pound)-02 DCJ-I$(JCL_AUTO_PX)WAIT-OK'\
$'-$(JCL_AUTO_at)$(JCL_AUTO_pound).RAISE
DCJ-I$(JCL_AUTO_PX)WAIT-OK-$(JCL_AUTO_at)$(JCL_AUTO_pound).RAISE : .AFTER .E'\
$'VENT.RAISE
DCJ-I$(JCL_AUTO_PS)$(JCL_AUTO_FIN)-OK-$(JCL_AUTO_at)$(JCL_AUTO_pound) : .VIR'\
$'TUAL .EVENT.WAIT
DCJ-I$(JCL_AUTO_PS)$(JCL_AUTO_FIN)-OK-$(JCL_AUTO_at)$(JCL_AUTO_pound)-01 : .'\
$'VIRTUAL .EVENT.WAIT
DCJ-I$(JCL_AUTO_PS)$(JCL_AUTO_FIN)-OK-$(JCL_AUTO_at)$(JCL_AUTO_pound)-02 : .'\
'VIRTUAL .EVENT.WAIT'
