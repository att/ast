# Tests for cut builtin

cat > "$TEST_DIR/foo" <<EOF
foo:bar:baz
bar:baz:foo
baz:foo:bar
foobarbaz
EOF

# ==========
# https://github.com/att/ast/issues/1157
#   -b, --bytes=list
#                   cut based on a list of byte counts.
actual=$(cut -b1 "$TEST_DIR/foo")
expect=$'f\nb\nb\nf'
[[ "$actual" = "$expect" ]] || log_error "'cut -b' failed"

# ==========
#   -c, --characters=list
#                   cut based on a list of character counts.
actual=$(cut -c1 "$TEST_DIR/foo")
expect=$'f\nb\nb\nf'
[[ "$actual" = "$expect" ]] || log_error "'cut -c' failed"

actual=$(cut -c1,3 "$TEST_DIR/foo")
expect=$'fo\nbr\nbz\nfo'
[[ "$actual" = "$expect" ]] || log_error "'cut -c1,3' failed"

# ==========
#   -d, --delimiter=delim
#                   The field character for the -f option is set to delim. The
#                   default is the tab character.
actual=$(cut -d: -f1 "$TEST_DIR/foo")
expect=$'foo\nbar\nbaz\nfoobarbaz'
[[ "$actual" = "$expect" ]] || log_error "'cut -d' failed" "$expect" "$actual"

# ==========
#   -f, --fields=list
#                   cut based on fields separated by the delimiter character
#                   specified with the -d optiion.
# This test also tests for default delimeter i.e. tab character
actual=$(cat "$TEST_DIR/foo" | tr ':' '\t' | cut -f1 )
expect=$'foo\nbar\nbaz\nfoobarbaz'
[[ "$actual" = "$expect" ]] || log_error "'cut -f' failed" "$expect" "$actual"

# ==========
#   -n, --split     Split multibyte characters selected by the -b option. On by
#                   default; -n means --nosplit.
actual=$(cut -b1 -n "$TEST_DIR/foo")
expect=$'f\nb\nb\nf'
[[ "$actual" = "$expect" ]] || log_error "'cut -n' failed"

# ==========
#   -R|r, --reclen=reclen
#                   If reclen > 0, the input will be read as fixed length records
#                   of length reclen when used with the -b or -c option.
actual=$(cut -b1 -r4 "$TEST_DIR/foo")
expect=$'f\nb\nb\nb\nb\nf\nb\nf\nb\nf\na'
[[ "$actual" = "$expect" ]] || log_error "'cut -r' failed"

# ==========
#   -s, --suppress|only-delimited
#                   Suppress lines with no delimiter characters, when used with
#                   the -f option. By default, lines with no delimiters will be
#                   passsed in untouched.
actual=$(cut -d: -f1 -s "$TEST_DIR/foo")
expect=$'foo\nbar\nbaz'
[[ "$actual" = "$expect" ]] || log_error "'cut -s' failed" "$expect" "$actual"

# ==========
#   -D, --line-delimeter|output-delimiter=ldelim
#                   The line delimiter character for the -f option is set to
#                   ldelim. The default is the newline character.
actual=$(cut -D: -f1 "$TEST_DIR/foo")
expect=$'foo:bar:baz\nbar:baz:foo\nbaz:foo:bar\nfoobarbaz'
[[ "$actual" = "$expect" ]] || log_error "'cut -D' failed" "$expect" "$actual"

# ==========
#   -N, --newline   Output new-lines at end of each record when used with the -b
#                   or -c option. On by default; -N means --nonewline.
#
actual=$(cut -N -d: -f1 "$TEST_DIR/foo")
expect=$'foo\nbar\nbaz\nfoobarbaz'
[[ "$actual" = "$expect" ]] || log_error "'cut -d' failed" "$expect" "$actual"

# ==========
actual=$(cut -c1 -f1 "$TEST_DIR/foo" 2>&1)
expect=$'cut: c option already specified'
[[ "$actual" =~ "$expect" ]] || log_error "'cut -b1 f1' should show an error" "$expect" "$actual"

# ==========
actual=$(cut -f1 -c1 "$TEST_DIR/foo" 2>&1)
expect=$'cut: f option already specified'
[[ "$actual" =~ "$expect" ]] || log_error "'cut -f1 c1' should show an error" "$expect" "$actual"

# ==========
actual=$(cut "$TEST_DIR/foo" 2>&1)
expect=$'cut: b, c or f option must be specified'
[[ "$actual" =~ "$expect" ]] || log_error "'cut' without b, c or f options should show an error" "$expect" "$actual"

# ==========
actual=$(cut -c -xyz "$TEST_DIR/foo" 2>&1)
expect=$'cut: bad list for c/f option'
[[ "$actual" =~ "$expect" ]] || log_error "'cut -b1 f1' should show an error" "$expect" "$actual"

# TODO: Add tests for multibyte characters
