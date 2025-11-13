/**
 * @file cognitive.h
 * @brief Cognitive subsystem - AtomSpace, ECAN, PLN, ESN
 * 
 * OpenCog cognitive components implemented as GGML tensor operations.
 */

#ifndef AICHAT_COGNITIVE_H
#define AICHAT_COGNITIVE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/* Forward declarations */
struct ggml_context;
struct ggml_tensor;

/**
 * @defgroup AtomSpace Hypergraph AtomSpace
 * @{
 */

/** Atom handle */
typedef uint64_t atom_handle_t;

/** Atom types */
typedef enum {
    ATOM_NODE = 0,
    ATOM_LINK = 1,
    ATOM_CONCEPT = 2,
    ATOM_PREDICATE = 3,
} atom_type_t;

/**
 * Allocate a new atom
 * @param type Atom type
 * @param name Atom name (can be NULL)
 * @return Atom handle or 0 on error
 */
atom_handle_t cog_atom_alloc(atom_type_t type, const char* name);

/**
 * Create link between atoms
 * @param type Link type
 * @param atoms Array of atom handles
 * @param n_atoms Number of atoms
 * @return Link atom handle or 0 on error
 */
atom_handle_t cog_link_create(atom_type_t type, atom_handle_t* atoms, size_t n_atoms);

/**
 * Initialize AtomSpace
 * @param ctx GGML context
 * @return 0 on success, negative on error
 */
int atomspace_init(struct ggml_context* ctx);

/** @} */

/**
 * @defgroup ECAN Economic Attention Networks
 * @{
 */

/** Attention value structure */
typedef struct {
    float sti;  /**< Short-term importance */
    float lti;  /**< Long-term importance */
    float vlti; /**< Very long-term importance */
} attention_value_t;

/**
 * Get attention value for atom
 * @param atom Atom handle
 * @return Attention value
 */
attention_value_t ecan_get_attention(atom_handle_t atom);

/**
 * Update attention allocation
 * @return Number of atoms updated
 */
int ecan_update(void);

/**
 * Initialize ECAN
 * @param ctx GGML context
 * @return 0 on success, negative on error
 */
int ecan_init(struct ggml_context* ctx);

/** @} */

/**
 * @defgroup PLN Probabilistic Logic Networks
 * @{
 */

/** Truth value structure */
typedef struct {
    float strength;    /**< Probability (0.0 to 1.0) */
    float confidence;  /**< Confidence (0.0 to 1.0) */
} truth_value_t;

/**
 * Evaluate PLN inference on tensor graph
 * @param query Query atom
 * @param context Context atoms
 * @param n_context Number of context atoms
 * @return Truth value
 */
truth_value_t pln_eval_tensor(atom_handle_t query, atom_handle_t* context, size_t n_context);

/**
 * Unify atoms in graph
 * @param a First atom
 * @param b Second atom
 * @return Unified atom handle or 0 if unification fails
 */
atom_handle_t pln_unify_graph(atom_handle_t a, atom_handle_t b);

/**
 * Initialize PLN
 * @param ctx GGML context
 * @return 0 on success, negative on error
 */
int pln_init(struct ggml_context* ctx);

/** @} */

/**
 * @defgroup ESN Echo State Networks
 * @{
 */

/** ESN reservoir handle */
typedef struct esn_reservoir* esn_reservoir_t;

/**
 * Create ESN reservoir
 * @param input_size Input dimension
 * @param reservoir_size Reservoir size
 * @param output_size Output dimension
 * @param spectral_radius Spectral radius (typically 0.9-0.99)
 * @return Reservoir handle or NULL on error
 */
esn_reservoir_t esn_create(size_t input_size, size_t reservoir_size, 
                           size_t output_size, float spectral_radius);

/**
 * Process input through reservoir
 * @param reservoir Reservoir handle
 * @param input Input tensor
 * @param output Output tensor
 * @return 0 on success, negative on error
 */
int esn_process(esn_reservoir_t reservoir, struct ggml_tensor* input, 
                struct ggml_tensor* output);

/**
 * Destroy reservoir
 * @param reservoir Reservoir handle
 */
void esn_destroy(esn_reservoir_t reservoir);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* AICHAT_COGNITIVE_H */
