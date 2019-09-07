#ifndef SCHEDULE_H
#define SCHEDULE_H

#define EXPDISTSCHED 1
#define LINUXSCHED 2
#define DEFAULT 0
int scheduler_class;

void setschedclass(int sched_class);
int getschedclass();

#endif
