
//***************************************************************************//

#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include <string.h>

#include "ntag5/ntag5.h"

#include "elerium/subsys/nfc.h"

//***************************************************************************//

#define NFC_SRAM_START_ADDR (0x2000)
#define NFC_SRAM_SIZE (256)
#define NFC_MESSAGE_PAGE_COUNT (ELERIUM_NFC_MESSAGE_SIZE / NTAG5_MEMORY_BLOCK_SIZE)

//***************************************************************************//

static int nfc_init(void);
static void nfc_ed_callback(void);
static uint32_t nfc_crc32(uint32_t initial, const uint8_t* data, size_t length);

//***************************************************************************//

// Kernel
SYS_INIT(nfc_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

// Devices
static const struct device* const ntag_dev = DEVICE_DT_GET(DT_ALIAS(ntag));

static const uint8_t magic_pattern[] = { 0xE1, 0xD1 };

// Module
static struct {
    struct k_mutex mut;
    struct elerium_nfc_message message;
} mod;

//***************************************************************************//

int elerium_nfc_read_message(struct elerium_nfc_message* message, k_timeout_t timeout) {
    // TODO;
    return -1;
}

int elerium_nfc_write_message(uint8_t flags, const struct elerium_nfc_message* message) {

    int rc;

    struct ntag5_block header_blocks[] = {

    };

    if (message != NULL) {
        if (message->length < ELERIUM_NFC_MESSAGE_SIZE) {
        }
    }

    return rc;
}

//***************************************************************************//

void nfc_ed_callback(void) {

    int rc;

    struct ntag5_block block = { 0 };
    uint16_t addr = NFC_SRAM_START_ADDR;
    rc = ntag5_read_block(ntag_dev, addr++, &block, 1);
    if (rc != 0) {
        return;
    }

    // Check magic
    if ((block.data[0] != magic_pattern[0]) || (block.data[1] != magic_pattern[1])) {
        return;
    }

    // Check message length
    const size_t length = block.data[3];
    if (length > ELERIUM_NFC_MESSAGE_SIZE) {
        return;
    }

    rc = ntag5_read_block(ntag_dev, addr++, &block, 1);
    if (rc != 0) {
        return;
    }

    const uint32_t expected_crc = *(const uint32_t*)(block.data);

    mod.message.length = length;

    for (size_t i = 0; i < length; i += sizeof(block.data)) {
        rc = ntag5_read_block(ntag_dev, addr++, &block, 1);
        if (rc != 0) {
            return;
        }

        (void)memcpy(&mod.message.data[i], block.data, sizeof(block.data));
    }

    const uint32_t actual_crc = nfc_crc32(0x00, mod.message.data, mod.message.length);
    if (actual_crc != expected_crc) {
        return;
    }
}

int nfc_init(void) {
    int rc = -ENODEV;

    k_mutex_init(&mod.mut);

    if (device_is_ready(ntag_dev)) {

        ntag5_set_callback(ntag_dev, nfc_ed_callback);

        rc = 0;
    }

    return rc;
}

static uint32_t nfc_crc32(uint32_t initial, const uint8_t* data, size_t length) {
    return 0x00;
}

//***************************************************************************//
