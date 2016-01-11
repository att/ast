# regression tests for the asa utilitiy

TEST 01 'basics'
	EXEC
		INPUT - $'1This is a test
 This is line2
 This is line3
+This is line4
0This is line 5
1NewPage
 Last line'
		OUTPUT - $'This is a test
This is line2
This is line3\rThis is line4

This is line 5
\fNewPage
Last line'
	EXEC -r15
		INPUT - $'1This is a test This is line2  This is line3 +This is line4 0This is line 51NewPage        Last line     '
