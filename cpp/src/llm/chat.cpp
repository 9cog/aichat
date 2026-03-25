/**
 * @file chat.cpp
 * @brief Chat completion implementation
 */

#include "aichat/llm.h"
#include "llm_internal.h"
#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>

/**
 * Generate chat completion
 */
extern "C" char* llm_chat_completion(llm_model_t model, chat_message_t* messages,
                                     size_t n_messages, generation_params_t* params,
                                     stream_callback_t callback, void* user_data) {
    if (!model || !messages || n_messages == 0) {
        return nullptr;
    }

    const llama_vocab* vocab = llama_model_get_vocab(model->model);

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

    int n_tokens = llama_tokenize(vocab, prompt_str.c_str(), (int32_t)prompt_str.size(),
                                   tokens.data(), (int32_t)tokens.size(), true, true);

    if (n_tokens < 0) {
        tokens.resize(-n_tokens);
        n_tokens = llama_tokenize(vocab, prompt_str.c_str(), (int32_t)prompt_str.size(),
                                   tokens.data(), (int32_t)tokens.size(), true, true);
    }

    tokens.resize(n_tokens);

    /* Build sampler chain */
    llama_sampler_chain_params chain_params = llama_sampler_chain_default_params();
    llama_sampler* smpl = llama_sampler_chain_init(chain_params);

    if (params && params->top_k > 0) {
        llama_sampler_chain_add(smpl, llama_sampler_init_top_k((int32_t)params->top_k));
    }
    if (params && params->top_p > 0.0f) {
        llama_sampler_chain_add(smpl, llama_sampler_init_top_p(params->top_p, 1));
    }
    float temp = params ? params->temperature : 0.8f;
    llama_sampler_chain_add(smpl, llama_sampler_init_temp(temp));
    llama_sampler_chain_add(smpl, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));

    /* Generate tokens */
    std::string response;
    int max_tokens = params ? params->max_tokens : 512;

    /* Evaluate prompt */
    llama_decode(model->ctx, llama_batch_get_one(tokens.data(), (int32_t)tokens.size()));

    for (int i = 0; i < max_tokens; i++) {
        llama_token new_token = llama_sampler_sample(smpl, model->ctx, -1);

        /* Check for EOS */
        if (llama_vocab_is_eog(vocab, new_token)) {
            break;
        }

        /* Decode token */
        char piece[256];
        int n_piece = llama_token_to_piece(vocab, new_token, piece, (int32_t)sizeof(piece), 0, true);

        if (n_piece > 0) {
            std::string token_str(piece, n_piece);
            response += token_str;

            /* Call streaming callback */
            if (callback) {
                callback(token_str.c_str(), user_data);
            }
        }

        /* Accept token and evaluate next */
        llama_sampler_accept(smpl, new_token);
        llama_decode(model->ctx, llama_batch_get_one(&new_token, 1));
    }

    llama_sampler_free(smpl);

    /* Return response */
    return strdup(response.c_str());
}
