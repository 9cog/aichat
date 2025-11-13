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
    } else {
        fprintf(stderr, "Unknown test: %s\n", argv[1]);
        return 1;
    }
    
    return ret;
}
