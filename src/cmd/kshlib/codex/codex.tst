# : : generated from codex.rt by mktest : : #

# regression tests for the ksh codex plugin

UNIT ksh

TEST 01 'aes + uu-mime'

	EXEC	-c $'builtin -f codex codex
\t\tp="open sesame"
\t\tq="close sesame"
\t\tfor ((i=1; i<=10; i++))
\t\tdo
\t\t\te=$(codex -p "$p" ">aes>uu-mime" <<<"hello newman")
\t\t\tcodex -p "$p" "<uu-mime<aes" <<<"$e"
\t\t\te=$(codex -p "$q" ">aes>uu-mime" <<<"see ya")
\t\t\tcodex -p "$q" "<uu-mime<aes" <<<"$e"
\t\tdone'
		INPUT -n -
		OUTPUT - $'hello newman
see ya
hello newman
see ya
hello newman
see ya
hello newman
see ya
hello newman
see ya
hello newman
see ya
hello newman
see ya
hello newman
see ya
hello newman
see ya
hello newman
see ya'
		ERROR -n -

	EXEC	-c $'builtin -f codex
\t\tp="open sesame"
\t\tq="close sesame"
\t\tfor ((i=1; i<=10; i++))
\t\tdo
\t\t\te=$(codex -p "$p" ">aes>uu-mime" <<<"hello newman")
\t\t\tcodex -p "$p" "<uu-mime<aes" <<<"$e"
\t\t\te=$(codex -p "$q" ">aes>uu-mime" <<<"see ya")
\t\t\tcodex -p "$q" "<uu-mime<aes" <<<"$e"
\t\tdone'
