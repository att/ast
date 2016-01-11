CC = cc

TEST_fnmatch = testfnmatch.dat
TEST_glob = testglob.dat
TEST_regex = locale.dat rxposix.dat standards.dat testmatch.dat testregex.dat

test : test.fnmatch test.glob test.regex

test.fnmatch : testfnmatch
	for i in $(TEST_fnmatch); do echo === $$i ===; ./testfnmatch -c < $$i; done

test.glob : testglob
	for i in $(TEST_glob); do echo === $$i ===; ./testglob -c < $$i; done

test.regex : testregex
	for i in $(TEST_regex); do echo === $$i ===; ./testregex -c < $$i; done

all : testfnmatch testglob testregex

testfnmatch : testfnmatch.o
	$(CC) -o testfnmatch testfnmatch.o

testglob : testglob.o
	$(CC) -o testglob testglob.o

testregex : testregex.o
	$(CC) -o testregex testregex.o
