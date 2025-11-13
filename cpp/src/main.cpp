/**
 * @file main.cpp
 * @brief AIChat main entry point
 */

#include "aichat.h"
#include "aichat/kernel.h"
#include "aichat/cli.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

/**
 * Initialize AIChat system
 */
int aichat_init(void) {
    /* Bootstrap kernel to Stage 3 */
    return kern_bootstrap_init(STAGE3_COGNITIVE);
}

/**
 * Shutdown AIChat system
 */
void aichat_shutdown(void) {
    /* Cleanup would go here */
}

/**
 * Get version string
 */
const char* aichat_version(void) {
    static char version[32];
    snprintf(version, sizeof(version), "%d.%d.%d",
             AICHAT_VERSION_MAJOR, AICHAT_VERSION_MINOR, AICHAT_VERSION_PATCH);
    return version;
}

/**
 * Main entry point
 */
int main(int argc, char** argv) {
    printf("AIChat C++ v%s - Cognitive Kernel Edition\n", aichat_version());
    printf("Built with GGML and llama.cpp\n\n");
    
    /* Initialize system */
    if (aichat_init() != 0) {
        fprintf(stderr, "Failed to initialize AIChat\n");
        return 1;
    }
    
    /* Parse command-line arguments */
    cli_config_t config;
    int parse_result = cli_parse_args(argc, argv, &config);
    
    if (parse_result < 0) {
        aichat_shutdown();
        return 1;
    }
    
    if (parse_result > 0) {
        /* Help was shown */
        aichat_shutdown();
        return 0;
    }
    
    /* Run appropriate mode */
    int result = 0;
    
    if (config.repl_mode) {
        result = cli_run_repl(&config);
    } else {
        /* Find query in remaining arguments */
        const char* query = nullptr;
        for (int i = 1; i < argc; i++) {
            if (argv[i][0] != '-' && 
                (i == 0 || argv[i-1][0] != '-' || 
                 (strcmp(argv[i-1], "-r") == 0 || strcmp(argv[i-1], "--repl") == 0 ||
                  strcmp(argv[i-1], "-s") == 0 || strcmp(argv[i-1], "--stream") == 0))) {
                query = argv[i];
                break;
            }
        }
        
        if (query) {
            result = cli_run_command(&config, query);
        } else {
            fprintf(stderr, "No query provided\n");
            result = 1;
        }
    }
    
    /* Shutdown */
    aichat_shutdown();
    
    return result;
}
