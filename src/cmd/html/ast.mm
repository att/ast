'''''\"
'''''\" troff2html additions to mm
'''''\" .xx used in lieu of a deeper understanding of troff
'''''\"
.ds Ei I
.de EI
.ds Ei \\$1
..
.de EX
.ie '\\$1'' .DS \\*(Ei
.el \{
.ds fG \\fBFigure \\$1\\fR: \\$2
.DF
.mk # \}
.ds fX \\n(.f
.L
..
.de EE
.if !'\\*(fG'' \{
.ce
\\*(fG
.sp -1
\h'-.5n'\L'|\\n#u-1'\l'\\n(.lu+1n\(ul'\L'-|\\n#u+1'\l'|0u-.5n\(ul'
.rm fG \}
.DE
.ft \\*(fX
..
.ds HP +2 +0 -1 -1
.de HX
.if \\$1=1 .ds }0 \\n(H1\ \ 
..
.de UL
\\$1\l'|0\(ul'
..
.de L
.ie\\n(.$ .nr ;G \\n(.f
.el.ft 5
.if\\n(.$ .if !\\n(.$-1 \&\f5\\$1
.if\\n(.$-1 \{.ds }i \^
.if\\n(.f5 .ds }i
.ds}I \&
.if\w\\$1 .ds }I \&\f5\\$1\fP\\*(}i
'br\}
.if\\n(.$-1 .if !\\n(.$-3 \\*(}I\\$2\f5\\$3
.if\\n(.$-3 .if !\\n(.$-5 \\*(}I\\$2\f5\\$3\fP\\*(}i\\$4\f5\\$5
.if\\n(.$-5 \\*(}I\\$2\f5\\$3\fP\\*(}i\\$4\f5\\$5\fP\\*(}i\\$6
.if\\n(.$ .ft \\n(;G
..
.de LR
.nr;G \\n(.f
.}S 5 1 \& "\\$1" "\\$2" "\\$3" "\\$4" "\\$5" "\\$6"
..
.de RL
.nr;G \\n(.f
.}S 1 5 \& "\\$1" "\\$2" "\\$3" "\\$4" "\\$5" "\\$6"
..
.de IL
.nr;G \\n(.f
.}S 2 5 \& "\\$1" "\\$2" "\\$3" "\\$4" "\\$5" "\\$6"
..
.de LO
.nr;G \\n(.f
.}S 5 2 \& "\\$1" "\\$2" "\\$3" "\\$4" "\\$5" "\\$6"
..
.de OL
.nr;G \\n(.f
.}S 2 5 \& "\\$1" "\\$2" "\\$3" "\\$4" "\\$5" "\\$6"
..
.de FS
.xx footnote=\\$1
..
.de FE
.xx nofootnote
..
