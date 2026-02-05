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

#define RINI_VALUE_DELIMITER '='
#define RINI_IMPLEMENTATION

#include "utils/config.h"

#include "hw/zvb/zvb.h"
#include "raylib.h"
#include "utils/log.h"
#include "utils/paths.h"

#if CONFIG_ENABLE_DEBUGGER
#include "debugger/debugger_ui.h"
#endif  // CONFIG_ENABLE_DEBUGGER

config_t config = {
    .arguments =
        {
            .config_path = "zeal.ini",
            .rom_filename = NULL,
            .config_save = false,
            .no_reset = false,
            .headless = false,
        },

    .debugger =
        {
            .enabled = false,
            .keyboard_passthru = false,
            .hex_upper = true,
            .width = -1,
            .height = -1,
            .x = -1,
            .y = -1,
        },

    .window =
        {
            .width = -1,
            .height = -1,
            .x = -1,
            .y = -1,
            .display = -1,
        },
};

const Vector2 vga_resolutions[] = {
    {320, 240},  {400, 300},   {512, 384},   {640, 480},   {800, 600},   {1024, 768},  {1152, 864},
    {1280, 960}, {1400, 1050}, {1600, 1200}, {1856, 1392}, {1920, 1440}, {2048, 1536},
};
const int vga_resolutions_size = sizeof(vga_resolutions) / sizeof(Vector2);

void config_debug(void) {
    log_printf("== CONFIG ==\n");

    log_printf("\n");
    log_printf("=== command line ===\n");
    log_printf("  config_path: %s\n", config.arguments.config_path);
    log_printf(" rom_filename: %s\n", config.arguments.rom_filename);
    log_printf("     map_file: %s\n", config.arguments.map_file);
    log_printf("debug_enabled: %s\n", config.debugger.enabled == DEBUGGER_STATE_ARG ? "True" : "False");
    log_printf("    headless: %s\n", config.arguments.headless ? "True" : "False");
    log_printf("  config_save: %s\n", config.arguments.config_save ? "True" : "False");
    log_printf("     no_reset: %s\n", config.arguments.no_reset ? "True" : "False");

    log_printf("\n");
    log_printf("=== debugger ===\n");
    log_printf("enabled: %s\n", config.debugger.enabled == DEBUGGER_STATE_CONFIG ? "True" : "False");

    log_printf("\n");
    log_printf("=== window ===\n");
    log_printf("  width: %d\n", config.window.width);
    log_printf(" height: %d\n", config.window.height);
    log_printf("      x: %d\n", config.window.x);
    log_printf("      y: %d\n", config.window.y);
    log_printf("display: %d\n", config.window.display);

    log_printf("\n");
    log_printf("=== debugger ===\n");
    log_printf("  width: %d\n", config.debugger.width);
    log_printf(" height: %d\n", config.debugger.height);
    log_printf("      x: %d\n", config.debugger.x);
    log_printf("      y: %d\n", config.debugger.y);
    log_printf("\n\n");
}

int usage(const char *progname) {
    log_printf("Usage: %s [OPTIONS]\n", progname);
    log_printf("\nOptions:\n");
    log_printf("  -c, --config <file>                Zeal Config\n");
    log_printf("  -s, --save <file>                  Save * arguments to Zeal Config\n");
    log_printf("  -r, --rom <file>                   * Load ROM file\n");
    log_printf(
        "  -u, --uprog <file>[,<addr>]        Load user program in "
        "romdisk at hex address\n");
    log_printf("  -e, --eeprom <file>                Load EEPROM file\n");
    log_printf("  -t, --tf <file>                    Load TF/SDcard file\n");
    log_printf(
        "  -m, --map <file>                   Load memory map file (for "
        "debugging)\n");
    log_printf("  -g, --debug                        * Enable debug mode\n");
    log_printf(
        "  -b, --brk <addr/sym>[,<addr/sym>]  * Set breakpoints on boot "
        "(requires debug mode)\n");
    log_printf(
        "  -n, --headless                     Run without GUI (no "
        "window/input/rendering)\n");
    log_printf(
        "  -q, --no-reset                     Exit emulator when a reset "
        "is detected\n");
    log_printf("  -v, --verbose                      Verbose console output\n");
    log_printf("  -h, --help                         Show this help message\n");
    log_printf("\n");
    log_printf("Example:\n");
    log_printf("  %s --rom game.bin --map mem.map --debug\n", progname);

    exit(0);
}

int parse_command_args(int argc, char *argv[]) {
    const char *config_path = get_config_path();
    if (config_path) {
        config.arguments.config_path = config_path;
    }

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];

/* Helper: consume next argv as the option's value */
#define NEXT_ARG()                                                            \
    do {                                                                      \
        if (++i >= argc) {                                                    \
            log_err_printf("[CONFIG] Option %s requires an argument\n", arg); \
            return 1;                                                         \
        }                                                                     \
    } while (0)

        if (strcmp(arg, "-c") == 0 || strcmp(arg, "--config") == 0) {
            NEXT_ARG();
            config.arguments.config_path = argv[i];
        } else if (strcmp(arg, "-s") == 0 || strcmp(arg, "--save") == 0) {
            config.arguments.config_save = true;
        } else if (strcmp(arg, "-r") == 0 || strcmp(arg, "--rom") == 0) {
            NEXT_ARG();
            config.arguments.rom_filename = argv[i];
        } else if (strcmp(arg, "-e") == 0 || strcmp(arg, "--eeprom") == 0) {
            NEXT_ARG();
            config.arguments.eeprom_filename = argv[i];
        } else if (strcmp(arg, "-t") == 0 || strcmp(arg, "--tf") == 0) {
            NEXT_ARG();
            config.arguments.tf_filename = argv[i];
        } else if (strcmp(arg, "-u") == 0 || strcmp(arg, "--uprog") == 0) {
            NEXT_ARG();
            config.arguments.uprog_filename = argv[i];
        } else if (strcmp(arg, "-C") == 0 || strcmp(arg, "--cf") == 0) {
            NEXT_ARG();
            config.arguments.cf_filename = argv[i];
        } else if (strcmp(arg, "-m") == 0 || strcmp(arg, "--map") == 0) {
            NEXT_ARG();
            config.arguments.map_file = argv[i];
        } else if (strcmp(arg, "-g") == 0 || strcmp(arg, "--debug") == 0) {
            /* debug can optionally take "0" to disable */
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                i++;
                if (argv[i][0] == '0' && argv[i][1] == '\0') {
                    config.debugger.enabled = DEBUGGER_STATE_ARG_DISABLE;
                } else {
                    config.debugger.enabled = DEBUGGER_STATE_ARG;
                }
            } else {
                config.debugger.enabled = DEBUGGER_STATE_ARG;
            }
        } else if (strcmp(arg, "-b") == 0 || strcmp(arg, "--brk") == 0) {
            NEXT_ARG();
            config.arguments.breakpoints = argv[i];
        } else if (strcmp(arg, "-n") == 0 || strcmp(arg, "--headless") == 0) {
            config.arguments.headless = true;
            config.debugger.enabled = DEBUGGER_STATE_ARG_DISABLE;
        } else if (strcmp(arg, "-v") == 0 || strcmp(arg, "--verbose") == 0) {
            config.arguments.verbose = true;
        } else if (strcmp(arg, "-q") == 0 || strcmp(arg, "--no-reset") == 0) {
            config.arguments.no_reset = true;
        } else if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            return usage(argv[0]);
        } else if (arg[0] == '-') {
            log_err_printf("[CONFIG] Unknown option %s\n", arg);
            return 1;
        } else {
            /* positional argument — treat as ROM filename if not set */
            if (config.arguments.rom_filename == NULL) {
                config.arguments.rom_filename = argv[i];
            } else {
                log_err_printf("[CONFIG] Unexpected argument: %s\n", arg);
                return 1;
            }
        }

#undef NEXT_ARG
    }

    return 0;
}

void config_parse_file(const char *file) {
    printf("[CONFIG] %s\n", path_sanitize(file));
    if (!path_exists(file))
        return;

    config.ini = rini_load_config(file);

    if (config.arguments.rom_filename == NULL) {
        config.arguments.rom_filename = rini_get_config_value_text_fallback(config.ini, "ROM_FILENAME", NULL);
    }

    config.debugger.config_enabled =
        rini_get_config_value_fallback(config.ini, "DEBUG_ENABLED", DEBUGGER_STATE_DISABLED);
    if (config.debugger.enabled != DEBUGGER_STATE_ARG && config.debugger.enabled != DEBUGGER_STATE_ARG_DISABLE)
        config.debugger.enabled = config.debugger.config_enabled;

    config.window.width = rini_get_config_value_fallback(config.ini, "WIN_WIDTH", -1);
    config.window.height = rini_get_config_value_fallback(config.ini, "WIN_HEIGHT", -1);
    config.window.x = rini_get_config_value_fallback(config.ini, "WIN_POS_X", -1);
    config.window.y = rini_get_config_value_fallback(config.ini, "WIN_POS_Y", -1);
    config.window.display = rini_get_config_value_fallback(config.ini, "WIN_DISPLAY", -1);

    config.debugger.width = rini_get_config_value_fallback(config.ini, "DEBUG_WIDTH", -1);
    config.debugger.height = rini_get_config_value_fallback(config.ini, "DEBUG_HEIGHT", -1);
    config.debugger.x = rini_get_config_value_fallback(config.ini, "DEBUG_POS_X", -1);
    config.debugger.y = rini_get_config_value_fallback(config.ini, "DEBUG_POS_Y", -1);
    config.debugger.hex_upper = rini_get_config_value_fallback(config.ini, "DEBUG_HEX_UPPER", 1);
}

int config_save(void) {
    rini_config ini = rini_load_config(NULL);

    // main header
    rini_set_config_comment_line(&ini, NULL);
    rini_set_config_comment_line(&ini, "Zeal Native Emulator");
    rini_set_config_comment_line(&ini, NULL);

    if (config.arguments.config_save) {
        // config.arguments header
        config_arguments_t *args = &config.arguments;
        rini_set_config_comment_line(&ini, "Arguments");
        if (args->rom_filename != NULL)
            rini_set_config_value_text(&ini, "ROM_FILENAME", args->rom_filename, "ROM Filename");

        if (config.debugger.enabled == DEBUGGER_STATE_ARG) {
            config.debugger.config_enabled = true;
        } else if (config.debugger.enabled == DEBUGGER_STATE_ARG_DISABLE) {
            config.debugger.enabled = false;
        }
    }

    config_window_t *window = &config.window;
    rini_set_config_comment_line(&ini, "Main Window");
    rini_set_config_value(&ini, "WIN_WIDTH", window->width, "Width");
    rini_set_config_value(&ini, "WIN_HEIGHT", window->height, "Height");
    rini_set_config_value(&ini, "WIN_POS_X", window->x, "X Position");
    rini_set_config_value(&ini, "WIN_POS_Y", window->y, "Y Position");
    rini_set_config_value(&ini, "WIN_DISPLAY", window->display, "Display Number");

#if CONFIG_ENABLE_DEBUGGER
    config_debugger_t *debugger = &config.debugger;
    rini_set_config_comment_line(&ini, "Debugger");
    rini_set_config_value(&ini, "DEBUG_WIDTH", debugger->width, "Width");
    rini_set_config_value(&ini, "DEBUG_HEIGHT", debugger->height, "Height");
    rini_set_config_value(&ini, "DEBUG_POS_X", debugger->x, "X Position");
    rini_set_config_value(&ini, "DEBUG_POS_Y", debugger->y, "Y Position");
    rini_set_config_value(&ini, "DEBUG_ENABLED", debugger->config_enabled, "Debug Enabled");
    rini_set_config_value(&ini, "DEBUG_HEX_UPPER", debugger->hex_upper, "Use Upper Hex");

    dbg_ui_config_save(&ini);
#endif  // CONFIG_ENABLE_DEBUGGER

    rini_save_config(ini, config.arguments.config_path);
    rini_unload_config(&ini);

    return 0;  // TODO: error checking to ensure things worked?
}

void config_unload(void) {
    if (config.ini.values != NULL) {
        rini_unload_config(&config.ini);
    }
}

int config_get(const char *key, int defaultValue) {
    return rini_get_config_value_fallback(config.ini, key, defaultValue);
}

const char *config_get_text(const char *key, const char *defaultValue) {
    return rini_get_config_value_text_fallback(config.ini, key, defaultValue);
}

void config_set(const char *key, int value, const char *desc) {
    rini_set_config_value(&config.ini, key, value, desc);
}

void config_set_text(const char *key, const char *value, const char *desc) {
    rini_set_config_value_text(&config.ini, key, value, desc);
}

void config_window_update(bool dbg_enabled) {
#ifdef PLATFORM_WEB
    return;
#endif
    if (dbg_enabled) {
        config.debugger.width = GetScreenWidth();
        config.debugger.height = GetScreenHeight();
        Vector2 position = GetWindowPosition();
        config.debugger.x = position.x;
        config.debugger.y = position.y;
    } else {
        config.window.width = GetScreenWidth();
        config.window.height = GetScreenHeight();
        Vector2 position = GetWindowPosition();
        config.window.x = position.x;
        config.window.y = position.y;
    }
    config.window.display = GetCurrentMonitor();
}

Vector2 config_aspect_force(Vector2 size) {
    Vector2 aspect = {.x = 4, .y = 3};
#ifdef PLATFORM_WEB
    return aspect;
#endif
    // calculate aspect for missing size

    if (size.x >= size.y) {
        size.y = (size.x * aspect.y) / aspect.x;
    } else {
        size.x = (size.y * aspect.x) / aspect.y;
    }

    return size;
}

bool config_keyboard_passthru(bool dbg_enabled) {
    if (!dbg_enabled)
        return false;  // only enable passthru in debugger ui
    return config.debugger.keyboard_passthru;
}

void config_window_set(bool dbg_enabled) {
#ifdef PLATFORM_WEB
    SetWindowSize(1280, 960);
    printf("PLATFORM_WEB: config_window_set disabled (%d, %d)\n", GetScreenWidth(), GetScreenHeight());
    return;
#endif

    int d = config.window.display >= 0 ? config.window.display : GetCurrentMonitor();
    SetWindowMonitor(d);

    Vector2 window_size = {
        .x = config.window.width,
        .y = config.window.height,
    };

    if (dbg_enabled) {
        window_size.x = config.debugger.width;
        window_size.y = config.debugger.height;
    }

    if (window_size.x < 0 && window_size.y < 0) {
        // default
        if (dbg_enabled) {
            window_size.x = (ZVB_MAX_RES_WIDTH * 2);
            window_size.y = (ZVB_MAX_RES_HEIGHT * 2);
        } else {
            window_size.x = ZVB_MAX_RES_WIDTH;
            window_size.y = ZVB_MAX_RES_HEIGHT;
        }
    }

    if (!dbg_enabled && config.window.aspect_force) {
        window_size = config_aspect_force(window_size);
    }

    SetWindowSize(window_size.x, window_size.y);

    Vector2 screen_offset = GetMonitorPosition(d);
    Vector2 screen = {
        .x = GetMonitorWidth(d),
        .y = GetMonitorHeight(d),
    };

    Vector2 window_pos = {
        .x = config.window.x,
        .y = config.window.y,
    };

    if (dbg_enabled) {
        window_pos.x = config.debugger.x;
        window_pos.y = config.debugger.y;
    }

    // calculate center if either coordinate is not set
    if (window_pos.x < 0)
        window_pos.x = screen_offset.x + ((screen.x - window_size.x) / 2);
    if (window_pos.y < 0)
        window_pos.y = screen_offset.y + ((screen.y - window_size.y) / 2);
    SetWindowPosition(window_pos.x, window_pos.y);
}

Vector2 config_get_next_resolution(int width) {
    Vector2 size = vga_resolutions[vga_resolutions_size - 1];
    for (int i = 0; i < vga_resolutions_size; i++) {
        if (vga_resolutions[i].x > width) {
            size = vga_resolutions[i];
            break;
        }
    }
    return size;
}

Vector2 config_get_prev_resolution(int width) {
    Vector2 size = vga_resolutions[0];
    for (int i = vga_resolutions_size - 1; i >= 0; i--) {
        if (vga_resolutions[i].x < width) {
            size = vga_resolutions[i];
            break;
        }
    }
    return size;
}
