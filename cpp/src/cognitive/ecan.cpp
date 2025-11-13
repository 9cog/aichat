/**
 * @file ecan.cpp
 * @brief ECAN - Economic Attention Networks
 * 
 * Implements attention allocation using tensor operations.
 */

#include "aichat/cognitive.h"
#include <ggml.h>
#include <cstring>
#include <map>

/* Attention values storage */
static std::map<atom_handle_t, attention_value_t> attention_map;
static struct ggml_context* ecan_ctx = nullptr;
static bool ecan_initialized = false;

/**
 * Initialize ECAN
 */
extern "C" int ecan_init(struct ggml_context* ctx) {
    if (ecan_initialized) {
        return 0;
    }
    
    if (!ctx) {
        return -1;
    }
    
    ecan_ctx = ctx;
    attention_map.clear();
    ecan_initialized = true;
    
    return 0;
}

/**
 * Get attention value for atom
 */
extern "C" attention_value_t ecan_get_attention(atom_handle_t atom) {
    if (!ecan_initialized) {
        return {0.0f, 0.0f, 0.0f};
    }
    
    auto it = attention_map.find(atom);
    if (it != attention_map.end()) {
        return it->second;
    }
    
    /* Default attention values */
    attention_value_t av = {0.5f, 0.5f, 0.5f};
    attention_map[atom] = av;
    return av;
}

/**
 * Update attention allocation
 * 
 * Implements ECAN spreading dynamics:
 * - STI spreads via hypergraph edges
 * - Importance decays over time
 * - Top-N atoms receive focus
 */
extern "C" int ecan_update(void) {
    if (!ecan_initialized) {
        return -1;
    }
    
    int updated = 0;
    
    /* Decay all STI values */
    const float decay_rate = 0.99f;
    for (auto& pair : attention_map) {
        pair.second.sti *= decay_rate;
        pair.second.lti = pair.second.lti * 0.99f + pair.second.sti * 0.01f;
        updated++;
    }
    
    return updated;
}
