/**
 * @file function.cpp
 * @brief Function/tool-calling implementation
 *
 * Implements the function registry, system-prompt injection, model-output
 * parsing, and tool dispatch.  Shell-command tools are executed via popen(3).
 *
 * No external dependencies beyond the C/C++ standard library are used.
 */

#include "aichat/function.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <new>

/* ── Internal types ─────────────────────────────────────────────────────── */

/** Tool kind */
enum ToolKind {
    TOOL_CALLBACK = 0,
    TOOL_SHELL    = 1,
};

/** Single registered tool entry */
struct ToolEntry {
    char  name[FUNCTION_MAX_NAME];
    std::string description;
    std::string params_schema;

    ToolKind            kind;
    function_callback_t callback;
    void*               user_data;
    std::string         cmd_template; /**< Used when kind == TOOL_SHELL */
};

struct function_registry {
    std::vector<ToolEntry> tools;
};

/* ── Helpers ────────────────────────────────────────────────────────────── */

/**
 * Escape a string for embedding inside a JSON string value.
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
 * Extract the text between the first occurrence of @p open_tag and
 * @p close_tag in @p text.  Returns empty string if not found.
 */
static std::string extract_between(const char* text,
                                   const char* open_tag,
                                   const char* close_tag) {
    if (!text || !open_tag || !close_tag) {
        return "";
    }
    const char* start = strstr(text, open_tag);
    if (!start) {
        return "";
    }
    start += strlen(open_tag);
    const char* end = strstr(start, close_tag);
    if (!end) {
        return "";
    }
    return std::string(start, end - start);
}

/**
 * Very lightweight JSON string-value extractor.
 * Reads the value for @p key from a flat (non-nested) JSON object string.
 * Returns empty string if the key is not found.
 */
static std::string json_get_string(const std::string& json, const char* key) {
    std::string needle = std::string("\"") + key + "\"";
    const char* p = strstr(json.c_str(), needle.c_str());
    if (!p) {
        return "";
    }
    p += needle.size();
    /* Skip whitespace and ':' */
    while (*p && (*p == ' ' || *p == ':' || *p == '\t')) p++;
    if (*p != '"') {
        return "";
    }
    p++; /* skip opening quote */

    std::string out;
    while (*p && *p != '"') {
        if (*p == '\\' && *(p + 1)) {
            p++;
            switch (*p) {
                case '"':  out += '"';  break;
                case '\\': out += '\\'; break;
                case 'n':  out += '\n'; break;
                case 'r':  out += '\r'; break;
                case 't':  out += '\t'; break;
                default:   out += *p;  break;
            }
        } else {
            out += *p;
        }
        p++;
    }
    return out;
}

/**
 * Extract the raw JSON object for the "args" key.
 * Handles one level of nested braces.
 */
static std::string json_get_args_object(const std::string& json) {
    const char* p = strstr(json.c_str(), "\"args\"");
    if (!p) {
        return "{}";
    }
    p += 6; /* skip "args" */
    while (*p && *p != '{') p++;
    if (!*p) {
        return "{}";
    }

    /* Copy from '{' to matching '}', tracking nesting depth */
    const char* start = p;
    int depth = 0;
    do {
        if (*p == '{') depth++;
        else if (*p == '}') depth--;
        p++;
    } while (*p && depth > 0);

    return std::string(start, p - start);
}

/**
 * Shell-command callback: substitutes {args} in cmd_template and runs via popen.
 */
static int shell_callback(const char* args_json,
                          char* result_buf, size_t result_size,
                          void* user_data) {
    const char* tmpl = (const char*)user_data;
    if (!tmpl) {
        if (result_buf && result_size > 0) {
            snprintf(result_buf, result_size, "error: no command template");
        }
        return -1;
    }

    /* Build command: replace {args} with the actual JSON args */
    std::string cmd;
    const char* p = tmpl;
    const char* placeholder = "{args}";
    size_t plen = strlen(placeholder);

    while (*p) {
        if (strncmp(p, placeholder, plen) == 0) {
            /* Shell-escape the args_json by wrapping in single quotes,
             * replacing any embedded single quotes with '\'' */
            cmd += '\'';
            for (const char* q = args_json; *q; q++) {
                if (*q == '\'') {
                    cmd += "'\\''";
                } else {
                    cmd += *q;
                }
            }
            cmd += '\'';
            p += plen;
        } else {
            cmd += *p++;
        }
    }

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        if (result_buf && result_size > 0) {
            snprintf(result_buf, result_size, "error: popen failed");
        }
        return -1;
    }

    size_t written = 0;
    if (result_buf && result_size > 1) {
        written = fread(result_buf, 1, result_size - 1, pipe);
        result_buf[written] = '\0';

        /* Strip trailing newline */
        while (written > 0 && (result_buf[written - 1] == '\n' ||
                                result_buf[written - 1] == '\r')) {
            result_buf[--written] = '\0';
        }
    }

    int rc = pclose(pipe);
    return (rc == 0) ? 0 : -1;
}

/* ── Public API ─────────────────────────────────────────────────────────── */

extern "C" function_registry_t function_registry_create(void) {
    function_registry* reg = new (std::nothrow) function_registry();
    return reg;
}

extern "C" void function_registry_destroy(function_registry_t reg) {
    if (!reg) {
        return;
    }
    /* Free cmd_template user_data pointers allocated for shell tools */
    for (auto& tool : reg->tools) {
        if (tool.kind == TOOL_SHELL && tool.user_data) {
            free(tool.user_data);
            tool.user_data = nullptr;
        }
    }
    delete reg;
}

extern "C" int function_register(function_registry_t  reg,
                                  const char*          name,
                                  const char*          description,
                                  const char*          params_schema,
                                  function_callback_t  callback,
                                  void*                user_data) {
    if (!reg || !name || !callback) {
        return -1;
    }
    if (reg->tools.size() >= FUNCTION_MAX_TOOLS) {
        return -1;
    }

    ToolEntry entry;
    strncpy(entry.name, name, FUNCTION_MAX_NAME - 1);
    entry.name[FUNCTION_MAX_NAME - 1] = '\0';
    entry.description  = description ? description : "";
    entry.params_schema = params_schema ? params_schema : "{}";
    entry.kind         = TOOL_CALLBACK;
    entry.callback     = callback;
    entry.user_data    = user_data;

    reg->tools.push_back(std::move(entry));
    return 0;
}

extern "C" int function_register_shell(function_registry_t reg,
                                        const char*         name,
                                        const char*         description,
                                        const char*         cmd_template) {
    if (!reg || !name || !cmd_template) {
        return -1;
    }
    if (reg->tools.size() >= FUNCTION_MAX_TOOLS) {
        return -1;
    }

    ToolEntry entry;
    strncpy(entry.name, name, FUNCTION_MAX_NAME - 1);
    entry.name[FUNCTION_MAX_NAME - 1] = '\0';
    entry.description   = description ? description : "";
    entry.params_schema = "{}";
    entry.kind          = TOOL_SHELL;
    entry.callback      = shell_callback;
    entry.cmd_template  = cmd_template;
    /* Pass a heap copy of the template as user_data for the shell callback */
    entry.user_data     = strdup(cmd_template);

    reg->tools.push_back(std::move(entry));
    return 0;
}

extern "C" size_t function_count(const function_registry_t reg) {
    return reg ? reg->tools.size() : 0;
}

extern "C" char* function_get_tools_prompt(const function_registry_t reg) {
    if (!reg || reg->tools.empty()) {
        return nullptr;
    }

    std::string prompt;
    prompt.reserve(1024);

    prompt += "You have access to the following tools. "
              "To call a tool, emit exactly one block in this format "
              "and nothing else on the same turn:\n"
              "<tool_call>{\"name\":\"<tool_name>\","
              "\"args\":{<arguments>}}</tool_call>\n\n"
              "Available tools:\n";

    for (const auto& tool : reg->tools) {
        prompt += "- ";
        prompt += tool.name;
        prompt += ": ";
        prompt += tool.description;
        if (tool.params_schema != "{}" && !tool.params_schema.empty()) {
            prompt += "\n  Parameters: ";
            prompt += tool.params_schema;
        }
        prompt += "\n";
    }

    return strdup(prompt.c_str());
}

extern "C" bool function_has_tool_call(const char* model_output) {
    if (!model_output) {
        return false;
    }
    return strstr(model_output, "<tool_call>") != nullptr;
}

extern "C" int function_dispatch(function_registry_t reg,
                                  const char*         model_output,
                                  char*               result_buf,
                                  size_t              result_size) {
    if (!reg || !model_output) {
        return -1;
    }

    /* Extract tool call block */
    std::string call_json = extract_between(model_output, "<tool_call>", "</tool_call>");
    if (call_json.empty()) {
        return 0; /* no tool call present */
    }

    /* Parse tool name */
    std::string tool_name = json_get_string(call_json, "name");
    if (tool_name.empty()) {
        return -2;
    }

    /* Parse args object */
    std::string args_json = json_get_args_object(call_json);

    /* Look up tool */
    ToolEntry* found = nullptr;
    for (auto& tool : reg->tools) {
        if (strcmp(tool.name, tool_name.c_str()) == 0) {
            found = &tool;
            break;
        }
    }

    if (!found) {
        if (result_buf && result_size > 0) {
            snprintf(result_buf, result_size,
                     "<tool_result>{\"name\":\"%s\","
                     "\"result\":\"error: unknown tool\"}</tool_result>",
                     json_escape(tool_name).c_str());
        }
        return -3;
    }

    /* Invoke the callback */
    char raw_result[FUNCTION_MAX_RESULT] = {0};
    int rc = found->callback(args_json.c_str(), raw_result, sizeof(raw_result),
                             found->user_data);

    /* Format the result */
    if (result_buf && result_size > 0) {
        const char* result_str = (rc == 0) ? raw_result : "error: tool execution failed";
        snprintf(result_buf, result_size,
                 "<tool_result>{\"name\":\"%s\",\"result\":\"%s\"}"
                 "</tool_result>",
                 json_escape(tool_name).c_str(),
                 json_escape(std::string(result_str)).c_str());
    }

    return 1; /* one tool call dispatched */
}
