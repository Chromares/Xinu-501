#include <stdio.h>
#include <sched.h>

extern int scheduler_class;
void setschedclass(int sched_class) {
	if((sched_class != EXPDISTSCHED) && (sched_class != LINUXSCHED))
		return;
	scheduler_class = sched_class;
	return;

}

int getschedclass() {
	return (scheduler_class);	
}
