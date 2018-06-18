void reader (char *msg, int lck)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);
	sleep(3);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void writer (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 10s\n", msg);
        sleep (10);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void mediumPriorityThread(char *msg){
	resched();
	int i;
	for(i=0;i<10;i++){
		kprintf("Medium priority thread is running\n");
		sleep(1);
	}
}

void test ()
{
        int     lck;
        int     rd;
        int     wr;
	int	med;

        kprintf("\nPriority inheritance test case\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test failed");
	
        rd = create(reader, 2000, 20, "reader", 2, "reader", lck);
        wr = create(writer, 2000, 40, "writer", 2, "writer", lck);
	med = create(mediumPriorityThread, 2000, 30, "mediumPriorityThread", 1, "mediumPriorityThread");
	
	kprintf("-start reader, then sleep 1s. lock granted to reader (prio 20)\n");
        resume(rd);
        sleep (1);

	kprintf("-start writer, then sleep 1s. writer(prio 40) blocked on the lock\n");
        resume(wr);
        sleep (1);
	assert (getprio(rd) == 40, "Test failed");

	kprintf("-start medium priority thread, then sleep 1s. medium priority thread runs\n");
        resume(med);
        sleep (1);

	sleep(10);
        kprintf ("Test OK\n");
}

//Test case with semaphore
void sreader (char *msg, int sem)
{
        kprintf ("  %s: to wait on semaphore\n", msg);
        wait (sem);
        kprintf ("  %s: acquired semaphore\n", msg);
	sleep(5);
        kprintf ("  %s: to signal semaphore\n", msg);
        signal (sem);
}

void swriter (char *msg, int sem)
{
        kprintf ("  %s: to wait on semaphore\n", msg);
        wait (sem);
        kprintf ("  %s: acquired semaphore\n", msg);
	sleep(10);
        kprintf ("  %s: to signal semaphore\n", msg);
        signal (sem);
}

void smediumPriorityThread(char *msg){
	resched();
	int i;
	for(i=0;i<10;i++){
		kprintf("Medium priority thread is running\n");
		sleep(1);
	}
}

void stest ()
{
        int     sem;
        int     rd;
        int     wr;
	int	med;

        kprintf("\nSemaphore test\n");
        sem  = screate (1);
        assert (sem != SYSERR, "Test failed");
	
        rd = create(sreader, 2000, 20, "sreader", 2, "sreader", sem);
        wr = create(swriter, 2000, 40, "swriter", 2, "swriter", sem);
	med = create(smediumPriorityThread, 2000, 30, "smediumPriorityThread", 1, "smediumPriorityThread");
	
	kprintf("-start reader, then sleep 1s. semaphore granted to reader (prio 20)\n");
        resume(rd);
        sleep (1);

	kprintf("-start writer, then sleep 1s. writer(prio 40) blocked on the semaphore\n");
        resume(wr);
        sleep (1);
	assert (getprio(rd) == 20, "Test failed");

	kprintf("-start medium priority thread, then sleep 1s. medium priority thread runs\n");
        resume(med);
        sleep (1);

	sleep(10);
        kprintf ("Test OK\n");
}