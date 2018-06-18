#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {
	if(npages > 128 || npages <= 0)
		return SYSERR;
	else if( (&bsm_tab[bs_id])->bs_created == BSM_NOTCREATED){
		(&bsm_tab[bs_id])->bs_created = BSM_CREATED;
		(&bsm_tab[bs_id])->bs_npages = npages;
	}
	return (&bsm_tab[bs_id])->bs_npages;
}


