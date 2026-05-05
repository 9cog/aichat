/**
 * @file timer.c
 * @brief High-resolution kernel timers
 *
 * Provides monotonic wall-clock timers built on CLOCK_MONOTONIC.
 * Designed for measuring short intervals (nanosecond resolution) so that
 * the scheduler and memory allocator performance targets can be verified.
 *
 * A timer can be:
 *   - Created  : kern_timer_create()
 *   - Started  : kern_timer_start()     → records start timestamp
 *   - Stopped  : kern_timer_stop()      → records stop timestamp
 *   - Queried  : kern_timer_elapsed_ns() → returns elapsed nanoseconds
 *   - Reset    : kern_timer_reset()      → clears start/stop
 *   - Destroyed: kern_timer_destroy()
 */

#include "aichat/kernel.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ── Internal types ─────────────────────────────────────────────────────── */

struct kern_timer {
    struct timespec start;
    struct timespec stop;
    bool            running;
    bool            has_start;
    bool            has_stop;
};

/* ── Public API ─────────────────────────────────────────────────────────── */

kern_timer_t kern_timer_create(void) {
    kern_timer_t t = (kern_timer_t)malloc(sizeof(struct kern_timer));
    if (!t) {
        return NULL;
    }
    memset(t, 0, sizeof(struct kern_timer));
    return t;
}

void kern_timer_destroy(kern_timer_t t) {
    free(t);
}

int kern_timer_start(kern_timer_t t) {
    if (!t) {
        return -1;
    }
    if (clock_gettime(CLOCK_MONOTONIC, &t->start) != 0) {
        return -1;
    }
    t->has_start = true;
    t->has_stop  = false;
    t->running   = true;
    return 0;
}

int kern_timer_stop(kern_timer_t t) {
    if (!t || !t->running) {
        return -1;
    }
    if (clock_gettime(CLOCK_MONOTONIC, &t->stop) != 0) {
        return -1;
    }
    t->has_stop = true;
    t->running  = false;
    return 0;
}

int64_t kern_timer_elapsed_ns(const kern_timer_t t) {
    if (!t || !t->has_start) {
        return -1;
    }

    struct timespec end;

    if (t->has_stop) {
        end = t->stop;
    } else if (t->running) {
        /* Snapshot current time for a live read */
        if (clock_gettime(CLOCK_MONOTONIC, &end) != 0) {
            return -1;
        }
    } else {
        return -1;
    }

    int64_t sec_ns  = (int64_t)(end.tv_sec  - t->start.tv_sec)  * 1000000000LL;
    int64_t nsec_ns = (int64_t)(end.tv_nsec - t->start.tv_nsec);
    return sec_ns + nsec_ns;
}

int64_t kern_timer_elapsed_us(const kern_timer_t t) {
    int64_t ns = kern_timer_elapsed_ns(t);
    return ns < 0 ? ns : ns / 1000;
}

int64_t kern_timer_elapsed_ms(const kern_timer_t t) {
    int64_t ns = kern_timer_elapsed_ns(t);
    return ns < 0 ? ns : ns / 1000000;
}

void kern_timer_reset(kern_timer_t t) {
    if (!t) {
        return;
    }
    memset(t, 0, sizeof(struct kern_timer));
}

bool kern_timer_is_running(const kern_timer_t t) {
    return t ? t->running : false;
}
