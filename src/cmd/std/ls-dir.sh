########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1989-2011 AT&T Intellectual Property          #
#                      and is licensed under the                       #
#                 Eclipse Public License, Version 1.0                  #
#                    by AT&T Intellectual Property                     #
#                                                                      #
#                A copy of the License is available at                 #
#          http://www.eclipse.org/org/documents/epl-v10.html           #
#         (with md5 checksum b35adb5213ca9657e911e9befb180842)         #
#                                                                      #
#              Information and Software Systems Research               #
#                            AT&T Research                             #
#                           Florham Park NJ                            #
#                                                                      #
#               Glenn Fowler <glenn.s.fowler@gmail.com>                #
#                                                                      #
########################################################################
: we do that too
ls 	-D header='%(dir.count:case:[01]::*:\n)s Directory of %(path)s\n' \
	-D meridian='%(mtime:time=%p)s' \
	-D trailer='%9(dir.files)lu file(s) %10(dir.bytes)lu bytes\n' \
	-Z '%-8.8(name:edit:\\([^.]*\\).*:\\1:u)s %3.3(name:edit:[^.]*\\.*\\(.*\\):\\1:u)s %5(mode:case:d*:<DIR>:l*:<LNK>:*x*:<EXE>)s%7(size)lu %(mtime:time=%m-%d-%y  %I:%M)s%(meridian:edit:\\(.\\).*:\\1:l)s' \
	"$@"
