/**
 * @file test_rag.cpp
 * @brief RAG subsystem tests
 */

#include "aichat/rag.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

/* ── Helpers ────────────────────────────────────────────────────────────── */

static const char* SAMPLE_TEXT_A =
    "The GGML library provides tensor operations for machine learning. "
    "It supports CPU and GPU backends including CUDA and Metal. "
    "GGML is used extensively in llama.cpp for local LLM inference.";

static const char* SAMPLE_TEXT_B =
    "CMake is a cross-platform build system generator. "
    "It uses CMakeLists.txt files to describe the build. "
    "FetchContent allows automatic downloading of dependencies at configure time.";

static const char* SAMPLE_TEXT_C =
    "AtomSpace is a hypergraph knowledge store used in OpenCog. "
    "Atoms represent concepts and their relationships. "
    "ECAN manages attention allocation across the AtomSpace graph.";

/* ── Tests ──────────────────────────────────────────────────────────────── */

static int test_create(void) {
    printf("Testing rag_create / rag_destroy...\n");

    rag_store_t store = rag_create(NULL);
    assert(store != NULL);
    assert(rag_chunk_count(store) == 0);

    rag_destroy(store);

    /* Custom config */
    rag_config_t cfg = rag_default_config();
    cfg.chunk_size    = 64;
    cfg.chunk_overlap = 16;
    cfg.top_k         = 2;

    store = rag_create(&cfg);
    assert(store != NULL);
    rag_destroy(store);

    printf("  PASS: rag_create / rag_destroy\n");
    return 0;
}

static int test_add_text(void) {
    printf("Testing rag_add_text...\n");

    rag_store_t store = rag_create(NULL);
    assert(store != NULL);

    int n = rag_add_text(store, SAMPLE_TEXT_A, "ggml");
    assert(n > 0);
    assert(rag_chunk_count(store) == (size_t)n);

    int n2 = rag_add_text(store, SAMPLE_TEXT_B, "cmake");
    assert(n2 > 0);
    assert(rag_chunk_count(store) == (size_t)(n + n2));

    /* NULL safety */
    assert(rag_add_text(NULL, SAMPLE_TEXT_A, "src") < 0);
    assert(rag_add_text(store, NULL, "src") < 0);

    rag_destroy(store);
    printf("  PASS: rag_add_text\n");
    return 0;
}

static int test_query(void) {
    printf("Testing rag_query...\n");

    rag_store_t store = rag_create(NULL);
    assert(store != NULL);

    rag_add_text(store, SAMPLE_TEXT_A, "ggml");
    rag_add_text(store, SAMPLE_TEXT_B, "cmake");
    rag_add_text(store, SAMPLE_TEXT_C, "opencog");

    /* Query about tensors – should score SAMPLE_TEXT_A highest */
    rag_chunk_t results[4];
    int n = rag_query(store, "tensor operations machine learning", results, 3);
    assert(n > 0);

    /* All scores should be in [0,1] */
    for (int i = 0; i < n; i++) {
        assert(results[i].score >= 0.0f && results[i].score <= 1.0f);
        assert(results[i].text   != NULL);
        assert(results[i].source != NULL);
    }

    /* Results should be sorted descending by score */
    for (int i = 0; i + 1 < n; i++) {
        assert(results[i].score >= results[i + 1].score);
    }

    /* NULL / empty-store safety */
    assert(rag_query(NULL, "query", results, 3) < 0);

    rag_destroy(store);
    printf("  PASS: rag_query\n");
    return 0;
}

static int test_build_context(void) {
    printf("Testing rag_build_context...\n");

    rag_store_t store = rag_create(NULL);
    assert(store != NULL);

    rag_add_text(store, SAMPLE_TEXT_A, "ggml");
    rag_add_text(store, SAMPLE_TEXT_B, "cmake");
    rag_add_text(store, SAMPLE_TEXT_C, "opencog");

    char* ctx = rag_build_context(store, "AtomSpace knowledge graph", 2);
    assert(ctx != NULL);

    /* Must contain the delimiters */
    assert(strstr(ctx, "--- Context ---") != NULL);
    assert(strstr(ctx, "--- End Context ---") != NULL);
    /* Must reference at least one source */
    assert(strstr(ctx, "[source:") != NULL);

    free(ctx);

    /* top_k=0 should use default (4) */
    ctx = rag_build_context(store, "llama inference", 0);
    assert(ctx != NULL);
    free(ctx);

    /* NULL safety */
    assert(rag_build_context(NULL, "query", 2) == NULL);

    rag_destroy(store);
    printf("  PASS: rag_build_context\n");
    return 0;
}

static int test_add_file(void) {
    printf("Testing rag_add_file...\n");

    rag_store_t store = rag_create(NULL);
    assert(store != NULL);

    /* Non-existent file */
    int n = rag_add_file(store, "/tmp/aichat_rag_nonexistent_file_xyz.txt");
    assert(n < 0);

    /* Write a temp file and ingest it */
    const char* tmppath = "/tmp/aichat_rag_test.txt";
    FILE* f = fopen(tmppath, "w");
    assert(f != NULL);
    fprintf(f, "Temporary test file for RAG unit test. "
               "Contains several words about tensors and embeddings. "
               "The vector store should chunk this and retrieve it.");
    fclose(f);

    n = rag_add_file(store, tmppath);
    assert(n > 0);

    remove(tmppath);
    rag_destroy(store);
    printf("  PASS: rag_add_file\n");
    return 0;
}

/* ── main ───────────────────────────────────────────────────────────────── */

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <test>\n", argv[0]);
        fprintf(stderr, "Tests: create add_text query build_context add_file\n");
        return 1;
    }

    int ret = 0;

    if (strcmp(argv[1], "create") == 0) {
        ret = test_create();
    } else if (strcmp(argv[1], "add_text") == 0) {
        ret = test_add_text();
    } else if (strcmp(argv[1], "query") == 0) {
        ret = test_query();
    } else if (strcmp(argv[1], "build_context") == 0) {
        ret = test_build_context();
    } else if (strcmp(argv[1], "add_file") == 0) {
        ret = test_add_file();
    } else {
        fprintf(stderr, "Unknown test: %s\n", argv[1]);
        return 1;
    }

    return ret;
}
