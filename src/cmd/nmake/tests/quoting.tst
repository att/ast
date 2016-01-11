# ast nmake quoting tests

INCLUDE test.def

TEST 01 'edit ops'

	EXEC	-n
		INPUT Makefile $'X = "(parenthesis)"
all :
	: $(X)
	: $(X:/$("(")/_/G)
	: $(X:/[()]/_/G)'
		OUTPUT - $'+ : "(parenthesis)"
+ : "_parenthesis)"
+ : "_parenthesis_"'

TEST 02 'state var quote'

	EXEC	-n
		INPUT Makefile $'TST == "tst aha"
tst :: tst.sh'
		INPUT tst.sh $'echo $TST'
		OUTPUT - $'+ case message:$OPTIND:$RANDOM in
+ ?*:*:*|*::*|*:*:$RANDOM)
+ 	;;
+ *)	if	ENV= LC_ALL=C x= $SHELL -nc \'[[ a || b ]] && : ${list[level]} $(( 1 + $x )) !(pattern)\' 2>/dev/null
+ 	then	if	grep \'### .*archaic.* ###\' >/dev/null
+ 		then	: tst contains archaic constructs :
+ 		else	ENV= LC_ALL=C $SHELL -n tst.sh
+ 		fi
+ 	fi
+ 	;;
+ esac
+ case \'\' in
+ "")	case 13 in
+ 	0)	cp tst.sh tst
+ 		;;
+ 	*)	{
+ 		i=`(read x; echo $x) < tst.sh`
+ 		case $i in
+ 		\'#!\'*|*\'||\'*|\':\'*|\'":"\'*|"\':\'"*)	echo "$i" ;;
+ 		esac
+ 		cat - tst.sh <<\'!\'
+ TST="tst aha"
+ !
+ 		} > tst
+ 		;;
+ 	esac
+ 	;;
+ *)	cat - tst.sh > tst <<\'!\'
+ 
+ TST="tst aha"
+ !
+ 	;;
+ esac
+ silent test -w tst -a -x tst || chmod u+w,+x tst'
