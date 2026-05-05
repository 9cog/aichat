/**
 * @file test_function.cpp
 * @brief Function/tool-calling subsystem tests
 */

#include "aichat/function.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

/* ── Sample callback ────────────────────────────────────────────────────── */

static int echo_callback(const char* args_json,
                         char*       result_buf,
                         size_t      result_size,
                         void*       user_data) {
    (void)user_data;
    snprintf(result_buf, result_size, "echo: %s", args_json ? args_json : "");
    return 0;
}

static int failing_callback(const char* args_json,
                             char* result_buf, size_t result_size,
                             void* user_data) {
    (void)args_json;
    (void)user_data;
    snprintf(result_buf, result_size, "intentional failure");
    return 1; /* non-zero = error */
}

/* ── Tests ──────────────────────────────────────────────────────────────── */

static int test_registry(void) {
    printf("Testing registry create / destroy...\n");

    function_registry_t reg = function_registry_create();
    assert(reg != NULL);
    assert(function_count(reg) == 0);

    function_registry_destroy(reg);
    function_registry_destroy(NULL); /* must be safe */

    printf("  PASS: registry create / destroy\n");
    return 0;
}

static int test_register(void) {
    printf("Testing function_register...\n");

    function_registry_t reg = function_registry_create();
    assert(reg != NULL);

    int rc = function_register(reg, "echo_tool", "Echoes arguments back",
                               "{\"input\": \"string\"}", echo_callback, NULL);
    assert(rc == 0);
    assert(function_count(reg) == 1);

    /* Duplicate name is allowed (registry does not enforce uniqueness here) */
    rc = function_register(reg, "another", "Another tool", NULL,
                           echo_callback, NULL);
    assert(rc == 0);
    assert(function_count(reg) == 2);

    /* NULL callback must be rejected */
    rc = function_register(reg, "bad", "bad tool", NULL, NULL, NULL);
    assert(rc < 0);
    assert(function_count(reg) == 2);

    function_registry_destroy(reg);
    printf("  PASS: function_register\n");
    return 0;
}

static int test_tools_prompt(void) {
    printf("Testing function_get_tools_prompt...\n");

    function_registry_t reg = function_registry_create();
    assert(reg != NULL);

    /* Empty registry should return NULL */
    char* prompt = function_get_tools_prompt(reg);
    assert(prompt == NULL);

    function_register(reg, "weather", "Get current weather",
                      "{\"city\": \"string\"}", echo_callback, NULL);
    function_register(reg, "calc", "Evaluate expression",
                      NULL, echo_callback, NULL);

    prompt = function_get_tools_prompt(reg);
    assert(prompt != NULL);

    /* Must mention both tools */
    assert(strstr(prompt, "weather") != NULL);
    assert(strstr(prompt, "calc") != NULL);
    /* Must contain the tool call protocol hint */
    assert(strstr(prompt, "<tool_call>") != NULL);

    free(prompt);
    function_registry_destroy(reg);
    printf("  PASS: function_get_tools_prompt\n");
    return 0;
}

static int test_dispatch(void) {
    printf("Testing function_dispatch...\n");

    function_registry_t reg = function_registry_create();
    assert(reg != NULL);

    function_register(reg, "echo_tool", "Echo tool", NULL, echo_callback, NULL);

    /* No tool call in output */
    char result[1024];
    int n = function_dispatch(reg, "Hello, this is plain text.", result, sizeof(result));
    assert(n == 0);

    /* Valid tool call */
    const char* call = "<tool_call>{\"name\":\"echo_tool\","
                       "\"args\":{\"msg\":\"hello\"}}</tool_call>";
    n = function_dispatch(reg, call, result, sizeof(result));
    assert(n == 1);

    /* Result must contain tool_result tags and the tool name */
    assert(strstr(result, "<tool_result>") != NULL);
    assert(strstr(result, "echo_tool") != NULL);

    /* Unknown tool */
    const char* unknown_call = "<tool_call>{\"name\":\"no_such_tool\","
                               "\"args\":{}}</tool_call>";
    n = function_dispatch(reg, unknown_call, result, sizeof(result));
    assert(n < 0);
    assert(strstr(result, "unknown tool") != NULL);

    /* NULL output – no crash */
    n = function_dispatch(reg, NULL, result, sizeof(result));
    assert(n < 0);

    function_registry_destroy(reg);
    printf("  PASS: function_dispatch\n");
    return 0;
}

static int test_has_tool_call(void) {
    printf("Testing function_has_tool_call...\n");

    assert(!function_has_tool_call(NULL));
    assert(!function_has_tool_call("plain text response"));
    assert( function_has_tool_call("<tool_call>{\"name\":\"x\"}</tool_call>"));
    assert(!function_has_tool_call("<tool_result>result</tool_result>"));

    printf("  PASS: function_has_tool_call\n");
    return 0;
}

static int test_shell_tool(void) {
    printf("Testing function_register_shell...\n");

    function_registry_t reg = function_registry_create();
    assert(reg != NULL);

    /* Register an echo shell tool */
    int rc = function_register_shell(reg, "sh_echo",
                                     "Echo via shell",
                                     "echo hello_from_shell");
    assert(rc == 0);
    assert(function_count(reg) == 1);

    /* Dispatch a call to it */
    const char* call = "<tool_call>{\"name\":\"sh_echo\","
                       "\"args\":{}}</tool_call>";
    char result[1024];
    int n = function_dispatch(reg, call, result, sizeof(result));
    assert(n == 1);
    assert(strstr(result, "<tool_result>") != NULL);
    /* The shell output "hello_from_shell" should appear in the result */
    assert(strstr(result, "hello_from_shell") != NULL);

    function_registry_destroy(reg);
    printf("  PASS: function_register_shell\n");
    return 0;
}

static int test_failing_callback(void) {
    printf("Testing failing callback result...\n");

    function_registry_t reg = function_registry_create();
    assert(reg != NULL);

    function_register(reg, "fail_tool", "Always fails", NULL,
                      failing_callback, NULL);

    const char* call = "<tool_call>{\"name\":\"fail_tool\","
                       "\"args\":{}}</tool_call>";
    char result[1024];
    /* dispatch returns 1 (one call dispatched) even when callback fails */
    int n = function_dispatch(reg, call, result, sizeof(result));
    assert(n == 1);
    /* Result should indicate an error */
    assert(strstr(result, "error") != NULL);

    function_registry_destroy(reg);
    printf("  PASS: failing callback result\n");
    return 0;
}

/* ── main ───────────────────────────────────────────────────────────────── */

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <test>\n", argv[0]);
        fprintf(stderr, "Tests: registry register tools_prompt dispatch "
                        "has_tool_call shell_tool failing_cb\n");
        return 1;
    }

    int ret = 0;

    if (strcmp(argv[1], "registry") == 0) {
        ret = test_registry();
    } else if (strcmp(argv[1], "register") == 0) {
        ret = test_register();
    } else if (strcmp(argv[1], "tools_prompt") == 0) {
        ret = test_tools_prompt();
    } else if (strcmp(argv[1], "dispatch") == 0) {
        ret = test_dispatch();
    } else if (strcmp(argv[1], "has_tool_call") == 0) {
        ret = test_has_tool_call();
    } else if (strcmp(argv[1], "shell_tool") == 0) {
        ret = test_shell_tool();
    } else if (strcmp(argv[1], "failing_cb") == 0) {
        ret = test_failing_callback();
    } else {
        fprintf(stderr, "Unknown test: %s\n", argv[1]);
        return 1;
    }

    return ret;
}
