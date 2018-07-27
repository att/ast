# These are the tests for the internal field separator (IFS).

IFS=e
set : :
[[ "$*" == ":e:" ]] || log_error "IFS failed" ":e:" "$*"

IFS='|' read -r first second third <<< 'one|two|three'
[[ "${first}" == "one" ]] || log_error "IFS failed" "one" "${first}"
[[ "${second}" == "two" ]] || log_error "IFS failed" "two" "${second}"
[[ "${third}" == "three" ]] || log_error "IFS failed" "three" "${third}"

# Multi-byte (wide) character checks will only work if UTF-8 inputs are enabled. We can't just set
# LC_ALL here because the literal UTF-8 strings will have already been read.
if [[ $LC_ALL == en_US.UTF-8 ]]
    then
    # 2 byte latin accented e character
    IFS=é
    set : :
    [[ "$*" == ":é:" ]] || log_error "IFS failed with multibyte character" ":é:" "$*"

    # 4 byte roman sestertius character
    IFS=𐆘 read -r first second third <<< 'one𐆘two𐆘three'
    [[ "${first}" == "one" ]] || log_error "IFS failed" "one" "${first}"
    [[ "${second}" == "two" ]] || log_error "IFS failed" "two" "${second}"
    [[ "${third}" == "three" ]] || log_error "IFS failed" "three" "${third}"

    # Ensure subshells don't get corrupted when IFS becomes multibyte character
    expect=$(printf ":é:\\ntrap -- 'echo end' EXIT\\nend")
    actual=$(IFS=é; set : :; echo "$*"; trap "echo end" EXIT; trap)
    [[ "$expect" == "$actual" ]] || log_error "IFS subshell failed" "$expect" "$actual"
fi
