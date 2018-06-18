#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <stdio.h>

extern int schedclass;

void setschedclass(int sched_class){
	schedclass = sched_class;
}

int getschedclass(){
	return schedclass;
}