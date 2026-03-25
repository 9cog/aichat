/**
 * @file session.cpp
 * @brief Session persistence - JSON-based conversation history
 *
 * Saves and restores chat sessions to/from a simple JSON file so that
 * multi-turn conversations can survive process restarts.
 *
 * The JSON format is intentionally minimal and human-readable:
 *
 *   {
 *     "id": "my-session",
 *     "messages": [
 *       {"role": "system",    "content": "You are a helpful assistant."},
 *       {"role": "user",      "content": "Hello"},
 *       {"role": "assistant", "content": "Hi there!"}
 *     ]
 *   }
 *
 * The parser is hand-written (no external JSON library) using only the C
 * standard library so that the module has zero additional dependencies.
 */

#include "aichat/session.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <new>

/* ── Internal types ─────────────────────────────────────────────────────── */

struct SessionMessage {
    message_role_t role;
    std::string    content;
};

struct session {
    std::string                  id;
    std::vector<SessionMessage>  messages;
};

/* ── Helpers ────────────────────────────────────────────────────────────── */

static const char* role_to_str(message_role_t role) {
    switch (role) {
        case ROLE_SYSTEM:    return "system";
        case ROLE_USER:      return "user";
        case ROLE_ASSISTANT: return "assistant";
        default:             return "user";
    }
}

static message_role_t str_to_role(const char* s) {
    if (strcmp(s, "system") == 0)    return ROLE_SYSTEM;
    if (strcmp(s, "assistant") == 0) return ROLE_ASSISTANT;
    return ROLE_USER;
}

/**
 * Escape a string for JSON output.
 * Handles backslash, double-quote, and common control characters.
 */
static std::string json_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 16);
    for (unsigned char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += (char)c;
                }
                break;
        }
    }
    return out;
}

/**
 * Minimal JSON string parser: reads a JSON string value from *p, advances *p
 * past the closing quote, and returns the unescaped string.
 * Returns empty string on parse error.
 */
static std::string parse_json_string(const char** p) {
    /* Skip whitespace */
    while (**p && **p != '"') {
        (*p)++;
    }

    if (**p != '"') {
        return "";
    }
    (*p)++; /* skip opening quote */

    std::string out;
    while (**p && **p != '"') {
        if (**p == '\\') {
            (*p)++;
            switch (**p) {
                case '"':  out += '"';  break;
                case '\\': out += '\\'; break;
                case '/':  out += '/';  break;
                case 'n':  out += '\n'; break;
                case 'r':  out += '\r'; break;
                case 't':  out += '\t'; break;
                case 'u': {
                    /* \uXXXX – simplified: just read 4 hex digits as-is */
                    char hex[5] = {0};
                    for (int i = 0; i < 4 && (*p)[1]; i++) {
                        (*p)++;
                        hex[i] = **p;
                    }
                    /* Convert to UTF-8 (BMP only) */
                    unsigned int cp = (unsigned int)strtoul(hex, nullptr, 16);
                    if (cp < 0x80) {
                        out += (char)cp;
                    } else if (cp < 0x800) {
                        out += (char)(0xC0 | (cp >> 6));
                        out += (char)(0x80 | (cp & 0x3F));
                    } else {
                        out += (char)(0xE0 | (cp >> 12));
                        out += (char)(0x80 | ((cp >> 6) & 0x3F));
                        out += (char)(0x80 | (cp & 0x3F));
                    }
                    break;
                }
                default:
                    out += **p;
                    break;
            }
        } else {
            out += **p;
        }
        (*p)++;
    }

    if (**p == '"') {
        (*p)++; /* skip closing quote */
    }

    return out;
}

/* ── Public API ─────────────────────────────────────────────────────────── */

extern "C" session_t session_create(const char* session_id) {
    session* s = new (std::nothrow) session();
    if (!s) {
        return nullptr;
    }
    s->id = session_id ? session_id : "";
    return s;
}

extern "C" void session_destroy(session_t sess) {
    delete sess;
}

extern "C" int session_append(session_t sess, message_role_t role, const char* content) {
    if (!sess || !content) {
        return -1;
    }
    if (sess->messages.size() >= SESSION_MAX_MESSAGES) {
        return -2;
    }
    if (strlen(content) >= SESSION_MAX_CONTENT) {
        return -3;
    }
    SessionMessage msg;
    msg.role    = role;
    msg.content = content;
    sess->messages.push_back(std::move(msg));
    return 0;
}

extern "C" size_t session_length(const session_t sess) {
    return sess ? sess->messages.size() : 0;
}

extern "C" int session_get(const session_t sess, size_t index, chat_message_t* out) {
    if (!sess || !out || index >= sess->messages.size()) {
        return -1;
    }
    const SessionMessage& m = sess->messages[index];
    out->role    = m.role;
    out->content = m.content.c_str();
    return 0;
}

extern "C" int session_save(const session_t sess, const char* path) {
    if (!sess || !path) {
        return -1;
    }

    FILE* f = fopen(path, "w");
    if (!f) {
        return -2;
    }

    fprintf(f, "{\n");
    fprintf(f, "  \"id\": \"%s\",\n", json_escape(sess->id).c_str());
    fprintf(f, "  \"messages\": [\n");

    for (size_t i = 0; i < sess->messages.size(); i++) {
        const SessionMessage& m = sess->messages[i];
        bool last = (i + 1 == sess->messages.size());
        fprintf(f, "    {\"role\": \"%s\", \"content\": \"%s\"}%s\n",
                role_to_str(m.role),
                json_escape(m.content).c_str(),
                last ? "" : ",");
    }

    fprintf(f, "  ]\n");
    fprintf(f, "}\n");

    fclose(f);
    return 0;
}

extern "C" session_t session_load(const char* path) {
    if (!path) {
        return nullptr;
    }

    FILE* f = fopen(path, "r");
    if (!f) {
        return nullptr;
    }

    /* Read entire file into memory */
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize <= 0 || fsize > 64 * 1024 * 1024) {
        fclose(f);
        return nullptr;
    }

    std::string buf(fsize, '\0');
    size_t n = fread(&buf[0], 1, fsize, f);
    fclose(f);
    buf.resize(n);

    /* Parse session */
    session* s = new (std::nothrow) session();
    if (!s) {
        return nullptr;
    }

    const char* p = buf.c_str();

    /* Locate "id" field */
    const char* id_key = strstr(p, "\"id\"");
    if (id_key) {
        const char* after = id_key + 4;
        /* Skip : and whitespace */
        while (*after && *after != '"') after++;
        s->id = parse_json_string(&after);
    }

    /* Locate "messages" array */
    const char* msgs_start = strstr(p, "\"messages\"");
    if (!msgs_start) {
        return s; /* empty messages, but valid session */
    }
    msgs_start = strchr(msgs_start, '[');
    if (!msgs_start) {
        return s;
    }
    msgs_start++; /* skip '[' */

    const char* cur = msgs_start;

    /* Iterate over message objects */
    while (*cur) {
        /* Find next '{' */
        while (*cur && *cur != '{' && *cur != ']') cur++;
        if (!*cur || *cur == ']') break;
        cur++; /* skip '{' */

        std::string role_str;
        std::string content_str;

        /* Parse key-value pairs inside this object */
        for (int kv = 0; kv < 2; kv++) {
            /* Find key */
            while (*cur && *cur != '"' && *cur != '}') cur++;
            if (!*cur || *cur == '}') break;

            std::string key = parse_json_string(&cur);

            /* Find ':' */
            while (*cur && *cur != ':') cur++;
            if (!*cur) break;
            cur++; /* skip ':' */

            std::string val = parse_json_string(&cur);

            if (key == "role") {
                role_str = val;
            } else if (key == "content") {
                content_str = val;
            }
        }

        if (!role_str.empty() && !content_str.empty()) {
            SessionMessage msg;
            msg.role    = str_to_role(role_str.c_str());
            msg.content = content_str;
            s->messages.push_back(std::move(msg));
        }

        /* Advance past '}' */
        while (*cur && *cur != '}') cur++;
        if (*cur == '}') cur++;
    }

    return s;
}

extern "C" const char* session_id(const session_t sess) {
    return sess ? sess->id.c_str() : "";
}

extern "C" void session_clear(session_t sess) {
    if (sess) {
        sess->messages.clear();
    }
}
