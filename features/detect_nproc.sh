#!/bin/sh
#
# Exit with a status reflecting the CPU count if we can determine it. If not
# assume two CPUs since at least one test, API/trehash.c, deadlocks if the CPU
# count is one.
if getconf _NPROCESSORS_ONLN >/dev/null 2>&1
then
	# Some systems have a getconf that supports this var. Use it in preference
	# to the options below because those other mechanisms include offline CPUs.
	exit $(getconf _NPROCESSORS_ONLN)
elif type nproc >/dev/null 2>&1
then
	# Linux and macOS but not other BSD.
	exit $(nproc)
elif sysctl -n hw.ncpu >/dev/null 2>&1
then
	# BSD
	exit $(sysctl -n hw.ncpu)
fi
exit 2
