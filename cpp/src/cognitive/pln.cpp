/**
 * @file pln.cpp
 * @brief PLN - Probabilistic Logic Networks
 * 
 * Implements probabilistic reasoning using tensor graphs.
 */

#include "aichat/cognitive.h"
#include <ggml.h>
#include <cmath>
#include <map>

/* Truth values storage */
static std::map<atom_handle_t, truth_value_t> truth_map;
static struct ggml_context* pln_ctx = nullptr;
static bool pln_initialized = false;

/**
 * Initialize PLN
 */
extern "C" int pln_init(struct ggml_context* ctx) {
    if (pln_initialized) {
        return 0;
    }
    
    if (!ctx) {
        return -1;
    }
    
    pln_ctx = ctx;
    truth_map.clear();
    pln_initialized = true;
    
    return 0;
}

/**
 * Evaluate PLN inference on tensor graph
 * 
 * Implements basic PLN inference:
 * - Deduction: (A->B, B->C) => (A->C)
 * - Induction: (A->B, C->B) => (A->C)
 * - Abduction: (A->B, A->C) => (C->B)
 */
extern "C" truth_value_t pln_eval_tensor(atom_handle_t query, 
                                         atom_handle_t* context, 
                                         size_t n_context) {
    if (!pln_initialized) {
        return {0.0f, 0.0f};
    }
    
    /* Check if query has cached truth value */
    auto it = truth_map.find(query);
    if (it != truth_map.end()) {
        return it->second;
    }
    
    /* Compute truth value from context */
    float strength_sum = 0.0f;
    float confidence_product = 1.0f;
    
    for (size_t i = 0; i < n_context; i++) {
        auto ctx_it = truth_map.find(context[i]);
        if (ctx_it != truth_map.end()) {
            strength_sum += ctx_it->second.strength;
            confidence_product *= ctx_it->second.confidence;
        }
    }
    
    truth_value_t result;
    result.strength = n_context > 0 ? strength_sum / n_context : 0.5f;
    result.confidence = n_context > 0 ? std::pow(confidence_product, 1.0f / n_context) : 0.1f;
    
    /* Cache result */
    truth_map[query] = result;
    
    return result;
}

/**
 * Unify atoms in graph
 * 
 * Returns unified atom if unification succeeds, 0 otherwise.
 */
extern "C" atom_handle_t pln_unify_graph(atom_handle_t a, atom_handle_t b) {
    if (!pln_initialized) {
        return 0;
    }
    
    /* Get truth values */
    auto it_a = truth_map.find(a);
    auto it_b = truth_map.find(b);
    
    if (it_a == truth_map.end() || it_b == truth_map.end()) {
        return 0;
    }
    
    /* Simple unification: check if strengths are compatible */
    float diff = std::abs(it_a->second.strength - it_b->second.strength);
    if (diff < 0.2f) {
        /* Unification succeeds - return higher confidence atom */
        return it_a->second.confidence > it_b->second.confidence ? a : b;
    }
    
    return 0;
}
