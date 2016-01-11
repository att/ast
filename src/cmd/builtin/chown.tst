KEEP *.map

function DATA
{
	for f
	do	test -f $f && continue
		case $f in
		chown.map)
			print $(id -nu):$(id -ng) 1:2
			;;
		esac > $f
	done
}

TEST 01 maps
	DO DATA chown.map
	EXEC -nX -m chown.map chown.map
		OUTPUT - $'chown uid:00000->00001 gid:00000->00002 chown.map'
