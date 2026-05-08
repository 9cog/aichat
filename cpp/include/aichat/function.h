/**
 * @file function.h
 * @brief Function/tool-calling subsystem
 *
 * Provides a registry of named tools that the LLM can invoke, a system-prompt
 * fragment that describes the available tools to the model, and a dispatcher
 * that parses model output and executes matching tool callbacks.
 *
 * Tool call protocol (in model output):
 * @verbatim
 *   <tool_call>{"name":"<tool>","args":{...}}</tool_call>
 * @endverbatim
 *
 * Tool result (injected back as a user message):
 * @verbatim
 *   <tool_result>{"name":"<tool>","result":"..."}</tool_result>
 * @endverbatim
 *
 * Typical usage:
 * @code
 *   function_registry_t reg = function_registry_create();
 *
 *   // Register a callback tool
 *   function_register(reg, "weather", "Get current weather for a city",
 *                     "{\"city\": \"string\"}", my_weather_cb, NULL);
 *
 *   // Register a shell command tool
 *   function_register_shell(reg, "calc", "Evaluate a math expression",
 *                            "echo $(( {args} ))");
 *
 *   // Prepend tools prompt to system message
 *   char* tools_prompt = function_get_tools_prompt(reg);
 *   // ... pass tools_prompt to llm ...
 *   free(tools_prompt);
 *
 *   // After model responds:
 *   char result[4096];
 *   int n = function_dispatch(reg, model_output, result, sizeof(result));
 *   if (n > 0) {
 *       // inject result back to model as a user message
 *   }
 *
 *   function_registry_destroy(reg);
 * @endcode
 */

#ifndef AICHAT_FUNCTION_H
#define AICHAT_FUNCTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

/**
 * @defgroup Function Function/Tool Calling
 * @{
 */

/** Maximum number of tools that can be registered in one registry */
#define FUNCTION_MAX_TOOLS 64

/** Maximum length of a tool name */
#define FUNCTION_MAX_NAME 64

/** Maximum length of a tool result written into result_buf */
#define FUNCTION_MAX_RESULT 8192

/**
 * Callback invoked when the model requests a tool call.
 *
 * @param args_json   Raw JSON object string from the model (e.g. {"city":"Paris"})
 * @param result_buf  Buffer to write the result into (null-terminated string)
 * @param result_size Size of result_buf in bytes
 * @param user_data   Opaque pointer supplied at registration time
 * @return 0 on success, non-zero on error
 */
typedef int (*function_callback_t)(const char* args_json,
                                   char*       result_buf,
                                   size_t      result_size,
                                   void*       user_data);

/** Function registry handle (opaque) */
typedef struct function_registry* function_registry_t;

/**
 * Create a new, empty function registry.
 * @return Registry handle, or NULL on allocation failure
 */
function_registry_t function_registry_create(void);

/**
 * Destroy a registry and free all associated memory.
 * @param reg Registry handle (safe to call with NULL)
 */
void function_registry_destroy(function_registry_t reg);

/**
 * Register a named tool backed by a C callback.
 *
 * @param reg           Registry handle
 * @param name          Unique tool name (alphanumeric + underscore)
 * @param description   One-line description shown to the model
 * @param params_schema JSON schema string for the args object (may be NULL)
 * @param callback      Function invoked on tool call
 * @param user_data     Opaque pointer forwarded to the callback
 * @return 0 on success, -1 if registry is full or arguments are invalid
 */
int function_register(function_registry_t  reg,
                      const char*          name,
                      const char*          description,
                      const char*          params_schema,
                      function_callback_t  callback,
                      void*                user_data);

/**
 * Register a named tool that executes a shell command.
 *
 * The shell command template may contain the literal token @c {args} which
 * is replaced at call time with the raw JSON args string supplied by the
 * model.  Output written to stdout by the command becomes the tool result.
 *
 * @param reg          Registry handle
 * @param name         Unique tool name
 * @param description  One-line description shown to the model
 * @param cmd_template Shell command template (e.g. "echo {args}")
 * @return 0 on success, negative on error
 */
int function_register_shell(function_registry_t reg,
                             const char*         name,
                             const char*         description,
                             const char*         cmd_template);

/**
 * Return the number of tools registered in the registry.
 * @param reg Registry handle
 * @return Tool count
 */
size_t function_count(const function_registry_t reg);

/**
 * Build a system-prompt fragment that lists all registered tools.
 *
 * The returned string must be freed by the caller with free().
 *
 * @param reg Registry handle
 * @return Heap-allocated string, or NULL on error
 */
char* function_get_tools_prompt(const function_registry_t reg);

/**
 * Parse model output, dispatch any embedded tool call, and write the result.
 *
 * The function scans @p model_output for a @c <tool_call>...</tool_call>
 * block.  If found, the named tool is looked up and invoked.  The formatted
 * @c <tool_result>...</tool_result> string is written to @p result_buf.
 *
 * @param reg          Registry handle
 * @param model_output Full model response text to scan
 * @param result_buf   Buffer to receive the formatted result (may be NULL)
 * @param result_size  Size of result_buf in bytes
 * @return Number of tool calls dispatched (0 or 1), or negative on error
 */
int function_dispatch(function_registry_t reg,
                      const char*         model_output,
                      char*               result_buf,
                      size_t              result_size);

/**
 * Quick check: does the model output contain a tool call tag?
 * @param model_output Model response text
 * @return true if a @c <tool_call> block is present
 */
bool function_has_tool_call(const char* model_output);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* AICHAT_FUNCTION_H */
