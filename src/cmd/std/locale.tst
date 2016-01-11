# ast locale utility tests

TEST 01 basics
	EXEC TEST=C
		OUTPUT - $'C'
	EXEC TEST=POSIX
	EXEC TEST=debug
		OUTPUT - $'debug'

TEST 02 maps
	EXEC TEST=usa
		OUTPUT - $'en_US'

TEST 03 styles
	EXEC TEST=Serbian
		OUTPUT - $'sr'
	EXEC TEST='Serbian (Latin)'
	EXEC TEST='Serbian (Cyrillic)'
		OUTPUT - $'sr.ISO8859-5'
	EXEC TEST=sr_SP.cyrillic
		OUTPUT - $'sr_SP.ISO8859-5'
	EXEC TEST=sr_SP.ISO8859-5
	EXEC TEST=sr_SP.ISO-8859-5
	EXEC TEST=sr_SP.iso8859-5
	EXEC TEST=sr_SP.iso-8859-5
	EXEC TEST=sr_SP.1251
	EXEC TEST=sr_SP.win-1251
	EXEC TEST=sr_SP.windows-1251
	EXEC TEST=sr_SP.MS-1251
	EXEC TEST='sr_SP.m$-1251'
	EXEC TEST='Spanish (Modern Sort)_Spain.1252'
		OUTPUT - $'es_ES@modern'
	EXEC TEST='Spanish - Traditional Sort_Spain.1252'
		OUTPUT - $'es_ES'

TEST 04 primaries
	EXEC TEST=chinese
		OUTPUT - $'zh_CN'
	EXEC TEST=china
	EXEC TEST=english
		OUTPUT - $'en_GB'
	EXEC TEST=united-kingdom
	EXEC TEST=england
	EXEC TEST=french
		OUTPUT - $'fr_FR'
	EXEC TEST=france
	EXEC TEST=spanish
		OUTPUT - $'es_ES'
	EXEC TEST=spain
