#!/usr/bin/env bash
set -e

# This script is used to setup iffe and other commands that are required for compilation

# This script is run from an unspecified directory so we have to determine directory paths
# http://mesonbuild.com/Reference-manual.html#run_command
if type realpath >/dev/null 2>&1
then
    script_path=`realpath "$0"`
elif type grealpath >/dev/null 2>&1
then
    script_path=`grealpath "$0"`
else
    echo Sorry but your system does not have a realpath or grealpath command >&2
    exit 1
fi
bin_dir=`dirname "$script_path"`
base_dir=`dirname "$bin_dir"`

echo "#!/usr/bin/env bash" > "$bin_dir/iffe"
cat "$base_dir/src/cmd/INIT/iffe.sh" >> "$bin_dir/iffe"
chmod u+x "$bin_dir/iffe"

cp "$base_dir/src/lib/libast/comp/conf.sh" "$bin_dir/conf"
chmod u+x "$bin_dir/conf"
