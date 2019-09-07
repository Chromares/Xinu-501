/* getpid.c - getpid */

#include <conf.h>
#include <kernel.h>
#include <proc.h>

/*------------------------------------------------------------------------
 * getpid  --  get the process id of currently executing process
 *------------------------------------------------------------------------
 */
SYSCALL getpid()
{	
	call_start_d(currpid, "getpid");
	call_end_d(currpid, "getpid");
	return(currpid);
}
