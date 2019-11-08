# Tests for `uname` builtin

# ==========
# uname -x
# Verify an unknown option is handled as a usage error.
actual=$(uname -x 2>&1)
expect="uname: -x: unknown option"
[[ "$actual" =~ "$expect".* ]] || log_error "uname -x" "$expect" "$actual"

# ==========
#   -a, --all       Equivalent to -snrvmpio.
actual=$(uname -a)
expect=$($bin_uname)
[[ "$actual" =~ "$expect" ]] || log_error "'uname -a' failed" "$expect" "$actual"

# ==========
# -s, --system|sysname|kernel-name
# The detailed kernel name. This is the default.
actual=$(uname -s)
expect=$($bin_uname -s)
[[ "$actual" = "$expect" ]] || log_error "'uname -s' failed" "$expect" "$actual"

# ==========
# -n, --nodename  The hostname or nodename.
actual=$(uname -n)
expect=$($bin_uname -n)
[[ "$actual" = "$expect" ]] || log_error "'uname -n' failed" "$expect" "$actual"

# ==========
# -r, --release|kernel-release
# The kernel release level.
actual=$(uname -r)
expect=$($bin_uname -r)
[[ "$actual" = "$expect" ]] || log_error "'uname -r' failed" "$expect" "$actual"

# ==========
# -v, --version|kernel-version
# The kernel version.
# Ugh! On FreeBSD the external command adds a space to the end of the line so remove it.
actual=$(uname -v)
expect=$($bin_uname -v | sed -e 's/ *$//')
[[ "$actual" = "$expect" ]] || log_error "'uname -v' failed" "$expect" "$actual"

# ==========
# -m, --machine   The name of the hardware type the system is running on.
actual=$(uname -m)
expect=$($bin_uname -m)
[[ "$actual" = "$expect" ]] || log_error "'uname -m' failed" "$expect" "$actual"

# ==========
# -p, --processor The name of the processor instruction set architecture.
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
# -o, --operating-system
# The generic operating system name. Some systems (e.g., macOS) don't have a `-o` flag but usually
# their `-s` flag is an acceptable substitute. However, on Solaris clones like IllumOS and
# Openindiana the value is the prefix of the `uname -v` output. Note that the ksh93u+ release
# reports "SunOS" for `uname -o`. So there isn't any point changing the implementation. Instead,
# adapt the unit test.
actual=$(uname -o)
expect=$($bin_uname -o 2>/dev/null || $bin_uname -s 2>&1)
[[ "$expect" =~ "$actual" || "$actual" == "SunOS" ]] ||
    log_error "'uname -o' failed" "$expect" "$actual"

# ==========
# -h, --host-id|id
# The host id in hex.
uname -h | grep -q -v "[0-9a-f]*" && log_error "'uname -h' failed"

# ==========
# -d, --domain
# The domain name returned by getdomainname(2). We can't actually verify the output because it may
# be an empty string or a non-empty string. This is mostly to verify the flag is handled and ensure
# the associated code is covered.
actual=$(uname -d)
# [[ ! -z "$actual" ]] || log_error "'uname -d' failed"
[[ $? == 0 ]] || log_error "'uname -d' failed"
