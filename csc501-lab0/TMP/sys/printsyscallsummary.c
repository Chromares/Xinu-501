#include <stdio.h>
#include <kernel.h>
#define ON 1
#define STOP 2
#define OFF 0

extern unsigned long  ctr1000;

typedef struct syscall_data {
	short int pid_count[50];
	char name[15];
	unsigned long start_end[50];
	unsigned long sum[50];
} syscall_data;

const char *call_names[] = {"freemem",
"chprio",
"getpid",
"getprio",
"gettime",
"kill",
"receive",
"recvclr",
"recvtim",
"resume",
"scount",
"sdelete",
"send",
"setdev",
"setnok",
"screate",
"signal",
"signaln",
"sleep",
"sleep10",
"sleep100",
"sleep1000",
"sreset",
"stacktrace",
"suspend",
"unsleep",
"wait" };

short int status = OFF;
//each process has its own index or pid, hence no need for locks
//their region will be isolated from other processes, concurrent, if any.
syscall_data stats[27];
unsigned short int caller_mem[50];

void sy_init() {
	int i, j;
	for(i = 0; i < 27; i++) {
		for(j = 0; j < 50; j++) {
			stats[i].pid_count[j] = 0;
			stats[i].sum[j] = 0;
			stats[i].start_end[j] = 0;
		}
		strcpy(stats[i].name, call_names[i]);
	}
	return;
}

int find_index(char *call) {
	int i = 0;
	for(i = 0; i < 27; i++) {
		if(!strcmp(call, stats[i].name))
			return i;	
	}
	return i;
}

void printsyscallsummary() {
	if(status == STOP) {
		//kprintf("%s %d %s index = %d\n", stats[4].name, stats[7].pid_count[3], stats[26].name, find_index("gettime"));
	}
	else if(status == ON) {
		kprintf("[syscallsummary.c] Tracer is running, call end function first.\n");
		return;
	}
	else {
		kprintf("[syscallsummary.c] Call start function first to start tracing.\n");
		return;
	}

	int i = 0, j = 0;
	for(i = 0; i < 50; i++) {
		if(!caller_mem[i])
			continue;
		kprintf("Process [pid:%d]\n", i);
		for(j = 0; j < 27; j++) {
			if(!stats[j].pid_count[i])
				continue;
			kprintf("\tSyscall: %s, count: %d, average execution time: %ld (ms)\n", stats[j].name, stats[j].pid_count[i], (stats[j].sum[i]));
		}
	}
	return;
}

void syscallsummary_start() {
	sy_init();
	if(status != OFF) {
		kprintf("[syscallsummary.c] Tracer already running.\n");
		return;
	}
	else {
		status = ON;
		return;
	}
}

void syscallsummary_stop() {
	if(status == ON) {
		status = STOP;
	}
	else {
		kprintf("[syscallsummary.c] Tracer was not running, nothing to stop.\n");
	}
	return;
}

void call_start_d(int pid, char *call) {
	if(!status) {
		return;
	}
	else {
		//kprintf("[debug] invoked %d for %s\n", pid, call);
		caller_mem[pid] = 1;
		int index = find_index(call);
		stats[index].pid_count[pid] += 1;
		stats[index].start_end[pid] = ctr1000;		
	}
	return;
}

void call_end_d(int pid, char *call) {	
	if(!status) {
		return;
	}
	else {
		int index = find_index(call);
		stats[index].start_end[pid] = ctr1000 - stats[index].start_end[pid];
		stats[index].sum[pid] += stats[index].start_end[pid];
		stats[index].start_end[pid] = 0;
	}
	return;
}
