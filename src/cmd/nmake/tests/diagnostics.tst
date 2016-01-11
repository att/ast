# ast nmake diagnostic message tests

INCLUDE test.def

TEST 01 'mode 000 files'

	DO	touch all
	DO	chmod 000 all

	EXEC	-n
		INPUT Makefile $'
all :
	false'
		OUTPUT - $'+ false'
		ERROR - $'make: warning: all ignored -- not readable
make: warning: all ignored -- not readable'

	EXEC	all
		OUTPUT -
		ERROR - $'make: warning: all ignored -- not readable
+ false
make: *** exit code 1 making all'
		EXIT 1

TEST 02 'silent + ignore'
	
	EXEC	SILENT= IGNORE= COMMAND=true
		INPUT Makefile $'all :
	$(SILENT) $(IGNORE) $(COMMAND)'
		ERROR - $'+ true'
	EXEC	SILENT= IGNORE=ignore COMMAND=true
		ERROR - $'+ ignore true'
	EXEC	SILENT=silent IGNORE= COMMAND=true
		ERROR -
	EXEC	SILENT=silent IGNORE=ignore COMMAND=true

	EXEC	SILENT= IGNORE= COMMAND=false
		ERROR - $'+ false
make: *** exit code 1 making all'
		EXIT 1
	EXEC	SILENT= IGNORE=ignore COMMAND=false
		ERROR - $'+ ignore false'
		EXIT 0
	EXEC	SILENT=silent IGNORE= COMMAND=false
		ERROR - $'make: *** exit code 1 making all'
		EXIT 1
	EXEC	SILENT=silent IGNORE=ignore COMMAND=false
		ERROR -
		EXIT 0

TEST 03 '.ERROR'

	EXEC	-n
		INPUT Makefile $'.ERROR : .FUNCTION
	return 1 -
all : nothing'
		ERROR - $'make: don\'t know how to make all : nothing'

	EXEC	-n
		INPUT Makefile $'.ERROR : .FUNCTION
	return 3 -
all : nothing'
		EXIT 1

	EXEC	-n
		INPUT Makefile $'.ERROR : .FUNCTION
	error 1 level=$(%:O=1) message="$(%:O>1)"
	return 1
all : nothing'
		ERROR - $'make: warning: level=3 message="don\'t know how to make all : nothing"'
		EXIT 0

	EXEC	-n
		INPUT Makefile $'.ERROR : .FUNCTION
	error 1 level=$(%:O=1) message="$(%:O>1)"
	return 3
all : nothing'
		ERROR - $'make: warning: level=3 message="don\'t know how to make all : nothing"'
		EXIT 1

TEST 04 'sh exit'

	EXEC	CODE=0
		INPUT Makefile $'all : beg mid end
beg end :
	: $(<)
mid :
	exit $(CODE)'
		ERROR - $'+ : beg
+ exit 0
+ : end'
	EXEC	CODE=1
		ERROR - $'+ : beg
+ exit 1
make: *** exit code 1 making mid'
		EXIT 1
	EXEC	CODE=63
		ERROR - $'+ : beg
+ exit 63
make: *** exit code 63 making mid'
	EXEC	CODE=65
		ERROR - $'+ : beg
+ exit 65
make: *** exit code 65 making mid'
	EXEC	CODE=127
		ERROR - $'+ : beg
+ exit 127
make: *** exit code 127 making mid'
	EXEC	CODE=129
		ERROR - $'+ : beg
+ exit 129
make: *** termination code 1 making mid'
	EXEC	CODE=255
		ERROR - $'+ : beg
+ exit 255
make: *** termination code 127 making mid'

TEST 05 'make exit'

	EXEC	CODE=0
		INPUT Makefile $'all : beg mid end
beg end :
	: $(<)
mid : .MAKE .VIRTUAL .FORCE
	exit $(CODE)'
		ERROR - $'+ : beg'
	EXEC	CODE=1
		ERROR - $'+ : beg'
		EXIT 1
	EXEC	CODE=63
		ERROR - $'+ : beg'
		EXIT 63
	EXEC	CODE=65
		ERROR - $'+ : beg'
		EXIT 65
	EXEC	CODE=127
		ERROR - $'+ : beg'
		EXIT 127
	EXEC	CODE=129
		ERROR - $'+ : beg'
		EXIT 129
	EXEC	CODE=255
		ERROR - $'+ : beg'
		EXIT 255

TEST 06 'concurrency check'

	EXEC	--regress
		INPUT Makefile $'all : .FORCE
	silent $(MAKE) $(-) $(=)'
		ERROR - $'make: warning: another make is running on Makefile in .
make: use -K to override
make: *** exit code 1 making all'
		EXIT 1

TEST 07 'frozen variable recompile'

	EXEC	--silent WHO=there
		INPUT Makefile $'WHO = world
who := $(WHO)
all :
	echo hello $(who)'
		OUTPUT - $'hello there'

	EXEC	--noexec
		OUTPUT - $'+ echo hello world'
		ERROR - $'make: warning: Makefile.mo: frozen variable WHO changed
make: warning: Makefile.mo: recompiling'

TEST 08 'included file change'

	EXEC
		INPUT Makefile $'include "test.mk"'
		INPUT test.mk $'all :'

	DO	{ sleep 1; touch test.mk; }

	EXEC	--noexec
		ERROR - $'make: warning: Makefile.mo: out of date with test.mk
make: warning: Makefile.mo: recompiling'

TEST 09 'no main target'

	EXEC	-n
		INPUT Makefile $''
		ERROR - $'make: Makefile: a main target must be specified'
		EXIT 1

	EXEC	-n
		INPUT Makefile $'all : .SPECIAL'

TEST 10 'include file missteps'

	EXEC	-n
		INPUT Makefile $'include "foo-dee-bar.mak'
		ERROR - $'make: "Makefile", line 1: EOF in "..." quote starting at line 0
make: "Makefile", line 1: warning: missing closing " quote
make: "Makefile", line 1: foo-dee-bar.mak: cannot read include file
make: Makefile: a main target must be specified'
		EXIT 1

	EXEC	-n
		INPUT Makefile $'include "foo-dee-bar.mak"'
		ERROR - $'make: "Makefile", line 1: foo-dee-bar.mak: cannot read include file
make: Makefile: a main target must be specified'
		EXIT 1

	EXEC	-n
		INPUT Makefile $':FOO_DEE_BAR:'
		INPUT FOO_DEE_BAR.mk $'all:'
		ERROR - $'make: "Makefile", line 1: warning: operator :FOO_DEE_BAR: not defined'
		EXIT 0

TEST 11 'reactions'

	EXEC	-n
		INPUT Makefile $'a :
	one
a :
	one'
		OUTPUT - $'+ one'

	EXEC	-n
		INPUT Makefile $'a :
	one
a :
	two'
		OUTPUT - $'+ two'
		ERROR - $'make: "Makefile", line 4: warning: multiple actions for a'

TEST 12 'memory stress (for a leak now fixed) -- stresses fmt too' && {

	{
		print 'include "test.mk"'
		for ((i=0; i < 1000; i++))
		do	print "x := \$(MAKE_TREE temp.mk dir[$i]=/.)
x := \$(MAKE_TREE temp.mk file[$i]=temp.mk)
x := \$(MAKE_TREE temp.mk owner[$i]=owner)
x := \$(MAKE_TREE temp.mk group[$i]=group)
x := \$(MAKE_TREE temp.mk mode[$i]=755)
x := \$(MAKE_TREE temp.mk package[$i]=EXPORT)"
		done
	} > Makefile

	EXEC
		INPUT test.mk $'all :
	silent echo $(~tree) | fmt -w72
tree : .VIRTUAL
.semaphore.do_tree : .SEMAPHORE
Xdo_tree : .USE .semaphore.do_tree
	set +x
do_tree : .USE .semaphore.do_tree
	set +x
	if [ $(~$(<)_params:N=package=*:O) -gt 1 ]; then
		echo "nmake: error: $(*:@C,^.*/,,): more than one package=" >&2
		false
	fi
	if [ $(~$(<)_params:N=package_default=*:O) -gt 1 ]; then
		echo "nmake: error: $(*:@C,^.*/,,): more than one default package=" >&2
		false
	fi
	PACKAGE=$(~$(<)_params:N=package*:H:O=1:C/^[^=]*=//:C/%/%%/G:C/@/%@/G:C/%@F/$(*:@B)/G:C/%@%@/@/G:C/%@/$(*:@C,^.*/,,)/G:C/%%/%/G)
		PACKAGE=${PACKAGE:-main}
	if [ $(~$(<)_params:N=dir=*:O) -gt 1 ]; then
		echo "nmake: error: $(*:@C,^.*/,,): more than one dir=" >&2
		false
	fi
	if [ $(~$(<)_params:N=dir_default=*:O) -gt 1 ]; then
		echo "nmake: error: $(*:@C,^.*/,,): more than one default dir=" >&2
		false
	fi
	DIR=$(~$(<)_params:N=dir*:H:O=1:C,,/,G:C/^[^=]*=//:C/%/%%/G:C/@/%@/G:C/%@F/$(*:@B)/G:C/%@%@/@/G:C/%@/$(*:@C,^.*/,,)/G:C/%%/%/G)
		DIR=${DIR:?nmake: error: $(*:@C,^.*/,,): missing dir=}
	if [ $(~$(<)_params:N=file=*:O) -gt 1 ]; then
		echo "nmake: error: $(*:@C,^.*/,,): more than one file=" >&2
		false
	fi
	if [ $(~$(<)_params:N=file_default=*:O) -gt 1 ]; then
		echo "nmake: error: $(*:@C,^.*/,,): more than one default file=" >&2
		false
	fi
	FILE=$(~$(<)_params:N=file*:H:O=1:C/^[^=]*=//:C/%/%%/G:C/@/%@/G:C/%@F/$(*:@B)/G:C/%@%@/@/G:C/%@/$(*:@C,^.*/,,)/G:C/%%/%/G)
		FILE=${FILE:?nmake: error: $(*:@C,^.*/,,): missing file=}
	if [ $(~$(<)_params:N=group=*:O) -gt 1 ]; then
		echo "nmake: error: $(*:@C,^.*/,,): more than one group=" >&2
		false
	fi
	if [ $(~$(<)_params:N=group_default=*:O) -gt 1 ]; then
		echo "nmake: error: $(*:@C,^.*/,,): more than one default group=" >&2
		false
	fi
	GROUP=$(~$(<)_params:N=group*:H:O=1:C/^[^=]*=//)
		GROUP=${GROUP:?nmake: error: $(*:@C,^.*/,,): missing group=}
	if [ $(~$(<)_params:N=mode=*:O) -gt 1 ]; then
		echo "nmake: error: $(*:@C,^.*/,,): more than one mode=" >&2
		false
	fi
	if [ $(~$(<)_params:N=mode_default=*:O) -gt 1 ]; then
		echo "nmake: error: $(*:@C,^.*/,,): more than one default mode=" >&2
		false
	fi
	MODE=$(~$(<)_params:N=mode*:H:O=1:C/^[^=]*=//)
		MODE=${MODE:?nmake: error: $(*:@C,^.*/,,): missing mode=}
	if [ $(~$(<)_params:N=owner=*:O) -gt 1 ]; then
		echo "nmake: error: $(*:@C,^.*/,,): more than one owner=" >&2
		false
	fi
	if [ $(~$(<)_params:N=owner_default=*:O) -gt 1 ]; then
		echo "nmake: error: $(*:@C,^.*/,,): more than one default owner=" >&2
		false
	fi
	OWNER=$(~$(<)_params:N=owner*:H:O=1:C/^[^=]*=//)
		OWNER=${OWNER:?nmake: error: $(*:@C,^.*/,,): missing owner=}
	if [ $(~$(<)_params:N=strip=*:O) -gt 1 ]; then
		echo "nmake: error: $(*:@C,^.*/,,): more than one strip=" >&2
		false
	fi
	if [ $(~$(<)_params:N=strip_default=*:O) -gt 1 ]; then
		echo "nmake: error: $(*:@C,^.*/,,): more than one default strip=" >&2
		false
	fi
	STRIP=$(~$(<)_params:N=strip*:H:O=1:C/^[^=]*=//)
		STRIP=${STRIP:-0}
	if [ $(~$(<)_params:N=copy=*:O) -gt 1 ]; then
		echo "nmake: error: $(*:@C,^.*/,,): more than one copy=" >&2
		false
	fi
	if [ $(~$(<)_params:N=copy_default=*:O) -gt 1 ]; then
		echo "nmake: error: $(*:@C,^.*/,,): more than one default copy=" >&2
		false
	fi
	COPY=$(~$(<)_params:N=copy*:H:O=1:C/^[^=]*=//)
		COPY=${COPY:-0}
	if [ $(~$(<)_params:N=part=*:O) -gt 1 ]; then
		echo "nmake: error: $(*:@C,^.*/,,): more than one part=" >&2
		false
	fi
	if [ $(~$(<)_params:N=part_default=*:O) -gt 1 ]; then
		echo "nmake: error: $(*:@C,^.*/,,): more than one default part=" >&2
		false
	fi
	PART=$(~$(<)_params:N=part*:H:O=1:C,^[^=]*=,/,)
		PART=${PART:-/}
	if [ "$PACKAGE" = EXPORT ]; then
		D=$(VROOT)/$PACKAGE$DIR
		SETPERM=$(VROOT)/$PACKAGE/permissions_$PACKAGE.sh
	else
		if [ "$PART" = / ]; then
			D=$(VROOT)/tree/$PACKAGE$DIR
			SETPERM=$(VROOT)/tree/permissions_$PACKAGE.sh
		else
			D=$(VROOT)/EXPORT/TAR/$PACKAGE/$PART$DIR
			SETPERM=$(VROOT)/EXPORT/TAR/$PACKAGE/$PART/set_permissions.sh
		fi
	fi
	: set -x
	if [ ! -d "$D" ]; then
		mkdir -p "${D%/.}"
	fi
	sed \'s/^|//\' <<-! >>"$SETPERM"
		|# begin .$DIR/$FILE
		|X=\\$?
		|chgrp $GROUP .$DIR/$FILE &&
		|chown $OWNER .$DIR/$FILE &&
		|chmod $MODE .$DIR/$FILE &&
		|[ \\$X -eq 0 ]
		|# end .$DIR/$FILE
		|
	!
	if [ "$FILE" != . ]; then
		rm -fr $D/$FILE
		test 0 = "$COPY" && ln "$(*)" $D/$FILE || cp "$(*)" $D/$FILE
		if [ 0 != "$STRIP" ]; then
			strip $D/$FILE || true
		fi
	fi
/* for handling tree rules */
MAKE_TREE : .MAKE .FUNCTIONAL .FORCE .REPEAT
	local N
	if "$(%:O=2)" == "dir?(\\[+([0-9])\\])=/+([-a-zA-Z0-9._@+])*(/+([-a-zA-Z0-9._@+]))"
		/* fall through */
	elif "$(%:O=2)" == "file?(\\[+([0-9])\\])=+([-a-zA-Z0-9._@+])"
		/* fall through */
	elif "$(%:O=2)" == "owner?(\\[+([0-9])\\])=+([-a-zA-Z0-9_])"
		/* fall through */
	elif "$(%:O=2)" == "group?(\\[+([0-9])\\])=+([-a-zA-Z0-9_])"
		/* fall through */
	elif "$(%:O=2)" == "mode?(\\[+([0-9])\\])=+([0-7])"
		/* fall through */
	elif "$(%:O=2)" == "package?(\\[+([0-9])\\])=+([-a-zA-Z0-9._])"
		/* fall through */
	elif "$(%:O=2)" == "strip?(\\[+([0-9])\\])=[01]"
		/* fall through */
	elif "$(%:O=2)" == "copy?(\\[+([0-9])\\])=[01]"
		/* fall through */
	elif "$(%:O=2)" == "part?(\\[+([0-9])\\])=+([-a-zA-Z0-9._])"
		/* fall through */
	elif "$(%:O=2)" == "default?(\\[+([0-9])\\])=[A-Z]*([A-Za-z0-9_])"
		N := $(%:O=2:C/^[a-z]*[^0-9=]*\\([0-9]*\\)[^0-9=]*=\\(.*\\)$/\\1/)
		tree : tree_$(%:O=1)_$(N)
		tree_$(%:O=1)_$(N) : .VIRTUAL .FORCE do_tree $(%:O=3:?$(%:O=3)?$(%:O=1)?)
		$($(%:O=2:C/^default[^=]*=//):C/=/_default=/:C,/,,G) : .MULTIPLE
		tree_$(%:O=1)_$(N)_params : $($(%:O=2:C/^default[^=]*=//):C/=/_default=/:C,/,,G)
		if "$(%:O=4)" != ""
			tree : tree_$(%:O=1)_$(N)_dll
			tree_$(%:O=1)_$(N)_dll : .VIRTUAL .FORCE do_tree $(%:O=4)
			$($(%:O=2:C/^default[^=]*=//):C/=/_default=/:C,/,,G) : .MULTIPLE
			tree_$(%:O=1)_$(N)_dll_params : $($(%:O=2:C/^default[^=]*=//):C/=/_default=/:C,/,,G)
		end
		return OK
	elif "$(%:O=2)" == "[A-Z]*([A-Za-z0-9_])=[A-Z]*([A-Za-z0-9_])"
		/* variable override, already handled */
		return OK
	else
		return ERROR
	end
	N := $(%:O=2:C/^[a-z]*[^0-9=]*\\([0-9]*\\)[^0-9=]*=\\(.*\\)$/\\1/)
	tree : tree_$(%:O=1)_$(N)
	tree_$(%:O=1)_$(N) : .VIRTUAL .FORCE do_tree $(%:O=1)
	$(%:O=2:C,/,,G) : .MULTIPLE
	tree_$(%:O=1)_$(N)_params : $(%:O=2:C,/,,G)
	if "$(%:O=3)" != ""
		tree : tree_$(%:O=1)_$(N)_dll
		tree_$(%:O=1)_$(N)_dll : .VIRTUAL .FORCE do_tree $(%:O=3)
		$(%:O=2:C,/,,G) : .MULTIPLE
		tree_$(%:O=1)_$(N)_dll_params : $(%:O=2:C,/,,G)
	end
	return OK'
		OUTPUT - $'tree_temp.mk_0 tree_temp.mk_1 tree_temp.mk_2 tree_temp.mk_3
tree_temp.mk_4 tree_temp.mk_5 tree_temp.mk_6 tree_temp.mk_7
tree_temp.mk_8 tree_temp.mk_9 tree_temp.mk_10 tree_temp.mk_11
tree_temp.mk_12 tree_temp.mk_13 tree_temp.mk_14 tree_temp.mk_15
tree_temp.mk_16 tree_temp.mk_17 tree_temp.mk_18 tree_temp.mk_19
tree_temp.mk_20 tree_temp.mk_21 tree_temp.mk_22 tree_temp.mk_23
tree_temp.mk_24 tree_temp.mk_25 tree_temp.mk_26 tree_temp.mk_27
tree_temp.mk_28 tree_temp.mk_29 tree_temp.mk_30 tree_temp.mk_31
tree_temp.mk_32 tree_temp.mk_33 tree_temp.mk_34 tree_temp.mk_35
tree_temp.mk_36 tree_temp.mk_37 tree_temp.mk_38 tree_temp.mk_39
tree_temp.mk_40 tree_temp.mk_41 tree_temp.mk_42 tree_temp.mk_43
tree_temp.mk_44 tree_temp.mk_45 tree_temp.mk_46 tree_temp.mk_47
tree_temp.mk_48 tree_temp.mk_49 tree_temp.mk_50 tree_temp.mk_51
tree_temp.mk_52 tree_temp.mk_53 tree_temp.mk_54 tree_temp.mk_55
tree_temp.mk_56 tree_temp.mk_57 tree_temp.mk_58 tree_temp.mk_59
tree_temp.mk_60 tree_temp.mk_61 tree_temp.mk_62 tree_temp.mk_63
tree_temp.mk_64 tree_temp.mk_65 tree_temp.mk_66 tree_temp.mk_67
tree_temp.mk_68 tree_temp.mk_69 tree_temp.mk_70 tree_temp.mk_71
tree_temp.mk_72 tree_temp.mk_73 tree_temp.mk_74 tree_temp.mk_75
tree_temp.mk_76 tree_temp.mk_77 tree_temp.mk_78 tree_temp.mk_79
tree_temp.mk_80 tree_temp.mk_81 tree_temp.mk_82 tree_temp.mk_83
tree_temp.mk_84 tree_temp.mk_85 tree_temp.mk_86 tree_temp.mk_87
tree_temp.mk_88 tree_temp.mk_89 tree_temp.mk_90 tree_temp.mk_91
tree_temp.mk_92 tree_temp.mk_93 tree_temp.mk_94 tree_temp.mk_95
tree_temp.mk_96 tree_temp.mk_97 tree_temp.mk_98 tree_temp.mk_99
tree_temp.mk_100 tree_temp.mk_101 tree_temp.mk_102 tree_temp.mk_103
tree_temp.mk_104 tree_temp.mk_105 tree_temp.mk_106 tree_temp.mk_107
tree_temp.mk_108 tree_temp.mk_109 tree_temp.mk_110 tree_temp.mk_111
tree_temp.mk_112 tree_temp.mk_113 tree_temp.mk_114 tree_temp.mk_115
tree_temp.mk_116 tree_temp.mk_117 tree_temp.mk_118 tree_temp.mk_119
tree_temp.mk_120 tree_temp.mk_121 tree_temp.mk_122 tree_temp.mk_123
tree_temp.mk_124 tree_temp.mk_125 tree_temp.mk_126 tree_temp.mk_127
tree_temp.mk_128 tree_temp.mk_129 tree_temp.mk_130 tree_temp.mk_131
tree_temp.mk_132 tree_temp.mk_133 tree_temp.mk_134 tree_temp.mk_135
tree_temp.mk_136 tree_temp.mk_137 tree_temp.mk_138 tree_temp.mk_139
tree_temp.mk_140 tree_temp.mk_141 tree_temp.mk_142 tree_temp.mk_143
tree_temp.mk_144 tree_temp.mk_145 tree_temp.mk_146 tree_temp.mk_147
tree_temp.mk_148 tree_temp.mk_149 tree_temp.mk_150 tree_temp.mk_151
tree_temp.mk_152 tree_temp.mk_153 tree_temp.mk_154 tree_temp.mk_155
tree_temp.mk_156 tree_temp.mk_157 tree_temp.mk_158 tree_temp.mk_159
tree_temp.mk_160 tree_temp.mk_161 tree_temp.mk_162 tree_temp.mk_163
tree_temp.mk_164 tree_temp.mk_165 tree_temp.mk_166 tree_temp.mk_167
tree_temp.mk_168 tree_temp.mk_169 tree_temp.mk_170 tree_temp.mk_171
tree_temp.mk_172 tree_temp.mk_173 tree_temp.mk_174 tree_temp.mk_175
tree_temp.mk_176 tree_temp.mk_177 tree_temp.mk_178 tree_temp.mk_179
tree_temp.mk_180 tree_temp.mk_181 tree_temp.mk_182 tree_temp.mk_183
tree_temp.mk_184 tree_temp.mk_185 tree_temp.mk_186 tree_temp.mk_187
tree_temp.mk_188 tree_temp.mk_189 tree_temp.mk_190 tree_temp.mk_191
tree_temp.mk_192 tree_temp.mk_193 tree_temp.mk_194 tree_temp.mk_195
tree_temp.mk_196 tree_temp.mk_197 tree_temp.mk_198 tree_temp.mk_199
tree_temp.mk_200 tree_temp.mk_201 tree_temp.mk_202 tree_temp.mk_203
tree_temp.mk_204 tree_temp.mk_205 tree_temp.mk_206 tree_temp.mk_207
tree_temp.mk_208 tree_temp.mk_209 tree_temp.mk_210 tree_temp.mk_211
tree_temp.mk_212 tree_temp.mk_213 tree_temp.mk_214 tree_temp.mk_215
tree_temp.mk_216 tree_temp.mk_217 tree_temp.mk_218 tree_temp.mk_219
tree_temp.mk_220 tree_temp.mk_221 tree_temp.mk_222 tree_temp.mk_223
tree_temp.mk_224 tree_temp.mk_225 tree_temp.mk_226 tree_temp.mk_227
tree_temp.mk_228 tree_temp.mk_229 tree_temp.mk_230 tree_temp.mk_231
tree_temp.mk_232 tree_temp.mk_233 tree_temp.mk_234 tree_temp.mk_235
tree_temp.mk_236 tree_temp.mk_237 tree_temp.mk_238 tree_temp.mk_239
tree_temp.mk_240 tree_temp.mk_241 tree_temp.mk_242 tree_temp.mk_243
tree_temp.mk_244 tree_temp.mk_245 tree_temp.mk_246 tree_temp.mk_247
tree_temp.mk_248 tree_temp.mk_249 tree_temp.mk_250 tree_temp.mk_251
tree_temp.mk_252 tree_temp.mk_253 tree_temp.mk_254 tree_temp.mk_255
tree_temp.mk_256 tree_temp.mk_257 tree_temp.mk_258 tree_temp.mk_259
tree_temp.mk_260 tree_temp.mk_261 tree_temp.mk_262 tree_temp.mk_263
tree_temp.mk_264 tree_temp.mk_265 tree_temp.mk_266 tree_temp.mk_267
tree_temp.mk_268 tree_temp.mk_269 tree_temp.mk_270 tree_temp.mk_271
tree_temp.mk_272 tree_temp.mk_273 tree_temp.mk_274 tree_temp.mk_275
tree_temp.mk_276 tree_temp.mk_277 tree_temp.mk_278 tree_temp.mk_279
tree_temp.mk_280 tree_temp.mk_281 tree_temp.mk_282 tree_temp.mk_283
tree_temp.mk_284 tree_temp.mk_285 tree_temp.mk_286 tree_temp.mk_287
tree_temp.mk_288 tree_temp.mk_289 tree_temp.mk_290 tree_temp.mk_291
tree_temp.mk_292 tree_temp.mk_293 tree_temp.mk_294 tree_temp.mk_295
tree_temp.mk_296 tree_temp.mk_297 tree_temp.mk_298 tree_temp.mk_299
tree_temp.mk_300 tree_temp.mk_301 tree_temp.mk_302 tree_temp.mk_303
tree_temp.mk_304 tree_temp.mk_305 tree_temp.mk_306 tree_temp.mk_307
tree_temp.mk_308 tree_temp.mk_309 tree_temp.mk_310 tree_temp.mk_311
tree_temp.mk_312 tree_temp.mk_313 tree_temp.mk_314 tree_temp.mk_315
tree_temp.mk_316 tree_temp.mk_317 tree_temp.mk_318 tree_temp.mk_319
tree_temp.mk_320 tree_temp.mk_321 tree_temp.mk_322 tree_temp.mk_323
tree_temp.mk_324 tree_temp.mk_325 tree_temp.mk_326 tree_temp.mk_327
tree_temp.mk_328 tree_temp.mk_329 tree_temp.mk_330 tree_temp.mk_331
tree_temp.mk_332 tree_temp.mk_333 tree_temp.mk_334 tree_temp.mk_335
tree_temp.mk_336 tree_temp.mk_337 tree_temp.mk_338 tree_temp.mk_339
tree_temp.mk_340 tree_temp.mk_341 tree_temp.mk_342 tree_temp.mk_343
tree_temp.mk_344 tree_temp.mk_345 tree_temp.mk_346 tree_temp.mk_347
tree_temp.mk_348 tree_temp.mk_349 tree_temp.mk_350 tree_temp.mk_351
tree_temp.mk_352 tree_temp.mk_353 tree_temp.mk_354 tree_temp.mk_355
tree_temp.mk_356 tree_temp.mk_357 tree_temp.mk_358 tree_temp.mk_359
tree_temp.mk_360 tree_temp.mk_361 tree_temp.mk_362 tree_temp.mk_363
tree_temp.mk_364 tree_temp.mk_365 tree_temp.mk_366 tree_temp.mk_367
tree_temp.mk_368 tree_temp.mk_369 tree_temp.mk_370 tree_temp.mk_371
tree_temp.mk_372 tree_temp.mk_373 tree_temp.mk_374 tree_temp.mk_375
tree_temp.mk_376 tree_temp.mk_377 tree_temp.mk_378 tree_temp.mk_379
tree_temp.mk_380 tree_temp.mk_381 tree_temp.mk_382 tree_temp.mk_383
tree_temp.mk_384 tree_temp.mk_385 tree_temp.mk_386 tree_temp.mk_387
tree_temp.mk_388 tree_temp.mk_389 tree_temp.mk_390 tree_temp.mk_391
tree_temp.mk_392 tree_temp.mk_393 tree_temp.mk_394 tree_temp.mk_395
tree_temp.mk_396 tree_temp.mk_397 tree_temp.mk_398 tree_temp.mk_399
tree_temp.mk_400 tree_temp.mk_401 tree_temp.mk_402 tree_temp.mk_403
tree_temp.mk_404 tree_temp.mk_405 tree_temp.mk_406 tree_temp.mk_407
tree_temp.mk_408 tree_temp.mk_409 tree_temp.mk_410 tree_temp.mk_411
tree_temp.mk_412 tree_temp.mk_413 tree_temp.mk_414 tree_temp.mk_415
tree_temp.mk_416 tree_temp.mk_417 tree_temp.mk_418 tree_temp.mk_419
tree_temp.mk_420 tree_temp.mk_421 tree_temp.mk_422 tree_temp.mk_423
tree_temp.mk_424 tree_temp.mk_425 tree_temp.mk_426 tree_temp.mk_427
tree_temp.mk_428 tree_temp.mk_429 tree_temp.mk_430 tree_temp.mk_431
tree_temp.mk_432 tree_temp.mk_433 tree_temp.mk_434 tree_temp.mk_435
tree_temp.mk_436 tree_temp.mk_437 tree_temp.mk_438 tree_temp.mk_439
tree_temp.mk_440 tree_temp.mk_441 tree_temp.mk_442 tree_temp.mk_443
tree_temp.mk_444 tree_temp.mk_445 tree_temp.mk_446 tree_temp.mk_447
tree_temp.mk_448 tree_temp.mk_449 tree_temp.mk_450 tree_temp.mk_451
tree_temp.mk_452 tree_temp.mk_453 tree_temp.mk_454 tree_temp.mk_455
tree_temp.mk_456 tree_temp.mk_457 tree_temp.mk_458 tree_temp.mk_459
tree_temp.mk_460 tree_temp.mk_461 tree_temp.mk_462 tree_temp.mk_463
tree_temp.mk_464 tree_temp.mk_465 tree_temp.mk_466 tree_temp.mk_467
tree_temp.mk_468 tree_temp.mk_469 tree_temp.mk_470 tree_temp.mk_471
tree_temp.mk_472 tree_temp.mk_473 tree_temp.mk_474 tree_temp.mk_475
tree_temp.mk_476 tree_temp.mk_477 tree_temp.mk_478 tree_temp.mk_479
tree_temp.mk_480 tree_temp.mk_481 tree_temp.mk_482 tree_temp.mk_483
tree_temp.mk_484 tree_temp.mk_485 tree_temp.mk_486 tree_temp.mk_487
tree_temp.mk_488 tree_temp.mk_489 tree_temp.mk_490 tree_temp.mk_491
tree_temp.mk_492 tree_temp.mk_493 tree_temp.mk_494 tree_temp.mk_495
tree_temp.mk_496 tree_temp.mk_497 tree_temp.mk_498 tree_temp.mk_499
tree_temp.mk_500 tree_temp.mk_501 tree_temp.mk_502 tree_temp.mk_503
tree_temp.mk_504 tree_temp.mk_505 tree_temp.mk_506 tree_temp.mk_507
tree_temp.mk_508 tree_temp.mk_509 tree_temp.mk_510 tree_temp.mk_511
tree_temp.mk_512 tree_temp.mk_513 tree_temp.mk_514 tree_temp.mk_515
tree_temp.mk_516 tree_temp.mk_517 tree_temp.mk_518 tree_temp.mk_519
tree_temp.mk_520 tree_temp.mk_521 tree_temp.mk_522 tree_temp.mk_523
tree_temp.mk_524 tree_temp.mk_525 tree_temp.mk_526 tree_temp.mk_527
tree_temp.mk_528 tree_temp.mk_529 tree_temp.mk_530 tree_temp.mk_531
tree_temp.mk_532 tree_temp.mk_533 tree_temp.mk_534 tree_temp.mk_535
tree_temp.mk_536 tree_temp.mk_537 tree_temp.mk_538 tree_temp.mk_539
tree_temp.mk_540 tree_temp.mk_541 tree_temp.mk_542 tree_temp.mk_543
tree_temp.mk_544 tree_temp.mk_545 tree_temp.mk_546 tree_temp.mk_547
tree_temp.mk_548 tree_temp.mk_549 tree_temp.mk_550 tree_temp.mk_551
tree_temp.mk_552 tree_temp.mk_553 tree_temp.mk_554 tree_temp.mk_555
tree_temp.mk_556 tree_temp.mk_557 tree_temp.mk_558 tree_temp.mk_559
tree_temp.mk_560 tree_temp.mk_561 tree_temp.mk_562 tree_temp.mk_563
tree_temp.mk_564 tree_temp.mk_565 tree_temp.mk_566 tree_temp.mk_567
tree_temp.mk_568 tree_temp.mk_569 tree_temp.mk_570 tree_temp.mk_571
tree_temp.mk_572 tree_temp.mk_573 tree_temp.mk_574 tree_temp.mk_575
tree_temp.mk_576 tree_temp.mk_577 tree_temp.mk_578 tree_temp.mk_579
tree_temp.mk_580 tree_temp.mk_581 tree_temp.mk_582 tree_temp.mk_583
tree_temp.mk_584 tree_temp.mk_585 tree_temp.mk_586 tree_temp.mk_587
tree_temp.mk_588 tree_temp.mk_589 tree_temp.mk_590 tree_temp.mk_591
tree_temp.mk_592 tree_temp.mk_593 tree_temp.mk_594 tree_temp.mk_595
tree_temp.mk_596 tree_temp.mk_597 tree_temp.mk_598 tree_temp.mk_599
tree_temp.mk_600 tree_temp.mk_601 tree_temp.mk_602 tree_temp.mk_603
tree_temp.mk_604 tree_temp.mk_605 tree_temp.mk_606 tree_temp.mk_607
tree_temp.mk_608 tree_temp.mk_609 tree_temp.mk_610 tree_temp.mk_611
tree_temp.mk_612 tree_temp.mk_613 tree_temp.mk_614 tree_temp.mk_615
tree_temp.mk_616 tree_temp.mk_617 tree_temp.mk_618 tree_temp.mk_619
tree_temp.mk_620 tree_temp.mk_621 tree_temp.mk_622 tree_temp.mk_623
tree_temp.mk_624 tree_temp.mk_625 tree_temp.mk_626 tree_temp.mk_627
tree_temp.mk_628 tree_temp.mk_629 tree_temp.mk_630 tree_temp.mk_631
tree_temp.mk_632 tree_temp.mk_633 tree_temp.mk_634 tree_temp.mk_635
tree_temp.mk_636 tree_temp.mk_637 tree_temp.mk_638 tree_temp.mk_639
tree_temp.mk_640 tree_temp.mk_641 tree_temp.mk_642 tree_temp.mk_643
tree_temp.mk_644 tree_temp.mk_645 tree_temp.mk_646 tree_temp.mk_647
tree_temp.mk_648 tree_temp.mk_649 tree_temp.mk_650 tree_temp.mk_651
tree_temp.mk_652 tree_temp.mk_653 tree_temp.mk_654 tree_temp.mk_655
tree_temp.mk_656 tree_temp.mk_657 tree_temp.mk_658 tree_temp.mk_659
tree_temp.mk_660 tree_temp.mk_661 tree_temp.mk_662 tree_temp.mk_663
tree_temp.mk_664 tree_temp.mk_665 tree_temp.mk_666 tree_temp.mk_667
tree_temp.mk_668 tree_temp.mk_669 tree_temp.mk_670 tree_temp.mk_671
tree_temp.mk_672 tree_temp.mk_673 tree_temp.mk_674 tree_temp.mk_675
tree_temp.mk_676 tree_temp.mk_677 tree_temp.mk_678 tree_temp.mk_679
tree_temp.mk_680 tree_temp.mk_681 tree_temp.mk_682 tree_temp.mk_683
tree_temp.mk_684 tree_temp.mk_685 tree_temp.mk_686 tree_temp.mk_687
tree_temp.mk_688 tree_temp.mk_689 tree_temp.mk_690 tree_temp.mk_691
tree_temp.mk_692 tree_temp.mk_693 tree_temp.mk_694 tree_temp.mk_695
tree_temp.mk_696 tree_temp.mk_697 tree_temp.mk_698 tree_temp.mk_699
tree_temp.mk_700 tree_temp.mk_701 tree_temp.mk_702 tree_temp.mk_703
tree_temp.mk_704 tree_temp.mk_705 tree_temp.mk_706 tree_temp.mk_707
tree_temp.mk_708 tree_temp.mk_709 tree_temp.mk_710 tree_temp.mk_711
tree_temp.mk_712 tree_temp.mk_713 tree_temp.mk_714 tree_temp.mk_715
tree_temp.mk_716 tree_temp.mk_717 tree_temp.mk_718 tree_temp.mk_719
tree_temp.mk_720 tree_temp.mk_721 tree_temp.mk_722 tree_temp.mk_723
tree_temp.mk_724 tree_temp.mk_725 tree_temp.mk_726 tree_temp.mk_727
tree_temp.mk_728 tree_temp.mk_729 tree_temp.mk_730 tree_temp.mk_731
tree_temp.mk_732 tree_temp.mk_733 tree_temp.mk_734 tree_temp.mk_735
tree_temp.mk_736 tree_temp.mk_737 tree_temp.mk_738 tree_temp.mk_739
tree_temp.mk_740 tree_temp.mk_741 tree_temp.mk_742 tree_temp.mk_743
tree_temp.mk_744 tree_temp.mk_745 tree_temp.mk_746 tree_temp.mk_747
tree_temp.mk_748 tree_temp.mk_749 tree_temp.mk_750 tree_temp.mk_751
tree_temp.mk_752 tree_temp.mk_753 tree_temp.mk_754 tree_temp.mk_755
tree_temp.mk_756 tree_temp.mk_757 tree_temp.mk_758 tree_temp.mk_759
tree_temp.mk_760 tree_temp.mk_761 tree_temp.mk_762 tree_temp.mk_763
tree_temp.mk_764 tree_temp.mk_765 tree_temp.mk_766 tree_temp.mk_767
tree_temp.mk_768 tree_temp.mk_769 tree_temp.mk_770 tree_temp.mk_771
tree_temp.mk_772 tree_temp.mk_773 tree_temp.mk_774 tree_temp.mk_775
tree_temp.mk_776 tree_temp.mk_777 tree_temp.mk_778 tree_temp.mk_779
tree_temp.mk_780 tree_temp.mk_781 tree_temp.mk_782 tree_temp.mk_783
tree_temp.mk_784 tree_temp.mk_785 tree_temp.mk_786 tree_temp.mk_787
tree_temp.mk_788 tree_temp.mk_789 tree_temp.mk_790 tree_temp.mk_791
tree_temp.mk_792 tree_temp.mk_793 tree_temp.mk_794 tree_temp.mk_795
tree_temp.mk_796 tree_temp.mk_797 tree_temp.mk_798 tree_temp.mk_799
tree_temp.mk_800 tree_temp.mk_801 tree_temp.mk_802 tree_temp.mk_803
tree_temp.mk_804 tree_temp.mk_805 tree_temp.mk_806 tree_temp.mk_807
tree_temp.mk_808 tree_temp.mk_809 tree_temp.mk_810 tree_temp.mk_811
tree_temp.mk_812 tree_temp.mk_813 tree_temp.mk_814 tree_temp.mk_815
tree_temp.mk_816 tree_temp.mk_817 tree_temp.mk_818 tree_temp.mk_819
tree_temp.mk_820 tree_temp.mk_821 tree_temp.mk_822 tree_temp.mk_823
tree_temp.mk_824 tree_temp.mk_825 tree_temp.mk_826 tree_temp.mk_827
tree_temp.mk_828 tree_temp.mk_829 tree_temp.mk_830 tree_temp.mk_831
tree_temp.mk_832 tree_temp.mk_833 tree_temp.mk_834 tree_temp.mk_835
tree_temp.mk_836 tree_temp.mk_837 tree_temp.mk_838 tree_temp.mk_839
tree_temp.mk_840 tree_temp.mk_841 tree_temp.mk_842 tree_temp.mk_843
tree_temp.mk_844 tree_temp.mk_845 tree_temp.mk_846 tree_temp.mk_847
tree_temp.mk_848 tree_temp.mk_849 tree_temp.mk_850 tree_temp.mk_851
tree_temp.mk_852 tree_temp.mk_853 tree_temp.mk_854 tree_temp.mk_855
tree_temp.mk_856 tree_temp.mk_857 tree_temp.mk_858 tree_temp.mk_859
tree_temp.mk_860 tree_temp.mk_861 tree_temp.mk_862 tree_temp.mk_863
tree_temp.mk_864 tree_temp.mk_865 tree_temp.mk_866 tree_temp.mk_867
tree_temp.mk_868 tree_temp.mk_869 tree_temp.mk_870 tree_temp.mk_871
tree_temp.mk_872 tree_temp.mk_873 tree_temp.mk_874 tree_temp.mk_875
tree_temp.mk_876 tree_temp.mk_877 tree_temp.mk_878 tree_temp.mk_879
tree_temp.mk_880 tree_temp.mk_881 tree_temp.mk_882 tree_temp.mk_883
tree_temp.mk_884 tree_temp.mk_885 tree_temp.mk_886 tree_temp.mk_887
tree_temp.mk_888 tree_temp.mk_889 tree_temp.mk_890 tree_temp.mk_891
tree_temp.mk_892 tree_temp.mk_893 tree_temp.mk_894 tree_temp.mk_895
tree_temp.mk_896 tree_temp.mk_897 tree_temp.mk_898 tree_temp.mk_899
tree_temp.mk_900 tree_temp.mk_901 tree_temp.mk_902 tree_temp.mk_903
tree_temp.mk_904 tree_temp.mk_905 tree_temp.mk_906 tree_temp.mk_907
tree_temp.mk_908 tree_temp.mk_909 tree_temp.mk_910 tree_temp.mk_911
tree_temp.mk_912 tree_temp.mk_913 tree_temp.mk_914 tree_temp.mk_915
tree_temp.mk_916 tree_temp.mk_917 tree_temp.mk_918 tree_temp.mk_919
tree_temp.mk_920 tree_temp.mk_921 tree_temp.mk_922 tree_temp.mk_923
tree_temp.mk_924 tree_temp.mk_925 tree_temp.mk_926 tree_temp.mk_927
tree_temp.mk_928 tree_temp.mk_929 tree_temp.mk_930 tree_temp.mk_931
tree_temp.mk_932 tree_temp.mk_933 tree_temp.mk_934 tree_temp.mk_935
tree_temp.mk_936 tree_temp.mk_937 tree_temp.mk_938 tree_temp.mk_939
tree_temp.mk_940 tree_temp.mk_941 tree_temp.mk_942 tree_temp.mk_943
tree_temp.mk_944 tree_temp.mk_945 tree_temp.mk_946 tree_temp.mk_947
tree_temp.mk_948 tree_temp.mk_949 tree_temp.mk_950 tree_temp.mk_951
tree_temp.mk_952 tree_temp.mk_953 tree_temp.mk_954 tree_temp.mk_955
tree_temp.mk_956 tree_temp.mk_957 tree_temp.mk_958 tree_temp.mk_959
tree_temp.mk_960 tree_temp.mk_961 tree_temp.mk_962 tree_temp.mk_963
tree_temp.mk_964 tree_temp.mk_965 tree_temp.mk_966 tree_temp.mk_967
tree_temp.mk_968 tree_temp.mk_969 tree_temp.mk_970 tree_temp.mk_971
tree_temp.mk_972 tree_temp.mk_973 tree_temp.mk_974 tree_temp.mk_975
tree_temp.mk_976 tree_temp.mk_977 tree_temp.mk_978 tree_temp.mk_979
tree_temp.mk_980 tree_temp.mk_981 tree_temp.mk_982 tree_temp.mk_983
tree_temp.mk_984 tree_temp.mk_985 tree_temp.mk_986 tree_temp.mk_987
tree_temp.mk_988 tree_temp.mk_989 tree_temp.mk_990 tree_temp.mk_991
tree_temp.mk_992 tree_temp.mk_993 tree_temp.mk_994 tree_temp.mk_995
tree_temp.mk_996 tree_temp.mk_997 tree_temp.mk_998 tree_temp.mk_999'

}

TEST 13 'self-documentation'

	EXEC	--???man
		ERROR - $'version=1'
		EXIT 2

	EXEC	-f - --???man

	EXEC	--short
		ERROR - $'Usage: make [-AabCcJnxeFiKGklNRrOsVtvwP] [-B level] [-X[action]] [-d level]
            [-E id] [-f file] [-g[file]] [-I directory] [-j level]
            [-M type[,subtype][:file[:parent[:directory]]]] [-Q mask]
            [-S[level]] [-q[action]] [-T mask] [-z seconds] [-o name[=value]]
            [-D name[=value]] [-U name] [ script ... ] [ target ... ]'

	EXEC	-f - --short

	EXEC	--long
		ERROR - $'Usage: make [--accept] [--alias] [--base] [--believe=level] [--compatibility]
            [--compile] [--corrupt[=action]] [--cross] [--debug=level]
            [--errorid=id] [--exec] [--expandview] [--explain] [--file=file]
            [--force] [--global[=file]] [--ignore] [--ignorelock]
            [--include=directory] [--intermediate] [--jobs=level] [--keepgoing]
            [--list] [--mam=type[,subtype][:file[:parent[:directory]]]]
            [--never] [--option=\'char;name;flags;set;description;values\']
            [--override] [--questionable=mask] [--readonly]
            [--readstate[=level]] [--regress[=action]] [--reread] [--ruledump]
            [--scan] [--serialize] [--silent] [--strictview] [--target-context]
            [--target-prefix=separator] [--test=mask] [--tolerance=seconds]
            [--touch] [--vardump] [--warn] [--writeobject[=file]]
            [--writestate[=file]] [--byname=name[=value]]
            [--define=name[=value]] [--preprocess] [--undef=name]
            [--all-static] [--ancestor=depth]
            [--ancestor-source=.SOURCE.suffix directory...]
            [--archive-clean=edit-ops] [--archive-output=file] [--cctype=type]
            [--clean-ignore=pattern] [--clobber[=pattern]] [--compare]
            [--debug-symbols] [--force-shared] [--instrument=command]
            [--ld-script=suffix] [--lib-type] [--link=pattern] [--local-static]
            [--native-pp=level] [--official-output=file] [--prefix-include]
            [--preserve[=pattern]] [--profile] [--recurse=action]
            [--recurse-enter=text] [--recurse-leave=text] [--select=edit-ops]
            [--separate-include] [--shared] [--static-link] [--strip-symbols]
            [--threads] [--variants[=pattern]] [--view-verify=level]
            [--virtual] [ script ... ] [ target ... ]'

	EXEC	-f - --long

	EXEC	--?file
		ERROR - $'make: [ options ] [ script ... ] [ target ... ]
OPTIONS
  -f, --file=file Read the makefile file. If --file is not specified then the
                  makefile names specified by $(MAKEFILES) are attempted in
                  order from left to right. The file - is equivalent to
                  /dev/null.'

	EXEC	-f - --?file

	EXEC	--?clobber
		ERROR - $'make: [ options ] [ script ... ] [ target ... ]
OPTIONS
  --clobber[=pattern]
                  (Makerules) Replace existing install action targets matching
                  pattern instead of renaming to target.old. If the option
                  value is omitted then * is assumed.'

	EXEC	-f - --?clobber

	EXEC	--unknown-option
		ERROR - $'make: unknown-option: unknown option'

	EXEC	-f - --unknown-option

	EXEC	-f
		ERROR - $'make: -f: file argument expected'

	EXEC	--file
		ERROR - $'make: file: file value expected'

TEST 14 'convoluted include + global makefiles'

	EXEC	--regress=sync
		INPUT Makefile $'include "foo/foo.mk"
all :
	: $(<) : $(~) :
foo FOO bar BAR global GLOBAL : .VIRTUAL'
		INPUT foo/foo.mk $'all : foo
.SOURCE.mk : bar
include "bar.mk"'
		INPUT bar/bar.mk $'all : bar'
		ERROR - $'+ : all : foo bar :'

	EXEC	--noexec
		OUTPUT - $'+ : all : foo bar :'
		ERROR -

	EXEC	--noexec
		INPUT bar/bar.mk $'all : BAR'
		OUTPUT - $'+ : all : foo BAR :'
		ERROR - $'make: warning: bar.mk: binding changed to bar.mk from bar/bar.mk
make: warning: Makefile.mo: recompiling'

	EXEC	--exec
		OUTPUT -
		ERROR - $'+ : all : foo BAR :'

	EXEC	--noexec
		OUTPUT - $'+ : all : foo BAR :'
		ERROR -

	EXEC	--noexec
		INPUT foo/foo.mk $'all : FOO
.SOURCE.mk : bar
include "bar.mk"'
		OUTPUT - $'+ : all : FOO BAR :'
		ERROR - $'make: warning: Makefile.mo: out of date with foo/foo.mk
make: warning: Makefile.mo: recompiling'

	EXEC	--exec
		OUTPUT -
		ERROR - $'+ : all : FOO BAR :'

	EXEC	--noexec --include=gbl --global=global.mk
		INPUT gbl/global.mk $'all : global'
		OUTPUT - $'+ : all : global FOO BAR :'
		ERROR - $'make: warning: Makefile.mo: global file global.mk not specified last time
make: warning: Makefile.mo: recompiling'

	EXEC	--exec --include=gbl --global=global.mk
		OUTPUT -
		ERROR - $'+ : all : global FOO BAR :'

	EXEC	--regress=sync --noexec --include=gbl --global=global.mk
		OUTPUT - $'+ : all : global FOO BAR :'
		ERROR -

	EXEC	--noexec --include=gbl --global=global.mk
		INPUT gbl/global.mk $'all : GLOBAL'
		OUTPUT - $'+ : all : GLOBAL FOO BAR :'
		ERROR - $'make: warning: Makefile.mo: out of date with gbl/global.mk
make: warning: Makefile.mo: recompiling'

	EXEC	--exec --include=gbl --global=global.mk
		OUTPUT -
		ERROR - $'+ : all : GLOBAL FOO BAR :'

	EXEC	--regress=sync --noexec
		OUTPUT - $'+ : all : FOO BAR :'
		ERROR - $'make: warning: Makefile.mo: global file global.mk was specified last time
make: warning: Makefile.mo: recompiling'

	EXEC	--noexec --include=gbl --global=global.mk
		INPUT global.mk $'all : global'
		OUTPUT - $'+ : all : global FOO BAR :'
		ERROR - $'make: warning: global.mk: binding changed to global.mk from gbl/global.mk
make: warning: Makefile.mo: recompiling'

	EXEC	--noexec
		OUTPUT - $'+ : all : FOO BAR :'

	EXEC	--exec --include=gbl --file=global.mk --compile
		OUTPUT -
		ERROR -

	EXEC	--noexec --global=global.mo
		OUTPUT - $'+ : all : global FOO BAR :'
		ERROR - $'make: warning: global.mk: binding changed to global.mk from gbl/global.mk
make: warning: Makefile.mo: recompiling'

	EXEC	--exec --global=global.mo
		OUTPUT -
		ERROR - $'+ : all : global FOO BAR :'

	EXEC	--noexec --global=global.mo
		INPUT global.mk $'all : GLOBAL'
		OUTPUT - $'+ : all : global FOO BAR :'
		ERROR -
