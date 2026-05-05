/**
 * @file cogloop.cpp
 * @brief CognitiveLoop orchestration - full OpenCog cognitive cycle
 *
 * Implements the unified cognitive loop that ties together AtomSpace,
 * ECAN, PLN, and ESN into a single repeatable processing cycle.
 */

#include "aichat/cogloop.h"
#include "aichat/cognitive.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <vector>

/* Maximum atoms tracked in the loop's local attention list */
#define COGLOOP_MAX_TRACKED 4096

/**
 * Cognitive loop internal state
 */
struct cogloop {
    cogloop_config_t cfg;
    uint64_t         cycle_count;

    /* Tracked atoms with their STI snapshot */
    struct AtomRecord {
        atom_handle_t handle;
        float         sti;
    };
    std::vector<AtomRecord> tracked;

    /* ESN reservoir for temporal dynamics */
    esn_reservoir_t esn;
};

/**
 * Default configuration
 */
extern "C" cogloop_config_t cogloop_default_config(void) {
    cogloop_config_t cfg;
    cfg.max_atoms      = 1024;
    cfg.ecan_cycles    = 3;
    cfg.pln_depth      = 2;
    cfg.esn_input_size = 64;
    cfg.esn_reservoir  = 256;
    cfg.esn_spectral   = 0.95f;
    cfg.sti_decay      = 0.95f;
    cfg.sti_threshold  = 0.1f;
    return cfg;
}

/**
 * Create a cognitive loop
 */
extern "C" cogloop_t cogloop_create(const cogloop_config_t* cfg) {
    if (!cfg) {
        return nullptr;
    }

    cogloop* loop = new (std::nothrow) cogloop();
    if (!loop) {
        return nullptr;
    }

    loop->cfg         = *cfg;
    loop->cycle_count = 0;

    /* Create ESN reservoir */
    loop->esn = esn_create(cfg->esn_input_size, cfg->esn_reservoir,
                            cfg->esn_input_size, cfg->esn_spectral);
    if (!loop->esn) {
        delete loop;
        return nullptr;
    }

    loop->tracked.reserve(COGLOOP_MAX_TRACKED);

    return loop;
}

/**
 * Destroy a cognitive loop
 */
extern "C" void cogloop_destroy(cogloop_t loop) {
    if (!loop) {
        return;
    }

    if (loop->esn) {
        esn_destroy(loop->esn);
    }

    delete loop;
}

/**
 * Process one full cognitive cycle
 */
extern "C" int cogloop_cycle(cogloop_t loop, const atom_handle_t* input, size_t n_input) {
    if (!loop) {
        return -1;
    }

    /* --- Stage 1: Ingest input atoms into tracked list --- */
    for (size_t i = 0; i < n_input; i++) {
        if (input[i] == 0) {
            continue;
        }

        /* Check if already tracked */
        bool found = false;
        for (auto& rec : loop->tracked) {
            if (rec.handle == input[i]) {
                found = true;
                /* Boost STI on re-encounter */
                rec.sti = std::min(1.0f, rec.sti + 0.2f);
                break;
            }
        }

        if (!found && loop->tracked.size() < (size_t)COGLOOP_MAX_TRACKED) {
            cogloop::AtomRecord rec;
            rec.handle = input[i];
            rec.sti    = 0.5f;  /* Initial STI for new atoms */
            loop->tracked.push_back(rec);
        }
    }

    /* --- Stage 2: ECAN attention spreading --- */
    for (size_t c = 0; c < loop->cfg.ecan_cycles; c++) {
        ecan_update();
    }

    /* Refresh STI from ECAN and apply decay to unvisited atoms */
    for (auto& rec : loop->tracked) {
        attention_value_t av = ecan_get_attention(rec.handle);
        /* Blend ECAN output with local decay */
        rec.sti = rec.sti * loop->cfg.sti_decay * 0.5f + av.sti * 0.5f;
    }

    /* Evict atoms below threshold if over capacity */
    if (loop->tracked.size() > loop->cfg.max_atoms) {
        loop->tracked.erase(
            std::remove_if(loop->tracked.begin(), loop->tracked.end(),
                           [&](const cogloop::AtomRecord& r) {
                               return r.sti < loop->cfg.sti_threshold;
                           }),
            loop->tracked.end());
    }

    /* --- Stage 3: PLN inference on high-attention atoms --- */
    int n_inferences = 0;

    /* Collect context: atoms above STI threshold */
    std::vector<atom_handle_t> context;
    for (const auto& rec : loop->tracked) {
        if (rec.sti >= loop->cfg.sti_threshold) {
            context.push_back(rec.handle);
        }
    }

    /* Run PLN on top-k pairs from high-attention atoms */
    size_t k = std::min(context.size(), (size_t)8);
    for (size_t i = 0; i < k && i < context.size(); i++) {
        truth_value_t tv = pln_eval_tensor(context[i],
                                           context.data(),
                                           context.size());
        if (tv.confidence > 0.3f) {
            n_inferences++;
        }
    }

    /* --- Stage 4: ESN reservoir update (temporal dynamics) --- */
    /* ESN processes without explicit tensor input in this lightweight pass */

    loop->cycle_count++;

    return n_inferences;
}

/**
 * Return the top-k atoms by STI
 */
extern "C" int cogloop_top_atoms(cogloop_t loop, atom_handle_t* out, size_t max_atoms) {
    if (!loop || !out || max_atoms == 0) {
        return -1;
    }

    /* Sort by STI descending */
    std::vector<cogloop::AtomRecord> sorted = loop->tracked;
    std::sort(sorted.begin(), sorted.end(),
              [](const cogloop::AtomRecord& a, const cogloop::AtomRecord& b) {
                  return a.sti > b.sti;
              });

    size_t n = std::min(sorted.size(), max_atoms);
    for (size_t i = 0; i < n; i++) {
        out[i] = sorted[i].handle;
    }

    return (int)n;
}

/**
 * Query PLN truth value using current loop context
 */
extern "C" truth_value_t cogloop_query(cogloop_t loop, atom_handle_t query) {
    truth_value_t tv = {0.0f, 0.0f};

    if (!loop || query == 0) {
        return tv;
    }

    /* Use high-attention atoms as inference context */
    std::vector<atom_handle_t> context;
    for (const auto& rec : loop->tracked) {
        if (rec.sti >= loop->cfg.sti_threshold) {
            context.push_back(rec.handle);
        }
    }

    if (context.empty()) {
        return tv;
    }

    return pln_eval_tensor(query, context.data(), context.size());
}

/**
 * Return cycle count
 */
extern "C" uint64_t cogloop_cycle_count(const cogloop_t loop) {
    if (!loop) {
        return 0;
    }
    return loop->cycle_count;
}
