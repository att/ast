########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1982-2011 AT&T Intellectual Property          #
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

set -o nounset

function test_arithmetric_expression_accesss_array_element_through_nameref
{
    compound out=( typeset stdout stderr ; integer res )
    compound -r -a tests=(
        (
            cmd='@@TYPE@@ -a @@VAR@@ ;  @@VAR@@[1]=90 ;       function x { nameref nz=$1 ;              print " $(( round(nz) ))==$(( round($nz) ))" ; } ; x @@VAR@@[1]'        ; stdoutpattern=' 90==90'
        )
        (
            cmd='@@TYPE@@ -a @@VAR@@=( [1]=90 ) ;             function x { nameref nz=$1 ;              print " $(( round(nz) ))==$(( round($nz) ))" ; } ; x @@VAR@@[1]'        ; stdoutpattern=' 90==90'
        )
        (
            cmd='@@TYPE@@ -a @@VAR@@ ;  @@VAR@@[1][3]=90 ;    function x { nameref nz=$1 ;               print " $(( round(nz) ))==$(( round($nz) ))" ; } ; x @@VAR@@[1][3]'    ; stdoutpattern=' 90==90'
        )
        (
            cmd='@@TYPE@@ -a @@VAR@@=( [1][3]=90 ) ;          function x { nameref nz=$1 ;               print " $(( round(nz) ))==$(( round($nz) ))" ; } ; x @@VAR@@[1][3]'    ; stdoutpattern=' 90==90'
        )
        (
            cmd='@@TYPE@@ -a @@VAR@@ ;  @@VAR@@[1][3][5]=90 ; function x { nameref nz=$1 ;               print " $(( round(nz) ))==$(( round($nz) ))" ; } ; x @@VAR@@[1][3][5]'    ; stdoutpattern=' 90==90'
        )
        (
            cmd='@@TYPE@@ -a @@VAR@@=( [1][3][5]=90 ) ;       function x { nameref nz=$1 ;               print " $(( round(nz) ))==$(( round($nz) ))" ; } ; x @@VAR@@[1][3][5]'    ; stdoutpattern=' 90==90'
        )
        (
            cmd='@@TYPE@@ -a @@VAR@@ ;  @@VAR@@[1][3][5]=90 ; function x { nameref nz=${1}[$2][$3][$4] ; print " $(( round(nz) ))==$(( round($nz) ))" ; } ; x @@VAR@@ 1 3 5'    ; stdoutpattern=' 90==90'
        )
        (
            cmd='@@TYPE@@ -A @@VAR@@ ;  @@VAR@@[1]=90 ;       function x { nameref nz=$1 ;               print " $(( round(nz) ))==$(( round($nz) ))" ; } ; x @@VAR@@[1]'        ; stdoutpattern=' 90==90'
        )
        (
            cmd='@@TYPE@@ -A @@VAR@@=( [1]=90 ) ;             function x { nameref nz=$1 ;               print " $(( round(nz) ))==$(( round($nz) ))" ; } ; x @@VAR@@[1]'        ; stdoutpattern=' 90==90'
        )
    )

    typeset testname
    integer i
    typeset mode
    typeset cmd

    for (( i=0 ; i < ${#tests[@]} ; i++ )) ; do
        # fixme: This list should include "typeset -lX" and "typeset -X" but ast-ksh.2010-03-09 fails like this:
        # 'typeset -X -a z ;  z[1][3]=90 ; function x { nameref nz=$1 ; print " $(( nz ))==$(( $nz ))" ; } ; x z[1][3]'
        # + typeset -X -a z
        # + z[1][3]=90
        # + x 'z[1][3]'
        # /home/test001/bin/ksh[1]: x: line 1: x1.68000000000000000000000000000000p: no parent
        for ty in \
            'typeset' \
            'integer' \
            'float' \
            'typeset -i' \
            'typeset -si' \
            'typeset -li' \
            'typeset -E' \
            'typeset -F' \
            'typeset -X' \
            'typeset -lE' \
            'typeset -lX' \
            'typeset -lF' ; do
            for mode in \
                'plain' \
                'in_compound' \
                'in_indexed_compound_array' \
                'in_2d_indexed_compound_array' \
                'in_4d_indexed_compound_array' \
                'in_associative_compound_array' \
                'in_compound_nameref' \
                'in_indexed_compound_array_nameref' \
                'in_2d_indexed_compound_array_nameref' \
                'in_4d_indexed_compound_array_nameref' \
                'in_associative_compound_array_nameref' \
                 ; do
                nameref tst=tests[i]
                cmd="${tst.cmd//@@TYPE@@/${ty}}"
                case "${mode}" in
                    'plain')
                        cmd="${cmd//@@VAR@@/z}"
                        ;;

                    'in_compound')
                        cmd="compound c ; ${cmd//@@VAR@@/c.z}"
                        ;;
                    'in_indexed_compound_array')
                        cmd="compound -a c ; ${cmd//@@VAR@@/c[11].z}"
                        ;;
                    'in_2d_indexed_compound_array')
                        cmd="compound -a c ; ${cmd//@@VAR@@/c[17][19].z}"
                        ;;
                    'in_4d_indexed_compound_array')
                        cmd="compound -a c ; ${cmd//@@VAR@@/c[17][19][23][27].z}"
                        ;;
                    'in_associative_compound_array')
                        cmd="compound -A c ; ${cmd//@@VAR@@/c[info].z}"
                        ;;

                    'in_compound_nameref')
                        cmd="compound c ; nameref ncr=c.z ; ${cmd//@@VAR@@/ncr}"
                        ;;
                    'in_indexed_compound_array_nameref')
                        cmd="compound -a c ; nameref ncr=c[11].z ; ${cmd//@@VAR@@/ncr}"
                        ;;
                    'in_2d_indexed_compound_array_nameref')
                        cmd="compound -a c ; nameref ncr=c[17][19].z ; ${cmd//@@VAR@@/ncr}"
                        ;;
                    'in_4d_indexed_compound_array_nameref')
                        cmd="compound -a c ; nameref ncr=c[17][19][23][27].z ; ${cmd//@@VAR@@/ncr}"
                        ;;
                    'in_associative_compound_array_nameref')
                        cmd="compound -A c ; nameref ncr=c[info].z ; ${cmd//@@VAR@@/ncr}"
                        ;;
                    *)
                        log_error "Unexpected mode ${mode}"
                        ;;
                esac

                testname="${0}/${cmd}"
#set -x
                out.stderr="${ { out.stdout="${ ${SHELL} -o nounset -o errexit -c "${cmd}" ; (( out.res=$? )) ; }" ; } 2>&1 ; }"
#set +x

                    [[ "${out.stdout}" == ${tst.stdoutpattern}      ]] || log_error "${testname}: Expected stdout to match $(printf '%q\n' "${tst.stdoutpattern}"), got $(printf '%q\n' "${out.stdout}")"
                       [[ "${out.stderr}" == ''            ]] || log_error "${testname}: Expected empty stderr, got $(printf '%q\n' "${out.stderr}")"
                (( out.res == 0 )) || log_error "${testname}: Unexpected exit code ${out.res}"
            done
        done
    done

    return 0
}

function test_has_iszero
{
    typeset str
    integer i

    typeset -r -a tests=(
        '(( iszero(0)   )) && print "OK"'
        '(( iszero(0.)  )) && print "OK"'
        '(( iszero(-0)  )) && print "OK"'
        '(( iszero(-0.) )) && print "OK"'
        'float n=0.  ; (( iszero(n) )) && print "OK"'
        'float n=+0. ; (( iszero(n) )) && print "OK"'
        'float n=-0. ; (( iszero(n) )) && print "OK"'
        'float n=1.  ; (( iszero(n) )) || print "OK"'
        'float n=1.  ; (( iszero(n-1.) )) && print "OK"'
        'float n=-1. ; (( iszero(n+1.) )) && print "OK"'
    )

    for (( i=0 ; i < ${#tests[@]} ; i++ )) ; do
        str="$( ${SHELL} -o errexit -c "${tests[i]}" 2>&1 )" || log_error "test $i: returned non-zero exit code $?"
        [[ "${str}" == 'OK' ]] || log_error "test $i: expected 'OK', got '${str}'"
    done

    return 0
}

# run tests
test_arithmetric_expression_accesss_array_element_through_nameref
test_has_iszero

# ==========
# Math functions

# Rounds of floating point numbers to 8 decimal places
function roundof {
    number=$1
    printf '%.8f' $number
}

# acos
expect=3.14159265
actual=$(roundof $(( acos(-1) )) )
[[ $actual -eq $expect ]] || log_error "acos failed" "$expect" "$actual"

# ==========
# acosh
expect=5.19292599
actual=$(roundof $(( acosh(90) )) )
[[ $actual -eq $expect ]] || log_error "acosh failed" "$expect" "$actual"

# ==========
# asin
expect=1.57079633
actual=$(roundof $(( asin(1) )) )
[[ $actual -eq $expect ]] || log_error "asin failed" "$expect" "$actual"

# ==========
# asinh
expect=5.19298771
actual=$(roundof $(( asinh(90) )) )
[[ $actual -eq $expect ]] || log_error "asinh failed" "$expect" "$actual"

# ==========
# atan
expect=1.55968567
actual=$(roundof $(( atan(90) )) )
[[ $actual -eq $expect ]] || log_error "atan failed" "$expect" "$actual"

# ==========
# atan2
expect=0.78539816
actual=$(roundof $(( atan2(90, 90) )) )
[[ $actual -eq $expect ]] || log_error "atan2 failed" "$expect" "$actual"

# ==========
# atanh
expect=0.54930614
actual=$(roundof $(( atanh(0.5) )) )
[[ $actual -eq $expect ]] || log_error "atanh failed" "$expect" "$actual"

# ==========
# cbrt
expect=4.48140475
actual=$(roundof $(( cbrt(90) )) )
[[ $actual -eq $expect ]] || log_error "cbrt failed" "$expect" "$actual"

# ==========
# ceil
expect=1.0
actual=$(( ceil(0.1) ))
[[ $actual -eq $expect ]] || log_error "ceil failed" "$expect" "$actual"

# ==========
# copysign
expect=-1.0
actual=$(( copysign(1.0, -3) ))
[[ $actual -eq $expect ]] || log_error "copysign failed" "$expect" "$actual"

# ==========
# cos
expect=0.15425145
actual=$(roundof $(( cos(30) )) )
[[ $actual -eq $expect ]] || log_error "cos failed" "$expect" "$actual"

# ==========
# cosh
expect=1.0
actual=$(( cosh(0) ))
[[ $actual -eq $expect ]] || log_error "cosh failed" "$expect" "$actual"

# ==========
# erf
expect=0.84270079
actual=$(roundof $(( erf(1) )) )
[[ $actual -eq $expect ]] || log_error "erf failed" "$expect" "$actual"

# ==========
# erfc
expect=0.15729921
actual=$(roundof $(( erfc(1) )) )
[[ $actual -eq $expect ]] || log_error "erfc failed" "$expect" "$actual"

# ==========
# exp
expect=2.71828183
actual=$(roundof $(( exp(1) )) )
[[ $actual -eq $expect ]] || log_error "exp failed" "$expect" "$actual"

# ==========
# exp2
expect=2
actual=$(( exp2(1) ))
[[ $actual -eq $expect ]] || log_error "exp2 failed" "$expect" "$actual"

# ==========
# expm1
expect=1.71828183
actual=$(roundof $(( expm1(1) )) )
[[ $actual -eq $expect ]] || log_error "expm1 failed" "$expect" "$actual"

# ==========
# fabs
expect=1.0
actual=$(( fabs(-1) ))
[[ $actual -eq $expect ]] || log_error "fabs failed" "$expect" "$actual"

# ==========
# abs
expect=1
actual=$(( abs(-1) ))
[[ $actual -eq $expect ]] || log_error "abs failed" "$expect" "$actual"

# ==========
# fdim - Return positive difference between arguments
expect=0
actual=$(( fdim(1, 3) ))
[[ $actual -eq $expect ]] || log_error "fdim failed" "$expect" "$actual"

# ==========
expect=2
actual=$(( fdim(3, 1) ))
[[ $actual -eq $expect ]] || log_error "fdim failed" "$expect" "$actual"

# ==========
# floor
expect=-2
actual=$(( floor(-1.5) ))
[[ $actual -eq $expect ]] || log_error "floor failed" "$expect" "$actual"

# ==========
# fmax
expect=1.1
actual=$(( fmax(1.0, 1.1) ))
[[ $actual -eq $expect ]] || log_error "fmax failed" "$expect" "$actual"

# ==========
# fmin
expect=1.0
actual=$(( fmin(1.0, 1.1) ))
[[ $actual -eq $expect ]] || log_error "fmin failed" "$expect" "$actual"

# ==========
# fmod
expect=9.99
actual=$(roundof $(( fmod(999.99, 10) )) )
[[ $actual -eq $expect ]] || log_error "fmod failed" "$expect" "$actual"

# ==========
# int
expect=1
actual=$(( int(1.9) ))
[[ $actual -eq $expect ]] || log_error "int failed" "$expect" "$actual"

# ==========
# isgreater
expect=0
actual=$(( isgreater(1, 2) ))
[[ $actual -eq $expect ]] || log_error "isgreater(1, 2) failed" "$expect" "$actual"

# ==========
expect=0
actual=$(( isgreater(2, 2) ))
[[ $actual -eq $expect ]] || log_error "isgreater(2, 2) failed" "$expect" "$actual"

# ==========
expect=1
actual=$(( isgreater(3, 2) ))
[[ $actual -eq $expect ]] || log_error "isgreater(3, 2) failed" "$expect" "$actual"

# ==========
# isless
expect=1
actual=$(( isless(1, 2) ))
[[ $actual -eq $expect ]] || log_error "isless(1, 2) failed" "$expect" "$actual"

# ==========
expect=0
actual=$(( isless(2, 2) ))
[[ $actual -eq $expect ]] || log_error "isless(2, 2) failed" "$expect" "$actual"

# ==========
expect=0
actual=$(( isless(3, 2) ))
[[ $actual -eq $expect ]] || log_error "isless(3, 2) failed" "$expect" "$actual"

# ==========
# iszero
expect=1
actual=$(( iszero(0) ))
[[ $actual -eq $expect ]] || log_error "iszero(0) failed" "$expect" "$actual"

# ==========
expect=0
actual=$(( iszero(1) ))
[[ $actual -eq $expect ]] || log_error "iszero(1) failed" "$expect" "$actual"

# ==========
# log
expect=4.60517019
actual=$(roundof $(( log(100) )) )
[[ $actual -eq $expect ]] || log_error "log failed" "$expect" "$actual"

# ==========
# pow
expect=65536
actual=$(( pow(2, 16) ))
[[ $actual -eq $expect ]] || log_error "pow failed" "$expect" "$actual"

# ==========
# remainder
expect=4
actual=$(( remainder(44, 10) ))
[[ $actual -eq $expect ]] || log_error "remainder failed" "$expect" "$actual"

# ==========
# round
expect=100
actual=$(( round(99.9) ))
[[ $actual -eq $expect ]] || log_error "round failed" "$expect" "$actual"

# ==========
# signbit
# TODO: It returns 512 if number is negative. Is that expected behavior ?

# ==========
# sin
expect=0.89399666
actual=$(roundof $(( sin(90) )) )
[[ $actual -eq $expect ]] || log_error "sin failed" "$expect" "$actual"

# ==========
# sinh
expect=1634508.68623590
actual=$(roundof $(( sinh(15) )) )
[[ $actual -eq $expect ]] || log_error "sinh failed" "$expect" "$actual"

# ==========
# sqrt
expect=40
actual=$(( sqrt(1600) ))
[[ $actual -eq $expect ]] || log_error "sqrt failed" "$expect" "$actual"

# ==========
# tan
expect=1.61977519
actual=$(roundof $(( tan(45) )) )
[[ $actual -eq $expect ]] || log_error "tan failed" "$expect" "$actual"

# ==========
# tanh
expect=1
actual=$(( tanh(45) ))
[[ $actual -eq $expect ]] || log_error "tanh failed" "$expect" "$actual"

# ==========
# trunc
expect=99
actual=$(( trunc(99.9) ))
[[ $actual -eq $expect ]] || log_error "trunc failed" "$expect" "$actual"
