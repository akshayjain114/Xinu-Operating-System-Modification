/* proc.h - isbadpid */

#ifndef _PROC_H_
#define _PROC_H_

/* process table declarations and defined constants			*/

#ifndef	NPROC				/* set the number of processes	*/
#define	NPROC		30		/*  allowed if not already done	*/
#endif

#ifndef	_NFILE
#define _NFILE		20		/* # of files allowed */
#endif

#define	FDFREE		-1		/* free file descriptor */
#define PRFREE		'\002'          /* process slot is free         */


/* process state constants */

#define	PRCURR		'\001'		/* process is currently running	*/
#define	PRFREE		'\002'		/* process slot is free		*/
#define	PRREADY		'\003'		/* process is on ready queue	*/
#define	PRRECV		'\004'		/* process waiting for message	*/
#define	PRSLEEP		'\005'		/* process is sleeping		*/
#define	PRSUSP		'\006'		/* process is suspended		*/
#define	PRWAIT		'\007'		/* process is on semaphore queue*/
#define	PRTRECV		'\010'		/* process is timing a receive	*/

#define	READ		0		/* process is read 		*/

#define	WRITE		1		/* process is write 		*/

#define NLOCKS		50		/* number of locks		*/

/* miscellaneous process definitions */

#define	PNMLEN		16		/* length of process "name"	*/

#define	NULLPROC	0		/* id of the null process; it	*/
					/*  is always eligible to run	*/
#define	BADPID		-1		/* used when invalid pid needed	*/

#define	isbadpid(x)	(x<=0 || x>=NPROC)

/* process table entry */

struct	pentry	{
	char	pstate;					/* process state: PRCURR, etc.		*/
	int	pprio;					/* process priority			*/
	int	pesp;					/* saved stack pointer			*/
	STATWORD pirmask;				/* saved interrupt mask			*/
	int	psem;					/* semaphore if process waiting		*/
	WORD	pmsg;					/* message sent to this process		*/
	char	phasmsg;				/* nonzero iff pmsg is valid		*/
	WORD	pbase;					/* base of run time stack		*/
	int	pstklen;				/* stack length				*/
	WORD	plimit;					/* lowest extent of stack		*/
	char	pname[PNMLEN];				/* process name				*/
	int	pargs;					/* initial number of arguments		*/
	WORD	paddr;					/* initial code address			*/
	WORD	pnxtkin;				/* next-of-kin notified of death	*/
	Bool	ptcpumode;				/* proc is in TCP urgent mode		*/
	short	pdevs[2];				/* devices to close upon exit		*/
	int	fildes[_NFILE];				/* file - device translation		*/
	int	ppagedev;				/* pageing dgram device			*/
	int	pwaitret;	

	int	plock;					/* lock if process waiting		*/
	int	plocktype;				/* 0 for READER, 1 for WRITER		*/
	int 	plockpriority;				/* priority for lock			*/
	int	plocksheld[2];				/* locks held by process using bits	*/
	unsigned long plockwaittime;			/* time when wait for lock was called	*/
	int	pinh;					/* inherited priority			*/
	int	lockid;					/* lock id of process if blocked	*/
	int	associatedlocks[NLOCKS];		/* to check lock deleted condition	*/
};


extern	struct	pentry proctab[];
extern	int	numproc;		/* currently active processes	*/
extern	int	nextproc;		/* search point for free slot	*/
extern	int	currpid;		/* currently executing process	*/

#endif
