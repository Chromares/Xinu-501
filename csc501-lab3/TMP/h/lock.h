#ifndef _LOCK_H_
#define _LOCK_H_

#ifndef NLOCK
#define NLOCK 50
#endif

#ifndef NLOCKS
#define NLOCKS NLOCK
#endif

#ifndef DELETED
#define DELETED -1
#endif

#define LFREE  '\01'
#define LUSED  '\02'

#define LTREAD 0
#define LTWRITE 1

#define READ LTREAD
#define WRITE LTWRITE
struct lentry {
	char lstate;
	int  lhead;
	int  ltail;
	int  nreaders;
	int  nwriters;
	int  lprio;
	int  held[NPROC];
};

extern struct lentry locktab[];
extern int  nextlock;
extern ctr1000;
#define isbadlock(l) (l < 0 || l >= NLOCK)
#endif
