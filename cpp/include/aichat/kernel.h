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
