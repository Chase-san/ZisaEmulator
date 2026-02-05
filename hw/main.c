/*
 * SPDX-FileCopyrightText: 2025 Zeal 8-bit Computer <contact@zeal8bit.com>; David Higgins <zoul0813@me.com>
 *
 * SPDX-FileCopyrightText: 2026 Robert Maupin <chasesan@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * SPDX-FileContributor: Modified by Robert Maupin 2026
 */

#include "hw/zeal.h"
#include "utils/config.h"
#include "utils/log.h"

static zeal_t machine;

int main(int argc, char *argv[]) {
    int code = 0;
    code = parse_command_args(argc, argv);
    if (code != 0) {
        return code;
    }

    config_parse_file(config.arguments.config_path);
    if (config.arguments.verbose) {
        config_debug();
    }

    if (zeal_init(&machine)) {
        log_err_printf("Error initializing the machine\n");
        goto deinit;
    }

    if (flash_load_from_file(&machine.rom, config.arguments.rom_filename, config.arguments.uprog_filename) != FLASH_ERR_OK) {
        goto deinit;
    }

    if (config.arguments.tf_filename != NULL && zvb_spi_load_tf_image(&machine.zvb.spi, config.arguments.tf_filename)) {
        goto deinit;
    }

    code = zeal_run(&machine);
    
    (void)flash_save_to_file(&machine.rom, config.arguments.rom_filename);

    int saved = config_save();
    config_unload();
    if (!saved && code == 0) {
        return saved;  // ???
    }

deinit:
    zvb_sound_deinit(&machine.zvb.sound);
    return code;
}
