# This script is used to run tests in Meson
script_path=`realpath "$0"`
bin_dir=`dirname "$script_path"`
base_dir=`dirname "$bin_dir"`

export SHELL="$base_dir/build/src/cmd/ksh93/ksh"
cd "$base_dir/src/cmd/ksh93/tests"

# shtests executes tests in 3 different modes: posix, utf8 and shcomp
# by default, all 3 types are on.
# TODO: Skip shcomp tests for now and enable them later
"$SHELL" ./shtests --posix --utf8
