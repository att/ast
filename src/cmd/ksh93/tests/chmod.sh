# Tests for `chmod` builtin.
#
# At the moment this is primarily to avoid a regression of issue #949.

# chmod - change the access permissions of files

#  chmod changes the permission of each file according to mode, which can be
#  either a symbolic representation of changes to make, or an octal number
#  representing the bit pattern for the new permissions.
#

umask 077  # start by disabling setting all group/other permission bits by default
mkdir foo
ln -s foo symlink_to_foo
touch foo/bar
ln -s bar symlink_to_bar

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
    exit 0
fi

# ==========
#  -H, --metaphysical
#                  Follow symbolic links for command arguments; otherwise don't
#                  follow symbolic links when traversing directories.
# chmod -H 0700 "$TEST_DIR/symlink_to_foo"
# actual=$(stat_perms "$TEST_DIR/foo/bar")
# expect="-rwx------"
# [[ "$actual" = "$expect" ]] || log_error "chmod -H should follow symbolic links" "$expect" "$actual"

# ==========
#  -L, --logical|follow
#                  Follow symbolic links when traversing directories.
chmod -RL 0777 "$TEST_DIR/symlink_to_foo"
actual=$(stat_perms "$TEST_DIR/foo/bar")
expect="-rwxrwxrwx"
[[ "$actual" = "$expect" ]] || log_error "chmod -L should follow symbolic links" "$expect" "$actual"

# ==========
#  -P, --physical|nofollow
#                  Don't follow symbolic links when traversing directories.
# expect=$(stat_perms "$TEST_DIR/foo/bar")
# chmod -RP 0755 "$TEST_DIR/symlink_to_foo"
# actual=$(stat_perms "$TEST_DIR/foo/bar")
# [[ "$actual" = "$expect" ]] || log_error "chmod -P should not follow symbolic links" "$expect" "$actual"

# ==========
#  -R, --recursive Change the mode for files in subdirectories recursively.
chmod -R 0777 "$TEST_DIR/foo"
actual=$(stat_perms "$TEST_DIR/foo/bar")
expect="-rwxrwxrwx"
[[ "$actual" = "$expect" ]] || log_error "chmod -R failed to change permissions" "$expect" "$actual"

# ==========
#  -c, --changes   Describe only files whose permission actually change.
actual=$(chmod -c 0755 "$TEST_DIR/foo/bar")
expect="mode changed to 0755 (rwxr-xr-x)"
[[ "$actual" =~ "$expect" ]] || log_error "chmod -c failed to change permissions"

# ==========
#  -f, --quiet|silent
#                  Do not report files whose permissioins fail to change.
actual=$(chmod -f 0777 "$TEST_DIR/this_file_does_not_exit")
expect=""
[[ "$actual" = "$expect" ]] || log_error "chmod -f should give empty output" "$expect" "$actual"

# ==========
#  -h|l, --symlink Change the mode of symbolic links on systems that support
#                  lchmod(2). Implies --physical.
actual=$(chmod -vl 0777 "$TEST_DIR/symlink_to_foo")
expect="$TEST_DIR/symlink_to_foo: mode changed to 0777 (rwxrwxrwx)"
[[ "$actual" = "$expect" ]] || log_error "chmod -l failed should change permissions on symbolic link"

# ==========
#  -i, --ignore-umask
#                  Ignore the umask(2) value in symbolic mode expressions. This
#                  is probably how you expect chmod to work.
touch "$TEST_DIR/this_file_ignores_umask"
chmod -i 777 "$TEST_DIR/this_file_ignores_umask"
actual=$(stat_perms "$TEST_DIR/this_file_ignores_umask")
expect="-rwxrwxrwx"
[[ "$actual" = "$expect" ]] || log_error "chmod -i fails to ignore umask" "$expect" "$actual"

# ==========
#  -n, --show      Show actions but do not change any file modes.
expect=$(stat_perms "$TEST_DIR/foo/bar")
chmod -n 000 "$TEST_DIR/foo/bar"
actual=$(stat_perms "$TEST_DIR/foo/bar")
[[ "$actual" = "$expect" ]] || log_error "chmod -n should not change permissions" "$expect" "$actual"

# ==========
#  -F, --reference=file
#                  Omit the mode operand and use the mode of file instead.
touch "$TEST_DIR/foo/baz"
expect=$(stat_perms "$TEST_DIR/foo/bar")
chmod -F "$TEST_DIR/foo/bar" "$TEST_DIR/foo/baz"
actual=$(stat_perms "$TEST_DIR/foo/baz")
[[ "$actual" = "$expect" ]] || log_error "chmod -F should use permission bits from reference file" "$expect" "$actual"

# ==========
#  -v, --verbose   Describe changed permissions of all files.
touch "$TEST_DIR/check_verbose_mode"
actual=$(chmod -v 000 "$TEST_DIR/check_verbose_mode")
expect=$'check_verbose_mode: mode changed to 0000 (---------)'
[[ "$actual" =~ "$expect" ]] || log_error "chmod -v does not give verbose output" "$expect" "$actual"


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
