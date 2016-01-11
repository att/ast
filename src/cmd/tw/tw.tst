# tests for the tw utility

function DATA
{
	typeset f i j k
	for f
	do	test -d $f && continue
		KEEP $f
		case $f in
		data)	mkdir data
			for i in aaa zzz
			do	i=data/$i
				mkdir $i
				for j in 111 222 333
				do	mkdir $i/$j
					for k in 4 5 6 7
					do	mkdir $i/$j/$k
						for l in q.c r.d s.z
						do	print $i $j $k $l > $i/$j/$k/$l
							chmod $k$k$k $i/$j/$k/$l
						done
					done
				done
			done
			;;
		link)	mkdir -p link/x/y/z
			ln -s x/y link/home
			;;
		match)	mkdir -p match/.ghi match/jkl
			: > match/.abc > match/def
			: > match/.ghi/.mno > match/.ghi/pqr
			: > match/jkl/.stu > match/jkl/vwx
			;;
		mode)	mkdir mode
			for i in 0 1 2 3 4 5 6 7
			do	: > mode/$i$i$i
				chmod $i$i$i mode/$i$i$i
			done
			;;
		one)	mkdir one
			> one/one
			;;
		esac
	done
}

TEST 01 'basics'
	DO	DATA data
	EXEC	-d data -e sort:name
		OUTPUT - $'data
data/aaa
data/aaa/111
data/aaa/111/4
data/aaa/111/4/q.c
data/aaa/111/4/r.d
data/aaa/111/4/s.z
data/aaa/111/5
data/aaa/111/5/q.c
data/aaa/111/5/r.d
data/aaa/111/5/s.z
data/aaa/111/6
data/aaa/111/6/q.c
data/aaa/111/6/r.d
data/aaa/111/6/s.z
data/aaa/111/7
data/aaa/111/7/q.c
data/aaa/111/7/r.d
data/aaa/111/7/s.z
data/aaa/222
data/aaa/222/4
data/aaa/222/4/q.c
data/aaa/222/4/r.d
data/aaa/222/4/s.z
data/aaa/222/5
data/aaa/222/5/q.c
data/aaa/222/5/r.d
data/aaa/222/5/s.z
data/aaa/222/6
data/aaa/222/6/q.c
data/aaa/222/6/r.d
data/aaa/222/6/s.z
data/aaa/222/7
data/aaa/222/7/q.c
data/aaa/222/7/r.d
data/aaa/222/7/s.z
data/aaa/333
data/aaa/333/4
data/aaa/333/4/q.c
data/aaa/333/4/r.d
data/aaa/333/4/s.z
data/aaa/333/5
data/aaa/333/5/q.c
data/aaa/333/5/r.d
data/aaa/333/5/s.z
data/aaa/333/6
data/aaa/333/6/q.c
data/aaa/333/6/r.d
data/aaa/333/6/s.z
data/aaa/333/7
data/aaa/333/7/q.c
data/aaa/333/7/r.d
data/aaa/333/7/s.z
data/zzz
data/zzz/111
data/zzz/111/4
data/zzz/111/4/q.c
data/zzz/111/4/r.d
data/zzz/111/4/s.z
data/zzz/111/5
data/zzz/111/5/q.c
data/zzz/111/5/r.d
data/zzz/111/5/s.z
data/zzz/111/6
data/zzz/111/6/q.c
data/zzz/111/6/r.d
data/zzz/111/6/s.z
data/zzz/111/7
data/zzz/111/7/q.c
data/zzz/111/7/r.d
data/zzz/111/7/s.z
data/zzz/222
data/zzz/222/4
data/zzz/222/4/q.c
data/zzz/222/4/r.d
data/zzz/222/4/s.z
data/zzz/222/5
data/zzz/222/5/q.c
data/zzz/222/5/r.d
data/zzz/222/5/s.z
data/zzz/222/6
data/zzz/222/6/q.c
data/zzz/222/6/r.d
data/zzz/222/6/s.z
data/zzz/222/7
data/zzz/222/7/q.c
data/zzz/222/7/r.d
data/zzz/222/7/s.z
data/zzz/333
data/zzz/333/4
data/zzz/333/4/q.c
data/zzz/333/4/r.d
data/zzz/333/4/s.z
data/zzz/333/5
data/zzz/333/5/q.c
data/zzz/333/5/r.d
data/zzz/333/5/s.z
data/zzz/333/6
data/zzz/333/6/q.c
data/zzz/333/6/r.d
data/zzz/333/6/s.z
data/zzz/333/7
data/zzz/333/7/q.c
data/zzz/333/7/r.d
data/zzz/333/7/s.z'

TEST 02 'patterns'
	DO	DATA data
	EXEC	-d data -e sort:name -e 'name=="*.c"'
		OUTPUT - $'data/aaa/111/4/q.c
data/aaa/111/5/q.c
data/aaa/111/6/q.c
data/aaa/111/7/q.c
data/aaa/222/4/q.c
data/aaa/222/5/q.c
data/aaa/222/6/q.c
data/aaa/222/7/q.c
data/aaa/333/4/q.c
data/aaa/333/5/q.c
data/aaa/333/6/q.c
data/aaa/333/7/q.c
data/zzz/111/4/q.c
data/zzz/111/5/q.c
data/zzz/111/6/q.c
data/zzz/111/7/q.c
data/zzz/222/4/q.c
data/zzz/222/5/q.c
data/zzz/222/6/q.c
data/zzz/222/7/q.c
data/zzz/333/4/q.c
data/zzz/333/5/q.c
data/zzz/333/6/q.c
data/zzz/333/7/q.c'
	EXEC	-d data -e sort:name -e 'name=="*.[cd]"'
		OUTPUT - $'data/aaa/111/4/q.c
data/aaa/111/4/r.d
data/aaa/111/5/q.c
data/aaa/111/5/r.d
data/aaa/111/6/q.c
data/aaa/111/6/r.d
data/aaa/111/7/q.c
data/aaa/111/7/r.d
data/aaa/222/4/q.c
data/aaa/222/4/r.d
data/aaa/222/5/q.c
data/aaa/222/5/r.d
data/aaa/222/6/q.c
data/aaa/222/6/r.d
data/aaa/222/7/q.c
data/aaa/222/7/r.d
data/aaa/333/4/q.c
data/aaa/333/4/r.d
data/aaa/333/5/q.c
data/aaa/333/5/r.d
data/aaa/333/6/q.c
data/aaa/333/6/r.d
data/aaa/333/7/q.c
data/aaa/333/7/r.d
data/zzz/111/4/q.c
data/zzz/111/4/r.d
data/zzz/111/5/q.c
data/zzz/111/5/r.d
data/zzz/111/6/q.c
data/zzz/111/6/r.d
data/zzz/111/7/q.c
data/zzz/111/7/r.d
data/zzz/222/4/q.c
data/zzz/222/4/r.d
data/zzz/222/5/q.c
data/zzz/222/5/r.d
data/zzz/222/6/q.c
data/zzz/222/6/r.d
data/zzz/222/7/q.c
data/zzz/222/7/r.d
data/zzz/333/4/q.c
data/zzz/333/4/r.d
data/zzz/333/5/q.c
data/zzz/333/5/r.d
data/zzz/333/6/q.c
data/zzz/333/6/r.d
data/zzz/333/7/q.c
data/zzz/333/7/r.d'
	EXEC	-d data -e sort:name -e 'name=="*.c" || name=="*.d"'

TEST 03 'types'
	DO	DATA data
	EXEC	-d data -e sort:name -e type==DIR
		OUTPUT - $'data
data/aaa
data/aaa/111
data/aaa/111/4
data/aaa/111/5
data/aaa/111/6
data/aaa/111/7
data/aaa/222
data/aaa/222/4
data/aaa/222/5
data/aaa/222/6
data/aaa/222/7
data/aaa/333
data/aaa/333/4
data/aaa/333/5
data/aaa/333/6
data/aaa/333/7
data/zzz
data/zzz/111
data/zzz/111/4
data/zzz/111/5
data/zzz/111/6
data/zzz/111/7
data/zzz/222
data/zzz/222/4
data/zzz/222/5
data/zzz/222/6
data/zzz/222/7
data/zzz/333
data/zzz/333/4
data/zzz/333/5
data/zzz/333/6
data/zzz/333/7'
	EXEC	-d data -e sort:name -e type==REG
		OUTPUT - $'data/aaa/111/4/q.c
data/aaa/111/4/r.d
data/aaa/111/4/s.z
data/aaa/111/5/q.c
data/aaa/111/5/r.d
data/aaa/111/5/s.z
data/aaa/111/6/q.c
data/aaa/111/6/r.d
data/aaa/111/6/s.z
data/aaa/111/7/q.c
data/aaa/111/7/r.d
data/aaa/111/7/s.z
data/aaa/222/4/q.c
data/aaa/222/4/r.d
data/aaa/222/4/s.z
data/aaa/222/5/q.c
data/aaa/222/5/r.d
data/aaa/222/5/s.z
data/aaa/222/6/q.c
data/aaa/222/6/r.d
data/aaa/222/6/s.z
data/aaa/222/7/q.c
data/aaa/222/7/r.d
data/aaa/222/7/s.z
data/aaa/333/4/q.c
data/aaa/333/4/r.d
data/aaa/333/4/s.z
data/aaa/333/5/q.c
data/aaa/333/5/r.d
data/aaa/333/5/s.z
data/aaa/333/6/q.c
data/aaa/333/6/r.d
data/aaa/333/6/s.z
data/aaa/333/7/q.c
data/aaa/333/7/r.d
data/aaa/333/7/s.z
data/zzz/111/4/q.c
data/zzz/111/4/r.d
data/zzz/111/4/s.z
data/zzz/111/5/q.c
data/zzz/111/5/r.d
data/zzz/111/5/s.z
data/zzz/111/6/q.c
data/zzz/111/6/r.d
data/zzz/111/6/s.z
data/zzz/111/7/q.c
data/zzz/111/7/r.d
data/zzz/111/7/s.z
data/zzz/222/4/q.c
data/zzz/222/4/r.d
data/zzz/222/4/s.z
data/zzz/222/5/q.c
data/zzz/222/5/r.d
data/zzz/222/5/s.z
data/zzz/222/6/q.c
data/zzz/222/6/r.d
data/zzz/222/6/s.z
data/zzz/222/7/q.c
data/zzz/222/7/r.d
data/zzz/222/7/s.z
data/zzz/333/4/q.c
data/zzz/333/4/r.d
data/zzz/333/4/s.z
data/zzz/333/5/q.c
data/zzz/333/5/r.d
data/zzz/333/5/s.z
data/zzz/333/6/q.c
data/zzz/333/6/r.d
data/zzz/333/6/s.z
data/zzz/333/7/q.c
data/zzz/333/7/r.d
data/zzz/333/7/s.z'

TEST 04 'symlink hops'
	DO	DATA link
	EXEC -d link -e sort:name -e "if(name=='home')status=FOLLOW;"
		OUTPUT - $'link
link/home
link/home/z
link/x
link/x/y
link/x/y/z'

TEST 05 '-name pattern'
	DO	DATA match
	EXEC	-d match -e sort:name
		OUTPUT - $'match
match/.abc
match/.ghi
match/.ghi/.mno
match/.ghi/pqr
match/def
match/jkl
match/jkl/.stu
match/jkl/vwx'
	EXEC	-d match -e sort:name -e 'name=="*"'
	EXEC	-d match -e sort:name -e 'path=="*"'
	EXEC	-d match -e sort:name -e 'name==".*"'
		OUTPUT - $'match/.abc
match/.ghi
match/.ghi/.mno
match/jkl/.stu'
	EXEC	-d match -e sort:name -e 'name=="*.*"'

TEST 06 'expressions and functions'

	EXEC 	-n -e '	int i;
			char* s = "abc pdq xyz";
			char* a, p, x;
			i = sscanf(s, "%s %s %s", &a, &p, &x);
			printf("i=%d a=\"%s\" p=\"%s\" x=\"%s\"\n", i, a, p, x);'
		OUTPUT - 'i=3 a="abc" p="pdq" x="xyz"'

	EXEC 	-n -e '	int i;
			char* s = "abc pdq xyz";
			char* a, p, x;
			i = sscanf(s, "%s %s %s", &a, &p, &x);
			printf("i=%d x=\"%s\" p=\"%s\" a=\"%s\"\n", i, x, p, a);'
		OUTPUT - 'i=3 x="xyz" p="pdq" a="abc"'

	EXEC 	-n -e '	int i;
			char* s = "abc pdq xyz";
			char* a, p, x;
			i = sscanf(s, "%s", &a);
			printf("i=%d a=\"%s\"\n", i, a);
			i = sscanf(s, "%s %s", &a, &p);
			printf("i=%d a=\"%s\" p=\"%s\"\n", i, a, p);
			i = sscanf(s, "%s %s %s", &a, &p, &x);
			printf("i=%d a=\"%s\" p=\"%s\" x=\"%s\"\n", i, a, p, x);
			i = sscanf(s, "%s %s %s", &a, &p, &x);
			printf("i=%d x=\"%s\" p=\"%s\" a=\"%s\"\n", i, x, p, a);
			printf("i=%d a=\"%s\" p=\"%s\" x=\"%s\"\n", i, a, p, x);'
		OUTPUT - 'i=1 a="abc"
i=2 a="abc" p="pdq"
i=3 a="abc" p="pdq" x="xyz"
i=3 x="xyz" p="pdq" a="abc"
i=3 a="abc" p="pdq" x="xyz"'

	EXEC 	-n -e '	int i,n;
			float f;
			char* s = "123 3.45e6";
			i = sscanf(s,"%d %g", &n, &f);
			printf("i=%d n=%d f=%g\n", i, n, f);'
		OUTPUT - 'i=2 n=123 f=3.45e+06'

	EXEC 	-n -e '	int i,n;
			float f;
			i = scanf("%d %g", &n, &f);
			printf("i=%d n=%d f=%g\n", i, n, f);'
		INPUT - '123 3.45e6'

	EXEC 	-n -e '	int n,i,j,k;
			char* s = "1";
			n = sscanf(s,"%d %d %d", &i, &j, &k);
			printf("n=%d i=%d j=%d k=%d\n", n, i, j, k);'
		OUTPUT - $'n=1 i=1 j=0 k=0'

	EXEC 	-n -e '	int i,n;
			float f;
			char* s = "123 3.45e6";
			i = sscanf(s,"%d %g", n, &f);
			printf("i=%d n=%d f=%g\n", i, n, f);'
		EXIT 1
		OUTPUT -
		ERROR - 'tw: line 4: i = sscanf(s,"%d %g", n, &f)<<< sscanf: address argument expected'

	EXEC 	-n -e '	int i,n;
			float f;
			char* s = "123 3.45e6";
			i = sscanf(s,"%d %g", &n, f);
			printf("i=%d n=%d f=%g\n", i, n, f);'
		ERROR - 'tw: line 4: i = sscanf(s,"%d %g", &n, f)<<< sscanf: address argument expected'

	EXEC 	-n -e '	int i,n;
			float f;
			char* s = "123 3.45e6";
			i = sscanf(s);
			printf("i=%d n=%d f=%g\n", i, n, f);'
		ERROR - 'tw: line 4: i = sscanf(s)<<< sscanf: format argument expected'

	EXEC 	-n -e '	int i,n;
			float f;
			char* s = "123 3.45e6";
			i = sscanf(s,"%d %g");
			printf("i=%d n=%d f=%g\n", i, n, f);'
		ERROR - 'tw: scanf: not enough arguments'

	EXEC 	-n -e '	int i,n;
			float f;
			char* s = "123 3.45e6";
			i = sscanf(s,"%d %g", &n);
			printf("i=%d n=%d f=%g\n", i, n, f);'
		ERROR - 'tw: scanf: not enough arguments'

	EXEC 	-n -e '	int i,n;
			float f;
			char* s = "123 3.45e6";
			i = sscanf(s,"%d %g", &n, &f, &i);
			printf("i=%d n=%d f=%g\n", i, n, f);'
		ERROR - 'tw: scanf: i: too many arguments'

	EXEC 	-n -e '	int i,n;
			float f;
			char* s = "123 3.45e6";
			i = sscanf(s,"%d %g", &f, &f);
			printf("i=%d n=%d f=%g\n", i, n, f);'
		ERROR - 'tw: scanf: f: integer variable address argument expected'

	EXEC 	-n -e '	int i,n;
			float f;
			char* s = "123 3.45e6";
			i = sscanf(s,"%d %g", &n, &n);
			printf("i=%d n=%d f=%g\n", i, n, f);'
		ERROR - 'tw: scanf: n: floating variable address argument expected'

TEST 07 'function and variable scoping'

	EXEC 	-n \
		-e 'static int i = 1; int foo() { return i++; }' \
		-e 'static int i = 11; int bar() { return i++; }' \
		-e 'int end() { int i; for (i = 0; i < 8; i++) printf("%2d %2d\n", foo(), bar()); }'
		OUTPUT - $' 1 11\n 2 12\n 3 13\n 4 14\n 5 15\n 6 16\n 7 17\n 8 18'

	EXEC 	-n \
		-e 'int i = 1; int foo() { return i++; }' \
		-e 'int bar() { return i++; }' \
		-e 'int end() { for (i = 0; i < 8; i++) printf("%2d %2d\n", foo(), bar()); }'
		OUTPUT - $' 0  1\n 3  4\n 6  7'

	EXEC 	-n \
		-e 'int i = 1; int foo() { return i++; }' \
		-e 'int bar() { return i++; }' \
		-e 'int end() { int i; for (i = 0; i < 8; i++) printf("%2d %2d\n", foo(), bar()); }'
		OUTPUT - $' 1  2\n 3  4\n 5  6\n 7  8\n 9 10\n11 12\n13 14\n15 16'

	EXEC 	-n \
		-e 'int i = 1; int foo() { return i++; }' \
		-e 'int bar() { return i++; }' \
		-e 'int end() { int j; for (j = 0; j < 8; j++) printf("%2d %2d\n", foo(), bar()); }'

TEST 08 'function declaration order'

	EXEC 	-e '
        		action: return 0;
        		begin: printf("begin\n");
        		select: type == REG; 
		'
		OUTPUT - begin

	EXEC	-e '
        		action: return;
        		select: type == REG; 
        		begin: printf("begin\n");
		'

	EXEC	-e '
        		begin: printf("begin\n");
        		action: return;
        		select: type == REG; 
		'

	EXEC	-e '
        		begin: printf("begin\n");
        		select: type == REG; 
        		action: return;
		'

	EXEC	-e '
        		select: type == REG; 
        		action: return;
        		begin: printf("begin\n");
		'

	EXEC	-e '
        		select: type == REG; 
        		begin: printf("begin\n");
        		action: return;
		'

TEST 09 'sum() functions'

	DO	DATA data

	EXEC	-d data -e '
			sort:	name;
        		select: type == REG; 
        		action: printf("%s %s\n", md5sum, path);
		'
		OUTPUT - '0ce2eddcb4625ca8b15d7e9833c3a595 data/aaa/111/4/q.c
08c79f0941ed1e4871ca0befe6fb3e60 data/aaa/111/4/r.d
09ec7be7fdff789bf2feca4aa88b9f26 data/aaa/111/4/s.z
164a40a6c492b79752adeffdfc4634b7 data/aaa/111/5/q.c
863bde74125c668b9030d05f5b1da9a9 data/aaa/111/5/r.d
2b84f501f250adab5bb5625bae3643c1 data/aaa/111/5/s.z
fda4ce9f6afe39ecd1ad5a9f310e1030 data/aaa/111/6/q.c
40f6268e648268cb63e36dbb23b3101b data/aaa/111/6/r.d
fadacf2cac73cc5a804e8501f833190c data/aaa/111/6/s.z
e5816438492c7aed3e6ec7f7095a4a03 data/aaa/111/7/q.c
96762d54d36764e6db861b576de65bcb data/aaa/111/7/r.d
2103906f692a2727dbb28ee8d416f48f data/aaa/111/7/s.z
2ebfa3cbd44f5b5c87e487f25b500e8c data/aaa/222/4/q.c
075011000e674a905803114d75516779 data/aaa/222/4/r.d
b2ce0e2b7835cd84e326c22d46f4dc30 data/aaa/222/4/s.z
d177f6fbe4502b44c8454447098e030b data/aaa/222/5/q.c
82611a1623e3ea3b0dcba2638729d118 data/aaa/222/5/r.d
65eeaf76d81ec5f8d741fadf9776c8ae data/aaa/222/5/s.z
f62a1b7936dd140f44f51ba8283804b8 data/aaa/222/6/q.c
484e8b637b558dc7e2e0304dff3ecf15 data/aaa/222/6/r.d
2377391b8f1bcc4781ffb8e19dacb832 data/aaa/222/6/s.z
900ea18f6ef44c5e79a34e4bc51cc5d2 data/aaa/222/7/q.c
12da74f69c1fcc04017cd09a30c4f73f data/aaa/222/7/r.d
fb7b21b88bcf7b7fcfc50274b6a3c237 data/aaa/222/7/s.z
961fa401343482e68be9599a92037e58 data/aaa/333/4/q.c
3ef00035f18017e72982b2f8a5790463 data/aaa/333/4/r.d
12689e0c1d85e81190036486520d02d5 data/aaa/333/4/s.z
29b58b5253db4828562fcf9926b5047c data/aaa/333/5/q.c
c5648273ba4e0469e64de8e0ce08d92e data/aaa/333/5/r.d
129fb07bd00a9931a96ea5dc26d2c81e data/aaa/333/5/s.z
2e000110d8e9106abed87e6cc6cf009d data/aaa/333/6/q.c
fed028a983cf5032b8d375729e0280da data/aaa/333/6/r.d
1052494b0f95295d5a294db30857dc2b data/aaa/333/6/s.z
bdb8640cdfd3e22840918f27dc3d5361 data/aaa/333/7/q.c
a4cd2aba3442e69beb6aed95de0e8b4d data/aaa/333/7/r.d
23c2783a4e7b0ff42cfa374ae6775e5e data/aaa/333/7/s.z
cdf5844020898cd043f3258ebe18eade data/zzz/111/4/q.c
c9c9b58dabc6e91ab7a11b1c5650487d data/zzz/111/4/r.d
73a6d3a18e3f93f5971bfa40557eb9d5 data/zzz/111/4/s.z
bb2943a66bed751c2c2afa798873a070 data/zzz/111/5/q.c
2dfea04533cb12d0758ccc1a5f5c5483 data/zzz/111/5/r.d
b8509437445bb5f6e1dba58e2ec8a850 data/zzz/111/5/s.z
c6e06d8a0f451707f94cd43f3ee09b67 data/zzz/111/6/q.c
a0a3a8987a03ef06145c8dc909185462 data/zzz/111/6/r.d
7727c55313e01ea2b8ce2b7374242c3e data/zzz/111/6/s.z
b470b8122213515e91165e4b56ff04f4 data/zzz/111/7/q.c
a06fbd5395e945abbbecfca702f98a2e data/zzz/111/7/r.d
f2ae57ca1772ab4bb1bd070e63bfba9e data/zzz/111/7/s.z
823f7c76572fe0edf20f624fe394b956 data/zzz/222/4/q.c
1d0dc05b2a598c4b825cc3e897bfc229 data/zzz/222/4/r.d
cbd967663e7ff6352816f1dd065c662c data/zzz/222/4/s.z
066b2e1f079577ca245ec725730620f2 data/zzz/222/5/q.c
0b5bde6a49aa20fd082791763077eb5d data/zzz/222/5/r.d
8d24e3f6d605d598df09959a29636c07 data/zzz/222/5/s.z
d9198d85b89ac8dd286306466169ef19 data/zzz/222/6/q.c
18ffa86777f40787a6ae4f7996372aa8 data/zzz/222/6/r.d
8077ac3beb5a2c17a5a5fa3c9ace2501 data/zzz/222/6/s.z
d86d2e020d383c2fd36f193e6ec722e4 data/zzz/222/7/q.c
2cd7e05f790558c085e8118f762519ed data/zzz/222/7/r.d
fea73f9ce2c7f3357d01d9a6dd1c8ee4 data/zzz/222/7/s.z
5e17c08e55f83812ee5e7693d912d112 data/zzz/333/4/q.c
fb3d17ffd9567c4e33b9cefc867dcb42 data/zzz/333/4/r.d
c77f4a1c5a8ce8a8c196575fe93cf0c1 data/zzz/333/4/s.z
17f6c3e5b2cc9120eb5d195ce773825a data/zzz/333/5/q.c
528e10d853d27c464dc1137d00776d71 data/zzz/333/5/r.d
9515f2cbd12a42cec6a972faa920c1cb data/zzz/333/5/s.z
2f79d27b5b051d16b703f1610758555b data/zzz/333/6/q.c
181a1b31f997d6d03df170bc3423cfae data/zzz/333/6/r.d
925a2196476fc20b94ed758d5e61b685 data/zzz/333/6/s.z
889df6329eb6befb3fcb3187fd5103d1 data/zzz/333/7/q.c
3687548d90c3a4e3c7325897d8448c9e data/zzz/333/7/r.d
927aaf31e6809ac0a952629e852db78c data/zzz/333/7/s.z'

	EXEC	-d data -e '
			sort:	name;
        		select: type == REG; 
        		action: printf("%s %s\n", sum("md5"), path);
		'

	EXEC	-d data -e '
			sort:	name;
        		select: type == REG; 
        		action: printf("%s %s\n", sum("sha1"), path);
		'
		OUTPUT - 'c24fa4b7672894c7e0df45a153427ca9d3986c76 data/aaa/111/4/q.c
483a536172ff95845ceaa8e985867ea4be199d66 data/aaa/111/4/r.d
316443ef5451d542d74e1f4d21018d3df552c8ee data/aaa/111/4/s.z
2fbcd368ad8e7f862b0ecc0e3eac2054c49e0896 data/aaa/111/5/q.c
38d458a09a5048faf10901e4847a64d93ffa2ecb data/aaa/111/5/r.d
a80cf370bfacf3b79839b9c93cd4ec13a58b68bf data/aaa/111/5/s.z
661abd372bd537bcededd2d1d18fcb3312f16ed3 data/aaa/111/6/q.c
17be6d66712a50c70ef520fdcafe5c4ccd49c3ee data/aaa/111/6/r.d
7c5044f36f7b98ea194c7eefa7b67a1e141e8302 data/aaa/111/6/s.z
5bc6c2a6c0ee11cdc67e74a529456e29aa722d56 data/aaa/111/7/q.c
a0c144a255677cc21000c377e844198794d448cc data/aaa/111/7/r.d
edbd3ccd1913a3fb3d8be17cc30abc7f4e264291 data/aaa/111/7/s.z
aef90ebbd3569d137556982bf565605f1995fdd5 data/aaa/222/4/q.c
22a2b3238deac86bb25f1ef254fb08d5cb3e9808 data/aaa/222/4/r.d
6a1971c609ab02851ff6ea7b463e9545aa1f6334 data/aaa/222/4/s.z
9e04c94cd485646a322d37c4ea164f3d0736b374 data/aaa/222/5/q.c
f78033e93e9b9dafdaf1a5e0994d2925da8fe674 data/aaa/222/5/r.d
0da0251a97fe1ba75f98ecea88c86983da38f8af data/aaa/222/5/s.z
139a9d820ab75f1c7a18fc66919a72dd7088fe47 data/aaa/222/6/q.c
05d471a128f4146cee3402ea00374948b3072c11 data/aaa/222/6/r.d
46d3f7d24d60548f78466314619d2f227db0198d data/aaa/222/6/s.z
8bdf9cd7da9df7a16fa5cd5d43d43af3ff1d1a00 data/aaa/222/7/q.c
bb3fa30c86d0ab714689b99200e52346cd497f2c data/aaa/222/7/r.d
f8736487e0893a573980cfe59ce719d9a17c0fac data/aaa/222/7/s.z
bad046c514383800829890ce417f8ec94234f188 data/aaa/333/4/q.c
001d7a36d76b18ddc14a673153f16fd67c43c96b data/aaa/333/4/r.d
0af67e9cade742b60a8cee6c93f8e2b4ed02cb8c data/aaa/333/4/s.z
6cbbaecec2690f0720d247c75f28df44adc0dd4f data/aaa/333/5/q.c
24a6a75f535917064bd78fdb0a9b2d24b3d66fd8 data/aaa/333/5/r.d
d1fa5a976babdcb884f5142d8241fd34a7e65fbb data/aaa/333/5/s.z
681d518a176f2682dca9f163d7036f51735933d0 data/aaa/333/6/q.c
63d54a2366f90d50025d3feae7ae5cc5671af655 data/aaa/333/6/r.d
62df4ed51882877c8137d6f62929ccf706f8f62f data/aaa/333/6/s.z
04c7cfc53ba59fd65feedce04e24e09854eba13d data/aaa/333/7/q.c
c0859c295cf549361f98a777cf0323438fd048a0 data/aaa/333/7/r.d
9a6d518f7f8079651dc40242eb38368c1bcd0518 data/aaa/333/7/s.z
1f4640b41232dd1bdc39733f0c77c34b297d7cdb data/zzz/111/4/q.c
ba13f002c3060c2001dfa1b800976ea29b23967b data/zzz/111/4/r.d
e1e0d6ee2eb87266b5c7855812534944bd78fefb data/zzz/111/4/s.z
505d3cd5fbdba518e9de7e5a1c353e0a4fa870e9 data/zzz/111/5/q.c
65cbeef43f79d05efb1f420f42bbd8ff8e497c2a data/zzz/111/5/r.d
38258ea9201d0f6d0455399c28a7aa634c763c92 data/zzz/111/5/s.z
911d2a2cac4173f6871ead5c15c23bc643eeba3f data/zzz/111/6/q.c
33b03eee2cdb49537beb37c000d8f88eaec8a9d4 data/zzz/111/6/r.d
e78253ac9af02dd2c885bd72c11920743b4acebe data/zzz/111/6/s.z
eb9cfa7005241e6797d1cbe6b6ee4bc54c11d9a9 data/zzz/111/7/q.c
92af32c63199a608b50640b50a0047048e023a13 data/zzz/111/7/r.d
7a830e75964cee82f827be0e962d8ada8d584d3d data/zzz/111/7/s.z
0617964b735103940929b32b02bd7c03be9b25da data/zzz/222/4/q.c
a78769ac577b3d79153881db51959c55ae16ceb7 data/zzz/222/4/r.d
0821390a94a164839f7902384e98571163015288 data/zzz/222/4/s.z
e6d843a857d745a7d78a34debbe5f756c2a63855 data/zzz/222/5/q.c
55155b4fa3b2ab0734d5a6d57d181df8854dd061 data/zzz/222/5/r.d
8071a5c5c9ad15a3284e263fcbf54300d861afac data/zzz/222/5/s.z
11a7ea44f83a81e3eb928062566f801559d0d53f data/zzz/222/6/q.c
b726c56f54843ee210c5b7fecb2183ddfe46af47 data/zzz/222/6/r.d
dc73dcb87e81036b2c676c528e2980424de74563 data/zzz/222/6/s.z
1be2211e8fd9b9c4896fe420db2cddbe977d7276 data/zzz/222/7/q.c
af69968f1392ced27d40f9ee4c72c635784d04dc data/zzz/222/7/r.d
5c7573bbaa84b504467b801d095bb890eb396a6a data/zzz/222/7/s.z
3050ed1b4b32dbb2c3be74e0628c9d6c0e6f994f data/zzz/333/4/q.c
f1541f83527a1ea31cedc5cc3bbfc3b9f927e497 data/zzz/333/4/r.d
02d03222ee65f5e77d6dcc1674440d367884edef data/zzz/333/4/s.z
dbf8d09f039b06bc6f8fae07aa5f57d50627ba82 data/zzz/333/5/q.c
fa1d1a51e765aeccf4128da8aecc373dde847cfe data/zzz/333/5/r.d
e5ddcaeaf01497f984c5db15c23a06b238cee8bf data/zzz/333/5/s.z
ee919e4f33b3aeab78883c19f43d4bcc71017340 data/zzz/333/6/q.c
7bd35b2871f05fc31b238c5cfac10abde6177a27 data/zzz/333/6/r.d
1e908ae449a96c3fd714fb3098fa991cfa01783f data/zzz/333/6/s.z
17409a61a261a4efa503324f73868ef80e827ea8 data/zzz/333/7/q.c
3df6c68236e46a3d5980824a1f7bb9392890a93b data/zzz/333/7/r.d
b13636b37c6f5b405ca26d92b91550af154cf940 data/zzz/333/7/s.z'

	EXEC	-d data -e '
			sort:	name;
        		select: type == REG; 
        		action: printf("%s %s\n", sum("tw"), path);
		'
		OUTPUT - '3kXJzK1r5HT@2oLqCa2E435o data/aaa/111/4/q.c
2jy7kg0ss0R23PS6EI2@zp2W data/aaa/111/4/r.d
21pgX_0eDo863dfAaX2Jjb9Z data/aaa/111/4/s.z
37XzeD1nU3P61AV4xz3S2KI5 data/aaa/111/5/q.c
26xY_90myuRG2__MA50cy4FD data/aaa/111/5/r.d
1Qp6CU3AqBXO2ppe6k3XhSMG data/aaa/111/5/s.z
2WXoVw0GPzvi0N2KsY141qiO data/aaa/111/6/q.c
1VxOG23FFepS2c9qvu1qwMgk data/aaa/111/6/r.d
1DoYhN3Tg2nG1ByU1J19gynn data/aaa/111/6/s.z
2JXeAp0ZRoQq3Zcool2i05Vv data/aaa/111/7/q.c
1IxEkX3YL_9@1oj4qT2EvrT1 data/aaa/111/7/r.d
1qoNYG3amFj60NIxZ62nfd@4 data/aaa/111/7/s.z
0xO9FN35kz7y0bY4gd0fpQ6b data/aaa/222/4/q.c
3wozqj04@8x61D2MiL0BVa3J data/aaa/222/4/r.d
3efJ220GF0f@10sdQ@0kEYaM data/aaa/222/4/s.z
0kN_kG3MtW3e3o5KbC1tovIU data/aaa/222/5/q.c
3jop5c0TQvxG0Pcqe81PTRGq data/aaa/222/5/r.d
31fyIX0BzBrS0cBTMn1yDDNt data/aaa/222/5/s.z
07NQ_z3z0k_m2Afo6_2HnbjB data/aaa/222/6/q.c
36oeM50yFLZO3_m49x31Sxh7 data/aaa/222/6/r.d
2QfonQ1g@WTG3oLxHM2MCjoa data/aaa/222/6/s.z
3WNGGs0u7g0q1Mp22o3VlSWi data/aaa/222/7/q.c
2Vo4q@1tK_JS3bvK4W0fRcTQ data/aaa/222/7/r.d
2Dfe2J13VPb22AVbD93@A@@T data/aaa/222/7/s.z
1KEBLQ1nDGDy1_8JWg1SLB6@ data/aaa/333/4/q.c
0Je_wm0k1LRe3qfpYO2deX4w data/aaa/333/4/r.d
0r69850y9oLW2PETv11X@Jbz data/aaa/333/4/s.z
1xErqJ1oYOHa1binRF34KgJH data/aaa/333/5/q.c
0weRbf0pqvRC2Cp3Ub3rdCHd data/aaa/333/5/r.d
0e5@O@0ThizK1_Oxqq39ZoOg data/aaa/333/5/s.z
1kEh5C1JOlvu0ns1N20iIYko data/aaa/333/6/q.c
0jeGS80GkLhO1OyJPA0FcihW data/aaa/333/6/r.d
015QtT0UvODm1bYblP0nY4oZ data/aaa/333/6/s.z
17E6Mv1@TpQm3zBHIr1wHDX5 data/aaa/333/7/q.c
06ewx10_h0Ma0@InKZ1TaZUD data/aaa/333/7/r.d
3Q5G8M3dqbH20o5Rhc1BWL_G data/aaa/333/7/s.z
2DLT1p1tpMP23FkfRR13YK13 data/zzz/111/4/q.c
1CmgNX2swoeC14qXUn1qs3@B data/zzz/111/4/r.d
1kdqpG2KXgbK0tQpqC19bS5E data/zzz/111/4/s.z
2qLIIi1wkeT@2RtVNe2hXpDM data/zzz/111/5/q.c
1pm6sQ2zJBQq0gABPM2EqLBi data/zzz/111/5/r.d
17dg4z2ZSQ_S3F@3l_2naxIl data/zzz/111/5/s.z
2dLynb1SLvjS21DzID3vW5et data/zzz/111/6/q.c
1clY7J2Tl2mi3sKfL93Sprb_ data/zzz/111/6/r.d
0Wd5Ls31dWbW2S7Jho3B9dj2 data/zzz/111/6/s.z
20Lo241XFfIa1dNdE00JUMRa data/zzz/111/7/q.c
0_lNOC34jF9S2ETVGy14o6OI data/zzz/111/7/r.d
0JcXql3maxny22hncN0P7UVL data/zzz/111/7/s.z
3QCj7s2XU_b21swVvU2Hiv1S data/zzz/222/4/q.c
2PcIT@3Yi1Ku2TDByq31NQ_o data/zzz/222/4/r.d
2x3SvJ3KtoPG2h134F2MxD6r data/zzz/222/4/s.z
3DC8Ol2E2e7W0EGzrh3VhaEz data/zzz/222/5/q.c
2CcyyT3FEGw623NftP0fMwC5 data/zzz/222/5/r.d
2k3IaC3rDBv@1taJ023@wiJ8 data/zzz/222/5/s.z
3qB@te2lcv2e3QQdmG17fSfg data/zzz/222/6/q.c
2pcodM3mBWxO1fWVpc1tLccO data/zzz/222/6/r.d
273xRv38G2bS0FkmXr1cu@jR data/zzz/222/6/s.z
3dBQ8722b80m30ZTi32lexRZ data/zzz/222/7/q.c
2ccdUF33yMJW0s4zkB2HJTPv data/zzz/222/7/r.d
1W3nwo0RJF7y3Ru0SQ2qtFWy data/zzz/222/7/s.z
11sLdv3Br_H@3fJz9X0iEg2F data/zzz/333/4/q.c
0038@12A4pdy0GQfct0F7C0b data/zzz/333/4/r.d
3JWiBM19Z1zW04dIKI0nTo7e data/zzz/333/4/s.z
0QsAUo2ggfTG2rTd5k1wCXFm data/zzz/333/5/q.c
3P2@EW1neiQe3SZV7S1T6hCU data/zzz/333/5/r.d
3wW8gF14SB_O3gnmG51BS3JX data/zzz/333/5/s.z
0Dsqzh23evqi1E0T0J2KBDg3 data/zzz/333/6/q.c
3C2QjP12gPlS337z3f354ZdB data/zzz/333/6/r.d
3jVZXy1THRze2sx0Bu2PQLkE data/zzz/333/6/s.z
0qsgea2@bMIu0QawY63YAiSM data/zzz/333/7/q.c
3p2F@I1Zl8hW2fhc@E0j3EQi data/zzz/333/7/r.d
36VPCr1yKgDC1EGGwT01PqXl data/zzz/333/7/s.z'

TEST 10 'symbol table scope'

	DO DATA one

	EXEC	-d one -e '
			 begin: printf("begin:\n");
        		select: if (type == REG) printf("select: %s\n", path); else return 0;
        		action: printf("action: %s\n", path);
        		   end: printf("end:\n");
		'
		OUTPUT - $'begin:\nselect: one/one\naction: one/one\nend:'
