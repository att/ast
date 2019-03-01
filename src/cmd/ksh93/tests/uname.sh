# Tests for `uname` builtin

if [[ $OS_NAME != "Linux" ]]
then
    log_info "Output of this command varies on different operating systems, so these tests are run only on Linux"
    exit 0
fi

bin_uname=$(which uname)

# ==========
#   -a, --all       Equivalent to -snrvmpio.
actual=$(uname -a)
expect=$($bin_uname)
[[ "$actual" =~ "$expect" ]] || log_error "'uname -a' failed" "$expect" "$actual"

# ==========
#   -s, --system|sysname|kernel-name
#                   The detailed kernel name. This is the default.
actual=$(uname -s)
expect=$($bin_uname -s)
[[ "$actual" = "$expect" ]] || log_error "'uname -s' failed" "$expect" "$actual"

# ==========
#   -n, --nodename  The hostname or nodename.
actual=$(uname -n)
expect=$($bin_uname -n)
[[ "$actual" = "$expect" ]] || log_error "'uname -n' failed" "$expect" "$actual"

# ==========
#   -r, --release|kernel-release
#                   The kernel release level.
actual=$(uname -r)
expect=$($bin_uname -r)
[[ "$actual" = "$expect" ]] || log_error "'uname -r' failed" "$expect" "$actual"

# ==========
#   -v, --version|kernel-version
#                   The kernel version level.
actual=$(uname -v)
expect=$($bin_uname -v)
[[ "$actual" = "$expect" ]] || log_error "'uname -v' failed" "$expect" "$actual"

# ==========
#   -m, --machine   The name of the hardware type the system is running on.
actual=$(uname -m)
expect=$($bin_uname -m)
[[ "$actual" = "$expect" ]] || log_error "'uname -m' failed" "$expect" "$actual"

# ==========
#   -p, --processor The name of the processor instruction set architecture.
actual=$(uname -p)
# Coreutils `uname -p` is unportable, instead use `uname -m`
expect=$($bin_uname -m)
[[ "$actual" = "$expect" ]] || log_error "'uname -p' failed" "$expect" "$actual"

# ==========
#   -i, --implementation|platform|hardware-platform
#                   The hardware implementation; this is --host-id on some
#                   systems.
actual=$(uname -i)
# Coreutils `uname -i` is unportable, instead use `uname -m`
expect=$($bin_uname -m)
[[ "$actual" = "$expect" ]] || log_error "'uname -i' failed" "$expect" "$actual"

# ==========
#   -o, --operating-system
#                   The generic operating system name.
actual=$(uname -o)
expect=$($bin_uname -o)
[[ "$expect" =~ "$actual" ]] || log_error "'uname -o' failed" "$expect" "$actual"

# ==========
#   -h, --host-id|id
#                   The host id in hex.
uname -h | grep -q -v "[0-9a-f]*" && log_error "'uname -h' failed"

# ==========
#   -d, --domain    The domain name returned by getdomainname(2).
actual=$(uname -d)
[[ ! -z "$actual" ]] || log_error "'uname -d' failed"

# ==========
#   -R, --extended-release
#                   The extended release name.
actual=$(uname -R)
expect=$(uname)
[[ "$actual" =~ "$expect" ]] || log_error "'uname -R' failed" "$expect" "$actual" "$expect" "$actual"

# ==========
#   -A, --everything
#                   Equivalent to -snrvmpiohdR.
actual=$(uname -A)
expect=$(uname -snrvmpiohdR)
[[ "$actual" = "$expect" ]] || log_error "'uname -A' failed" "$expect" "$actual"

# ==========
#   -f, --list      List all sysinfo(2) names and values, one per line.
actual=$(uname -f)
[[ ! -z "$actual" ]] || log_error "'uname -f' should be non-empty" "$expect" "$actual"

# ==========
#   -S, --sethost=name
#                   Set the hostname or nodename to name. No output is written to
#                   standard output.
# Try to run this command as non-root user, it should fail.
if [[ $(id -u) -ne 0 ]]; then
    actual=$(uname -S foo 2>&1)
    expect="uname: foo: cannot set host name"
    [[ "$actual" =~ "$expect" ]] || log_error "'uname -S' should fail when not run as root" "$expect" "$actual"
fi
