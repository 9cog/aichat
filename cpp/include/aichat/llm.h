/**
 * @file llm.h
 * @brief LLM Integration - llama.cpp wrapper and inference
 */

#ifndef AICHAT_LLM_H
#define AICHAT_LLM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * @defgroup LLM LLM Integration
 * @{
 */

/** Model handle */
typedef struct llm_model* llm_model_t;

/** Chat message role */
typedef enum {
    ROLE_SYSTEM = 0,
    ROLE_USER = 1,
    ROLE_ASSISTANT = 2,
} message_role_t;

/** Chat message */
typedef struct {
    message_role_t role;
    const char* content;
} chat_message_t;

/** Generation parameters */
typedef struct {
    int max_tokens;
    float temperature;
    float top_p;
    float top_k;
    bool stream;
} generation_params_t;

/** Streaming callback */
typedef void (*stream_callback_t)(const char* token, void* user_data);

/**
 * Load LLM model
 * @param model_path Path to GGUF model file
 * @return Model handle or NULL on error
 */
llm_model_t llm_load_model(const char* model_path);

/**
 * Generate chat completion
 * @param model Model handle
 * @param messages Array of chat messages
 * @param n_messages Number of messages
 * @param params Generation parameters
 * @param callback Streaming callback (can be NULL)
 * @param user_data User data for callback
 * @return Generated response (caller must free) or NULL on error
 */
char* llm_chat_completion(llm_model_t model, chat_message_t* messages, 
                          size_t n_messages, generation_params_t* params,
                          stream_callback_t callback, void* user_data);

/**
 * Unload model
 * @param model Model handle
 */
void llm_unload_model(llm_model_t model);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* AICHAT_LLM_H */
