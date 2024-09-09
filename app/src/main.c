
//***************************************************************************//

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "elerium/subsys/nfc.h"
#include "elerium/subsys/wallet.h"

//***************************************************************************//

enum elerium_cmd {
    ELERIUM_CMD_WALLET_CREATE = 0xA0,
    ELERIUM_CMD_WALLET_SIGN = 0xA1,
    ELERIUM_CMD_WALLET_SEED = 0xA2,
};

//***************************************************************************//

LOG_MODULE_DECLARE(elerium);

//***************************************************************************//

static int handle_message(const struct elerium_nfc_message* req_msg,
                          struct elerium_nfc_message* res_msg) {
    int rc = -EINVAL;
    switch ((enum elerium_cmd)req_msg->data[0]) {
        case ELERIUM_CMD_WALLET_CREATE:
            rc = elerium_wallet_create(NULL, &res_msg->data[4]);
            res_msg->length = 4 + 32;
            break;
        case ELERIUM_CMD_WALLET_SEED:
            rc = elerium_wallet_seed(NULL, &res_msg->data[4]);
            res_msg->length = 4 + 32;
            break;
        case ELERIUM_CMD_WALLET_SIGN:
            rc = elerium_wallet_sign(
                elerium_wallet_get(NULL), &req_msg->data[4], 32, &res_msg->data[4]);
            res_msg->length = 4 + 64;
            break;
    }
    return rc;
}

//***************************************************************************//

int main(void) {

    while (true) {
        int rc;
        struct elerium_nfc_message req_msg = { 0 };
        struct elerium_nfc_message res_msg = { 0 };

        elerium_nfc_read_message(&req_msg, K_FOREVER);

        rc = handle_message(&req_msg, &res_msg);

        uint8_t flags = 0x00;
        if (rc == 0) {
            flags |= ELERIUM_NFC_MESSAGE_FLAG_OK;
        }

        elerium_nfc_write_message(flags, &res_msg);
    }

    return 0;
}

//***************************************************************************//
