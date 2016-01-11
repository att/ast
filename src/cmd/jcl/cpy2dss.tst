# : : generated from cpy2dss.rt by mktest : : #

# tests for the cpy2dss command

UNIT cpy2dss

TEST 01 basics

	EXEC	 --regress
		INPUT - $'       01 TEST.
         10 TEST-FIELD-01 PIC X(8).
         10 TEST-FIELD-02 PIC X(8).
         05 TEST-FIELD-03 PIC S9(3)V     COMP-3.
         05 TEST-FIELD-04 PIC S9(8)V     COMP-3.
         05 TEST-FIELD-05 PIC S9(3)V     COMP-3.
         10 TEST-FIELD-06 PIC X(10).
         10 TEST-FIELD-07 PIC X(10).
         10 TEST-FIELD-08 PIC X(1).
         10 TEST-FIELD-09 PIC X(8).
         10 TEST-FIELD-10 PIC X(5).
         10 TEST-FIELD-11 PIC X(1).
         10 TEST-FIELD-12 PIC X(1).
         10 TEST-FIELD-13 PIC X(1).
         10 TEST-FIELD-14 PIC X(8).
         10 TEST-FIELD-15 PIC X(1).
         10 TEST-FIELD-16 PIC X(8).
         10 TEST-FIELD-17 PIC S9(2)V9(13) COMP-3.
         10 TEST-FIELD-18 PIC X(8).
         10 TEST-FIELD-19 PIC X(8).
         10 TEST-FIELD-20 PIC X(1).
         10 TEST-FIELD-21 PIC X(1).
         10 TEST-FIELD-22 PIC X(10).
         10 TEST-FIELD-23 PIC X(3).
         10 TEST-FIELD-24 PIC X(1).
         10 TEST-FIELD-25 PIC X(3).
         10 TEST-FIELD-26 PIC X(26).
         10 TEST-FIELD-27.
           49 TEST-FIELD-27-01 PIC S9(4) COMP-5.
           49 TEST-FIELD-27-02 PIC X(55).'
		OUTPUT - $'<METHOD>flat</>
<FLAT>
 <NAME>TEST</>
 <DESCRIPTION>converted by cpy2dss from copybook standard input</>
 <IDENT>@(#)$Id: TEST (AT&T Research) 2005-07-17 $</>
 <LIBRARY>num_t</>
 <FIELD>
  <NAME>TEST_FIELD_01</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_02</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_03</>
  <TYPE>number</>
  <PHYSICAL>
   <TYPE>bcd_t</>
   <WIDTH>2</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_04</>
  <TYPE>number</>
  <PHYSICAL>
   <TYPE>bcd_t</>
   <WIDTH>5</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_05</>
  <TYPE>number</>
  <PHYSICAL>
   <TYPE>bcd_t</>
   <WIDTH>2</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_06</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>10</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_07</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>10</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_08</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_09</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_10</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>5</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_11</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_12</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_13</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_14</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_15</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_16</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_17</>
  <TYPE>number</>
  <PHYSICAL>
   <TYPE>bcd_t</>
   <FIXEDPOINT>13</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_18</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_19</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_20</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_21</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_22</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>10</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_23</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>3</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_24</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_25</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>3</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_26</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>26</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_27</>
  <FIELD>
   <NAME>TEST_FIELD_27_01</>
   <TYPE>number</>
   <PHYSICAL>
    <TYPE>le_t</>
    <WIDTH>2</>
   </>
  </>
  <FIELD>
   <NAME>TEST_FIELD_27_02</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>55</>
   </>
  </>
 </>
</>'

	EXEC	 --regress --comp=5:1
		OUTPUT - $'<METHOD>flat</>
<FLAT>
 <NAME>TEST</>
 <DESCRIPTION>converted by cpy2dss from copybook standard input</>
 <IDENT>@(#)$Id: TEST (AT&T Research) 2005-07-17 $</>
 <LIBRARY>num_t</>
 <FIELD>
  <NAME>TEST_FIELD_01</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_02</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_03</>
  <TYPE>number</>
  <PHYSICAL>
   <TYPE>bcd_t</>
   <WIDTH>2</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_04</>
  <TYPE>number</>
  <PHYSICAL>
   <TYPE>bcd_t</>
   <WIDTH>5</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_05</>
  <TYPE>number</>
  <PHYSICAL>
   <TYPE>bcd_t</>
   <WIDTH>2</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_06</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>10</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_07</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>10</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_08</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_09</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_10</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>5</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_11</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_12</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_13</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_14</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_15</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_16</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_17</>
  <TYPE>number</>
  <PHYSICAL>
   <TYPE>bcd_t</>
   <FIXEDPOINT>13</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_18</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_19</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_20</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_21</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_22</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>10</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_23</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>3</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_24</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_25</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>3</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_26</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>26</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_27</>
  <FIELD>
   <NAME>TEST_FIELD_27_01</>
   <TYPE>number</>
   <PHYSICAL>
    <TYPE>be_t</>
    <WIDTH>2</>
   </>
  </>
  <FIELD>
   <NAME>TEST_FIELD_27_02</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>55</>
   </>
  </>
 </>
</>'

	EXEC	 --regress --variable '--terminator=&#21;'
		OUTPUT - $'<METHOD>flat</>
<FLAT>
 <NAME>TEST</>
 <DESCRIPTION>converted by cpy2dss from copybook standard input</>
 <IDENT>@(#)$Id: TEST (AT&T Research) 2005-07-17 $</>
 <LIBRARY>num_t</>
 <FIELD>
  <NAME>TEST_FIELD_01</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_02</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_03</>
  <TYPE>number</>
  <PHYSICAL>
   <TYPE>bcd_t</>
   <WIDTH>2</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_04</>
  <TYPE>number</>
  <PHYSICAL>
   <TYPE>bcd_t</>
   <WIDTH>5</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_05</>
  <TYPE>number</>
  <PHYSICAL>
   <TYPE>bcd_t</>
   <WIDTH>2</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_06</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>10</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_07</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>10</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_08</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_09</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_10</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>5</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_11</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_12</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_13</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_14</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_15</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_16</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_17</>
  <TYPE>number</>
  <PHYSICAL>
   <TYPE>bcd_t</>
   <FIXEDPOINT>13</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_18</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_19</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_20</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_21</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_22</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>10</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_23</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>3</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_24</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_25</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>3</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_26</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>26</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_27</>
  <FIELD>
   <NAME>TEST_FIELD_27_01</>
   <TYPE>number</>
   <PHYSICAL>
    <TYPE>le_t</>
    <WIDTH>2</>
   </>
  </>
  <FIELD>
   <NAME>TEST_FIELD_27_02</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>55</>
    <DELIMITER>&#21;</>
   </>
  </>
 </>
</>'

	EXEC	 --regress --variable
		OUTPUT - $'<METHOD>flat</>
<FLAT>
 <NAME>TEST</>
 <DESCRIPTION>converted by cpy2dss from copybook standard input</>
 <IDENT>@(#)$Id: TEST (AT&T Research) 2005-07-17 $</>
 <LIBRARY>num_t</>
 <FIELD>
  <NAME>TEST_FIELD_01</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_02</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_03</>
  <TYPE>number</>
  <PHYSICAL>
   <TYPE>bcd_t</>
   <WIDTH>2</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_04</>
  <TYPE>number</>
  <PHYSICAL>
   <TYPE>bcd_t</>
   <WIDTH>5</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_05</>
  <TYPE>number</>
  <PHYSICAL>
   <TYPE>bcd_t</>
   <WIDTH>2</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_06</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>10</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_07</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>10</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_08</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_09</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_10</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>5</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_11</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_12</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_13</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_14</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_15</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_16</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_17</>
  <TYPE>number</>
  <PHYSICAL>
   <TYPE>bcd_t</>
   <FIXEDPOINT>13</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_18</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_19</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>8</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_20</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_21</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_22</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>10</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_23</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>3</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_24</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_25</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>3</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_26</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>26</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_27</>
  <FIELD>
   <NAME>TEST_FIELD_27_01</>
   <TYPE>number</>
   <PHYSICAL>
    <TYPE>le_t</>
    <WIDTH>2</>
   </>
  </>
  <FIELD>
   <NAME>TEST_FIELD_27_02</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>55</>
    <DELIMITER>&newline;</>
   </>
  </>
 </>
</>'

	EXEC	 --regress --text --reclen=123
		OUTPUT - $'<METHOD>flat</>
<FLAT>
 <NAME>TEST</>
 <DESCRIPTION>converted by cpy2dss from copybook standard input</>
 <IDENT>@(#)$Id: TEST (AT&T Research) 2005-07-17 $</>
 <LIBRARY>num_t</>
 <RECORD>
  <FIXED>123</>
 </>
 <PHYSICAL>
  <QUOTE>"</>
  <QUOTEALL>1</>
 </>
 <FIELD>
  <NAME>TEST_FIELD_01</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_02</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_03</>
  <TYPE>number</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_04</>
  <TYPE>number</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_05</>
  <TYPE>number</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_06</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_07</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_08</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_09</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_10</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_11</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_12</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_13</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_14</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_15</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_16</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_17</>
  <TYPE>number</>
  <PHYSICAL>
   <CODESET>ascii</>
   <FIXEDPOINT>13</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_18</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_19</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_20</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_21</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_22</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_23</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_24</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_25</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_26</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_27</>
  <FIELD>
   <NAME>TEST_FIELD_27_01</>
   <TYPE>number</>
   <PHYSICAL>
    <CODESET>ascii</>
    <DELIMITER>|</>
   </>
  </>
  <FIELD>
   <NAME>TEST_FIELD_27_02</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ascii</>
    <DELIMITER>&newline;</>
   </>
  </>
 </>
</>'

	EXEC	 --regress --bytemask
		OUTPUT - 000000000000000033333333300000000000000000000000000000000000000000000000000000\
033333333000000000000000000000000000000000000000000000000000000000000055000000\
0000000000000000000000000000000000000000000000000

	EXEC	 --regress --offsets
		OUTPUT - $'TEST_FIELD_01\t0\t8\t0\tstring
TEST_FIELD_02\t8\t8\t0\tstring
TEST_FIELD_03\t16\t2\t0\tbcd_t
TEST_FIELD_04\t18\t5\t0\tbcd_t
TEST_FIELD_05\t23\t2\t0\tbcd_t
TEST_FIELD_06\t25\t10\t0\tstring
TEST_FIELD_07\t35\t10\t0\tstring
TEST_FIELD_08\t45\t1\t0\tstring
TEST_FIELD_09\t46\t8\t0\tstring
TEST_FIELD_10\t54\t5\t0\tstring
TEST_FIELD_11\t59\t1\t0\tstring
TEST_FIELD_12\t60\t1\t0\tstring
TEST_FIELD_13\t61\t1\t0\tstring
TEST_FIELD_14\t62\t8\t0\tstring
TEST_FIELD_15\t70\t1\t0\tstring
TEST_FIELD_16\t71\t8\t0\tstring
TEST_FIELD_17\t79\t8\t0\tbcd_t
TEST_FIELD_18\t87\t8\t0\tstring
TEST_FIELD_19\t95\t8\t0\tstring
TEST_FIELD_20\t103\t1\t0\tstring
TEST_FIELD_21\t104\t1\t0\tstring
TEST_FIELD_22\t105\t10\t0\tstring
TEST_FIELD_23\t115\t3\t0\tstring
TEST_FIELD_24\t118\t1\t0\tstring
TEST_FIELD_25\t119\t3\t0\tstring
TEST_FIELD_26\t122\t26\t0\tstring
TEST_FIELD_27\t148\t59\t0\tstruct
TEST_FIELD_27_01\t148\t2\t0\tle_t
TEST_FIELD_27_02\t150\t55\t0\tstring
.\t205\t0\t0\tstruct'

TEST 02 'embedded variable length'

	EXEC	 --regress
		INPUT - $'       01 TEST.
          10 TEST-FIELD-01 PIC X(4).
          10 TEST-FIELD-02.
            49 TEST-FIELD-02-SIZE PIC S9(4) COMP-5.
            49 TEST-FIELD-02-DATA PIC X(32700).
          10 TEST-FIELD-03 PIC X(4).'
		OUTPUT - $'<METHOD>flat</>
<FLAT>
 <NAME>TEST</>
 <DESCRIPTION>converted by cpy2dss from copybook standard input</>
 <IDENT>@(#)$Id: TEST (AT&T Research) 2005-07-17 $</>
 <LIBRARY>num_t</>
 <FIELD>
  <NAME>TEST_FIELD_01</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>4</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_02</>
  <FIELD>
   <NAME>TEST_FIELD_02_SIZE</>
   <TYPE>number</>
   <PHYSICAL>
    <TYPE>le_t</>
    <WIDTH>2</>
   </>
  </>
  <FIELD>
   <NAME>TEST_FIELD_02_DATA</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>32700</>
    <WIDTH>TEST_FIELD_02_SIZE</>
   </>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_03</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>4</>
  </>
 </>
</>'

	EXEC	 --regress --reclen=256
		OUTPUT - $'<METHOD>flat</>
<FLAT>
 <NAME>TEST</>
 <DESCRIPTION>converted by cpy2dss from copybook standard input</>
 <IDENT>@(#)$Id: TEST (AT&T Research) 2005-07-17 $</>
 <LIBRARY>num_t</>
 <RECORD>
  <FIXED>256</>
 </>
 <FIELD>
  <NAME>TEST_FIELD_01</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>4</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_02</>
  <FIELD>
   <NAME>TEST_FIELD_02_SIZE</>
   <TYPE>number</>
   <PHYSICAL>
    <TYPE>le_t</>
    <WIDTH>2</>
   </>
  </>
  <FIELD>
   <NAME>TEST_FIELD_02_DATA</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>246</>
    <WIDTH>TEST_FIELD_02_SIZE</>
   </>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_03</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>4</>
  </>
 </>
</>'
		ERROR - 'cpy2dss: line 6: warning: TEST_FIELD_02_DATA: maximum variable field size sh'\
'ortened from 32700 to 246 to comply with fixed record size 256'

	EXEC	 --regress --reclen=256 --text
		OUTPUT - $'<METHOD>flat</>
<FLAT>
 <NAME>TEST</>
 <DESCRIPTION>converted by cpy2dss from copybook standard input</>
 <IDENT>@(#)$Id: TEST (AT&T Research) 2005-07-17 $</>
 <LIBRARY>num_t</>
 <RECORD>
  <FIXED>256</>
 </>
 <PHYSICAL>
  <QUOTE>"</>
  <QUOTEALL>1</>
 </>
 <FIELD>
  <NAME>TEST_FIELD_01</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_02</>
  <FIELD>
   <NAME>TEST_FIELD_02_SIZE</>
   <TYPE>number</>
   <PHYSICAL>
    <CODESET>ascii</>
    <DELIMITER>|</>
   </>
  </>
  <FIELD>
   <NAME>TEST_FIELD_02_DATA</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ascii</>
    <WIDTH>TEST_FIELD_02_SIZE</>
    <DELIMITER>|</>
   </>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_03</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>&newline;</>
  </>
 </>
</>'
		ERROR -

	EXEC	 --regress --text
		OUTPUT - $'<METHOD>flat</>
<FLAT>
 <NAME>TEST</>
 <DESCRIPTION>converted by cpy2dss from copybook standard input</>
 <IDENT>@(#)$Id: TEST (AT&T Research) 2005-07-17 $</>
 <LIBRARY>num_t</>
 <PHYSICAL>
  <QUOTE>"</>
  <QUOTEALL>1</>
 </>
 <FIELD>
  <NAME>TEST_FIELD_01</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_02</>
  <FIELD>
   <NAME>TEST_FIELD_02_SIZE</>
   <TYPE>number</>
   <PHYSICAL>
    <CODESET>ascii</>
    <DELIMITER>|</>
   </>
  </>
  <FIELD>
   <NAME>TEST_FIELD_02_DATA</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ascii</>
    <WIDTH>TEST_FIELD_02_SIZE</>
    <DELIMITER>|</>
   </>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_03</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>&newline;</>
  </>
 </>
</>'

	EXEC	 --regress --reclen=256 --text --variable
		OUTPUT - $'<METHOD>flat</>
<FLAT>
 <NAME>TEST</>
 <DESCRIPTION>converted by cpy2dss from copybook standard input</>
 <IDENT>@(#)$Id: TEST (AT&T Research) 2005-07-17 $</>
 <LIBRARY>num_t</>
 <RECORD>
  <FIXED>256</>
 </>
 <PHYSICAL>
  <QUOTE>"</>
  <QUOTEALL>1</>
 </>
 <FIELD>
  <NAME>TEST_FIELD_01</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_02</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_03</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>&newline;</>
  </>
 </>
</>'

	EXEC	 --regress --text --variable
		OUTPUT - $'<METHOD>flat</>
<FLAT>
 <NAME>TEST</>
 <DESCRIPTION>converted by cpy2dss from copybook standard input</>
 <IDENT>@(#)$Id: TEST (AT&T Research) 2005-07-17 $</>
 <LIBRARY>num_t</>
 <PHYSICAL>
  <QUOTE>"</>
  <QUOTEALL>1</>
 </>
 <FIELD>
  <NAME>TEST_FIELD_01</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_02</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>|</>
  </>
 </>
 <FIELD>
  <NAME>TEST_FIELD_03</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ascii</>
   <DELIMITER>&newline;</>
  </>
 </>
</>'

TEST 03 unions

	EXEC	 --regress
		INPUT - $'       01  TEST-B29-B23-B55.
           03  TEST-B74-B55-B36            PIC S9(4)     COMP
                                                       VALUE ZEROES.
           03  TEST-B23-C069-B33.
               06  TEST-C068-B23-C069-C002.
                   09  TEST-D0018-B37-B08.
                       12  TEST-C007-A4    PIC X(10)   VALUE SPACES.
                       12  TEST-C037-A5    PIC S9(13)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-C037-B14-A12-B28.
                       12  TEST-C036-B07-C003
                                           PIC S9(13)     COMP-3
                                                       VALUE ZEROES.
                       12  TEST-B07-C003-C076
                                           PIC X(2)    VALUE SPACES.
                       12  TEST-B14-A1     PIC X(2)    VALUE SPACES.
                   09  TEST-B37-B23-B55-C076
                                           PIC X       VALUE SPACES.
                   09  TEST-A13-B59-C069-B42
                                           PIC S9(3)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-B56-C003-A5    PIC X(38)   VALUE SPACES.
                   09  TEST-B12-A13-B59-C040
                                           PIC S9(3)     COMP
                                                       VALUE ZEROES.
                   09  TEST-D0023-B23-B55-C076
                                           PIC X       VALUE SPACES.
                   09  TEST-C065-B28-B58-A1
                                           PIC S9(5)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-B61-B01-C037-B28.
                       12  TEST-B27-B61-B01-C037
                                           PIC X(13)   VALUE SPACES.
                       12  TEST-B57-B61-B01-C037
                                           PIC X(13)   VALUE SPACES.
                       12  TEST-C078-C055-B26
                                           PIC X       VALUE SPACES.
                   09  TEST-B23-C058-C069-B28.
                       12  TEST-C058-D0024-C069-B28.
                       15  TEST-C037-D0024-B33
                                           PIC X(80)   VALUE SPACES.
                       12  TEST-C058-C069-B28.
                       15  TEST-C036-C069-B28
                           OCCURS 5 TIMES
                           INDEXED BY  TEST-C059-C069-C040.
                       18  FILLER          PIC X(50) VALUE D0031.
                   09  TEST-C065-A7-B28-B58-A1
                                           PIC S9(5)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-B23-D0028-C069-B28.
                       12  TEST-D0030-C069-B28.
                       15  TEST-C036-C069-B28
                           OCCURS 5 TIMES
                           INDEXED BY  TEST-D0028-C069-C040.
                       18  FILLER          PIC X(50) VALUE D0031.
               06  TEST-B23-B59-B58-A3     PIC S9(3)     COMP-3
                                                       VALUE ZEROES.
           03  TEST-B23-B12-C002-B28.
               06  TEST-B31-B12-C002.
                   09  TEST-B06-A5.
                       12  TEST-C042-B06-A5.
                       15  TEST-C028-E00006-A1
                                           PIC X(8)    VALUE SPACES.
                       15  TEST-B55-B58-B42
                                           PIC 9(8)    VALUE ZEROES.
                       15  TEST-B29-B58-B42
                                           PIC S9(3)     COMP-3
                                                       VALUE ZEROES.
                       12  TEST-B06-B58-B42
                                           PIC S9(4)     COMP-3
                                                       VALUE ZEROES.
                       12  TEST-C071-B06-A5.
                       15  TEST-C067-A4    PIC X(10)   VALUE SPACES.
                       15  TEST-C067-B58-B42
                                           PIC S9(13)     COMP-3
                                                       VALUE ZEROES.
                       15  TEST-B60-B64    PIC X(3)    VALUE SPACES.
                   09  TEST-B06-C076       PIC X(3)    VALUE SPACES.
                   09  TEST-D0026-C076     PIC X(2)    VALUE SPACES.
                   09  TEST-C006-C075-A1   PIC S9(8)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-A2-B32         PIC X       VALUE SPACES.
                   09  TEST-A6-B55-C076-A1 PIC X(2)    VALUE \'D\'.
                   09  TEST-B39-A16-C065-B32
                                           PIC X       VALUE SPACES.
                   09  TEST-D0005-A10-C066  PIC X(6)    VALUE SPACES.
                   09  TEST-C007-B13-B11-B32
                                           PIC X       VALUE SPACES.
                   09  TEST-B06-C018-C041-E00003.
                       12  TEST-C006-C066-A5.
                       15  TEST-B42-B28.
                       18  TEST-C052-C066-C076-A1
                                           PIC X       VALUE SPACES.
                       18  TEST-C036-C066-B42-B28.
                       21  TEST-C036-C066-B42
                                           PIC X(35)   VALUE SPACES.
                       18  TEST-B41-C066-B42-B28
                           REDEFINES TEST-C036-C066-B42-B28.
                       21  TEST-B43        PIC X(3).
                       21  TEST-B50        PIC X(3).
                       21  TEST-C046-B42   PIC X(4).
                       21  FILLER          PIC X(25).
                       18  TEST-D0017-C066-B42-B28
                           REDEFINES TEST-C036-C066-B42-B28.
                       21  TEST-D0017-B42-B36
                                           PIC X(2).
                       21  TEST-D0017-B42-B28.
                       24  TEST-D0017-B42-B52
                                           PIC X(10).
                       24  TEST-D0017-B42-B53
                                           PIC X(6).
                       21  FILLER          PIC X(17).
                       18  TEST-B43-B50-B42-B28
                           REDEFINES TEST-C036-C066-B42-B28.
                       21  TEST-C054-B43-B50
                                           PIC X(6).
                       21  FILLER          PIC X(29).
                       15  TEST-C006-C066-B60-B42-B28
                           REDEFINES TEST-B42-B28.
                       18  TEST-C006-C052-C066-B70
                                           PIC X.
                       18  TEST-C006-C066-B42-B28.
                       21  TEST-C006-B43   PIC X(3).
                       21  TEST-C006-B50   PIC X(3).
                       21  TEST-C006-C046-B42
                                           PIC X(4).
                       21  FILLER          PIC X(25).
                       18  TEST-C006-B41-B42-B28
                           REDEFINES TEST-C006-C066-B42-B28.
                       21  TEST-C006-B42-B41
                                           PIC X(10).
                       21  FILLER          PIC X(25).
                       18  TEST-C006-D0017-C066-B42-B28
                           REDEFINES TEST-C006-C066-B42-B28.
                       21  TEST-D0017-B42-B36
                                           PIC X(2).
                       21  TEST-C006-D0017-B42-B28.
                       24  FILLER          PIC X(16).
                       21  FILLER          PIC X(17).
                       18  TEST-C006-D0022-B42-B28
                           REDEFINES TEST-C006-C066-B42-B28.
                       21  TEST-C006-C066-D0004-A5
                                           PIC X(30).
                       21  TEST-D0004-C048-A1
                                           PIC X(4).
                       12  TEST-C071-C037-A12-A5
                                           PIC S9(13)     COMP-3
                                                       VALUE ZEROES.
                       12  TEST-C037-B14-A12-A5
                                           PIC S9(13)     COMP-3
                                                       VALUE ZEROES.
                       12  TEST-C066-C003-A5
                                           PIC S9(13)     COMP-3
                                                       VALUE ZEROES.
                       12  TEST-C037-A12-A5 PIC S9(13)     COMP-3
                                                       VALUE ZEROES.
                       12  TEST-D0027-C003 PIC X(4)    VALUE SPACES.
                       12  TEST-B63-C001-C039
                                           PIC 9(13)   VALUE ZEROES.
                       12  TEST-B34-A1     PIC X(2)    VALUE SPACES.
                       12  TEST-D0020-C073-A1
                                           PIC X(2)
                           OCCURS 2 TIMES
                           VALUE SPACES.
                   09  TEST-C018-C003-A5   PIC S9(8)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-B06-C060-D0033.
                       12  TEST-C060-C053-A1
                                           PIC X(8)    VALUE SPACES.
                       12  TEST-C060-COMPNT-A1
                                           PIC X(8)    VALUE SPACES.
                       12  TEST-C043-C060-C041-B66.
                       15  TEST-C043-C060-C041-B28
                           OCCURS 5 TIMES.
                       18  TEST-C043-C060-C076-A1
                                           PIC X       VALUE SPACES.
                       18  TEST-C043-C060-B35-A1
                                           PIC X(4)    VALUE SPACES.
                       12  TEST-C060-C006-A5
                                           PIC X(8)    VALUE SPACES.
                       12  TEST-C006-C026-A1
                                           PIC X(8)    VALUE SPACES.
                       12  TEST-C006-C026-B09-A1
                                           PIC X       VALUE SPACES.
                       12  TEST-B48-E00006 PIC S9(3)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-C062-D0033-B75-B42
                                           PIC S9(3)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-C041-C051-B28.
                       12  TEST-C036-D0013 PIC S9(17)     COMP-3
                           OCCURS 6 TIMES
                           INDEXED BY  TEST-D0013-B30
                           VALUE ZEROES.
                       12  TEST-C036-B54   PIC S9(15)V9(2)  COMP-3
                           OCCURS 5 TIMES
                           INDEXED BY  TEST-B54-B30
                           VALUE ZEROES.
                       12  TEST-C036-B03   PIC S9(8)V9(7)  COMP-3
                           OCCURS 7 TIMES
                           INDEXED BY  TEST-B03-B30
                           VALUE ZEROES.
                   09  TEST-B31-C020-B28.
                       12  TEST-D0015-A16-A4
                                           PIC X(10)   VALUE SPACES.
                       12  TEST-B06-B62-A4 PIC X(10)   VALUE SPACES.
                       12  TEST-B06-B21-A4 PIC X(10)   VALUE SPACES.
                   09  TEST-B71-A1         PIC X(5)    VALUE SPACES.
                   09  TEST-B12-E00002-C019.
                       12  TEST-E00009-A14-D0007-A1
                                           PIC X(2)    VALUE SPACES.
                       12  TEST-D0034-A14-D0007-A1
                                           PIC X(2)    VALUE SPACES.
                       12  TEST-E00009-C045
                                           PIC X(3)    VALUE SPACES.
                       12  TEST-D0034-C045 PIC X(3)    VALUE SPACES.
                       12  TEST-B08-A5     PIC X(4)    VALUE SPACES.
                   09  TEST-C061-B32       PIC X       VALUE SPACES.
                   09  TEST-E00007-D0005-A10-C066
                                           PIC X(5)    VALUE SPACES.
                   09  TEST-E00008-C011    PIC S9(6)V9(2)  COMP-3
                                                       VALUE ZEROES.
                   09  TEST-C044-D0019-A1  PIC X       VALUE SPACES.
                   09  TEST-D0012-C076     PIC X       VALUE SPACES.
                   09  TEST-D0012-C027-B32 PIC X       VALUE SPACES.
                   09  TEST-C010-D0013-B54-C002.
                       12  TEST-C009-D0013-C064
                                           PIC S9(10)V9(5)  COMP-3
                                                       VALUE ZEROES.
                       12  TEST-F000003-D0013-C064
                                           PIC S9(10)V9(5)  COMP-3
                                                       VALUE ZEROES.
                       12  TEST-D0014-B54  PIC S9(7)V9(3)  COMP-3
                                                       VALUE ZEROES.
                       12  TEST-C038-C009-C005-B54
                                           PIC S9(7)V9(2)  COMP-3
                                                       VALUE ZEROES.
                   09  TEST-E00004-B03     PIC S9(8)V9(7)  COMP-3
                                                       VALUE ZEROES.
                   09  TEST-C072-C011-B32  PIC X       VALUE SPACES.
                   09  TEST-E00005-C056-B28.
                       12  TEST-E00005-C024
                                           PIC S9(4)     COMP-3
                                                       VALUE ZEROES.
                       12  TEST-C056-B42   PIC X(9)    VALUE SPACES.
                   09  TEST-B31-B65-B28.
                       12  TEST-A15-B65-B09-A1
                                           PIC X(2)    VALUE SPACES.
                       12  TEST-D0016-E00012.
                       15  TEST-B65-D0001-B32
                                           PIC X       VALUE SPACES.
                       15  TEST-B65-D0019-A1-B28.
                       18  TEST-B41-A1     PIC X(2)    VALUE SPACES.
                       18  TEST-D0032-A1   PIC X(2)    VALUE SPACES.
                       18  TEST-C015-A1    PIC X(3)    VALUE SPACES.
                       18  TEST-C012-A1    PIC X(4)    VALUE SPACES.
                       18  TEST-F000005-A1 PIC X(2)    VALUE SPACES.
                       15  TEST-B44-B32    PIC X       VALUE SPACES.
                       15  TEST-B65-B22    PIC X(5)    VALUE SPACES.
                       15  TEST-C076-A10-C001
                                           PIC X(2)    VALUE SPACES.
                       15  TEST-COMPLNC-C075-A1
                                           PIC X       VALUE SPACES.
                       15  TEST-C066-B28   PIC X       VALUE SPACES.
                       15  TEST-B04-F000001-B07
                                           PIC X(2)    VALUE SPACES.
                       15  TEST-C033-A10-C011
                                           PIC X       VALUE SPACES.
                       15  TEST-C011-C076-A1
                                           PIC X(2)    VALUE SPACES.
                       15  TEST-F000006-B65-C008
                                           PIC X(2)    VALUE SPACES.
                       15  TEST-C047-B40-B74
                                           PIC 9(7)    VALUE ZEROES.
                       15  TEST-C047-C066-D0006-A5
                                           PIC 9(8)    VALUE ZEROES.
                       15  TEST-B65-B42-B02-D0021
                                           PIC 9(7)    VALUE ZEROES.
                       15  TEST-B65-E00002 PIC X(2)    VALUE SPACES.
                       15  TEST-B65-C021   PIC X       VALUE SPACES.
                       15  TEST-B24-B65-C030
                                           PIC X       VALUE SPACES.
                       15  TEST-C074-B65-A1
                                           PIC X       VALUE SPACES.
                       12  TEST-D0016-F000004.
                       15  TEST-C060-C026-A1
                                           PIC X(10)   VALUE SPACES.
                       15  TEST-D0035-C063-A1
                                           PIC X(4)    VALUE SPACES.
                       15  TEST-B65-D0002-B28.
                       18  TEST-F000002-B65-D0002
                                           PIC X       VALUE SPACES.
                       18  TEST-D0032-B65-D0002
                                           PIC X       VALUE SPACES.
                       18  TEST-C012-B65-D0002
                                           PIC X       VALUE SPACES.
                       18  TEST-E00001-B65-D0002
                                           PIC X       VALUE SPACES.
                       18  TEST-F000005-B65-D0002
                                           PIC X       VALUE SPACES.
                       15  TEST-C060-B73-A1
                                           PIC X(2)    VALUE SPACES.
                       15  TEST-C060-C016-B03
                                           PIC S9(11)V9(4)  COMP-3
                                                       VALUE ZEROES.
                       12  TEST-C075-C076  PIC X(4)    VALUE SPACES.
                       12  TEST-C063-C076  PIC X(4)    VALUE SPACES.
                       12  TEST-B68-B65-B03
                                           PIC S9(11)V9(7)  COMP-3
                                                       VALUE ZEROES.
                       12  TEST-B42-A10-OCCURS
                                           PIC S9(4)     COMP
                                                       VALUE 0.
               06  TEST-B05-B12-C002.
                   09  TEST-C037-D0024-B33 PIC X(80)   VALUE SPACES.
                   09  TEST-C060-D0033-C023.
                       12  TEST-C060-C031-C023
                                           PIC X(30)   VALUE SPACES.
                       12  TEST-C060-C076-C023
                                           PIC X(30)   VALUE SPACES.
                       12  TEST-C011-B28   PIC X(30)   VALUE SPACES.
                       12  TEST-B02-C076-C023
                                           PIC X(30)   VALUE SPACES.
                       12  TEST-C013-A10-D0003-C023
                                           PIC X(30)   VALUE SPACES.
                   09  TEST-C035           PIC S9(10)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-D0018-D0009-B61-A1
                                           PIC S9(3)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-C066-B42-B35   PIC X(30)   VALUE SPACES.
                   09  TEST-D0018-D0009-C023
                                           PIC X(30)   VALUE SPACES.
                   09  TEST-C066-C003-A5   PIC S9(13)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-D0012-B49-B45  PIC S9(2)V9(13)  COMP-3
                                                       VALUE ZEROES.
                   09  TEST-C001-B28-C011-B32
                                           PIC X       VALUE SPACES.
                   09  TEST-B12-C037-A12-C023
                                           PIC X(30)   VALUE SPACES.
                   09  TEST-B12-B10-A5     PIC X(6)    VALUE SPACES.
                   09  TEST-B73-C011-A1    PIC X(2)    VALUE \'D\'.
               06  TEST-B23-B12-C002.
                   09  TEST-D0025-C037-B38-B42
                                           PIC S9(2)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-E00010-C037-A12-A5
                                           PIC S9(13)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-C037-A12-B58-B42
                                           PIC S9(6)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-C034-C065-B28-A1
                                           PIC X(6)    VALUE SPACES.
                   09  TEST-C034-D0029-B28-A1
                                           PIC X(5)    VALUE SPACES.
                   09  TEST-C065-B28-B09-A1
                                           PIC X       VALUE SPACES.
                   09  TEST-D0010-A5       PIC X(8)    VALUE SPACES.
                   09  TEST-C065-A11-C025-A1
                                           PIC X       VALUE SPACES.
                   09  TEST-A13-B72-B23-B32 PIC X
                       OCCURS 2 TIMES
                       VALUE SPACES.
                   09  TEST-C022-C060-B35-B28.
                       12  TEST-C043-C060-B35-B58
                                           PIC S9(2)     COMP-3
                           OCCURS 5 TIMES
                           VALUE ZEROES.
                       12  TEST-C043-C060-B35-B15
                                           PIC X(40)
                           OCCURS 5 TIMES
                           VALUE SPACES.
                       12  TEST-C043-C060-B35-B16
                                           PIC X(30)
                           OCCURS 5 TIMES
                           VALUE SPACES.
                       12  TEST-C043-C060-B35-B17
                                           PIC X(25)
                           OCCURS 5 TIMES
                           VALUE SPACES.
                       12  TEST-C043-C060-B35-B18
                                           PIC X(25)
                           OCCURS 5 TIMES
                           VALUE SPACES.
                       12  TEST-C043-C060-B35-B19
                                           PIC X(20)
                           OCCURS 5 TIMES
                           VALUE SPACES.
                   09  TEST-C022-C006-C075-B28.
                       12  TEST-C006-C075-B58
                                           PIC S9(8)     COMP-3
                                                       VALUE ZEROES.
                       12  TEST-C006-C075-C023-B46
                                           PIC X(40)   VALUE SPACES.
                       12  TEST-C006-C075-C023-B69
                                           PIC X(30)   VALUE SPACES.
                       12  TEST-C006-C075-C023-D0036
                                           PIC X(25)   VALUE SPACES.
                       12  TEST-C006-C075-C023-C032
                                           PIC X(25)   VALUE SPACES.
                       12  TEST-C006-C075-C023-C029
                                           PIC X(20)   VALUE SPACES.
                   09  TEST-B67-C023       PIC X(45)   VALUE SPACES.
                   09  TEST-C062-B51-A1-C023
                                           PIC X(30)   VALUE SPACES.
                   09  TEST-A14-D0007-C023  PIC X(25)   VALUE SPACES.
                   09  TEST-D0020-C073     PIC X
                       OCCURS 2 TIMES
                       VALUE SPACES.
                   09  TEST-C009-B20-B29-A1
                                           PIC X       VALUE SPACES.
                   09  TEST-B25-A1         PIC X(7)    VALUE SPACES.
                   09  TEST-B71-A9         PIC X(15)   VALUE SPACES.
                   09  TEST-D0020-C023-B28
                       OCCURS 2 TIMES.
                       12  TEST-D0020-C023-B46
                                           PIC X(55)   VALUE SPACES.
                       12  TEST-D0020-C023-B69
                                           PIC X(110)  VALUE SPACES.
                   09  TEST-D0008-C009-B02-B42
                                           PIC X(23)   VALUE SPACES.
                   09  TEST-D0008-C009-C023
                                           PIC X(25)   VALUE SPACES.
           03  TEST-B29-B23-C017-B28.
               06  TEST-B29-B23-C017-C002.
                   09  TEST-C037-A5        PIC S9(13)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-C037-B14-A12-B28.
                       12  TEST-C036-B07-C003
                                           PIC S9(13)     COMP-3
                                                       VALUE ZEROES.
                       12  TEST-B07-C003-C076
                                           PIC X(2)    VALUE SPACES.
                       12  TEST-B14-A1     PIC X(2)    VALUE SPACES.
                   09  TEST-D0026-C076     PIC X(2)    VALUE SPACES.
                   09  TEST-C006-C050      PIC X       VALUE SPACES.
                   09  TEST-C049-C018-B28-A1
                                           PIC X(4)    VALUE SPACES.
                   09  TEST-C007-D0011-A1  PIC S9(2)     COMP-3
                                                       VALUE 0.
                   09  TEST-B63-D0011-A5   PIC S9(4)     COMP
                                                       VALUE ZEROES.
                   09  TEST-C006-E00011    PIC S9(1)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-B14-C070-A1    PIC X       VALUE SPACES.
                   09  TEST-C004-A1        PIC X       VALUE SPACES.
                   09  TEST-C057-B47-A8    PIC X(21)   VALUE SPACES.
                   09  TEST-C043-C018-B42  PIC X(16)   VALUE SPACES.
                   09  TEST-C014-C001-B42  PIC X(8)    VALUE SPACES.'
		OUTPUT - $'<METHOD>flat</>
<FLAT>
 <NAME>TEST_B29_B23_B55</>
 <DESCRIPTION>converted by cpy2dss from copybook standard input</>
 <IDENT>@(#)$Id: TEST_B29_B23_B55 (AT&T Research) 2005-07-17 $</>
 <LIBRARY>num_t</>
 <FIELD>
  <NAME>TEST_B74_B55_B36</>
  <TYPE>number</>
  <PHYSICAL>
   <TYPE>be_t</>
   <WIDTH>2</>
  </>
 </>
 <FIELD>
  <NAME>TEST_B23_C069_B33</>
  <FIELD>
   <NAME>TEST_C068_B23_C069_C002</>
   <FIELD>
    <NAME>TEST_D0018_B37_B08</>
    <FIELD>
     <NAME>TEST_C007_A4</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>10</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C037_A5</>
     <TYPE>number</>
     <PHYSICAL>
      <TYPE>bcd_t</>
      <WIDTH>7</>
     </>
    </>
   </>
   <FIELD>
    <NAME>TEST_C037_B14_A12_B28</>
    <FIELD>
     <NAME>TEST_C036_B07_C003</>
     <TYPE>number</>
     <PHYSICAL>
      <TYPE>bcd_t</>
      <WIDTH>7</>
     </>
    </>
    <FIELD>
     <NAME>TEST_B07_C003_C076</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>2</>
     </>
    </>
    <FIELD>
     <NAME>TEST_B14_A1</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>2</>
     </>
    </>
   </>
   <FIELD>
    <NAME>TEST_B37_B23_B55_C076</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>1</>
    </>
   </>
   <FIELD>
    <NAME>TEST_A13_B59_C069_B42</>
    <TYPE>number</>
    <PHYSICAL>
     <TYPE>bcd_t</>
     <WIDTH>2</>
    </>
   </>
   <FIELD>
    <NAME>TEST_B56_C003_A5</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>38</>
    </>
   </>
   <FIELD>
    <NAME>TEST_B12_A13_B59_C040</>
    <TYPE>number</>
    <PHYSICAL>
     <TYPE>be_t</>
     <WIDTH>2</>
    </>
   </>
   <FIELD>
    <NAME>TEST_D0023_B23_B55_C076</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>1</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C065_B28_B58_A1</>
    <TYPE>number</>
    <PHYSICAL>
     <TYPE>bcd_t</>
     <WIDTH>3</>
    </>
   </>
   <FIELD>
    <NAME>TEST_B61_B01_C037_B28</>
    <FIELD>
     <NAME>TEST_B27_B61_B01_C037</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>13</>
     </>
    </>
    <FIELD>
     <NAME>TEST_B57_B61_B01_C037</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>13</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C078_C055_B26</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>1</>
     </>
    </>
   </>
   <FIELD>
    <NAME>TEST_B23_C058_C069_B28</>
    <FIELD>
     <NAME>TEST_C058_D0024_C069_B28</>
     <FIELD>
      <NAME>TEST_C037_D0024_B33</>
      <TYPE>string</>
      <PHYSICAL>
       <CODESET>ebcdic-m</>
       <WIDTH>80</>
      </>
     </>
    </>
    <FIELD>
     <NAME>TEST_C058_C069_B28</>
     <FIELD>
      <NAME>TEST_C036_C069_B28</>
      <ARRAY>
       <SIZE>5</>
      </>
      <FIELD>
       <NAME>FILLER</>
       <TYPE>string</>
       <PHYSICAL>
        <CODESET>ebcdic-m</>
        <WIDTH>50</>
       </>
      </>
     </>
    </>
   </>
   <FIELD>
    <NAME>TEST_C065_A7_B28_B58_A1</>
    <TYPE>number</>
    <PHYSICAL>
     <TYPE>bcd_t</>
     <WIDTH>3</>
    </>
   </>
   <FIELD>
    <NAME>TEST_B23_D0028_C069_B28</>
    <FIELD>
     <NAME>TEST_D0030_C069_B28</>
     <FIELD>
      <NAME>TEST_C036_C069_B28_2</>
      <ARRAY>
       <SIZE>5</>
      </>
      <FIELD>
       <NAME>FILLER_2</>
       <TYPE>string</>
       <PHYSICAL>
        <CODESET>ebcdic-m</>
        <WIDTH>50</>
       </>
      </>
     </>
    </>
   </>
  </>
  <FIELD>
   <NAME>TEST_B23_B59_B58_A3</>
   <TYPE>number</>
   <PHYSICAL>
    <TYPE>bcd_t</>
    <WIDTH>2</>
   </>
  </>
 </>
 <FIELD>
  <NAME>TEST_B23_B12_C002_B28</>
  <FIELD>
   <NAME>TEST_B31_B12_C002</>
   <FIELD>
    <NAME>TEST_B06_A5</>
    <FIELD>
     <NAME>TEST_C042_B06_A5</>
     <FIELD>
      <NAME>TEST_C028_E00006_A1</>
      <TYPE>string</>
      <PHYSICAL>
       <CODESET>ebcdic-m</>
       <WIDTH>8</>
      </>
     </>
     <FIELD>
      <NAME>TEST_B55_B58_B42</>
      <TYPE>unsigned number</>
      <PHYSICAL>
       <CODESET>ebcdic-m</>
       <FILL>0</>
       <WIDTH>8</>
      </>
     </>
     <FIELD>
      <NAME>TEST_B29_B58_B42</>
      <TYPE>number</>
      <PHYSICAL>
       <TYPE>bcd_t</>
       <WIDTH>2</>
      </>
     </>
    </>
    <FIELD>
     <NAME>TEST_B06_B58_B42</>
     <TYPE>number</>
     <PHYSICAL>
      <TYPE>bcd_t</>
      <WIDTH>3</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C071_B06_A5</>
     <FIELD>
      <NAME>TEST_C067_A4</>
      <TYPE>string</>
      <PHYSICAL>
       <CODESET>ebcdic-m</>
       <WIDTH>10</>
      </>
     </>
     <FIELD>
      <NAME>TEST_C067_B58_B42</>
      <TYPE>number</>
      <PHYSICAL>
       <TYPE>bcd_t</>
       <WIDTH>7</>
      </>
     </>
     <FIELD>
      <NAME>TEST_B60_B64</>
      <TYPE>string</>
      <PHYSICAL>
       <CODESET>ebcdic-m</>
       <WIDTH>3</>
      </>
     </>
    </>
   </>
   <FIELD>
    <NAME>TEST_B06_C076</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>3</>
    </>
   </>
   <FIELD>
    <NAME>TEST_D0026_C076</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>2</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C006_C075_A1</>
    <TYPE>number</>
    <PHYSICAL>
     <TYPE>bcd_t</>
     <WIDTH>5</>
    </>
   </>
   <FIELD>
    <NAME>TEST_A2_B32</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>1</>
    </>
   </>
   <FIELD>
    <NAME>TEST_A6_B55_C076_A1</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>2</>
    </>
   </>
   <FIELD>
    <NAME>TEST_B39_A16_C065_B32</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>1</>
    </>
   </>
   <FIELD>
    <NAME>TEST_D0005_A10_C066</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>6</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C007_B13_B11_B32</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>1</>
    </>
   </>
   <FIELD>
    <NAME>TEST_B06_C018_C041_E00003</>
    <FIELD>
     <NAME>TEST_C006_C066_A5</>
     <FIELD>
      <NAME>TEST_B42_B28</>
      <FIELD>
       <NAME>TEST_C052_C066_C076_A1</>
       <TYPE>string</>
       <PHYSICAL>
        <CODESET>ebcdic-m</>
        <WIDTH>1</>
       </>
      </>
      <FIELD>
       <NAME>TEST_C036_C066_B42_B28</>
       <FIELD>
        <NAME>TEST_C036_C066_B42</>
        <TYPE>string</>
        <PHYSICAL>
         <CODESET>ebcdic-m</>
         <WIDTH>35</>
        </>
       </>
      </>
      <FIELD>
       <NAME>TEST_B41_C066_B42_B28</>
       <UNION>TEST_C036_C066_B42_B28</>
       <FIELD>
        <NAME>TEST_B43</>
        <TYPE>string</>
        <PHYSICAL>
         <CODESET>ebcdic-m</>
         <WIDTH>3</>
        </>
       </>
       <FIELD>
        <NAME>TEST_B50</>
        <TYPE>string</>
        <PHYSICAL>
         <CODESET>ebcdic-m</>
         <WIDTH>3</>
        </>
       </>
       <FIELD>
        <NAME>TEST_C046_B42</>
        <TYPE>string</>
        <PHYSICAL>
         <CODESET>ebcdic-m</>
         <WIDTH>4</>
        </>
       </>
       <FIELD>
        <NAME>FILLER_3</>
        <TYPE>string</>
        <PHYSICAL>
         <CODESET>ebcdic-m</>
         <WIDTH>25</>
        </>
       </>
      </>
      <FIELD>
       <NAME>TEST_D0017_C066_B42_B28</>
       <UNION>TEST_C036_C066_B42_B28</>
       <FIELD>
        <NAME>TEST_D0017_B42_B36</>
        <TYPE>string</>
        <PHYSICAL>
         <CODESET>ebcdic-m</>
         <WIDTH>2</>
        </>
       </>
       <FIELD>
        <NAME>TEST_D0017_B42_B28</>
        <FIELD>
         <NAME>TEST_D0017_B42_B52</>
         <TYPE>string</>
         <PHYSICAL>
          <CODESET>ebcdic-m</>
          <WIDTH>10</>
         </>
        </>
        <FIELD>
         <NAME>TEST_D0017_B42_B53</>
         <TYPE>string</>
         <PHYSICAL>
          <CODESET>ebcdic-m</>
          <WIDTH>6</>
         </>
        </>
       </>
       <FIELD>
        <NAME>FILLER_4</>
        <TYPE>string</>
        <PHYSICAL>
         <CODESET>ebcdic-m</>
         <WIDTH>17</>
        </>
       </>
      </>
      <FIELD>
       <NAME>TEST_B43_B50_B42_B28</>
       <UNION>TEST_C036_C066_B42_B28</>
       <FIELD>
        <NAME>TEST_C054_B43_B50</>
        <TYPE>string</>
        <PHYSICAL>
         <CODESET>ebcdic-m</>
         <WIDTH>6</>
        </>
       </>
       <FIELD>
        <NAME>FILLER_5</>
        <TYPE>string</>
        <PHYSICAL>
         <CODESET>ebcdic-m</>
         <WIDTH>29</>
        </>
       </>
      </>
     </>
     <FIELD>
      <NAME>TEST_C006_C066_B60_B42_B28</>
      <UNION>TEST_B42_B28</>
      <FIELD>
       <NAME>TEST_C006_C052_C066_B70</>
       <TYPE>string</>
       <PHYSICAL>
        <CODESET>ebcdic-m</>
        <WIDTH>1</>
       </>
      </>
      <FIELD>
       <NAME>TEST_C006_C066_B42_B28</>
       <FIELD>
        <NAME>TEST_C006_B43</>
        <TYPE>string</>
        <PHYSICAL>
         <CODESET>ebcdic-m</>
         <WIDTH>3</>
        </>
       </>
       <FIELD>
        <NAME>TEST_C006_B50</>
        <TYPE>string</>
        <PHYSICAL>
         <CODESET>ebcdic-m</>
         <WIDTH>3</>
        </>
       </>
       <FIELD>
        <NAME>TEST_C006_C046_B42</>
        <TYPE>string</>
        <PHYSICAL>
         <CODESET>ebcdic-m</>
         <WIDTH>4</>
        </>
       </>
       <FIELD>
        <NAME>FILLER_6</>
        <TYPE>string</>
        <PHYSICAL>
         <CODESET>ebcdic-m</>
         <WIDTH>25</>
        </>
       </>
      </>
      <FIELD>
       <NAME>TEST_C006_B41_B42_B28</>
       <UNION>TEST_C006_C066_B42_B28</>
       <FIELD>
        <NAME>TEST_C006_B42_B41</>
        <TYPE>string</>
        <PHYSICAL>
         <CODESET>ebcdic-m</>
         <WIDTH>10</>
        </>
       </>
       <FIELD>
        <NAME>FILLER_7</>
        <TYPE>string</>
        <PHYSICAL>
         <CODESET>ebcdic-m</>
         <WIDTH>25</>
        </>
       </>
      </>
      <FIELD>
       <NAME>TEST_C006_D0017_C066_B42_B28</>
       <UNION>TEST_C006_C066_B42_B28</>
       <FIELD>
        <NAME>TEST_D0017_B42_B36_2</>
        <TYPE>string</>
        <PHYSICAL>
         <CODESET>ebcdic-m</>
         <WIDTH>2</>
        </>
       </>
       <FIELD>
        <NAME>TEST_C006_D0017_B42_B28</>
        <FIELD>
         <NAME>FILLER_8</>
         <TYPE>string</>
         <PHYSICAL>
          <CODESET>ebcdic-m</>
          <WIDTH>16</>
         </>
        </>
       </>
       <FIELD>
        <NAME>FILLER_9</>
        <TYPE>string</>
        <PHYSICAL>
         <CODESET>ebcdic-m</>
         <WIDTH>17</>
        </>
       </>
      </>
      <FIELD>
       <NAME>TEST_C006_D0022_B42_B28</>
       <UNION>TEST_C006_C066_B42_B28</>
       <FIELD>
        <NAME>TEST_C006_C066_D0004_A5</>
        <TYPE>string</>
        <PHYSICAL>
         <CODESET>ebcdic-m</>
         <WIDTH>30</>
        </>
       </>
       <FIELD>
        <NAME>TEST_D0004_C048_A1</>
        <TYPE>string</>
        <PHYSICAL>
         <CODESET>ebcdic-m</>
         <WIDTH>4</>
        </>
       </>
      </>
     </>
    </>
    <FIELD>
     <NAME>TEST_C071_C037_A12_A5</>
     <TYPE>number</>
     <PHYSICAL>
      <TYPE>bcd_t</>
      <WIDTH>7</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C037_B14_A12_A5</>
     <TYPE>number</>
     <PHYSICAL>
      <TYPE>bcd_t</>
      <WIDTH>7</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C066_C003_A5</>
     <TYPE>number</>
     <PHYSICAL>
      <TYPE>bcd_t</>
      <WIDTH>7</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C037_A12_A5</>
     <TYPE>number</>
     <PHYSICAL>
      <TYPE>bcd_t</>
      <WIDTH>7</>
     </>
    </>
    <FIELD>
     <NAME>TEST_D0027_C003</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>4</>
     </>
    </>
    <FIELD>
     <NAME>TEST_B63_C001_C039</>
     <TYPE>unsigned number</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <FILL>0</>
      <WIDTH>13</>
     </>
    </>
    <FIELD>
     <NAME>TEST_B34_A1</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>2</>
     </>
    </>
    <FIELD>
     <NAME>TEST_D0020_C073_A1</>
     <ARRAY>
      <SIZE>2</>
     </>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>2</>
     </>
    </>
   </>
   <FIELD>
    <NAME>TEST_C018_C003_A5</>
    <TYPE>number</>
    <PHYSICAL>
     <TYPE>bcd_t</>
     <WIDTH>5</>
    </>
   </>
   <FIELD>
    <NAME>TEST_B06_C060_D0033</>
    <FIELD>
     <NAME>TEST_C060_C053_A1</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>8</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C060_COMPNT_A1</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>8</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C043_C060_C041_B66</>
     <FIELD>
      <NAME>TEST_C043_C060_C041_B28</>
      <ARRAY>
       <SIZE>5</>
      </>
      <FIELD>
       <NAME>TEST_C043_C060_C076_A1</>
       <TYPE>string</>
       <PHYSICAL>
        <CODESET>ebcdic-m</>
        <WIDTH>1</>
       </>
      </>
      <FIELD>
       <NAME>TEST_C043_C060_B35_A1</>
       <TYPE>string</>
       <PHYSICAL>
        <CODESET>ebcdic-m</>
        <WIDTH>4</>
       </>
      </>
     </>
    </>
    <FIELD>
     <NAME>TEST_C060_C006_A5</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>8</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C006_C026_A1</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>8</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C006_C026_B09_A1</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>1</>
     </>
    </>
    <FIELD>
     <NAME>TEST_B48_E00006</>
     <TYPE>number</>
     <PHYSICAL>
      <TYPE>bcd_t</>
      <WIDTH>2</>
     </>
    </>
   </>
   <FIELD>
    <NAME>TEST_C062_D0033_B75_B42</>
    <TYPE>number</>
    <PHYSICAL>
     <TYPE>bcd_t</>
     <WIDTH>2</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C041_C051_B28</>
    <FIELD>
     <NAME>TEST_C036_D0013</>
     <ARRAY>
      <SIZE>6</>
     </>
     <TYPE>number</>
     <PHYSICAL>
      <TYPE>bcd_t</>
      <WIDTH>9</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C036_B54</>
     <ARRAY>
      <SIZE>5</>
     </>
     <TYPE>number</>
     <PHYSICAL>
      <TYPE>bcd_t</>
      <FIXEDPOINT>2</>
      <WIDTH>9</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C036_B03</>
     <ARRAY>
      <SIZE>7</>
     </>
     <TYPE>number</>
     <PHYSICAL>
      <TYPE>bcd_t</>
      <FIXEDPOINT>7</>
      <WIDTH>8</>
     </>
    </>
   </>
   <FIELD>
    <NAME>TEST_B31_C020_B28</>
    <FIELD>
     <NAME>TEST_D0015_A16_A4</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>10</>
     </>
    </>
    <FIELD>
     <NAME>TEST_B06_B62_A4</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>10</>
     </>
    </>
    <FIELD>
     <NAME>TEST_B06_B21_A4</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>10</>
     </>
    </>
   </>
   <FIELD>
    <NAME>TEST_B71_A1</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>5</>
    </>
   </>
   <FIELD>
    <NAME>TEST_B12_E00002_C019</>
    <FIELD>
     <NAME>TEST_E00009_A14_D0007_A1</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>2</>
     </>
    </>
    <FIELD>
     <NAME>TEST_D0034_A14_D0007_A1</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>2</>
     </>
    </>
    <FIELD>
     <NAME>TEST_E00009_C045</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>3</>
     </>
    </>
    <FIELD>
     <NAME>TEST_D0034_C045</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>3</>
     </>
    </>
    <FIELD>
     <NAME>TEST_B08_A5</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>4</>
     </>
    </>
   </>
   <FIELD>
    <NAME>TEST_C061_B32</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>1</>
    </>
   </>
   <FIELD>
    <NAME>TEST_E00007_D0005_A10_C066</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>5</>
    </>
   </>
   <FIELD>
    <NAME>TEST_E00008_C011</>
    <TYPE>number</>
    <PHYSICAL>
     <TYPE>bcd_t</>
     <FIXEDPOINT>2</>
     <WIDTH>5</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C044_D0019_A1</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>1</>
    </>
   </>
   <FIELD>
    <NAME>TEST_D0012_C076</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>1</>
    </>
   </>
   <FIELD>
    <NAME>TEST_D0012_C027_B32</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>1</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C010_D0013_B54_C002</>
    <FIELD>
     <NAME>TEST_C009_D0013_C064</>
     <TYPE>number</>
     <PHYSICAL>
      <TYPE>bcd_t</>
      <FIXEDPOINT>5</>
      <WIDTH>8</>
     </>
    </>
    <FIELD>
     <NAME>TEST_F000003_D0013_C064</>
     <TYPE>number</>
     <PHYSICAL>
      <TYPE>bcd_t</>
      <FIXEDPOINT>5</>
      <WIDTH>8</>
     </>
    </>
    <FIELD>
     <NAME>TEST_D0014_B54</>
     <TYPE>number</>
     <PHYSICAL>
      <TYPE>bcd_t</>
      <FIXEDPOINT>3</>
      <WIDTH>6</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C038_C009_C005_B54</>
     <TYPE>number</>
     <PHYSICAL>
      <TYPE>bcd_t</>
      <FIXEDPOINT>2</>
      <WIDTH>5</>
     </>
    </>
   </>
   <FIELD>
    <NAME>TEST_E00004_B03</>
    <TYPE>number</>
    <PHYSICAL>
     <TYPE>bcd_t</>
     <FIXEDPOINT>7</>
     <WIDTH>8</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C072_C011_B32</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>1</>
    </>
   </>
   <FIELD>
    <NAME>TEST_E00005_C056_B28</>
    <FIELD>
     <NAME>TEST_E00005_C024</>
     <TYPE>number</>
     <PHYSICAL>
      <TYPE>bcd_t</>
      <WIDTH>3</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C056_B42</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>9</>
     </>
    </>
   </>
   <FIELD>
    <NAME>TEST_B31_B65_B28</>
    <FIELD>
     <NAME>TEST_A15_B65_B09_A1</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>2</>
     </>
    </>
    <FIELD>
     <NAME>TEST_D0016_E00012</>
     <FIELD>
      <NAME>TEST_B65_D0001_B32</>
      <TYPE>string</>
      <PHYSICAL>
       <CODESET>ebcdic-m</>
       <WIDTH>1</>
      </>
     </>
     <FIELD>
      <NAME>TEST_B65_D0019_A1_B28</>
      <FIELD>
       <NAME>TEST_B41_A1</>
       <TYPE>string</>
       <PHYSICAL>
        <CODESET>ebcdic-m</>
        <WIDTH>2</>
       </>
      </>
      <FIELD>
       <NAME>TEST_D0032_A1</>
       <TYPE>string</>
       <PHYSICAL>
        <CODESET>ebcdic-m</>
        <WIDTH>2</>
       </>
      </>
      <FIELD>
       <NAME>TEST_C015_A1</>
       <TYPE>string</>
       <PHYSICAL>
        <CODESET>ebcdic-m</>
        <WIDTH>3</>
       </>
      </>
      <FIELD>
       <NAME>TEST_C012_A1</>
       <TYPE>string</>
       <PHYSICAL>
        <CODESET>ebcdic-m</>
        <WIDTH>4</>
       </>
      </>
      <FIELD>
       <NAME>TEST_F000005_A1</>
       <TYPE>string</>
       <PHYSICAL>
        <CODESET>ebcdic-m</>
        <WIDTH>2</>
       </>
      </>
     </>
     <FIELD>
      <NAME>TEST_B44_B32</>
      <TYPE>string</>
      <PHYSICAL>
       <CODESET>ebcdic-m</>
       <WIDTH>1</>
      </>
     </>
     <FIELD>
      <NAME>TEST_B65_B22</>
      <TYPE>string</>
      <PHYSICAL>
       <CODESET>ebcdic-m</>
       <WIDTH>5</>
      </>
     </>
     <FIELD>
      <NAME>TEST_C076_A10_C001</>
      <TYPE>string</>
      <PHYSICAL>
       <CODESET>ebcdic-m</>
       <WIDTH>2</>
      </>
     </>
     <FIELD>
      <NAME>TEST_COMPLNC_C075_A1</>
      <TYPE>string</>
      <PHYSICAL>
       <CODESET>ebcdic-m</>
       <WIDTH>1</>
      </>
     </>
     <FIELD>
      <NAME>TEST_C066_B28</>
      <TYPE>string</>
      <PHYSICAL>
       <CODESET>ebcdic-m</>
       <WIDTH>1</>
      </>
     </>
     <FIELD>
      <NAME>TEST_B04_F000001_B07</>
      <TYPE>string</>
      <PHYSICAL>
       <CODESET>ebcdic-m</>
       <WIDTH>2</>
      </>
     </>
     <FIELD>
      <NAME>TEST_C033_A10_C011</>
      <TYPE>string</>
      <PHYSICAL>
       <CODESET>ebcdic-m</>
       <WIDTH>1</>
      </>
     </>
     <FIELD>
      <NAME>TEST_C011_C076_A1</>
      <TYPE>string</>
      <PHYSICAL>
       <CODESET>ebcdic-m</>
       <WIDTH>2</>
      </>
     </>
     <FIELD>
      <NAME>TEST_F000006_B65_C008</>
      <TYPE>string</>
      <PHYSICAL>
       <CODESET>ebcdic-m</>
       <WIDTH>2</>
      </>
     </>
     <FIELD>
      <NAME>TEST_C047_B40_B74</>
      <TYPE>unsigned number</>
      <PHYSICAL>
       <CODESET>ebcdic-m</>
       <FILL>0</>
       <WIDTH>7</>
      </>
     </>
     <FIELD>
      <NAME>TEST_C047_C066_D0006_A5</>
      <TYPE>unsigned number</>
      <PHYSICAL>
       <CODESET>ebcdic-m</>
       <FILL>0</>
       <WIDTH>8</>
      </>
     </>
     <FIELD>
      <NAME>TEST_B65_B42_B02_D0021</>
      <TYPE>unsigned number</>
      <PHYSICAL>
       <CODESET>ebcdic-m</>
       <FILL>0</>
       <WIDTH>7</>
      </>
     </>
     <FIELD>
      <NAME>TEST_B65_E00002</>
      <TYPE>string</>
      <PHYSICAL>
       <CODESET>ebcdic-m</>
       <WIDTH>2</>
      </>
     </>
     <FIELD>
      <NAME>TEST_B65_C021</>
      <TYPE>string</>
      <PHYSICAL>
       <CODESET>ebcdic-m</>
       <WIDTH>1</>
      </>
     </>
     <FIELD>
      <NAME>TEST_B24_B65_C030</>
      <TYPE>string</>
      <PHYSICAL>
       <CODESET>ebcdic-m</>
       <WIDTH>1</>
      </>
     </>
     <FIELD>
      <NAME>TEST_C074_B65_A1</>
      <TYPE>string</>
      <PHYSICAL>
       <CODESET>ebcdic-m</>
       <WIDTH>1</>
      </>
     </>
    </>
    <FIELD>
     <NAME>TEST_D0016_F000004</>
     <FIELD>
      <NAME>TEST_C060_C026_A1</>
      <TYPE>string</>
      <PHYSICAL>
       <CODESET>ebcdic-m</>
       <WIDTH>10</>
      </>
     </>
     <FIELD>
      <NAME>TEST_D0035_C063_A1</>
      <TYPE>string</>
      <PHYSICAL>
       <CODESET>ebcdic-m</>
       <WIDTH>4</>
      </>
     </>
     <FIELD>
      <NAME>TEST_B65_D0002_B28</>
      <FIELD>
       <NAME>TEST_F000002_B65_D0002</>
       <TYPE>string</>
       <PHYSICAL>
        <CODESET>ebcdic-m</>
        <WIDTH>1</>
       </>
      </>
      <FIELD>
       <NAME>TEST_D0032_B65_D0002</>
       <TYPE>string</>
       <PHYSICAL>
        <CODESET>ebcdic-m</>
        <WIDTH>1</>
       </>
      </>
      <FIELD>
       <NAME>TEST_C012_B65_D0002</>
       <TYPE>string</>
       <PHYSICAL>
        <CODESET>ebcdic-m</>
        <WIDTH>1</>
       </>
      </>
      <FIELD>
       <NAME>TEST_E00001_B65_D0002</>
       <TYPE>string</>
       <PHYSICAL>
        <CODESET>ebcdic-m</>
        <WIDTH>1</>
       </>
      </>
      <FIELD>
       <NAME>TEST_F000005_B65_D0002</>
       <TYPE>string</>
       <PHYSICAL>
        <CODESET>ebcdic-m</>
        <WIDTH>1</>
       </>
      </>
     </>
     <FIELD>
      <NAME>TEST_C060_B73_A1</>
      <TYPE>string</>
      <PHYSICAL>
       <CODESET>ebcdic-m</>
       <WIDTH>2</>
      </>
     </>
     <FIELD>
      <NAME>TEST_C060_C016_B03</>
      <TYPE>number</>
      <PHYSICAL>
       <TYPE>bcd_t</>
       <FIXEDPOINT>4</>
       <WIDTH>8</>
      </>
     </>
    </>
    <FIELD>
     <NAME>TEST_C075_C076</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>4</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C063_C076</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>4</>
     </>
    </>
    <FIELD>
     <NAME>TEST_B68_B65_B03</>
     <TYPE>number</>
     <PHYSICAL>
      <TYPE>bcd_t</>
      <FIXEDPOINT>7</>
      <WIDTH>10</>
     </>
    </>
    <FIELD>
     <NAME>TEST_B42_A10_OCCURS</>
     <TYPE>number</>
     <PHYSICAL>
      <TYPE>be_t</>
      <WIDTH>2</>
     </>
    </>
   </>
  </>
  <FIELD>
   <NAME>TEST_B05_B12_C002</>
   <FIELD>
    <NAME>TEST_C037_D0024_B33_2</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>80</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C060_D0033_C023</>
    <FIELD>
     <NAME>TEST_C060_C031_C023</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>30</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C060_C076_C023</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>30</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C011_B28</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>30</>
     </>
    </>
    <FIELD>
     <NAME>TEST_B02_C076_C023</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>30</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C013_A10_D0003_C023</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>30</>
     </>
    </>
   </>
   <FIELD>
    <NAME>TEST_C035</>
    <TYPE>number</>
    <PHYSICAL>
     <TYPE>bcd_t</>
     <WIDTH>6</>
    </>
   </>
   <FIELD>
    <NAME>TEST_D0018_D0009_B61_A1</>
    <TYPE>number</>
    <PHYSICAL>
     <TYPE>bcd_t</>
     <WIDTH>2</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C066_B42_B35</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>30</>
    </>
   </>
   <FIELD>
    <NAME>TEST_D0018_D0009_C023</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>30</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C066_C003_A5_2</>
    <TYPE>number</>
    <PHYSICAL>
     <TYPE>bcd_t</>
     <WIDTH>7</>
    </>
   </>
   <FIELD>
    <NAME>TEST_D0012_B49_B45</>
    <TYPE>number</>
    <PHYSICAL>
     <TYPE>bcd_t</>
     <FIXEDPOINT>13</>
     <WIDTH>8</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C001_B28_C011_B32</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>1</>
    </>
   </>
   <FIELD>
    <NAME>TEST_B12_C037_A12_C023</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>30</>
    </>
   </>
   <FIELD>
    <NAME>TEST_B12_B10_A5</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>6</>
    </>
   </>
   <FIELD>
    <NAME>TEST_B73_C011_A1</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>2</>
    </>
   </>
  </>
  <FIELD>
   <NAME>TEST_B23_B12_C002</>
   <FIELD>
    <NAME>TEST_D0025_C037_B38_B42</>
    <TYPE>number</>
    <PHYSICAL>
     <TYPE>bcd_t</>
     <WIDTH>2</>
    </>
   </>
   <FIELD>
    <NAME>TEST_E00010_C037_A12_A5</>
    <TYPE>number</>
    <PHYSICAL>
     <TYPE>bcd_t</>
     <WIDTH>7</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C037_A12_B58_B42</>
    <TYPE>number</>
    <PHYSICAL>
     <TYPE>bcd_t</>
     <WIDTH>4</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C034_C065_B28_A1</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>6</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C034_D0029_B28_A1</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>5</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C065_B28_B09_A1</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>1</>
    </>
   </>
   <FIELD>
    <NAME>TEST_D0010_A5</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>8</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C065_A11_C025_A1</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>1</>
    </>
   </>
   <FIELD>
    <NAME>TEST_A13_B72_B23_B32</>
    <ARRAY>
     <SIZE>2</>
    </>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>1</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C022_C060_B35_B28</>
    <FIELD>
     <NAME>TEST_C043_C060_B35_B58</>
     <ARRAY>
      <SIZE>5</>
     </>
     <TYPE>number</>
     <PHYSICAL>
      <TYPE>bcd_t</>
      <WIDTH>2</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C043_C060_B35_B15</>
     <ARRAY>
      <SIZE>5</>
     </>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>40</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C043_C060_B35_B16</>
     <ARRAY>
      <SIZE>5</>
     </>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>30</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C043_C060_B35_B17</>
     <ARRAY>
      <SIZE>5</>
     </>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>25</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C043_C060_B35_B18</>
     <ARRAY>
      <SIZE>5</>
     </>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>25</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C043_C060_B35_B19</>
     <ARRAY>
      <SIZE>5</>
     </>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>20</>
     </>
    </>
   </>
   <FIELD>
    <NAME>TEST_C022_C006_C075_B28</>
    <FIELD>
     <NAME>TEST_C006_C075_B58</>
     <TYPE>number</>
     <PHYSICAL>
      <TYPE>bcd_t</>
      <WIDTH>5</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C006_C075_C023_B46</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>40</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C006_C075_C023_B69</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>30</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C006_C075_C023_D0036</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>25</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C006_C075_C023_C032</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>25</>
     </>
    </>
    <FIELD>
     <NAME>TEST_C006_C075_C023_C029</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>20</>
     </>
    </>
   </>
   <FIELD>
    <NAME>TEST_B67_C023</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>45</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C062_B51_A1_C023</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>30</>
    </>
   </>
   <FIELD>
    <NAME>TEST_A14_D0007_C023</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>25</>
    </>
   </>
   <FIELD>
    <NAME>TEST_D0020_C073</>
    <ARRAY>
     <SIZE>2</>
    </>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>1</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C009_B20_B29_A1</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>1</>
    </>
   </>
   <FIELD>
    <NAME>TEST_B25_A1</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>7</>
    </>
   </>
   <FIELD>
    <NAME>TEST_B71_A9</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>15</>
    </>
   </>
   <FIELD>
    <NAME>TEST_D0020_C023_B28</>
    <ARRAY>
     <SIZE>2</>
    </>
    <FIELD>
     <NAME>TEST_D0020_C023_B46</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>55</>
     </>
    </>
    <FIELD>
     <NAME>TEST_D0020_C023_B69</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>110</>
     </>
    </>
   </>
   <FIELD>
    <NAME>TEST_D0008_C009_B02_B42</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>23</>
    </>
   </>
   <FIELD>
    <NAME>TEST_D0008_C009_C023</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>25</>
    </>
   </>
  </>
 </>
 <FIELD>
  <NAME>TEST_B29_B23_C017_B28</>
  <FIELD>
   <NAME>TEST_B29_B23_C017_C002</>
   <FIELD>
    <NAME>TEST_C037_A5_2</>
    <TYPE>number</>
    <PHYSICAL>
     <TYPE>bcd_t</>
     <WIDTH>7</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C037_B14_A12_B28_2</>
    <FIELD>
     <NAME>TEST_C036_B07_C003_2</>
     <TYPE>number</>
     <PHYSICAL>
      <TYPE>bcd_t</>
      <WIDTH>7</>
     </>
    </>
    <FIELD>
     <NAME>TEST_B07_C003_C076_2</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>2</>
     </>
    </>
    <FIELD>
     <NAME>TEST_B14_A1_2</>
     <TYPE>string</>
     <PHYSICAL>
      <CODESET>ebcdic-m</>
      <WIDTH>2</>
     </>
    </>
   </>
   <FIELD>
    <NAME>TEST_D0026_C076_2</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>2</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C006_C050</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>1</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C049_C018_B28_A1</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>4</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C007_D0011_A1</>
    <TYPE>number</>
    <PHYSICAL>
     <TYPE>bcd_t</>
     <WIDTH>2</>
    </>
   </>
   <FIELD>
    <NAME>TEST_B63_D0011_A5</>
    <TYPE>number</>
    <PHYSICAL>
     <TYPE>be_t</>
     <WIDTH>2</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C006_E00011</>
    <TYPE>number</>
    <PHYSICAL>
     <TYPE>bcd_t</>
     <WIDTH>1</>
    </>
   </>
   <FIELD>
    <NAME>TEST_B14_C070_A1</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>1</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C004_A1</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>1</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C057_B47_A8</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>21</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C043_C018_B42</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>16</>
    </>
   </>
   <FIELD>
    <NAME>TEST_C014_C001_B42</>
    <TYPE>string</>
    <PHYSICAL>
     <CODESET>ebcdic-m</>
     <WIDTH>8</>
    </>
   </>
  </>
 </>
</>'

	EXEC	 --regress --bytemask
		OUTPUT - 110000000000333333333333330000033000000000000000000000000000000000000001103330\
000000000000000000000000000000000000000000000000000000000000000000000000000000\
000000000000000000000000000000000000000000000000000000000000000000000000000000\
333000000000000000000000000000000000000000000000000003300000000000000003333300\
000000003333333000000003333300000000000000000000000000000000000000000000000000\
000000000000000000000000000000000000000000000000000000000000000000000000000000\
000000000000000000000000000000000000000000000000000000000000000000000000000000\
000000000000000000000000000000000000000000000000000000000000000000000000000000\
000000003333333333333333333333333333000000000000000000000003333300000000000000\
000000000000000000000000333333333333333333333333333333333333333333333333333333\
333333333333333333333333333333333333333333333333333333333333333333333333333333\
333333333333333333333333333000000000000000000000000000000000000000000000000000\
000033333000333333333333333333333333333333333330333000000000000000000000000000\
000000000000000000000000000000000000000000000000000000000000000333333330000000\
033333333331100000000000000000000000000000000000000000000000000000000000000000\
000000000000000000000000000000000000000000000000000000000000000000000000000000\
000000000000000000000000000000000000000000000000000000000000000000000000000000\
000000000333333330000000000000000000000000000000000000000000000000000000000003\
333333333333330000000000000000000000000000000000000003333333333333000000000000\
000000000003333333333000000000000000000000000000000000000000000000000000000000\
000000000000000000000000000000000000000000000000000000000000000000000000000000\
000000000000000000000000000000000000000000000000000000000000000000000000000000\
000000000000000000000000000000000000000000000000000000000000000000000000000000\
000000000000000000000000000000000000000000000000000000000000000000000000000000\
000000000000000000000000000000000000000000000000000000000000000000000000000000\
000000000000000000000000000000000000000000000000000000000000000000000000000000\
000000000000000000000000000000000000000000000000000000000000000000000000000000\
000000000000000000000000000000000000000000000000000000000000000000000000000000\
000000000000000000033333000000000000000000000000000000000000000000000000000000\
000000000000000000000000000000000000000000000000000000000000000000000000000000\
000000000000000000000000000000000000000000000000000000000000000000000000000000\
000000000000000000000000000000000000000000000000000000000000000000000000000000\
000000000000000000000000000000000000000000000000000000000000000000000000000000\
000000000000000000000000000000000000000000000000000000000000000000000000000000\
000000000000000000000000000000000033333333333333000000000003311300000000000000\
000000000000000000000000000000000

	EXEC	 --regress --offsets
		OUTPUT - $'TEST_B74_B55_B36\t0\t2\t0\tbe_t
TEST_B23_C069_B33\t2\t306\t0\tstruct
TEST_C068_B23_C069_C002\t2\t303\t0\tstruct
TEST_D0018_B37_B08\t2\t23\t0\tstruct
TEST_C007_A4\t2\t10\t0\tstring
TEST_C037_A5\t12\t7\t0\tbcd_t
TEST_C037_B14_A12_B28\t19\t17\t0\tstruct
TEST_C036_B07_C003\t19\t7\t0\tbcd_t
TEST_B07_C003_C076\t26\t2\t0\tstring
TEST_B14_A1\t28\t2\t0\tstring
TEST_B37_B23_B55_C076\t30\t1\t0\tstring
TEST_A13_B59_C069_B42\t31\t2\t0\tbcd_t
TEST_B56_C003_A5\t33\t38\t0\tstring
TEST_B12_A13_B59_C040\t71\t2\t0\tbe_t
TEST_D0023_B23_B55_C076\t73\t1\t0\tstring
TEST_C065_B28_B58_A1\t74\t3\t0\tbcd_t
TEST_B61_B01_C037_B28\t77\t27\t0\tstruct
TEST_B27_B61_B01_C037\t77\t13\t0\tstring
TEST_B57_B61_B01_C037\t90\t13\t0\tstring
TEST_C078_C055_B26\t103\t1\t0\tstring
TEST_B23_C058_C069_B28\t104\t130\t0\tstruct
TEST_C058_D0024_C069_B28\t104\t80\t0\tstruct
TEST_C037_D0024_B33\t104\t80\t0\tstring
TEST_C058_C069_B28\t184\t50\t0\tstruct
TEST_C036_C069_B28\t184\t250\t5\tstruct
FILLER\t184\t50\t0\tstring
TEST_C065_A7_B28_B58_A1\t434\t3\t0\tbcd_t
TEST_B23_D0028_C069_B28\t437\t50\t0\tstruct
TEST_D0030_C069_B28\t437\t50\t0\tstruct
TEST_C036_C069_B28_2\t437\t250\t5\tstruct
FILLER_2\t437\t50\t0\tstring
TEST_B23_B59_B58_A3\t687\t2\t0\tbcd_t
TEST_B23_B12_C002_B28\t689\t2653\t0\tstruct
TEST_B31_B12_C002\t689\t1043\t0\tstruct
TEST_B06_A5\t689\t49\t0\tstruct
TEST_C042_B06_A5\t689\t19\t0\tstruct
TEST_C028_E00006_A1\t689\t8\t0\tstring
TEST_B55_B58_B42\t697\t8\t0\tunsigned
TEST_B29_B58_B42\t705\t2\t0\tbcd_t
TEST_B06_B58_B42\t707\t3\t0\tbcd_t
TEST_C071_B06_A5\t710\t26\t0\tstruct
TEST_C067_A4\t710\t10\t0\tstring
TEST_C067_B58_B42\t720\t7\t0\tbcd_t
TEST_B60_B64\t727\t3\t0\tstring
TEST_B06_C076\t730\t3\t0\tstring
TEST_D0026_C076\t733\t2\t0\tstring
TEST_C006_C075_A1\t735\t5\t0\tbcd_t
TEST_A2_B32\t740\t1\t0\tstring
TEST_A6_B55_C076_A1\t741\t2\t0\tstring
TEST_B39_A16_C065_B32\t743\t1\t0\tstring
TEST_D0005_A10_C066\t744\t6\t0\tstring
TEST_C007_B13_B11_B32\t750\t1\t0\tstring
TEST_B06_C018_C041_E00003\t751\t356\t0\tstruct
TEST_C006_C066_A5\t751\t281\t0\tstruct
TEST_B42_B28\t751\t141\t0\tstruct
TEST_C052_C066_C076_A1\t751\t1\t0\tstring
TEST_C036_C066_B42_B28\t752\t35\t0\tstruct
TEST_C036_C066_B42\t752\t35\t0\tstring
TEST_B41_C066_B42_B28\t752\t35\t0\tstruct
TEST_B43\t752\t3\t0\tstring
TEST_B50\t755\t3\t0\tstring
TEST_C046_B42\t758\t4\t0\tstring
FILLER_3\t762\t25\t0\tstring
TEST_D0017_C066_B42_B28\t752\t35\t0\tstruct
TEST_D0017_B42_B36\t752\t2\t0\tstring
TEST_D0017_B42_B28\t754\t16\t0\tstruct
TEST_D0017_B42_B52\t754\t10\t0\tstring
TEST_D0017_B42_B53\t764\t6\t0\tstring
FILLER_4\t770\t17\t0\tstring
TEST_B43_B50_B42_B28\t752\t35\t0\tstruct
TEST_C054_B43_B50\t752\t6\t0\tstring
FILLER_5\t758\t29\t0\tstring
TEST_C006_C066_B60_B42_B28\t751\t140\t0\tstruct
TEST_C006_C052_C066_B70\t751\t1\t0\tstring
TEST_C006_C066_B42_B28\t752\t35\t0\tstruct
TEST_C006_B43\t752\t3\t0\tstring
TEST_C006_B50\t755\t3\t0\tstring
TEST_C006_C046_B42\t758\t4\t0\tstring
FILLER_6\t762\t25\t0\tstring
TEST_C006_B41_B42_B28\t752\t35\t0\tstruct
TEST_C006_B42_B41\t752\t10\t0\tstring
FILLER_7\t762\t25\t0\tstring
TEST_C006_D0017_C066_B42_B28\t752\t35\t0\tstruct
TEST_D0017_B42_B36_2\t752\t2\t0\tstring
TEST_C006_D0017_B42_B28\t754\t16\t0\tstruct
FILLER_8\t754\t16\t0\tstring
FILLER_9\t770\t17\t0\tstring
TEST_C006_D0022_B42_B28\t752\t34\t0\tstruct
TEST_C006_C066_D0004_A5\t752\t30\t0\tstring
TEST_D0004_C048_A1\t782\t4\t0\tstring
TEST_C071_C037_A12_A5\t787\t7\t0\tbcd_t
TEST_C037_B14_A12_A5\t794\t7\t0\tbcd_t
TEST_C066_C003_A5\t801\t7\t0\tbcd_t
TEST_C037_A12_A5\t808\t7\t0\tbcd_t
TEST_D0027_C003\t815\t4\t0\tstring
TEST_B63_C001_C039\t819\t13\t0\tunsigned
TEST_B34_A1\t832\t2\t0\tstring
TEST_D0020_C073_A1\t834\t4\t2\tstring
TEST_C018_C003_A5\t838\t5\t0\tbcd_t
TEST_B06_C060_D0033\t843\t41\t0\tstruct
TEST_C060_C053_A1\t843\t8\t0\tstring
TEST_C060_COMPNT_A1\t851\t8\t0\tstring
TEST_C043_C060_C041_B66\t859\t5\t0\tstruct
TEST_C043_C060_C041_B28\t859\t25\t5\tstruct
TEST_C043_C060_C076_A1\t859\t1\t0\tstring
TEST_C043_C060_B35_A1\t860\t4\t0\tstring
TEST_C060_C006_A5\t884\t8\t0\tstring
TEST_C006_C026_A1\t892\t8\t0\tstring
TEST_C006_C026_B09_A1\t900\t1\t0\tstring
TEST_B48_E00006\t901\t2\t0\tbcd_t
TEST_C062_D0033_B75_B42\t903\t2\t0\tbcd_t
TEST_C041_C051_B28\t905\t292\t0\tstruct
TEST_C036_D0013\t905\t54\t6\tbcd_t
TEST_C036_B54\t959\t45\t5\tbcd_t
TEST_C036_B03\t1004\t56\t7\tbcd_t
TEST_B31_C020_B28\t1060\t30\t0\tstruct
TEST_D0015_A16_A4\t1060\t10\t0\tstring
TEST_B06_B62_A4\t1070\t10\t0\tstring
TEST_B06_B21_A4\t1080\t10\t0\tstring
TEST_B71_A1\t1090\t5\t0\tstring
TEST_B12_E00002_C019\t1095\t14\t0\tstruct
TEST_E00009_A14_D0007_A1\t1095\t2\t0\tstring
TEST_D0034_A14_D0007_A1\t1097\t2\t0\tstring
TEST_E00009_C045\t1099\t3\t0\tstring
TEST_D0034_C045\t1102\t3\t0\tstring
TEST_B08_A5\t1105\t4\t0\tstring
TEST_C061_B32\t1109\t1\t0\tstring
TEST_E00007_D0005_A10_C066\t1110\t5\t0\tstring
TEST_E00008_C011\t1115\t5\t0\tbcd_t
TEST_C044_D0019_A1\t1120\t1\t0\tstring
TEST_D0012_C076\t1121\t1\t0\tstring
TEST_D0012_C027_B32\t1122\t1\t0\tstring
TEST_C010_D0013_B54_C002\t1123\t49\t0\tstruct
TEST_C009_D0013_C064\t1123\t8\t0\tbcd_t
TEST_F000003_D0013_C064\t1131\t8\t0\tbcd_t
TEST_D0014_B54\t1139\t6\t0\tbcd_t
TEST_C038_C009_C005_B54\t1145\t5\t0\tbcd_t
TEST_E00004_B03\t1150\t8\t0\tbcd_t
TEST_C072_C011_B32\t1158\t1\t0\tstring
TEST_E00005_C056_B28\t1159\t13\t0\tstruct
TEST_E00005_C024\t1159\t3\t0\tbcd_t
TEST_C056_B42\t1162\t9\t0\tstring
TEST_B31_B65_B28\t1171\t126\t0\tstruct
TEST_A15_B65_B09_A1\t1171\t2\t0\tstring
TEST_D0016_E00012\t1173\t58\t0\tstruct
TEST_B65_D0001_B32\t1173\t1\t0\tstring
TEST_B65_D0019_A1_B28\t1174\t13\t0\tstruct
TEST_B41_A1\t1174\t2\t0\tstring
TEST_D0032_A1\t1176\t2\t0\tstring
TEST_C015_A1\t1178\t3\t0\tstring
TEST_C012_A1\t1181\t4\t0\tstring
TEST_F000005_A1\t1185\t2\t0\tstring
TEST_B44_B32\t1187\t1\t0\tstring
TEST_B65_B22\t1188\t5\t0\tstring
TEST_C076_A10_C001\t1193\t2\t0\tstring
TEST_COMPLNC_C075_A1\t1195\t1\t0\tstring
TEST_C066_B28\t1196\t1\t0\tstring
TEST_B04_F000001_B07\t1197\t2\t0\tstring
TEST_C033_A10_C011\t1199\t1\t0\tstring
TEST_C011_C076_A1\t1200\t2\t0\tstring
TEST_F000006_B65_C008\t1202\t2\t0\tstring
TEST_C047_B40_B74\t1204\t7\t0\tunsigned
TEST_C047_C066_D0006_A5\t1211\t8\t0\tunsigned
TEST_B65_B42_B02_D0021\t1219\t7\t0\tunsigned
TEST_B65_E00002\t1226\t2\t0\tstring
TEST_B65_C021\t1228\t1\t0\tstring
TEST_B24_B65_C030\t1229\t1\t0\tstring
TEST_C074_B65_A1\t1230\t1\t0\tstring
TEST_D0016_F000004\t1231\t36\t0\tstruct
TEST_C060_C026_A1\t1231\t10\t0\tstring
TEST_D0035_C063_A1\t1241\t4\t0\tstring
TEST_B65_D0002_B28\t1245\t5\t0\tstruct
TEST_F000002_B65_D0002\t1245\t1\t0\tstring
TEST_D0032_B65_D0002\t1246\t1\t0\tstring
TEST_C012_B65_D0002\t1247\t1\t0\tstring
TEST_E00001_B65_D0002\t1248\t1\t0\tstring
TEST_F000005_B65_D0002\t1249\t1\t0\tstring
TEST_C060_B73_A1\t1250\t2\t0\tstring
TEST_C060_C016_B03\t1252\t8\t0\tbcd_t
TEST_C075_C076\t1260\t4\t0\tstring
TEST_C063_C076\t1264\t4\t0\tstring
TEST_B68_B65_B03\t1268\t10\t0\tbcd_t
TEST_B42_A10_OCCURS\t1278\t2\t0\tbe_t
TEST_B05_B12_C002\t1280\t370\t0\tstruct
TEST_C037_D0024_B33_2\t1280\t80\t0\tstring
TEST_C060_D0033_C023\t1360\t150\t0\tstruct
TEST_C060_C031_C023\t1360\t30\t0\tstring
TEST_C060_C076_C023\t1390\t30\t0\tstring
TEST_C011_B28\t1420\t30\t0\tstring
TEST_B02_C076_C023\t1450\t30\t0\tstring
TEST_C013_A10_D0003_C023\t1480\t30\t0\tstring
TEST_C035\t1510\t6\t0\tbcd_t
TEST_D0018_D0009_B61_A1\t1516\t2\t0\tbcd_t
TEST_C066_B42_B35\t1518\t30\t0\tstring
TEST_D0018_D0009_C023\t1548\t30\t0\tstring
TEST_C066_C003_A5_2\t1578\t7\t0\tbcd_t
TEST_D0012_B49_B45\t1585\t8\t0\tbcd_t
TEST_C001_B28_C011_B32\t1593\t1\t0\tstring
TEST_B12_C037_A12_C023\t1594\t30\t0\tstring
TEST_B12_B10_A5\t1624\t6\t0\tstring
TEST_B73_C011_A1\t1630\t2\t0\tstring
TEST_B23_B12_C002\t1632\t1240\t0\tstruct
TEST_D0025_C037_B38_B42\t1632\t2\t0\tbcd_t
TEST_E00010_C037_A12_A5\t1634\t7\t0\tbcd_t
TEST_C037_A12_B58_B42\t1641\t4\t0\tbcd_t
TEST_C034_C065_B28_A1\t1645\t6\t0\tstring
TEST_C034_D0029_B28_A1\t1651\t5\t0\tstring
TEST_C065_B28_B09_A1\t1656\t1\t0\tstring
TEST_D0010_A5\t1657\t8\t0\tstring
TEST_C065_A11_C025_A1\t1665\t1\t0\tstring
TEST_A13_B72_B23_B32\t1666\t2\t2\tstring
TEST_C022_C060_B35_B28\t1668\t710\t0\tstruct
TEST_C043_C060_B35_B58\t1668\t10\t5\tbcd_t
TEST_C043_C060_B35_B15\t1678\t200\t5\tstring
TEST_C043_C060_B35_B16\t1878\t150\t5\tstring
TEST_C043_C060_B35_B17\t2028\t125\t5\tstring
TEST_C043_C060_B35_B18\t2153\t125\t5\tstring
TEST_C043_C060_B35_B19\t2278\t100\t5\tstring
TEST_C022_C006_C075_B28\t2378\t148\t0\tstruct
TEST_C006_C075_B58\t2378\t5\t0\tbcd_t
TEST_C006_C075_C023_B46\t2383\t40\t0\tstring
TEST_C006_C075_C023_B69\t2423\t30\t0\tstring
TEST_C006_C075_C023_D0036\t2453\t25\t0\tstring
TEST_C006_C075_C023_C032\t2478\t25\t0\tstring
TEST_C006_C075_C023_C029\t2503\t20\t0\tstring
TEST_B67_C023\t2523\t45\t0\tstring
TEST_C062_B51_A1_C023\t2568\t30\t0\tstring
TEST_A14_D0007_C023\t2598\t25\t0\tstring
TEST_D0020_C073\t2623\t2\t2\tstring
TEST_C009_B20_B29_A1\t2625\t1\t0\tstring
TEST_B25_A1\t2626\t7\t0\tstring
TEST_B71_A9\t2633\t15\t0\tstring
TEST_D0020_C023_B28\t2648\t330\t2\tstruct
TEST_D0020_C023_B46\t2648\t55\t0\tstring
TEST_D0020_C023_B69\t2703\t110\t0\tstring
TEST_D0008_C009_B02_B42\t2978\t23\t0\tstring
TEST_D0008_C009_C023\t3001\t25\t0\tstring
TEST_B29_B23_C017_B28\t3026\t91\t0\tstruct
TEST_B29_B23_C017_C002\t3026\t91\t0\tstruct
TEST_C037_A5_2\t3026\t7\t0\tbcd_t
TEST_C037_B14_A12_B28_2\t3033\t17\t0\tstruct
TEST_C036_B07_C003_2\t3033\t7\t0\tbcd_t
TEST_B07_C003_C076_2\t3040\t2\t0\tstring
TEST_B14_A1_2\t3042\t2\t0\tstring
TEST_D0026_C076_2\t3044\t2\t0\tstring
TEST_C006_C050\t3046\t1\t0\tstring
TEST_C049_C018_B28_A1\t3047\t4\t0\tstring
TEST_C007_D0011_A1\t3051\t2\t0\tbcd_t
TEST_B63_D0011_A5\t3053\t2\t0\tbe_t
TEST_C006_E00011\t3055\t1\t0\tbcd_t
TEST_B14_C070_A1\t3056\t1\t0\tstring
TEST_C004_A1\t3057\t1\t0\tstring
TEST_C057_B47_A8\t3058\t21\t0\tstring
TEST_C043_C018_B42\t3079\t16\t0\tstring
TEST_C014_C001_B42\t3095\t8\t0\tstring
.\t3103\t0\t0\tstruct'

	EXEC	 --regress --offsets --keep
		OUTPUT - $'TEST-B74-B55-B36\t0\t2\t0\tbe_t
TEST-B23-C069-B33\t2\t306\t0\tstruct
TEST-C068-B23-C069-C002\t2\t303\t0\tstruct
TEST-D0018-B37-B08\t2\t23\t0\tstruct
TEST-C007-A4\t2\t10\t0\tstring
TEST-C037-A5\t12\t8\t0\tbe_t
TEST-C037-B14-A12-B28\t20\t17\t0\tstruct
TEST-C036-B07-C003\t20\t8\t0\tbe_t
TEST-B07-C003-C076\t28\t2\t0\tstring
TEST-B14-A1\t30\t2\t0\tstring
TEST-B37-B23-B55-C076\t32\t1\t0\tstring
TEST-A13-B59-C069-B42\t33\t2\t0\tbe_t
TEST-B56-C003-A5\t35\t38\t0\tstring
TEST-B12-A13-B59-C040\t73\t2\t0\tbe_t
TEST-D0023-B23-B55-C076\t75\t1\t0\tstring
TEST-C065-B28-B58-A1\t76\t4\t0\tbe_t
TEST-B61-B01-C037-B28\t80\t27\t0\tstruct
TEST-B27-B61-B01-C037\t80\t13\t0\tstring
TEST-B57-B61-B01-C037\t93\t13\t0\tstring
TEST-C078-C055-B26\t106\t1\t0\tstring
TEST-B23-C058-C069-B28\t107\t130\t0\tstruct
TEST-C058-D0024-C069-B28\t107\t80\t0\tstruct
TEST-C037-D0024-B33\t107\t80\t0\tstring
TEST-C058-C069-B28\t187\t50\t0\tstruct
TEST-C036-C069-B28\t187\t250\t5\tstruct
FILLER\t187\t50\t0\tstring
TEST-C065-A7-B28-B58-A1\t437\t4\t0\tbe_t
TEST-B23-D0028-C069-B28\t441\t50\t0\tstruct
TEST-D0030-C069-B28\t441\t50\t0\tstruct
TEST-C036-C069-B28_2\t441\t250\t5\tstruct
FILLER_2\t441\t50\t0\tstring
TEST-B23-B59-B58-A3\t691\t2\t0\tbe_t
TEST-B23-B12-C002-B28\t693\t2653\t0\tstruct
TEST-B31-B12-C002\t693\t1043\t0\tstruct
TEST-B06-A5\t693\t49\t0\tstruct
TEST-C042-B06-A5\t693\t19\t0\tstruct
TEST-C028-E00006-A1\t693\t8\t0\tstring
TEST-B55-B58-B42\t701\t8\t0\tunsigned
TEST-B29-B58-B42\t709\t2\t0\tbe_t
TEST-B06-B58-B42\t711\t2\t0\tbe_t
TEST-C071-B06-A5\t713\t26\t0\tstruct
TEST-C067-A4\t713\t10\t0\tstring
TEST-C067-B58-B42\t723\t8\t0\tbe_t
TEST-B60-B64\t731\t3\t0\tstring
TEST-B06-C076\t734\t3\t0\tstring
TEST-D0026-C076\t737\t2\t0\tstring
TEST-C006-C075-A1\t739\t4\t0\tbe_t
TEST-A2-B32\t743\t1\t0\tstring
TEST-A6-B55-C076-A1\t744\t2\t0\tstring
TEST-B39-A16-C065-B32\t746\t1\t0\tstring
TEST-D0005-A10-C066\t747\t6\t0\tstring
TEST-C007-B13-B11-B32\t753\t1\t0\tstring
TEST-B06-C018-C041-E00003\t754\t356\t0\tstruct
TEST-C006-C066-A5\t754\t281\t0\tstruct
TEST-B42-B28\t754\t141\t0\tstruct
TEST-C052-C066-C076-A1\t754\t1\t0\tstring
TEST-C036-C066-B42-B28\t755\t35\t0\tstruct
TEST-C036-C066-B42\t755\t35\t0\tstring
TEST-B41-C066-B42-B28\t755\t35\t0\tstruct
TEST-B43\t755\t3\t0\tstring
TEST-B50\t758\t3\t0\tstring
TEST-C046-B42\t761\t4\t0\tstring
FILLER_3\t765\t25\t0\tstring
TEST-D0017-C066-B42-B28\t755\t35\t0\tstruct
TEST-D0017-B42-B36\t755\t2\t0\tstring
TEST-D0017-B42-B28\t757\t16\t0\tstruct
TEST-D0017-B42-B52\t757\t10\t0\tstring
TEST-D0017-B42-B53\t767\t6\t0\tstring
FILLER_4\t773\t17\t0\tstring
TEST-B43-B50-B42-B28\t755\t35\t0\tstruct
TEST-C054-B43-B50\t755\t6\t0\tstring
FILLER_5\t761\t29\t0\tstring
TEST-C006-C066-B60-B42-B28\t754\t140\t0\tstruct
TEST-C006-C052-C066-B70\t754\t1\t0\tstring
TEST-C006-C066-B42-B28\t755\t35\t0\tstruct
TEST-C006-B43\t755\t3\t0\tstring
TEST-C006-B50\t758\t3\t0\tstring
TEST-C006-C046-B42\t761\t4\t0\tstring
FILLER_6\t765\t25\t0\tstring
TEST-C006-B41-B42-B28\t755\t35\t0\tstruct
TEST-C006-B42-B41\t755\t10\t0\tstring
FILLER_7\t765\t25\t0\tstring
TEST-C006-D0017-C066-B42-B28\t755\t35\t0\tstruct
TEST-D0017-B42-B36_2\t755\t2\t0\tstring
TEST-C006-D0017-B42-B28\t757\t16\t0\tstruct
FILLER_8\t757\t16\t0\tstring
FILLER_9\t773\t17\t0\tstring
TEST-C006-D0022-B42-B28\t755\t34\t0\tstruct
TEST-C006-C066-D0004-A5\t755\t30\t0\tstring
TEST-D0004-C048-A1\t785\t4\t0\tstring
TEST-C071-C037-A12-A5\t790\t8\t0\tbe_t
TEST-C037-B14-A12-A5\t798\t8\t0\tbe_t
TEST-C066-C003-A5\t806\t8\t0\tbe_t
TEST-C037-A12-A5\t814\t8\t0\tbe_t
TEST-D0027-C003\t822\t4\t0\tstring
TEST-B63-C001-C039\t826\t13\t0\tunsigned
TEST-B34-A1\t839\t2\t0\tstring
TEST-D0020-C073-A1\t841\t4\t2\tstring
TEST-C018-C003-A5\t845\t4\t0\tbe_t
TEST-B06-C060-D0033\t849\t41\t0\tstruct
TEST-C060-C053-A1\t849\t8\t0\tstring
TEST-C060-COMPNT-A1\t857\t8\t0\tstring
TEST-C043-C060-C041-B66\t865\t5\t0\tstruct
TEST-C043-C060-C041-B28\t865\t25\t5\tstruct
TEST-C043-C060-C076-A1\t865\t1\t0\tstring
TEST-C043-C060-B35-A1\t866\t4\t0\tstring
TEST-C060-C006-A5\t890\t8\t0\tstring
TEST-C006-C026-A1\t898\t8\t0\tstring
TEST-C006-C026-B09-A1\t906\t1\t0\tstring
TEST-B48-E00006\t907\t2\t0\tbe_t
TEST-C062-D0033-B75-B42\t909\t2\t0\tbe_t
TEST-C041-C051-B28\t911\t292\t0\tstruct
TEST-C036-D0013\t911\t48\t6\tbe_t
TEST-C036-B54\t959\t40\t5\tbe_t
TEST-C036-B03\t999\t56\t7\tbe_t
TEST-B31-C020-B28\t1055\t30\t0\tstruct
TEST-D0015-A16-A4\t1055\t10\t0\tstring
TEST-B06-B62-A4\t1065\t10\t0\tstring
TEST-B06-B21-A4\t1075\t10\t0\tstring
TEST-B71-A1\t1085\t5\t0\tstring
TEST-B12-E00002-C019\t1090\t14\t0\tstruct
TEST-E00009-A14-D0007-A1\t1090\t2\t0\tstring
TEST-D0034-A14-D0007-A1\t1092\t2\t0\tstring
TEST-E00009-C045\t1094\t3\t0\tstring
TEST-D0034-C045\t1097\t3\t0\tstring
TEST-B08-A5\t1100\t4\t0\tstring
TEST-C061-B32\t1104\t1\t0\tstring
TEST-E00007-D0005-A10-C066\t1105\t5\t0\tstring
TEST-E00008-C011\t1110\t4\t0\tbe_t
TEST-C044-D0019-A1\t1114\t1\t0\tstring
TEST-D0012-C076\t1115\t1\t0\tstring
TEST-D0012-C027-B32\t1116\t1\t0\tstring
TEST-C010-D0013-B54-C002\t1117\t49\t0\tstruct
TEST-C009-D0013-C064\t1117\t8\t0\tbe_t
TEST-F000003-D0013-C064\t1125\t8\t0\tbe_t
TEST-D0014-B54\t1133\t8\t0\tbe_t
TEST-C038-C009-C005-B54\t1141\t4\t0\tbe_t
TEST-E00004-B03\t1145\t8\t0\tbe_t
TEST-C072-C011-B32\t1153\t1\t0\tstring
TEST-E00005-C056-B28\t1154\t13\t0\tstruct
TEST-E00005-C024\t1154\t2\t0\tbe_t
TEST-C056-B42\t1156\t9\t0\tstring
TEST-B31-B65-B28\t1165\t126\t0\tstruct
TEST-A15-B65-B09-A1\t1165\t2\t0\tstring
TEST-D0016-E00012\t1167\t58\t0\tstruct
TEST-B65-D0001-B32\t1167\t1\t0\tstring
TEST-B65-D0019-A1-B28\t1168\t13\t0\tstruct
TEST-B41-A1\t1168\t2\t0\tstring
TEST-D0032-A1\t1170\t2\t0\tstring
TEST-C015-A1\t1172\t3\t0\tstring
TEST-C012-A1\t1175\t4\t0\tstring
TEST-F000005-A1\t1179\t2\t0\tstring
TEST-B44-B32\t1181\t1\t0\tstring
TEST-B65-B22\t1182\t5\t0\tstring
TEST-C076-A10-C001\t1187\t2\t0\tstring
TEST-COMPLNC-C075-A1\t1189\t1\t0\tstring
TEST-C066-B28\t1190\t1\t0\tstring
TEST-B04-F000001-B07\t1191\t2\t0\tstring
TEST-C033-A10-C011\t1193\t1\t0\tstring
TEST-C011-C076-A1\t1194\t2\t0\tstring
TEST-F000006-B65-C008\t1196\t2\t0\tstring
TEST-C047-B40-B74\t1198\t7\t0\tunsigned
TEST-C047-C066-D0006-A5\t1205\t8\t0\tunsigned
TEST-B65-B42-B02-D0021\t1213\t7\t0\tunsigned
TEST-B65-E00002\t1220\t2\t0\tstring
TEST-B65-C021\t1222\t1\t0\tstring
TEST-B24-B65-C030\t1223\t1\t0\tstring
TEST-C074-B65-A1\t1224\t1\t0\tstring
TEST-D0016-F000004\t1225\t36\t0\tstruct
TEST-C060-C026-A1\t1225\t10\t0\tstring
TEST-D0035-C063-A1\t1235\t4\t0\tstring
TEST-B65-D0002-B28\t1239\t5\t0\tstruct
TEST-F000002-B65-D0002\t1239\t1\t0\tstring
TEST-D0032-B65-D0002\t1240\t1\t0\tstring
TEST-C012-B65-D0002\t1241\t1\t0\tstring
TEST-E00001-B65-D0002\t1242\t1\t0\tstring
TEST-F000005-B65-D0002\t1243\t1\t0\tstring
TEST-C060-B73-A1\t1244\t2\t0\tstring
TEST-C060-C016-B03\t1246\t8\t0\tbe_t
TEST-C075-C076\t1254\t4\t0\tstring
TEST-C063-C076\t1258\t4\t0\tstring
TEST-B68-B65-B03\t1262\t8\t0\tbe_t
TEST-B42-A10-OCCURS\t1270\t2\t0\tbe_t
TEST-B05-B12-C002\t1272\t370\t0\tstruct
TEST-C037-D0024-B33_2\t1272\t80\t0\tstring
TEST-C060-D0033-C023\t1352\t150\t0\tstruct
TEST-C060-C031-C023\t1352\t30\t0\tstring
TEST-C060-C076-C023\t1382\t30\t0\tstring
TEST-C011-B28\t1412\t30\t0\tstring
TEST-B02-C076-C023\t1442\t30\t0\tstring
TEST-C013-A10-D0003-C023\t1472\t30\t0\tstring
TEST-C035\t1502\t8\t0\tbe_t
TEST-D0018-D0009-B61-A1\t1510\t2\t0\tbe_t
TEST-C066-B42-B35\t1512\t30\t0\tstring
TEST-D0018-D0009-C023\t1542\t30\t0\tstring
TEST-C066-C003-A5_2\t1572\t8\t0\tbe_t
TEST-D0012-B49-B45\t1580\t8\t0\tbe_t
TEST-C001-B28-C011-B32\t1588\t1\t0\tstring
TEST-B12-C037-A12-C023\t1589\t30\t0\tstring
TEST-B12-B10-A5\t1619\t6\t0\tstring
TEST-B73-C011-A1\t1625\t2\t0\tstring
TEST-B23-B12-C002\t1627\t1240\t0\tstruct
TEST-D0025-C037-B38-B42\t1627\t1\t0\tbe_t
TEST-E00010-C037-A12-A5\t1628\t8\t0\tbe_t
TEST-C037-A12-B58-B42\t1636\t4\t0\tbe_t
TEST-C034-C065-B28-A1\t1640\t6\t0\tstring
TEST-C034-D0029-B28-A1\t1646\t5\t0\tstring
TEST-C065-B28-B09-A1\t1651\t1\t0\tstring
TEST-D0010-A5\t1652\t8\t0\tstring
TEST-C065-A11-C025-A1\t1660\t1\t0\tstring
TEST-A13-B72-B23-B32\t1661\t2\t2\tstring
TEST-C022-C060-B35-B28\t1663\t710\t0\tstruct
TEST-C043-C060-B35-B58\t1663\t5\t5\tbe_t
TEST-C043-C060-B35-B15\t1668\t200\t5\tstring
TEST-C043-C060-B35-B16\t1868\t150\t5\tstring
TEST-C043-C060-B35-B17\t2018\t125\t5\tstring
TEST-C043-C060-B35-B18\t2143\t125\t5\tstring
TEST-C043-C060-B35-B19\t2268\t100\t5\tstring
TEST-C022-C006-C075-B28\t2368\t148\t0\tstruct
TEST-C006-C075-B58\t2368\t4\t0\tbe_t
TEST-C006-C075-C023-B46\t2372\t40\t0\tstring
TEST-C006-C075-C023-B69\t2412\t30\t0\tstring
TEST-C006-C075-C023-D0036\t2442\t25\t0\tstring
TEST-C006-C075-C023-C032\t2467\t25\t0\tstring
TEST-C006-C075-C023-C029\t2492\t20\t0\tstring
TEST-B67-C023\t2512\t45\t0\tstring
TEST-C062-B51-A1-C023\t2557\t30\t0\tstring
TEST-A14-D0007-C023\t2587\t25\t0\tstring
TEST-D0020-C073\t2612\t2\t2\tstring
TEST-C009-B20-B29-A1\t2614\t1\t0\tstring
TEST-B25-A1\t2615\t7\t0\tstring
TEST-B71-A9\t2622\t15\t0\tstring
TEST-D0020-C023-B28\t2637\t330\t2\tstruct
TEST-D0020-C023-B46\t2637\t55\t0\tstring
TEST-D0020-C023-B69\t2692\t110\t0\tstring
TEST-D0008-C009-B02-B42\t2967\t23\t0\tstring
TEST-D0008-C009-C023\t2990\t25\t0\tstring
TEST-B29-B23-C017-B28\t3015\t91\t0\tstruct
TEST-B29-B23-C017-C002\t3015\t91\t0\tstruct
TEST-C037-A5_2\t3015\t8\t0\tbe_t
TEST-C037-B14-A12-B28_2\t3023\t17\t0\tstruct
TEST-C036-B07-C003_2\t3023\t8\t0\tbe_t
TEST-B07-C003-C076_2\t3031\t2\t0\tstring
TEST-B14-A1_2\t3033\t2\t0\tstring
TEST-D0026-C076_2\t3035\t2\t0\tstring
TEST-C006-C050\t3037\t1\t0\tstring
TEST-C049-C018-B28-A1\t3038\t4\t0\tstring
TEST-C007-D0011-A1\t3042\t1\t0\tbe_t
TEST-B63-D0011-A5\t3043\t2\t0\tbe_t
TEST-C006-E00011\t3045\t1\t0\tbe_t
TEST-B14-C070-A1\t3046\t1\t0\tstring
TEST-C004-A1\t3047\t1\t0\tstring
TEST-C057-B47-A8\t3048\t21\t0\tstring
TEST-C043-C018-B42\t3069\t16\t0\tstring
TEST-C014-C001-B42\t3085\t8\t0\tstring
.\t3093\t0\t0\tstruct'

TEST 04 'multiple REDEFINES'

	EXEC	 --regress --offsets
		INPUT - $'       01  TEST-0000-000-001.
      ******************************************************************
      *                                                                *
      * YADAYADAYADAYADAYADAYADAYADAYADAYADAYADAYADAYADAYADAYADAYADAYA *
      *                                                                *
      ******************************************************************
           03  TEST-000-000-002            PIC S9(4)     COMP
                                                       VALUE ZEROES.
           03  TEST-000-0000-003.
               06  TEST-0000-000-0000-0004.
                   09  TEST-00000-000-005.
                       12  TEST-0000-06    PIC X(10)   VALUE SPACES.
                       12  TEST-0000-07    PIC S9(13)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-0000-000-00-008.
                       12  TEST-0000-000-0009
                                           PIC S9(13)     COMP-3
                                                       VALUE ZEROES.
                       12  TEST-000-0000-0010
                                           PIC X(2)    VALUE SPACES.
                       12  TEST-000-29     PIC X(2)    VALUE SPACES.
                   09  TEST-000-000-000-0031
                                           PIC X       VALUE SPACES.
                   09  TEST-00-000-0000-036
                                           PIC S9(3)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-000-0000-37    PIC X(38)   VALUE SPACES.
                   09  TEST-000-00-000-038
                                           PIC S9(3)     COMP
                                                       VALUE ZEROES.
                   09  TEST-00000-000-000-0039
                                           PIC X       VALUE SPACES.
                   09  TEST-0000-000-000-42
                                           PIC S9(5)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-000-000-0000-043.
                       12  TEST-000-000-000-0044
                                           PIC X(13)   VALUE SPACES.
                       12  TEST-000-000-000-0045
                                           PIC X(13)   VALUE SPACES.
                       12  TEST-0000-0000-046
                                           PIC X       VALUE SPACES.
                   09  TEST-000-0000-0000-047.
                       12  TEST-0000-00000-0000-048.
                       15  TEST-0000-00000-049
                                           PIC X(80)   VALUE SPACES.
                       12  TEST-0000-0000-050.
                       15  TEST-0000-0000-051
                           OCCURS 5 TIMES
                           INDEXED BY TEST-0000-0000-052.
                       18  FILLER          PIC X(50) VALUE SPACE.
                   09  TEST-0000-00-000-000-053
                                           PIC S9(5)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-000-00000-0000-054.
                       12  TEST-00000-0000-056.
                       15  TEST-0000-0000-057
                           OCCURS 5 TIMES
                           INDEXED BY TEST-00000-0000-058.
                       18  FILLER          PIC X(50) VALUE SPACE.
               06  TEST-000-000-000-059     PIC S9(3)     COMP-3
                                                       VALUE ZEROES.
           03  TEST-000-000-0000-060.
               06  TEST-000-000-0061.
                   09  TEST-000-062.
                       12  TEST-0000-000-63.
                       15  TEST-0000-000000-64
                                           PIC X(8)    VALUE SPACES.
                       15  TEST-000-000-065
                                           PIC 9(8)    VALUE ZEROES.
                       15  TEST-000-000-066
                                           PIC S9(3)     COMP-3
                                                       VALUE ZEROES.
                       12  TEST-000-000-067
                                           PIC S9(4)     COMP-3
                                                       VALUE ZEROES.
                       12  TEST-0000-000-68.
                       15  TEST-0000-00    PIC X(10)   VALUE SPACES.
                       15  TEST-0000-000-069
                                           PIC S9(13)     COMP-3
                                                       VALUE ZEROES.
                       15  TEST-000-070    PIC X(3)    VALUE SPACES.
                   09  TEST-000-0071       PIC X(3)    VALUE SPACES.
                   09  TEST-00000-0072     PIC X(2)    VALUE SPACES.
                   09  TEST-0000-0000-73   PIC S9(8)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-00-074         PIC X       VALUE SPACES.
                   09  TEST-00-000-0000-75 PIC X(2)    VALUE \'D\'.
                   09  TEST-000-00-0000-076
                                           PIC X       VALUE SPACES.
                   09  TEST-00000-00-0077  PIC X(6)    VALUE SPACES.
                   09  TEST-0000-000-000-078
                                           PIC X       VALUE SPACES.
                   09  TEST-000-0000-0000-000079.
                       12  TEST-0000-0000-80.
                       15  TEST-999-002.
                       18  TEST-0000-0000-0000-81
                                           PIC X       VALUE SPACES.
                       18  TEST-9999-0000-000-004.
                       21  TEST-0000-0000-082
                                           PIC X(35)   VALUE SPACES.
                       18  TEST-000-0000-000-083
                           REDEFINES TEST-9999-0000-000-004.
                       21  TEST-084        PIC X(3).
                       21  TEST-085        PIC X(3).
                       21  TEST-0000-086   PIC X(4).
                       21  FILLER          PIC X(25).
                       18  TEST-00000-0000-000-087
                           REDEFINES TEST-9999-0000-000-004.
                       21  TEST-00000-000-088
                                           PIC X(2).
                       21  TEST-00000-000-089.
                       24  TEST-00000-000-090
                                           PIC X(10).
                       24  TEST-00000-000-091
                                           PIC X(6).
                       21  FILLER          PIC X(17).
                       18  TEST-000-000-000-092
                           REDEFINES TEST-9999-0000-000-004.
                       21  TEST-0000-000-093
                                           PIC X(6).
                       21  FILLER          PIC X(29).
                       15  TEST-0000-0000-000-000-094
                           REDEFINES TEST-999-002.
                       18  TEST-0000-0000-0000-095
                                           PIC X.
                       18  TEST-9999-0000-000-005.
                       21  TEST-0000-096   PIC X(3).
                       21  TEST-0000-097   PIC X(3).
                       21  TEST-0000-0000-098
                                           PIC X(4).
                       21  FILLER          PIC X(25).
                       18  TEST-0000-000-000-099
                           REDEFINES TEST-9999-0000-000-005.
                       21  TEST-0000-000-100
                                           PIC X(10).
                       21  FILLER          PIC X(25).
                       18  TEST-0000-00000-0000-000-101
                           REDEFINES TEST-9999-0000-000-005.
                       21  TEST-00000-000-102
                                           PIC X(2).
                       21  TEST-0000-00000-000-103.
                       24  FILLER          PIC X(16).
                       21  FILLER          PIC X(17).
                       18  TEST-0000-00000-000-104
                           REDEFINES TEST-9999-0000-000-005.
                       21  TEST-0000-0000-0000-105
                                           PIC X(30).
                       21  TEST-00000-000-106
                                           PIC X(4).
                       12  TEST-0000-0000-0-107
                                           PIC S9(13)     COMP-3
                                                       VALUE ZEROES.
                       12  TEST-0000-000-0-108
                                           PIC S9(13)     COMP-3
                                                       VALUE ZEROES.
                       12  TEST-0000-000-109
                                           PIC S9(13)     COMP-3
                                                       VALUE ZEROES.
                       12  TEST-0000-0-110 PIC S9(13)     COMP-3
                                                       VALUE ZEROES.
                       12  TEST-00000-0111 PIC X(4)    VALUE SPACES.
                       12  TEST-000-0000-0112
                                           PIC 9(13)   VALUE ZEROES.
                       12  TEST-00-113     PIC X(2)    VALUE SPACES.
                       12  TEST-00000-000-114
                                           PIC X(2)
                           OCCURS 2 TIMES
                           VALUE SPACES.
                   09  TEST-0000-000-115   PIC S9(8)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-000-0000-00116.
                       12  TEST-0000-000-117
                                           PIC X(8)    VALUE SPACES.
                       12  TEST-0000-00000-118
                                           PIC X(8)    VALUE SPACES.
                       12  TEST-0000-0000-0000-119.
                       15  TEST-0000-0000-0000-120
                           OCCURS 5 TIMES.
                       18  TEST-0000-0000-000-121
                                           PIC X       VALUE SPACES.
                       18  TEST-0000-0000-00-122
                                           PIC X(4)    VALUE SPACES.
                       12  TEST-0000-000-123
                                           PIC X(8)    VALUE SPACES.
                       12  TEST-0000-000-124
                                           PIC X(8)    VALUE SPACES.
                       12  TEST-0000-0000-00-125
                                           PIC X       VALUE SPACES.
                       12  TEST-000-000126 PIC S9(3)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-0000-00000-000-127
                                           PIC S9(3)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-0000-0000-128.
                       12  TEST-0000-00129 PIC S9(17)     COMP-3
                           OCCURS 6 TIMES
                           INDEXED BY TEST-00000-130
                           VALUE ZEROES.
                       12  TEST-0000-131   PIC S9(15)V9(2)  COMP-3
                           OCCURS 5 TIMES
                           INDEXED BY TEST-000-132
                           VALUE ZEROES.
                       12  TEST-0000-133   PIC S9(8)V9(7)  COMP-3
                           OCCURS 7 TIMES
                           INDEXED BY TEST-000-134
                           VALUE ZEROES.
                   09  TEST-000-0000-135.
                       12  TEST-00000-0-136
                                           PIC X(10)   VALUE SPACES.
                       12  TEST-000-00-137 PIC X(10)   VALUE SPACES.
                       12  TEST-000-00-138 PIC X(10)   VALUE SPACES.
                   09  TEST-00-139         PIC X(5)    VALUE SPACES.
                   09  TEST-000-00000-0140.
                       12  TEST-000000-00-0000-141
                                           PIC X(2)    VALUE SPACES.
                       12  TEST-00000-00-0000-142
                                           PIC X(2)    VALUE SPACES.
                       12  TEST-000000-0143
                                           PIC X(3)    VALUE SPACES.
                       12  TEST-00000-0144 PIC X(3)    VALUE SPACES.
                       12  TEST-00-145     PIC X(4)    VALUE SPACES.
                   09  TEST-0000-146       PIC X       VALUE SPACES.
                   09  TEST-000000-00000-00-0146
                                           PIC X(5)    VALUE SPACES.
                   09  TEST-000000-0147    PIC S9(6)V9(2)  COMP-3
                                                       VALUE ZEROES.
                   09  TEST-0000-0000-148  PIC X       VALUE SPACES.
                   09  TEST-00000-0149     PIC X       VALUE SPACES.
                   09  TEST-00000-0000-150 PIC X       VALUE SPACES.
                   09  TEST-0000-00000-000-0151.
                       12  TEST-0000-00000-0152
                                           PIC S9(10)V9(5)  COMP-3
                                                       VALUE ZEROES.
                       12  TEST-0000000-00000-0153
                                           PIC S9(10)V9(5)  COMP-3
                                                       VALUE ZEROES.
                       12  TEST-00000-154  PIC S9(7)V9(3)  COMP-3
                                                       VALUE ZEROES.
                       12  TEST-0000-0000-0000-155
                                           PIC S9(7)V9(2)  COMP-3
                                                       VALUE ZEROES.
                   09  TEST-000000-156     PIC S9(8)V9(7)  COMP-3
                                                       VALUE ZEROES.
                   09  TEST-0000-0000-157  PIC X       VALUE SPACES.
                   09  TEST-000000-0000-158.
                       12  TEST-000000-0159
                                           PIC S9(4)     COMP-3
                                                       VALUE ZEROES.
                       12  TEST-0000-160   PIC X(9)    VALUE SPACES.
                   09  TEST-000-000-161.
                       12  TEST-00-000-00-162
                                           PIC X(2)    VALUE SPACES.
                       12  TEST-00000-000163.
                       15  TEST-000-00000-164
                                           PIC X       VALUE SPACES.
                       15  TEST-000-00000-00-165.
                       18  TEST-00-166     PIC X(2)    VALUE SPACES.
                       18  TEST-0000-167   PIC X(2)    VALUE SPACES.
                       18  TEST-000-168    PIC X(3)    VALUE SPACES.
                       18  TEST-000-169    PIC X(4)    VALUE SPACES.
                       18  TEST-000000-170 PIC X(2)    VALUE SPACES.
                       15  TEST-000-171    PIC X       VALUE SPACES.
                       15  TEST-000-172    PIC X(5)    VALUE SPACES.
                       15  TEST-0000-00-0173
                                           PIC X(2)    VALUE SPACES.
                       15  TEST-0000000-000-174
                                           PIC X       VALUE SPACES.
                       15  TEST-0000-175   PIC X       VALUE SPACES.
                       15  TEST-000-0000000-176
                                           PIC X(2)    VALUE SPACES.
                       15  TEST-0000-00-0177
                                           PIC X       VALUE SPACES.
                       15  TEST-0000-000-178
                                           PIC X(2)    VALUE SPACES.
                       15  TEST-0000000-000-0179
                                           PIC X(2)    VALUE SPACES.
                       15  TEST-0000-000-180
                                           PIC 9(7)    VALUE ZEROES.
                       15  TEST-0000-0000-0000-181
                                           PIC 9(8)    VALUE ZEROES.
                       15  TEST-000-000-000-00182
                                           PIC 9(7)    VALUE ZEROES.
                       15  TEST-000-000183 PIC X(2)    VALUE SPACES.
                       15  TEST-000-0184   PIC X       VALUE SPACES.
                       15  TEST-000-000-185
                                           PIC X       VALUE SPACES.
                       15  TEST-0000-00-186
                                           PIC X       VALUE SPACES.
                       12  TEST-00000-0000187.
                       15  TEST-0000-000-188
                                           PIC X(10)   VALUE SPACES.
                       15  TEST-00000-000-189
                                           PIC X(4)    VALUE SPACES.
                       15  TEST-000-00000-190.
                       18  TEST-0000000-000-00191
                                           PIC X       VALUE SPACES.
                       18  TEST-00000-000-00192
                                           PIC X       VALUE SPACES.
                       18  TEST-0000-000-00193
                                           PIC X       VALUE SPACES.
                       18  TEST-000000-000-00194
                                           PIC X       VALUE SPACES.
                       18  TEST-0000000-000-00195
                                           PIC X       VALUE SPACES.
                       15  TEST-0000-00-196
                                           PIC X(2)    VALUE SPACES.
                       15  TEST-0000-0000-197
                                           PIC S9(11)V9(4)  COMP-3
                                                       VALUE ZEROES.
                       12  TEST-0000-0198  PIC X(4)    VALUE SPACES.
                       12  TEST-0000-0199  PIC X(4)    VALUE SPACES.
                       12  TEST-000-000-200
                                           PIC S9(11)V9(7)  COMP-3
                                                       VALUE ZEROES.
                       12  TEST-000-00-000201
                                           PIC S9(4)     COMP
                                                       VALUE 0.
               06  TEST-000-000-0202.
                   09  TEST-0000-00000-203 PIC X(80)   VALUE SPACES.
                   09  TEST-0000-00000-0204.
                       12  TEST-0000-0000-0205
                                           PIC X(30)   VALUE SPACES.
                       12  TEST-0000-0000-0206
                                           PIC X(30)   VALUE SPACES.
                       12  TEST-0000-207   PIC X(30)   VALUE SPACES.
                       12  TEST-000-0000-0208
                                           PIC X(30)   VALUE SPACES.
                       12  TEST-0000-00-00000-0209
                                           PIC X(30)   VALUE SPACES.
                   09  TEST-0210           PIC S9(10)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-00000-00000-00-211
                                           PIC S9(3)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-0000-000-212   PIC X(30)   VALUE SPACES.
                   09  TEST-00000-00000-0213
                                           PIC X(30)   VALUE SPACES.
                   09  TEST-0000-000-214   PIC S9(13)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-00000-000-215  PIC S9(2)V9(13)  COMP-3
                                                       VALUE ZEROES.
                   09  TEST-0000-000-0000-216
                                           PIC X       VALUE SPACES.
                   09  TEST-000-0000-00-0217
                                           PIC X(30)   VALUE SPACES.
                   09  TEST-000-00-218     PIC X(6)    VALUE SPACES.
                   09  TEST-000-000-219    PIC X(2)    VALUE \'D\'.
               06  TEST-000-000-0227.
                   09  TEST-00000-0000-000-228
                                           PIC S9(2)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-000000-0000-0-229
                                           PIC S9(13)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-0000-00-000-231
                                           PIC S9(6)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-0000-0000-00-233
                                           PIC X(6)    VALUE SPACES.
                   09  TEST-0000-00000-00-234
                                           PIC X(5)    VALUE SPACES.
                   09  TEST-0000-000-00-235
                                           PIC X       VALUE SPACES.
                   09  TEST-0000-236       PIC X(8)    VALUE SPACES.
                   09  TEST-0000-00-000-237
                                           PIC X       VALUE SPACES.
                   09  TEST-00-000-000-238 PIC X
                       OCCURS 2 TIMES
                       VALUE SPACES.
                   09  TEST-0000-0000-000-239.
                       12  TEST-0000-0000-000-240
                                           PIC S9(2)     COMP-3
                           OCCURS 5 TIMES
                           VALUE ZEROES.
                       12  TEST-0000-0000-000-241
                                           PIC X(40)
                           OCCURS 5 TIMES
                           VALUE SPACES.
                       12  TEST-0000-0000-000-242
                                           PIC X(30)
                           OCCURS 5 TIMES
                           VALUE SPACES.
                       12  TEST-0000-0000-000-243
                                           PIC X(25)
                           OCCURS 5 TIMES
                           VALUE SPACES.
                       12  TEST-0000-0000-000-244
                                           PIC X(25)
                           OCCURS 5 TIMES
                           VALUE SPACES.
                       12  TEST-0000-0000-000-245
                                           PIC X(20)
                           OCCURS 5 TIMES
                           VALUE SPACES.
                   09  TEST-0000-0000-0000-246.
                       12  TEST-0000-0000-257
                                           PIC S9(8)     COMP-3
                                                       VALUE ZEROES.
                       12  TEST-0000-0000-0000-248
                                           PIC X(40)   VALUE SPACES.
                       12  TEST-0000-0000-0000-249
                                           PIC X(30)   VALUE SPACES.
                       12  TEST-0000-0000-0000-00250
                                           PIC X(25)   VALUE SPACES.
                       12  TEST-0000-0000-0000-0251
                                           PIC X(25)   VALUE SPACES.
                       12  TEST-0000-0000-0000-0252
                                           PIC X(20)   VALUE SPACES.
                   09  TEST-000-0253       PIC X(45)   VALUE SPACES.
                   09  TEST-0000-000-00-0254
                                           PIC X(30)   VALUE SPACES.
                   09  TEST-00-00000-0255  PIC X(25)   VALUE SPACES.
                   09  TEST-00000-0256     PIC X
                       OCCURS 2 TIMES
                       VALUE SPACES.
                   09  TEST-0000-000-00-256
                                           PIC X       VALUE SPACES.
                   09  TEST-00-257         PIC X(7)    VALUE SPACES.
                   09  TEST-00-258         PIC X(15)   VALUE SPACES.
                   09  TEST-00000-0000-259
                       OCCURS 2 TIMES.
                       12  TEST-00000-0000-260
                                           PIC X(55)   VALUE SPACES.
                       12  TEST-00000-0000-261
                                           PIC X(110)  VALUE SPACES.
                   09  TEST-00000-0000-000-262
                                           PIC X(23)   VALUE SPACES.
                   09  TEST-00000-0000-0263
                                           PIC X(25)   VALUE SPACES.
           03  TEST-0000-000-0000-264.
               06  TEST-0000-000-0000-0265.
                   09  TEST-00000-000-266  PIC S9(2)V9(13)  COMP-3
                                                       VALUE ZEROES.
                   09  TEST-000-00-000-267.
                       12  TEST-999-002.
                       15  TEST-0000-0000-000-268
                                           PIC X       VALUE SPACES.
                       15  TEST-9999-0000-000-004.
                       18  TEST-0000-0000-269
                                           PIC X(35)   VALUE SPACES.
                       15  TEST-000-0000-000-270
                           REDEFINES TEST-9999-0000-000-004.
                       18  TEST-271        PIC X(3).
                       18  TEST-272        PIC X(3).
                       18  TEST-0000-273   PIC X(4).
                       18  FILLER          PIC X(25).
                       15  TEST-00000-0000-000-274
                           REDEFINES TEST-9999-0000-000-004.
                       18  TEST-00000-000-275
                                           PIC X(2).
                       18  TEST-00000-000-276.
                       21  TEST-00000-000-277
                                           PIC X(10).
                       21  TEST-00000-000-278
                                           PIC X(6).
                       18  FILLER          PIC X(17).
                       15  TEST-000-000-000-279
                           REDEFINES TEST-9999-0000-000-004.
                       18  TEST-0000-000-280
                                           PIC X(6).
                       18  FILLER          PIC X(29).
                       12  TEST-000-00-000-000-281
                           REDEFINES TEST-999-002.
                       15  TEST-0000-0000-0000-282
                                           PIC X.
                       15  TEST-999-00-0000-000-003.
                       18  TEST-00-283     PIC X(3).
                       18  TEST-00-284     PIC X(3).
                       18  TEST-00-0000-285
                                           PIC X(4).
                       18  FILLER          PIC X(25).
                       15  TEST-000-00-000-000-286
                           REDEFINES TEST-999-00-0000-000-003.
                       18  FILLER          PIC X(35).
                       15  TEST-000-00-00000-0000-000-287
                           REDEFINES TEST-999-00-0000-000-003.
                       18  TEST-00000-000-288
                                           PIC X(2).
                       18  TEST-000-00-00000-000-289.
                       21  FILLER          PIC X(16).
                       18  FILLER          PIC X(17).
                       12  TEST-000-00-00000-000-290
                           REDEFINES TEST-999-002.
                       15  TEST-000-00-0000-291
                                           PIC X(30).
                       15  TEST-00000-000-292
                                           PIC X(4).
                       15  FILLER          PIC X(2).
                   09  TEST-00000-000-293  PIC X(8)    VALUE SPACES.
                   09  TEST-00000-0000-000-294
                                           PIC S9(3)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-00000-00000-295
                                           PIC S9(3)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-00000-000-296  PIC X(4)    VALUE SPACES.
                   09  TEST-00000-0000-00297
                                           PIC X       VALUE SPACES.
                   09  TEST-00000-000-298  PIC X       VALUE SPACES.
                   09  TEST-00000-0000-00-299
                                           PIC X(10)   VALUE SPACES.
                   09  TEST-00000-0000-00-300
                                           PIC X(10)   VALUE SPACES.
                   09  TEST-000-0000-000301.
                       12  TEST-000-00-0000-302
                                           PIC X       VALUE SPACES.
                       12  TEST-000-000-303
                                           PIC X       VALUE SPACES.
                       12  TEST-000-0000-000-304
                                           PIC X(2)    VALUE SPACES.
                       12  TEST-000-305    PIC 9(5)    VALUE ZEROES.
                       12  TEST-000-000-00-306
                                           PIC X(10)   VALUE SPACES.
                       12  TEST-0000-00000-307
                                           PIC X       VALUE \'N\'.
                   09  TEST-000000-0000-308
                                           PIC S9(2)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-000000-000-309 PIC S9(9)V9(4)  COMP-3
                                                       VALUE ZEROES.
                   09  TEST-0000-0000-310  PIC X       VALUE SPACES.
                   09  TEST-0000-311       PIC X       VALUE SPACES.
                   09  TEST-0000-000-0-312 PIC S9(13)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-0000-000-313   PIC X(10)   VALUE SPACES.
                   09  TEST-0000-000-314   PIC X       VALUE SPACES.
                   09  TEST-000-000-315    PIC X       VALUE SPACES.
                   09  TEST-00000-000-316  PIC S9(8)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-00000-317      PIC X       VALUE \'N\'.
                   09  TEST-00000-318      PIC X       VALUE SPACES.
                   09  TEST-0000-000-0000-0000-319
                                           PIC X(8)    VALUE SPACES.
                   09  TEST-0000000-320    PIC X(2)    VALUE SPACES.
                   09  TEST-000-321        PIC S9(8)V9(5)  COMP-3
                                                       VALUE ZEROES.
                   09  TEST-999-0000-1     PIC X(25)   VALUE SPACES.
                   09  TEST-00-00000-322
                       REDEFINES TEST-999-0000-1.
                       12  TEST-000000-000323
                                           PIC X(2).
                       12  TEST-0000000-000-00324
                                           PIC 9(7)V9(2).
                       12  FILLER          PIC X(14).
                   09  TEST-000-0-325      PIC S9(9)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-000-0000-00000-326.
                       12  TEST-000-327    PIC X(2)    VALUE SPACES.
                       12  TEST-000-328    PIC X(2)    VALUE SPACES.
                       12  TEST-0000-00329 PIC X(2)    VALUE SPACES.
                   09  TEST-000-0000-0000-330.
                       12  TEST-0000-00-331
                                           PIC X(2)    VALUE SPACES.
                       12  TEST-000-000-332
                                           PIC X(2)    VALUE SPACES.
                       12  TEST-000-0333   PIC X       VALUE SPACES.
                       12  TEST-000000-0334
                                           PIC X(3)    VALUE SPACES.
                   09  TEST-0000000-000-335.
                       12  TEST-0000000-336
                                           PIC X       VALUE SPACES.
                       12  TEST-0000-365   PIC X(2)    VALUE SPACES.
                   09  TEST-0000000-00337  PIC X(2)    VALUE SPACES.
                   09  TEST-000000-000000-338
                                           PIC 9(10)   VALUE ZEROES.
                   09  TEST-00000-0000-339 PIC X       VALUE SPACES.
                   09  TEST-000-340        PIC X(5)    VALUE SPACES.
                   09  TEST-000-341        PIC X(5)    VALUE SPACES.
                   09  TEST-000-342        PIC X(5)    VALUE SPACES.
                   09  TEST-0000-000-343   PIC X(8)    VALUE SPACES.
                   09  TEST-00000-0000-345 PIC X(8)    VALUE SPACES.
                   09  TEST-000000-356     PIC X       VALUE SPACES.
                   09  TEST-000-347        PIC X       VALUE SPACES.
                   09  TEST-0000-000-348   PIC X       VALUE SPACES.
                   09  TEST-000-0000-000-349
                                           PIC X(5)    VALUE SPACES.
                   09  TEST-000-000-0350   PIC X(8)    VALUE SPACES.
                   09  TEST-000-000-0351   PIC X(8)    VALUE SPACES.
                   09  TEST-00-0000-352    PIC X(2)    VALUE SPACES.
                   09  TEST-00-00000-0353  PIC X(2)    VALUE SPACES.
                   09  TEST-0000-0000-00-354
                                           PIC X(4)    VALUE SPACES.
                   09  TEST-00000-0000-355 PIC X       VALUE SPACES.
                   09  TEST-0000-00000-0000-356
                                           PIC X       VALUE SPACES.
                   09  TEST-000000-0000-357
                                           PIC X(2)    VALUE SPACES.
                   09  TEST-000-358        PIC X       VALUE SPACES.
                   09  TEST-00-0000-000-359
                                           PIC X(2)    VALUE SPACES.
               06  TEST-00000-000-0000-0360.
                   09  TEST-00000-0000-0361
                                           PIC X(50)   VALUE SPACES.
                   09  TEST-00000-0000-00-362
                                           PIC X(10)   VALUE SPACES.
                   09  TEST-0000-000-000-363
                                           PIC S9(4)     COMP-3
                                                       VALUE ZEROES.
                   09  TEST-0000-000-364   PIC S9(3)V9(12)  COMP-3
                                                       VALUE ZEROES.'
		OUTPUT - $'TEST_000_000_002\t0\t2\t0\tbe_t
TEST_000_0000_003\t2\t306\t0\tstruct
TEST_0000_000_0000_0004\t2\t303\t0\tstruct
TEST_00000_000_005\t2\t23\t0\tstruct
TEST_0000_06\t2\t10\t0\tstring
TEST_0000_07\t12\t7\t0\tbcd_t
TEST_0000_000_00_008\t19\t17\t0\tstruct
TEST_0000_000_0009\t19\t7\t0\tbcd_t
TEST_000_0000_0010\t26\t2\t0\tstring
TEST_000_29\t28\t2\t0\tstring
TEST_000_000_000_0031\t30\t1\t0\tstring
TEST_00_000_0000_036\t31\t2\t0\tbcd_t
TEST_000_0000_37\t33\t38\t0\tstring
TEST_000_00_000_038\t71\t2\t0\tbe_t
TEST_00000_000_000_0039\t73\t1\t0\tstring
TEST_0000_000_000_42\t74\t3\t0\tbcd_t
TEST_000_000_0000_043\t77\t27\t0\tstruct
TEST_000_000_000_0044\t77\t13\t0\tstring
TEST_000_000_000_0045\t90\t13\t0\tstring
TEST_0000_0000_046\t103\t1\t0\tstring
TEST_000_0000_0000_047\t104\t130\t0\tstruct
TEST_0000_00000_0000_048\t104\t80\t0\tstruct
TEST_0000_00000_049\t104\t80\t0\tstring
TEST_0000_0000_050\t184\t50\t0\tstruct
TEST_0000_0000_051\t184\t250\t5\tstruct
FILLER\t184\t50\t0\tstring
TEST_0000_00_000_000_053\t434\t3\t0\tbcd_t
TEST_000_00000_0000_054\t437\t50\t0\tstruct
TEST_00000_0000_056\t437\t50\t0\tstruct
TEST_0000_0000_057\t437\t250\t5\tstruct
FILLER_2\t437\t50\t0\tstring
TEST_000_000_000_059\t687\t2\t0\tbcd_t
TEST_000_000_0000_060\t689\t2653\t0\tstruct
TEST_000_000_0061\t689\t1043\t0\tstruct
TEST_000_062\t689\t49\t0\tstruct
TEST_0000_000_63\t689\t19\t0\tstruct
TEST_0000_000000_64\t689\t8\t0\tstring
TEST_000_000_065\t697\t8\t0\tunsigned
TEST_000_000_066\t705\t2\t0\tbcd_t
TEST_000_000_067\t707\t3\t0\tbcd_t
TEST_0000_000_68\t710\t26\t0\tstruct
TEST_0000_00\t710\t10\t0\tstring
TEST_0000_000_069\t720\t7\t0\tbcd_t
TEST_000_070\t727\t3\t0\tstring
TEST_000_0071\t730\t3\t0\tstring
TEST_00000_0072\t733\t2\t0\tstring
TEST_0000_0000_73\t735\t5\t0\tbcd_t
TEST_00_074\t740\t1\t0\tstring
TEST_00_000_0000_75\t741\t2\t0\tstring
TEST_000_00_0000_076\t743\t1\t0\tstring
TEST_00000_00_0077\t744\t6\t0\tstring
TEST_0000_000_000_078\t750\t1\t0\tstring
TEST_000_0000_0000_000079\t751\t356\t0\tstruct
TEST_0000_0000_80\t751\t281\t0\tstruct
TEST_999_002\t751\t141\t0\tstruct
TEST_0000_0000_0000_81\t751\t1\t0\tstring
TEST_9999_0000_000_004\t752\t35\t0\tstruct
TEST_0000_0000_082\t752\t35\t0\tstring
TEST_000_0000_000_083\t752\t35\t0\tstruct
TEST_084\t752\t3\t0\tstring
TEST_085\t755\t3\t0\tstring
TEST_0000_086\t758\t4\t0\tstring
FILLER_3\t762\t25\t0\tstring
TEST_00000_0000_000_087\t752\t35\t0\tstruct
TEST_00000_000_088\t752\t2\t0\tstring
TEST_00000_000_089\t754\t16\t0\tstruct
TEST_00000_000_090\t754\t10\t0\tstring
TEST_00000_000_091\t764\t6\t0\tstring
FILLER_4\t770\t17\t0\tstring
TEST_000_000_000_092\t752\t35\t0\tstruct
TEST_0000_000_093\t752\t6\t0\tstring
FILLER_5\t758\t29\t0\tstring
TEST_0000_0000_000_000_094\t751\t140\t0\tstruct
TEST_0000_0000_0000_095\t751\t1\t0\tstring
TEST_9999_0000_000_005\t752\t35\t0\tstruct
TEST_0000_096\t752\t3\t0\tstring
TEST_0000_097\t755\t3\t0\tstring
TEST_0000_0000_098\t758\t4\t0\tstring
FILLER_6\t762\t25\t0\tstring
TEST_0000_000_000_099\t752\t35\t0\tstruct
TEST_0000_000_100\t752\t10\t0\tstring
FILLER_7\t762\t25\t0\tstring
TEST_0000_00000_0000_000_101\t752\t35\t0\tstruct
TEST_00000_000_102\t752\t2\t0\tstring
TEST_0000_00000_000_103\t754\t16\t0\tstruct
FILLER_8\t754\t16\t0\tstring
FILLER_9\t770\t17\t0\tstring
TEST_0000_00000_000_104\t752\t34\t0\tstruct
TEST_0000_0000_0000_105\t752\t30\t0\tstring
TEST_00000_000_106\t782\t4\t0\tstring
TEST_0000_0000_0_107\t787\t7\t0\tbcd_t
TEST_0000_000_0_108\t794\t7\t0\tbcd_t
TEST_0000_000_109\t801\t7\t0\tbcd_t
TEST_0000_0_110\t808\t7\t0\tbcd_t
TEST_00000_0111\t815\t4\t0\tstring
TEST_000_0000_0112\t819\t13\t0\tunsigned
TEST_00_113\t832\t2\t0\tstring
TEST_00000_000_114\t834\t4\t2\tstring
TEST_0000_000_115\t838\t5\t0\tbcd_t
TEST_000_0000_00116\t843\t41\t0\tstruct
TEST_0000_000_117\t843\t8\t0\tstring
TEST_0000_00000_118\t851\t8\t0\tstring
TEST_0000_0000_0000_119\t859\t5\t0\tstruct
TEST_0000_0000_0000_120\t859\t25\t5\tstruct
TEST_0000_0000_000_121\t859\t1\t0\tstring
TEST_0000_0000_00_122\t860\t4\t0\tstring
TEST_0000_000_123\t884\t8\t0\tstring
TEST_0000_000_124\t892\t8\t0\tstring
TEST_0000_0000_00_125\t900\t1\t0\tstring
TEST_000_000126\t901\t2\t0\tbcd_t
TEST_0000_00000_000_127\t903\t2\t0\tbcd_t
TEST_0000_0000_128\t905\t292\t0\tstruct
TEST_0000_00129\t905\t54\t6\tbcd_t
TEST_0000_131\t959\t45\t5\tbcd_t
TEST_0000_133\t1004\t56\t7\tbcd_t
TEST_000_0000_135\t1060\t30\t0\tstruct
TEST_00000_0_136\t1060\t10\t0\tstring
TEST_000_00_137\t1070\t10\t0\tstring
TEST_000_00_138\t1080\t10\t0\tstring
TEST_00_139\t1090\t5\t0\tstring
TEST_000_00000_0140\t1095\t14\t0\tstruct
TEST_000000_00_0000_141\t1095\t2\t0\tstring
TEST_00000_00_0000_142\t1097\t2\t0\tstring
TEST_000000_0143\t1099\t3\t0\tstring
TEST_00000_0144\t1102\t3\t0\tstring
TEST_00_145\t1105\t4\t0\tstring
TEST_0000_146\t1109\t1\t0\tstring
TEST_000000_00000_00_0146\t1110\t5\t0\tstring
TEST_000000_0147\t1115\t5\t0\tbcd_t
TEST_0000_0000_148\t1120\t1\t0\tstring
TEST_00000_0149\t1121\t1\t0\tstring
TEST_00000_0000_150\t1122\t1\t0\tstring
TEST_0000_00000_000_0151\t1123\t49\t0\tstruct
TEST_0000_00000_0152\t1123\t8\t0\tbcd_t
TEST_0000000_00000_0153\t1131\t8\t0\tbcd_t
TEST_00000_154\t1139\t6\t0\tbcd_t
TEST_0000_0000_0000_155\t1145\t5\t0\tbcd_t
TEST_000000_156\t1150\t8\t0\tbcd_t
TEST_0000_0000_157\t1158\t1\t0\tstring
TEST_000000_0000_158\t1159\t13\t0\tstruct
TEST_000000_0159\t1159\t3\t0\tbcd_t
TEST_0000_160\t1162\t9\t0\tstring
TEST_000_000_161\t1171\t126\t0\tstruct
TEST_00_000_00_162\t1171\t2\t0\tstring
TEST_00000_000163\t1173\t58\t0\tstruct
TEST_000_00000_164\t1173\t1\t0\tstring
TEST_000_00000_00_165\t1174\t13\t0\tstruct
TEST_00_166\t1174\t2\t0\tstring
TEST_0000_167\t1176\t2\t0\tstring
TEST_000_168\t1178\t3\t0\tstring
TEST_000_169\t1181\t4\t0\tstring
TEST_000000_170\t1185\t2\t0\tstring
TEST_000_171\t1187\t1\t0\tstring
TEST_000_172\t1188\t5\t0\tstring
TEST_0000_00_0173\t1193\t2\t0\tstring
TEST_0000000_000_174\t1195\t1\t0\tstring
TEST_0000_175\t1196\t1\t0\tstring
TEST_000_0000000_176\t1197\t2\t0\tstring
TEST_0000_00_0177\t1199\t1\t0\tstring
TEST_0000_000_178\t1200\t2\t0\tstring
TEST_0000000_000_0179\t1202\t2\t0\tstring
TEST_0000_000_180\t1204\t7\t0\tunsigned
TEST_0000_0000_0000_181\t1211\t8\t0\tunsigned
TEST_000_000_000_00182\t1219\t7\t0\tunsigned
TEST_000_000183\t1226\t2\t0\tstring
TEST_000_0184\t1228\t1\t0\tstring
TEST_000_000_185\t1229\t1\t0\tstring
TEST_0000_00_186\t1230\t1\t0\tstring
TEST_00000_0000187\t1231\t36\t0\tstruct
TEST_0000_000_188\t1231\t10\t0\tstring
TEST_00000_000_189\t1241\t4\t0\tstring
TEST_000_00000_190\t1245\t5\t0\tstruct
TEST_0000000_000_00191\t1245\t1\t0\tstring
TEST_00000_000_00192\t1246\t1\t0\tstring
TEST_0000_000_00193\t1247\t1\t0\tstring
TEST_000000_000_00194\t1248\t1\t0\tstring
TEST_0000000_000_00195\t1249\t1\t0\tstring
TEST_0000_00_196\t1250\t2\t0\tstring
TEST_0000_0000_197\t1252\t8\t0\tbcd_t
TEST_0000_0198\t1260\t4\t0\tstring
TEST_0000_0199\t1264\t4\t0\tstring
TEST_000_000_200\t1268\t10\t0\tbcd_t
TEST_000_00_000201\t1278\t2\t0\tbe_t
TEST_000_000_0202\t1280\t370\t0\tstruct
TEST_0000_00000_203\t1280\t80\t0\tstring
TEST_0000_00000_0204\t1360\t150\t0\tstruct
TEST_0000_0000_0205\t1360\t30\t0\tstring
TEST_0000_0000_0206\t1390\t30\t0\tstring
TEST_0000_207\t1420\t30\t0\tstring
TEST_000_0000_0208\t1450\t30\t0\tstring
TEST_0000_00_00000_0209\t1480\t30\t0\tstring
TEST_0210\t1510\t6\t0\tbcd_t
TEST_00000_00000_00_211\t1516\t2\t0\tbcd_t
TEST_0000_000_212\t1518\t30\t0\tstring
TEST_00000_00000_0213\t1548\t30\t0\tstring
TEST_0000_000_214\t1578\t7\t0\tbcd_t
TEST_00000_000_215\t1585\t8\t0\tbcd_t
TEST_0000_000_0000_216\t1593\t1\t0\tstring
TEST_000_0000_00_0217\t1594\t30\t0\tstring
TEST_000_00_218\t1624\t6\t0\tstring
TEST_000_000_219\t1630\t2\t0\tstring
TEST_000_000_0227\t1632\t1240\t0\tstruct
TEST_00000_0000_000_228\t1632\t2\t0\tbcd_t
TEST_000000_0000_0_229\t1634\t7\t0\tbcd_t
TEST_0000_00_000_231\t1641\t4\t0\tbcd_t
TEST_0000_0000_00_233\t1645\t6\t0\tstring
TEST_0000_00000_00_234\t1651\t5\t0\tstring
TEST_0000_000_00_235\t1656\t1\t0\tstring
TEST_0000_236\t1657\t8\t0\tstring
TEST_0000_00_000_237\t1665\t1\t0\tstring
TEST_00_000_000_238\t1666\t2\t2\tstring
TEST_0000_0000_000_239\t1668\t710\t0\tstruct
TEST_0000_0000_000_240\t1668\t10\t5\tbcd_t
TEST_0000_0000_000_241\t1678\t200\t5\tstring
TEST_0000_0000_000_242\t1878\t150\t5\tstring
TEST_0000_0000_000_243\t2028\t125\t5\tstring
TEST_0000_0000_000_244\t2153\t125\t5\tstring
TEST_0000_0000_000_245\t2278\t100\t5\tstring
TEST_0000_0000_0000_246\t2378\t148\t0\tstruct
TEST_0000_0000_257\t2378\t5\t0\tbcd_t
TEST_0000_0000_0000_248\t2383\t40\t0\tstring
TEST_0000_0000_0000_249\t2423\t30\t0\tstring
TEST_0000_0000_0000_00250\t2453\t25\t0\tstring
TEST_0000_0000_0000_0251\t2478\t25\t0\tstring
TEST_0000_0000_0000_0252\t2503\t20\t0\tstring
TEST_000_0253\t2523\t45\t0\tstring
TEST_0000_000_00_0254\t2568\t30\t0\tstring
TEST_00_00000_0255\t2598\t25\t0\tstring
TEST_00000_0256\t2623\t2\t2\tstring
TEST_0000_000_00_256\t2625\t1\t0\tstring
TEST_00_257\t2626\t7\t0\tstring
TEST_00_258\t2633\t15\t0\tstring
TEST_00000_0000_259\t2648\t330\t2\tstruct
TEST_00000_0000_260\t2648\t55\t0\tstring
TEST_00000_0000_261\t2703\t110\t0\tstring
TEST_00000_0000_000_262\t2978\t23\t0\tstring
TEST_00000_0000_0263\t3001\t25\t0\tstring
TEST_0000_000_0000_264\t3026\t671\t0\tstruct
TEST_0000_000_0000_0265\t3026\t592\t0\tstruct
TEST_00000_000_266\t3026\t8\t0\tbcd_t
TEST_000_00_000_267\t3034\t283\t0\tstruct
TEST_999_002_2\t3034\t141\t0\tstruct
TEST_0000_0000_000_268\t3034\t1\t0\tstring
TEST_9999_0000_000_004_2\t3035\t35\t0\tstruct
TEST_0000_0000_269\t3035\t35\t0\tstring
TEST_000_0000_000_270\t3035\t35\t0\tstruct
TEST_271\t3035\t3\t0\tstring
TEST_272\t3038\t3\t0\tstring
TEST_0000_273\t3041\t4\t0\tstring
FILLER_10\t3045\t25\t0\tstring
TEST_00000_0000_000_274\t3035\t35\t0\tstruct
TEST_00000_000_275\t3035\t2\t0\tstring
TEST_00000_000_276\t3037\t16\t0\tstruct
TEST_00000_000_277\t3037\t10\t0\tstring
TEST_00000_000_278\t3047\t6\t0\tstring
FILLER_11\t3053\t17\t0\tstring
TEST_000_000_000_279\t3035\t35\t0\tstruct
TEST_0000_000_280\t3035\t6\t0\tstring
FILLER_12\t3041\t29\t0\tstring
TEST_000_00_000_000_281\t3034\t106\t0\tstruct
TEST_0000_0000_0000_282\t3034\t1\t0\tstring
TEST_999_00_0000_000_003\t3035\t35\t0\tstruct
TEST_00_283\t3035\t3\t0\tstring
TEST_00_284\t3038\t3\t0\tstring
TEST_00_0000_285\t3041\t4\t0\tstring
FILLER_13\t3045\t25\t0\tstring
TEST_000_00_000_000_286\t3035\t35\t0\tstruct
FILLER_14\t3035\t35\t0\tstring
TEST_000_00_00000_0000_000_287\t3035\t35\t0\tstruct
TEST_00000_000_288\t3035\t2\t0\tstring
TEST_000_00_00000_000_289\t3037\t16\t0\tstruct
FILLER_15\t3037\t16\t0\tstring
FILLER_16\t3053\t17\t0\tstring
TEST_000_00_00000_000_290\t3034\t36\t0\tstruct
TEST_000_00_0000_291\t3034\t30\t0\tstring
TEST_00000_000_292\t3064\t4\t0\tstring
FILLER_17\t3068\t2\t0\tstring
TEST_00000_000_293\t3070\t8\t0\tstring
TEST_00000_0000_000_294\t3078\t2\t0\tbcd_t
TEST_00000_00000_295\t3080\t2\t0\tbcd_t
TEST_00000_000_296\t3082\t4\t0\tstring
TEST_00000_0000_00297\t3086\t1\t0\tstring
TEST_00000_000_298\t3087\t1\t0\tstring
TEST_00000_0000_00_299\t3088\t10\t0\tstring
TEST_00000_0000_00_300\t3098\t10\t0\tstring
TEST_000_0000_000301\t3108\t20\t0\tstruct
TEST_000_00_0000_302\t3108\t1\t0\tstring
TEST_000_000_303\t3109\t1\t0\tstring
TEST_000_0000_000_304\t3110\t2\t0\tstring
TEST_000_305\t3112\t5\t0\tunsigned
TEST_000_000_00_306\t3117\t10\t0\tstring
TEST_0000_00000_307\t3127\t1\t0\tstring
TEST_000000_0000_308\t3128\t2\t0\tbcd_t
TEST_000000_000_309\t3130\t7\t0\tbcd_t
TEST_0000_0000_310\t3137\t1\t0\tstring
TEST_0000_311\t3138\t1\t0\tstring
TEST_0000_000_0_312\t3139\t7\t0\tbcd_t
TEST_0000_000_313\t3146\t10\t0\tstring
TEST_0000_000_314\t3156\t1\t0\tstring
TEST_000_000_315\t3157\t1\t0\tstring
TEST_00000_000_316\t3158\t5\t0\tbcd_t
TEST_00000_317\t3163\t1\t0\tstring
TEST_00000_318\t3164\t1\t0\tstring
TEST_0000_000_0000_0000_319\t3165\t8\t0\tstring
TEST_0000000_320\t3173\t2\t0\tstring
TEST_000_321\t3175\t7\t0\tbcd_t
TEST_999_0000_1\t3182\t25\t0\tstring
TEST_00_00000_322\t3182\t25\t0\tstruct
TEST_000000_000323\t3182\t2\t0\tstring
TEST_0000000_000_00324\t3184\t10\t0\tunsigned
FILLER_18\t3194\t14\t0\tstring
TEST_000_0_325\t3208\t5\t0\tbcd_t
TEST_000_0000_00000_326\t3213\t6\t0\tstruct
TEST_000_327\t3213\t2\t0\tstring
TEST_000_328\t3215\t2\t0\tstring
TEST_0000_00329\t3217\t2\t0\tstring
TEST_000_0000_0000_330\t3219\t8\t0\tstruct
TEST_0000_00_331\t3219\t2\t0\tstring
TEST_000_000_332\t3221\t2\t0\tstring
TEST_000_0333\t3223\t1\t0\tstring
TEST_000000_0334\t3224\t3\t0\tstring
TEST_0000000_000_335\t3227\t3\t0\tstruct
TEST_0000000_336\t3227\t1\t0\tstring
TEST_0000_365\t3228\t2\t0\tstring
TEST_0000000_00337\t3230\t2\t0\tstring
TEST_000000_000000_338\t3232\t10\t0\tunsigned
TEST_00000_0000_339\t3242\t1\t0\tstring
TEST_000_340\t3243\t5\t0\tstring
TEST_000_341\t3248\t5\t0\tstring
TEST_000_342\t3253\t5\t0\tstring
TEST_0000_000_343\t3258\t8\t0\tstring
TEST_00000_0000_345\t3266\t8\t0\tstring
TEST_000000_356\t3274\t1\t0\tstring
TEST_000_347\t3275\t1\t0\tstring
TEST_0000_000_348\t3276\t1\t0\tstring
TEST_000_0000_000_349\t3277\t5\t0\tstring
TEST_000_000_0350\t3282\t8\t0\tstring
TEST_000_000_0351\t3290\t8\t0\tstring
TEST_00_0000_352\t3298\t2\t0\tstring
TEST_00_00000_0353\t3300\t2\t0\tstring
TEST_0000_0000_00_354\t3302\t4\t0\tstring
TEST_00000_0000_355\t3306\t1\t0\tstring
TEST_0000_00000_0000_356\t3307\t1\t0\tstring
TEST_000000_0000_357\t3308\t2\t0\tstring
TEST_000_358\t3310\t1\t0\tstring
TEST_00_0000_000_359\t3311\t2\t0\tstring
TEST_00000_000_0000_0360\t3313\t79\t0\tstruct
TEST_00000_0000_0361\t3313\t50\t0\tstring
TEST_00000_0000_00_362\t3363\t10\t0\tstring
TEST_0000_000_000_363\t3373\t3\t0\tbcd_t
TEST_0000_000_364\t3376\t8\t0\tbcd_t
.\t3384\t0\t0\tstruct'

TEST 05 'SYNC alignment'

	EXEC	 --regress --offsets
		INPUT - $'       01 TEST.
          05  FLAG PIC X.
          05  STAT PIC 9(8) COMP SYNC.
          05  CHAR PIC X.
          05  NUMB PIC 9(2) COMP SYNC.
          05  LAST PIC X.'
		OUTPUT - $'FLAG\t0\t1\t0\tstring
STAT\t4\t4\t0\tbe_t
CHAR\t8\t1\t0\tstring
NUMB\t10\t1\t0\tbe_t
LAST\t11\t1\t0\tstring
.\t12\t0\t0\tstruct'

	EXEC	 --regress
		OUTPUT - $'<METHOD>flat</>
<FLAT>
 <NAME>TEST</>
 <DESCRIPTION>converted by cpy2dss from copybook standard input</>
 <IDENT>@(#)$Id: TEST (AT&T Research) 2005-07-17 $</>
 <LIBRARY>num_t</>
 <FIELD>
  <NAME>FLAG</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>STAT</>
  <TYPE>unsigned number</>
  <PHYSICAL>
   <TYPE>be_t</>
   <ALIGN>4</>
   <WIDTH>4</>
  </>
 </>
 <FIELD>
  <NAME>CHAR</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>NUMB</>
  <TYPE>unsigned number</>
  <PHYSICAL>
   <TYPE>be_t</>
   <ALIGN>2</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>LAST</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
</>'

TEST 06 'variations on a PIC'

	EXEC	 --regress
		INPUT - $'       01  WS-RECURING-REC.                                             000'\
$'40920
           03  WS-RECURING-TEN-BYTES          PIC 9(10).                0004'\
$'0930
           03  WS-RECURING-TWO-BYTES          PIC X(02)   VALUE \'01\'.   00'\
$'040940
           03  WS-RECURING-DTL-ID             PIC X       VALUE \'5\'.    00'\
$'040950
           03  WS-RECURING-REC-TYPE-CD        PIC X(2)    VALUE \'29\'.   00'\
$'040960
           03  WS-RECURING-HIER-KEY-GRP .                               0004'\
$'0970
               05  WS-RECURING-HIER-ID    PIC S9(13)  COMP-3            0004'\
$'0980
                                                      VALUE ZEROES.     0004'\
$'0990
               05  WS-RECURING-HIER-PT-ID PIC S9(13)  COMP-3            0004'\
$'1000
                                                      VALUE ZEROES.     0004'\
$'1010
               05  WS-RECURING-HIER-LVL-NBR                             0004'\
$'1020
                                          PIC S9(2)   COMP-3            0004'\
$'1030
                                                      VALUE ZEROES.     0004'\
$'1040
               05  WS-RECURING-HIER-PRORD-KEY                           0004'\
$'1050
                                          PIC X(80)   VALUE SPACES.     0004'\
$'1060
               05  WS-RECURING-HIER-PT-SEQ-NBR                          0004'\
$'1070
                                          PIC S9(6)   COMP-3            0004'\
$'1080
                                                      VALUE ZEROES.     0004'\
$'1090
               05  WS-RECURING-BUS-ARGT   PIC S9(13)  COMP-3            0004'\
$'1100
                                                      VALUE ZEROES.     0004'\
$'1110
           03  WS-RECURING-REC-GRP.                                     0004'\
$'1120
                   10  WS-RECURING-SVCARRID   PIC X(55)   VALUE SPACES. 0004'\
$'1130
                   10  WS-RECURING-PFAMFK     PIC 9(04)   VALUE ZEROES. 0004'\
$'1140
                   10  WS-RECURING-PTYPFK     PIC 9(04)   VALUE ZEROES. 0004'\
$'1150
                   10  WS-RECURING-CHGGFK     PIC 9(04)   VALUE ZEROES. 0004'\
$'1160
                   10  WS-RECURING-TRANFK     PIC 9(08)   VALUE ZEROES. 0004'\
$'1170
                   10  WS-RECURING-CHGDESC    PIC X(30)   VALUE SPACES. 0004'\
$'1180
                   10  WS-RECURING-QUANTITY  PIC 9(11).999 VALUE ZEROES.0004'\
$'1190
                   10  WS-RECURING-UNITPRICE  PIC -9(8).99 VALUE ZEROES.0004'\
$'1200
                   10  WS-RECURING-REGELIGCHG PIC -9(8).99 VALUE ZEROES.0004'\
$'1210
                   10  WS-RECURING-PREDSCCHG  PIC -9(8).99 VALUE ZEROES.0004'\
$'1220
                   10  WS-RECURING-DSCAMOUNT  PIC -9(8).99 VALUE ZEROES.0004'\
$'1230
                   10  WS-RECURING-TAXAMOUNT  PIC -9(8).99 VALUE ZEROES.0004'\
$'1240
                   10  WS-RECURING-FROM-DATE  PIC 9(8)    VALUE ZEROES. 0004'\
$'1250
                   10  WS-RECURING-TO-DATE    PIC 9(8)    VALUE ZEROES. 0004'\
$'1260
                   10  WS-RECURING-LEGEND1FK  PIC X(02)   VALUE SPACES. 0004'\
$'1270
                   10  WS-RECURING-LEGEND2FK  PIC X(02)   VALUE SPACES. 0004'\
$'1280
                   10  WS-RECURING-GIRN       PIC 9(10)   VALUE ZEROES. 0004'\
$'1290
                   10 WS-RECURING-WW-CIRCT      PIC X(30).              0004'\
$'1300
                   10 WS-RECURING-EXCH-RATE  PIC -9(3).9(6).            0004'\
$'1310
                   10 WS-RECURING-CURRENCY-NM   PIC X(30).              0004'\
$'1320
                   10 WS-RECURING-ATTCO-1-CITY  PIC X(27).              0004'\
$'1330
                   10 WS-RECURING-ATTCO-1-STATE PIC X(02).              0004'\
$'1340
                   10 WS-RECURING-ATTCO-2-CITY  PIC X(27).              0004'\
$'1350
                   10 WS-RECURING-ATTCO-2-STATE PIC X(02).              0004'\
$'1360
                   10 WS-RECURING-DIST-IN-CHNL  PIC 9(05).              0004'\
$'1370
                   10 WS-RECURING-CUST-NM       PIC X(27).              0004'\
$'1380
                   10 WS-RECURING-INTER-CORP-CD PIC X(01).              0004'\
$'1390
                   10 WS-RECURING-PL-CNTRY-CD-1 PIC X(02).              0004'\
$'1400
                   10 WS-RECURING-PL-CNTRY-CD-2 PIC X(02).              0004'\
$'1410
                   10 WS-RECURING-BIT-STA-DT      PIC X(8)              0004'\
$'1420
                                                          VALUE SPACES. 0004'\
$'1430
                   10 WS-RECURING-CPI-CANC-DT     PIC X(8)              0004'\
$'1440
                                                          VALUE SPACES. 0004'\
$'1450
                   10 WS-RECURING-CONTRACT-NBR    PIC X(8)              0004'\
$'1460
                         VALUE SPACES.                                  0004'\
$'1470
                   10 WS-RECURING-CHRG-CLSS-TEXT  PIC X(45)             0004'\
$'1480
                         VALUE SPACES.                                  0004'\
$'1490
                   10 WS-RECURING-CHRG-CLSS-SORT-CD PIC X(2)            0004'\
$'1500
                         VALUE SPACES.                                  0004'\
$'1510
                   10 WS-RECURING-CHRG-CLSS-DESC   PIC X(45)            0004'\
$'1520
                         VALUE SPACES.                                  0004'\
$'1530
                   10 WS-RECURING-DISC-PLAN-NAME-1 PIC X(50)            0004'\
$'1540
                         VALUE SPACES.                                  0004'\
$'1550
                   10 WS-RECURING-DISC-PCT-OFF-1   PIC 9(3).9(2)        0004'\
$'1560
                         VALUE ZERO.                                    0004'\
$'1570
                   10 WS-RECURING-DISC-PLAN-NAME-2 PIC X(50)            0004'\
$'1580
                         VALUE SPACES.                                  0004'\
$'1590
                   10 WS-RECURING-DISC-PCT-OFF-2   PIC 9(3).9(2)        0004'\
$'1600
                         VALUE ZERO.                                    0004'\
$'1610
                   10 WS-RECURING-DISC-PLAN-NAME-3 PIC X(50)            0004'\
$'1620
                         VALUE SPACES.                                  0004'\
$'1630
                   10 WS-RECURING-DISC-PCT-OFF-3   PIC 9(3).9(2)        0004'\
$'1640
                         VALUE ZERO.                                    0004'\
$'1650
                   10 WS-RECURING-DISC-PLAN-NAME-4 PIC X(50)            0004'\
$'1660
                         VALUE SPACES.                                  0004'\
$'1670
                   10 WS-RECURING-DISC-PCT-OFF-4   PIC 9(3).9(2)        0004'\
$'1680
                         VALUE ZERO.                                    0004'\
$'1690
                   10 WS-RECURING-DISC-PLAN-NAME-5 PIC X(50)            0004'\
$'1700
                         VALUE SPACES.                                  0004'\
$'1710
                   10 WS-RECURING-DISC-PCT-OFF-5   PIC 9(3).9(2)        0004'\
$'1720
                         VALUE ZERO.                                    0004'\
$'1730
                   10 WS-RECURING-DISC-PLAN-NAME-6 PIC X(50)            0004'\
$'1740
                         VALUE SPACES.                                  0004'\
$'1750
                   10 WS-RECURING-DISC-PCT-OFF-6   PIC 9(3).9(2)        0004'\
$'1760
                         VALUE ZERO.                                    0004'\
$'1770
                   10 WS-RECURING-DISC-PLAN-NAME-7 PIC X(50)            0004'\
$'1780
                         VALUE SPACES.                                  0004'\
$'1790
                   10 WS-RECURING-DISC-PCT-OFF-7   PIC 9(3).9(2)        0004'\
$'1800
                         VALUE ZERO.                                    0004'\
$'1810
                   10 WS-RECURING-VPN-NAME-2              PIC X(25)     0004'\
$'1820
                         VALUE SPACES.                                  0004'\
$'1830
                   10 WS-RECURING-UEBILL-VPN-NBR-2        PIC X(9)      0004'\
$'1840
                         VALUE SPACES.                                  0004'\
$'1850
                   10 WS-RECURING-VPN-RGN-CD-1            PIC X(8)      0004'\
$'1860
                         VALUE SPACES.                                  0004'\
$'1870
                   10 WS-RECURING-VPN-RGN-NM-1            PIC X(25)     0004'\
$'1880
                         VALUE SPACES.                                  0004'\
$'1890
                   10 WS-RECURING-VPN-RGN-CD-2            PIC X(8)      0004'\
$'1900
                         VALUE SPACES.                                  0004'\
$'1910
                   10 WS-RECURING-VPN-RGN-NM-2            PIC X(25)     0004'\
$'1920
                         VALUE SPACES.                                  0004'\
$'1930
                   10 WS-RECURING-PLAN-CHNG-CD            PIC X         0004'\
$'1940
                         VALUE SPACES.                                  0004'\
$'1950
                   10 WS-RECURING-CHRG-ALLOC-PC           PIC 9(2).9(4) 0004'\
$'1960
                         VALUE ZERO.                                    0004'\
$'1970
                   10 WS-RECURING-NO-PORTS                PIC 9(4)      0004'\
$'1980
                         VALUE ZERO.                                    0004'\
$'1990
P3301              10  WS-RECURING-RSRVTN-DESC-TX         PIC X(30)     0004'\
$'2000
P3301                                                     VALUE SPACES. 0004'\
$'2010
P3301              10  WS-RECURING-CHRG-TYPE-CD           PIC X(2)      0004'\
$'2020
P3301                                                     VALUE SPACES. 0004'\
$'2030
P2779              10  WS-RECURING-SIID                   PIC X(16)     0004'\
$'2040
P2779                                                     VALUE SPACES. 0004'\
$'2050
P2975              10 WS-RECURING-CKL-NUM       PIC X(04) VALUE SPACES. 0004'\
$'2060
                   10  WS-RECURING-VPN-NAME-1              PIC X(25)    0004'\
$'2070
                                                           VALUE SPACES.0004'\
$'2080
                   10 WS-RECURING-UEBILL-VPN-NBR-1        PIC X(9)      0004'\
$'2090
                         VALUE SPACES.                                  0004'\
$'2100
                   10 WS-RECURING-PLAN-CHNG-MSG-TXT       PIC X(40)     0004'\
$'2110
                         VALUE SPACES.                                  0004'\
$'2120
                   10 FILLER                    PIC X(02).              0004'\
$'2130
      *     R0304 P2780 TML START                                       0004'\
2140
		OUTPUT - $'<METHOD>flat</>
<FLAT>
 <NAME>WS_RECURING_REC</>
 <DESCRIPTION>converted by cpy2dss from copybook standard input</>
 <IDENT>@(#)$Id: WS_RECURING_REC (AT&T Research) 2005-07-17 $</>
 <LIBRARY>num_t</>
 <FIELD>
  <NAME>WS_RECURING_TEN_BYTES</>
  <TYPE>unsigned number</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <FILL>0</>
   <WIDTH>10</>
  </>
 </>
 <FIELD>
  <NAME>WS_RECURING_TWO_BYTES</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>2</>
  </>
 </>
 <FIELD>
  <NAME>WS_RECURING_DTL_ID</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>1</>
  </>
 </>
 <FIELD>
  <NAME>WS_RECURING_REC_TYPE_CD</>
  <TYPE>string</>
  <PHYSICAL>
   <CODESET>ebcdic-m</>
   <WIDTH>2</>
  </>
 </>
 <FIELD>
  <NAME>WS_RECURING_HIER_KEY_GRP</>
  <FIELD>
   <NAME>WS_RECURING_HIER_ID</>
   <TYPE>number</>
   <PHYSICAL>
    <TYPE>bcd_t</>
    <WIDTH>7</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_HIER_PT_ID</>
   <TYPE>number</>
   <PHYSICAL>
    <TYPE>bcd_t</>
    <WIDTH>7</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_HIER_LVL_NBR</>
   <TYPE>number</>
   <PHYSICAL>
    <TYPE>bcd_t</>
    <WIDTH>2</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_HIER_PRORD_KEY</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>80</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_HIER_PT_SEQ_NBR</>
   <TYPE>number</>
   <PHYSICAL>
    <TYPE>bcd_t</>
    <WIDTH>4</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_BUS_ARGT</>
   <TYPE>number</>
   <PHYSICAL>
    <TYPE>bcd_t</>
    <WIDTH>7</>
   </>
  </>
 </>
 <FIELD>
  <NAME>WS_RECURING_REC_GRP</>
  <FIELD>
   <NAME>WS_RECURING_SVCARRID</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>55</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_PFAMFK</>
   <TYPE>unsigned number</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <FILL>0</>
    <WIDTH>4</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_PTYPFK</>
   <TYPE>unsigned number</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <FILL>0</>
    <WIDTH>4</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_CHGGFK</>
   <TYPE>unsigned number</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <FILL>0</>
    <WIDTH>4</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_TRANFK</>
   <TYPE>unsigned number</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <FILL>0</>
    <WIDTH>8</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_CHGDESC</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>30</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_QUANTITY</>
   <TYPE>unsigned number</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <FILL>0</>
    <FIXEDPOINT>3</>
    <WIDTH>15</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_UNITPRICE</>
   <TYPE>number</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <FILL>0</>
    <FIXEDPOINT>2</>
    <WIDTH>12</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_REGELIGCHG</>
   <TYPE>number</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <FILL>0</>
    <FIXEDPOINT>2</>
    <WIDTH>12</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_PREDSCCHG</>
   <TYPE>number</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <FILL>0</>
    <FIXEDPOINT>2</>
    <WIDTH>12</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_DSCAMOUNT</>
   <TYPE>number</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <FILL>0</>
    <FIXEDPOINT>2</>
    <WIDTH>12</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_TAXAMOUNT</>
   <TYPE>number</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <FILL>0</>
    <FIXEDPOINT>2</>
    <WIDTH>12</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_FROM_DATE</>
   <TYPE>unsigned number</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <FILL>0</>
    <WIDTH>8</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_TO_DATE</>
   <TYPE>unsigned number</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <FILL>0</>
    <WIDTH>8</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_LEGEND1FK</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>2</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_LEGEND2FK</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>2</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_GIRN</>
   <TYPE>unsigned number</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <FILL>0</>
    <WIDTH>10</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_WW_CIRCT</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>30</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_EXCH_RATE</>
   <TYPE>number</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <FILL>0</>
    <FIXEDPOINT>6</>
    <WIDTH>11</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_CURRENCY_NM</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>30</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_ATTCO_1_CITY</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>27</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_ATTCO_1_STATE</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>2</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_ATTCO_2_CITY</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>27</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_ATTCO_2_STATE</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>2</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_DIST_IN_CHNL</>
   <TYPE>unsigned number</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <FILL>0</>
    <WIDTH>5</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_CUST_NM</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>27</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_INTER_CORP_CD</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>1</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_PL_CNTRY_CD_1</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>2</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_PL_CNTRY_CD_2</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>2</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_BIT_STA_DT</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>8</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_CPI_CANC_DT</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>8</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_CONTRACT_NBR</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>8</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_CHRG_CLSS_TEXT</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>45</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_CHRG_CLSS_SORT_CD</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>2</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_CHRG_CLSS_DESC</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>45</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_DISC_PLAN_NAME_1</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>50</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_DISC_PCT_OFF_1</>
   <TYPE>unsigned number</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <FILL>0</>
    <FIXEDPOINT>2</>
    <WIDTH>6</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_DISC_PLAN_NAME_2</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>50</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_DISC_PCT_OFF_2</>
   <TYPE>unsigned number</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <FILL>0</>
    <FIXEDPOINT>2</>
    <WIDTH>6</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_DISC_PLAN_NAME_3</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>50</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_DISC_PCT_OFF_3</>
   <TYPE>unsigned number</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <FILL>0</>
    <FIXEDPOINT>2</>
    <WIDTH>6</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_DISC_PLAN_NAME_4</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>50</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_DISC_PCT_OFF_4</>
   <TYPE>unsigned number</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <FILL>0</>
    <FIXEDPOINT>2</>
    <WIDTH>6</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_DISC_PLAN_NAME_5</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>50</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_DISC_PCT_OFF_5</>
   <TYPE>unsigned number</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <FILL>0</>
    <FIXEDPOINT>2</>
    <WIDTH>6</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_DISC_PLAN_NAME_6</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>50</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_DISC_PCT_OFF_6</>
   <TYPE>unsigned number</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <FILL>0</>
    <FIXEDPOINT>2</>
    <WIDTH>6</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_DISC_PLAN_NAME_7</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>50</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_DISC_PCT_OFF_7</>
   <TYPE>unsigned number</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <FILL>0</>
    <FIXEDPOINT>2</>
    <WIDTH>6</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_VPN_NAME_2</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>25</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_UEBILL_VPN_NBR_2</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>9</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_VPN_RGN_CD_1</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>8</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_VPN_RGN_NM_1</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>25</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_VPN_RGN_CD_2</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>8</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_VPN_RGN_NM_2</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>25</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_PLAN_CHNG_CD</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>1</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_CHRG_ALLOC_PC</>
   <TYPE>unsigned number</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <FILL>0</>
    <FIXEDPOINT>4</>
    <WIDTH>7</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_NO_PORTS</>
   <TYPE>unsigned number</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <FILL>0</>
    <WIDTH>4</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_RSRVTN_DESC_TX</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>30</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_CHRG_TYPE_CD</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>2</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_SIID</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>16</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_CKL_NUM</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>4</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_VPN_NAME_1</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>25</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_UEBILL_VPN_NBR_1</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>9</>
   </>
  </>
  <FIELD>
   <NAME>WS_RECURING_PLAN_CHNG_MSG_TXT</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>40</>
   </>
  </>
  <FIELD>
   <NAME>FILLER</>
   <TYPE>string</>
   <PHYSICAL>
    <CODESET>ebcdic-m</>
    <WIDTH>2</>
   </>
  </>
 </>
</>'
