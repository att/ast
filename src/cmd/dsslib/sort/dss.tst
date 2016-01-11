# tests for the dss sort query

TITLE + sort

export TZ=EST5EDT

VIEW data

TEST 01 'string field basics'

	EXEC -x $data/pwd.dss '{sort}' $data/pwd.dat
		OUTPUT - $'adm:*:5:3:Accounting Files Owner:/var/adm:/bin/sh
aha::-2:0::/:/bin/sh
as:*:30001:30001:Alan Smithee:/home/as:/bin/ksh
auditor:*:11:0:Audit Activity Owner:/auditor:/bin/sh
bin:*:2:2:System Tools Owner:/bin:/dev/null
daemon:*:1:1:daemons:/:/dev/null
demos:*:993:997:Demonstration User:/usr/demos:/bin/csh
diag:*:-3:996:Hardware Diagnostics:/usr/diags:/bin/csh
dup::998:998:oops dup uid gid:/home/he:/bin/ksh
ftp:*:112:112:File Transfer:/home/ftp:/dev/null
guest:*:998:998:Guest Account:/usr/people/guest:/bin/csh
he::30002:30002:Hammond Egger:/home/he:/bin/ksh
he::90002:90002:oops dup name:/home/he:/bin/ksh
jethro::00030001:80001:oops dup uid number different text:/home/he:/bin/ksh
lg:*:30001:10003:Len Gernwolf:/home/lg:/bin/ksh
lp:*:9:9:Print Spooler Owner:/var/spool/lp:/bin/sh
noaccess:*:60002:60002:uid no access:/dev/null:/dev/null
nobody:*:60001:60001:original nobody uid:/dev/null:/dev/null
root:*:0:0:Super-User:/:/bin/csh
sys:*:4:0:System Activity Owner:/var/adm:/bin/sh
sysadm:*:0:9:System V Administration:/usr/admin:/bin/sh
uucp:*:3:5:UUCP Owner:/usr/lib/uucp:/bin/csh'

	EXEC -x $data/pwd.dss '{sort --uniq}' $data/pwd.dat

	EXEC -x $data/pwd.dss '{sort --reverse}' $data/pwd.dat
		OUTPUT - $'uucp:*:3:5:UUCP Owner:/usr/lib/uucp:/bin/csh
sysadm:*:0:9:System V Administration:/usr/admin:/bin/sh
sys:*:4:0:System Activity Owner:/var/adm:/bin/sh
root:*:0:0:Super-User:/:/bin/csh
nobody:*:60001:60001:original nobody uid:/dev/null:/dev/null
noaccess:*:60002:60002:uid no access:/dev/null:/dev/null
lp:*:9:9:Print Spooler Owner:/var/spool/lp:/bin/sh
lg:*:30001:10003:Len Gernwolf:/home/lg:/bin/ksh
jethro::00030001:80001:oops dup uid number different text:/home/he:/bin/ksh
he::90002:90002:oops dup name:/home/he:/bin/ksh
he::30002:30002:Hammond Egger:/home/he:/bin/ksh
guest:*:998:998:Guest Account:/usr/people/guest:/bin/csh
ftp:*:112:112:File Transfer:/home/ftp:/dev/null
dup::998:998:oops dup uid gid:/home/he:/bin/ksh
diag:*:-3:996:Hardware Diagnostics:/usr/diags:/bin/csh
demos:*:993:997:Demonstration User:/usr/demos:/bin/csh
daemon:*:1:1:daemons:/:/dev/null
bin:*:2:2:System Tools Owner:/bin:/dev/null
auditor:*:11:0:Audit Activity Owner:/auditor:/bin/sh
as:*:30001:30001:Alan Smithee:/home/as:/bin/ksh
aha::-2:0::/:/bin/sh
adm:*:5:3:Accounting Files Owner:/var/adm:/bin/sh'

	EXEC -x $data/pwd.dss '{sort --reverse --uniq}' $data/pwd.dat

TEST 02 'numeric field basics'

	EXEC -x $data/pwd.dss '{sort uid}' $data/pwd.dat
		OUTPUT - $'diag:*:-3:996:Hardware Diagnostics:/usr/diags:/bin/csh
aha::-2:0::/:/bin/sh
root:*:0:0:Super-User:/:/bin/csh
sysadm:*:0:9:System V Administration:/usr/admin:/bin/sh
daemon:*:1:1:daemons:/:/dev/null
bin:*:2:2:System Tools Owner:/bin:/dev/null
uucp:*:3:5:UUCP Owner:/usr/lib/uucp:/bin/csh
sys:*:4:0:System Activity Owner:/var/adm:/bin/sh
adm:*:5:3:Accounting Files Owner:/var/adm:/bin/sh
lp:*:9:9:Print Spooler Owner:/var/spool/lp:/bin/sh
auditor:*:11:0:Audit Activity Owner:/auditor:/bin/sh
ftp:*:112:112:File Transfer:/home/ftp:/dev/null
demos:*:993:997:Demonstration User:/usr/demos:/bin/csh
guest:*:998:998:Guest Account:/usr/people/guest:/bin/csh
dup::998:998:oops dup uid gid:/home/he:/bin/ksh
as:*:30001:30001:Alan Smithee:/home/as:/bin/ksh
jethro::00030001:80001:oops dup uid number different text:/home/he:/bin/ksh
lg:*:30001:10003:Len Gernwolf:/home/lg:/bin/ksh
he::30002:30002:Hammond Egger:/home/he:/bin/ksh
nobody:*:60001:60001:original nobody uid:/dev/null:/dev/null
noaccess:*:60002:60002:uid no access:/dev/null:/dev/null
he::90002:90002:oops dup name:/home/he:/bin/ksh'

	EXEC -x $data/pwd.dss '{sort --uniq uid}' $data/pwd.dat
		OUTPUT - $'diag:*:-3:996:Hardware Diagnostics:/usr/diags:/bin/csh
aha::-2:0::/:/bin/sh
root:*:0:0:Super-User:/:/bin/csh
daemon:*:1:1:daemons:/:/dev/null
bin:*:2:2:System Tools Owner:/bin:/dev/null
uucp:*:3:5:UUCP Owner:/usr/lib/uucp:/bin/csh
sys:*:4:0:System Activity Owner:/var/adm:/bin/sh
adm:*:5:3:Accounting Files Owner:/var/adm:/bin/sh
lp:*:9:9:Print Spooler Owner:/var/spool/lp:/bin/sh
auditor:*:11:0:Audit Activity Owner:/auditor:/bin/sh
ftp:*:112:112:File Transfer:/home/ftp:/dev/null
demos:*:993:997:Demonstration User:/usr/demos:/bin/csh
guest:*:998:998:Guest Account:/usr/people/guest:/bin/csh
as:*:30001:30001:Alan Smithee:/home/as:/bin/ksh
he::30002:30002:Hammond Egger:/home/he:/bin/ksh
nobody:*:60001:60001:original nobody uid:/dev/null:/dev/null
noaccess:*:60002:60002:uid no access:/dev/null:/dev/null
he::90002:90002:oops dup name:/home/he:/bin/ksh'

	EXEC -x $data/pwd.dss '{sort --reverse uid}' $data/pwd.dat
		OUTPUT - $'he::90002:90002:oops dup name:/home/he:/bin/ksh
noaccess:*:60002:60002:uid no access:/dev/null:/dev/null
nobody:*:60001:60001:original nobody uid:/dev/null:/dev/null
he::30002:30002:Hammond Egger:/home/he:/bin/ksh
lg:*:30001:10003:Len Gernwolf:/home/lg:/bin/ksh
jethro::00030001:80001:oops dup uid number different text:/home/he:/bin/ksh
as:*:30001:30001:Alan Smithee:/home/as:/bin/ksh
dup::998:998:oops dup uid gid:/home/he:/bin/ksh
guest:*:998:998:Guest Account:/usr/people/guest:/bin/csh
demos:*:993:997:Demonstration User:/usr/demos:/bin/csh
ftp:*:112:112:File Transfer:/home/ftp:/dev/null
auditor:*:11:0:Audit Activity Owner:/auditor:/bin/sh
lp:*:9:9:Print Spooler Owner:/var/spool/lp:/bin/sh
adm:*:5:3:Accounting Files Owner:/var/adm:/bin/sh
sys:*:4:0:System Activity Owner:/var/adm:/bin/sh
uucp:*:3:5:UUCP Owner:/usr/lib/uucp:/bin/csh
bin:*:2:2:System Tools Owner:/bin:/dev/null
daemon:*:1:1:daemons:/:/dev/null
sysadm:*:0:9:System V Administration:/usr/admin:/bin/sh
root:*:0:0:Super-User:/:/bin/csh
aha::-2:0::/:/bin/sh
diag:*:-3:996:Hardware Diagnostics:/usr/diags:/bin/csh'

	EXEC -x $data/pwd.dss '{sort --reverse --uniq uid}' $data/pwd.dat
		OUTPUT - $'he::90002:90002:oops dup name:/home/he:/bin/ksh
noaccess:*:60002:60002:uid no access:/dev/null:/dev/null
nobody:*:60001:60001:original nobody uid:/dev/null:/dev/null
he::30002:30002:Hammond Egger:/home/he:/bin/ksh
lg:*:30001:10003:Len Gernwolf:/home/lg:/bin/ksh
dup::998:998:oops dup uid gid:/home/he:/bin/ksh
demos:*:993:997:Demonstration User:/usr/demos:/bin/csh
ftp:*:112:112:File Transfer:/home/ftp:/dev/null
auditor:*:11:0:Audit Activity Owner:/auditor:/bin/sh
lp:*:9:9:Print Spooler Owner:/var/spool/lp:/bin/sh
adm:*:5:3:Accounting Files Owner:/var/adm:/bin/sh
sys:*:4:0:System Activity Owner:/var/adm:/bin/sh
uucp:*:3:5:UUCP Owner:/usr/lib/uucp:/bin/csh
bin:*:2:2:System Tools Owner:/bin:/dev/null
daemon:*:1:1:daemons:/:/dev/null
sysadm:*:0:9:System V Administration:/usr/admin:/bin/sh
aha::-2:0::/:/bin/sh
diag:*:-3:996:Hardware Diagnostics:/usr/diags:/bin/csh'

TEST 03 'multiple fields'

	EXEC -x $data/pwd.dss '{sort uid gid}' $data/pwd.dat
		OUTPUT - $'diag:*:-3:996:Hardware Diagnostics:/usr/diags:/bin/csh
aha::-2:0::/:/bin/sh
root:*:0:0:Super-User:/:/bin/csh
sysadm:*:0:9:System V Administration:/usr/admin:/bin/sh
daemon:*:1:1:daemons:/:/dev/null
bin:*:2:2:System Tools Owner:/bin:/dev/null
uucp:*:3:5:UUCP Owner:/usr/lib/uucp:/bin/csh
sys:*:4:0:System Activity Owner:/var/adm:/bin/sh
adm:*:5:3:Accounting Files Owner:/var/adm:/bin/sh
lp:*:9:9:Print Spooler Owner:/var/spool/lp:/bin/sh
auditor:*:11:0:Audit Activity Owner:/auditor:/bin/sh
ftp:*:112:112:File Transfer:/home/ftp:/dev/null
demos:*:993:997:Demonstration User:/usr/demos:/bin/csh
guest:*:998:998:Guest Account:/usr/people/guest:/bin/csh
dup::998:998:oops dup uid gid:/home/he:/bin/ksh
lg:*:30001:10003:Len Gernwolf:/home/lg:/bin/ksh
as:*:30001:30001:Alan Smithee:/home/as:/bin/ksh
jethro::00030001:80001:oops dup uid number different text:/home/he:/bin/ksh
he::30002:30002:Hammond Egger:/home/he:/bin/ksh
nobody:*:60001:60001:original nobody uid:/dev/null:/dev/null
noaccess:*:60002:60002:uid no access:/dev/null:/dev/null
he::90002:90002:oops dup name:/home/he:/bin/ksh'

	EXEC -x $data/pwd.dss '{sort --uniq uid gid}' $data/pwd.dat
		OUTPUT - $'diag:*:-3:996:Hardware Diagnostics:/usr/diags:/bin/csh
aha::-2:0::/:/bin/sh
root:*:0:0:Super-User:/:/bin/csh
sysadm:*:0:9:System V Administration:/usr/admin:/bin/sh
daemon:*:1:1:daemons:/:/dev/null
bin:*:2:2:System Tools Owner:/bin:/dev/null
uucp:*:3:5:UUCP Owner:/usr/lib/uucp:/bin/csh
sys:*:4:0:System Activity Owner:/var/adm:/bin/sh
adm:*:5:3:Accounting Files Owner:/var/adm:/bin/sh
lp:*:9:9:Print Spooler Owner:/var/spool/lp:/bin/sh
auditor:*:11:0:Audit Activity Owner:/auditor:/bin/sh
ftp:*:112:112:File Transfer:/home/ftp:/dev/null
demos:*:993:997:Demonstration User:/usr/demos:/bin/csh
guest:*:998:998:Guest Account:/usr/people/guest:/bin/csh
lg:*:30001:10003:Len Gernwolf:/home/lg:/bin/ksh
as:*:30001:30001:Alan Smithee:/home/as:/bin/ksh
jethro::00030001:80001:oops dup uid number different text:/home/he:/bin/ksh
he::30002:30002:Hammond Egger:/home/he:/bin/ksh
nobody:*:60001:60001:original nobody uid:/dev/null:/dev/null
noaccess:*:60002:60002:uid no access:/dev/null:/dev/null
he::90002:90002:oops dup name:/home/he:/bin/ksh'

	EXEC -x $data/pwd.dss '{sort --reverse uid gid}' $data/pwd.dat
		OUTPUT - $'he::90002:90002:oops dup name:/home/he:/bin/ksh
noaccess:*:60002:60002:uid no access:/dev/null:/dev/null
nobody:*:60001:60001:original nobody uid:/dev/null:/dev/null
he::30002:30002:Hammond Egger:/home/he:/bin/ksh
jethro::00030001:80001:oops dup uid number different text:/home/he:/bin/ksh
as:*:30001:30001:Alan Smithee:/home/as:/bin/ksh
lg:*:30001:10003:Len Gernwolf:/home/lg:/bin/ksh
dup::998:998:oops dup uid gid:/home/he:/bin/ksh
guest:*:998:998:Guest Account:/usr/people/guest:/bin/csh
demos:*:993:997:Demonstration User:/usr/demos:/bin/csh
ftp:*:112:112:File Transfer:/home/ftp:/dev/null
auditor:*:11:0:Audit Activity Owner:/auditor:/bin/sh
lp:*:9:9:Print Spooler Owner:/var/spool/lp:/bin/sh
adm:*:5:3:Accounting Files Owner:/var/adm:/bin/sh
sys:*:4:0:System Activity Owner:/var/adm:/bin/sh
uucp:*:3:5:UUCP Owner:/usr/lib/uucp:/bin/csh
bin:*:2:2:System Tools Owner:/bin:/dev/null
daemon:*:1:1:daemons:/:/dev/null
sysadm:*:0:9:System V Administration:/usr/admin:/bin/sh
root:*:0:0:Super-User:/:/bin/csh
aha::-2:0::/:/bin/sh
diag:*:-3:996:Hardware Diagnostics:/usr/diags:/bin/csh'

	EXEC -x $data/pwd.dss '{sort --reverse --uniq uid gid}' $data/pwd.dat
		OUTPUT - $'he::90002:90002:oops dup name:/home/he:/bin/ksh
noaccess:*:60002:60002:uid no access:/dev/null:/dev/null
nobody:*:60001:60001:original nobody uid:/dev/null:/dev/null
he::30002:30002:Hammond Egger:/home/he:/bin/ksh
jethro::00030001:80001:oops dup uid number different text:/home/he:/bin/ksh
as:*:30001:30001:Alan Smithee:/home/as:/bin/ksh
lg:*:30001:10003:Len Gernwolf:/home/lg:/bin/ksh
dup::998:998:oops dup uid gid:/home/he:/bin/ksh
demos:*:993:997:Demonstration User:/usr/demos:/bin/csh
ftp:*:112:112:File Transfer:/home/ftp:/dev/null
auditor:*:11:0:Audit Activity Owner:/auditor:/bin/sh
lp:*:9:9:Print Spooler Owner:/var/spool/lp:/bin/sh
adm:*:5:3:Accounting Files Owner:/var/adm:/bin/sh
sys:*:4:0:System Activity Owner:/var/adm:/bin/sh
uucp:*:3:5:UUCP Owner:/usr/lib/uucp:/bin/csh
bin:*:2:2:System Tools Owner:/bin:/dev/null
daemon:*:1:1:daemons:/:/dev/null
sysadm:*:0:9:System V Administration:/usr/admin:/bin/sh
root:*:0:0:Super-User:/:/bin/csh
aha::-2:0::/:/bin/sh
diag:*:-3:996:Hardware Diagnostics:/usr/diags:/bin/csh'

TEST 03 'sort uniq count threshhold'

	EXEC -x $data/pwd.dss '{sort --count uid}' $data/pwd.dat
		OUTPUT - $'1 diag:*:-3:996:Hardware Diagnostics:/usr/diags:/bin/csh
1 aha::-2:0::/:/bin/sh
1 sysadm:*:0:9:System V Administration:/usr/admin:/bin/sh
1 daemon:*:1:1:daemons:/:/dev/null
1 bin:*:2:2:System Tools Owner:/bin:/dev/null
1 uucp:*:3:5:UUCP Owner:/usr/lib/uucp:/bin/csh
1 sys:*:4:0:System Activity Owner:/var/adm:/bin/sh
1 adm:*:5:3:Accounting Files Owner:/var/adm:/bin/sh
1 lp:*:9:9:Print Spooler Owner:/var/spool/lp:/bin/sh
1 auditor:*:11:0:Audit Activity Owner:/auditor:/bin/sh
1 ftp:*:112:112:File Transfer:/home/ftp:/dev/null
1 demos:*:993:997:Demonstration User:/usr/demos:/bin/csh
1 dup::998:998:oops dup uid gid:/home/he:/bin/ksh
1 jethro::00030001:80001:oops dup uid number different text:/home/he:/bin/ksh
1 lg:*:30001:10003:Len Gernwolf:/home/lg:/bin/ksh
1 he::30002:30002:Hammond Egger:/home/he:/bin/ksh
1 nobody:*:60001:60001:original nobody uid:/dev/null:/dev/null
1 noaccess:*:60002:60002:uid no access:/dev/null:/dev/null
1 he::90002:90002:oops dup name:/home/he:/bin/ksh
2 root:*:0:0:Super-User:/:/bin/csh
2 guest:*:998:998:Guest Account:/usr/people/guest:/bin/csh
3 as:*:30001:30001:Alan Smithee:/home/as:/bin/ksh'

	EXEC -x $data/pwd.dss '{sort --count=1 uid}' $data/pwd.dat

	EXEC -x $data/pwd.dss '{sort --count=2 uid}' $data/pwd.dat
		OUTPUT - $'2 root:*:0:0:Super-User:/:/bin/csh
2 guest:*:998:998:Guest Account:/usr/people/guest:/bin/csh
3 as:*:30001:30001:Alan Smithee:/home/as:/bin/ksh'

	EXEC -x $data/pwd.dss '{sort --count=3 uid}' $data/pwd.dat
		OUTPUT - $'3 as:*:30001:30001:Alan Smithee:/home/as:/bin/ksh'

	EXEC -x $data/pwd.dss '{sort --count=4 uid}' $data/pwd.dat
		OUTPUT -
