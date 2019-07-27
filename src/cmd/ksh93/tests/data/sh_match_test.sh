# input text
xmltext="$( < "$1" )"

print -f "%d characters to process...\n" "${#xmltext}"

#
# parse the XML data
#
typeset dummy
function parse_xmltext
{
    typeset xmltext="$2"
    nameref ar="$1"

    # fixme:
    # - We want to enforce standard conformance - does ~(Exp) or ~(Ex-p) does that ?
    dummy="${xmltext//~(Ex-p)(?:
        (<!--.*-->)+?|			# xml comments
        (<[:_[:alnum:]-]+
            (?: # attributes
                [[:space:]]+
                (?: # four different types of name=value syntax
                    (?:[:_[:alnum:]-]+=[^\"\'[:space:]]+?)|	#x='foo=bar huz=123'
                    (?:[:_[:alnum:]-]+=\"[^\"]*?\")|		#x='foo="ba=r o" huz=123'
                    (?:[:_[:alnum:]-]+=\'[^\']*?\')|		#x="foox huz=123"
                    (?:[:_[:alnum:]-]+)				#x="foox huz=123"
                )
            )*
            [[:space:]]*
            \/?	# start tags which are end tags, too (like <foo\/>)
        >)+?|				# xml start tags
        (<\/[:_[:alnum:]-]+>)+?|	# xml end tags
        ([^<]+)				# xml text
        )/D}"

    # copy ".sh.match" to array "ar"
    integer i j
    for i in "${!.sh.match[@]}" ; do
        for j in "${!.sh.match[i][@]}" ; do
            [[ -v .sh.match[i][j] ]] && ar[i][j]="${.sh.match[i][j]}"
        done
    done

    return 0
}

function rebuild_xml_and_verify
{
    nameref ar="$1"
    typeset xtext="$2" # xml text

    #
    # rebuild the original text from "ar" (copy of ".sh.match")
    # and compare it to the content of "xtext"
    #
    {
        # rebuild the original text, based on our matches
        nameref nodes_all=ar[0]		# contains all matches
        nameref nodes_comments=ar[1]	# contains only XML comment matches
        nameref nodes_start_tags=ar[2]	# contains only XML start tag matches
        nameref nodes_end_tags=ar[3]	# contains only XML end tag matches
        nameref nodes_text=ar[4]	# contains only XML text matches
        integer i
        for (( i = 0 ; i < ${#nodes_all[@]} ; i++ )) ; do
            [[ -v nodes_comments[i]		]] && printf '%s' "${nodes_comments[i]}"
            [[ -v nodes_start_tags[i]	]] && printf '%s' "${nodes_start_tags[i]}"
            [[ -v nodes_end_tags[i]		]] && printf '%s' "${nodes_end_tags[i]}"
            [[ -v nodes_text[i]		]] && printf '%s' "${nodes_text[i]}"
        done
        printf '\n'
    } > tmp_file

    diff -u <( printf '%s\n' "${xtext}")  tmp_file
    if cmp <( printf '%s\n' "${xtext}")  tmp_file ; then
        printf "#input and output OK (%d characters).\n" "$(wc -m < tmp_file)"
    else
        printf "#difference between input and output found.\n"
    fi

    return 0
}

# main
set -o nounset

typeset -a xar
parse_xmltext xar "$xmltext"
rebuild_xml_and_verify xar "$xmltext"
