/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
	if( (&bsm_tab[source])->bs_created == BSM_CREATED
		&& (&bsm_tab[source])->bs_access_modifier == BSM_PUBLIC 
		&& virtpage >= VIRTUALPAGE0)
	{
		if( bsm_map(currpid, virtpage, source, npages, BSM_PUBLIC) == OK ){
			(&proctab[currpid])->vhpno = virtpage;
			return OK;
		}
		else{
			//kprintf("xmmap call invalid\n");
			return SYSERR;
		}
	}
	else
	{
		kprintf("xmmap call invalid\n");
		return SYSERR;
	}
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
	free_pageframes(currpid);
	return bsm_unmap(currpid, virtpage, -1);
}
