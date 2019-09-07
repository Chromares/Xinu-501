#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>
#include <lock.h>

void readera(char *msg, int lck, int x) {
        int ret;
	int count = 0;
        kprintf ("a: Acquiring lock\n");
        lock(lck, READ, x);
	kprintf ("a: Acquired lock\n");
	sleep(2);
	for(ret = 0; ret < 1000001; ret++)
		count++;
	releaseall (1, lck);
	kprintf("a: Relased lock\n");
	return;
}


void readerb(char *msg, int lck, int x) {
        int ret;
	int count = 0;
        kprintf ("b: Started running\n");
	for(ret = 0; ret < 1000000; ret++)
		count++;
	for(ret = 0; ret < 20; ret++)
		kprintf("b");
	kprintf(" (%d)\n", count);
	kprintf("b: Completed running\n");
	return;
}


void writer(char *msg, int lck, int x) {
        int ret;
	int count = 0;
        kprintf("c: Acquiring lock\n");
        lock(lck, WRITE, x);
        kprintf("c: Acquired lock\n");
	for(ret = 0; ret < 999999; ret++)
		count++;
	releaseall (1, lck);
	kprintf("c: Relased lock\n");
	return;
}

void test1() {
	int lck, a, b, c;
	lck = lcreate();
	
	kprintf("Reader/Writer locks\n\n");

	a = create(readera, 2000, 10, "readera", 3, 'A', lck, 20);
        b = create(readerb, 2000, 20, "readerb", 3, 'B', lck, 30);
        c = create(writer, 2000, 30, "writer", 3, 'C', lck, 25);

	resume(a);
	sleep(1);
	resume(c);
	sleep(1);
	resume(b);
	sleep(4);
	kprintf("\n___________________\n\n");
	return;
}
//____________________________test 2_________________________

void reader2a(char *msg, int lck, int x) {
        int ret;
	int count = 0;
        kprintf ("a: Acquiring lock\n");
        wait(lck);
	kprintf ("a: Acquired lock\n");
	sleep(2);
	for(ret = 0; ret < 1000001; ret++)
		count++;
        signal(lck);
	kprintf("a: Relased lock\n");
	return;
}


void reader2b(char *msg, int lck, int x) {
        int ret;
	int count = 0;
	char b = 'b';
        kprintf ("b: Started running\n");
	for(ret = 0; ret < 1000000; ret++)
		count++;
	for(ret = 0; ret < 20; ret++)
		kprintf("%c", b);
	kprintf(" (%d)\n", count);
	kprintf("b: Completed running\n");
	return;
}


void writer2(char *msg, int lck, int x) {
        int ret;
	int count = 0;
        kprintf("c: Acquiring lock\n");
        wait(lck);
        kprintf("c: Acquired lock\n");
	for(ret = 0; ret < 999999; ret++)
		count++;
        signal(lck);
	kprintf("c: Relased lock\n");
	return;
}


void test2() {
	int lock, a, b, c;
	lock = screate(1);
	kprintf("Semaphores\n\n");
	
	a = create(reader2a, 2000, 10, "reader2a", 3, 'A', lock, 20);
        b = create(reader2b, 2000, 20, "reader2b", 3, 'B', lock, 30);
        c = create(writer2, 2000, 30, "writer2", 3, 'C', lock, 25);

	resume(a);
	sleep(1);
	resume(c);
	sleep(1);
	resume(b);
	sleep(4);
	return;
}



int main() {
	
	test1();
	sleep(10);
	test2();
	sleep(10);
	shutdown();
	return OK;
}
