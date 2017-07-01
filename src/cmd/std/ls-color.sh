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
: color ls
ls -D name='%(mode:case:d*:\E[01;34m%(name)s\E[0m:l*:\E[01;36m%(name)s\E[0m:*x*:\E[01;32m%(name)s\E[0m:*:%(name)s)s' "$@"
