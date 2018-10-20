# Tests for `chmod` builtin.
#
# TODO: Add many more tests. Especially for optional behaviors such as how symlinks are handled.
# At the moment this is primarily to avoid a regression of issue #949.


# Create some files to use in this test.
umask 077  # start by disabling setting all group/other permission bits by default
mkdir d
touch a b d/e d/f

# Create a function to report the pathname permissions in a fashion that works on BSD, Linux, and
# other platforms. This could be simplified if ksh supported a `stat` builtin.
if [[ $(stat -f '%Sp' a 2>/dev/null) == '-rw-------' ]]
then
    # This appears to be a BSD `stat` command that supports the `-f` flag.
    function stat_perms {
        stat -f '%Sp' "$1"
    }
elif [[ $(stat --format '%A' a 2>/dev/null) == '-rw-------' ]]
then
    # This appears to be a GNU `stat` command that supports the `-A` flag.
    function stat_perms {
        stat -c '%A' "$1"
    }
else
    # We have no idea how to report pathname permissions in a human readable form.
    log_error "no usable 'stat' command found"
    function stat_perms {
        print ''
    }
fi

# ==========
# Quick sanity check that the perms are as expected before being modified below.
expect='-rw-------'
actual=$(stat_perms a)
[[ $actual == $expect ]] || log_error "file 'a' perms wrong at start of test" "$expect" "$actual"
actual=$(stat_perms d/f)
[[ $actual == $expect ]] || log_error "file 'd/f' perms wrong at start of test" "$expect" "$actual"

# ==========
# We expect `chmod` to be enabled as a builtin when this test is run. We don't bother to skip the
# subsequent tests because it is sufficient to log that we're not testing the builtin.
[[ $(whence chmod) == '/opt/ast/bin/chmod' ]] || log_error "chmod is not a builtin"

# ==========
# Can we change the perms on files and dirs without affecting the files in the dir.
# Test by adding "group" read permission without recursion into dirs.
chmod g+r a d b
expect='-rw-r-----'
actual=$(stat_perms a)
[[ $actual == $expect ]] || log_error "file 'a' perms wrong" "$expect" "$actual"
actual=$(stat_perms b)
[[ $actual == $expect ]] || log_error "file 'b' perms wrong" "$expect" "$actual"

expect='drwxr-----'
actual=$(stat_perms d)
[[ $actual == $expect ]] || log_error "dir 'd' perms wrong" "$expect" "$actual"

# These two files should not have been modified.
expect='-rw-------'
actual=$(stat_perms d/e)
[[ $actual == $expect ]] || log_error "file 'd/e' perms wrong" "$expect" "$actual"
actual=$(stat_perms d/f)
[[ $actual == $expect ]] || log_error "file 'd/f' perms wrong" "$expect" "$actual"

# ==========
# Can we change the perms on files and dirs including the files in the dir.
# Test by adding "other" write permission without recursion into dirs.
chmod -R o+w b a d
expect='-rw-r---w-'
actual=$(stat_perms a)
[[ $actual == $expect ]] || log_error "file 'a' perms wrong" "$expect" "$actual"
actual=$(stat_perms b)
[[ $actual == $expect ]] || log_error "file 'b' perms wrong" "$expect" "$actual"

expect='drwxr---w-'
actual=$(stat_perms d)
[[ $actual == $expect ]] || log_error "dir 'd' perms wrong" "$expect" "$actual"

# These two subdir files should have been modified.
expect='-rw-----w-'
actual=$(stat_perms d/e)
[[ $actual == $expect ]] || log_error "file 'd/e' perms wrong" "$expect" "$actual"
actual=$(stat_perms d/f)
[[ $actual == $expect ]] || log_error "file 'd/f' perms wrong" "$expect" "$actual"

# ==========
# Setting multiple bits using human friendly form.
chmod o=rw,g=x,u-w d/e
expect='-r----xrw-'
actual=$(stat_perms d/e)
[[ $actual == $expect ]] || log_error "file 'd/e' perms wrong" "$expect" "$actual"

# ==========
# Setting perms using numeric form.
chmod 755 a d
expect='-rwxr-xr-x'
actual=$(stat_perms a)
[[ $actual == $expect ]] || log_error "file 'a' perms wrong" "$expect" "$actual"
expect='drwxr-xr-x'
actual=$(stat_perms d)
[[ $actual == $expect ]] || log_error "dir 'd' perms wrong" "$expect" "$actual"
