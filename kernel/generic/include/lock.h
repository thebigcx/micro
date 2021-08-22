#pragma once

typedef volatile int lock_t;

#define LOCK(name)\
	while (__sync_lock_test_and_set(&name, 1)) asm ("pause");

#define TEST_LOCK(name) ({ int stat; stat = __sync_lock_test_and_set(&name, 1); stat; })

#define UNLOCK(name)\
	__sync_lock_release(&name);
