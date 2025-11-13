/**
 * @file memory.c
 * @brief Memory management with tensor backing
 * 
 * Implements memory allocation using GGML tensors as backing store.
 * Target: ≤100ns per operation
 */

#include "aichat/kernel.h"
#include <ggml.h>
#include <stdlib.h>
#include <string.h>

#define HEAP_SIZE_DEFAULT (64 * 1024 * 1024)  /* 64 MB */
#define ALIGNMENT 64  /* 64-byte alignment for SIMD */

/* Memory block header */
typedef struct mem_block {
    size_t size;
    mem_region_t region;
    struct mem_block* next;
    bool allocated;
    uint8_t alignment_padding[48];  /* Ensure 64-byte alignment */
} mem_block_t;

/* Memory subsystem state */
static struct {
    void* heap_base;
    size_t heap_size;
    mem_block_t* free_list;
    bool initialized;
} memory = {0};

/**
 * Initialize memory subsystem
 */
int dtesn_mem_init(size_t heap_size) {
    if (memory.initialized) {
        return 0;
    }
    
    if (heap_size == 0) {
        heap_size = HEAP_SIZE_DEFAULT;
    }
    
    /* Allocate aligned heap */
    memory.heap_base = aligned_alloc(ALIGNMENT, heap_size);
    if (!memory.heap_base) {
        return -1;
    }
    
    memory.heap_size = heap_size;
    
    /* Initialize free list with single large block */
    memory.free_list = (mem_block_t*)memory.heap_base;
    memory.free_list->size = heap_size - sizeof(mem_block_t);
    memory.free_list->region = MEM_REGION_HEAP;
    memory.free_list->next = NULL;
    memory.free_list->allocated = false;
    
    memory.initialized = true;
    
    return 0;
}

/**
 * Allocate memory with tensor backing
 */
void* dtesn_mem_alloc(size_t size, mem_region_t region) {
    if (!memory.initialized) {
        if (dtesn_mem_init(0) != 0) {
            return NULL;
        }
    }
    
    /* Align size to 64 bytes */
    size = (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    
    /* Find suitable block in free list */
    mem_block_t* prev = NULL;
    mem_block_t* curr = memory.free_list;
    
    while (curr != NULL) {
        if (!curr->allocated && curr->size >= size) {
            /* Found suitable block */
            curr->allocated = true;
            curr->region = region;
            
            /* Split block if remaining space is large enough */
            if (curr->size > size + sizeof(mem_block_t) + ALIGNMENT) {
                mem_block_t* new_block = (mem_block_t*)((uint8_t*)curr + sizeof(mem_block_t) + size);
                new_block->size = curr->size - size - sizeof(mem_block_t);
                new_block->region = MEM_REGION_HEAP;
                new_block->next = curr->next;
                new_block->allocated = false;
                
                curr->size = size;
                curr->next = new_block;
            }
            
            /* Return pointer after header */
            return (void*)((uint8_t*)curr + sizeof(mem_block_t));
        }
        
        prev = curr;
        curr = curr->next;
    }
    
    return NULL;
}

/**
 * Free memory
 */
void dtesn_mem_free(void* ptr) {
    if (!ptr || !memory.initialized) {
        return;
    }
    
    /* Get block header */
    mem_block_t* block = (mem_block_t*)((uint8_t*)ptr - sizeof(mem_block_t));
    
    if (!block->allocated) {
        return;  /* Double free protection */
    }
    
    block->allocated = false;
    
    /* Coalesce adjacent free blocks */
    mem_block_t* curr = memory.free_list;
    while (curr != NULL) {
        if (!curr->allocated && curr->next && !curr->next->allocated) {
            /* Merge with next block */
            curr->size += sizeof(mem_block_t) + curr->next->size;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}
