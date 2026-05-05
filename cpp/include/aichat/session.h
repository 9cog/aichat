/**
 * @file session.h
 * @brief Session persistence - save and restore conversation history
 *
 * Provides a lightweight JSON-based session format for persisting
 * multi-turn chat histories across process invocations.
 */

#ifndef AICHAT_SESSION_H
#define AICHAT_SESSION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "aichat/llm.h"
#include <stddef.h>
#include <stdbool.h>

/**
 * @defgroup Session Session Persistence
 * @{
 */

/** Maximum messages per session */
#define SESSION_MAX_MESSAGES 4096

/** Maximum message content length */
#define SESSION_MAX_CONTENT  65536

/** Session handle */
typedef struct session* session_t;

/**
 * Create a new empty session
 * @param session_id Optional session identifier string (can be NULL)
 * @return Session handle or NULL on error
 */
session_t session_create(const char* session_id);

/**
 * Destroy session and free all resources
 * @param sess Session handle
 */
void session_destroy(session_t sess);

/**
 * Append a message to the session
 * @param sess    Session handle
 * @param role    Message role
 * @param content Message content (copied internally)
 * @return 0 on success, negative on error
 */
int session_append(session_t sess, message_role_t role, const char* content);

/**
 * Get the number of messages in the session
 * @param sess Session handle
 * @return Number of messages
 */
size_t session_length(const session_t sess);

/**
 * Get a message from the session
 * @param sess  Session handle
 * @param index Message index
 * @param out   Output message (pointers valid until next session_append/session_destroy)
 * @return 0 on success, negative on error
 */
int session_get(const session_t sess, size_t index, chat_message_t* out);

/**
 * Save session to a JSON file
 *
 * Format:
 * {
 *   "id": "...",
 *   "messages": [
 *     {"role": "user",      "content": "..."},
 *     {"role": "assistant", "content": "..."}
 *   ]
 * }
 *
 * @param sess Session handle
 * @param path File path to write
 * @return 0 on success, negative on error
 */
int session_save(const session_t sess, const char* path);

/**
 * Load session from a JSON file
 * @param path File path to read
 * @return Session handle or NULL on error
 */
session_t session_load(const char* path);

/**
 * Get session identifier string
 * @param sess Session handle
 * @return Identifier string or empty string if none
 */
const char* session_id(const session_t sess);

/**
 * Clear all messages from the session (preserves session ID)
 * @param sess Session handle
 */
void session_clear(session_t sess);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* AICHAT_SESSION_H */
