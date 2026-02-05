/*
 * SPDX-FileCopyrightText: 2025 Zeal 8-bit Computer <contact@zeal8bit.com>; David Higgins <zoul0813@me.com>
 *
 * SPDX-FileCopyrightText: 2026 Robert Maupin <chasesan@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * SPDX-FileContributor: Modified by Robert Maupin 2026
 */

#include <stdint.h>
#include <string.h>

#ifdef _WIN32
/* Including windows.h can lead to conflicts with Raylib API, stdlib defines PATH_MAX */
#include <stdlib.h>
#define realpath(N, R) _fullpath((R), (N), _MAX_PATH)
#define HOME_VAR "APPDATA"
#define HOME_SANITIZE "%%APPDATA%%"
#define FOPEN_BINARY "b"
#define OPEN_BINARY O_BINARY
#elif __linux__
#include <linux/limits.h>
#include <unistd.h>
#define HOME_VAR "HOME"
#define HOME_SANITIZE "~"
#define FOPEN_BINARY ""
#define OPEN_BINARY 0
#elif __APPLE__
#include <limits.h>
#include <mach-o/dyld.h>
#define HOME_VAR "HOME"
#define HOME_SANITIZE "~"
#define FOPEN_BINARY ""
#define OPEN_BINARY 0
#elif PLATFORM_WEB
#define PATH_MAX 4096
#define HOME_VAR "HOME"
#define HOME_SANITIZE "~"
#define FOPEN_BINARY ""
#define OPEN_BINARY 0
#endif
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifdef _WIN32
static inline int os_mkdir(const char *path, int mode) {
    (void)mode;
    extern int mkdir(const char *);
    return mkdir(path);
}

#else

#define os_mkdir mkdir

#endif  // _WIN32

void get_executable_path(char *buffer, size_t size);
void get_executable_dir(char *buffer, size_t size);

int get_install_dir_file(char dst[PATH_MAX], const char *name);
const char *get_shaders_path(char dst[PATH_MAX], const char *name);

int path_exists(const char *path);
char *get_relative_path(const char *absolute_path);
const char *get_home_dir(void);
const char *get_config_dir(void);
const char *get_config_path(void);
const char *path_sanitize(const char *path);
