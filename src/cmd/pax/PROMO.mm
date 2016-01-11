.H 1 pax
.B pax
is a POSIX 1003.2 conformant replacement for
.B tar
and
.B cpio
that handles most UNIX archive and tape formats.
.B pax
uses the
.I vdelta
algorithm to construct efficient delta archives that contain bytewise
changes from a given base archive. 
.B gzip
and
.B compress
archives are also handled on input and output.
.P
To create a
.I vdelta
compressed base archive:
.EX
tw | pax -w -f base -z -
.EE
To create a delta archive on the base above (record file changes only):
.EX
tw | pax -w -f delta -z base
.EE
To read the delta archive:
.EX
pax -r -f delta -z base
.EE
To create a delta archive from linux-1.2.0 to linux-1.2.5:
.EX
pax -rf linux-1.2.5.tar.gz -wf 0-5.pax.gz -x gzip -z linux-1.2.0.tar.gz
.EE
To create the linux-1.2.5 archive:
.EX
pax -rf 0-5.pax.gz -z linux-1.2.0.tar.gz -wf linux-1.2.5.tar.gz -x tar -x gzip
.EE
The
.B bax
script, shipped with
.BR pax ,
combines
.B tw
and
.B pax
for handy backup control.
