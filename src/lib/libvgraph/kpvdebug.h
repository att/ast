#ifndef _KPVDEBUG_H
#define _KPVDEBUG_H	1

#ifdef DEBUG

_BEGIN_EXTERNS_
extern void	abort _ARG_((void));
_END_EXTERNS_
#ifndef __LINE__
#define __LINE__	0
#endif
#ifndef __FILE__
#define __FILE__	"Unknown"
#endif

static void _oops(char* file, int line)
{
	char buf[1024];
	sprintf(buf, "\nFailed at %s:%d\n", file, line);
	write(2,buf,strlen(buf));
	abort();
}

#include <sys/times.h>
#include <sys/resource.h>
static double _getTime ( void )
{	double		tm;
	struct rusage	u;
	getrusage ( RUSAGE_SELF, &u );
	tm = (double)u.ru_utime.tv_sec  + (double)u.ru_utime.tv_usec/1000000.0;
	return tm;
}

static double		_Kpvtime;
#define BEGTIME()	(_Kpvtime = _getTime())
#define GETTIME()	(_getTime() - _Kpvtime)
#define ASSERT(p)	((p) ? 0 : (_oops(__FILE__, __LINE__),0))
#define COUNT(n)	((n) += 1)
#define TALLY(c,n,v)	((c) ? ((n) += (v)) : (n))
#define DECLARE(t,v)	t v
#define SET(n,v)	((n) = (v))
#define PRINT(fd,s,v)	do {char _b[1024];sprintf(_b,s,v);write((fd),_b,strlen(_b));} while(0)
#define WRITE(fd,d,n)	write((fd),(d),(n))
#define KPV(temp)	(temp) /* debugging stuff that should be removed */
#define RETURN(x)	(_oops(__FILE__, __LINE__), (x))
#define BREAK		(_oops(__FILE__, __LINE__))
#define GOTO(label)	do { _oops(__FILE__, __LINE__); goto label; } while(0)

#else

#define BEGTIME()
#define GETTIME()
#define ASSERT(p)
#define COUNT(n)
#define TALLY(c,n,v)
#define DECLARE(t,v)
#define SET(n,v)
#define PRINT(fd,s,v)
#define WRITE(fd,d,n)
#define KPV(x)
#define RETURN(x)	return(x)
#define BREAK		break
#define GOTO(label)	goto label

#endif /*DEBUG*/

#endif /*_KPVDEBUG_H*/
