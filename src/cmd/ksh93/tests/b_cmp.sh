# Tests for `cmp` builtin

cat  > "$TEST_DIR/file1" <<EOF
This is file 1
EOF

cat  > "$TEST_DIR/file2" <<EOF
This is file 2
EOF

cat  > "$TEST_DIR/file23" <<EOF
This is file 23
EOF

# EXIT STATUS
#     0     The files or portions compared are identical.
cmp "$TEST_DIR/file1" "$TEST_DIR/file1"
actual=$?
expect=0
[[ "$actual" -eq "$expect" ]] || log_error "cmp should return 0 for identical files" "$expect" "$actual"

#     1     The files are different.
cmp "$TEST_DIR/file1" "$TEST_DIR/file2" > /dev/null
actual=$?
expect=1
[[ "$actual" -eq "$expect" ]] || log_error "cmp should return 1 for not identical files" "$expect" "$actual"

#     >1    An error occurred.
cmp "$TEST_DIR/file1" "$TEST_DIR/this_file_does_not_exist" 2> /dev/null
actual=$?
expect=2
[[ "$actual" -eq "$expect" ]] || log_error "cmp should return 2 on error" "$expect" "$actual"

# ==========
#  -b, --print-bytes
#                  Print differing bytes as 3 digit octal values.

actual=$(cmp -b "$TEST_DIR/file1" "$TEST_DIR/file2")
expect="$TEST_DIR/file1 $TEST_DIR/file2 differ: char 14, line 1, 061 062"
[[ "$actual" = "$expect" ]] || log_error "'cmp -b' failed" "$expect" "$actual"

# ==========
#  -c, --print-chars
#                  Print differing bytes as follows: non-space printable
#                  characters as themselves; space and control characters as ^
#                  followed by a letter of the alphabet; and characters with the
#                  high bit set as the lower 7 bit character prefixed by M^ for
#                  7 bit space and non-printable characters and M- for all other
#                  characters. If the 7 bit character encoding is not ASCII then
#                  the characters are converted to ASCII to determine high bit
#                  set, and if set it is cleared and converted back to the
#                  native encoding. Multibyte characters in the current locale
#                  are treated as printable characters.
actual=$(cmp -c "$TEST_DIR/file1" "$TEST_DIR/file2")
expect="$TEST_DIR/file1 $TEST_DIR/file2 differ: char 14, line 1,   1   2"
[[ "$actual" = "$expect" ]] || log_error "'cmp -c' failed" "$expect" "$actual"

# ==========
#  -d, --differences=differences
#                  Print at most differences differences using --verbose output
#                  format. --differences=0 is equivalent to --silent.
actual=$(cmp -d 1 "$TEST_DIR/file1" "$TEST_DIR/file23")
expect="    14  061 062"
[[ "$actual" = "$expect" ]] || log_error "'cmp -d' failed" "$expect" "$actual"

# ==========
#  -i, --ignore-initial|skip=skip1[:skip2]
#                  Skip the the first skip1 bytes in file1 and the first skip2
#                  bytes in file2. If skip2 is omitted then skip1 is used. The
#                  default value is 0:0.
actual=$(cmp -c -i 8:8 "$TEST_DIR/file1" "$TEST_DIR/file2")
expect="$TEST_DIR/file1 $TEST_DIR/file2 differ: char 6, line 1,   1   2"
[[ "$actual" = "$expect" ]] || log_error "'cmp -i 8' should ignore first 8 bytes" "$expect" "$actual"

# ==========
#  -l, --verbose   Write the decimal byte number and the differing bytes (in
#                  octal) for each difference.
actual=$(cmp -l "$TEST_DIR/file1" "$TEST_DIR/file2")
expect="    14  061 062"
[[ "$actual" = "$expect" ]] || log_error "'cmp -l' failed" "$expect" "$actual"

# ==========
#  -n, --count|bytes=count
#                  Compare at most count bytes.
cmp -n 13 "$TEST_DIR/file1" "$TEST_DIR/file2"
actual=$?
expect=0
[[ "$actual" = "$expect" ]] || log_error "'cmp -n 13' should compare at most 13 bytes" "$expect" "$actual"

# ==========
#  -s, --quiet|silent
#                  Write nothing for differing files; return non-zero exit
#                  status only.
#
actual=$(cmp -s "$TEST_DIR/file1" "$TEST_DIR/file2")
expect=""
[[ "$actual" -eq "$expect" ]] || log_error "'cmp -s' should give empty output" "$expect" "$actual"
