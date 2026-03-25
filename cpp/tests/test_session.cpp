/**
 * @file test_session.cpp
 * @brief Session persistence unit tests
 */

#include "aichat/session.h"
#include <cstdio>
#include <cstring>
#include <cassert>
#include <cstdlib>

/* Test basic session creation and message append */
static int test_create(void) {
    printf("Testing session create/append/length...\n");

    session_t s = session_create("test-session");
    assert(s != nullptr);
    assert(strcmp(session_id(s), "test-session") == 0);
    assert(session_length(s) == 0);

    int ret = session_append(s, ROLE_SYSTEM, "You are a helpful assistant.");
    assert(ret == 0);
    assert(session_length(s) == 1);

    ret = session_append(s, ROLE_USER, "Hello, world!");
    assert(ret == 0);
    assert(session_length(s) == 2);

    ret = session_append(s, ROLE_ASSISTANT, "Hi there! How can I help?");
    assert(ret == 0);
    assert(session_length(s) == 3);

    /* Read back */
    chat_message_t msg;

    ret = session_get(s, 0, &msg);
    assert(ret == 0);
    assert(msg.role == ROLE_SYSTEM);
    assert(strcmp(msg.content, "You are a helpful assistant.") == 0);

    ret = session_get(s, 1, &msg);
    assert(ret == 0);
    assert(msg.role == ROLE_USER);
    assert(strcmp(msg.content, "Hello, world!") == 0);

    ret = session_get(s, 2, &msg);
    assert(ret == 0);
    assert(msg.role == ROLE_ASSISTANT);
    assert(strcmp(msg.content, "Hi there! How can I help?") == 0);

    /* Out-of-bounds get */
    ret = session_get(s, 99, &msg);
    assert(ret < 0);

    /* Clear */
    session_clear(s);
    assert(session_length(s) == 0);
    assert(strcmp(session_id(s), "test-session") == 0);

    /* NULL safety */
    session_destroy(nullptr);
    assert(session_length(nullptr) == 0);

    session_destroy(s);

    printf("  PASS: session create/append/length/get/clear\n");
    return 0;
}

/* Test save and load roundtrip */
static int test_persist(void) {
    printf("Testing session save/load roundtrip...\n");

    const char* tmp_path = "/tmp/aichat_test_session.json";

    /* Build a session with special characters */
    session_t s = session_create("persist-test");
    assert(s != nullptr);

    session_append(s, ROLE_SYSTEM,    "Act as a C++ expert.");
    session_append(s, ROLE_USER,      "What is \"RAII\"?");
    session_append(s, ROLE_ASSISTANT, "RAII stands for:\n1. Resource\n2. Acquisition\n3. Is\n4. Initialization");
    session_append(s, ROLE_USER,      "Thanks! Can you show a\\ncode example?");

    assert(session_length(s) == 4);

    /* Save */
    int ret = session_save(s, tmp_path);
    assert(ret == 0);

    session_destroy(s);

    /* Load */
    session_t s2 = session_load(tmp_path);
    assert(s2 != nullptr);
    assert(strcmp(session_id(s2), "persist-test") == 0);
    assert(session_length(s2) == 4);

    chat_message_t msg;

    session_get(s2, 0, &msg);
    assert(msg.role == ROLE_SYSTEM);
    assert(strcmp(msg.content, "Act as a C++ expert.") == 0);

    session_get(s2, 1, &msg);
    assert(msg.role == ROLE_USER);
    assert(strcmp(msg.content, "What is \"RAII\"?") == 0);

    session_get(s2, 2, &msg);
    assert(msg.role == ROLE_ASSISTANT);
    /* Verify newlines were preserved */
    assert(strchr(msg.content, '\n') != nullptr);

    session_get(s2, 3, &msg);
    assert(msg.role == ROLE_USER);

    session_destroy(s2);

    /* Clean up temp file */
    remove(tmp_path);

    /* Load non-existent file returns NULL */
    session_t s3 = session_load("/tmp/nonexistent_aichat_session_xyz.json");
    assert(s3 == nullptr);

    printf("  PASS: session save/load roundtrip\n");
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <test>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "create")  == 0) return test_create();
    if (strcmp(argv[1], "persist") == 0) return test_persist();

    fprintf(stderr, "Unknown test: %s\n", argv[1]);
    return 1;
}
