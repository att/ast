# tests for the nl command

function DATA
{
	for f
	do	test -f $f && continue
		case $f in
		f1)	cat <<'!'
line 1
line 2
\:\:\:
this is a header
\:\:
line 1


line 4




line 8
xxxx
\:
this is a trailer
line 2
\:\:\:
line 11
line 34

!
			;;
		esac > $f
	done
}


TEST 01 'basics'
	DO DATA f1
	EXEC	f1
		OUTPUT - $'     1	line 1
     2	line 2

       this is a header

     1	line 1
       
       
     2	line 4
       
       
       
       
     3	line 8
     4	xxxx

       this is a trailer
       line 2

       line 11
       line 34
       '
	EXEC	-w 10 f1
		OUTPUT - $'         1	line 1
         2	line 2

           this is a header

         1	line 1
           
           
         2	line 4
           
           
           
           
         3	line 8
         4	xxxx

           this is a trailer
           line 2

           line 11
           line 34
           '
	EXEC	--number-width=10 f1

TEST 02 'diagnostics'
	DO DATA f1
	EXEC	--FOO f1
		DIAGNOSTICS
		EXIT [12]

TEST 03 single_options
	DO DATA f1
	EXEC -w 4 f1
		OUTPUT - $'   1	line 1
   2	line 2

     this is a header

   1	line 1
     
     
   2	line 4
     
     
     
     
   3	line 8
   4	xxxx

     this is a trailer
     line 2

     line 11
     line 34
     '
	EXEC --number-width  4 f1
	EXEC -w 8 f1
		OUTPUT - $'       1	line 1
       2	line 2

         this is a header

       1	line 1
         
         
       2	line 4
         
         
         
         
       3	line 8
       4	xxxx

         this is a trailer
         line 2

         line 11
         line 34
         '
	EXEC --number-width  8 f1
	EXEC -w 20 f1
		OUTPUT - $'                   1	line 1
                   2	line 2

                     this is a header

                   1	line 1
                     
                     
                   2	line 4
                     
                     
                     
                     
                   3	line 8
                   4	xxxx

                     this is a trailer
                     line 2

                     line 11
                     line 34
                     '
	EXEC --number-width  20 f1
	EXEC -l 1 f1
		OUTPUT - $'     1	line 1
     2	line 2

       this is a header

     1	line 1
       
       
     2	line 4
       
       
       
       
     3	line 8
     4	xxxx

       this is a trailer
       line 2

       line 11
       line 34
       '
	EXEC --join-blank-lines  1 f1
	EXEC -l 2 f1
		OUTPUT - $'     1	line 1
     2	line 2

       this is a header

     1	line 1
       
       
     2	line 4
       
       
       
       
     3	line 8
     4	xxxx

       this is a trailer
       line 2

       line 11
       line 34
       '
	EXEC --join-blank-lines  2 f1
	EXEC -l 4 f1
		OUTPUT - $'     1	line 1
     2	line 2

       this is a header

     1	line 1
       
       
     2	line 4
       
       
       
       
     3	line 8
     4	xxxx

       this is a trailer
       line 2

       line 11
       line 34
       '
	EXEC --join-blank-lines  4 f1
	EXEC -f a f1
		OUTPUT - $'     1	line 1
     2	line 2

       this is a header

     1	line 1
       
       
     2	line 4
       
       
       
       
     3	line 8
     4	xxxx

     5	this is a trailer
     6	line 2

       line 11
       line 34
       '
	EXEC --footer-numbering  a f1
	EXEC -f t f1
		OUTPUT - $'     1	line 1
     2	line 2

       this is a header

     1	line 1
       
       
     2	line 4
       
       
       
       
     3	line 8
     4	xxxx

     5	this is a trailer
     6	line 2

       line 11
       line 34
       '
	EXEC --footer-numbering  t f1
	EXEC -f n f1
		OUTPUT - $'     1	line 1
     2	line 2

       this is a header

     1	line 1
       
       
     2	line 4
       
       
       
       
     3	line 8
     4	xxxx

       this is a trailer
       line 2

       line 11
       line 34
       '
	EXEC --footer-numbering  n f1
	EXEC -f pline f1
		OUTPUT - $'     1	line 1
     2	line 2

       this is a header

     1	line 1
       
       
     2	line 4
       
       
       
       
     3	line 8
     4	xxxx

       this is a trailer
     5	line 2

       line 11
       line 34
       '
	EXEC --footer-numbering  pline f1
	EXEC -b a f1
		OUTPUT - $'     1	line 1
     2	line 2

       this is a header

     1	line 1
     2	
     3	
     4	line 4
     5	
     6	
     7	
     8	
     9	line 8
    10	xxxx

       this is a trailer
       line 2

       line 11
       line 34
       '
	EXEC --body-numbering  a f1
	EXEC -b t f1
		OUTPUT - $'     1	line 1
     2	line 2

       this is a header

     1	line 1
       
       
     2	line 4
       
       
       
       
     3	line 8
     4	xxxx

       this is a trailer
       line 2

       line 11
       line 34
       '
	EXEC --body-numbering  t f1
	EXEC -b n f1
		OUTPUT - $'       line 1
       line 2

       this is a header

       line 1
       
       
       line 4
       
       
       
       
       line 8
       xxxx

       this is a trailer
       line 2

       line 11
       line 34
       '
	EXEC --body-numbering  n f1
	EXEC -b pline f1
		OUTPUT - $'     1	line 1
     2	line 2

       this is a header

     1	line 1
       
       
     2	line 4
       
       
       
       
     3	line 8
       xxxx

       this is a trailer
       line 2

       line 11
       line 34
       '
	EXEC --body-numbering  pline f1
	EXEC -h a f1
		OUTPUT - $'     1	line 1
     2	line 2

     1	this is a header

     2	line 1
       
       
     3	line 4
       
       
       
       
     4	line 8
     5	xxxx

       this is a trailer
       line 2

     1	line 11
     2	line 34
     3	'
	EXEC --header-numbering  a f1
	EXEC -h t f1
		OUTPUT - $'     1	line 1
     2	line 2

     1	this is a header

     2	line 1
       
       
     3	line 4
       
       
       
       
     4	line 8
     5	xxxx

       this is a trailer
       line 2

     1	line 11
     2	line 34
       '
	EXEC --header-numbering  t f1
	EXEC -h n f1
		OUTPUT - $'     1	line 1
     2	line 2

       this is a header

     1	line 1
       
       
     2	line 4
       
       
       
       
     3	line 8
     4	xxxx

       this is a trailer
       line 2

       line 11
       line 34
       '
	EXEC --header-numbering  n f1
	EXEC -h pline f1
		OUTPUT - $'     1	line 1
     2	line 2

       this is a header

     1	line 1
       
       
     2	line 4
       
       
       
       
     3	line 8
     4	xxxx

       this is a trailer
       line 2

     1	line 11
     2	line 34
       '
	EXEC --header-numbering  pline f1
	EXEC -i 1 f1
		OUTPUT - $'     1	line 1
     2	line 2

       this is a header

     1	line 1
       
       
     2	line 4
       
       
       
       
     3	line 8
     4	xxxx

       this is a trailer
       line 2

       line 11
       line 34
       '
	EXEC --page-increment  1 f1
	EXEC -i 3 f1
		OUTPUT - $'     1	line 1
     4	line 2

       this is a header

     1	line 1
       
       
     4	line 4
       
       
       
       
     7	line 8
    10	xxxx

       this is a trailer
       line 2

       line 11
       line 34
       '
	EXEC --page-increment  3 f1
	EXEC -i 5 f1
		OUTPUT - $'     1	line 1
     6	line 2

       this is a header

     1	line 1
       
       
     6	line 4
       
       
       
       
    11	line 8
    16	xxxx

       this is a trailer
       line 2

       line 11
       line 34
       '
	EXEC --page-increment  5 f1
	EXEC -v 1 f1
		OUTPUT - $'     1	line 1
     2	line 2

       this is a header

     1	line 1
       
       
     2	line 4
       
       
       
       
     3	line 8
     4	xxxx

       this is a trailer
       line 2

       line 11
       line 34
       '
	EXEC --starting-line-number  1 f1
	EXEC -v 5 f1
		OUTPUT - $'     5	line 1
     6	line 2

       this is a header

     5	line 1
       
       
     6	line 4
       
       
       
       
     7	line 8
     8	xxxx

       this is a trailer
       line 2

       line 11
       line 34
       '
	EXEC --starting-line-number  5 f1
	EXEC -v -5 f1
		OUTPUT - $'    -5	line 1
    -4	line 2

       this is a header

    -5	line 1
       
       
    -4	line 4
       
       
       
       
    -3	line 8
    -2	xxxx

       this is a trailer
       line 2

       line 11
       line 34
       '
	EXEC --starting-line-number  -5 f1
	EXEC -n ln f1
		OUTPUT - $'1     	line 1
2     	line 2

       this is a header

1     	line 1
       
       
2     	line 4
       
       
       
       
3     	line 8
4     	xxxx

       this is a trailer
       line 2

       line 11
       line 34
       '
	EXEC --number-format  ln f1
	EXEC -n rn f1
		OUTPUT - $'     1	line 1
     2	line 2

       this is a header

     1	line 1
       
       
     2	line 4
       
       
       
       
     3	line 8
     4	xxxx

       this is a trailer
       line 2

       line 11
       line 34
       '
	EXEC --number-format  rn f1
	EXEC -n rz f1
		OUTPUT - $'000001	line 1
000002	line 2

       this is a header

000001	line 1
       
       
000002	line 4
       
       
       
       
000003	line 8
000004	xxxx

       this is a trailer
       line 2

       line 11
       line 34
       '
	EXEC --number-format  rz f1
	EXEC -p f1
		OUTPUT - $'     1	line 1
     2	line 2

       this is a header

     3	line 1
       
       
     4	line 4
       
       
       
       
     5	line 8
     6	xxxx

       this is a trailer
       line 2

       line 11
       line 34
       '
	EXEC --no-renumber f1
	EXEC -d ab f1
		OUTPUT - $'     1	line 1
     2	line 2
     3	\\:\\:\\:
     4	this is a header
     5	\\:\\:
     6	line 1
       
       
     7	line 4
       
       
       
       
     8	line 8
     9	xxxx
    10	\\:
    11	this is a trailer
    12	line 2
    13	\\:\\:\\:
    14	line 11
    15	line 34
       '
	EXEC --section-delimiter  ab f1
	EXEC -d c f1
		OUTPUT - $'     1	line 1
     2	line 2
     3	\\:\\:\\:
     4	this is a header
     5	\\:\\:
     6	line 1
       
       
     7	line 4
       
       
       
       
     8	line 8
     9	xxxx
    10	\\:
    11	this is a trailer
    12	line 2
    13	\\:\\:\\:
    14	line 11
    15	line 34
       '
	EXEC --section-delimiter  c f1
	EXEC -d '\:' f1
		OUTPUT - $'     1	line 1
     2	line 2

       this is a header

     1	line 1
       
       
     2	line 4
       
       
       
       
     3	line 8
     4	xxxx

       this is a trailer
       line 2

       line 11
       line 34
       '
	EXEC --section-delimiter  '\:' f1
