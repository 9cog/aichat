/**
 * @file bootstrap.c
 * @brief Bootstrap initialization - Stage 0-3
 * 
 * Implements Echo.Kern bootstrap sequence:
 * - Stage 0: Hardware/tensor initialization
 * - Stage 1: Hypergraph filesystem setup
 * - Stage 2: Scheduler initialization
 * - Stage 3: Cognitive components
 */

#include "aichat/kernel.h"
#include <ggml.h>
#include <stdio.h>
#include <stdlib.h>

/* Global GGML context */
static struct ggml_context* g_ggml_ctx = NULL;

/* Bootstrap state */
static bootstrap_stage_t g_current_stage = STAGE0_INIT;

/**
 * Stage 0: Initialize hardware and tensor context
 */
static int bootstrap_stage0(void) {
    struct ggml_init_params params = {
        .mem_size = 128 * 1024 * 1024,  /* 128 MB */
        .mem_buffer = NULL,
        .no_alloc = false,
    };
    
    g_ggml_ctx = ggml_init(params);
    if (!g_ggml_ctx) {
        fprintf(stderr, "Failed to initialize GGML context\n");
        return -1;
    }
    
    printf("[STAGE0] GGML context initialized (128 MB)\n");
    return 0;
}

/**
 * Stage 1: Initialize hypergraph filesystem
 */
static int bootstrap_stage1(void) {
    if (!g_ggml_ctx) {
        fprintf(stderr, "GGML context not initialized\n");
        return -1;
    }
    
    if (hgfs_init(g_ggml_ctx) != 0) {
        fprintf(stderr, "Failed to initialize hypergraph filesystem\n");
        return -1;
    }
    
    printf("[STAGE1] Hypergraph filesystem initialized\n");
    return 0;
}

/**
 * Stage 2: Initialize scheduler
 */
static int bootstrap_stage2(void) {
    if (dtesn_sched_init() != 0) {
        fprintf(stderr, "Failed to initialize scheduler\n");
        return -1;
    }
    
    printf("[STAGE2] Scheduler initialized (target: %d µs/tick)\n", SCHED_TICK_TARGET_US);
    return 0;
}

/**
 * Stage 3: Initialize cognitive components
 */
static int bootstrap_stage3(void) {
    if (!g_ggml_ctx) {
        fprintf(stderr, "GGML context not initialized\n");
        return -1;
    }
    
    /* Initialize AtomSpace */
    if (atomspace_init(g_ggml_ctx) != 0) {
        fprintf(stderr, "Failed to initialize AtomSpace\n");
        return -1;
    }
    
    /* Initialize ECAN */
    if (ecan_init(g_ggml_ctx) != 0) {
        fprintf(stderr, "Failed to initialize ECAN\n");
        return -1;
    }
    
    /* Initialize PLN */
    if (pln_init(g_ggml_ctx) != 0) {
        fprintf(stderr, "Failed to initialize PLN\n");
        return -1;
    }
    
    printf("[STAGE3] Cognitive components initialized\n");
    return 0;
}

/**
 * Initialize bootstrap sequence up to specified stage
 */
int kern_bootstrap_init(bootstrap_stage_t target_stage) {
    int ret = 0;
    
    /* Execute stages sequentially */
    if (g_current_stage <= STAGE0_INIT && target_stage >= STAGE0_INIT) {
        ret = bootstrap_stage0();
        if (ret != 0) return ret;
        g_current_stage = STAGE0_INIT;
    }
    
    if (g_current_stage <= STAGE1_HYPERGRAPH && target_stage >= STAGE1_HYPERGRAPH) {
        ret = bootstrap_stage1();
        if (ret != 0) return ret;
        g_current_stage = STAGE1_HYPERGRAPH;
    }
    
    if (g_current_stage <= STAGE2_SCHEDULER && target_stage >= STAGE2_SCHEDULER) {
        ret = bootstrap_stage2();
        if (ret != 0) return ret;
        g_current_stage = STAGE2_SCHEDULER;
    }
    
    if (g_current_stage <= STAGE3_COGNITIVE && target_stage >= STAGE3_COGNITIVE) {
        ret = bootstrap_stage3();
        if (ret != 0) return ret;
        g_current_stage = STAGE3_COGNITIVE;
    }
    
    printf("[BOOTSTRAP] Completed up to stage %d\n", target_stage);
    return 0;
}

/**
 * Get global GGML context
 */
struct ggml_context* kern_get_ggml_context(void) {
    return g_ggml_ctx;
}
