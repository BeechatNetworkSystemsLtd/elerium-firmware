
#ifndef ELERIUM_SUBSYS_NFC_H_
#define ELERIUM_SUBSYS_NFC_H_

//***************************************************************************//

#include <zephyr/kernel.h>

//***************************************************************************//

#define ELERIUM_NFC_SRAM_SIZE 256
#define ELERIUM_NFC_HEADER_SIZE (4 + 4)
#define ELERIUM_NFC_MESSAGE_SIZE (ELERIUM_NFC_SRAM_SIZE - ELERIUM_NFC_HEADER_SIZE)

//***************************************************************************//

struct elerium_nfc_message {
    size_t length;
    uint8_t data[ELERIUM_NFC_MESSAGE_SIZE];
};

//***************************************************************************//

int elerium_nfc_read_message(struct elerium_nfc_message* message, k_timeout_t timeout);

int elerium_nfc_write_message(uint8_t flags, const struct elerium_nfc_message* message);

//***************************************************************************//

#endif // ELERIUM_SUBSYS_NFC_H_
