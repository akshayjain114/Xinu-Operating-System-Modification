#include <conf.h>
#include <kernel.h>
#include <q.h>
#include <lock.h>

struct	lentry	locks[NLOCKS];	/* Lock table				*/
int	nextlock;		/* next lock slot to use in lcreate	*/

void linit(){
	struct lentry *lptr;
	nextlock = NLOCKS - 1;
	int i;
	for (i=0 ; i<NLOCKS ; i++) {	/* initialize locks */
		(lptr = &locks[i])->lstate = LFREE;
		lptr->lqtail = 1 + (lptr->lqhead = newqueue());
	}
}