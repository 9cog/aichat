/**
 * @file kernel.h
 * @brief Kernel subsystem - Bootstrap, Scheduler, Memory, Hypergraph FS
 * 
 * Echo.Kern implementation following OpenCog Cognitive Core architecture.
 * Provides real-time cognitive kernel primitives using GGML tensors.
 */

#ifndef AICHAT_KERNEL_H
#define AICHAT_KERNEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Forward declarations for GGML types */
struct ggml_context;
struct ggml_tensor;

/**
 * @defgroup Bootstrap Bootstrap and Initialization
 * @{
 */

/** Bootstrap stages following Echo.Kern architecture */
typedef enum {
    STAGE0_INIT = 0,      /**< Stage 0: Hardware/tensor init */
    STAGE1_HYPERGRAPH,    /**< Stage 1: Hypergraph FS setup */
    STAGE2_SCHEDULER,     /**< Stage 2: Scheduler init */
    STAGE3_COGNITIVE,     /**< Stage 3: Cognitive components */
} bootstrap_stage_t;

/**
 * Initialize bootstrap sequence
 * @param stage Target bootstrap stage
 * @return 0 on success, negative on error
 */
int kern_bootstrap_init(bootstrap_stage_t stage);

/** @} */

/**
 * @defgroup Scheduler Membrane-Aware Task Scheduler
 * @{
 */

/** Scheduler tick target: ≤5µs per tick */
#define SCHED_TICK_TARGET_US 5

/** Task priority levels */
typedef enum {
    PRIORITY_CRITICAL = 0,
    PRIORITY_HIGH = 1,
    PRIORITY_NORMAL = 2,
    PRIORITY_LOW = 3,
} task_priority_t;

/** Task handle */
typedef uint64_t task_handle_t;

/** Task callback function */
typedef void (*task_fn_t)(void* data);

/**
 * Initialize scheduler
 * @return 0 on success, negative on error
 */
int dtesn_sched_init(void);

/**
 * Schedule a task
 * @param fn Task function
 * @param data Task data
 * @param priority Task priority
 * @param depth Membrane depth (OEIS A000081)
 * @return Task handle or 0 on error
 */
task_handle_t dtesn_sched_task(task_fn_t fn, void* data, 
                                task_priority_t priority, uint32_t depth);

/**
 * Execute one scheduler tick
 * @return Number of tasks executed
 */
int dtesn_sched_tick(void);

/** @} */

/**
 * @defgroup Memory Memory Management with Tensor Backing
 * @{
 */

/** Memory operation target: ≤100ns per operation */
#define MEM_OP_TARGET_NS 100

/** Memory region types */
typedef enum {
    MEM_REGION_CODE = 0,
    MEM_REGION_DATA = 1,
    MEM_REGION_HEAP = 2,
    MEM_REGION_TENSOR = 3,
} mem_region_t;

/**
 * Initialize memory subsystem
 * @param heap_size Total heap size in bytes
 * @return 0 on success, negative on error
 */
int dtesn_mem_init(size_t heap_size);

/**
 * Allocate memory with tensor backing
 * @param size Size in bytes
 * @param region Memory region type
 * @return Pointer to allocated memory or NULL on error
 */
void* dtesn_mem_alloc(size_t size, mem_region_t region);

/**
 * Free memory
 * @param ptr Pointer to memory
 */
void dtesn_mem_free(void* ptr);

/** @} */

/**
 * @defgroup Sync Synchronization Primitives
 * @{
 */

/** Opaque mutex handle */
typedef struct kern_mutex*    kern_mutex_t;

/** Opaque spinlock handle */
typedef struct kern_spinlock* kern_spinlock_t;

/** Opaque semaphore handle */
typedef struct kern_sem*      kern_sem_t;

/* ── Mutex ── */

/**
 * Create and initialise a mutex
 * @param out Output handle
 * @return 0 on success, negative on error
 */
int kern_mutex_init(kern_mutex_t* out);

/**
 * Destroy mutex and free resources
 * @param m Mutex handle
 */
void kern_mutex_destroy(kern_mutex_t m);

/**
 * Lock mutex (blocking)
 * @param m Mutex handle
 * @return 0 on success, negative on error
 */
int kern_mutex_lock(kern_mutex_t m);

/**
 * Try to lock mutex (non-blocking)
 * @param m Mutex handle
 * @return 0 on success, 1 if would block, negative on error
 */
int kern_mutex_trylock(kern_mutex_t m);

/**
 * Unlock mutex
 * @param m Mutex handle
 * @return 0 on success, negative on error
 */
int kern_mutex_unlock(kern_mutex_t m);

/* ── Spinlock ── */

/**
 * Create and initialise a spinlock
 * @param out Output handle
 * @return 0 on success, negative on error
 */
int kern_spinlock_init(kern_spinlock_t* out);

/**
 * Destroy spinlock
 * @param sl Spinlock handle
 */
void kern_spinlock_destroy(kern_spinlock_t sl);

/**
 * Acquire spinlock (busy-wait)
 * @param sl Spinlock handle
 */
void kern_spinlock_lock(kern_spinlock_t sl);

/**
 * Try to acquire spinlock (non-blocking)
 * @param sl Spinlock handle
 * @return 0 on success, 1 if contended, negative on error
 */
int kern_spinlock_trylock(kern_spinlock_t sl);

/**
 * Release spinlock
 * @param sl Spinlock handle
 */
void kern_spinlock_unlock(kern_spinlock_t sl);

/* ── Semaphore ── */

/**
 * Create and initialise a counting semaphore
 * @param out           Output handle
 * @param initial_value Initial counter value
 * @return 0 on success, negative on error
 */
int kern_sem_init(kern_sem_t* out, unsigned int initial_value);

/**
 * Destroy semaphore
 * @param s Semaphore handle
 */
void kern_sem_destroy(kern_sem_t s);

/**
 * Decrement semaphore (blocking wait)
 * @param s Semaphore handle
 * @return 0 on success, negative on error
 */
int kern_sem_wait(kern_sem_t s);

/**
 * Decrement semaphore (non-blocking)
 * @param s Semaphore handle
 * @return 0 on success, 1 if would block, negative on error
 */
int kern_sem_trywait(kern_sem_t s);

/**
 * Increment semaphore
 * @param s Semaphore handle
 * @return 0 on success, negative on error
 */
int kern_sem_post(kern_sem_t s);

/**
 * Get current semaphore value
 * @param s     Semaphore handle
 * @param value Output value
 * @return 0 on success, negative on error
 */
int kern_sem_getvalue(kern_sem_t s, int* value);

/** @} */

/**
 * @defgroup Timer High-Resolution Timers
 * @{
 */

/** Opaque timer handle */
typedef struct kern_timer* kern_timer_t;

/**
 * Create a timer object
 * @return Timer handle or NULL on error
 */
kern_timer_t kern_timer_create(void);

/**
 * Destroy timer and free resources
 * @param t Timer handle
 */
void kern_timer_destroy(kern_timer_t t);

/**
 * Start (or restart) the timer
 * @param t Timer handle
 * @return 0 on success, negative on error
 */
int kern_timer_start(kern_timer_t t);

/**
 * Stop the timer
 * @param t Timer handle
 * @return 0 on success, negative on error
 */
int kern_timer_stop(kern_timer_t t);

/**
 * Elapsed time in nanoseconds (works while running or after stop)
 * @param t Timer handle
 * @return Elapsed nanoseconds, or negative on error
 */
int64_t kern_timer_elapsed_ns(const kern_timer_t t);

/**
 * Elapsed time in microseconds
 * @param t Timer handle
 * @return Elapsed microseconds, or negative on error
 */
int64_t kern_timer_elapsed_us(const kern_timer_t t);

/**
 * Elapsed time in milliseconds
 * @param t Timer handle
 * @return Elapsed milliseconds, or negative on error
 */
int64_t kern_timer_elapsed_ms(const kern_timer_t t);

/**
 * Reset timer to initial state
 * @param t Timer handle
 */
void kern_timer_reset(kern_timer_t t);

/**
 * Check if timer is currently running
 * @param t Timer handle
 * @return true if running
 */
bool kern_timer_is_running(const kern_timer_t t);

/** @} */

/**
 * @defgroup HGFS Hypergraph Filesystem
 * @{
 */

/**
 * Allocate memory as a GGML tensor node in hypergraph
 * @param size Size in bytes
 * @param depth Membrane depth (OEIS A000081)
 * @return Pointer to allocated memory or NULL on error
 */
void* hgfs_alloc(size_t size, uint32_t depth);

/**
 * Create edge in hypergraph
 * @param src Source tensor node
 * @param dst Destination tensor node
 * @param weight Edge weight
 * @return Edge handle or 0 on error
 */
uint64_t hgfs_edge(void* src, void* dst, float weight);

/**
 * Initialize hypergraph filesystem
 * @param ctx GGML context
 * @return 0 on success, negative on error
 */
int hgfs_init(struct ggml_context* ctx);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* AICHAT_KERNEL_H */
