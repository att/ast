# Tests for `cat` builtin
#
cat > "$TEST_DIR/sample_file" <<EOF
foo
bar


baz
EOF

print \\033x > "$TEST_DIR/file_with_control_character"

# ==========
# -b, --number-nonblank
# Number lines as with -n but omit line numbers from blank
# lines.
actual=$(cat -b $TEST_DIR/sample_file)
expect=$'     1\tfoo\n     2\tbar\n\n\n     3\tbaz'
[[ "$actual" = "$expect" ]] || log_error "cat -b failed" "$expect" "$actual"

# ==========
# TODO: Shall we continue to support this option ?
# -d, --dos-input
#  Input files are opened in textmode which removes carriage
#  returns in front of new-lines on some systems.
# actual=$(cat -d $TEST_DIR/sample_file)
# expect=""
# [[ "$actual" = "$expect" ]] || log_error "cat -vE failed" "$expect" "$actual"

# ==========
# -e Equivalent to -vE.
actual=$(cat -e $TEST_DIR/sample_file)
expect=$(cat -vE $TEST_DIR/sample_file)
[[ "$actual" = "$expect" ]] || log_error "cat -vE failed" "$expect" "$actual"

# ==========
# -n, --number
#  Causes a line number to be inserted at the beginning of each
#  line.
actual=$(cat -n $TEST_DIR/sample_file)
expect=$'     1\tfoo\n     2\tbar\n     3\t\n     4\t\n     5\tbaz'
[[ "$actual" = "$expect" ]] || log_error "cat -n failed" "$expect" "$actual"

# ==========
# -s Equivalent to -S for att universe and -B otherwise.
actual=$(cat -s $TEST_DIR/sample_file)
expect=$(cat -B $TEST_DIR/sample_file)
[[ "$actual" = "$expect" ]] || log_error "cat -s failed" "$expect" "$actual"

# ==========
# -t Equivalent to -vT.
actual=$(cat -t $TEST_DIR/sample_file)
expect=$(cat -vT $TEST_DIR/sample_file)
[[ "$actual" = "$expect" ]] || log_error "cat -t failed" "$expect" "$actual"

# ==========
# -u, --unbuffer  The output is not delayed by buffering.
actual=$(cat -u $TEST_DIR/sample_file)
expect=$'foo\nbar\n\n\nbaz'
[[ "$actual" = "$expect" ]] || log_error "cat -u failed" "$expect" "$actual"

# ==========
# -v, --show-nonprinting|print-chars
#  Print characters as follows: space and printable characters
#  as themselves; control characters as ^ followed by a letter
#  of the alphabet; and characters with the high bit set as the
#  lower 7 bit character prefixed by M^ for 7 bit non-printable
#  characters and M- for all other characters. If the 7 bit
#  character encoding is not ASCII then the characters are
#  converted to ASCII to determine high bit set, and if set it
#  is cleared and converted back to the native encoding.
#  Multibyte characters in the current locale are treated as
#  printable characters.
actual=$(cat -v "$TEST_DIR/file_with_control_character")
expect="^[x"
[[ "$actual" = "$expect" ]] || log_error "cat -v failed" "$expect" "$actual"

# ==========
# -A, --show-all  Equivalent to -vET.
actual=$(cat -A $TEST_DIR/sample_file)
expect=$(cat -vET $TEST_DIR/sample_file)
[[ "$actual" = "$expect" ]] || log_error "cat -A failed" "$expect" "$actual"

# ==========
# -B, --squeeze-blank
#  Multiple adjacent new-line characters are replace by one
#  new-line.
actual=$(cat -B $TEST_DIR/sample_file)
expect=$'foo\nbar\n\nbaz'
[[ "$actual" = "$expect" ]] || log_error "cat -B failed" "$expect" "$actual"

# ==========
# TODO: Shall we continue to support it ?
# -D, --dos-output
#  Output files are opened in textmode which inserts carriage
#  returns in front of new-lines on some systems.
# actual=$(cat -D $TEST_DIR/sample_file)
# expect=$'foo\nbar\n\n\nbaz'
# [[ "$actual" = "$expect" ]] || log_error "cat -D failed" "$expect" "$actual"

# ==========
# -E, --show-ends Causes a $ to be inserted before each new-line.
actual=$(cat -E $TEST_DIR/sample_file)
expect=$'foo$\nbar$\n$\n$\nbaz$'
[[ "$actual" = "$expect" ]] || log_error "cat -E failed" "$expect" "$actual"

# ==========
# TODO: It seems it's related to converting control characters,
# but I can't find a good test case for it.
# -R, --regress   Regression test defaults: -v buffer size 4.
# actual=$(cat -R $TEST_DIR/sample_file)
# expect=$'foo\nbar\n\n\nbaz'
# [[ "$actual" = "$expect" ]] || log_error "cat -R failed" "$expect" "$actual"

# ==========
# -S, --silent    cat is silent about non-existent files.
actual=$(cat -S this_file_does_not_exist 2>&1)
expect=""
[[ "$actual" = "$expect" ]] || log_error "cat -S should give no error on non-existent files" "$expect" "$actual"

# ==========
# -T, --show-blank
#  Causes tabs to be copied as ^I and formfeeds as ^L.
# TODO: Despite of being documented it does not convert formfeeds to ^L.
# But this behavior is compatible with GNU cat. Is it a bug ?
print "a\tb" > "$TEST_DIR/file_with_tabs"
actual=$(cat -T "$TEST_DIR/file_with_tabs")
expect="a^Ib"
[[ "$actual" = "$expect" ]] || log_error "cat -T failed to convert tabs to ^I."

# ==========
actual=$(cat this_file_does_not_exist 2>&1)
expect="this_file_does_not_exist: cannot open [No such file or directory]"
[[ "$actual" =~ "$expect" ]] || log_error "cat should give an error on non-existent files" "$expect" "$actual"
