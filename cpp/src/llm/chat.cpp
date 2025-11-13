/**
 * @file chat.cpp
 * @brief Chat completion implementation
 */

#include "aichat/llm.h"
#include "llama.h"
#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>

/**
 * Generate chat completion
 */
extern "C" char* llm_chat_completion(llm_model_t model, chat_message_t* messages,
                                     size_t n_messages, generation_params_t* params,
                                     stream_callback_t callback, void* user_data) {
    if (!model || !messages || n_messages == 0) {
        return nullptr;
    }
    
    /* Build prompt from messages */
    std::ostringstream prompt;
    
    for (size_t i = 0; i < n_messages; i++) {
        switch (messages[i].role) {
            case ROLE_SYSTEM:
                prompt << "<|system|>\n" << messages[i].content << "\n";
                break;
            case ROLE_USER:
                prompt << "<|user|>\n" << messages[i].content << "\n";
                break;
            case ROLE_ASSISTANT:
                prompt << "<|assistant|>\n" << messages[i].content << "\n";
                break;
        }
    }
    
    prompt << "<|assistant|>\n";
    std::string prompt_str = prompt.str();
    
    /* Tokenize prompt */
    std::vector<llama_token> tokens;
    tokens.resize(prompt_str.size() + 16);
    
    int n_tokens = llama_tokenize(model->model, prompt_str.c_str(), prompt_str.size(),
                                   tokens.data(), tokens.size(), true, true);
    
    if (n_tokens < 0) {
        tokens.resize(-n_tokens);
        n_tokens = llama_tokenize(model->model, prompt_str.c_str(), prompt_str.size(),
                                   tokens.data(), tokens.size(), true, true);
    }
    
    tokens.resize(n_tokens);
    
    /* Prepare sampling */
    llama_sampling_params sampling_params = llama_sampling_default_params();
    if (params) {
        sampling_params.temp = params->temperature;
        sampling_params.top_p = params->top_p;
        sampling_params.top_k = params->top_k;
    }
    
    llama_sampling_context* ctx_sampling = llama_sampling_init(sampling_params);
    
    /* Generate tokens */
    std::string response;
    int max_tokens = params ? params->max_tokens : 512;
    
    /* Evaluate prompt */
    llama_decode(model->ctx, llama_batch_get_one(tokens.data(), tokens.size(), 0, 0));
    
    for (int i = 0; i < max_tokens; i++) {
        llama_token new_token = llama_sampling_sample(ctx_sampling, model->ctx, nullptr);
        
        /* Check for EOS */
        if (llama_token_is_eog(model->model, new_token)) {
            break;
        }
        
        /* Decode token */
        char piece[256];
        int n_piece = llama_token_to_piece(model->model, new_token, piece, sizeof(piece), 0, true);
        
        if (n_piece > 0) {
            std::string token_str(piece, n_piece);
            response += token_str;
            
            /* Call streaming callback */
            if (callback) {
                callback(token_str.c_str(), user_data);
            }
        }
        
        /* Accept token for sampling context */
        llama_sampling_accept(ctx_sampling, model->ctx, new_token, true);
        
        /* Evaluate next token */
        llama_batch batch = llama_batch_get_one(&new_token, 1, tokens.size() + i, 0);
        llama_decode(model->ctx, batch);
    }
    
    llama_sampling_free(ctx_sampling);
    
    /* Return response */
    return strdup(response.c_str());
}
