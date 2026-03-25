/**
 * @file cogloop.h
 * @brief CognitiveLoop - full OpenCog cognitive cycle orchestration
 *
 * Orchestrates AtomSpace, ECAN, PLN, and ESN in a unified cognitive loop
 * that processes inputs, updates attention, performs inference, and generates
 * responses via reservoir computing.
 */

#ifndef AICHAT_COGLOOP_H
#define AICHAT_COGLOOP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "aichat/cognitive.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * @defgroup CognitiveLoop Cognitive Loop Orchestration
 * @{
 */

/** Cognitive loop configuration */
typedef struct {
    size_t  max_atoms;        /**< Maximum atoms in AtomSpace */
    size_t  ecan_cycles;      /**< ECAN update cycles per loop tick */
    size_t  pln_depth;        /**< PLN inference depth */
    size_t  esn_input_size;   /**< ESN reservoir input dimension */
    size_t  esn_reservoir;    /**< ESN reservoir size */
    float   esn_spectral;     /**< ESN spectral radius */
    float   sti_decay;        /**< STI decay factor per cycle (0..1) */
    float   sti_threshold;    /**< Minimum STI for PLN consideration */
} cogloop_config_t;

/** Cognitive loop handle */
typedef struct cogloop* cogloop_t;

/** Default cognitive loop configuration */
cogloop_config_t cogloop_default_config(void);

/**
 * Create a cognitive loop instance
 * @param cfg Loop configuration
 * @return Loop handle or NULL on error
 */
cogloop_t cogloop_create(const cogloop_config_t* cfg);

/**
 * Destroy a cognitive loop instance and free resources
 * @param loop Loop handle
 */
void cogloop_destroy(cogloop_t loop);

/**
 * Process one cognitive cycle
 *
 * Runs the full OpenCog cycle:
 *   1. Ingest input atoms into AtomSpace
 *   2. ECAN attention spreading
 *   3. PLN inference on high-attention atoms
 *   4. ESN reservoir update
 *
 * @param loop    Loop handle
 * @param input   Input atoms (can be NULL if n_input == 0)
 * @param n_input Number of input atoms
 * @return Number of inferences produced, or negative on error
 */
int cogloop_cycle(cogloop_t loop, const atom_handle_t* input, size_t n_input);

/**
 * Query the most salient atoms (highest STI) after a cycle
 * @param loop       Loop handle
 * @param out        Output buffer for atom handles
 * @param max_atoms  Maximum atoms to return
 * @return Number of atoms returned, or negative on error
 */
int cogloop_top_atoms(cogloop_t loop, atom_handle_t* out, size_t max_atoms);

/**
 * Get PLN truth value for a query atom using the current loop state
 * @param loop    Loop handle
 * @param query   Query atom handle
 * @return Truth value for the query
 */
truth_value_t cogloop_query(cogloop_t loop, atom_handle_t query);

/**
 * Get cycle count (number of times cogloop_cycle has been called)
 * @param loop Loop handle
 * @return Cycle count
 */
uint64_t cogloop_cycle_count(const cogloop_t loop);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* AICHAT_COGLOOP_H */
