/**
 * @file rag.cpp
 * @brief Retrieval-Augmented Generation implementation
 *
 * Implements a self-contained vector store for RAG using:
 *  - Text chunking with configurable size and overlap
 *  - Hash-projection feature space (RAG_FEATURE_DIM dimensions)
 *    via FNV-1a word hashing – no external embedding model required
 *  - L2-normalised term-frequency feature vectors
 *  - Cosine similarity (dot product of unit vectors) for retrieval
 *
 * No dependencies beyond the C/C++ standard library.
 */

#include "aichat/rag.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cctype>
#include <string>
#include <vector>
#include <algorithm>
#include <new>

/* ── Internal types ─────────────────────────────────────────────────────── */

/** Single stored chunk with its pre-computed feature vector */
struct RagChunk {
    std::string text;
    std::string source;
    float       features[RAG_FEATURE_DIM]; /**< L2-normalised TF vector */
};

struct rag_store {
    rag_config_t          cfg;
    std::vector<RagChunk> chunks;
};

/* ── Feature extraction ─────────────────────────────────────────────────── */

/**
 * FNV-1a 32-bit hash of a null-terminated lower-case word.
 */
static uint32_t fnv1a(const char* word) {
    uint32_t h = 2166136261u;
    while (*word) {
        h ^= (uint8_t)*word++;
        h *= 16777619u;
    }
    return h;
}

/**
 * Tokenise text into lower-case words and accumulate term frequencies
 * into a RAG_FEATURE_DIM float array.
 */
static void text_to_features(const char* text, float* features) {
    memset(features, 0, RAG_FEATURE_DIM * sizeof(float));

    const char* p = text;
    char word[256];
    size_t wlen = 0;

    auto accumulate_word = [&]() {
        if (wlen == 0) {
            return;
        }
        word[wlen] = '\0';
        uint32_t h = fnv1a(word);
        features[h % RAG_FEATURE_DIM] += 1.0f;
        wlen = 0;
    };

    while (*p) {
        unsigned char c = (unsigned char)*p;
        if (isalpha(c) || c == '\'') {
            if (wlen < sizeof(word) - 1) {
                word[wlen++] = (char)tolower(c);
            }
        } else {
            accumulate_word();
        }
        p++;
    }
    accumulate_word();

    /* L2 normalise */
    float norm = 0.0f;
    for (int i = 0; i < RAG_FEATURE_DIM; i++) {
        norm += features[i] * features[i];
    }
    if (norm > 0.0f) {
        norm = sqrtf(norm);
        for (int i = 0; i < RAG_FEATURE_DIM; i++) {
            features[i] /= norm;
        }
    }
}

/**
 * Cosine similarity between two L2-normalised vectors.
 * Equivalent to a plain dot product since both vectors are unit-length.
 */
static float cosine_similarity(const float* a, const float* b) {
    float dot = 0.0f;
    for (int i = 0; i < RAG_FEATURE_DIM; i++) {
        dot += a[i] * b[i];
    }
    /* Clamp to [0,1] to avoid tiny floating-point negatives */
    return (dot < 0.0f) ? 0.0f : (dot > 1.0f ? 1.0f : dot);
}

/* ── Text chunking ──────────────────────────────────────────────────────── */

/**
 * Split @p text into overlapping character-level chunks and append them
 * to @p out.  @p source is attached to every chunk.
 */
static int chunk_text(const std::string& text, const std::string& source,
                      size_t chunk_size, size_t overlap,
                      size_t max_chunks, std::vector<RagChunk>& out) {
    if (text.empty() || chunk_size == 0) {
        return 0;
    }

    int added = 0;
    size_t len = text.size();
    size_t pos = 0;

    while (pos < len) {
        if (max_chunks > 0 && out.size() >= max_chunks) {
            break;
        }

        size_t end = pos + chunk_size;
        if (end > len) {
            end = len;
        }

        /* Extend to the end of the current word so we don't cut mid-word */
        while (end < len && !isspace((unsigned char)text[end])) {
            end++;
        }

        RagChunk chunk;
        chunk.text   = text.substr(pos, end - pos);
        chunk.source = source;
        text_to_features(chunk.text.c_str(), chunk.features);
        out.push_back(std::move(chunk));
        added++;

        if (end >= len) {
            break;
        }

        /* Advance by (chunk_size - overlap), but at least 1 character */
        size_t step = (chunk_size > overlap) ? (chunk_size - overlap) : 1;
        pos += step;
    }

    return added;
}

/* ── Public API ─────────────────────────────────────────────────────────── */

extern "C" rag_config_t rag_default_config(void) {
    rag_config_t cfg;
    cfg.chunk_size    = 512;
    cfg.chunk_overlap = 128;
    cfg.max_chunks    = 0;   /* no limit */
    cfg.top_k         = 4;
    return cfg;
}

extern "C" rag_store_t rag_create(const rag_config_t* cfg) {
    rag_store* store = new (std::nothrow) rag_store();
    if (!store) {
        return nullptr;
    }

    if (cfg) {
        store->cfg = *cfg;
    } else {
        store->cfg = rag_default_config();
    }

    return store;
}

extern "C" void rag_destroy(rag_store_t store) {
    delete store;
}

extern "C" int rag_add_text(rag_store_t store, const char* text, const char* source) {
    if (!store || !text) {
        return -1;
    }

    std::string src = source ? source : "unknown";

    int added = chunk_text(std::string(text), src,
                            store->cfg.chunk_size,
                            store->cfg.chunk_overlap,
                            store->cfg.max_chunks,
                            store->chunks);
    return added;
}

extern "C" int rag_add_file(rag_store_t store, const char* path) {
    if (!store || !path) {
        return -1;
    }

    FILE* f = fopen(path, "r");
    if (!f) {
        return -2;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize <= 0 || fsize > 64 * 1024 * 1024) {
        fclose(f);
        return -3;
    }

    std::string buf(fsize, '\0');
    size_t n = fread(&buf[0], 1, (size_t)fsize, f);
    fclose(f);
    buf.resize(n);

    return rag_add_text(store, buf.c_str(), path);
}

extern "C" int rag_query(rag_store_t store, const char* query,
                         rag_chunk_t* results, size_t max_results) {
    if (!store || !query || !results || max_results == 0) {
        return -1;
    }

    if (store->chunks.empty()) {
        return 0;
    }

    /* Compute query feature vector */
    float qfeatures[RAG_FEATURE_DIM];
    text_to_features(query, qfeatures);

    /* Score every chunk */
    struct ScoredIdx {
        float  score;
        size_t idx;
    };
    std::vector<ScoredIdx> scored;
    scored.reserve(store->chunks.size());

    for (size_t i = 0; i < store->chunks.size(); i++) {
        float score = cosine_similarity(qfeatures, store->chunks[i].features);
        scored.push_back({score, i});
    }

    /* Partial sort: bring top max_results to the front */
    size_t k = std::min(max_results, scored.size());
    std::partial_sort(scored.begin(), scored.begin() + (ptrdiff_t)k, scored.end(),
                      [](const ScoredIdx& a, const ScoredIdx& b) {
                          return a.score > b.score;
                      });

    for (size_t i = 0; i < k; i++) {
        const RagChunk& chunk = store->chunks[scored[i].idx];
        results[i].text   = chunk.text.c_str();
        results[i].source = chunk.source.c_str();
        results[i].score  = scored[i].score;
    }

    return (int)k;
}

extern "C" char* rag_build_context(rag_store_t store, const char* query, size_t top_k) {
    if (!store || !query) {
        return nullptr;
    }

    size_t k = (top_k > 0) ? top_k : store->cfg.top_k;
    if (k == 0) {
        k = 4;
    }

    std::vector<rag_chunk_t> results(k);
    int n = rag_query(store, query, results.data(), k);
    if (n <= 0) {
        return nullptr;
    }

    /* Build context string */
    std::string ctx;
    ctx.reserve(4096);
    ctx += "--- Context ---\n";

    for (int i = 0; i < n; i++) {
        ctx += "[source: ";
        ctx += results[i].source;
        ctx += "]\n";
        ctx += results[i].text;
        ctx += "\n\n";
    }

    ctx += "--- End Context ---\n";

    return strdup(ctx.c_str());
}

extern "C" size_t rag_chunk_count(const rag_store_t store) {
    return store ? store->chunks.size() : 0;
}
