// Startup problems, in one place.
//
// Anything that goes wrong while the mod sets itself up (a hook that would not attach, a debug
// setting left over in config.json, a model that is missing parts) lands here, once, keyed by a
// short id. The Co-op menu lists them in a "Problems" box and a toast points at it, so nothing
// fails silently, and features that depend on a failed hook can ask coop_issue_has() and turn
// themselves off.

#include "mod.hpp"
#include "print.hpp"

#include "mods/svc/config.h"

#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace {

struct Issue {
    std::string key;
    std::string text;
    CoopIssueKind kind = kIssueHook;
};

std::vector<Issue> s_issues;
uint32_t s_revision = 0;
bool s_toasted = false;

const char* kind_label(CoopIssueKind kind) {
    switch (kind) {
    case kIssueHook: return "Hook";
    case kIssueDebug: return "Debug";
    case kIssueModel: return "Model";
    case kIssueSync: return "Sync";
    }
    return "Problem";
}

bool env_allows_debug() {
    const char* v = std::getenv("COOP_ALLOW_DEBUG");
    return v != nullptr && v[0] != '\0' && std::strcmp(v, "0") != 0;
}

bool is_debug_name(const char* name) {
    return name != nullptr && std::strncmp(name, "debug_", 6) == 0;
}

std::string value_text(const ConfigVarDesc& desc, ConfigVarHandle handle, bool* nonDefault) {
    *nonDefault = false;
    switch (desc.type) {
    case CONFIG_VAR_BOOL: {
        bool v = desc.default_bool;
        svc_config->get_bool(mod_ctx, handle, &v);
        *nonDefault = v != desc.default_bool;
        return v ? "on" : "off";
    }
    case CONFIG_VAR_INT: {
        int64_t v = desc.default_int;
        svc_config->get_int(mod_ctx, handle, &v);
        *nonDefault = v != desc.default_int;
        return std::to_string(v);
    }
    default:
        return "";
    }
}

void reset_to_default(const ConfigVarDesc& desc, ConfigVarHandle handle) {
    switch (desc.type) {
    case CONFIG_VAR_BOOL: svc_config->set_bool(mod_ctx, handle, desc.default_bool); break;
    case CONFIG_VAR_INT: svc_config->set_int(mod_ctx, handle, desc.default_int); break;
    default: break;
    }
}

}

bool coop_debug_allowed() {
#if defined(COOP_PUBLIC_BUILD) && COOP_PUBLIC_BUILD
    return env_allows_debug();
#else
    return true;
#endif
}

ModResult coop_register_var(ModContext* ctx, const ConfigVarDesc* desc, ConfigVarHandle* out) {
    if (svc_config == nullptr || desc == nullptr || out == nullptr) return MOD_INVALID_ARGUMENT;
    const ModResult r = svc_config->register_var(ctx, desc, out);
    if (r != MOD_OK || !is_debug_name(desc->name)) return r;

    bool nonDefault = false;
    const std::string value = value_text(*desc, *out, &nonDefault);
    if (!nonDefault) return r;

    const std::string key = std::string("debug:") + desc->name;
    if (coop_debug_allowed()) {
        coop_log::warn("coop_mod: [DEBUG] '{}' is {} - this is a debug setting, not for normal play",
            desc->name, value);
        coop_issue(key.c_str(), kIssueDebug,
            (std::string(desc->name) + " is " + value +
                ". This is a debug setting and can break your game. Set it back to " +
                (desc->type == CONFIG_VAR_BOOL ? (desc->default_bool ? "on" : "off")
                                               : std::to_string(desc->default_int)) +
                " unless you are running a test.").c_str());
        return r;
    }

    reset_to_default(*desc, *out);
    coop_log::warn("coop_mod: [DEBUG] '{}' was {} in config.json - turned it off (release build; "
        "launch with COOP_ALLOW_DEBUG=1 to keep debug settings)", desc->name, value);
    coop_issue(key.c_str(), kIssueDebug,
        (std::string(desc->name) + " was " + value +
            " in an old config file. It is a debug setting, so it has been turned off.").c_str());
    return r;
}

void coop_issue(const char* key, CoopIssueKind kind, const char* text) {
    if (key == nullptr || text == nullptr) return;
    for (Issue& i : s_issues) {
        if (i.key != key) continue;
        if (i.text != text) {
            i.text = text;
            ++s_revision;
        }
        return;
    }
    s_issues.push_back(Issue{key, text, kind});
    ++s_revision;
    coop_log::warn("coop_mod: [ISSUE] {}: {}", kind_label(kind), text);
}

void coop_issue_hook(const char* key, bool attached, const char* consequence) {
    if (attached) return;
    coop_issue(key, kIssueHook, consequence != nullptr ? consequence : key);
}

void coop_issue_clear(const char* key) {
    if (key == nullptr) return;
    for (size_t i = 0; i < s_issues.size(); ++i) {
        if (s_issues[i].key != key) continue;
        s_issues.erase(s_issues.begin() + static_cast<std::ptrdiff_t>(i));
        ++s_revision;
        return;
    }
}

void coop_issue_clear_prefix(const char* prefix) {
    if (prefix == nullptr) return;
    const size_t n = std::strlen(prefix);
    for (size_t i = 0; i < s_issues.size();) {
        if (s_issues[i].key.compare(0, n, prefix) == 0) {
            s_issues.erase(s_issues.begin() + static_cast<std::ptrdiff_t>(i));
            ++s_revision;
        } else {
            ++i;
        }
    }
}

bool coop_issue_has(const char* key) {
    if (key == nullptr) return false;
    for (const Issue& i : s_issues) {
        if (i.key == key) return true;
    }
    return false;
}

size_t coop_issue_count() {
    return s_issues.size();
}

uint32_t coop_issue_revision() {
    return s_revision;
}

std::string coop_issues_text() {
    if (s_issues.empty()) return "";
    std::string out;
    for (const Issue& i : s_issues) {
        if (!out.empty()) out += "\n";
        out += "- ";
        out += kind_label(i.kind);
        out += ": ";
        out += i.text;
    }
    return out;
}

void coop_issues_update() {
    if (s_toasted || s_issues.empty()) return;
    s_toasted = true;
    const std::string body = std::to_string(s_issues.size()) +
                             (s_issues.size() == 1 ? " problem" : " problems") +
                             " at startup. Open Co-op to see what was turned off.";
    coop_toast("Co-op", body.c_str());
}
