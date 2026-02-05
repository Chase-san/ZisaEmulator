/*
 * SPDX-FileCopyrightText: 2025 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-FileCopyrightText: 2026 Robert Maupin <chasesan@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * SPDX-FileContributor: Modified by Robert Maupin 2026
 */

#include "hw/zvb/zvb_dma.h"

#include <stdint.h>


#define DEBUG_DMA 0

/// A more portable packed structure alternative
static inline uint32_t dma_get_addr(const uint8_t addr[3]) {
    return addr[0] | (addr[1] << 8) | (addr[2] << 16);
}

/// A more portable packed structure alternative
static inline void dma_set_addr(uint8_t addr[3], uint32_t val) {
    addr[0] = val & 0xFF;
    addr[1] = (val >> 8) & 0xFF;
    addr[2] = (val >> 16) & 0xFF;
}

static void dma_start_transfer(zvb_dma_t *dma) {
    /* Grab the first descriptor from memory */
    zvb_dma_descriptor_t desc = {0};

    do {
        memory_phys_read_bytes(dma->ops, dma->desc_addr, (void *)&desc, sizeof(zvb_dma_descriptor_t));
        const int rd_ops = desc.flags.rd_op;
        const int wr_ops = desc.flags.wr_op;

#if DEBUG_DMA
        log_printf("Descriptor @ %08x:\n", dma->desc_addr);
        log_printf("  Read Address: 0x%08X\n", dma_get_addr(desc.rd_addr));
        log_printf("  Write Address: 0x%08X\n", dma_get_addr(desc.wr_addr));
        log_printf("  Length: %d\n", desc.length);
        log_printf("  Flags:\n");
        log_printf("    Read Operation: %d\n", desc.flags.rd_op);
        log_printf("    Write Operation: %d\n", desc.flags.wr_op);
        log_printf("    Last: %d\n", desc.flags.last);
#endif

        /* Descriptor is ready, perform the copy */
        for (int i = 0; i < desc.length; i++) {
            const uint8_t data = memory_phys_read_byte(dma->ops, dma_get_addr(desc.rd_addr));
            memory_phys_write_byte(dma->ops, dma_get_addr(desc.wr_addr), data);

#if DEBUG_DMA
            log_printf("Transfer: src=0x%08X, dst=0x%08X, byte=0x%02X\n", dma_get_addr(desc.rd_addr),
                       dma_get_addr(desc.wr_addr), data);
#endif

            /* Check if we have to modify the addresses */
            if (rd_ops == DMA_OP_INC) {
                dma_set_addr(desc.rd_addr, dma_get_addr(desc.rd_addr) + 1);
            } else if (rd_ops == DMA_OP_DEC) {
                dma_set_addr(desc.rd_addr, dma_get_addr(desc.rd_addr) - 1);
            }

            if (wr_ops == DMA_OP_INC) {
                dma_set_addr(desc.wr_addr, dma_get_addr(desc.wr_addr) + 1);
            } else if (wr_ops == DMA_OP_DEC) {
                dma_set_addr(desc.wr_addr, dma_get_addr(desc.wr_addr) - 1);
            }
        }
        /* Make the descriptor pointer go to the next descriptor */
        dma->desc_addr += sizeof(zvb_dma_descriptor_t);
    } while (!desc.flags.last);

    /* TODO: add number of elapsed T-states to the CPU? */
}

void zvb_dma_init(zvb_dma_t *dma, const memory_op_t *ops) {
    dma->clk.rd_cycle = 1;
    dma->clk.wr_cycle = 1;
    dma->desc_addr = 0;
    dma->ops = ops;
}

void zvb_dma_reset(zvb_dma_t *dma) {
    /* Different than boot values */
    dma->clk.rd_cycle = 6;
    dma->clk.wr_cycle = 5;
    /* Descriptor address unchanged on reset */
}

uint8_t zvb_dma_read(zvb_dma_t *dma, uint32_t port) {
    switch (port) {
        case DMA_REG_DESC_ADDR0:
            return dma->desc_addr0;
        case DMA_REG_DESC_ADDR1:
            return dma->desc_addr1;
        case DMA_REG_DESC_ADDR2:
            return dma->desc_addr2;
        case DMA_REG_CLK_DIV:
            return dma->clk.raw;
        default:
            return 0;
    }
}

void zvb_dma_write(zvb_dma_t *dma, uint32_t port, uint8_t value) {
    switch (port) {
        case DMA_REG_CTRL:
            if ((value & DMA_CTRL_START) != 0) {
                dma_start_transfer(dma);
            }
            break;
        case DMA_REG_DESC_ADDR0:
            dma->desc_addr0 = value;
            break;
        case DMA_REG_DESC_ADDR1:
            dma->desc_addr1 = value;
            break;
        case DMA_REG_DESC_ADDR2:
            dma->desc_addr2 = value;
            break;
        case DMA_REG_CLK_DIV:
            dma->clk.raw = value;
            break;
        default:
            break;
    }
}
