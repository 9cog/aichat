/**
 * @file repl.cpp
 * @brief REPL mode implementation
 */

#include "aichat/cli.h"
#include "aichat/llm.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

/* Use readline if available, fall back to fgets otherwise */
#ifdef HAVE_READLINE
#  include <readline/readline.h>
#  include <readline/history.h>
#  define REPL_READLINE(prompt)     readline(prompt)
#  define REPL_ADD_HISTORY(line)    add_history(line)
#  define REPL_FREE_LINE(line)      free(line)
#else
static char* repl_fgets_readline(const char* prompt) {
    printf("%s", prompt);
    fflush(stdout);
    static char buf[4096];
    if (!fgets(buf, sizeof(buf), stdin)) {
        return nullptr;
    }
    /* Strip trailing newline */
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    }
    return buf;
}
#  define REPL_READLINE(prompt)     repl_fgets_readline(prompt)
#  define REPL_ADD_HISTORY(line)    ((void)(line))
#  define REPL_FREE_LINE(line)      ((void)(line))
#endif

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
        char* line = REPL_READLINE("> ");

        if (!line) {
            break;
        }

        if (strlen(line) == 0) {
            REPL_FREE_LINE(line);
            continue;
        }

        REPL_ADD_HISTORY(line);

        /* Check for quit command */
        if (strcmp(line, "quit") == 0 || strcmp(line, "exit") == 0) {
            REPL_FREE_LINE(line);
            break;
        }

        /* Create message */
        chat_message_t msg = {ROLE_USER, line};

        /* Prepare generation parameters */
        generation_params_t params;
        params.max_tokens  = config->max_tokens;
        params.temperature = config->temperature;
        params.top_p       = 0.9f;
        params.top_k       = 40;
        params.stream      = config->stream;

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
        REPL_FREE_LINE(line);
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
    params.max_tokens  = config->max_tokens;
    params.temperature = config->temperature;
    params.top_p       = 0.9f;
    params.top_k       = 40;
    params.stream      = config->stream;

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

