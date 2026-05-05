/**
 * @file inference.cpp
 * @brief LLM inference using llama.cpp
 */

#include "aichat/llm.h"
#include "llm_internal.h"
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>

/* LLM Model structure is defined in llm_internal.h */

/**
 * Load LLM model
 */
extern "C" llm_model_t llm_load_model(const char* model_path) {
    if (!model_path) {
        return nullptr;
    }

    /* Initialize llama backend */
    llama_backend_init();

    /* Load model */
    llama_model_params model_params = llama_model_default_params();
    llama_model* model = llama_model_load_from_file(model_path, model_params);

    if (!model) {
        return nullptr;
    }

    /* Create context */
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 4096;
    ctx_params.n_batch = 512;
    ctx_params.n_threads = 4;

    llama_context* ctx = llama_init_from_model(model, ctx_params);

    if (!ctx) {
        llama_model_free(model);
        return nullptr;
    }

    /* Create model structure */
    llm_model_t llm = new llm_model();
    llm->model = model;
    llm->ctx = ctx;
    llm->model_path = model_path;

    return llm;
}

/**
 * Unload model
 */
extern "C" void llm_unload_model(llm_model_t model) {
    if (!model) {
        return;
    }

    if (model->ctx) {
        llama_free(model->ctx);
    }

    if (model->model) {
        llama_model_free(model->model);
    }

    delete model;

    llama_backend_free();
}
