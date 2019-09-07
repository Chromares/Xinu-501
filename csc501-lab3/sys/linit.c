#include <kernel.h>
//#include <conf.h>
#include <proc.h>
#include <q.h>
#include <lock.h>

struct lentry locktab[NLOCK];
int nextlock;

void linit() {
	int index = 0;
	int lock = 0;
	nextlock = NLOCK - 1;
	struct lentry *lptr;
	for(index = 0; index < NLOCK; index++) {
		locktab[index].lstate = LFREE;
		locktab[index].lhead = newqueue();
		locktab[index].ltail = locktab[index].lhead + 1;
		locktab[index].nreaders = 0;
		locktab[index].nwriters = 0;
		lptr = &locktab[index];
		while(lock < NPROC) {
			lptr->held[lock] = 0;
			lock++;
		}
	}
	return;
}
