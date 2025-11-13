/**
 * @file cli.h
 * @brief Command-line interface and REPL
 */

#ifndef AICHAT_CLI_H
#define AICHAT_CLI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/**
 * @defgroup CLI Command-Line Interface
 * @{
 */

/** CLI configuration */
typedef struct {
    const char* model_path;
    const char* config_path;
    bool repl_mode;
    bool stream;
    float temperature;
    int max_tokens;
} cli_config_t;

/**
 * Parse command-line arguments
 * @param argc Argument count
 * @param argv Argument vector
 * @param config Output configuration
 * @return 0 on success, negative on error
 */
int cli_parse_args(int argc, char** argv, cli_config_t* config);

/**
 * Run REPL mode
 * @param config CLI configuration
 * @return 0 on success, negative on error
 */
int cli_run_repl(cli_config_t* config);

/**
 * Run command mode (single query)
 * @param config CLI configuration
 * @param query Query string
 * @return 0 on success, negative on error
 */
int cli_run_command(cli_config_t* config, const char* query);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* AICHAT_CLI_H */
