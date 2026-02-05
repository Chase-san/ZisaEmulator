/*
 * SPDX-FileCopyrightText: 2025 Zeal 8-bit Computer <contact@zeal8bit.com>;
 * David Higgins <zoul0813@me.com>
 *
 * SPDX-FileCopyrightText: 2026 Robert Maupin <chasesan@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * SPDX-FileContributor: Modified by Robert Maupin 2026
 */

#include "utils/paths.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static char path_buffer[PATH_MAX] = {0};

#ifdef _WIN32
#include <windows.h>

void get_executable_path(char *buffer, size_t size) {
    GetModuleFileName(NULL, buffer, (DWORD)size);
}

char *getcwd(char *buf, size_t size) {
    DWORD ret = GetCurrentDirectoryA((DWORD)size, buf);
    if (ret == 0) {
        errno = EACCES;
        return NULL;
    }
    if (ret >= size) {
        errno = ERANGE;
        return NULL;
    }
    return buf;
}
#elif __linux__
#include <limits.h>
#include <unistd.h>

void get_executable_path(char *buffer, size_t size) {
    ssize_t len = readlink("/proc/self/exe", buffer, size - 1);
    if (len != -1) {
        buffer[len] = '\0';
    }
}
#elif __APPLE__
#include <limits.h>
#include <mach-o/dyld.h>

void get_executable_path(char *buffer, size_t size) {
    uint32_t bufsize = (uint32_t)size;
    if (_NSGetExecutablePath(buffer, &bufsize) != 0) {
        fprintf(stderr, "Buffer too small; need size %u\n", bufsize);
    }
}
#elif PLATFORM_WEB
void get_executable_path(char *buffer, size_t size) {
    if (size > 1) {
        buffer[0] = '/';
        buffer[1] = 0;
    }
}
#else
#error "Unsupported platform"
#endif

/**
 * A simple cross-platform implementation of dirname.
 * @author Robert Maupin (2026)
 */
static char *dirname(char *path) {
    static char dot[] = ".";
    char *last_slash;

    if (path == NULL || *path == '\0') {
        return dot;
    }

    /* Strip trailing slashes */
    last_slash = path + strlen(path) - 1;
    while (last_slash > path && (*last_slash == '/' || *last_slash == '\\')) {
        *last_slash-- = '\0';
    }

    /* Find last slash */
    last_slash = strrchr(path, '/');
    char *last_bslash = strrchr(path, '\\');
    if (last_bslash > last_slash) {
        last_slash = last_bslash;
    }

    if (last_slash == NULL) {
        return dot;
    }

    /* Handle root paths like "C:\" or "\" */
    if (last_slash == path || (last_slash == path + 2 && path[1] == ':')) {
        last_slash[1] = '\0';
        return path;
    }

    /* Strip trailing slashes from result */
    *last_slash = '\0';

    /* Handle "C:foo" -> "C:" */
    if (strlen(path) == 2 && path[1] == ':') {
        return path;
    }

    return path;
}

void get_executable_dir(char *buffer, size_t size) {
    get_executable_path(buffer, size);
    const char *dir = dirname(buffer);
    /* strcpy doesn't handle overlapping pointers properly */
    if (strlen(dir) + 1 > size) {
        return;  // too big
    }
    memmove(buffer, dir, strlen(dir) + 1);
}

int get_install_dir_file(char dst[PATH_MAX], const char *name) {
    /* Only get it once */
    if (path_buffer[0] == 0) {
        get_executable_dir(path_buffer, PATH_MAX);
    }
    const int wrote = snprintf(dst, PATH_MAX, "%s/%s", path_buffer, name);
    return wrote < PATH_MAX;
}

int path_exists(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

char *get_relative_path(const char *absolute_path) {
    static char relative_path[PATH_MAX];
    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        fprintf(stderr, "Could not get current working directory: %s\n", strerror(errno));
        return NULL;
    }

    char abs_cwd[PATH_MAX];
    char abs_path[PATH_MAX];

    if (realpath(cwd, abs_cwd) == NULL || realpath(absolute_path, abs_path) == NULL) {
        fprintf(stderr, "Could not resolve paths: %s\n", strerror(errno));
        return NULL;
    }

    const char *cwd_ptr = abs_cwd;
    const char *path_ptr = abs_path;

    // Find the common prefix
    while (*cwd_ptr && *path_ptr && *cwd_ptr == *path_ptr) {
        cwd_ptr++;
        path_ptr++;
    }

    // If they are identical → return "."
    if (*cwd_ptr == '\0' && *path_ptr == '\0') {
        strcpy(relative_path, ".");
        return relative_path;
    }

    // Count the remaining directories in cwd
    int up_levels = 0;
    for (const char *p = cwd_ptr; *p != '\0'; p++) {
        if (*p == '/') {
            up_levels++;
        }
    }

    // Construct the relative path
    char *rel_ptr = relative_path;
    for (int i = 0; i < up_levels; i++) {
        strcpy(rel_ptr, "../");
        rel_ptr += 3;
    }
    strcpy(rel_ptr, path_ptr);

    return relative_path;
}

const char *get_home_dir(void) {
    const char *home = getenv(HOME_VAR);
    if (!home) {
        fprintf(stderr, HOME_VAR " environment variable not set\n");
        return NULL;
    }
    return home;
}

const char *get_config_dir(void) {
    static char path[PATH_MAX];
    const char *home = get_home_dir();
    snprintf(path, sizeof(path), "%s/.zeal8bit", home);

    if (os_mkdir(path, 0755) != 0 && errno != EEXIST) {
        perror("mkdir");
        return NULL;
    }

    return path;
}

const char *get_config_path(void) {
    static char path[PATH_MAX];
    const char *config_dir = get_config_dir();

    if (os_mkdir(config_dir, 0755) != 0 && errno != EEXIST) {
        perror("mkdir");
        return NULL;
    }

    snprintf(path, sizeof(path), "%s/zeal.ini", config_dir);
    return path;
}

const char *path_sanitize(const char *path) {
    static char sanitized[PATH_MAX];
    const char *home = get_home_dir();
    size_t home_len = home ? strlen(home) : 0;

    if (home && strncmp(path, home, home_len) == 0) {
        snprintf(sanitized, sizeof(sanitized), HOME_SANITIZE "/%s", path + home_len + 1);
    } else {
        snprintf(sanitized, sizeof(sanitized), "%s", path);
    }

    return sanitized;
}
