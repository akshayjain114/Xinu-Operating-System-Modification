/* policy.c = srpolicy*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>


extern int page_replace_policy;
extern int sc_head;

/*-------------------------------------------------------------------------
 * srpolicy - set page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL srpolicy(int policy)
{
  policydebugging = 1;
  /* sanity check ! */
  if (policy != SC && policy != LFU)
	return SYSERR;

  page_replace_policy = policy;

  return OK;
}

/*-------------------------------------------------------------------------
 * grpolicy - get page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL grpolicy()
{
  return page_replace_policy;
}

SYSCALL getFrameFromPolicy(int *frame){
	if( grpolicy() == SC){
		return SecondChance(frame);
	}
	else if( grpolicy() == LFU){
		return LeastFrequentlyUsed(frame);
	}
	else
		return SYSERR;
}

SYSCALL LeastFrequentlyUsed(int *frame){
	int f;
	int min_fr_refcnt = 2147483647;
	int largest_fr_vpno = -1;

	for(f = 0; f < NFRAMES; f++){
		if( (&frm_tab[f])->fr_type == FR_PAGE){
			int refcnt = (&frm_tab[f])->fr_refcnt;
			int vpno = (&frm_tab[f])->fr_vpno;
			if(refcnt < min_fr_refcnt){
				*frame = f;
				min_fr_refcnt = refcnt;
				largest_fr_vpno = vpno;
			}
			else if(refcnt == min_fr_refcnt){
				if(vpno > largest_fr_vpno){
					*frame = f;
					min_fr_refcnt = refcnt;
					largest_fr_vpno = vpno;
				}
			}
		}
		
	}
	if( *frame>= 0 && *frame < NFRAMES){
		return OK;
	}
	else{
		return SYSERR;
	}

}


SYSCALL SecondChance(int *frame){
	STATWORD ps;
	disable(ps);

	if(schead == NULL)
		return SYSERR;
	else{
		while(TRUE){		
			int f = schead->frameno;
			unsigned int a = (&frm_tab[f])->fr_vpno * NBPG;
			//kprintf("VPNO = %u\n", (&frm_tab[f])->fr_vpno);
			int pagetableindex = (a & 0xFFC00000) >> 22;
			int pageindex = (a & 0x003FF000) >> 12;
			int pid = (&frm_tab[f])->fr_pid;
			unsigned int *pd = (&proctab[pid])->pdbr;
			unsigned long pdentryvalue = *(pd+pagetableindex);
			unsigned int *pagetablestart = pdentryvalue & 0xfffff000;
			int ptentryvalue = *(pagetablestart + pageindex);
			int referencebit = (ptentryvalue & 0x00000020);
			if(referencebit == 0){
				*frame = f;
				scprev = schead;
				schead = schead->next;
				removeFromSCQueue(f);
				restore(ps);
				return OK;
			}
			else{
				*(pagetablestart+pageindex) = *(pagetablestart+pageindex) & ~(0x00000020);
			}

			scprev = schead;
			schead = schead->next;
		}
		restore(ps);
		return OK;
	}
}

SYSCALL insertForSC(int f){
	STATWORD ps;
	disable(ps);
	if(schead == NULL){
		schead = getmem(8);
		schead->frameno = f;
		schead->next = schead;
	}
	else if(scprev == NULL){
		scprev = getmem(8);
		scprev->frameno = f;
		scprev->next = schead;
		schead->next = scprev;
	}
	else{
		struct scqueue *newnode = getmem(8);
		newnode->frameno = f;
		newnode->next = schead;
		scprev->next = newnode;
		scprev = newnode;
	}
	restore(ps);
	return OK;
}

SYSCALL removeFromSCQueue(int f){
	STATWORD ps;
	disable(ps);

	if(schead == NULL){
		kprintf("No frame to remove from SC Queue\n");
		restore(ps);
		return SYSERR; //do nothing. The frame is not being used for second chance yet.
	}
	if(schead->frameno == f){
		if(scprev == NULL)
			schead = NULL;
		else if(schead->next == scprev){
			schead = scprev;
			scprev = NULL;
			schead->next = schead;
		}
		else{
			schead = schead->next;
		}
		return OK;
	}
	else{
		struct scqueue *curr = schead;
		int found = 0;
		while(curr->next != NULL && curr->next->frameno != f){
			curr = curr->next;
		}
		if(curr->next == NULL){
			restore(ps);
			return SYSERR;
		}
		else{
			if(curr->next != scprev){
				curr->next = curr->next->next;
				restore(ps);
				return OK;
			}
			else{
				if(schead->next == scprev){
					schead->next = schead;
					scprev = NULL;
					restore(ps);
					return OK;
				}
				else{
					scprev = curr;
					scprev->next = schead;
					restore(ps);
					return OK;
				}
			}
		}
	}
}