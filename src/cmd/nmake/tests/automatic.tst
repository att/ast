# ast nmake automatic variable tests

INCLUDE test.def

TEST 01 'basics'

	EXEC	-n
		INPUT Makefile $'.SOURCE: src inc1 inc2
DEBUG == 1
VALUE == 2
all : some
some : d.c main.c a.c b.c c.c (VALUE)
	: "<" : $(<)
	: ">" : $(>)
	: "*" : $(*)
	: "~" : $(~)
	: "@" : $(@:V:@/\\n/ /G)
	: "%" : $(%)
	: "!" : $(!:H)
	: "&" : $(&:H)
	: "?" : $(?:H)
	: "^" : $(^)'
		INPUT inc1/a.h $'int debug = DEBUG;'
		INPUT inc2/b.h
		INPUT src/a.c $'#include "a.h"'
		INPUT src/b.c
		INPUT src/c.c
		INPUT src/d.c
		INPUT src/main.c
		INPUT src/some
		OUTPUT - $'+ : "<" : some
+ : ">" : src/d.c src/main.c src/a.c src/b.c src/c.c
+ : "*" : src/d.c src/main.c src/a.c src/b.c src/c.c
+ : "~" : src/d.c src/main.c src/a.c src/b.c src/c.c (VALUE)
+ : "@" : : "<" : $(<) : ">" : $(>) : "*" : $(*) : "~" : $(~) : "@" : $(@:V:@/\\n/ /G) : "%" : $(%) : "!" : $(!:H) : "&" : $(&:H) : "?" : $(?:H) : "^" : $(^)
+ : "%" : some
+ : "!" : inc1/a.h src/a.c src/b.c src/c.c src/d.c src/main.c
+ : "&" : (DEBUG) (VALUE)
+ : "?" : (DEBUG) (VALUE) inc1/a.h src/a.c src/b.c src/c.c src/d.c src/main.c
+ : "^" : '

TEST 02 'explicit chain'

	EXEC	-n
		INPUT gen.mk $'":gen:" : .MAKE .OPERATOR
	$(<) : $(>)
		: $(<) : $(*) : $(<:T=M)
		: $(<<) : $(**) : $(<:T=M)
		: $(<<<) : $(***) : $(<:T=M)'
		INPUT Makefile $'a :gen: b
b :gen: c
c :gen: d
d :gen: e
e :gen: f'
		ERROR - $'make: don\'t know how to make a : b : c : d : e : f'
		EXIT 1

	EXEC -n
		INPUT f
		OUTPUT - $'+ : e : f : a b c d e
+ : d : e : a b c d e
+ : c : d : a b c d e
+ : d : e : a b c d
+ : c : d : a b c d
+ : b : c : a b c d
+ : c : d : a b c
+ : b : c : a b c
+ : a : b : a b c
+ : b : c : a b
+ : a : b : a b
+ : .INTERNAL :  : a b
+ : a : b : a
+ : .INTERNAL :  : a
+ :  :  : a'
		ERROR -
		EXIT 0

TEST 03 'command line or makeargs assignments and assertions'

	EXEC	-n 'FOO == 1' 'BAR== 2' 'foo : bar'
		INPUT Makefile $'all :
	args $(=)'
		OUTPUT - $'+ args \'FOO == 1\' BAR==\' 2\' \'foo : bar\''

	EXEC	-n
		INPUT Makeargs $'FOO == 1
BAR== 2
foo : bar'
