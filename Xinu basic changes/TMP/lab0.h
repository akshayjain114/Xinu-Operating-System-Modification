/* lab0.h */

#ifndef _LAB0_H_
#define _LAB0_H_

extern	int	zfunction();	/*new zfunction*/

struct SystemCallInfo{
	char *name;
	int count;
	int totalTime;
};

struct ProcessCall{
	int processCalledWhileTracing;
	struct SystemCallInfo systemCallsInfo[27];
};

extern struct ProcessCall processCallsInfo[NPROC];
extern int traceSystemCalls;

#define	freememsyscall 0
#define chpriosyscall 1
#define getpidsyscall 2 
#define getpriosyscall 3
#define	gettimesyscall 4
#define killsyscall 5
#define	receivesyscall 6
#define	recvclrsyscall 7
#define	recvtimsyscall 8
#define resumesyscall 9
#define scountsyscall 10
#define sdeletesyscall 11
#define	sendsyscall 12
#define	setdevsyscall 13
#define	setnoksyscall 14
#define screatesyscall 15
#define signalsyscall 16
#define signalnsyscall 17
#define	sleepsyscall 18
#define	sleep10syscall 19
#define sleep100syscall 20
#define sleep1000syscall 21
#define sresetsyscall 22
#define stacktracesyscall 23
#define	suspendsyscall 24
#define	unsleepsyscall 25
#define	waitsyscall 26

#endif