/*
 * SPDX-FileCopyrightText: 2025 Zeal 8-bit Computer <contact@zeal8bit.com>; David Higgins <zoul0813@me.com>
 *
 * SPDX-FileCopyrightText: 2026 Robert Maupin <chasesan@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * SPDX-FileContributor: Modified by Robert Maupin 2026
 */

#pragma once

#include <math.h>

#define KB 1024
#define CPUFREQ 10000000UL
#define TSTATES_US (1.0 / CPUFREQ * 1000000)

#define ONE_MILLIS us_to_tstates(1000)

#define US_TO_TSTATES(v) ((v) * 10)

#define BIT(val, n) (((val) >> (n)) & 1)
#define DIM(t) (sizeof(t) / sizeof(*t))

static inline unsigned long us_to_tstates(double us) {
    return (unsigned long)(us * 10);
}

#ifdef _WIN32
#define zstrdup _strdup
#else
#define zstrdup strdup
#endif
