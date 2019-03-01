# Tests for `wc` builtin
#   wc - print the number of bytes, words, and lines in files

cat > "$TEST_DIR/file1" <<EOF
This is line 1 in file1
This is line 2 in file1
This is line 3 in file1
This is line 4 in file1
This is line 5 in file1
EOF

cat > "$TEST_DIR/file2" <<EOF
This is line 1 in file2
This is line 2 in file2
This is line 3 in file2
This is line 4 in file2
This is line 5 in file2
This is the longest line in file2
神
EOF

# ==========
#   -l, --lines     List the line counts.
actual=$(wc -l "$TEST_DIR/file1")
expect=$"       5 $TEST_DIR/file1"
[[ "$actual" = "$expect" ]] || log_error "'wc -l' failed" "$expect" "$actual"

# ==========
#   -w, --words     List the word counts.
actual=$(wc -w "$TEST_DIR/file1")
expect=$"      30 $TEST_DIR/file1"
[[ "$actual" = "$expect" ]] || log_error "'wc -w' failed" "$expect" "$actual"


# ==========
#   -c, --bytes|chars
#                   List the byte counts.
actual=$(wc -c "$TEST_DIR/file1")
expect=$"     120 $TEST_DIR/file1"
[[ "$actual" = "$expect" ]] || log_error "'wc -c' failed" "$expect" "$actual"

# ==========
#   -m|C, --multibyte-chars
#                   List the character counts.
actual=$(wc -m "$TEST_DIR/file2")
expect=$"     156 $TEST_DIR/file2"
[[ "$actual" = "$expect" ]] || log_error "'wc -m' failed" "$expect" "$actual"


# ==========
#   -q, --quiet     Suppress invalid multibyte character warnings.
actual=$(wc -q -m "$TEST_DIR/file2")
expect=$"     156 $TEST_DIR/file2"
[[ "$actual" = "$expect" ]] || log_error "'wc -q -m' failed" "$expect" "$actual"

# ==========
#   -L, --longest-line|max-line-length
#                   List the longest line length; the newline,if any, is not
#                   counted in the length.
actual=$(wc -L "$TEST_DIR/file2")
expect=$"      33 $TEST_DIR/file2"
[[ "$actual" = "$expect" ]] || log_error "'wc -l' failed" "$expect" "$actual"

# ==========
#   -N, --utf8      For UTF-8 locales --noutf8 disables UTF-8 optimzations and
#                   relies on the native mbtowc(3). On by default; -N means
#                   --noutf8.
actual=$(wc -N "$TEST_DIR/file2")
expect="       7      38     158 $TEST_DIR/file2"
[[ "$actual" = "$expect" ]] || log_error "'wc -N' failed" "$expect" "$actual"
