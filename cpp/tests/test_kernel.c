/**
 * @file test_kernel.c
 * @brief Kernel subsystem tests
 */

#include "aichat/kernel.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* Test bootstrap */
static int test_bootstrap(void) {
    printf("Testing bootstrap...\n");
    
    int ret = kern_bootstrap_init(STAGE3_COGNITIVE);
    assert(ret == 0);
    
    printf("  PASS: Bootstrap to Stage 3\n");
    return 0;
}

/* Test scheduler */
static void test_task(void* data) {
    int* count = (int*)data;
    (*count)++;
}

static int test_scheduler(void) {
    printf("Testing scheduler...\n");
    
    int ret = dtesn_sched_init();
    assert(ret == 0);
    
    int count = 0;
    task_handle_t h = dtesn_sched_task(test_task, &count, PRIORITY_NORMAL, 0);
    assert(h != 0);
    
    int executed = dtesn_sched_tick();
    assert(executed == 1);
    assert(count == 1);
    
    printf("  PASS: Task scheduling\n");
    return 0;
}

/* Test memory */
static int test_memory(void) {
    printf("Testing memory...\n");
    
    int ret = dtesn_mem_init(1024 * 1024);
    assert(ret == 0);
    
    void* ptr = dtesn_mem_alloc(256, MEM_REGION_HEAP);
    assert(ptr != NULL);
    
    dtesn_mem_free(ptr);
    
    printf("  PASS: Memory allocation\n");
    return 0;
}

/* Test HGFS */
static int test_hgfs(void) {
    printf("Testing hypergraph FS...\n");
    
    /* Bootstrap first */
    kern_bootstrap_init(STAGE1_HYPERGRAPH);
    
    void* node1 = hgfs_alloc(128, 0);
    assert(node1 != NULL);
    
    void* node2 = hgfs_alloc(128, 0);
    assert(node2 != NULL);
    
    uint64_t edge = hgfs_edge(node1, node2, 1.0f);
    assert(edge != 0);
    
    printf("  PASS: Hypergraph allocation\n");
    return 0;
}

/* Test sync primitives */
static int test_sync(void) {
    printf("Testing sync primitives...\n");

    /* Mutex */
    kern_mutex_t mtx = NULL;
    int ret = kern_mutex_init(&mtx);
    assert(ret == 0);
    assert(mtx != NULL);

    ret = kern_mutex_lock(mtx);
    assert(ret == 0);

    int try_ret = kern_mutex_trylock(mtx);  /* should fail (ERRORCHECK) */
    assert(try_ret != 0);

    ret = kern_mutex_unlock(mtx);
    assert(ret == 0);

    ret = kern_mutex_trylock(mtx);  /* should succeed now */
    assert(ret == 0);
    kern_mutex_unlock(mtx);

    kern_mutex_destroy(mtx);

    /* Spinlock */
    kern_spinlock_t sl = NULL;
    ret = kern_spinlock_init(&sl);
    assert(ret == 0);
    assert(sl != NULL);

    kern_spinlock_lock(sl);

    int tsl = kern_spinlock_trylock(sl);  /* contended */
    assert(tsl != 0);

    kern_spinlock_unlock(sl);

    tsl = kern_spinlock_trylock(sl);  /* free */
    assert(tsl == 0);
    kern_spinlock_unlock(sl);

    kern_spinlock_destroy(sl);

    /* Semaphore */
    kern_sem_t sem = NULL;
    ret = kern_sem_init(&sem, 2);
    assert(ret == 0);

    int val = 0;
    kern_sem_getvalue(sem, &val);
    assert(val == 2);

    kern_sem_wait(sem);
    kern_sem_getvalue(sem, &val);
    assert(val == 1);

    kern_sem_post(sem);
    kern_sem_getvalue(sem, &val);
    assert(val == 2);

    kern_sem_destroy(sem);

    /* NULL safety */
    kern_mutex_destroy(NULL);
    kern_spinlock_destroy(NULL);
    kern_sem_destroy(NULL);

    printf("  PASS: Sync primitives (mutex, spinlock, semaphore)\n");
    return 0;
}

/* Test timers */
static int test_timer(void) {
    printf("Testing timers...\n");

    kern_timer_t t = kern_timer_create();
    assert(t != NULL);

    assert(!kern_timer_is_running(t));

    int ret = kern_timer_start(t);
    assert(ret == 0);
    assert(kern_timer_is_running(t));

    /* Busy-wait ~1ms to get a measurable elapsed time */
    volatile long sum = 0;
    for (long i = 0; i < 1000000L; i++) sum += i;
    (void)sum;

    /* Live read while running */
    int64_t live_ns = kern_timer_elapsed_ns(t);
    assert(live_ns > 0);

    ret = kern_timer_stop(t);
    assert(ret == 0);
    assert(!kern_timer_is_running(t));

    int64_t ns = kern_timer_elapsed_ns(t);
    int64_t us = kern_timer_elapsed_us(t);
    int64_t ms = kern_timer_elapsed_ms(t);

    assert(ns > 0);
    assert(us >= 0);
    assert(ms >= 0);
    assert(us == ns / 1000);
    assert(ms == ns / 1000000);

    kern_timer_reset(t);
    assert(!kern_timer_is_running(t));
    /* elapsed_ns must return error (-1) when timer has been reset and not started */
    assert(kern_timer_elapsed_ns(t) < 0);

    kern_timer_destroy(t);

    /* NULL safety */
    kern_timer_destroy(NULL);
    assert(kern_timer_elapsed_ns(NULL) < 0);

    printf("  PASS: Timers (elapsed %lld ns)\n", (long long)ns);
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <test>\n", argv[0]);
        return 1;
    }
    
    int ret = 0;
    
    if (strcmp(argv[1], "bootstrap") == 0) {
        ret = test_bootstrap();
    } else if (strcmp(argv[1], "scheduler") == 0) {
        ret = test_scheduler();
    } else if (strcmp(argv[1], "memory") == 0) {
        ret = test_memory();
    } else if (strcmp(argv[1], "hgfs") == 0) {
        ret = test_hgfs();
    } else if (strcmp(argv[1], "sync") == 0) {
        ret = test_sync();
    } else if (strcmp(argv[1], "timer") == 0) {
        ret = test_timer();
    } else {
        fprintf(stderr, "Unknown test: %s\n", argv[1]);
        return 1;
    }
    
    return ret;
}
