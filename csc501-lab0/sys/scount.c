/* scount.c - scount */

#include <conf.h>
#include <kernel.h>
#include <sem.h>

/*------------------------------------------------------------------------
 *  scount  --  return a semaphore count
 *------------------------------------------------------------------------
 */
SYSCALL scount(int sem)
{
	call_start_d(getpid(), "scount");
extern	struct	sentry	semaph[];

	if (isbadsem(sem) || semaph[sem].sstate==SFREE)
		return(SYSERR);
	
	call_end_d(getpid(), "scount");
	return(semaph[sem].semcnt);
}
