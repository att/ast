# regression tests for the dss text method

TITLE + text

export TZ=EST5EDT

VIEW data

TEST 01 'text method/format'
	pwd='%(name)s:%(passwd:Encrypted password.)s:%(uid)d:%(gid)d:%(comment)s:%(home)s:%(shell)s'
	EXEC -x text:"$pwd" 'passwd==""&&uid==0' $data/pwd-txt.dat
		OUTPUT - $'aha::0:0::/:/bin/sh'
	EXEC -x text:"$pwd" 'passwd==""' $data/pwd-txt.dat
		OUTPUT - $'aha::0:0::/:/bin/sh
he::30002:30002:Hammond Egger:/home/he:/bin/ksh'
	EXEC -x text:"$pwd" 'shell=="/bin/ksh"' $data/pwd-txt.dat
		OUTPUT - $'as:*:30001:30001:Alan Smithee:/home/as:/bin/ksh
he::30002:30002:Hammond Egger:/home/he:/bin/ksh'
	EXEC -c -x text:"$pwd" 'shell=~".*/ksh"' $data/pwd-txt.dat
		OUTPUT - $'2/18'
	EXEC -c -x text:"$pwd" 'shell!~".*/ksh"' $data/pwd-txt.dat
		OUTPUT - $'16/18'
