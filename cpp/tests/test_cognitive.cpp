/**
 * @file test_cognitive.cpp
 * @brief Cognitive subsystem tests
 */

#include "aichat/cognitive.h"
#include "aichat/kernel.h"
#include <cstdio>
#include <cstring>
#include <cassert>

/* Test AtomSpace */
static int test_atomspace(void) {
    printf("Testing AtomSpace...\n");
    
    /* Bootstrap first */
    kern_bootstrap_init(STAGE3_COGNITIVE);
    
    /* Create atoms */
    atom_handle_t a1 = cog_atom_alloc(ATOM_CONCEPT, "cat");
    assert(a1 != 0);
    
    atom_handle_t a2 = cog_atom_alloc(ATOM_CONCEPT, "animal");
    assert(a2 != 0);
    
    /* Create link */
    atom_handle_t atoms[2] = {a1, a2};
    atom_handle_t link = cog_link_create(ATOM_LINK, atoms, 2);
    assert(link != 0);
    
    printf("  PASS: AtomSpace operations\n");
    return 0;
}

/* Test ECAN */
static int test_ecan(void) {
    printf("Testing ECAN...\n");
    
    kern_bootstrap_init(STAGE3_COGNITIVE);
    
    atom_handle_t a = cog_atom_alloc(ATOM_CONCEPT, "test");
    assert(a != 0);
    
    attention_value_t av = ecan_get_attention(a);
    assert(av.sti >= 0.0f && av.sti <= 1.0f);
    
    int updated = ecan_update();
    assert(updated >= 0);
    
    printf("  PASS: ECAN attention allocation\n");
    return 0;
}

/* Test PLN */
static int test_pln(void) {
    printf("Testing PLN...\n");
    
    kern_bootstrap_init(STAGE3_COGNITIVE);
    
    atom_handle_t query = cog_atom_alloc(ATOM_PREDICATE, "likes");
    assert(query != 0);
    
    atom_handle_t context[2];
    context[0] = cog_atom_alloc(ATOM_CONCEPT, "john");
    context[1] = cog_atom_alloc(ATOM_CONCEPT, "pizza");
    
    truth_value_t tv = pln_eval_tensor(query, context, 2);
    assert(tv.strength >= 0.0f && tv.strength <= 1.0f);
    assert(tv.confidence >= 0.0f && tv.confidence <= 1.0f);
    
    printf("  PASS: PLN inference\n");
    return 0;
}

/* Test ESN */
static int test_esn(void) {
    printf("Testing ESN...\n");
    
    esn_reservoir_t res = esn_create(10, 100, 5, 0.95f);
    assert(res != nullptr);
    
    esn_destroy(res);
    
    printf("  PASS: ESN reservoir\n");
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <test>\n", argv[0]);
        return 1;
    }
    
    int ret = 0;
    
    if (strcmp(argv[1], "atomspace") == 0) {
        ret = test_atomspace();
    } else if (strcmp(argv[1], "ecan") == 0) {
        ret = test_ecan();
    } else if (strcmp(argv[1], "pln") == 0) {
        ret = test_pln();
    } else if (strcmp(argv[1], "esn") == 0) {
        ret = test_esn();
    } else {
        fprintf(stderr, "Unknown test: %s\n", argv[1]);
        return 1;
    }
    
    return ret;
}
