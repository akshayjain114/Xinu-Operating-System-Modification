/* lock.h - isbadlock */
#ifndef _LOCK_H_
#define _LOCK_H_

#define	NLOCKS 50	/* number of locks, if not defined	*/

#define	READ 0	/* current proces holding the lock	*/

#define	WRITE 1

#define	LFREE	'\01'		/* this semaphore is free			*/
#define	LUSED	'\02'		/* this semaphore is used			*/

struct	lentry	{		/* lock table entry			*/
	char	lstate;		/* the state LFREE or LUSED			*/
	int	lreadercnt;	/* reader count for this lock			*/
	int	lwritercnt;	/* writer count for this lock			*/
	int currentprocesstype;	/* 0 - READ, 1 - WRITE, default is READ		*/
	int	lqhead;		/* q index of head of list			*/
	int	lqtail;		/* q index of tail of list			*/
	int	lprio;		/* max priority amongst blocked processes	*/
	int 	processes[2];	/* processes that have acquired the lock	*/	
};
extern	struct	lentry	locks[];
extern	int	nextlock;

#define	isbadlock(l)	(l<0 || l>=NLOCKS)

int highestWaitingWriter(int lockdescriptor); /* returns highest priority amongst writers waiting for lockdescriptor*/

/* checks if reader has higher priority (or equal with more wait time) than the highest waiting writer */
int isReaderWorthReadying(int pid, int highestWriterPid);

int checkWriteOrReadNext(int lockdescriptor); /* check whether read process(es) or a write process will be unblocked next */

void updateAllPriorities(int lockdescriptor, int lprio);
void pupdateAllPriorities(int pid, int prioritytobechecked, int isalive);

#endif
