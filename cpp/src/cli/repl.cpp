/**
 * @file repl.cpp
 * @brief REPL mode implementation with session persistence
 */

#include "aichat/cli.h"
#include "aichat/llm.h"
#include "aichat/session.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>

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
 * Build a flat message array from the session for llm_chat_completion.
 * Returns the number of messages written into `out` (up to `max`).
 */
static size_t session_to_messages(session_t sess, chat_message_t* out, size_t max) {
    size_t n = session_length(sess);
    if (n > max) {
        n = max;
    }
    for (size_t i = 0; i < n; i++) {
        session_get(sess, i, &out[i]);
    }
    return n;
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
    printf("AIChat REPL (type 'quit' to exit");
    if (config->session_path) {
        printf(", session: %s", config->session_path);
    }
    printf(")\n\n");

    /* ── Session setup ── */
    session_t sess = nullptr;

    if (config->session_path) {
        /* Try to resume existing session */
        sess = session_load(config->session_path);
        if (sess) {
            printf("Resumed session '%s' (%zu messages)\n\n",
                   session_id(sess), session_length(sess));
        } else {
            sess = session_create("repl");
        }
    } else {
        sess = session_create("repl");
    }

    if (!sess) {
        fprintf(stderr, "Failed to create session\n");
        llm_unload_model(model);
        return -1;
    }

    /* Prepend system prompt if provided and not already in session */
    if (config->system_prompt && session_length(sess) == 0) {
        session_append(sess, ROLE_SYSTEM, config->system_prompt);
    }

    /* Reusable message buffer (max history depth) */
    static const size_t MAX_SESSION_MESSAGES = SESSION_MAX_MESSAGES;
    chat_message_t* msgs = new (std::nothrow) chat_message_t[MAX_SESSION_MESSAGES];
    if (!msgs) {
        session_destroy(sess);
        llm_unload_model(model);
        return -1;
    }

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

        /* Built-in commands */
        if (strcmp(line, "quit") == 0 || strcmp(line, "exit") == 0) {
            REPL_FREE_LINE(line);
            break;
        }

        if (strcmp(line, "/clear") == 0) {
            session_clear(sess);
            /* Re-add system prompt after clear */
            if (config->system_prompt) {
                session_append(sess, ROLE_SYSTEM, config->system_prompt);
            }
            printf("Session cleared.\n\n");
            REPL_FREE_LINE(line);
            continue;
        }

        if (strcmp(line, "/history") == 0) {
            size_t n = session_length(sess);
            printf("History (%zu messages):\n", n);
            for (size_t i = 0; i < n; i++) {
                chat_message_t m;
                session_get(sess, i, &m);
                const char* role_str =
                    m.role == ROLE_SYSTEM    ? "system" :
                    m.role == ROLE_ASSISTANT ? "assistant" : "user";
                printf("  [%zu] %s: %.80s%s\n", i, role_str, m.content,
                       strlen(m.content) > 80 ? "..." : "");
            }
            printf("\n");
            REPL_FREE_LINE(line);
            continue;
        }

        /* Add user message to session */
        session_append(sess, ROLE_USER, line);
        REPL_FREE_LINE(line);

        /* Build message array from full session history */
        size_t n_msgs = session_to_messages(sess, msgs, MAX_SESSION_MESSAGES);

        /* Prepare generation parameters */
        generation_params_t params;
        params.max_tokens  = config->max_tokens;
        params.temperature = config->temperature;
        params.top_p       = 0.9f;
        params.top_k       = 40;
        params.stream      = config->stream;

        /* Generate response */
        char* response = llm_chat_completion(model, msgs, n_msgs, &params,
                                             config->stream ? stream_callback : nullptr,
                                             nullptr);

        if (response) {
            if (!config->stream) {
                printf("%s\n", response);
            } else {
                printf("\n");
            }

            /* Record assistant response in session */
            session_append(sess, ROLE_ASSISTANT, response);
            free(response);

            /* Persist session if path configured */
            if (config->session_path) {
                session_save(sess, config->session_path);
            }
        } else {
            fprintf(stderr, "Error generating response\n");
        }

        printf("\n");
    }

    /* Cleanup */
    delete[] msgs;
    session_destroy(sess);
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

    /* Build message list: optional system prompt + user query */
    chat_message_t msgs[2];
    size_t n_msgs = 0;

    if (config->system_prompt) {
        msgs[n_msgs].role    = ROLE_SYSTEM;
        msgs[n_msgs].content = config->system_prompt;
        n_msgs++;
    }

    msgs[n_msgs].role    = ROLE_USER;
    msgs[n_msgs].content = query;
    n_msgs++;

    /* Prepare generation parameters */
    generation_params_t params;
    params.max_tokens  = config->max_tokens;
    params.temperature = config->temperature;
    params.top_p       = 0.9f;
    params.top_k       = 40;
    params.stream      = config->stream;

    /* Generate response */
    char* response = llm_chat_completion(model, msgs, n_msgs, &params,
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

