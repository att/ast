# : : generated from /home/gsf/src/cmd/tests/strtof-32.6.37.38-64.15.307.308-64.15.307.308.rt by mktest : : #

TEST 01 'min/max boundaries'

	EXEC	2.225073858507201e-308 2.225073858507202e-308 2.225073858507203e-308
		INPUT -n -
		OUTPUT - $'strtod   "2.225073858507201e-308" "" 0.00000000000000e+00 ERANGE
strtold  "2.225073858507201e-308" "" 0.00000000000000e+00 ERANGE
strntod  22 "2.225073858507201e-308" "" 0.00000000000000e+00 ERANGE
strntod  21 "2.225073858507201e-30" "8" 2.22507385850720e-30 OK
strntold 22 "2.225073858507201e-308" "" 0.00000000000000e+00 ERANGE
strntold 21 "2.225073858507201e-30" "8" 2.22507385850720e-30 OK

strtod   "2.225073858507202e-308" "" 2.22507385850720e-308 OK
strtold  "2.225073858507202e-308" "" 2.22507385850720e-308 OK
strntod  22 "2.225073858507202e-308" "" 2.22507385850720e-308 OK
strntod  21 "2.225073858507202e-30" "8" 2.22507385850720e-30 OK
strntold 22 "2.225073858507202e-308" "" 2.22507385850720e-308 OK
strntold 21 "2.225073858507202e-30" "8" 2.22507385850720e-30 OK

strtod   "2.225073858507203e-308" "" 2.22507385850720e-308 OK
strtold  "2.225073858507203e-308" "" 2.22507385850720e-308 OK
strntod  22 "2.225073858507203e-308" "" 2.22507385850720e-308 OK
strntod  21 "2.225073858507203e-30" "8" 2.22507385850720e-30 OK
strntold 22 "2.225073858507203e-308" "" 2.22507385850720e-308 OK
strntold 21 "2.225073858507203e-30" "8" 2.22507385850720e-30 OK'
		ERROR -n -

	EXEC	1.797693134862314e+308 1.797693134862315e+308 1.797693134862316e+308
		OUTPUT - $'strtod   "1.797693134862314e+308" "" 1.79769313486232e+308 OK
strtold  "1.797693134862314e+308" "" 1.79769313486232e+308 OK
strntod  22 "1.797693134862314e+308" "" 1.79769313486232e+308 OK
strntod  21 "1.797693134862314e+30" "8" 1.79769313486231e+30 OK
strntold 22 "1.797693134862314e+308" "" 1.79769313486232e+308 OK
strntold 21 "1.797693134862314e+30" "8" 1.79769313486231e+30 OK

strtod   "1.797693134862315e+308" "" 1.79769313486232e+308 OK
strtold  "1.797693134862315e+308" "" 1.79769313486232e+308 OK
strntod  22 "1.797693134862315e+308" "" 1.79769313486232e+308 OK
strntod  21 "1.797693134862315e+30" "8" 1.79769313486232e+30 OK
strntold 22 "1.797693134862315e+308" "" 1.79769313486232e+308 OK
strntold 21 "1.797693134862315e+30" "8" 1.79769313486232e+30 OK

strtod   "1.797693134862316e+308" "" inf ERANGE
strtold  "1.797693134862316e+308" "" inf ERANGE
strntod  22 "1.797693134862316e+308" "" inf ERANGE
strntod  21 "1.797693134862316e+30" "8" 1.79769313486232e+30 OK
strntold 22 "1.797693134862316e+308" "" inf ERANGE
strntold 21 "1.797693134862316e+30" "8" 1.79769313486232e+30 OK'

	EXEC	3.362103143112093506e-4932 3.36210314311209350626e-4932 3.362103143112093507e-4932
		OUTPUT - $'strtod   "3.362103143112093506e-4932" "" 0.00000000000000e+00 ERANGE
strtold  "3.362103143112093506e-4932" "" 0.00000000000000e+00 ERANGE
strntod  26 "3.362103143112093506e-4932" "" 0.00000000000000e+00 ERANGE
strntod  25 "3.362103143112093506e-493" "2" 0.00000000000000e+00 ERANGE
strntold 26 "3.362103143112093506e-4932" "" 0.00000000000000e+00 ERANGE
strntold 25 "3.362103143112093506e-493" "2" 0.00000000000000e+00 ERANGE

strtod   "3.36210314311209350626e-4932" "" 0.00000000000000e+00 ERANGE
strtold  "3.36210314311209350626e-4932" "" 0.00000000000000e+00 ERANGE
strntod  28 "3.36210314311209350626e-4932" "" 0.00000000000000e+00 ERANGE
strntod  27 "3.36210314311209350626e-493" "2" 0.00000000000000e+00 ERANGE
strntold 28 "3.36210314311209350626e-4932" "" 0.00000000000000e+00 ERANGE
strntold 27 "3.36210314311209350626e-493" "2" 0.00000000000000e+00 ERANGE

strtod   "3.362103143112093507e-4932" "" 0.00000000000000e+00 ERANGE
strtold  "3.362103143112093507e-4932" "" 0.00000000000000e+00 ERANGE
strntod  26 "3.362103143112093507e-4932" "" 0.00000000000000e+00 ERANGE
strntod  25 "3.362103143112093507e-493" "2" 0.00000000000000e+00 ERANGE
strntold 26 "3.362103143112093507e-4932" "" 0.00000000000000e+00 ERANGE
strntold 25 "3.362103143112093507e-493" "2" 0.00000000000000e+00 ERANGE'

	EXEC	1.189731495357231765e+4932L 1.18973149535723176502e+4932 1.189731495357231766e+4932L
		OUTPUT - $'strtod   "1.189731495357231765e+4932L" "L" inf ERANGE
strtold  "1.189731495357231765e+4932L" "L" inf ERANGE
strntod  27 "1.189731495357231765e+4932L" "L" inf ERANGE
strntod  26 "1.189731495357231765e+4932" "L" inf ERANGE
strntold 27 "1.189731495357231765e+4932L" "L" inf ERANGE
strntold 26 "1.189731495357231765e+4932" "L" inf ERANGE

strtod   "1.18973149535723176502e+4932" "" inf ERANGE
strtold  "1.18973149535723176502e+4932" "" inf ERANGE
strntod  28 "1.18973149535723176502e+4932" "" inf ERANGE
strntod  27 "1.18973149535723176502e+493" "2" inf ERANGE
strntold 28 "1.18973149535723176502e+4932" "" inf ERANGE
strntold 27 "1.18973149535723176502e+493" "2" inf ERANGE

strtod   "1.189731495357231766e+4932L" "L" inf ERANGE
strtold  "1.189731495357231766e+4932L" "L" inf ERANGE
strntod  27 "1.189731495357231766e+4932L" "L" inf ERANGE
strntod  26 "1.189731495357231766e+4932" "L" inf ERANGE
strntold 27 "1.189731495357231766e+4932L" "L" inf ERANGE
strntold 26 "1.189731495357231766e+4932" "L" inf ERANGE'

TEST 02 'optional suffix'

	EXEC	1.1754943E-38F 3.4028234e+38F
		INPUT -n -
		OUTPUT - $'strtod   "1.1754943E-38F" "F" 1.17549430000000e-38 OK
strtold  "1.1754943E-38F" "F" 1.17549430000000e-38 OK
strntod  14 "1.1754943E-38F" "F" 1.17549430000000e-38 OK
strntod  13 "1.1754943E-38" "F" 1.17549430000000e-38 OK
strntold 14 "1.1754943E-38F" "F" 1.17549430000000e-38 OK
strntold 13 "1.1754943E-38" "F" 1.17549430000000e-38 OK

strtod   "3.4028234e+38F" "F" 3.40282340000000e+38 OK
strtold  "3.4028234e+38F" "F" 3.40282340000000e+38 OK
strntod  14 "3.4028234e+38F" "F" 3.40282340000000e+38 OK
strntod  13 "3.4028234e+38" "F" 3.40282340000000e+38 OK
strntold 14 "3.4028234e+38F" "F" 3.40282340000000e+38 OK
strntold 13 "3.4028234e+38" "F" 3.40282340000000e+38 OK'
		ERROR -n -

	EXEC	3.3621031431120935063e-4932L 1.1897314953572317650E+4932L
		OUTPUT - $'strtod   "3.3621031431120935063e-4932L" "L" 0.00000000000000e+00 ERANGE
strtold  "3.3621031431120935063e-4932L" "L" 0.00000000000000e+00 ERANGE
strntod  28 "3.3621031431120935063e-4932L" "L" 0.00000000000000e+00 ERANGE
strntod  27 "3.3621031431120935063e-4932" "L" 0.00000000000000e+00 ERANGE
strntold 28 "3.3621031431120935063e-4932L" "L" 0.00000000000000e+00 ERANGE
strntold 27 "3.3621031431120935063e-4932" "L" 0.00000000000000e+00 ERANGE

strtod   "1.1897314953572317650E+4932L" "L" inf ERANGE
strtold  "1.1897314953572317650E+4932L" "L" inf ERANGE
strntod  28 "1.1897314953572317650E+4932L" "L" inf ERANGE
strntod  27 "1.1897314953572317650E+4932" "L" inf ERANGE
strntold 28 "1.1897314953572317650E+4932L" "L" inf ERANGE
strntold 27 "1.1897314953572317650E+4932" "L" inf ERANGE'

TEST 03 'hexadecimal floating point'

	EXEC	0x1.0000000000000000p-16382 0x1.fffffffffffffffep+16383
		INPUT -n -
		OUTPUT - $'strtod   "0x1.0000000000000000p-16382" "" 0.00000000000000e+00 ERANGE
strtold  "0x1.0000000000000000p-16382" "" 0.00000000000000e+00 ERANGE
strntod  27 "0x1.0000000000000000p-16382" "" 0.00000000000000e+00 ERANGE
strntod  26 "0x1.0000000000000000p-1638" "2" 0.00000000000000e+00 ERANGE
strntold 27 "0x1.0000000000000000p-16382" "" 0.00000000000000e+00 ERANGE
strntold 26 "0x1.0000000000000000p-1638" "2" 0.00000000000000e+00 ERANGE

strtod   "0x1.fffffffffffffffep+16383" "" inf ERANGE
strtold  "0x1.fffffffffffffffep+16383" "" inf ERANGE
strntod  27 "0x1.fffffffffffffffep+16383" "" inf ERANGE
strntod  26 "0x1.fffffffffffffffep+1638" "3" inf ERANGE
strntold 27 "0x1.fffffffffffffffep+16383" "" inf ERANGE
strntold 26 "0x1.fffffffffffffffep+1638" "3" inf ERANGE'
		ERROR -n -

	EXEC	0x1p+16383 -0x1p+16383 0x1p-16382 -0x1p-16382
		OUTPUT - $'strtod   "0x1p+16383" "" inf ERANGE
strtold  "0x1p+16383" "" inf ERANGE
strntod  10 "0x1p+16383" "" inf ERANGE
strntod   9 "0x1p+1638" "3" inf ERANGE
strntold 10 "0x1p+16383" "" inf ERANGE
strntold  9 "0x1p+1638" "3" inf ERANGE

strtod   "-0x1p+16383" "" -inf ERANGE
strtold  "-0x1p+16383" "" -inf ERANGE
strntod  11 "-0x1p+16383" "" -inf ERANGE
strntod  10 "-0x1p+1638" "3" -inf ERANGE
strntold 11 "-0x1p+16383" "" -inf ERANGE
strntold 10 "-0x1p+1638" "3" -inf ERANGE

strtod   "0x1p-16382" "" 0.00000000000000e+00 ERANGE
strtold  "0x1p-16382" "" 0.00000000000000e+00 ERANGE
strntod  10 "0x1p-16382" "" 0.00000000000000e+00 ERANGE
strntod   9 "0x1p-1638" "2" 0.00000000000000e+00 ERANGE
strntold 10 "0x1p-16382" "" 0.00000000000000e+00 ERANGE
strntold  9 "0x1p-1638" "2" 0.00000000000000e+00 ERANGE

strtod   "-0x1p-16382" "" 0.00000000000000e+00 ERANGE
strtold  "-0x1p-16382" "" 0.00000000000000e+00 ERANGE
strntod  11 "-0x1p-16382" "" 0.00000000000000e+00 ERANGE
strntod  10 "-0x1p-1638" "2" -0.00000000000000e+00 ERANGE
strntold 11 "-0x1p-16382" "" 0.00000000000000e+00 ERANGE
strntold 10 "-0x1p-1638" "2" -0.00000000000000e+00 ERANGE'

	EXEC	0x1p+16383 -0x1p+16383 0x1p+16384 -0x1p+16384 0x1p-16382 -0x1p-16382 0x1p-16383 -0x1p-16383
		OUTPUT - $'strtod   "0x1p+16383" "" inf ERANGE
strtold  "0x1p+16383" "" inf ERANGE
strntod  10 "0x1p+16383" "" inf ERANGE
strntod   9 "0x1p+1638" "3" inf ERANGE
strntold 10 "0x1p+16383" "" inf ERANGE
strntold  9 "0x1p+1638" "3" inf ERANGE

strtod   "-0x1p+16383" "" -inf ERANGE
strtold  "-0x1p+16383" "" -inf ERANGE
strntod  11 "-0x1p+16383" "" -inf ERANGE
strntod  10 "-0x1p+1638" "3" -inf ERANGE
strntold 11 "-0x1p+16383" "" -inf ERANGE
strntold 10 "-0x1p+1638" "3" -inf ERANGE

strtod   "0x1p+16384" "" inf ERANGE
strtold  "0x1p+16384" "" inf ERANGE
strntod  10 "0x1p+16384" "" inf ERANGE
strntod   9 "0x1p+1638" "4" inf ERANGE
strntold 10 "0x1p+16384" "" inf ERANGE
strntold  9 "0x1p+1638" "4" inf ERANGE

strtod   "-0x1p+16384" "" -inf ERANGE
strtold  "-0x1p+16384" "" -inf ERANGE
strntod  11 "-0x1p+16384" "" -inf ERANGE
strntod  10 "-0x1p+1638" "4" -inf ERANGE
strntold 11 "-0x1p+16384" "" -inf ERANGE
strntold 10 "-0x1p+1638" "4" -inf ERANGE

strtod   "0x1p-16382" "" 0.00000000000000e+00 ERANGE
strtold  "0x1p-16382" "" 0.00000000000000e+00 ERANGE
strntod  10 "0x1p-16382" "" 0.00000000000000e+00 ERANGE
strntod   9 "0x1p-1638" "2" 0.00000000000000e+00 ERANGE
strntold 10 "0x1p-16382" "" 0.00000000000000e+00 ERANGE
strntold  9 "0x1p-1638" "2" 0.00000000000000e+00 ERANGE

strtod   "-0x1p-16382" "" 0.00000000000000e+00 ERANGE
strtold  "-0x1p-16382" "" 0.00000000000000e+00 ERANGE
strntod  11 "-0x1p-16382" "" 0.00000000000000e+00 ERANGE
strntod  10 "-0x1p-1638" "2" -0.00000000000000e+00 ERANGE
strntold 11 "-0x1p-16382" "" 0.00000000000000e+00 ERANGE
strntold 10 "-0x1p-1638" "2" -0.00000000000000e+00 ERANGE

strtod   "0x1p-16383" "" 0.00000000000000e+00 ERANGE
strtold  "0x1p-16383" "" 0.00000000000000e+00 ERANGE
strntod  10 "0x1p-16383" "" 0.00000000000000e+00 ERANGE
strntod   9 "0x1p-1638" "3" 0.00000000000000e+00 ERANGE
strntold 10 "0x1p-16383" "" 0.00000000000000e+00 ERANGE
strntold  9 "0x1p-1638" "3" 0.00000000000000e+00 ERANGE

strtod   "-0x1p-16383" "" 0.00000000000000e+00 ERANGE
strtold  "-0x1p-16383" "" 0.00000000000000e+00 ERANGE
strntod  11 "-0x1p-16383" "" 0.00000000000000e+00 ERANGE
strntod  10 "-0x1p-1638" "3" -0.00000000000000e+00 ERANGE
strntold 11 "-0x1p-16383" "" 0.00000000000000e+00 ERANGE
strntold 10 "-0x1p-1638" "3" -0.00000000000000e+00 ERANGE'

	EXEC	0x1p127 0x1.p127 0x1.0p127 0x.1p131 0x0.1p131 0x0.10p131
		OUTPUT - $'strtod   "0x1p127" "" 1.70141183460469e+38 OK
strtold  "0x1p127" "" 1.70141183460469e+38 OK
strntod   7 "0x1p127" "" 1.70141183460469e+38 OK
strntod   6 "0x1p12" "7" 4.09600000000000e+03 OK
strntold  7 "0x1p127" "" 1.70141183460469e+38 OK
strntold  6 "0x1p12" "7" 4.09600000000000e+03 OK

strtod   "0x1.p127" "" 1.70141183460469e+38 OK
strtold  "0x1.p127" "" 1.70141183460469e+38 OK
strntod   8 "0x1.p127" "" 1.70141183460469e+38 OK
strntod   7 "0x1.p12" "7" 4.09600000000000e+03 OK
strntold  8 "0x1.p127" "" 1.70141183460469e+38 OK
strntold  7 "0x1.p12" "7" 4.09600000000000e+03 OK

strtod   "0x1.0p127" "" 1.70141183460469e+38 OK
strtold  "0x1.0p127" "" 1.70141183460469e+38 OK
strntod   9 "0x1.0p127" "" 1.70141183460469e+38 OK
strntod   8 "0x1.0p12" "7" 4.09600000000000e+03 OK
strntold  9 "0x1.0p127" "" 1.70141183460469e+38 OK
strntold  8 "0x1.0p12" "7" 4.09600000000000e+03 OK

strtod   "0x.1p131" "" 1.70141183460469e+38 OK
strtold  "0x.1p131" "" 1.70141183460469e+38 OK
strntod   8 "0x.1p131" "" 1.70141183460469e+38 OK
strntod   7 "0x.1p13" "1" 5.12000000000000e+02 OK
strntold  8 "0x.1p131" "" 1.70141183460469e+38 OK
strntold  7 "0x.1p13" "1" 5.12000000000000e+02 OK

strtod   "0x0.1p131" "" 1.70141183460469e+38 OK
strtold  "0x0.1p131" "" 1.70141183460469e+38 OK
strntod   9 "0x0.1p131" "" 1.70141183460469e+38 OK
strntod   8 "0x0.1p13" "1" 5.12000000000000e+02 OK
strntold  9 "0x0.1p131" "" 1.70141183460469e+38 OK
strntold  8 "0x0.1p13" "1" 5.12000000000000e+02 OK

strtod   "0x0.10p131" "" 1.70141183460469e+38 OK
strtold  "0x0.10p131" "" 1.70141183460469e+38 OK
strntod  10 "0x0.10p131" "" 1.70141183460469e+38 OK
strntod   9 "0x0.10p13" "1" 5.12000000000000e+02 OK
strntold 10 "0x0.10p131" "" 1.70141183460469e+38 OK
strntold  9 "0x0.10p13" "1" 5.12000000000000e+02 OK'

	EXEC	0x12345p127 0x12345.6789ap127 1.26866461572665980e+43
		OUTPUT - $'strtod   "0x12345p127" "" 1.26865773447299e+43 OK
strtold  "0x12345p127" "" 1.26865773447299e+43 OK
strntod  11 "0x12345p127" "" 1.26865773447299e+43 OK
strntod  10 "0x12345p12" "7" 3.05418240000000e+08 OK
strntold 11 "0x12345p127" "" 1.26865773447299e+43 OK
strntold 10 "0x12345p12" "7" 3.05418240000000e+08 OK

strtod   "0x12345.6789ap127" "" 1.26866461572666e+43 OK
strtold  "0x12345.6789ap127" "" 1.26866461572666e+43 OK
strntod  17 "0x12345.6789ap127" "" 1.26866461572666e+43 OK
strntod  16 "0x12345.6789ap12" "7" 3.05419896601563e+08 OK
strntold 17 "0x12345.6789ap127" "" 1.26866461572666e+43 OK
strntold 16 "0x12345.6789ap12" "7" 3.05419896601563e+08 OK

strtod   "1.26866461572665980e+43" "" 1.26866461572666e+43 OK
strtold  "1.26866461572665980e+43" "" 1.26866461572666e+43 OK
strntod  23 "1.26866461572665980e+43" "" 1.26866461572666e+43 OK
strntod  22 "1.26866461572665980e+4" "3" 1.26866461572666e+04 OK
strntold 23 "1.26866461572665980e+43" "" 1.26866461572666e+43 OK
strntold 22 "1.26866461572665980e+4" "3" 1.26866461572666e+04 OK'

TEST 04 'to infinity and beyond'

	EXEC	inf +inf -inf
		INPUT -n -
		OUTPUT - $'strtod   "inf" "" inf OK
strtold  "inf" "" inf OK
strntod   3 "inf" "" inf OK
strntod   2 "in" "inf" 0.00000000000000e+00 OK
strntold  3 "inf" "" inf OK
strntold  2 "in" "inf" 0.00000000000000e+00 OK

strtod   "+inf" "" inf OK
strtold  "+inf" "" inf OK
strntod   4 "+inf" "" inf OK
strntod   3 "+in" "+inf" 0.00000000000000e+00 OK
strntold  4 "+inf" "" inf OK
strntold  3 "+in" "+inf" 0.00000000000000e+00 OK

strtod   "-inf" "" -inf OK
strtold  "-inf" "" -inf OK
strntod   4 "-inf" "" -inf OK
strntod   3 "-in" "-inf" 0.00000000000000e+00 OK
strntold  4 "-inf" "" -inf OK
strntold  3 "-in" "-inf" 0.00000000000000e+00 OK'
		ERROR -n -

	EXEC	Inf +Inf -Inf
		OUTPUT - $'strtod   "Inf" "" inf OK
strtold  "Inf" "" inf OK
strntod   3 "Inf" "" inf OK
strntod   2 "In" "Inf" 0.00000000000000e+00 OK
strntold  3 "Inf" "" inf OK
strntold  2 "In" "Inf" 0.00000000000000e+00 OK

strtod   "+Inf" "" inf OK
strtold  "+Inf" "" inf OK
strntod   4 "+Inf" "" inf OK
strntod   3 "+In" "+Inf" 0.00000000000000e+00 OK
strntold  4 "+Inf" "" inf OK
strntold  3 "+In" "+Inf" 0.00000000000000e+00 OK

strtod   "-Inf" "" -inf OK
strtold  "-Inf" "" -inf OK
strntod   4 "-Inf" "" -inf OK
strntod   3 "-In" "-Inf" 0.00000000000000e+00 OK
strntold  4 "-Inf" "" -inf OK
strntold  3 "-In" "-Inf" 0.00000000000000e+00 OK'

	EXEC	InFiNiTy +InFiNiTy -InFiNiTy
		OUTPUT - $'strtod   "InFiNiTy" "" inf OK
strtold  "InFiNiTy" "" inf OK
strntod   8 "InFiNiTy" "" inf OK
strntod   7 "InFiNiT" "iNiTy" inf OK
strntold  8 "InFiNiTy" "" inf OK
strntold  7 "InFiNiT" "iNiTy" inf OK

strtod   "+InFiNiTy" "" inf OK
strtold  "+InFiNiTy" "" inf OK
strntod   9 "+InFiNiTy" "" inf OK
strntod   8 "+InFiNiT" "iNiTy" inf OK
strntold  9 "+InFiNiTy" "" inf OK
strntold  8 "+InFiNiT" "iNiTy" inf OK

strtod   "-InFiNiTy" "" -inf OK
strtold  "-InFiNiTy" "" -inf OK
strntod   9 "-InFiNiTy" "" -inf OK
strntod   8 "-InFiNiT" "iNiTy" -inf OK
strntold  9 "-InFiNiTy" "" -inf OK
strntold  8 "-InFiNiT" "iNiTy" -inf OK'

	EXEC	infi +infi -infi
		OUTPUT - $'strtod   "infi" "i" inf OK
strtold  "infi" "i" inf OK
strntod   4 "infi" "i" inf OK
strntod   3 "inf" "i" inf OK
strntold  4 "infi" "i" inf OK
strntold  3 "inf" "i" inf OK

strtod   "+infi" "i" inf OK
strtold  "+infi" "i" inf OK
strntod   5 "+infi" "i" inf OK
strntod   4 "+inf" "i" inf OK
strntold  5 "+infi" "i" inf OK
strntold  4 "+inf" "i" inf OK

strtod   "-infi" "i" -inf OK
strtold  "-infi" "i" -inf OK
strntod   5 "-infi" "i" -inf OK
strntod   4 "-inf" "i" -inf OK
strntold  5 "-infi" "i" -inf OK
strntold  4 "-inf" "i" -inf OK'

	EXEC	in +in -in
		OUTPUT - $'strtod   "in" "in" 0.00000000000000e+00 OK
strtold  "in" "in" 0.00000000000000e+00 OK
strntod   2 "in" "in" 0.00000000000000e+00 OK
strntod   1 "i" "in" 0.00000000000000e+00 OK
strntold  2 "in" "in" 0.00000000000000e+00 OK
strntold  1 "i" "in" 0.00000000000000e+00 OK

strtod   "+in" "+in" 0.00000000000000e+00 OK
strtold  "+in" "+in" 0.00000000000000e+00 OK
strntod   3 "+in" "+in" 0.00000000000000e+00 OK
strntod   2 "+i" "+in" 0.00000000000000e+00 OK
strntold  3 "+in" "+in" 0.00000000000000e+00 OK
strntold  2 "+i" "+in" 0.00000000000000e+00 OK

strtod   "-in" "-in" 0.00000000000000e+00 OK
strtold  "-in" "-in" 0.00000000000000e+00 OK
strntod   3 "-in" "-in" 0.00000000000000e+00 OK
strntod   2 "-i" "-in" 0.00000000000000e+00 OK
strntold  3 "-in" "-in" 0.00000000000000e+00 OK
strntold  2 "-i" "-in" 0.00000000000000e+00 OK'

	EXEC	NaN +NaN -NaN
		OUTPUT - $'strtod   "NaN" "" nan OK
strtold  "NaN" "" nan OK
strntod   3 "NaN" "" nan OK
strntod   2 "Na" "NaN" 0.00000000000000e+00 OK
strntold  3 "NaN" "" nan OK
strntold  2 "Na" "NaN" 0.00000000000000e+00 OK

strtod   "+NaN" "" nan OK
strtold  "+NaN" "" nan OK
strntod   4 "+NaN" "" nan OK
strntod   3 "+Na" "+NaN" 0.00000000000000e+00 OK
strntold  4 "+NaN" "" nan OK
strntold  3 "+Na" "+NaN" 0.00000000000000e+00 OK

strtod   "-NaN" "" -nan OK
strtold  "-NaN" "" -nan OK
strntod   4 "-NaN" "" -nan OK
strntod   3 "-Na" "-NaN" 0.00000000000000e+00 OK
strntold  4 "-NaN" "" -nan OK
strntold  3 "-Na" "-NaN" 0.00000000000000e+00 OK'

	EXEC	NaN12-34abc.def +NaN12-34abc.def -NaN12-34abc.def
		OUTPUT - $'strtod   "NaN12-34abc.def" "" nan OK
strtold  "NaN12-34abc.def" "" nan OK
strntod  15 "NaN12-34abc.def" "" nan OK
strntod  14 "NaN12-34abc.de" "f" nan OK
strntold 15 "NaN12-34abc.def" "" nan OK
strntold 14 "NaN12-34abc.de" "f" nan OK

strtod   "+NaN12-34abc.def" "" nan OK
strtold  "+NaN12-34abc.def" "" nan OK
strntod  16 "+NaN12-34abc.def" "" nan OK
strntod  15 "+NaN12-34abc.de" "f" nan OK
strntold 16 "+NaN12-34abc.def" "" nan OK
strntold 15 "+NaN12-34abc.de" "f" nan OK

strtod   "-NaN12-34abc.def" "" -nan OK
strtold  "-NaN12-34abc.def" "" -nan OK
strntod  16 "-NaN12-34abc.def" "" -nan OK
strntod  15 "-NaN12-34abc.de" "f" -nan OK
strntold 16 "-NaN12-34abc.def" "" -nan OK
strntold 15 "-NaN12-34abc.de" "f" -nan OK'

	EXEC	0 -0 0. -0. 0.0 -0.0
		OUTPUT - $'strtod   "0" "" 0.00000000000000e+00 OK
strtold  "0" "" 0.00000000000000e+00 OK
strntod   1 "0" "" 0.00000000000000e+00 OK
strntod   0 "" "0" 0.00000000000000e+00 OK
strntold  1 "0" "" 0.00000000000000e+00 OK
strntold  0 "" "0" 0.00000000000000e+00 OK

strtod   "-0" "" -0.00000000000000e+00 OK
strtold  "-0" "" -0.00000000000000e+00 OK
strntod   2 "-0" "" -0.00000000000000e+00 OK
strntod   1 "-" "-0" 0.00000000000000e+00 OK
strntold  2 "-0" "" -0.00000000000000e+00 OK
strntold  1 "-" "-0" 0.00000000000000e+00 OK

strtod   "0." "" 0.00000000000000e+00 OK
strtold  "0." "" 0.00000000000000e+00 OK
strntod   2 "0." "" 0.00000000000000e+00 OK
strntod   1 "0" "." 0.00000000000000e+00 OK
strntold  2 "0." "" 0.00000000000000e+00 OK
strntold  1 "0" "." 0.00000000000000e+00 OK

strtod   "-0." "" -0.00000000000000e+00 OK
strtold  "-0." "" -0.00000000000000e+00 OK
strntod   3 "-0." "" -0.00000000000000e+00 OK
strntod   2 "-0" "." -0.00000000000000e+00 OK
strntold  3 "-0." "" -0.00000000000000e+00 OK
strntold  2 "-0" "." -0.00000000000000e+00 OK

strtod   "0.0" "" 0.00000000000000e+00 OK
strtold  "0.0" "" 0.00000000000000e+00 OK
strntod   3 "0.0" "" 0.00000000000000e+00 OK
strntod   2 "0." "0" 0.00000000000000e+00 OK
strntold  3 "0.0" "" 0.00000000000000e+00 OK
strntold  2 "0." "0" 0.00000000000000e+00 OK

strtod   "-0.0" "" -0.00000000000000e+00 OK
strtold  "-0.0" "" -0.00000000000000e+00 OK
strntod   4 "-0.0" "" -0.00000000000000e+00 OK
strntod   3 "-0." "0" -0.00000000000000e+00 OK
strntold  4 "-0.0" "" -0.00000000000000e+00 OK
strntold  3 "-0." "0" -0.00000000000000e+00 OK'

TEST 05 'simple, right?'

	EXEC	1 12 1.2 1.2.3
		INPUT -n -
		OUTPUT - $'strtod   "1" "" 1.00000000000000e+00 OK
strtold  "1" "" 1.00000000000000e+00 OK
strntod   1 "1" "" 1.00000000000000e+00 OK
strntod   0 "" "1" 0.00000000000000e+00 OK
strntold  1 "1" "" 1.00000000000000e+00 OK
strntold  0 "" "1" 0.00000000000000e+00 OK

strtod   "12" "" 1.20000000000000e+01 OK
strtold  "12" "" 1.20000000000000e+01 OK
strntod   2 "12" "" 1.20000000000000e+01 OK
strntod   1 "1" "2" 1.00000000000000e+00 OK
strntold  2 "12" "" 1.20000000000000e+01 OK
strntold  1 "1" "2" 1.00000000000000e+00 OK

strtod   "1.2" "" 1.20000000000000e+00 OK
strtold  "1.2" "" 1.20000000000000e+00 OK
strntod   3 "1.2" "" 1.20000000000000e+00 OK
strntod   2 "1." "2" 1.00000000000000e+00 OK
strntold  3 "1.2" "" 1.20000000000000e+00 OK
strntold  2 "1." "2" 1.00000000000000e+00 OK

strtod   "1.2.3" ".3" 1.20000000000000e+00 OK
strtold  "1.2.3" ".3" 1.20000000000000e+00 OK
strntod   5 "1.2.3" ".3" 1.20000000000000e+00 OK
strntod   4 "1.2." ".3" 1.20000000000000e+00 OK
strntold  5 "1.2.3" ".3" 1.20000000000000e+00 OK
strntold  4 "1.2." ".3" 1.20000000000000e+00 OK'
		ERROR -n -
