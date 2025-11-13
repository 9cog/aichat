/**
 * @file repl.cpp
 * @brief REPL mode implementation
 */

#include "aichat/cli.h"
#include "aichat/llm.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <readline/readline.h>
#include <readline/history.h>

/* Streaming callback for displaying tokens */
static void stream_callback(const char* token, void* user_data) {
    printf("%s", token);
    fflush(stdout);
}

/**
 * Run REPL mode
 */
extern "C" int cli_run_repl(cli_config_t* config) {
    if (!config) {
        return -1;
    }
    
    /* Load model */
    printf("Loading model: %s\n", config->model_path);
    llm_model_t model = llm_load_model(config->model_path);
    if (!model) {
        fprintf(stderr, "Failed to load model\n");
        return -1;
    }
    
    printf("Model loaded successfully\n");
    printf("AIChat REPL (type 'quit' to exit)\n\n");
    
    /* REPL loop */
    while (true) {
        char* line = readline("> ");
        
        if (!line) {
            break;
        }
        
        if (strlen(line) == 0) {
            free(line);
            continue;
        }
        
        add_history(line);
        
        /* Check for quit command */
        if (strcmp(line, "quit") == 0 || strcmp(line, "exit") == 0) {
            free(line);
            break;
        }
        
        /* Create message */
        chat_message_t msg = {ROLE_USER, line};
        
        /* Prepare generation parameters */
        generation_params_t params;
        params.max_tokens = config->max_tokens;
        params.temperature = config->temperature;
        params.top_p = 0.9f;
        params.top_k = 40;
        params.stream = config->stream;
        
        /* Generate response */
        char* response = llm_chat_completion(model, &msg, 1, &params,
                                            config->stream ? stream_callback : nullptr,
                                            nullptr);
        
        if (response) {
            if (!config->stream) {
                printf("%s\n", response);
            } else {
                printf("\n");
            }
            free(response);
        } else {
            fprintf(stderr, "Error generating response\n");
        }
        
        printf("\n");
        free(line);
    }
    
    /* Cleanup */
    llm_unload_model(model);
    printf("\nGoodbye!\n");
    
    return 0;
}

/**
 * Run command mode (single query)
 */
extern "C" int cli_run_command(cli_config_t* config, const char* query) {
    if (!config || !query) {
        return -1;
    }
    
    /* Load model */
    llm_model_t model = llm_load_model(config->model_path);
    if (!model) {
        fprintf(stderr, "Failed to load model\n");
        return -1;
    }
    
    /* Create message */
    chat_message_t msg = {ROLE_USER, query};
    
    /* Prepare generation parameters */
    generation_params_t params;
    params.max_tokens = config->max_tokens;
    params.temperature = config->temperature;
    params.top_p = 0.9f;
    params.top_k = 40;
    params.stream = config->stream;
    
    /* Generate response */
    char* response = llm_chat_completion(model, &msg, 1, &params,
                                        config->stream ? stream_callback : nullptr,
                                        nullptr);
    
    if (response) {
        if (!config->stream) {
            printf("%s\n", response);
        } else {
            printf("\n");
        }
        free(response);
    } else {
        fprintf(stderr, "Error generating response\n");
        llm_unload_model(model);
        return -1;
    }
    
    /* Cleanup */
    llm_unload_model(model);
    
    return 0;
}
