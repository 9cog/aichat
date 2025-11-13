/**
 * @file aichat.h
 * @brief Main header for AIChat C/C++ implementation
 * 
 * AIChat Cognitive Kernel - GGML/llama.cpp Integration
 * 
 * This header provides the core API for the AIChat cognitive kernel,
 * implementing OpenCog primitives as GGML tensor operations.
 */

#ifndef AICHAT_H
#define AICHAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Version information */
#define AICHAT_VERSION_MAJOR 0
#define AICHAT_VERSION_MINOR 1
#define AICHAT_VERSION_PATCH 0

/* Kernel subsystem headers */
#include "aichat/kernel.h"
#include "aichat/cognitive.h"
#include "aichat/llm.h"
#include "aichat/cli.h"

/**
 * Initialize the AIChat system
 * @return 0 on success, negative on error
 */
int aichat_init(void);

/**
 * Shutdown the AIChat system
 */
void aichat_shutdown(void);

/**
 * Get version string
 * @return Version string in format "major.minor.patch"
 */
const char* aichat_version(void);

#ifdef __cplusplus
}
#endif

#endif /* AICHAT_H */
