/*
 * SPDX-FileCopyrightText: 2025 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-FileCopyrightText: 2026 Robert Maupin <chasesan@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * SPDX-FileContributor: Modified by Robert Maupin 2026
 */

#pragma once

#include <errno.h>
#include <stdio.h>

#define log_printf(...) printf(__VA_ARGS__)
#define log_err_printf(...) fprintf(stderr, __VA_ARGS__)
#define log_perror(fmt, ...) fprintf(stderr, fmt ": %s\n" __VA_OPT__(, ) __VA_ARGS__, strerror(errno))
