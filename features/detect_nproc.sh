#!/bin/sh
#
# Exit with a status reflecting the CPU count if we can determine it. If not
# assume two CPUs since at least one test, API/trehash.c, deadlocks if the CPU
# count is one.
nproc=''
if getconf _NPROCESSORS_ONLN >/dev/null 2>&1
then
	# Some systems have a getconf that supports this var. Use it in preference
	# to the options below because those other mechanisms include offline CPUs.
	nproc=$(getconf _NPROCESSORS_ONLN)
elif type nproc >/dev/null 2>&1
then
	# Linux and macOS but not other BSD.
	nproc=$(nproc)
elif sysctl -n hw.ncpu >/dev/null 2>&1
then
	# BSD
	nproc=$(sysctl -n hw.ncpu)
fi

if [ -z "$nproc" ]
then
	nproc=2
elif [ $nproc -lt 2 ]
then
	nproc=2
elif [ $nproc -gt 64 ]
then
	nproc=64
fi
exit $nproc
