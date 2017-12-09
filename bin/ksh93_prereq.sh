#!/usr/bin/env bash

# This script is used for feature detections that are required to build ksh93
set -x
set -e

# This script is run from an unspecified directory so we have to determine directory paths
# http://mesonbuild.com/Reference-manual.html#run_command
script_path=`realpath "$0"`
bin_dir=`dirname "$script_path"`
base_dir=`dirname "$bin_dir"`

PATH=$bin_dir:$PATH

iffe_tests=( options poll )

iffe_tests_2=( nvapi shellapi )

iffe_tests_3=( nfsd acct execargs pstat )

function cc_fun {
    cc -D_BLD_ast -I../../../lib/libast/include/ -I../../../lib/libast/features/  "$@"
}

export -f cc_fun

pushd "$base_dir/src/cmd/ksh93/features"

for iffe_test in ${iffe_tests[@]}; do
    iffe -v -X ast -X std -c cc_fun run $iffe_test
done

for iffe_test in ${iffe_tests_2[@]}; do
    iffe -v -X ast -X std -c 'cc' run $iffe_test
    cp "$base_dir/src/cmd/ksh93/features/FEATURE/$iffe_test" "$base_dir/src/cmd/ksh93/features/$iffe_test.h"
done

for iffe_test in ${iffe_tests_3[@]}; do
    iffe -v -c 'cc' : def $iffe_test
done

# Generate a c source file for ksh93 bash compatiblity
echo "const char bash_pre_rc[] = " > "$base_dir/src/cmd/ksh93/data/bash_pre_rc.c"
sed -e 's/\\/\\\\/g' -e 's/"/\\"/g' -e 's/'"'"'/\\'"'"'/g' -e 's/^[[:space:]]*\(.*\)$/\"\1\\n\"/' "$base_dir/src/cmd/ksh93/data/bash_pre_rc.sh" >> "$base_dir/src/cmd/ksh93/data/bash_pre_rc.c"
echo ";" >> "$base_dir/src/cmd/ksh93/data/bash_pre_rc.c"

popd
