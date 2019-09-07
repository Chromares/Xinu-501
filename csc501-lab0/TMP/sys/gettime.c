/* gettime.c - gettime */

#include <conf.h>
#include <kernel.h>
#include <date.h>

extern int getutim(unsigned long *);

/*------------------------------------------------------------------------
 *  gettime  -  get local time in seconds past Jan 1, 1970
 *------------------------------------------------------------------------
 */
SYSCALL	gettime(long *timvar)
{
	call_start_d(getpid(), "gettime");
    /* long	now; */

	/* FIXME -- no getutim */
	call_end_d(getpid(), "gettime");
    return OK;
}
