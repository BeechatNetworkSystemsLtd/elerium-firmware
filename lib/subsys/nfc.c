
//***************************************************************************//

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/crc.h>

#include <string.h>

#include "ntag5/ntag5.h"

#include "elerium/subsys/nfc.h"

//***************************************************************************//

#define NFC_SRAM_START_ADDR (0x2000)
#define NFC_SRAM_END_ADDR (0x203E)
#define NFC_SRAM_SIZE (256)
#define NFC_MESSAGE_PAGE_COUNT (ELERIUM_NFC_MESSAGE_SIZE / NTAG5_MEMORY_BLOCK_SIZE)

//***************************************************************************//

static int nfc_init(void);
static void nfc_ed_callback(void);
static uint32_t nfc_crc32(uint32_t initial, const uint8_t* data, size_t length);
static int nfc_control_switch(void);
static int parse_message(struct elerium_nfc_message* message);

//***************************************************************************//

// Kernel
SYS_INIT(nfc_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

// Devices
static const struct device* const ntag_dev = DEVICE_DT_GET(DT_ALIAS(ntag));

static const uint8_t magic_pattern[] = { 0xE1, 0xED };

static uint8_t message_queue_buffer[sizeof(struct elerium_nfc_message)] = { 0 };

// Module
static struct {
    struct k_mutex mut;
    struct elerium_nfc_message message;
    struct k_msgq queue;
} mod;

//***************************************************************************//

int elerium_nfc_read_message(struct elerium_nfc_message* message, k_timeout_t timeout) {
    return k_msgq_get(&mod.queue, message, timeout);
}

int elerium_nfc_write_message(uint8_t flags, const struct elerium_nfc_message* message) {

    int rc;

    struct ntag5_block block;

    uint16_t addr = NFC_SRAM_START_ADDR;

    // Header
    block.data[0] = magic_pattern[0];
    block.data[1] = magic_pattern[1];
    block.data[2] = flags;
    block.data[3] = message->length;
    rc = ntag5_write_block(ntag_dev, addr++, &block, 1);
    if (rc != 0) {
        return -EIO;
    }

    // CRC
    const uint32_t crc = nfc_crc32(0x00, message->data, message->length);
    (void)memcpy(block.data, &crc, sizeof(crc));
    rc = ntag5_write_block(ntag_dev, addr++, &block, 1);
    if (rc != 0) {
        return -EIO;
    }

    size_t offset = 0;
    const size_t block_step = sizeof(block.data);
    while (addr < NFC_SRAM_END_ADDR) {

        if (offset < message->length) {

            const size_t length = ((offset + block_step) <= message->length)
                ? block_step
                : (message->length - offset);

            memcpy(block.data, &message->data[offset], length);

        } else {
            memset(block.data, 0x00, sizeof(block.data));
        }

        offset += block_step;

        rc = ntag5_write_block(ntag_dev, addr++, &block, 1);
        if (rc != 0) {
            break;
        }
    }

    nfc_control_switch();

    return rc;
}

//***************************************************************************//

void nfc_ed_callback(void) {
    int rc;

    rc = parse_message(&mod.message);

    if (rc == 0) {
        rc = k_msgq_put(&mod.queue, &mod.message, K_MSEC(250));
    }

    if (rc != 0) {
        nfc_control_switch();
    }
}

int nfc_init(void) {
    int rc = -ENODEV;

    k_mutex_init(&mod.mut);
    k_msgq_init(&mod.queue, message_queue_buffer, sizeof(struct elerium_nfc_message), 1);

    if (device_is_ready(ntag_dev)) {
        ntag5_set_callback(ntag_dev, nfc_ed_callback);
        rc = 0;
    }

    return rc;
}

int parse_message(struct elerium_nfc_message* message) {
    int rc;

    struct ntag5_block block = { 0 };

    uint16_t addr = NFC_SRAM_START_ADDR;

    rc = ntag5_read_block(ntag_dev, addr++, &block, 1);
    if (rc != 0) {
        return -EIO;
    }

    // Check magic
    if ((block.data[0] != magic_pattern[0]) || (block.data[1] != magic_pattern[1])) {
        return -ENOTTY;
    }

    // Check message length
    const size_t length = block.data[3];
    if (length > ELERIUM_NFC_MESSAGE_SIZE) {
        return -ENOMEM;
    }

    rc = ntag5_read_block(ntag_dev, addr++, &block, 1);
    if (rc != 0) {
        return -EIO;
    }

    const uint32_t expected_crc = *(const uint32_t*)(block.data);

    mod.message.length = length;

    for (size_t i = 0; i < length; i += sizeof(block.data)) {
        rc = ntag5_read_block(ntag_dev, addr++, &block, 1);
        if (rc != 0) {
            return -EIO;
        }

        (void)memcpy(&mod.message.data[i], block.data, sizeof(block.data));
    }

    const uint32_t actual_crc = nfc_crc32(0x00, mod.message.data, mod.message.length);
    if (actual_crc != expected_crc) {
        return -EINVAL;
    }

    return 0;
}

int nfc_control_switch(void) {
    struct ntag5_block block = { 0 };
    return ntag5_read_block(ntag_dev, NFC_SRAM_START_ADDR + 0x3F, &block, 1);
}

static uint32_t nfc_crc32(uint32_t initial, const uint8_t* data, size_t length) {
    return crc32_ieee_update(initial, data, length);
}

//***************************************************************************//
