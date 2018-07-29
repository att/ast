# These are the tests for the internal field separator (IFS).

IFS=e
set : :
[[ "$*" == ":e:" ]] || log_error "IFS failed" ":e:" "$*"

IFS='|' read -r first second third <<< 'one|two|three'
[[ "${first}" == "one" ]] || log_error "IFS failed" "one" "${first}"
[[ "${second}" == "two" ]] || log_error "IFS failed" "two" "${second}"
[[ "${third}" == "three" ]] || log_error "IFS failed" "three" "${third}"

# Multi-byte character checks will only work if UTF-8 inputs are enabled
if [ "${LC_ALL}" = "en_US.UTF-8" ]
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
    expected_output=$(printf ":é:\\ntrap -- 'echo end' EXIT\\nend")
    output=$(LANG=C.UTF-8; IFS=é; set : :; echo "$*"; trap "echo end" EXIT; trap)
    [[ "${output}" == "${expected_output}" ]] || log_error "IFS subshell failed" "${expected_output}" "${output}"
fi
