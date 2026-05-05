/**
 * @file test_cogloop.cpp
 * @brief CognitiveLoop unit tests
 */

#include "aichat/cogloop.h"
#include "aichat/cognitive.h"
#include "aichat/kernel.h"
#include <cstdio>
#include <cstring>
#include <cassert>

/* Test cogloop creation and destruction */
static int test_create(void) {
    printf("Testing CognitiveLoop create/destroy...\n");

    kern_bootstrap_init(STAGE3_COGNITIVE);

    cogloop_config_t cfg = cogloop_default_config();
    cogloop_t loop = cogloop_create(&cfg);
    assert(loop != nullptr);

    assert(cogloop_cycle_count(loop) == 0);

    cogloop_destroy(loop);

    /* NULL safety */
    cogloop_destroy(nullptr);
    assert(cogloop_cycle_count(nullptr) == 0);

    printf("  PASS: CognitiveLoop create/destroy\n");
    return 0;
}

/* Test full cognitive cycle */
static int test_cycle(void) {
    printf("Testing CognitiveLoop cycle...\n");

    kern_bootstrap_init(STAGE3_COGNITIVE);

    cogloop_config_t cfg = cogloop_default_config();
    cogloop_t loop = cogloop_create(&cfg);
    assert(loop != nullptr);

    /* Create some atoms */
    atom_handle_t a1 = cog_atom_alloc(ATOM_CONCEPT, "dog");
    atom_handle_t a2 = cog_atom_alloc(ATOM_CONCEPT, "animal");
    atom_handle_t a3 = cog_atom_alloc(ATOM_PREDICATE, "is_a");
    assert(a1 != 0 && a2 != 0 && a3 != 0);

    atom_handle_t inputs[3] = {a1, a2, a3};

    /* Run a cycle with inputs */
    int inferences = cogloop_cycle(loop, inputs, 3);
    assert(inferences >= 0);
    assert(cogloop_cycle_count(loop) == 1);

    /* Run another cycle with no new inputs */
    int inferences2 = cogloop_cycle(loop, nullptr, 0);
    assert(inferences2 >= 0);
    assert(cogloop_cycle_count(loop) == 2);

    /* Top atoms should return something */
    atom_handle_t top[10];
    int n = cogloop_top_atoms(loop, top, 10);
    assert(n >= 0);

    cogloop_destroy(loop);

    printf("  PASS: CognitiveLoop cycle (%d + %d inferences)\n",
           inferences, inferences2);
    return 0;
}

/* Test cogloop query */
static int test_query(void) {
    printf("Testing CognitiveLoop query...\n");

    kern_bootstrap_init(STAGE3_COGNITIVE);

    cogloop_config_t cfg = cogloop_default_config();
    cogloop_t loop = cogloop_create(&cfg);
    assert(loop != nullptr);

    atom_handle_t subject = cog_atom_alloc(ATOM_CONCEPT, "bird");
    atom_handle_t pred    = cog_atom_alloc(ATOM_PREDICATE, "can_fly");
    assert(subject != 0 && pred != 0);

    atom_handle_t inputs[2] = {subject, pred};
    cogloop_cycle(loop, inputs, 2);
    cogloop_cycle(loop, inputs, 2); /* second pass boosts STI */

    truth_value_t tv = cogloop_query(loop, pred);
    assert(tv.strength >= 0.0f && tv.strength <= 1.0f);
    assert(tv.confidence >= 0.0f && tv.confidence <= 1.0f);

    /* NULL safety */
    truth_value_t tv_null = cogloop_query(nullptr, pred);
    assert(tv_null.strength == 0.0f && tv_null.confidence == 0.0f);

    cogloop_destroy(loop);

    printf("  PASS: CognitiveLoop query (strength=%.3f confidence=%.3f)\n",
           tv.strength, tv.confidence);
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <test>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "create") == 0) return test_create();
    if (strcmp(argv[1], "cycle")  == 0) return test_cycle();
    if (strcmp(argv[1], "query")  == 0) return test_query();

    fprintf(stderr, "Unknown test: %s\n", argv[1]);
    return 1;
}
