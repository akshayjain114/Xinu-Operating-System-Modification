/* clean.c - create, newpid */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

SYSCALL clean(int pid){
	free_frames(pid);
	bsm_unmap(pid, (&proctab[pid])->vhpno);

	/* Depends on whether backing store should be usable after private process unmaps it
	if( (&bsm_tab[ (&proctab[pid])->store ])->bs_access_modifier == BSM_PRIVATE )
		release_bs((&proctab[pid])->store);
	*/
}