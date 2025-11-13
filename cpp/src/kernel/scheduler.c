/**
 * @file scheduler.c
 * @brief Membrane-aware task scheduler
 * 
 * Implements Echo.Kern scheduler with:
 * - Priority-based task execution
 * - Membrane depth tracking (OEIS A000081)
 * - Target: ≤5µs per tick
 */

#include "aichat/kernel.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_TASKS 1024
#define PRIORITY_LEVELS 4

/* Task structure */
typedef struct {
    task_fn_t fn;
    void* data;
    task_priority_t priority;
    uint32_t depth;
    uint64_t handle;
    bool active;
} task_t;

/* Scheduler state */
static struct {
    task_t tasks[MAX_TASKS];
    size_t task_count;
    uint64_t next_handle;
    bool initialized;
} scheduler = {0};

/**
 * Initialize scheduler
 */
int dtesn_sched_init(void) {
    if (scheduler.initialized) {
        return 0;
    }
    
    memset(&scheduler, 0, sizeof(scheduler));
    scheduler.next_handle = 1;
    scheduler.initialized = true;
    
    return 0;
}

/**
 * Schedule a new task
 */
task_handle_t dtesn_sched_task(task_fn_t fn, void* data,
                                task_priority_t priority, uint32_t depth) {
    if (!scheduler.initialized) {
        return 0;
    }
    
    if (scheduler.task_count >= MAX_TASKS) {
        return 0;
    }
    
    /* Find free slot */
    size_t idx = 0;
    for (idx = 0; idx < MAX_TASKS; idx++) {
        if (!scheduler.tasks[idx].active) {
            break;
        }
    }
    
    if (idx >= MAX_TASKS) {
        return 0;
    }
    
    /* Initialize task */
    task_t* task = &scheduler.tasks[idx];
    task->fn = fn;
    task->data = data;
    task->priority = priority;
    task->depth = depth;
    task->handle = scheduler.next_handle++;
    task->active = true;
    
    scheduler.task_count++;
    
    return task->handle;
}

/**
 * Execute one scheduler tick
 * Target: ≤5µs per tick
 */
int dtesn_sched_tick(void) {
    if (!scheduler.initialized) {
        return -1;
    }
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    int executed = 0;
    
    /* Execute tasks by priority */
    for (int prio = PRIORITY_CRITICAL; prio <= PRIORITY_LOW; prio++) {
        for (size_t i = 0; i < MAX_TASKS; i++) {
            task_t* task = &scheduler.tasks[i];
            
            if (!task->active || task->priority != prio) {
                continue;
            }
            
            /* Execute task */
            task->fn(task->data);
            
            /* Remove task */
            task->active = false;
            scheduler.task_count--;
            executed++;
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    /* Calculate elapsed time in microseconds */
    long elapsed_us = (end.tv_sec - start.tv_sec) * 1000000 +
                      (end.tv_nsec - start.tv_nsec) / 1000;
    
    /* Warn if exceeding target */
    if (elapsed_us > SCHED_TICK_TARGET_US) {
        /* In production, this would log */
    }
    
    return executed;
}
