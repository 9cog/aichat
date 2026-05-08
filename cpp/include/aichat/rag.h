/**
 * @file rag.h
 * @brief Retrieval-Augmented Generation (RAG) subsystem
 *
 * Provides document loading, chunking, indexing, and semantic retrieval
 * for augmenting LLM prompts with relevant context.
 *
 * The vector store uses a hash-projection feature space (1024-dimensional)
 * with normalized term-frequency vectors and cosine similarity retrieval.
 * No external dependencies beyond the C/C++ standard library are required.
 *
 * Typical usage:
 * @code
 *   rag_store_t store = rag_create(NULL);
 *   rag_add_file(store, "docs/manual.txt");
 *   rag_add_text(store, "Extra facts...", "manual");
 *
 *   char* ctx = rag_build_context(store, "How do I configure X?", 4);
 *   // prepend ctx to system prompt before calling llm_chat_completion
 *   free(ctx);
 *   rag_destroy(store);
 * @endcode
 */

#ifndef AICHAT_RAG_H
#define AICHAT_RAG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

/**
 * @defgroup RAG Retrieval-Augmented Generation
 * @{
 */

/** Feature-space dimensionality for chunk embeddings */
#define RAG_FEATURE_DIM 1024

/** RAG store handle (opaque) */
typedef struct rag_store* rag_store_t;

/** RAG configuration */
typedef struct {
    size_t chunk_size;    /**< Characters per chunk (default: 512) */
    size_t chunk_overlap; /**< Overlap between consecutive chunks in chars (default: 128) */
    size_t max_chunks;    /**< Maximum total chunks to store (0 = no limit) */
    size_t top_k;         /**< Default retrieval count used by rag_build_context (default: 4) */
} rag_config_t;

/** Single retrieved chunk returned by rag_query() */
typedef struct {
    const char* text;   /**< Chunk text (valid until store is modified or destroyed) */
    const char* source; /**< Source label supplied when the text was added */
    float       score;  /**< Cosine similarity score in [0, 1] */
} rag_chunk_t;

/**
 * Return a populated rag_config_t with sensible defaults.
 * @return Default configuration
 */
rag_config_t rag_default_config(void);

/**
 * Create a new RAG store.
 * @param cfg  Configuration, or NULL to use defaults
 * @return Store handle, or NULL on allocation failure
 */
rag_store_t rag_create(const rag_config_t* cfg);

/**
 * Destroy a RAG store and release all memory.
 * @param store Store handle (safe to call with NULL)
 */
void rag_destroy(rag_store_t store);

/**
 * Ingest raw text into the store by splitting it into overlapping chunks.
 * @param store  Store handle
 * @param text   UTF-8 text to ingest
 * @param source Source label attached to every resulting chunk (e.g. file path)
 * @return Number of new chunks added, or a negative error code
 */
int rag_add_text(rag_store_t store, const char* text, const char* source);

/**
 * Load a plain-text file and ingest its contents.
 * @param store Store handle
 * @param path  File system path to read
 * @return Number of new chunks added, or a negative error code
 */
int rag_add_file(rag_store_t store, const char* path);

/**
 * Retrieve the top-k most relevant chunks for a query string.
 *
 * Results are written into the caller-allocated @p results array and
 * sorted by descending similarity score.
 *
 * @param store       Store handle
 * @param query       Query text
 * @param results     Output array (caller-allocated, at least max_results elements)
 * @param max_results Maximum number of results to return
 * @return Number of results filled (0..max_results), or a negative error code
 */
int rag_query(rag_store_t store, const char* query,
              rag_chunk_t* results, size_t max_results);

/**
 * Build a formatted context string from the top-k most relevant chunks.
 *
 * The returned string has the format:
 * @verbatim
 * --- Context ---
 * [source: <label>]
 * <chunk text>
 *
 * [source: <label>]
 * <chunk text>
 * --- End Context ---
 * @endverbatim
 *
 * The caller is responsible for freeing the returned string with free().
 *
 * @param store Store handle
 * @param query Query text
 * @param top_k Chunks to include; 0 uses the config default
 * @return Heap-allocated context string, or NULL on error
 */
char* rag_build_context(rag_store_t store, const char* query, size_t top_k);

/**
 * Return the total number of chunks currently held in the store.
 * @param store Store handle
 * @return Chunk count
 */
size_t rag_chunk_count(const rag_store_t store);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* AICHAT_RAG_H */
