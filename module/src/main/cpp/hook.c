#include <android/log.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "lsplt.h"

#define LOG_TAG "auditpatch"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static int (*old_vasprintf)(char **strp, const char *fmt, va_list ap) = NULL;

static bool has_quote_after(const char *pos, size_t match_len) {
    const char *end = pos + match_len;
    while (*end != '\0') {
        if (*end == '"') {
            return true;
        }
        end++;
    }
    return false;
}

static int my_vasprintf(char **strp, const char *fmt, va_list ap) {
    // https://cs.android.com/android/platform/superproject/main/+/main:system/logging/logd/LogAudit.cpp;l=210
    int result = old_vasprintf(strp, fmt, ap);

    if (result > 0 && *strp) {
        // https://cs.android.com/android/platform/superproject/main/+/main:external/selinux/libselinux/src/android/android_seapp.c;l=694
        const char *target_context = "tcontext=u:r:priv_app:s0:c512,c768";
        const char *source_contexts[] = {
            "tcontext=u:r:su:s0",
            "tcontext=u:r:magisk:s0"
        };
        size_t source_contexts_len = sizeof(source_contexts) / sizeof(source_contexts[0]);
        size_t target_len = strlen(target_context);

        for (size_t i = 0; i < source_contexts_len; i++) {
            const char *source = source_contexts[i];
            char *pos = strstr(*strp, source);

            if (pos && !has_quote_after(pos, strlen(source))) {
                size_t source_len = strlen(source);
                size_t extra_space = (target_len > source_len) ? (target_len - source_len) : 0;

                // Reverse double space in case
                char *new_str = malloc(result + 2 * extra_space + 1);
                if (!new_str) return result;

                strcpy(new_str, *strp);
                pos = new_str + (pos - *strp);

                if (source_len != target_len) {
                    memmove(pos + target_len, pos + source_len,
                            strlen(pos + source_len) + 1);
                }
                memcpy(pos, target_context, target_len);

                free(*strp);
                *strp = new_str;
                return (int)strlen(new_str);
            }
        }
    }

    return result;
}

static bool find_libc_info(dev_t *dev, ino_t *inode) {
    struct lsplt_map_info *maps = lsplt_scan_maps("self");
    if (!maps) {
        LOGE("Failed to scan maps");
        return false;
    }

    bool found = false;
    for (size_t i = 0; i < maps->length; i++) {
        if (maps->maps[i].path && strstr(maps->maps[i].path, "libc.so")) {
            *dev = maps->maps[i].dev;
            *inode = maps->maps[i].inode;
            found = true;
            break;
        }
    }

    lsplt_free_maps(maps);
    return found;
}

__attribute__((constructor))
static void init(void) {
    LOGI("Initializing audit log patch...");

    dev_t dev;
    ino_t inode;
    
    if (!find_libc_info(&dev, &inode)) {
        LOGE("Failed to find libc.so");
        return;
    }

    if (!lsplt_register_hook(dev, inode, "vasprintf", (void *)my_vasprintf, (void **)&old_vasprintf)) {
        LOGE("Failed to register vasprintf hook");
        return;
    }

    if (!lsplt_commit_hook()) {
        LOGE("Failed to commit hooks");
        return;
    }

    LOGI("PLT hook success");
}
