##########################################################
#  MG: A UNIX Tool to generate new MAKEFILES from 
#      shell commands
#        A. Kumar, ATT-BL, CB-2454, 3C-291
#  THIS TOOL IS NOT SUPPORTED at this time
#           Sept. 1, 1988   
#  However, please send any suggestions/bugs/enhancements
#  to me (cblph!ajy) for future generics.
##########################################################
###############################################################
#          AT&T BELL LABS - PROPRIETARY                  #
#        Use pursuant to company Instructions                 #
#      This source code is a proprietary information of       #
#  AT&T and it is not for use or disclosure    #
#  outside company except written agreement.                  #
###############################################################
BEGIN { 
#YOU COULD ADD to these lists for command recognition
#do not give full path names, only the name of the command 

	CC_cmds="cc m32cc m15cc m32ld ld ceccc cecc m4"
	AR_cmds="ar m32ar"
	CP_cmds="cp mv ln"
	CPMV_cmds="cpmv move"
	CHMOD_cmds="chmod"
	STRIP_cmds="strip m32strip"
	MKLN_cmds="mkln 3b2mkln"
	PAT_cmds="ppmkpat nepat pat"

	if (dl == "")
	{
		dl="_"
		debug_level=0
	}
	else
	{
		debug_level=dl
	}
	#changing this debug level will print more stuff.
	#this is for future debugging...
	if (oOPT == 0)
	{
		format_slash=0
		MAXPERLINE=50
	}
	else
	{
		format_slash=1
		MAXPERLINE=oOPT
	}
	#change this variable to =1 for backslash output...
	DSTYLE="ajy"
	Style_new = 0
	if (sOPT == 0) Style=DSTYLE
	else if (sOPT == 1)
	{
		print "new style"
		Style_new = 1
		Style = sOPT
	}
	else
	{
		print "using default version! Bad Style=" sOPT
		Style=DSTYLE
	}
	########################
	if (mOPT <= 0) FUZZYMATCH=2
	else
	{
		printf("FUZZYMATCH=%d", mOPT)
		FUZZYMATCH=mOPT	
	}
	#FUZZYMATCH=4
	#how far in the line could an executable exist on $0
	# for example 
	# + x cc  will not be recoginized with FUZZYMATCH=2
	# but
	# x cc will be recoginixzed...
	debug(1, "%s \n", "pass 1: begin", 0);
	if (varfile == "") varfile="_"
	if (invarfile == "" ) invarfile="_"
	if (premakefile == "" ) premakefile="_"
	#value TO define name/variable function
	if (fv2n == "" ) fv2n="_"
	NULL=""
	#used by var_i,val_i arrays.
	iIN=0
	iCMD=0; 
	CMD = 0
	var[CMD] = "COMMANDS" 
	Vmake="M"
	iMAKE = 0
	#edge list for include files (i,j)
	# where i and j are include directories.
	iEDGES = 0
	# used by lib directory lists
	iaEDGES = 0
	resth["1"]=""
	resta["1"]=""
	resto["1"]=""
	restc["1"]=""
	iINCDIR=0
	iLIBDIR=0
	iCMD=0  #noof commands in the install script.
	kCMD=0  #noof comamnds in the install script. insorder[1..kCMD]
	CONINVAR="INSTDIR"
	## if you have an initial list of vars, put in invar array.
	#shell variables are input here, 
	XCMDNAME="CMD"
	VXCMDNAME="VCMD"
	resto["1"]=""
	restc["1"]=""
	
	iVAR++
	#assert iVAR=1
	PAT="PATTERNS" #hardcoded at .ALLlist #MAJOR KLUDGE
	var[1]=PAT
	val[1]=""
	shenv();
	varfill();	
	invarfill();	
	
	#lint files in LINTFILES
	iVAR++
	iLINT=iVAR
	var[iLINT]="LINTFILES"
	val[iLINT]=""

	###test
}
END {
	debug(1, "%s \n", "pass 2: begin", 0);
	ts_makefile();
	# build the variable list, typically install dirs INSTDIR1, ...and others
	bld_invar();
	#builds Sdep lines for SOURCE.a..etc. and also variables.
	bld_SOURCE();
	# build COMMAND and LIBxxx and LIBxxxFILES variables
	# var is only concerned with variables and not with any install dirs
	bld_var();
	# build the dependency lines, such as .ALL:, a:: rules etc 
	bld_dep();
	# print the above list
	# print_invar MUST follow all bld's because all install directories not
	# used are not printed by default
	print_invar();
	# build and print the SOURCE, .a lists
	print_SOURCE();
	# print the above
	print_var();
	print_dep();
	# print the above dep lines
	print_install2(); #print the install directory, after gathering common installs
		 # also, as a sideeffect generate .CMD options
print_CMD(); #print the .CMD command set
}

#chmod xxx tar  tar tar ...
#  j    j+1   j+2
#strip tar  tar tar ...
#  j   J+1 
#mkln tar tar tar ...
#  j  j+1 
{ #BEGIN OF MAIN PROGRAM
lmatch=0
debug(9, "L:%s\n", $0)

for (idi=1; ((idi<=FUZZYMATCH) && (idi <= NF)) ; idi++){
	#global variable cmdtype is set by get_cmds.
	debug(1, "idi=%d $idi=%s\n", idi, $idi);
	cmd=get_cmds($idi)
	if (cmd == "")
		continue
	#reaching here means command is recognized, get index
	debug(1, "cmd=%s type=%s idi=%d\n", cmd, cmdtype, idi)
	iindex=idi
	if (cmdtype == "CC_cmds") {
		call_CC_cmds($0, NF, cmd, cmdtype, iindex)
		lmatch=1
		break
	}
	else if (cmdtype == "AR_cmds") {
		call_AR_cmds($0, NF, cmd, cmdtype, iindex)
		lmatch=1
		break
	}
	else if ((cmdtype == "CP_cmds") || (cmdtype=="CPMV_cmds")) {
		call_CP_cmds($0, NF, cmd, cmdtype, iindex)
		lmatch=1
		break
	}
	else if ((cmdtype == "CHMOD_cmds") || (cmdtype == "STRIP_cmds") || (cmdtype == "MKLN_cmds") ) {
		call_CH_cmds($0, NF, cmd, cmdtype, iindex)
		lmatch=1
		break
	}
	else if (cmdtype == "PAT_cmds" ) {
		call_PAT_cmds($0, NF, cmd, cmdtype, iindex)
		lmatch=1
		break
	}
	else {
		print "[internal error]" 1023394830498
		debug(1, "internal error\n");
	}
}#end of for loop for checking args..
	if (lmatch == 0) {
		#print "ignored:\n" $0
		ignore($0)
	}
} #END OF MAIN PROGRAM

#returns the actual command matched
#cmd from *_cmds arrays defined earlier
#GLOBALS:also cmdtype is set 
function get_cmds(str,	i,j,k,jj,ii,kk,arr,tmp)
{
debug(9, "get_cmds %s\n", str)
basename(str); bstr=base
dirname(str); dstr=dir
if (isin(CC_cmds, base) > 0 ) {
	cmdtype="CC_cmds"
	cmd=base
	return(cmd)
}
if (isin(CP_cmds,base) > 0 ) {
	cmdtype="CP_cmds"
	cmd=base
	return(cmd)
}
if (isin(CPMV_cmds, base) > 0 ) {
	cmdtype="CPMV_cmds"
	cmd=base
	return(cmd)
}
if (isin(AR_cmds, base) > 0 ) {
	cmdtype="AR_cmds"
	cmd=base
	return(cmd)
}
if (isin(CH_cmds, base) > 0 ) {
	cmdtype="CH_cmds"
	cmd=base
	return(cmd)
}
if (isin(CHMOD_cmds, base) > 0 ) {
	cmdtype="CHMOD_cmds"
	cmd=base
	return(cmd)
}
if (isin(STRIP_cmds, base) > 0 ) {
	cmdtype="STRIP_cmds"
	cmd=base
	return(cmd)
}
if (isin(MKLN_cmds, base) > 0 ) {
	cmdtype="MKLN_cmds"
	cmd=base
	return(cmd)
}
if (isin(PAT_cmds, base) > 0 ) {
	cmdtype="PAT_cmds"
	cmd=base
	return(cmd)
}
cmd=""
cmdtype=""
return(cmd);
}#end of function 1

#/chmod / || /strip / m32strip or mkln
function call_CH_cmds(lin, nlin, cmd, cmdtype, iindex,  i,j,k,arr,tmp,opt)
{
	if (cmdtype == "CHMOD_cmds") {
		opt=$(iindex+1)
		starti=iindex+2
	}
	else if (cmdtype== "STRIP_cmds") {
		opt=""
		for (k=iindex+1; k<=NF; k++)
		 	if ($(k) ~ /^\-.*/ )
				opt=opt " " $(k)
			else {
				starti=k
				break
			}
	}
	else if (cmdtype == "MKLN_cmds") {
		opt=$(iindex+1)
		starti=iindex+1
	}
	debug(2, "cmd=%s opt=%s starti=%d\n", cmd, opt, starti)
	for (k=starti; k <= NF; k++) {
		tar = $(k)
			#find if the target already exists?
			#loop over targets : 
		if  (tar in target)	{	
				#this is a change on that target...
				#must have an installation ?
			basename(tar); src=base
				#src=  src is the ???
			for(i=1; icmd[src,i]!=""; i++);
				#KLUDGE : $(<) or $(>) find it from target.
			if (src == tar)
				direct = "$(>)"
			else 
				direct = "$(<)"	
			icmd[src, i]=sprintf("\t %s %s %s", vcommand(cmd), opt, direct);
			icmd[src, i+1] = "" #mark the end of list!
			debug(1, "mod on target=%s icmd[%s,%d]\n", tar, src,i);
		} 
		else if ((ic=destins(tar)) > 0 ) {
			for(i=1; kcmd[ic, i]!=""; i++);
				kcmd[ic, i] =sprintf("\t %s %s $(<)", vcommand(cmd), opt);
			debug(1, "destins\n");
		
		} else {
#now the object is neither in target list, nor in install list, so must
#be something not dependent on anything, need a ins[] line here?
		if (cmdtype == "MKLN_cmds" ) {
			val[iLINT]=val[iLINT] " " nbasename(any2ln(tar))
			var_cnt[iLINT]++
			spath(tar)
		 } else {
			print "[NOTFOUND] confused at line:" $0 "  target=? " tar
			print "no dependency why have it in this makefile"
			ignore($0)
		 } #mkln chk.
	} #the last else
       } # for all targets
} #e_o_f:call_CH_cmds


function call_CC_cmds(lin, nlin, cmd, cmdtype, iindex,  i,j,k,arr,tmp,opt)
{
	j=smatch($0, "-o")	
	if (j > 0) {
		#this is a cc line with -o 
		debug(5, "cc -o line%s \n", $0)
		tar=$(j+1)
		if (tar ~ /^\.\/.*/ )
			sub(/^\.\//,"",tar)
		tar_in_ins(tar)
		debug(5, "checked tarinins\n");
		if (tar in target) {
			print "ERROR: multiple target definition: " tar
			ignore($0)
			next
		}
		else {
			target[tar] = tar 
			tlist = tlist " " tar
		}
		#convert all .o to .c 
		#also any .c's are put in the _cli list.
		#side effects are to build .SOURCE.c,.o in future versions
		dot_o2c($0)
		cdep[tar] = _cli
			#process any libraries
			#side effects are to build .SOURCE.a (ok)
		dot_as($0)
		adep[tar] = _ali
			#process any header file dependency
			#side effects are to build .SOURCE (ok)
		dot_Is($0)
		dflags($0)
		if (_Dli != "")  {
			print "(FLAG) rule"
			#new dependency is here
			Ddep[tar] = _Dli
		}
	} #end of -o processing
	else { # this is a normal compilation, NO a.outs please
		# usually -c option 
		dot_o2c($0)
		dot_Is($0)
		dflags($0)
		if (_Dli != "")  {
#gsf#			print ".o : rule"
			j1=smatch($0, "-c")	
			if (j1 > 0) {
				#this is a cc line with -c 
				#may want to do a check if more .c on line
				for(ii=1; ii<=NF; ii++) {
					if (isc($ii) > 0 ) {
						basename($ii); tar=base
						gsub(/\.c$/,"",tar)
						tar=tar ".o"	
#gsf#						print ".o " tar _Dli
						target[tar] = tar
						cdep[tar] = ""
						adep[tar] = ""
						Ddep[tar] = _Dli		
					}
				}
				#tar=
			} else {
#gsf#				print "what is =" $0
			}			
			#new dependency is here
		}
	} #end of -c processing
debug(5, "exit:call_CC_cmds()\n");
}#e_o_f:CC_cmds
 
#if ($0 ~ /mv / || /cp / || /ln / || /move / || /cpmv / ) 
function call_CP_cmds(lin, nlin, cmd, cmdtype, iindex,  i,j,k,arr,tmp,opt, kloop, command, src, dest, starti, ddest, dsrc, bdest, bsrc, opt, u)
{
	command=cmd
	j=iindex
	if (cmdtype == "CP_cmds") {
				src=$(j+1)
				dest=$(j+2)
				starti=j+2
	}
	if (cmdtype == "CPMV_cmds") {
					src=$(j+1)
					dest=$(j+5)
					starti=j+5
	}
	opt=""
            for(kloop=starti;kloop <= NF; kloop++) {
		dest=$(kloop)
		if (dest ~ /^\-.*/ ) {
			opt=opt " " dest
			continue
		}
		debug(1,"%s -> %s\n", src, dest);	
		basename(src);bsrc=base
		dirname(src); dsrc=dir
		#print bsrc "\ " dsrc
		if (dsrc== "./") dsrc=""
		if (tOPT == 1) { #test the destination direcotry
			debug(7,"1: %s %s\n", dsrc, ddest);
			if (isfile(dest) > 0)  {
				basename(dest); bdest= base
				dirname(dest); ddest=dir
			debug(7,"2: %s %s\n", dsrc, ddest);
			} else {
				bdest=bsrc
				ddest=dest
				print "WARNING: destination is a dir=" dest
			}
		} else { #donot test any directories, assume fullpath name
				basename(dest); bdest= base
				dirname(dest); ddest=dir
			debug(7,"3: %s %s\n", dsrc, ddest);
		}	
			if (ddest== "./") ddest=""
			if (ddest ~ /^\.\/.*/) {
				sub(/^\.\//,"", ddest)
			}
			if ((bdest== ".") || (bdest == "") || (bdest == " ") ) bdest=bsrc
			debug(7,"8: %s %s\n", dsrc, ddest);
		kCMD++;
#check to see if bsrc is built here, and any precommands exist.
#such as a m32strip, chmod etc.
		if (icmd[bsrc,1] != "" ) {
			print "bsrc was modified"
			for(u=1; icmd[bsrc, u]!=""; u++) {
				for(k=1; kcmd[kCMD,k]!=""; k++);
				kcmd[kCMD,k] = icmd[bsrc,u]; 
				#print "putting:" icmd[bsrc,u] " TO " k
			}
			for(u=1; icmd[bsrc,u]!=""; u++)
				icmd[bsrc,u]="" #empty it out
		}
		#call subinvar simply to increment the count, to make sure these
		# variables are used...
			debug(7,"sub: %s %s\n", dsrc, ddest);
			subinvar(dsrc); subinvar(ddest);
			ins[dsrc, bsrc, ddest, bdest] = kCMD
			insorder[kCMD]=dsrc SUBSEP bsrc SUBSEP ddest SUBSEP bdest
			instype[kCMD]="install"
			#print "insorder" kCMD " {}-"insorder[kCMD]

			for(k=1; kcmd[kCMD,k]!=""; k++);
			if (cmdtype == "CPMV_cmds")
			kcmd[kCMD,k] = sprintf("\t %s $(>) %s %s %s $(<)",
				vcommand(command), $(j+2), $(j+3), $(j+4));
			else if (cmdtype == "CP_cmds" )
			kcmd[kCMD,k] = sprintf("\t %s  $(>) $(<)", 
				vcommand(command) )
			else  {
				plogic("arrr!WHATCMD");
				debug(1, "failed terribly %s\n", cmdtype);
			}
			
		instdir[ddest]=""
           } #for loop for each destination
} #e_o_f:CP_cmds 
#/ar / 
function call_AR_cmds(lin, nlin, cmd, cmdtype, iindex,  i,j,k,arr,tmp,opt, kloop, command, src, dest, starti, ddest, dsrc, bdest, bsrc, opt, u,jj)
{
		j=iindex
		tar=$(j+2)
		if (tar in target) {
			#print "Warning:multiple archive (ar) target: " tar
			dot_o2c($0)
			chklist = _cli
			x=split(chklist,  arr, " ")
			if (x > 0 ) {
				for (jj=1; jj<=x; jj++)
					if (isin(cdep[tar], arr[jj]) <  0) {
						newstr=cdep[tar] " " arr[jj]
						cdep[tar] = newstr
					}
			}	
		} else {
			target[tar]=tar
		#AR command processing, build the list	
		dot_o2c($0) 
		cdep[tar] = _cli
		}
}#e_o_f:AR_cmds


function call_PAT_cmds(lin, nlin, cmd, cmdtype, iindex,  i,j,k,arr,tmp,opt)
{
			cin="";
			cout="";
			find_in_out($0)
			debug(1, "in=%s out=%s\n",  cin , cout)
			rule_pat($0, cmd, iindex)
}


function ignore(thisline) 
{
	print "note " thisline
}
function print_CMD(s, ar, cmd, ic)
{
	for (s in cmdl) {
		printf("\n\n")  
		cmd=sprintf(".CMD.%d",cmdl[s]) 
		printf("%s : .USE\n", cmd) 
		split(s, ar, "/")
		printf("\tcpmv $(>) %s %s %s $(<)", ar["1"], ar["2"], ar["3"])
		printf("\n") 
	}

	for (s in ccmdl) {
		printf("\n\n")  
		printf(".CMD.%d : .USE\n", s) 
		ic = ccmdl[s] # the iindex for some icmd arrary
			for(k=1; icmd[ic,k]!=""; k++)
					printf("%s\n", icmd[ic,k]) ;


	}
	for (s in kcmdl) {
		ic = kcmdl[s] # the index for some icmd arrary
		#print "s="s "ic="ic
	        if (s in normal_kkcmd) {
		printf("\n\n")  
		printf(".%s.%d : .USE\n",XCMDNAME, s) 
			for(k=1; kcmd[ic,k]!=""; k++)
					printf("%s\n", kcmd[ic,k]) ;
		}else;
			#print "not used"
	}
	for (s in kcmdl) {
			ic = kcmdl[s] # the index for some icmd arrary
		#print "s="s "ic="ic
		if (s in virtual_kkcmd) {
			printf("\n\n")  
			printf(".%s.%d : .USE\n",VXCMDNAME, s) 
			for(k=1; kcmd[ic,k]!=""; k++)
					printf("%s\n", vircnv(kcmd[ic,k]) ) ;
		}else ;
			#print "not used"
	}
}
#prints the SOURCE.h, .a lists
function bld_SOURCE() 
{
top_sort("Check: INCLUDE directory list in SOURCE.h", snode, enode, iEDGES);
hdrlist = _SOURCEh
notonlist(hdrlist, incdir, iINCDIR, resth)
if ((hdrlist == "" ) && (resth["1"] == "" ) )
	#dont print SOURCE.h
	#print "empty SOURCE.h"
	#noop();
	Sdeph=""
else {
#	printf(".SOURCE.h:%s %s\n\n", v2(hdrlist), v2(resth["1"])) 
	Sdeph=sprintf(".SOURCE.h:%s %s\n\n", v2(hdrlist), v2(resth["1"])) 
}
top_sort("Check: LIB directory in SOURCE.a", sanode, eanode, iaEDGES);
hdrlist = _SOURCEh
debug(1,"hdr=%s", hdrlist);
notonlist(hdrlist, libdir, iLIBDIR, resta)
if ((hdrlist == "" ) && (resta["1"] == "" ) )
	#dont print SOURCE.h
	#print "empty SOURCE.a"
	#noop();	
	Sdepa=""
else
	#printf(".SOURCE.a:%s %s\n\n", v2(hdrlist), v2(resta["1"])) 
	Sdepa=sprintf(".SOURCE.a:%s %s\n\n", v2(hdrlist), v2(resta["1"])) 
top_sort("Check: OBJ directory in SOURCE.o", sonode, eonode, ioEDGES);
hdrlist = _SOURCEh
notonlist(hdrlist, objdir, iOBJDIR, resto)
if ((hdrlist == "" ) && (resto["1"] == "" ) )
	#dont print SOURCE.h
	#print "empty SOURCE.a"
	#noop();	
	Sdepo=""
else
	Sdepo=sprintf(".SOURCE.o:%s %s\n\n", v2(hdrlist), v2(resto["1"])) 
	#printf(".SOURCE.o:%s %s\n\n", v2(hdrlist), v2(resta["1"])) 
top_sort("Check: .c directory list in SOURCE.c", sonode, eonode, ioEDGES);
hdrlist = _SOURCEh
notonlist(hdrlist, dotcdir, iDOTCDIR, restc)
if ((hdrlist == "" ) && (restc["1"] == "" ) )
	#dont print SOURCE.h
	#print "empty SOURCE.a"
	#noop();	
	Sdepc=""
else
	Sdepc=sprintf(".SOURCE.c:%s %s\n\n", v2(hdrlist), v2(restc["1"])) 
	#printf(".SOURCE.c:%s %s\n\n", v2(hdrlist), v2(restc["1"])) 
}
function print_SOURCE()
{
if (Sdeph != "")
	printf("%s", Sdeph) 
if (Sdepa != "")
	printf("%s", Sdepa) 
if (Sdepc != "")
	printf("%s", Sdepc) 
if (Sdepo != "")
	printf("%s", Sdepo) 
}

#print install directory variable names
function print_invar(vardir)
{
for (vardir in invar ) {
	if (vardir in noprintvar) { #if on this list of donotprint
		debug(5,"noprint: %s\n", vardir)
		continue
	}
	if (invar_cnt[vardir] <= 0 ){
		debug(5, "nouse: %s\n", vardir)
		continue
	}
	if (invar[vardir] == "" )
		printf("%s=%s\n", vardir, "./") ;	
	else
		printf("%s=%s\n", vardir, invar[vardir]) ;	
}
printf("\n") ;
}

function bld_invar(i,j,dir, found)
{
#instdir[dir(path] = VAR 
#invar[VAR] = val
#generate a define, and assign it 	
#longest common substring, number of times used etc.

for (dir in instdir ) {
	xdir=dir
	found=0
	for (vardir in invar)
		if (dir == invar[vardir] ){
			#print "1"
			found=1	
		}
		else if (dir"/" == invar[vardir] ) {
			#print "2"
			found=1
		}
		else if (compress(xdir) == invar[vardir] ) {
			#print "3"
			found=1
		}
		else if (dir !=  subinvar(dir) ) {
			#print "v sub match" dir "->" subinvar(dir)
			found=1
		}
	#either found or not found
	if (found == 0 ) { #not found, build a new name
		iINVAR++
		newvar = sprintf("%s%s",CONINVAR, iINVAR);
		invar[newvar] = compress(dir)
		invar_cnt[newvar]++	
	#	instdir[dir] = compress(newvar)
		debug(6, " new var %s%s=%s\n", CONINVAR, iINVAR, dir);
	} else 
		invar_cnt[vardir]++	
		#instdir[dir] = vardir 
	} # for	
}

#add a new invar[xx] = dir type variable ...
function add_invar(dir, i,j,dir, found)
{
	found=0
	for (vardir in invar)
		if (dir == invar[vardir] )
			found=1	
	#either found or not found
	if (found == 0 ) { #not found, build a new name
		iINVAR++
		newvar = sprintf("%s%s",CONINVAR, iINVAR);
		invar[newvar] = compress(dir)
	#	instdir[dir] = newvar
		debug(6, "new var %s%s=%s\n", CONINVAR, iINVAR, dir);
	} else 
		#instdir[dir] = vardir 
}

#version 2.0 nmake install uses an explicit install: dependency rule
# print_install2(	i,j, dir, found[A#version 2.0 nmake install uses an explicit install: dependency r
function print_install2(	i,j, dir, found, vardir, ic, nm)
{
debug(5, "install2: enter\n");
#first build install: dependency line
#next print each specific dependency line as a .CMD command
#may be in future combine all installdirectories as one variable.
#in ins, all extra commands.
install_list = install_list " " v_install_list
	for (iorder=1; iorder<=kCMD ; iorder++) {
		jj=insorder[iorder]  #jj is the unique install option
		itype = instype[iorder]
		jjn = split(jj, arr, SUBSEP);
		debug(9, "jj=%s\n", jj);
		dir = arr["3"]			
			#AJYBLANK printf("\n");
			nm=subinvar(dir);
			if (dir != nm ) {
				#print "sub: " dir " -> " nm
				found=1
				myvardir=compress(nm) #removes "//" etc.	
			}
			else 
			{
			found=0
			for (vardir in invar)
				if  (invar[vardir] == dir) 
#KLUDGE		             (compress(invar[vardir]) == compress(dir))
					{
                                   print "FOUND" vardir ":"dir":" invar[vardir]
					if ((dir == "") || (dir== "./")  )
						myvardir=""
					else
						myvardir="$("vardir ")"
					found = 1
					invar_cnt[vardir]++
						break;	
				}
			if (found ==1)
				print ".." myvardir
			}
			if ((myvardir == "") || (myvardir == "./") )
				pmyvardir=""
			else
				pmyvardir= compress(myvardir) "/"
			debug(9, "install2:pmy=%s b=%s\n",  pmyvardir, arr["4"])
			if ((found == 1) && (itype == "install" ) ) {
				if ((Style_new==1) &&(virtual_ins[iorder]==1) )
					install_list=install_list " " virtual_n(pmyvardir,arr["4"], arr["2"])
				else
			   	 install_list=install_list " " pmyvardir arr["4"]  
			   	 #install_list=install_list " $("tayvardir")/"arr["4"]  
                            #printf("\nINSTALLDIR=$(%s)\n", myvardir) 
			} else {
				;
				#print "WARNING: empty install: rule"
			}
				#print "LOGIC ERROR:call author immediately"
				#printf("\nINSTALLDIR=%s\n", dir) 
		# now we have the install line
		# now find the actual load line
		if  (itype=="pattern") { #KLUDGE =i="" {
			if ((arr["1"] == "") || (arr["1"] == "./") || (arr["1"] == "." ) ) #no sub
	 		printf("%s : %s", arr["4"], arr["2"]) ;
			else
	 		printf("%s : %s", arr["4"], subinvar(arr["1"] "/" arr["2"]) ) ;
		} 
		if ((itype == "install") && (Style_new == 1) && (virtual_ins[iorder]==1) ) {
			if ((arr["1"] == "") || (arr["1"] == "./") ) #no sub
	 		printf("%s : %s%s", virtual_g(pmyvardir, arr["4"], arr["2"]), pmyvardir, arr["4"] ) ;
			else
			printf("%s : %s", virtual_g(pmyvardir, arr["4"], arr["2"]), compress(subinvar(arr["1"]) "/" arr["2"]) ) ;			
		}

		if (((itype == "install") && (Style_new != 1) ) || ((Style_new==1) && (virtual_ins[iorder] != 1) && (itype== "install"  )) ) {
		if ((arr["1"] == "") || (arr["1"] == "./") ) #no sub
	 		printf("%s%s : %s",pmyvardir, arr["4"], arr["2"] ) ;
			else
			printf("%s%s : %s", pmyvardir, arr["4"], compress(subinvar(arr["1"]) "/" arr["2"]) ) ;			
		}
		kkcmd = ins[jj]
		ic = getkcmdi(kkcmd);
		if  ((Style_new==1) && (virtual_ins[iorder] == 1) ) {
			printf(" .%s.%d",VXCMDNAME, ic) ;
			printf(" .VIRTUAL\n") ;
			virtual_kkcmd[ic]=1
		} else {
			normal_kkcmd[ic]=1
			printf(" .%s.%d",XCMDNAME, ic) ;
			printf("\n") ;
		}
	}
#now the install line:
if (format_slash == 0) { #this is
	printf("\ninstall: %s\n", install_list) ;
} else {
	printf("\ninstall: \\\n")  ;
		nl(install_list) ;
} 
}
function print_install(	i,j, dir, found, vardir, ic)
{
	print "&*^%$#FATAL ERROR : nosuchfunction exists"
}

function print_dep(tar, i)
{
for (i=0; i<=iDEP; i++)
	if ((ldep[i] != "") && (COMD[ldep[i]] != 1) && (iso(ldep[i]) <= 0) )
		ALLlist = ALLlist " " ldep[i]
#gsf#	if ((val[CMD] != "") && (COMD[ldep[i]] != 1))
#gsf#		ALLlist = ALLlist " $(COMMANDS) "
	#MAJOR KLUDGE::
	if ((var[1] == PAT) && (val[1] != "") ) {  
		ALLlist = ALLlist " $(PATTERNS) "
	}
	for (i=0; i<=iDEP; i++) 
		if (ldep[i] != "" && rdep[i] !~ "(.*)") {
			#distinguish between :: and : rules
				if (iso(ldep[i]) > 0 ) 
					operator=":"
				else
					operator="::"
			if (format_slash==0) { #hello
					printf("%s %s %s %s\n\n", ldep[i], 
						f_ldep_more(i), 
						operator, rdep[i]) 
			} else {
				#use "backslash for formatting.
				printf("%s %s %s \\\n",  ldep[i], 
					f_ldep_more(i), operator ) 
				nl(rdep[i]) 
			}
		}

	
	if (val[iLINT]!="")  {
		printf("slint: %s\n\n", "$(" var[iLINT] ")" ) ;
		print "WARNING: following rule be defined:"
		print "				*.ln : *.c  "
		print "					mkln $(>) "
	}
	for (i=0; i<=viDEP; i++) 
		if (vldep[i] != "" ) {
			printf("%s : %s .VIRTUAL\n", vldep[i], vrdep[i]) ;
			basename(vrdep[i])
			for (j=1; icmd[base, j] != ""; j++)
				printf("%s\n", vircnv(icmd[base,j]) );
		}
	

}
#builds dep lines
#uses global lists
function bld_dep(	i, tar, tmp, blib, jj, jjn, arr, tmp,iorder)
{
for (tar in target)
	if (isa(tar) > 0) {
		blib  = tar
		sub(/\.a/, "", blib)
		sub("^lib","", blib) 
		i = ivar[tar]
		iDEP++
		ldep[iDEP] = "$(" var[i] ")"
		rdep[iDEP] = val[i+1] #KLUDGE : i+1 is libfiles.
	} else {
		# a command from .c,.o
		#i = ivar[tar]
		iDEP++
		ldep[iDEP] = subinvar(tar)
		rdep[iDEP] = cdep[tar] " " adep[tar] " " Ddep[tar]
		COMD[subinvar(tar)]=1
		basename(tar)
		#XAJY print tar "**" base "***" icmd[tar,1]
		if (icmd[base,1] != "") {
			viDEP++
			print "NEED A VIRTUAL=%s", base
			vldep[viDEP] = "VV_"base
			vrdep[viDEP] = ldep[iDEP]
			v_install_list = v_install_list " " vldep[viDEP]
		}
	} 

#XAJY for(i=0;i<=iDEP; i++) print i"="ldep[i]
#go thru all targets...if new style
if (Style_new==1) {
	for (iorder=1; iorder<=kCMD ; iorder++) {
		jj=insorder[iorder]  #jj is the unique install option
		itype = instype[iorder]
		jjn = split(jj, arr, SUBSEP);
			##ins[dsrc, bsrc, ddest, bdest] = kCMD
		dsrc=arr["1"]
		bsrc=arr["2"]
		ddest=arr["3"]
		bdest=arr["4"]
				#print "NEW STYLE:" iDEP
				for(jjj=0;jjj<=iDEP;jjj++) {
				#print "LDEP=" ldep[jjj]
				basename(ldep[jjj])
				dirname(ldep[jjj])
				if ((dir=="") || (dir=="./")) dir=""
				if (!(dir ~ /^\$.*/) 	&& (dir != "") )
					dir=compress(subinvar(dir))
				if ((dsrc=="") || (dsrc=="./")) dirins=dsrc
				if (!(dirins ~ /^\$.*/) && (dirins != "") )
					dirins=compress(subinvar(dir))
				debug(1, "CK:dsrc=%s bsrc=%s dir=%s base=%s\n",
					dirins, bsrc, dir, base)
				if ((dirins == dir) && (bsrc == base) ) {
					print "virtual:" dsrc "-" bsrc	
					virtual_ins[iorder]=1
				destdir=ddest
				if ((destdir=="") || (destdir=="./")) destdir=""
				if (!(destdir ~ /^\$.*/) 	&& (destdir != "") )
					destdir=compress(subinvar(destdir))
					
					if (destdir != "")
						ldep_more[jjj]=ldep_more[jjj] ", " compress(destdir "/" bdest) 
					else  
						ldep_more[jjj]=ldep_more[jjj] ", " compress(bdest) 
						
				}#if match!
				}#forloop	
		}#fo all install tars
	}
}

function print_var(i)
{
for (i=2; i<=iVAR; i++) {
	if (var[i] in noprintvar) { #if on this list of donotprint
		debug(5, "noprint: %s\n", var[i])
		continue
	}
	if (var_cnt[i] <= 0 ) { #if not used
		debug(5, "nouse: %s\n", var[i])
		continue
	}	
	if (var[i] != "" ) #if no variable oops
			if (val[i] != "" ) # print nonnull fields
				printf("setv %s %s\n", var[i], val[i]) 
}
}

#builds var list
#uses global lists...
function bld_var(	i, tar, tmp, blib)
{
for (tar in target  ) {
	tar_in_ins(tar);
	if (isa(tar) > 0) {
	    #print tar "is lib"
	    #is a library!
	     if (tar in ivar) 	
		printf "multiple target definition: " tar
	      else {
		blib  = tar
		basename(blib); blib=base;
		sub(/\.a/, "", blib)
		sub("^lib","", blib) 
		iVAR++
		ivar[tar]  = iVAR
		var[iVAR]  = "LIB" blib
		val[iVAR]  = "lib" blib ".a"
		var_cnt[iVAR]++
		iVAR++
		#ivar[tar]  = iVAR
		var[iVAR] = "LIB" blib "FILES"
		val[iVAR] = cdep[tar]	
		var_cnt[iVAR]++
		noprintvar[var[iVAR]]=""
	     }		
	}else  if (iso(tar) > 0 ) {
		; #donot put in commands line, nouse
	} else {
		#if command from .c,.osas usual
		# var[CMD] = "CMDS" as defined
		#substitute with subinvar, if tar has a dir?
		dirname(tar); 
		basename(tar); 
		nm=subinvar(dir)
		if (dir == nm )
			val[CMD] = val[CMD] " " tar
		else {
			print "dir match by (sub)invar"
			val[CMD] = val[CMD] " " compress(nm "/"base)
		}
		var_cnt[CMD]++
		COMD[tar] = 1
		#iVAR++
		#ivar[tar]  = iVAR
		#var[iVAR] = "CMD" tar "FILES"
		#val[iVAR] = cdep[tar]	
	}
  } #for
		
}
function chkcmd(s, n, t, 	i, bas)
{
	for(i=1; i<=n; i++) {
		basename($i); bas=base;
		#print "bas=" bas " test=" t
		if(bas == t) 
	 		return i	
	}
	return -1

}

function getcmdi(ar1 ,	ti) {
	if (ar1  in cmdl ) {
		return (cmdl[ar1 ])
	}
	else {
		cmdl[ar1 ]=iCMD++;
		return (cmdl[ar1 ])
	}
}

function getccmdi(ic, i, iass,k,  xmatch) {
for (i in ccmdl )  {
	iass = ccmdl[i]
#	print "checking " iass
	xmatch=1
	for (k=1; icmd[ic,k]!=""; k++)
		if (icmd[ic,k] != icmd[iass,k])
			xmatch=0
	if (xmatch == 1 )
		if (icmd[iass,k] != "" )
			xmatch =0
	#check if matched, or else go over next one
	if (xmatch == 1) {	
		#print "match" ic i
		return(i)
	}			
}
#if we reach here, means we found no match.
iCCMD++
ccmdl[iCCMD] = ic
return(iCCMD)
}

function getkcmdi(ic, i, iass,k,  xmatch) {
for (i in kcmdl )  {
	iass = kcmdl[i]
#	print "checking " iass
	xmatch=1
	for (k=1; kcmd[ic,k]!=""; k++)
		if (kcmd[ic,k] != kcmd[iass,k])
			xmatch=0
	if (xmatch == 1 )
		if (kcmd[iass,k] != "" )
			xmatch =0
	#check if matched, or else go over next one
	if (xmatch == 1) {	
		#print "match" ic i
		return(i)
	}			
}
#if we reach here, means we found no match.
iKCMD++
kcmdl[iKCMD] = ic
return(iKCMD)
}

function smatch(s, t, i)
{
for(i=1; i<=NF; i++)
	if($i == t) 
	 	return i	
return -1
}

function print_Is(i)
{
for (i=0; i<=iEDGES; i++)
	print "("snode[i] "," enode[i]	")"
}

function dot_Is(s, bincl, k, prev)
{
debug(6, "enter:dot_Is()\n");
prev=""
_Ili=""
for(k=1; k<=NF; k++)
	if (isI($k) > 0) {
		bincl = $k 
		sub(/\-I/,"", bincl)
		if ((bincl == "") || (bincl == "-") || (bincl == ".") )
			continue;
		_Ili = _Ili " " bincl
		if (!(bincl in incdir_a) ) {
			incdir_a[bincl]="y"
			iINCDIR++
			incdir[iINCDIR]=bincl
		}
		if (prev == "") 
			prev=bincl
		else {
			iEDGES++
			snode[iEDGES] = prev
			enode[iEDGES] = bincl
			prev = bincl
		}
	}
debug(6, "exit:dot_Ts()\n");
}


function dot_as(s, blib,k,prev)
{
debug(6, "entered dot_as\n");
prev=""
_ali=""
for(k=1; k<=NF; k++) {
	if (isa($k) > 0) {
		basename($k)
		blib=base
		dirname($k)
		dlib=dir
		sub(/\.a/, "", blib)
		sub("^lib","", blib) 
		_ali=_ali " " "-l"blib
		if (!(dir in libdir_a) ) {
			libdir_a[dir] = "y"
			iLIBDIR++
			libdir[iLIBDIR]=dir
		}
		if (prev == "") 
			prev=dlib
		else {
			iaEDGES++
			sanode[iaEDGES] = prev
			eanode[iaEDGES] = dlib
			prev = bincl
		}
	}	
	if (isl($k) > 0) {
		basename($k)
		blib=base
		dirname($k)
		dlib=dir
		sub(/^\-l/,"", blib)
		_ali=_ali " " "-l"blib
		if (dlib == "./") dlib=""
		if (dlib != "" ) {
		 if (!(dir in libdir_a) ) {
			libdir_a[dir] = "y"
			iLIBDIR++
			libdir[iLIBDIR]=dir
		 }
		 if (prev == "") 
			prev=dlib
		 else {
			iaEDGES++
			sanode[iaEDGES] = prev
			eanode[iaEDGES] = dlib
			prev = bincl
		 }
		}#if dlib is not= ""
	}
   } #for all members in the line
debug(6, "exit: dot_as()\n");
}

#_cli is a global variable.
function dot_o2c(s, ss, arr, k, ssdir)
{
debug(6, "enter:dot_o2c()\n")
#print $0
_cli=""
prev_o=""
prev_c=""
for(k=1; k<=NF; k++) {
    tar_in_ins($k);
    if (iso($k) > 0) {  #if a .o file name
	ss = arr[split($k, arr, "/")]
	if (sub(/\.o/,"", ss) == 1 ) {
		_cli=_cli " " ss".c"
	} #if
        dirname($k); ssdir=dir;
	sub(/\/\//, "", ssdir);
	if ((ssdir == "") || (ssdir == ".") || (ssdir == "./") )
		noop();
	else {
		if (!(ssdir in objdir_a) ) {
			objdir_a[ssdir]="y"
			iOBJDIR++
			objdir[iOBJDIR]=ssdir
		}
		if (prev_o == "") 
			prev_o=ssdir
		else {
			ioEDGES++
			sonode[ioEDGES] = prev_o
			eonode[ioEDGES] = ssdir
			prev_o = ssdir
		}
	}
	
     } #if a .o 
     if (isc($k) > 0) { #if a .c file name
	basename($k);
	_cli = _cli " " base
        dirname($k); ssdir=dir;
	sub(/\/\//, "", ssdir);
	if ((ssdir == "") || (ssdir == ".") || (ssdir == "./") )
		noop();
	else {
		if (!(ssdir in dotcdir_a) ) {
			dotcdir_a[ssdir]="y"
			iDOTCDIR++
			dotcdir[iDOTCDIR]=ssdir
		}
		if (prev_c == "") 
			prev_c=ssdir
		else {
			icEDGES++
			scnode[icEDGES] = prev_c
			ecnode[icEDGES] = ssdir
			prev_c = ssdir
		}
	}
     }
} # for loop
debug(6, "exit:dot_o2c()\n");
}

#base is a global variable, need to learn how
# to pass/return string values.
function basename(path, 	arr)
{
base=arr[split(path, arr, "/")]
}
function nbasename(path, 	arr)
{
return(arr[split(path, arr, "/")])
}

#dir is a global;
function dirname(path, 	b,arr)
{
dir=""
nfi=split(path, arr, "/");
b=arr[split(path, arr, "/")]
for (i=1; i<nfi; i++) {
	#print arr[i] "/"
	dir=dir arr[i] "/"
}
if (dir == "./") dir=""
debug(9, "path=%s dir=%s\n", path, dir)
return
debug(9, "\npath=%s arr:1=%s 2=%s", path, arr["1"], arr["2"]);
debug(9, "b=%d path=%s\n", b, path)
if (b == "" )
	printf("internal error: empty string to dirname()\n"); 
else
	sub(b,"",path)
if ((path == "./") || (path == ".//") ) # KLUDGE need a regular expr ./*
	path=""
debug(9, "b=%s path=%s\n", b, path)
dir=path
}




function iso(is)
{
	sss = arr1[split(is, arr1, "/")]
	if (sub(/\.o/,"", sss) == 1 ) 
		return 1
	else
		return -1
}

function isI(s)
{
	if (sub(/\-I/,"", s) == 1 ) {
		return 1
	} else {
		return 0
	}
}

function isa(s)
{
	ss = arr[split(s, arr, "/")]
	if (sub(/\.a/,"", ss) == 1 )
		return 1
	else
		return -1
}

function isl(s, arr, ss)
{
	if (s ~ /^\-l/ )
		return 1
	else
		return -1
}

function isc(s)
{
	ss = arr[split(s, arr, "/")]
	if (sub(/\.c/,"", ss) == 1 )
		return 1
	else
		return -1
}

#top_sort
#globals: snode, enode, _SOURCEh
function top_sort(tmsg, snode, enode, nc, a,b, node, nodecnt, i,front,back)
{
_SOURCEh=""
for (node in pcnt)
	delete pcnt[node]
for (i in slist)
	delete slist[i]
for (i in scnt)
	delete scnt[i]
for (i in q)
	delete q[i]
nodecnt = 0

for (j=1; j<=nc; j++) { #kludge, start=1 0r 0
	a=snode[j]
	if (!(a in pcnt) )
		pcnt[a] = 0
	b=enode[j]
	pcnt[b]++
	slist[a, ++scnt[a] ] = b
}
	back = 0
	for (node in pcnt) {
		nodecnt++
		if (pcnt[node] == 0) {
			#print "no back, pred:" back node
			q[++back] = node
		}
	}
	#print "n=" nodecnt
	for (front = 1; front <= back; front++) {
		node = q[front]
		#printf(" %s", node = q[front])
		#printf(" %s", node )
		_SOURCEh = _SOURCEh " " node
		for (i=1; i<=scnt[node]; i++)
			if (--pcnt[slist[node, i]] == 0)
				q[++back] = slist[node, i]
	}
	if (back != nodecnt) {
		print "warning: TOPSORT failed, cycle found, check-> " tmsg
	}

}

# a sample call : notonlist(hdrlist, dotcdir, iDOTCDIR, restc)
function notonlist(s, list, nlist, al, arr, 	i,n)
{
n = split(s, arr, " ");
#arr["1"] ["2"],...["i"]
for (i=1; i<=nlist; i++)
	if (list[i] == "")
		continue;
	else {
		found=0
		for (j=1; j<=n; j++)
			if (list[i] == arr[j]) 
				found=1	
		if (found == 0) {
			al["1"] = al["1"] " " list[i]	
			#list[i] does not belong to arr/s
		}
				
	}

}

#check ins to see if tar == ddest/bdest
function destins(tar, k, i, tmptar, tmptar1, x)
{
tmptar = tar
#next piece of code removes all multiple occurences of ///->to one /
#KLUDGE : use a for loop with a # of selections made counter
gsub(/\/\//,"/", tmptar)
gsub(/\/\//,"/", tmptar)
gsub(/\/\//,"/", tmptar)
gsub(/\/\//,"/", tmptar)
for (jj in ins) {
	i =  split(jj, arr, SUBSEP);
	tmptar1 = arr["3"] arr["4"]
	gsub(/\/\//,"/", tmptar1)
	gsub(/\/\//,"/", tmptar1)
	gsub(/\/\//,"/", tmptar1)
	gsub(/\/\//,"/", tmptar1)
	# now compare tmptar1 and tmptar
	if (tmptar == tmptar1) {
		x=ins[jj]
		return(x)
	}
}
return(-2)
}

#takes in a ccl, a list of defines,
# sideeffects add to a list of cc flags.
# shcc[] changed
function bld_shcc(ccl,	 arr, i,j,k)
{
print "CC flags(predefined outside makefile): "
k=split(ccl, arr, " ")
for(i=1; i<=k; i++) {
	if (arr[i] ~ /^-D/ ) {
		gsub(/^-D/, "", arr[i])
		print arr[i] " "	
		shcc[arr[i]] = ""
	} else
		print "ignoring " arr[i] 
}
}		

#determines if l has any -D type commands, and returns them
#in  a global list _Dli
#other side effects is to generate a define var list.
# to be printed at the begining. we will use var[], val[] arrays.
function dflags(l, i,k,j, arr, def)
{
_Dli=""
#assume $1, $2, ... etc. are valid arguments
for(k=1; k<=NF; k++) {
	if ($k ~ /^-D/ ) #yes a -D type var
	{
		def = $k	
		gsub(/^-D/, "", def)
		gsub(/\=.*/,"",def)
		defval=$k
		if (gsub(/^-D.*\=/, "", defval) <= 0 )
			defval=""
		if (defval == "") 	#just -DXXX
			defval=$k
		#print "a define: " def ": " defval
		if (def in shcc) {
			#print "already in shcc"
			#ok, do nothing 
			;
		}
		else {
			#aha, this define should go into _Dli
			_Dli = _Dli " " "("def")"
			#also put it in var/val list so that it gets 
			# printed when those variables get printed
			found=0
			for(i=1;i<=iVAR; i++)
				if (var[i] == def )
					found=1 
			if (found == 0) {
				#print "in array"
				iVAR++
				var[iVAR] = def
				val[iVAR] = defval
				var_cnt[iVAR]++
			}	
		}	
	} #if $k is ^-D	

} #for loop over all items in l

#print "_Dli at return=", _Dli
}

function noop(){}

function isin(listy, ele, i, j,k,x ,tmp, arr)
{
if (listy == "")
	return(-1)
tmp=listy
#print listy
x=split(tmp, arr, " ")
for (j=1; j<=x; j++)
	if(arr[j] == ele ) {
		#print listy
		return(1)
	}

#print listy
return(-1)
}
function shenv(sname, svalue,x) {
	system("env >.ajyENV")
	while (getline x <".ajyENV" > 0 ){
		iSHVAR++
		sname = x
		svalue = x
		gsub(/\=.*/,"",sname)	
		gsub(/.*\=/,"",svalue)	
		isshvar[sname] = iSHVAR # a fast search array
		shvar[iSHVAR] = sname
		shval[iSHVAR] = svalue
		#print "< " sname " " svalue " >"
		}	
	#this is the default for cc flags, call bld_shcc multiple times
	# if you want more flags as defaulted outside makefile
	if ("NOPRINTVAR" in isshvar) {
		print "NOPRINTVAR:these variables will not be printed"
		print " 	in the Makefile(s) generated"
		t=shval[isshvar["NOPRINTVAR"]]
		i8=split(t,arr8, " "); print "i=" i8 "t="t;
		for(j=1; j<=i8; j++) {
			noprintvar[arr8[j]]=""		
		}
	}
	CC_SHVAR = "ENVCCF"
	if (CC_SHVAR in isshvar) {
		#print "ok"
		bld_shcc(shval[isshvar[CC_SHVAR]] ); #builds list of cc flags
	}
	else;
	# note that this is used in determining what flags to set
	# during the makefile generation since some flags on CC
	# might be simply a property within that makefile.
	NMAKE_SHVAR = "NMAKE_VER"
	if (NMAKE_SHVAR in isshvar ) {
		if (shval[NMAKE_SHVAR] == "1.4" ) 
			nmake_ver="1.4"
		else if (shval[NMAKE_SHVAR] == "2.0" )
			nmake_ver="2.0"
		else {
			print "1.4 or 2.0 in NMAKE_VER var"	
		}
	} else {
			; #print "using 2.0 nmake version"
	}
#nmake_var is unused currently
}

function plogic(s)
{
	printf("[%s]logic error: call support, results could be incorrect", s);
}

#$0
function find_in_out(s, arr, n,i )
{
n=split(s, arr, " ")
cin="";cout="";
for(i=1;i<=n;i++) {
	if (arr[i] == ">")	
		cout=arr[i+1]
	else if (arr[i] == "<" )
		cin=arr[i+1]
	else if (arr[i] ~ /^\>/ ) {
		gsub(/\>/, "", arr[i] )
		cout=arr[i]
	} else if (arr[i] ~ /^\</ ) {
		gsub(/\</, "", arr[i] )
		cin=arr[i]
	}
}
}

function tar_in_ins(tar, btar, dtar , i, jj)
{
basename(tar);btar=base
dirname(tar); dtar=dir
#print btar " - " dtar
	for (jj in ins) {
		i =  split(jj, arr, SUBSEP);
		gsub(/\/\//, "/", arr["3"]);	
		gsub(/\/\//, "/", arr["3"]);	
		gsub(/\/\//, "/", dtar) 
		gsub(/\/\//, "/", dtar) 
		if (dtar == "") dtar="./"	
		if (arr["3"] == "") arr["3"]="./"
		#print arr["3"] " - " arr["4"]
	
		if ((btar == arr["4"] ) && (dtar == arr["3"] ) )
		{
			print tar " in install????" 
			print arr["1"] arr["2"] "->" dtar btar
			print "WARNING: CHECK Makefile.0 "
		}	
	} #for

}
			#ins[dsrc, bsrc, ddest, bdest] = kCMD

#rule for patterns
# syntax : 
# <cmd> <options> <name> < <fname>
# cat <fname> | <cmd> <options> <name> - this is not implemented.
# .p : .pat .CMD.xx
function rule_pat(s, myi, jmy, i,j,k,jj, jmy)
{
cin="";cout=""
find_in_out(s); #use s instead later
fname=cin
opts=""
	for (k=jmy+1; k<=NF; k++){
		if ($k ~ /\-/) {
			print "Warning: options on pattern could be ignored"
			opts=opt " " $k
			#options, use them
		} else if ($k ~ /</ ) { 
			#break out of loop
			break;
		} else {
			name=$k
			break;
		}
	} #for loop	
	#print "name=" name " fname=" fname
	#KLUDGE 1 is for patterns PATTERNS
	#KLUDGE 0 is for COMMANDS variables.
	val[1] = val[1] " " name".p"
	var_cnt[1]++

	dirname(fname);basename(fname);
	if (dir == "") dir = ""
	dsrc=dir; bsrc=base; ddest="./"; bdest=name".p" ; 
	kCMD++
	ins[dsrc, bsrc, ddest, bdest] = kCMD
	insorder[kCMD]=dsrc SUBSEP bsrc SUBSEP ddest SUBSEP bdest
	instype[kCMD]="pattern"
#	print "insorder" kCMD " {}-"insorder[kCMD]

	for(k=1; kcmd[kCMD,k]!=""; k++);
	kcmd[kCMD,k] = sprintf("\t %s %s %s < $(<)", vcommand(myi), opts, name)
	instdir[ddest]=""
}

function isfile(path, tmp)
{
#use if this is set
tmp=sprintf("test -f %s", path)
if (system(tmp) == 0 ) 
	return 1
else 
	return 0
}

function rm_vpath(path, vpath, i, varr, parr,iv, j, k, arr, tmp)
{
	#KLUDGE AND INEFFICIENT!
	for (i=0;i<=15; i++)
		gsub(/\/\//, "", path) 
	for (i=0;i<=15; i++)
		gsub(/\/\//, "", vpath) 
	iv=split(vpath, varr, ":")
	ip=split(path, parr,  "/");
	for (i=1; i<=iv;  i++) { #for each vpath path
			
	
	}
}

function abs2relpath(path, i, j)
{

}

function vcommand(command, i, j, k)
{
for(i=1; i<=iVAR; i++)
	if (val[i] == command) {
		var_cnt[i]++
		return("$("var[i]")")
	}
return(command);
}

function varfill(sname, svalue,x) {
	if (varfile == "_")
		return
	if (isfile(varfile) <= 0 ) {
		print "ERROR: cannot find file: " varfile
		return
	}
	debug(7, "varfile:%s\n" varfile);	
	while (getline x <varfile > 0 ){
		iVAR++
		sname = x
		svalue = x
		gsub(/\=.*/,"",sname)	
		gsub(/.*\=/,"",svalue)	
		isvar[sname] = iVAR # a fast search array
		var[iVAR] = sname
		val[iVAR] = svalue
		var_cnt[iVAR]=0 #increment number of times used.
		debug(7, "%s %s\n", sname ,svalue); 
		}	
}
function invarfill(sname, svalue,x) {
	if (invarfile == "_" )
		return
	if (isfile(invarfile) <= 0 ) {
		print "ERROR: cannot find: " invarfile
		return
	}
	debug(7, "invarfile:%s\n" invarfile);	
	while (getline x <invarfile > 0 ){
		#iINVAR++
		sname = x
		svalue = x
		gsub(/\=.*/,"",sname)	
		gsub(/.*\=/,"",svalue)	
		svalue=compress(svalue)
		invar_cnt[sname]=0  #counts how many times sname used!
		### added for X=xxx:xx:yyy:zzz kind of assignments.
		ia=split(svalue, arr,  ":" );
		debug(9, "ia=%d", ia)
		if (ia > 1) {
			debug(5, ": define found! %s\n", sname);
			isinvar[sname] = iVAR # a fast search array
			if (arr["1"] ~ /^\+.*/){
				tmpa=arr["1"];
				sub(/^\+/,"", tmpa); 
				invar[sname]=tmpa #use first define...as default
			} else 
				invar[sname]=arr["1"] #use first define...as default
			debug(9, "invar[%s]=%s\n", sname , invar[sname])
			invar_more[sname]=1#this indicates if we need
				#to search the var_i/val_i arrays... 
			for (i=1; i<=ia; i++) {
				debug(9, ":%s", arr[i]);	
				if (arr[i] ~ /^\+.*/ ) {
					debug(9, "+VPATH");
					sub(/^\+/,"", arr[i]);
					print "VPATH +"arr[i]
					itill=iIN;
					for(iti=0; iti<=itill; iti++){
						if (var_i[iti] == "VPATH"){
							iIN++;
							var_i[iIN]=sname
				val_i[iIN]=compress(val_i[iti] "/" arr[i])
				debug(7, ":+(%s=%s)\n", sname ,val_i[iIN]); 
						}
					}
				} else {
				iIN++
				var_i[iIN]=sname
				val_i[iIN]=compress(arr[i])
				debug(7, ":%s %s\n", sname ,val_i[i]); 
				}
			}
		} else  { #if a : separated define exists ...	
			isinvar[sname] = iVAR # a fast search array
			invar[sname]=svalue
			invar_cnt[sname]=0  #counts how many times sname used!
			debug(7, "%s %s\n", sname ,svalue); 
			if (svalue ~ /^\.\/.*/ ) {
				invar_more[sname]=1
				iIN++
				var_i[iIN]=sname
				val_i[iIN]=svalue
				sub(/^\.\//, "", val_i[iIN]);
			}
	
		} #regular define with no colons:...
		}#while allinvars in invarfile...
}

#puts time stamp in the makefile generated.
function ts_makefile(d, x)
{
	printf("note # # make abstract machine file generated by mg # #\n");
	if (isfile(premakefile) >= 0 ) {
		while (getline x <premakefile >0)
			printf("%s\n", x) 
		printf("\n\n") ;
	}	
}

#returns a variable or NULL string depending on if it is
#defined.
function v2n(val, pwd, e, i, j, k, tmp, arr, val1)
{
e=""
nm=subinvar(val)
if (val != nm ) {
	#print "found a sub " val ":" nm
	return(nm);
}
#print "v2n: " "val=" val"<" " pwd=" pwd " fv2n=" fv2n"<"
if (fv2n != "_" ) {
	print "shell: " fv2n " "val
	fv2n " "val " "KLU++ | getline e
		#print "e=" e
	if (e == "" ) {
		#try another variation
		val1 = val"/"
		print "shell: " fv2n " "val1
		fv2n " "val1 " "KLU++ | getline e
	}
	return e
} else
	return NULL
}

#in:a list "/eald ../sdlfk /./..sdf" etc.
#out: a list with variables substitutes.
#typiclaly used in .SOURCE.h, .a...etc. lines
function v2(ilist, ia, i, vval, vvar, jlist, vfdir)
{
	for (i in ztmp)
		delete ztmp[i]
	jlist=""
	ia=split(ilist, arr,  " ");
	for (i=1; i<=ia; i++) {
		vval=arr[i]
		vval=compress(vval)	
		#print "vval=" vval
		#vvar=v2n(vval);
		vvar=compress(subinvar(vval)) ;
		debug(1, "S: %s %s\n", vval, vvar);
		if (vvar in ztmp) 
			debug(9, "already in S:%s\n", vvar)
		else {
			ztmp[vvar]=1
			jlist = jlist " " vvar
		}
	}
return(jlist);

#this piece of code is extinct below:::
		if (vvar == NULL ) {
		#check the INSTDIR list...
		found=0
		for (vdir in invar) {
			vali = invar[vdir]
			vali1=compress(vali)
			if (vval == vali1) {
				vfdir=vdir
				debug(6, "ffvdir=%s vval=%s\n" vfdir , vali1)
				found=1
				invar_cnt[vdir]++
			break;
			}
		}
		if (found == 1)
			jlist = jlist " " "$("vfdir")"
		        else
				jlist = jlist " " arr[i]		
	}
}

function compress(xval, i, val)
{
if (xval ~/^\.\/.*/ )
	debug(9, "WARNING: ./ matching will fail %s", xval);
val=xval
#no doble slashes; KLUDGE
	for (i=0;i<=5; i++)
		gsub(/\/\//, "/", val) 
#no trailing single / eg. /usr/include/ => /usr/include
	if (val == "/" )  {
		#cannot remove this single slash! so return...
		debug(3, "compress: single slash\n");
		return(val);
	}
	gsub(/\/$/, "", val);
return(val);
}

#substiture the s with invar[VAR]=VAL, using first match!!!
#bug removal1: ./ or "" matching prohibited...
function subinvar(s1, myvar, i, j, k, match_length, match_val, match_var)
{
s=compress(s1)
debug(8,"subinvar:%s -compress> %s\n", s1, s)
match_length=0
match_val=""
match_var=""
for(yvar in invar) {
	yval=invar[yvar]
	debug(8, "subinvar: check %s=%s\n", yvar, yval);
	if (length(yval) < match_length) #we do not want a smaller match!
			continue

	if ((yval == "") || (yval == "./") ) {
		#cant match this noops.
		continue
	}
	r=""
	i=index(s, yval);
	#print i "-" s "/" length(s) "-" yval "/"length(yval)
	r=substr(s, length(yval)+1 )	
	debug(8, "index: i=%d, s=%s yval=%s r=%s\n", i, s, yval, r);
	if ((r ~ /\/.*/) || (r == "") )
		debug(8, "ok\n");	
	else {
		debug(8, "no match %s %s i=%d\n",s, r, i);
		continue	
		}	
	if (i== 1) {
		#print yvar " val=" yval "len=" length(yval)
		r=substr(s, length(yval)+1 )	
	if ((r ~ /\/.*/) || (r == "") )
		debug(8, "ok\n");	
	else {
		debug(8, "no match %s %s i=%d\n",s, r, i);
		continue	
	}	
		#print "r=" r
		debug(8, "yvar=%s %d SUB=%s\n", yvar, invar_cnt[yvar], s)
		match_var = yvar
		match_val = "$(" yvar ")/" r;
		match_length=length(yval)
	}
} #for match
debug(8, ":check var_i/val_i arrays for maxmatch \n");
############new codebelow for checking val_i/var_i lists.
for(j=0;j<=iIN; j++) {
	r=""
	yvar=var_i[j]
	yval=val_i[j]
	debug(8, ":%s %s \n", yvar, yval)
	if (length(yval) < match_length) #we do not want a smaller match!
			continue

	if ((yval == "") || (yval == "./") ) {
		#cant match this noops.
		continue
	}
	i=index(s, yval);
	if (i== 1) {
		debug(8, "ahh! var_i/val_i found\n")
		#print yvar " val=" yval "len=" length(yval)
		r=substr(s, length(yval)+1 )	
	if ((r ~ /\/.*/) || (r == "") )
		noop();#ok
	else {
		debug(8, "no match %s %s i=%d\n",s, r, i);
		continue	
	}	
		#print "r=" r
		debug(8, ":yvar=%s %d SUB=%s\n", yvar, invar_cnt[yvar], s)
		match_var = yvar
		match_val = "$(" yvar ")/" r;
		match_length=length(yval)
	}
}#loop thru all var_i	

if (match_length > 0) {
	#something matched!
	invar_cnt[match_var]++
	debug(4, "\tmatch:%s %s\n", match_val, s)
	return(match_val)
}
#else return same old string...
debug(4, "NO MATCH:%s\n", s1);
return(s1)

}

		#return ("$(" yvar ")/" r);

#given a string s, determines any .SOURCE.* lines for
#modification...
function spath(s)
{
	if (isc(s) > 0 ) {
		dirname(s);
		if (!(dir in dotcdir_a) ) {
			dotcdir_a[dir]="y"
			iDOTCDIR++
			dotcdir[iDOTCDIR]=dir
		}
	} else {
		dirname(s);
		if (dir != "") 
			print "add .SOURCE: line manually" dir " for " s
	}
}

#converts a any file to a .ln file
function any2ln(s, tmp)
{
	if (isc(s) > 0) {
		#is a .c so get base name and convert...
		basename(s);
		tmp=s
		sub(/\.c$/, "", tmp)
		return(tmp".ln")	
	} else {
		return(s".ln")
	}
}
function nl(Plist,Llist,Larr,Lc,Li, thisline)
{
	Llist=Plist;
	Lc=split(Llist,Larr," ");
	thisline=0
	maxline=MAXPERLINE
	if (maxline <= 0) 
		maxline=1
	#print maxline
	for (Li=1;Li<=Lc;Li++)
	{
	#print thisline maxline Larr[Li]
		if (thisline < maxline){
			printf("\t%s ",Larr[Li]) 
			thisline++
		}else {
			#here we have printed max on this line, start newline
			printf("\\\n") ;
			printf("\t%s ",Larr[Li]) 
			thisline=1
		}
	}
	#printf("\t\t%s\n\n",Larr[Li]) 
	printf("\n\n") 
}

function debug(level, con_str, a,b,c,d,e,f,g,h,i,j,k,l)
{
if (level > debug_level )
	return
printf(con_str, a,b,c,d,e,f,g,h,i,j,k,l)
}

function virtual_g(a,b,c, 	tmp)
{
return(avu[a,b,c]);
}

function virtual_n(a,b,c,	i,j,k,arr,tmp)
{
tmp=a SUBSEP b SUBSEP c
if (tmp in avu) {
	print "Virtual error:duplicate" a "--" b "--"  c
} else {
	usage[c]++
	if (usage[c] == 1)
		avu[a,b,c]="V_" c
	else
		avu[a,b,c]="V_" c usage[c]
}
return(avu[a,b,c]);
}

function print_dep_new()
{
}
#returns "" or some more dependencies...
function f_ldep_more(ind, iorder, i,j,k,tmp,arr, jj, itype, jjn, dir, nm)
{
return(ldep_more[ind]);
if (ind in ldep_more) {
	debug(9,"ok");
}else {
	debug(1, "nomore for %s", ldep[i]);
	return("");
}
#here there are more targets, ....
#following code from install dire...
	iorder=ldep_more[ind] #k is the index into ins array
		jj=insorder[iorder]  #jj is the unique install option
		itype = instype[iorder]
		jjn = split(jj, arr, SUBSEP);
		dir = arr["3"]			
		nm=subinvar(dir);
		debug(1,"more returns: %s\n", compress(nm) );
		return(compress(nm))
}

function vircnv(s, tmp)
{
	gsub(/\</, ">", s)	
	return(s);
}
