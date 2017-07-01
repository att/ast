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
########################################################################
#                                                                      #
#               This software is part of the ast package               #
#                    Copyright (c) 2012 Roland Mainz                   #
#                      and is licensed under the                       #
#                 Eclipse Public License, Version 1.0                  #
#                    by AT&T Intellectual Property                     #
#                                                                      #
#                A copy of the License is available at                 #
#          http://www.eclipse.org/org/documents/epl-v10.html           #
#         (with md5 checksum b35adb5213ca9657e911e9befb180842)         #
#                                                                      #
#                                                                      #
#                Roland Mainz <roland.mainz@nrubsig.org>               #
#                                                                      #
########################################################################

#
# Copyright (c) 2012, Roland Mainz. All rights reserved.
#

#
# Test module for ".sh.match"
#

#
# This test module tests the .sh.match pattern matching facility
#

# test setup
function err_exit
{
	print -u2 -n '\t'
	print -u2 -r ${Command}[$1]: "${@:2}"
	(( Errors++ ))
}
alias err_exit='err_exit $LINENO'

set -o nounset
Command=${0##*/}
integer Errors=0

typeset ocwd
typeset tmpdir

# create temporary test directory
ocwd="${PWD}"
tmpdir="${ mktemp -t -d 'test_sh_match.XXXXXXXX' ; }" || err_exit 'Cannot create temporary directory.'

cd "${tmpdir}" || { err_exit "cd ${tmpdir} failed." ; exit $((Errors<125?Errors:125)) ; }

# tests
function test_xmlfragment1
{
	typeset -r testscript='test1_script.sh'
cat >"${testscript}" <<-TEST1SCRIPT
	# memory safeguards to prevent out-of-control memory consumption
	ulimit -M \$(( 1024 * 1024 ))
	ulimit -v \$(( 1024 * 1024 ))
	ulimit -d \$(( 1024 * 1024 ))
	
	# input text
	xmltext="\$( < "\$1" )"
	
	print -f "%d characters to process...\\n" "\${#xmltext}"
		
	#
	# parse the XML data
	#
	typeset dummy
	function parse_xmltext
	{
		typeset xmltext="\$2"
		nameref ar="\$1"
	
		# fixme:
		# - We want to enforce standard conformance - does ~(Exp) or ~(Ex-p) does that ?
		dummy="\${xmltext//~(Ex-p)(?:
			(<!--.*-->)+?|			# xml comments
			(<[:_[:alnum:]-]+
				(?: # attributes
					[[:space:]]+
					(?: # four different types of name=value syntax
						(?:[:_[:alnum:]-]+=[^\\"\\'[:space:]]+?)|	#x='foo=bar huz=123'
						(?:[:_[:alnum:]-]+=\\"[^\\"]*?\\")|		#x='foo="ba=r o" huz=123'
						(?:[:_[:alnum:]-]+=\\'[^\\']*?\\')|		#x="foox huz=123"
						(?:[:_[:alnum:]-]+)				#x="foox huz=123"
					)
				)*
				[[:space:]]*
				\\/?	# start tags which are end tags, too (like <foo\\/>)
			>)+?|				# xml start tags
			(<\\/[:_[:alnum:]-]+>)+?|	# xml end tags
			([^<]+)				# xml text
			)/D}"
	
		# copy ".sh.match" to array "ar"
		integer i j
		for i in "\${!.sh.match[@]}" ; do
			for j in "\${!.sh.match[i][@]}" ; do
				[[ -v .sh.match[i][j] ]] && ar[i][j]="\${.sh.match[i][j]}"
			done
		done
	
		return 0
	}
	
	function rebuild_xml_and_verify
	{
		nameref ar="\$1"
		typeset xtext="\$2" # xml text
	
		#
		# rebuild the original text from "ar" (copy of ".sh.match")
		# and compare it to the content of "xtext"
		#
		tmpfile=\$(mktemp)
	
		{
			# rebuild the original text, based on our matches
			nameref nodes_all=ar[0]		# contains all matches
			nameref nodes_comments=ar[1]	# contains only XML comment matches
			nameref nodes_start_tags=ar[2]	# contains only XML start tag matches
			nameref nodes_end_tags=ar[3]	# contains only XML end tag matches
			nameref nodes_text=ar[4]	# contains only XML text matches
			integer i
			for (( i = 0 ; i < \${#nodes_all[@]} ; i++ )) ; do
				[[ -v nodes_comments[i]		]] && printf '%s' "\${nodes_comments[i]}"
				[[ -v nodes_start_tags[i]	]] && printf '%s' "\${nodes_start_tags[i]}"
				[[ -v nodes_end_tags[i]		]] && printf '%s' "\${nodes_end_tags[i]}"
				[[ -v nodes_text[i]		]] && printf '%s' "\${nodes_text[i]}"
			done
			printf '\\n'
		} >"\${tmpfile}"
	
		diff -u <( printf '%s\\n' "\${xtext}") "\${tmpfile}"
		if cmp <( printf '%s\\n' "\${xtext}") "\${tmpfile}" ; then
			printf "#input and output OK (%d characters).\\n" "\$(wc -m <"\${tmpfile}")"
		else
			printf "#difference between input and output found.\\n"
		fi
	
		rm -f "\${tmpfile}"
		return 0
	}
	
	# main
	set -o nounset
	
	typeset -a xar
	parse_xmltext xar "\$xmltext"
	rebuild_xml_and_verify xar "\$xmltext"
TEST1SCRIPT

cat >'testfile1.xml' <<-EOF
	<refentry>
		<refentryinfo>
			<title>&dhtitle;</title>
			<productname>&dhpackage;</productname>
			<releaseinfo role="version">&dhrelease;</releaseinfo>
			<date>&dhdate;</date>
			<authorgroup>
				<author>
					<firstname>XXXX</firstname>
					<surname>YYYYYYYYYYYY</surname>
					<contrib>Wrote this example manpage for the &quot;SunOS Man Page Howto&quot;, available at <ulink url="http://www.YYYYYYYYYYYY.xxx/foo_batt_12345.abcd"/> or <ulink url="http://www.1234.xxx/info/SunOS-mini/123-4567.hhhh"/>.</contrib>
					<address>
						<email>mailmail@YYYYYYYYYYYY.xxx</email>
					</address>
				</author>
				<author>
					<firstname>&dhfirstname;</firstname>
					<surname>&dhsurname;</surname>
					<contrib>Rewrote and extended the example manpage in DocBook XML for the Zebras distribution.</contrib>
					<address>
						<email>&dhemail;</email>
					</address>
				</author>
			</authorgroup>
			<copyright>
				<year>1995</year>
				<year>1996</year>
				<year>1997</year>
				<year>1998</year>
				<year>1999</year>
				<year>2000</year>
				<year>2001</year>
				<year>2002</year>
				<year>2003</year>
				<holder>XXXX YYYYYYYYYYYY</holder>
			</copyright>
			<copyright>
				<year>2006</year>
				<holder>&dhusername;</holder>
			</copyright>
			<legalnotice>
				<para>The Howto containing this example, was offered under the following conditions:</para>
				<para>Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:</para>
				<orderedlist>
					<listitem>
						<para>Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.</para>
					</listitem>
					<listitem>
						<para>Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.</para>
					</listitem>
				</orderedlist>
				<para>THIS SOFTWARE IS PROVIDED BY THE AUTHOR &quot;AS IS&quot; AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.</para>
			</legalnotice>
		</refentryinfo>
		<refmeta>
			<refentrytitle>&dhucpackage;</refentrytitle>
			<manvolnum>&dhsection;</manvolnum>
		</refmeta>
		<refnamediv>
			<refname>&dhpackage;</refname>
			<refpurpose>frobnicate the bar library</refpurpose>
		</refnamediv>
		<refsynopsisdiv>
			<cmdsynopsis>
				<command>&dhpackage;</command>
				<arg choice="opt"><option>-bar</option></arg>
				<group choice="opt">
					<arg choice="plain"><option>-b</option></arg>
					<arg choice="plain"><option>--busy</option></arg>
				</group>
				<group choice="opt">
					<arg choice="plain"><option>-c <replaceable>config-file</replaceable></option></arg>
					<arg choice="plain"><option>--config=<replaceable>config-file</replaceable></option></arg>
				</group>
				<arg choice="opt">
					<group choice="req">
						<arg choice="plain"><option>-e</option></arg>
						<arg choice="plain"><option>--example</option></arg>
					</group>
					<replaceable class="option">this</replaceable>
				</arg>
				<arg choice="opt">
					<group choice="req">
						<arg choice="plain"><option>-e</option></arg>
						<arg choice="plain"><option>--example</option></arg>
					</group>
					<group choice="req">
						<arg choice="plain"><replaceable>this</replaceable></arg>
						<arg choice="plain"><replaceable>that</replaceable></arg>
					</group>
				</arg>
				<arg choice="plain" rep="repeat"><replaceable>file(s)</replaceable></arg>
			</cmdsynopsis>
			<cmdsynopsis>
				<command>&dhpackage;</command>
	      <!-- Normally the help and version options make the programs stop
				     right after outputting the requested information. -->
				<group choice="opt">
					<arg choice="plain">
						<group choice="req">
							<arg choice="plain"><option>-h</option></arg>
							<arg choice="plain"><option>--help</option></arg>
						</group>
					</arg>
					<arg choice="plain">
						<group choice="req">
							<arg choice="plain"><option>-v</option></arg>
							<arg choice="plain"><option>--version</option></arg>
						</group>
					</arg>
				</group>
			</cmdsynopsis>
		</refsynopsisdiv>
		<refsect1 id="description">
			<title>DESCRIPTION</title>
			<para><command>&dhpackage;</command> frobnicates the <application>bar</application> library by tweaking internal symbol tables. By default it parses all baz segments and rearranges them in reverse order by time for the <citerefentry><refentrytitle>xyzzy</refentrytitle><manvolnum>1</manvolnum></citerefentry> linker to find them. The symdef entry is then compressed using the <abbrev>WBG</abbrev> (Whiz-Bang-Gizmo) algorithm. All files are processed in the order specified.</para>
		</refsect1>
		<refsect1 id="options">
			<title>OPTIONS</title>
			<variablelist>
				<!-- Use the variablelist.term.separator and the
				     variablelist.term.break.after parameters to
				     control the term elements. -->
				<varlistentry>
					<term><option>-b</option></term>
					<term><option>--busy</option></term>
					<listitem>
						<para>Do not write <quote>busy</quote> to <filename class="devicefile">stdout</filename> while processing.</para>
					</listitem>
				</varlistentry>
				<varlistentry>
					<term><option>-c <replaceable class="parameter">config-file</replaceable></option></term>
					<term><option>--config=<replaceable class="parameter">config-file</replaceable></option></term>
					<listitem>
						<para>Use the alternate system wide <replaceable>config-file</replaceable> instead of the <filename>/etc/foo.conf</filename>. This overrides any <envar>FOOCONF</envar> environment variable.</para>
					</listitem>
				</varlistentry>
				<varlistentry>
					<term><option>-a</option></term>
					<listitem>
						<para>In addition to the baz segments, also parse the <citerefentry><refentrytitle>blurfl</refentrytitle><manvolnum>3</manvolnum></citerefentry> headers.</para>
					</listitem>
				</varlistentry>
				<varlistentry>
					<term><option>-r</option></term>
					<listitem>
						<para>Recursive mode. Operates as fast as lightning at the expense of a megabyte of virtual memory.</para>
					</listitem>
				</varlistentry>
			</variablelist>
		</refsect1>
		<refsect1 id="files">
			<title>FILES</title>
			<variablelist>
				<varlistentry>
					<term><filename>/etc/foo.conf</filename></term>
					<listitem>
						<para>The system-wide configuration file. See <citerefentry><refentrytitle>foo.conf</refentrytitle><manvolnum>5</manvolnum></citerefentry> for further details.</para>
					</listitem>
				</varlistentry>
				<varlistentry>
					<term><filename>\${HOME}/.foo.conf</filename></term>
					<listitem>
						<para>The per-user configuration file. See <citerefentry><refentrytitle>foo.conf</refentrytitle><manvolnum>5</manvolnum></citerefentry> for further details.</para>
					</listitem>
				</varlistentry>
			</variablelist>
		</refsect1>
		<refsect1 id="environment">
			<title>ENVIONMENT</title>
			<variablelist>
				<varlistentry>
				<term><envar>FOOCONF</envar></term>
					<listitem>
						<para>The full pathname for an alternate system wide configuration file <citerefentry><refentrytitle>foo.conf</refentrytitle><manvolnum>5</manvolnum></citerefentry> (see also <xref linkend="files"/>). Overridden by the <option>-c</option> option.</para>
					</listitem>
				</varlistentry>
			</variablelist>
		</refsect1>
		<refsect1 id="diagnostics">
			<title>DIAGNOSTICS</title>
			<para>The following diagnostics may be issued on <filename class="devicefile">stderr</filename>:</para>
			<variablelist>
				<varlistentry>
					<term><quote><errortext>Bad magic number.</errortext></quote></term>
					<listitem>
						<para>The input file does not look like an archive file.</para>
					</listitem>
				</varlistentry>
				<varlistentry>
					<term><quote><errortext>Old style baz segments.</errortext></quote></term>
					<listitem>
						<para><command>&dhpackage;</command> can only handle new style baz segments. <acronym>COBOL</acronym> object libraries are not supported in this version.</para>
					</listitem>
				</varlistentry>
			</variablelist>
			<para>The following return codes can be used in scripts:</para>
			<segmentedlist>
				<segtitle>Errorcode</segtitle>
				<segtitle>Errortext</segtitle>
				<segtitle>Diagnostic</segtitle>
				<seglistitem>
					<seg><errorcode>0</errorcode></seg>
					<seg><errortext>Program exited normally.</errortext></seg>
					<seg>No error. Program ran successfully.</seg>
				</seglistitem>
				<seglistitem>
					<seg><errorcode>1</errorcode></seg>
					<seg><errortext>Bad magic number.</errortext></seg>
					<seg>The input file does not look like an archive file.</seg>
				</seglistitem>
				<seglistitem>
					<seg><errorcode>2</errorcode></seg>
					<seg><errortext>Old style baz segments.</errortext></seg>
					<seg><command>&dhpackage;</command> can only handle new style baz segments. <acronym>COBOL</acronym> object libraries are not supported in this version.</seg>
				</seglistitem>
			</segmentedlist>
		</refsect1>
		<refsect1 id="bugs">
			<!-- Or use this section to tell about upstream BTS. -->
			<title>BUGS</title>
			<para>The command name should have been chosen more carefully to reflect its purpose.</para>
			<para>The upstreams <acronym>BTS</acronym> can be found at <ulink url="http://bugzilla.foo.tld"/>.</para>
		</refsect1>
		<refsect1 id="see_also">
			<title>SEE ALSO</title>
			<!-- In alpabetical order. -->
			<para><citerefentry>
					<refentrytitle>bar</refentrytitle>
					<manvolnum>1</manvolnum>
				</citerefentry>, <citerefentry>
					<refentrytitle>foo</refentrytitle>
					<manvolnum>1</manvolnum>
				</citerefentry>, <citerefentry>
					<refentrytitle>foo.conf</refentrytitle>
					<manvolnum>5</manvolnum>
				</citerefentry>, <citerefentry>
					<refentrytitle>xyzzy</refentrytitle>
					<manvolnum>1</manvolnum>
				</citerefentry></para>
			<para>The programs are documented fully by <citetitle>The Rise and Fall of a Fooish Bar</citetitle> available via the <application>Info</application> system.</para>
		</refsect1>
	</refentry>
EOF

# Note: Standalone '>' is valid XML text
printf "%s" $'<h1 style=\'nice\' h="bar">> <oook:banana color="<yellow />"><oook:apple-mash color="<green />"><div style="some green"><illegal tag /><br /> a text </div>More [TEXT].<!-- a comment (<disabled>) --></h1>' >'testfile2.xml'

	compound -r -a tests=(
		(
			file='testfile1.xml'
			expected_output=$'9762 characters to process...\n#input and output OK (9763 characters).'
		)
		(
			file='testfile2.xml'
			expected_output=$'201 characters to process...\n#input and output OK (202 characters).'
		)
	)
	compound out=( typeset stdout stderr ; integer res )
	integer i
	typeset expected_output
	typeset testname

	for (( i=0 ; i < ${#tests[@]} ; i++ )) ; do
		nameref tst=tests[i]
		testname="${0}/${i}/${tst.file}"
		expected_output="${tst.expected_output}"

		out.stderr="${ { out.stdout="${ ${SHELL} -o nounset "${testscript}" "${tst.file}" ; (( out.res=$? )) ; }" ; } 2>&1 ; }"

		[[ "${out.stdout}" == "${expected_output}" ]] || err_exit "${testname}: Expected stdout==${ printf '%q\n' "${expected_output}" ;}, got ${ printf '%q\n' "${out.stdout}" ; }"
		[[ "${out.stderr}" == ''		   ]] || err_exit "${testname}: Expected empty stderr, got ${ printf '%q\n' "${out.stderr}" ; }"
		(( out.res == 0 )) || err_exit "${testname}: Unexpected exit code ${out.res}"
	done

	rm "${testscript}"
	rm 'testfile1.xml'
	rm 'testfile2.xml'

	return 0
}

# test whether the [[ -v .sh.match[x][y] ]] operator works, try1
function test_testop_v1
{
	compound out=( typeset stdout stderr ; integer res )
	integer i
	typeset testname
	typeset expected_output

	compound -r -a tests=(
		(
			cmd='s="aaa bbb 333 ccc 555" ; s="${s//~(E)([[:alpha:]]+)|([[:digit:]]+)/NOP}" ;                   [[ -v .sh.match[2][3]   ]] || print "OK"'
			expected_output='OK'
		)
		(
			cmd='s="aaa bbb 333 ccc 555" ; s="${s//~(E)([[:alpha:]]+)|([[:digit:]]+)/NOP}" ; integer i=2 j=3 ; [[ -v .sh.match[$i][$j] ]] || print "OK"'
			expected_output='OK'
		)
		(
			cmd='s="aaa bbb 333 ccc 555" ; s="${s//~(E)([[:alpha:]]+)|([[:digit:]]+)/NOP}" ; integer i=2 j=3 ; [[ -v .sh.match[i][j]   ]] || print "OK"'
			expected_output='OK'
		)
	)

	for (( i=0 ; i < ${#tests[@]} ; i++ )) ; do
		nameref tst=tests[i]
		testname="${0}/${i}/${tst.cmd}"
		expected_output="${tst.expected_output}"

		out.stderr="${ { out.stdout="${ ${SHELL} -o nounset -c "${tst.cmd}" ; (( out.res=$? )) ; }" ; } 2>&1 ; }"

		[[ "${out.stdout}" == "${expected_output}" ]] || err_exit "${testname}: Expected stdout==${ printf '%q\n' "${expected_output}" ;}, got ${ printf '%q\n' "${out.stdout}" ; }"
		[[ "${out.stderr}" == ''		   ]] || err_exit "${testname}: Expected empty stderr, got ${ printf '%q\n' "${out.stderr}" ; }"
		(( out.res == 0 )) || err_exit "${testname}: Unexpected exit code ${out.res}"
	done

	return 0
}

# test whether the [[ -v .sh.match[x][y] ]] operator works, try2
function test_testop_v2
{
	compound out=( typeset stdout stderr ; integer res )
	integer i
	integer j
	integer j
	typeset testname
	typeset cmd

	compound -r -a tests=(
		(
			cmd='s="aaa bbb 333 ccc 555" ; s="${s//~(E)([[:alpha:]]+)|([[:digit:]]+)/NOP}"'
			integer y=6
			expected_output_1d=$'[0]\n[1]\n[2]'
			expected_output_2d=$'[0][0]\n[0][1]\n[0][2]\n[0][3]\n[0][4]\n[1][0]\n[1][1]\n[1][3]\n[2][2]\n[2][4]'
		)
		# FIXME: Add more hideous horror tests here
	)

	for (( i=0 ; i < ${#tests[@]} ; i++ )) ; do
		nameref tst=tests[i]

		#
		# test first dimension, by plain number
		#
		cmd="${tst.cmd}"
		for (( j=0 ; j < tst.y ; j++ )) ; do
			cmd+="; $( printf "[[ -v .sh.match[%d] ]] && print '[%d]'\n" j j )"
		done
		cmd+='; true'

		testname="${0}/${i}/plain_number_index_1d/${cmd}"

		out.stderr="${ { out.stdout="${ ${SHELL} -o nounset -c "${cmd}" ; (( out.res=$? )) ; }" ; } 2>&1 ; }"

		[[ "${out.stdout}" == "${tst.expected_output_1d}" ]] || err_exit "${testname}: Expected stdout==${ printf '%q\n' "${tst.expected_output_1d}" ;}, got ${ printf '%q\n' "${out.stdout}" ; }"
		[[ "${out.stderr}" == ''		   ]] || err_exit "${testname}: Expected empty stderr, got ${ printf '%q\n' "${out.stderr}" ; }"
		(( out.res == 0 )) || err_exit "${testname}: Unexpected exit code ${out.res}"


		#
		# test second dimension, by plain number
		#
		cmd="${tst.cmd}"
		for (( j=0 ; j < tst.y ; j++ )) ; do
			for (( k=0 ; k < tst.y ; k++ )) ; do
				cmd+="; $( printf "[[ -v .sh.match[%d][%d] ]] && print '[%d][%d]'\n" j k j k )"
			done
		done
		cmd+='; true'

		testname="${0}/${i}/plain_number_index_2d/${cmd}"

		out.stderr="${ { out.stdout="${ ${SHELL} -o nounset -c "${cmd}" ; (( out.res=$? )) ; }" ; } 2>&1 ; }"

		[[ "${out.stdout}" == "${tst.expected_output_2d}" ]] || err_exit "${testname}: Expected stdout==${ printf '%q\n' "${tst.expected_output_2d}" ;}, got ${ printf '%q\n' "${out.stdout}" ; }"
		[[ "${out.stderr}" == ''		   ]] || err_exit "${testname}: Expected empty stderr, got ${ printf '%q\n' "${out.stderr}" ; }"
		(( out.res == 0 )) || err_exit "${testname}: Unexpected exit code ${out.res}"

		#
		# test first dimension, by variable index
		#
		cmd="${tst.cmd} ; integer i"
		for (( j=0 ; j < tst.y ; j++ )) ; do
			cmd+="; $( printf "(( i=%d )) ; [[ -v .sh.match[i] ]] && print '[%d]'\n" j j )"
		done
		cmd+='; true'

		testname="${0}/${i}/variable_index_1d/${cmd}"

		out.stderr="${ { out.stdout="${ ${SHELL} -o nounset -c "${cmd}" ; (( out.res=$? )) ; }" ; } 2>&1 ; }"

		[[ "${out.stdout}" == "${tst.expected_output_1d}" ]] || err_exit "${testname}: Expected stdout==${ printf '%q\n' "${tst.expected_output_1d}" ;}, got ${ printf '%q\n' "${out.stdout}" ; }"
		[[ "${out.stderr}" == ''		   ]] || err_exit "${testname}: Expected empty stderr, got ${ printf '%q\n' "${out.stderr}" ; }"
		(( out.res == 0 )) || err_exit "${testname}: Unexpected exit code ${out.res}"


		#
		# test second dimension, by variable index
		#
		cmd="${tst.cmd} ; integer i j"
		for (( j=0 ; j < tst.y ; j++ )) ; do
			for (( k=0 ; k < tst.y ; k++ )) ; do
				cmd+="; $( printf "(( i=%d , j=%d )) ; [[ -v .sh.match[i][j] ]] && print '[%d][%d]'\n" j k j k )"
			done
		done
		cmd+='; true'

		testname="${0}/${i}/variable_index_2d/${cmd}"

		out.stderr="${ { out.stdout="${ ${SHELL} -o nounset -c "${cmd}" ; (( out.res=$? )) ; }" ; } 2>&1 ; }"

		[[ "${out.stdout}" == "${tst.expected_output_2d}" ]] || err_exit "${testname}: Expected stdout==${ printf '%q\n' "${tst.expected_output_2d}" ;}, got ${ printf '%q\n' "${out.stdout}" ; }"
		[[ "${out.stderr}" == ''		   ]] || err_exit "${testname}: Expected empty stderr, got ${ printf '%q\n' "${out.stderr}" ; }"
		(( out.res == 0 )) || err_exit "${testname}: Unexpected exit code ${out.res}"

	done

	return 0
}

# test whether ${#.sh.match[0][@]} returns the right number of elements
function test_num_elements1
{
	compound out=( typeset stdout stderr ; integer res )
	integer i
	typeset testname
	typeset expected_output

	compound -r -a tests=(
		(
			cmd='s="a1a2a3" ; d="${s//~(E)([[:alpha:]])|([[:digit:]])/dummy}" ; printf "num=%d\n" "${#.sh.match[0][@]}"'
			expected_output='num=6'
		)
		(
			cmd='s="ababab" ; d="${s//~(E)([[:alpha:]])|([[:digit:]])/dummy}" ; printf "num=%d\n" "${#.sh.match[0][@]}"'
			expected_output='num=6'
		)
		(
			cmd='s="123456" ; d="${s//~(E)([[:alpha:]])|([[:digit:]])/dummy}" ; printf "num=%d\n" "${#.sh.match[0][@]}"'
			expected_output='num=6'
		)
	)

	for (( i=0 ; i < ${#tests[@]} ; i++ )) ; do
		nameref tst=tests[i]
		testname="${0}/${i}/${tst.cmd}"
		expected_output="${tst.expected_output}"

		out.stderr="${ { out.stdout="${ ${SHELL} -o nounset -c "${tst.cmd}" ; (( out.res=$? )) ; }" ; } 2>&1 ; }"

		[[ "${out.stdout}" == "${expected_output}" ]] || err_exit "${testname}: Expected stdout==${ printf '%q\n' "${expected_output}" ; }, got ${ printf '%q\n' "${out.stdout}" ; }"
		[[ "${out.stderr}" == ''		   ]] || err_exit "${testname}: Expected empty stderr, got ${ printf '%q\n' "${out.stderr}" ; }"
		(( out.res == 0 )) || err_exit "${testname}: Unexpected exit code ${out.res}"
	done

	return 0
}

function test_nomatch
{
	integer j k
	compound c
	compound -a c.attrs

	attrdata=$' x=\'1\' y=\'2\' z="3" end="world"'
	dummy="${attrdata//~(Ex-p)(?:
		[[:space:]]+
		( # four different types of name=value syntax
			(?:([:_[:alnum:]-]+)=([^\"\'[:space:]]+?))|	#x='foo=bar huz=123'
			(?:([:_[:alnum:]-]+)=\"([^\"]*?)\")|		#x='foo="ba=r o" huz=123'
			(?:([:_[:alnum:]-]+)=\'([^\']*?)\')|		#x="foox huz=123"
			(?:([:_[:alnum:]-]+))				#x="foox huz=123"
		)
		)/D}"
	for (( j=0 ; j < ${#.sh.match[0][@]} ; j++ ))
	do
		if [[ -v .sh.match[2][j] && -v .sh.match[3][j] ]]
		then	c.attrs+=( name="${.sh.match[2][j]}" value="${.sh.match[3][j]}" )
		fi
		if [[ -v .sh.match[4][j] && -v .sh.match[5][j] ]]
		then	c.attrs+=( name="${.sh.match[4][j]}" value="${.sh.match[5][j]}" )
		fi
		if [[ -v .sh.match[6][j] && -v .sh.match[7][j] ]] ; then
			c.attrs+=( name="${.sh.match[6][j]}" value="${.sh.match[7][j]}" )
		fi
	done
	expect='(
	typeset -a attrs=(
		[0]=(
			name=x
			value=1
		)
		[1]=(
			name=y
			value=2
		)
		[2]=(
			name=z
			value=3
		)
		[3]=(
			name=end
			value=world
		)
	)
)'
	got=$(print -v c)
	[[ $got == "$expect" ]] || err_exit 'unset submatches not handled correctly'
}

# run tests
test_xmlfragment1
test_testop_v1
test_testop_v2
test_num_elements1
test_nomatch


# cleanup
cd "${ocwd}"
rmdir "${tmpdir}" || err_exit "Cannot remove temporary directory ${tmpdir}."

set +u
x=1234
compound co
: "${x//~(X)([012])|([345])/ }"
x=$(print -v .sh.match)
typeset -m co.array=.sh.match
y=$(print -v co.array)
[[ $y == "$x" ]] || 'typeset -m of .sh.match to variable not working'

# tests done
exit $((Errors<125?Errors:125))
