/**
 * @file llm_internal.h
 * @brief Internal LLM model structure shared by inference.cpp and chat.cpp
 */

#ifndef AICHAT_LLM_INTERNAL_H
#define AICHAT_LLM_INTERNAL_H

#include "llama.h"
#include <string>

/* LLM Model structure (internal) */
struct llm_model {
    llama_model*  model;
    llama_context* ctx;
    std::string   model_path;
};

#endif /* AICHAT_LLM_INTERNAL_H */
