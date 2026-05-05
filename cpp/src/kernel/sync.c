/**
 * @file sync.c
 * @brief Kernel synchronization primitives - mutex, spinlock, semaphore
 *
 * Implements POSIX-backed synchronization primitives for use within the
 * cognitive kernel.  All operations are designed to be lightweight so that
 * they do not exceed the scheduler's ≤5µs tick budget when called from
 * task callbacks.
 */

#include "aichat/kernel.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>

/* ── Mutex ─────────────────────────────────────────────────────────────── */

struct kern_mutex {
    pthread_mutex_t pmtx;
    bool            initialized;
};

int kern_mutex_init(kern_mutex_t* out) {
    if (!out) {
        return -1;
    }

    kern_mutex_t m = (kern_mutex_t)malloc(sizeof(struct kern_mutex));
    if (!m) {
        return -1;
    }

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);

    if (pthread_mutex_init(&m->pmtx, &attr) != 0) {
        pthread_mutexattr_destroy(&attr);
        free(m);
        return -1;
    }

    pthread_mutexattr_destroy(&attr);
    m->initialized = true;
    *out = m;
    return 0;
}

void kern_mutex_destroy(kern_mutex_t m) {
    if (!m) {
        return;
    }
    pthread_mutex_destroy(&m->pmtx);
    m->initialized = false;
    free(m);
}

int kern_mutex_lock(kern_mutex_t m) {
    if (!m || !m->initialized) {
        return -1;
    }
    return pthread_mutex_lock(&m->pmtx) == 0 ? 0 : -1;
}

int kern_mutex_trylock(kern_mutex_t m) {
    if (!m || !m->initialized) {
        return -1;
    }
    int r = pthread_mutex_trylock(&m->pmtx);
    if (r == 0) return 0;
    if (r == EBUSY) return 1;   /* would block */
    return -1;
}

int kern_mutex_unlock(kern_mutex_t m) {
    if (!m || !m->initialized) {
        return -1;
    }
    return pthread_mutex_unlock(&m->pmtx) == 0 ? 0 : -1;
}

/* ── Spinlock ───────────────────────────────────────────────────────────── */

struct kern_spinlock {
    atomic_flag flag;
};

int kern_spinlock_init(kern_spinlock_t* out) {
    if (!out) {
        return -1;
    }

    kern_spinlock_t sl = (kern_spinlock_t)malloc(sizeof(struct kern_spinlock));
    if (!sl) {
        return -1;
    }

    atomic_flag_clear(&sl->flag);
    *out = sl;
    return 0;
}

void kern_spinlock_destroy(kern_spinlock_t sl) {
    free(sl);
}

void kern_spinlock_lock(kern_spinlock_t sl) {
    if (!sl) {
        return;
    }
    while (atomic_flag_test_and_set_explicit(&sl->flag, memory_order_acquire)) {
        /* spin */
    }
}

int kern_spinlock_trylock(kern_spinlock_t sl) {
    if (!sl) {
        return -1;
    }
    return atomic_flag_test_and_set_explicit(&sl->flag, memory_order_acquire) ? 1 : 0;
}

void kern_spinlock_unlock(kern_spinlock_t sl) {
    if (!sl) {
        return;
    }
    atomic_flag_clear_explicit(&sl->flag, memory_order_release);
}

/* ── Semaphore ──────────────────────────────────────────────────────────── */

struct kern_sem {
    sem_t        sem;
    bool         initialized;
};

int kern_sem_init(kern_sem_t* out, unsigned int initial_value) {
    if (!out) {
        return -1;
    }

    kern_sem_t s = (kern_sem_t)malloc(sizeof(struct kern_sem));
    if (!s) {
        return -1;
    }

    if (sem_init(&s->sem, 0, initial_value) != 0) {
        free(s);
        return -1;
    }

    s->initialized = true;
    *out = s;
    return 0;
}

void kern_sem_destroy(kern_sem_t s) {
    if (!s) {
        return;
    }
    sem_destroy(&s->sem);
    s->initialized = false;
    free(s);
}

int kern_sem_wait(kern_sem_t s) {
    if (!s || !s->initialized) {
        return -1;
    }
    return sem_wait(&s->sem) == 0 ? 0 : -1;
}

int kern_sem_trywait(kern_sem_t s) {
    if (!s || !s->initialized) {
        return -1;
    }
    int r = sem_trywait(&s->sem);
    if (r == 0) return 0;
    if (errno == EAGAIN) return 1;  /* would block */
    return -1;
}

int kern_sem_post(kern_sem_t s) {
    if (!s || !s->initialized) {
        return -1;
    }
    return sem_post(&s->sem) == 0 ? 0 : -1;
}

int kern_sem_getvalue(kern_sem_t s, int* value) {
    if (!s || !s->initialized || !value) {
        return -1;
    }
    return sem_getvalue(&s->sem, value) == 0 ? 0 : -1;
}
