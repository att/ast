# Tests for cut builtin

cat > "$TEST_DIR/foo" <<EOF
foo:bar:baz
bar:baz:foo
baz:foo:bar
foobarbaz
EOF

# ==========
# TODO
# https://github.com/att/ast/issues/1157
#   -b, --bytes=list
#                   cut based on a list of byte counts.
# actual=$(cut -b1 "$TEST_DIR/foo")
# expect=$'f\nb\nb\nf'
# [[ "$actual" = "$expect" ]] || log_error "'cut -b' failed"

# ==========
# TODO
#   -c, --characters=list
#                   cut based on a list of character counts.
# actual=$(cut -c1 "$TEST_DIR/foo")
# expect=$'f\nb\nb\nf'
# [[ "$actual" = "$expect" ]] || log_error "'cut -c' failed"

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
# TODO
# https://github.com/att/ast/issues/1157
#   -n, --split     Split multibyte characters selected by the -b option. On by
#                   default; -n means --nosplit.

# ==========
# TODO
#   -R|r, --reclen=reclen
#                   If reclen > 0, the input will be read as fixed length records
#                   of length reclen when used with the -b or -c option.

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
