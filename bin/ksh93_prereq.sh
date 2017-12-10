#!/usr/bin/env bash

# This script is used for generating bash compatibility source file that is required to build ksh93
set -x
set -e

# This script is run from an unspecified directory so we have to determine directory paths
# http://mesonbuild.com/Reference-manual.html#run_command
script_path=`realpath "$0"`
bin_dir=`dirname "$script_path"`
base_dir=`dirname "$bin_dir"`

pushd "$base_dir/src/cmd/ksh93/features"

# Generate a c source file for ksh93 bash compatiblity
echo "const char bash_pre_rc[] = " > "$base_dir/src/cmd/ksh93/data/bash_pre_rc.c"
sed -e 's/\\/\\\\/g' -e 's/"/\\"/g' -e 's/'"'"'/\\'"'"'/g' -e 's/^[[:space:]]*\(.*\)$/\"\1\\n\"/' "$base_dir/src/cmd/ksh93/data/bash_pre_rc.sh" >> "$base_dir/src/cmd/ksh93/data/bash_pre_rc.c"
echo ";" >> "$base_dir/src/cmd/ksh93/data/bash_pre_rc.c"

popd
