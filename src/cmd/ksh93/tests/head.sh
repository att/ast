# Tests for `head` builtin

cat > "$TEST_DIR/file1" <<EOF
This is line 1 in file1
This is line 2 in file1
This is line 3 in file1
This is line 4 in file1
This is line 5 in file1
EOF

cat > "$TEST_DIR/file2" <<EOF2
This is line 1 in file2
This is line 2 in file2
This is line 3 in file2
This is line 4 in file2
This is line 5 in file2
EOF2

# ==========
#   -n, --lines=lines
#                   Copy lines lines from each file. The default value is 10.
actual=$(head -n 3 "$TEST_DIR/file1")
expect=$'This is line 1 in file1\nThis is line 2 in file1\nThis is line 3 in file1'
[[ "$actual" = "$expect" ]] || log_error "'head -n' failed"

# ==========
#   -c, --bytes=chars
#                   Copy chars bytes from each file.
actual=$(head -c 14 "$TEST_DIR/file1")
expect=$'This is line 1'
[[ "$actual" = "$expect" ]] || log_error "'head -c' failed"

# ==========
#   -q, --quiet|silent
#                   Never ouput filename headers.
actual=$(head -q -n 3 "$TEST_DIR/file1" "$TEST_DIR/file2")
expect=$'This is line 1 in file1\nThis is line 2 in file1\nThis is line 3 in file1\nThis is line 1 in file2\nThis is line 2 in file2\nThis is line 3 in file2'
[[ "$actual" = "$expect" ]] || log_error "'head -q' failed"

# ==========
#   -s, --skip=skip Skip skip characters or lines from each file before copying.
actual=$(head -s 5 -c 18 "$TEST_DIR/file1")
expect=$'is line 1 in file1'
[[ "$actual" = "$expect" ]] || log_error "'head -c' failed"

# ==========
#   -v, --verbose   Always ouput filename headers.
actual=$(head -v -n 3 "$TEST_DIR/file1")
expect=$'file1 <==\nThis is line 1 in file1\nThis is line 2 in file1\nThis is line 3 in file1'
[[ "$actual" =~ "$expect" ]] || log_error "'head -v' failed"
