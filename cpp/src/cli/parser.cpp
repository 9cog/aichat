/**
 * @file parser.cpp
 * @brief Command-line argument parser
 */

#include "aichat/cli.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>

/**
 * Print usage information
 */
static void print_usage(const char* program) {
    printf("Usage: %s [OPTIONS] [QUERY]\n", program);
    printf("\n");
    printf("Options:\n");
    printf("  -m, --model PATH     Path to GGUF model file\n");
    printf("  -c, --config PATH    Path to configuration file\n");
    printf("  -r, --repl           Start in REPL mode\n");
    printf("  -s, --stream         Enable streaming output\n");
    printf("  -t, --temperature T  Sampling temperature (default: 0.7)\n");
    printf("  -n, --max-tokens N   Maximum tokens to generate (default: 512)\n");
    printf("  -h, --help           Show this help message\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s -m model.gguf \"Hello, how are you?\"\n", program);
    printf("  %s -r -m model.gguf\n", program);
}

/**
 * Parse command-line arguments
 */
extern "C" int cli_parse_args(int argc, char** argv, cli_config_t* config) {
    if (!config) {
        return -1;
    }
    
    /* Initialize defaults */
    config->model_path = nullptr;
    config->config_path = nullptr;
    config->repl_mode = false;
    config->stream = false;
    config->temperature = 0.7f;
    config->max_tokens = 512;
    
    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 1;
        }
        else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--model") == 0) {
            if (i + 1 < argc) {
                config->model_path = argv[++i];
            } else {
                fprintf(stderr, "Error: -m requires an argument\n");
                return -1;
            }
        }
        else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0) {
            if (i + 1 < argc) {
                config->config_path = argv[++i];
            } else {
                fprintf(stderr, "Error: -c requires an argument\n");
                return -1;
            }
        }
        else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--repl") == 0) {
            config->repl_mode = true;
        }
        else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--stream") == 0) {
            config->stream = true;
        }
        else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--temperature") == 0) {
            if (i + 1 < argc) {
                config->temperature = atof(argv[++i]);
            } else {
                fprintf(stderr, "Error: -t requires an argument\n");
                return -1;
            }
        }
        else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--max-tokens") == 0) {
            if (i + 1 < argc) {
                config->max_tokens = atoi(argv[++i]);
            } else {
                fprintf(stderr, "Error: -n requires an argument\n");
                return -1;
            }
        }
    }
    
    /* Validate required arguments */
    if (!config->model_path) {
        fprintf(stderr, "Error: Model path is required (-m/--model)\n");
        print_usage(argv[0]);
        return -1;
    }
    
    return 0;
}
