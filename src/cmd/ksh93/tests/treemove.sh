########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1982-2013 AT&T Intellectual Property          #
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
#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
#
# You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at usr/src/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END
#

#
# Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
# Contributed by Roland Mainz <roland.mainz@nrubsig.org>

#
# This test checks whether "typeset -m" correctly moves local variables
# into a global variable tree.
#
# This was reported as CR #XXXXXXXX ("XXXX"):
# -- snip --
#XXXX
# -- snip --
#

# Some platforms, such as OpenBSD and Cygwin, do not handle NaN correctly.
# See https://marc.info/?l=openbsd-bugs&m=152488432922625&w=2
if typeset -f .sh.math.signbit >/dev/null && (( signbit(-NaN) ))
then
    HAVE_signbit=1
else
    log_warning '-lm does not support signbit(-NaN)'
    HAVE_signbit=0
fi

function idempotent
{
    typeset var action='typeset -p'
    [[ $1 == -* ]] && { shift; var=$2=; action='print -v'; }
    typeset -n expect=$1
    typeset actual=$($SHELL <<- EOF
		$3
		$var$expect
		$action $2
	EOF)

    [[ "$actual" == "$expect" ]] || log_error "idempotent test failed" "$expect" "$actual"
}

## test start
typeset -C tree1 tree2

# add node to tree which uses "typeset -m" to move a local variable
# into tree1.subtree["a_node"]
function f1
{
    nameref tr=$1
    typeset -A tr.subtree
    typeset -C node
    node.one="hello"
    node.two="world"
    # move local note into the array
    typeset -m tr.subtree["a_node"]=node
    return 0
}

# Alternative version which uses "nameref" instead of "typeset -m"
function f2
{
    nameref tr=$1
    typeset -A tr.subtree
    nameref node=tr.subtree["a_node"]
    node.one="hello"
    node.two="world"
    return 0
}

f1 tree1
f2 tree2

[[ "${tree1.subtree["a_node"].one}" == "hello" ]] || log_error "expected tree1.subtree[\"a_node\"].one == 'hello', got ${tree1.subtree["a_node"].one}"
[[ "${tree1.subtree["a_node"].two}" == "world" ]] || log_error "expected tree1.subtree[\"a_node\"].two == 'world', got ${tree1.subtree["a_node"].two}"
[[ "${tree1}" == "${tree2}" ]] || log_error "tree1 and tree2 differ:$'\n'"

unset c
compound c
typeset -C -a c.ar
c.ar[4]=( a4=1 )
typeset -m "c.ar[5]=c.ar[4]"
expect=$'(\n\ttypeset -C -a ar=(\n\t\t[5]=(\n\t\t\ta4=1\n\t\t)\n\t)\n)'
actual=$(print -v c)
[[ "$actual" == "$expect" ]] || log_error 'typeset -m "c.ar[5]=c.ar[4]" not working' "$expect" "$actual"
idempotent -v exexpect c

typeset -T x_t=( hello=world )
function m
{
    compound c
    compound -a c.x
    x_t c.x[4][5][8].field
    x_t x
    typeset -m c.x[4][6][9].field=x
    exp=$'(\n\ttypeset -C -a x=(\n\t\ttypeset -a [4]=(\n\t\t\ttypeset -a [5]=(\n\t\t\t\t[8]=(\n\t\t\t\t\tx_t field=(\n\t\t\t\t\t\thello=world\n\t\t\t\t\t)\n\t\t\t\t)\n\t\t\t)\n\t\t\ttypeset -a [6]=(\n\t\t\t\t[9]=(\n\t\t\t\t\tx_t field=(\n\t\t\t\t\t\thello=world\n\t\t\t\t\t)\n\t\t\t\t)\n\t\t\t)\n\t\t)\n\t)\n)'

    [[ $(print -v c) == "$exp" ]] || log_error "typeset -m c.x[4][6][9].field=x where x is a type is not working"
}
m

function moveme
{
    nameref src=$2 dest=$1
    typeset -m dest=src
}

function main
{
    compound a=( aa=1 )
    compound -a ar
    moveme ar[4] a 2> /dev/null || log_error 'function moveme fails'
    exp=$'(\n\t[4]=(\n\t\taa=1\n\t)\n)'
    [[ $(print -v ar) == "$exp" ]] || log_error 'typeset -m dest=src where dest and src are name references fails'
}
main

{
$SHELL <<- \EOF
    function main
    {
        compound c=(
            compound -a board
        )
        for ((i=0 ; i < 2 ; i++ )) ; do
            compound el=(typeset id='pawn')
            typeset -m "c.board[1][i]=el"
        done
        exp=$'(\n\ttypeset -C -a board=(\n\t\t[1]=(\n\t\t\t(\n\t\t\t\tid=pawn\n\t\t\t)\n\t\t\t(\n\t\t\t\tid=pawn\n\t\t\t)\n\t\t)\n\t)\n)'
        [[ $(print -v c) == "$exp" ]] || exit 1
    }
    main
EOF
} 2> /dev/null
if ((exitval=$?))
then
    if [[ $(kill -l $exitval) == SEGV ]]
    then
        log_error 'typeset -m "c.board[1][i]=el" core dumps'
    else
        log_error 'typeset -m "c.board[1][i]=el" gives wrong value'
    fi
fi

compound c=(
    compound -a ar=(
        ( float i=4 )
        ( float i=7 )
        ( float i=2 )
        ( float i=1 )
        ( float i=24 )
        ( float i=-1 )
    )
)

function sortar
{
    nameref ar=$1
    integer  i i_max=${#ar[@]}
    bool swapped=true
    while $swapped
    do
        swapped=false
        for (( i=1 ; i < i_max ; i++ ))
        do
            if (( ar[i].i > ar[i-1].i ))
            then
                typeset -m "tmpvar=ar[i-1]"
                typeset -m "ar[i-1]=ar[i]"
                typeset -m "ar[i]=tmpvar"
                swapped=true
            fi
        done
    done
    return 0
}

sortar c.ar
expect='typeset -C -a c.ar=((typeset -l -E i=24) (typeset -l -E i=7) (typeset -l -E i=4) (typeset -l -E i=2) (typeset -l -E i=1) (typeset -l -E i=-1))'
actual=$(typeset -p c.ar)
[[ "$actual" == "$expect" ]] || log_error 'sorting compound arrays with typeset -m failed' "$expect" "$actual"
idempotent expect c.ar 'typeset -C c'

typeset -T objstack_t=(
        compound -a st
        integer st_n=0
        function pushobj
        {
                nameref obj=$1
                typeset -m "_.st[$((_.st_n++))].obj=obj"
        }
        function popobj
        {
                nameref obj=$1
                typeset -m "obj=_.st[$((--_.st_n))].obj"
        s="$(typeset -p _.st[_.st_n].obj)"
        [[ "$s" == '' ]] || log_error '_.st[_.st_n].obj should be empty after typeset -m'
        }
)
compound c
objstack_t c.ost
compound foo=( integer val=5 )
c.ost.pushobj foo
compound res
c.ost.popobj res.a
expect='typeset -C res.a=(typeset -l -i val=5)'
actual=$(typeset -p res.a)
[[ "$actual" == "$expect" ]] || log_error 'typeset -m for compound variable in a type not working' "$expect" "$actual"
idempotent expect res.a 'typeset -C res'

unset c
compound c dummy
objstack_t c.ost
compound foo=( integer val=5 )
typeset -a bar=(  2 3 4 )
c.ost.pushobj foo
c.ost.pushobj bar
c.ost.popobj dummy
expect='typeset -C c=(objstack_t ost=(typeset -l -i st_n=1;st[0]=(obj=(typeset -l -i val=5))))'
actual=$(typeset -p c)
[[ "$actual" == "$expect" ]] || log_error 'typeset -m for types not working' "$expect" "$actual"
idempotent expect c "$(typeset -T)"

unset c
typeset -p c
compound c
objstack_t c.ost
compound foo=( integer val=5 )
c.ost.pushobj foo
compound sc
objstack_t sc.s
compound c1=( integer a=1 )
compound c2=( integer a=2 )
sc.s.pushobj c1
sc.s.pushobj c2
c.ost.pushobj sc
expect='typeset -C c=(objstack_t ost=(typeset -l -i st_n=2;st[0]=(obj=(typeset -l -i val=5;););st[1]=(obj=(objstack_t s=(typeset -l -i st_n=2;st[0]=(obj=(typeset -l -i a=1;););st[1]=(obj=(typeset -l -i a=2)))))))'
actual=$(typeset -p c)
[[ "$actual" == "$expect" ]] || log_error 'typeset -m not working' "$expect" "$actual"

typeset -T printfish_t=(
            typeset fname
        unset() { :;}
    )
    function createfish_t
    {
            nameref ret=$1
            typeset fishname="$2"
            printfish_t f
            f.fname="$fishname"
        typeset -m 'ret=f'
    }
    compound c
    compound -a c.cx
    compound c.cx[4][9].ca
    createfish_t c.cx[4][9].ca.shark 'coelacanth'
    createfish_t c.cx[4][9].ca.horse 'horse'
    exp='typeset -C -a c.cx=(typeset -a [4]=([9]=(ca=(printfish_t horse=(fname=horse;);printfish_t shark=(fname=coelacanth)))) )'
    [[ $(typeset -p c.cx) == "$exp" ]] || log_error 'typeset -m for types not working'

typeset -T key_t=( float i)
compound c=(
    key_t -a ar=(
        ( i=7 )
        ( i=1 )
        ( i=11 )
    )
)
nameref ar=c.ar
typeset -m "tmpvar=ar[1]"
command typeset -m "ar[1]=ar[2]" 2> /dev/null || log_error 'typeset -m ar[1]=ar[2] not working when ar is numerical'
typeset -m "ar[2]=tmpvar"
expect='key_t -a c.ar=((typeset -l -E i=7) (typeset -l -E i=11) (typeset -l -E i=1))'
actual=$(typeset -p c.ar)
[[ "$actual" == "$expect" ]] 2> /dev/null || log_error 'typeset -m c.ar has wrong value' "$expect" "$actual"
idempotent expect c.ar 'typeset -T key_t=(float i);compound c'

function sortar
{
    nameref ar=$1
    integer i i_max=${#ar[@]}
    bool swapped=true
    while $swapped
    do
        swapped=false
        for (( i=1 ; i < i_max ; i++ ))
        do
            if ((isnan(ar[i].i) || isgreater(ar[i].i-ar[i-1].i, 0.)))
            then
                typeset -m "tmpvar=ar[i-1]"
                typeset -m "ar[i-1]=ar[i]"
                typeset -m "ar[i]=tmpvar"
                swapped=true
            fi
        done
    done
}

function main
{
    compound c=(
        compound -a ar=( ( float i=4 ) ( float i=-nan ) ( float i=2 )
            ( float i=-inf )  ( float i=1 ) ( float i=24 )
            ( float i=+inf ) ( float i=-1 ) )
    )
    sortar c.ar
    expect='typeset -C -a c.ar=((typeset -l -E i=-nan) (typeset -l -E i=inf) (typeset -l -E i=24) (typeset -l -E i=4) (typeset -l -E i=2) (typeset -l -E i=1) (typeset -l -E i=-1) (typeset -l -E i=-inf))'
    if (( HAVE_signbit ))
    then
        actual=$(typeset -p c.ar)
        [[ "$actual" == "$expect" ]] || log_error 'typeset -m not working when passed a reference to an local argument from a calling function' "$expect" "$actual"
        idempotent expect c.ar 'compound c'
    fi
 }
main

typeset -T X_t=( compound xc )
compound cc
X_t cc.a
compound cc.a.xc
typeset cc.a.xc.i=5
X_t cc.b
typeset -m "cc.b.xc.j=cc.a.xc.i"
expect='typeset -C cc=(X_t a=(typeset -C xc);X_t b=(xc=(j=5)))'
actual=$(typeset -p cc)
[[ "$actual" == "$expect" ]] || log_error 'typeset -m compound variable embedded in type not working' "$expect" "$actual"
idempotent expect cc 'typeset -T X_t=( typeset -C xc )'

unset d
compound d=(
    compound sta=( compound -a st;integer st_numelements=0)
)
expect='typeset -C d=(sta=(typeset -C -a st=( [0]=(obj=(typeset -l -i t=3;);););typeset -l -i st_numelements=1))'
compound foo=( integer t=3 )
typeset -m "d.sta.st[$((d.sta.st_numelements++))].obj=foo"
actual=$(typeset -p d)
[[ "$actual" == "$expect" ]] || log_error 'compound variable not displayed properly' "$expect" "$actual"
idempotent expect d

function f2
{
    nameref mar=$1 exp=$2
    typeset dummy x="-1a2b3c4d9u"
    dummy="${x//~(E)([[:digit:]])|([[:alpha:]])/D}"
    exp=${ print -v .sh.match;}
    typeset -m "mar=.sh.match"
}
function f1
{
    typeset matchar exp
    f2 matchar exp
    [[ ${ print -v matchar;}  == "$exp" ]] || log_error  'move .sh.match to a function local variable using a name reference fails'
}
f1
