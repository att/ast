########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1982-2012 AT&T Intellectual Property          #
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
#                    David Korn <dgkorn@gmail.com>                     #
#                                                                      #
########################################################################

# Memory leaks in these tests should be caught by asan (or valgrind)
# Test for variable reset leak
function test_reset
{
    integer i n=$1

    for ((i = 0; i < n; i++))
    do
        u=$i
    done
}

n=1000
test_reset $n

typeset -A foo
typeset -lui i=0

for (( i=0; i<500; i++ ))
do
    unset foo[bar]
    typeset -A foo[bar]
    foo[bar][elem0]="data0"
    foo[bar][elem1]="data1"
    foo[bar][elem2]="data2"
    foo[bar][elem3]="data3"
    foo[bar][elem4]="data4"
    foo[bar][elem5]="data5"
    foo[bar][elem6]="data6"
    foo[bar][elem7]="data7"
    foo[bar][elem8]="data8"
    foo[bar][elem9]="data9"
    foo[bar][elem9]="data9"
done

# Test for leak in executing subshell after PATH is reset
for (( i=0; i<100; i++ ))
do
    PATH=.
    for DIR in /usr/bin /usr/sbin /bin /usr/local/bin
    do
        if [[ -d ${DIR} ]]
        then
            PATH=${PATH}:${DIR}
        fi
        time=$(date '+%T' 2>/dev/null)
    done
done

# Memory leak with read -C when deleting compound variable
data="(v=;sid=;di=;hi=;ti='1328244300';lv='o';id='172.3.161.178';var=(k='conn_num._total';u=;fr=;l='Number of Connections';n='22';t='number';))"
read -C stat <<< "$data"
for ((i=0; i < 1; i++))
do
    print -r -- "$data"
done |    while read -u$n -C stat
    do    :
    done    {n}<&0-

    for ((i=0; i < 500; i++))
do    print -r -- "$data"
done |    while read -u$n -C stat
    do    :
    done    {n}<&0-

# Memory leak with read -C when using <<<
read -C stat <<< "$data"
for ((i=0; i < 500; i++))
do      read -C stat <<< "$data"
done
