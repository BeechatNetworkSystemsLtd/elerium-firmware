
//***************************************************************************//

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "elerium/subsys/nfc.h"
#include "elerium/subsys/url_sign.h"
#include "elerium/subsys/wallet.h"

//***************************************************************************//

#define ELERIUM_URL_SIGN_MAX_PWD_LEN 8

//***************************************************************************//

enum elerium_cmd {
    ELERIUM_CMD_WALLET_CREATE = 0xA0,
    ELERIUM_CMD_WALLET_SIGN = 0xA1,
    ELERIUM_CMD_WALLET_SEED = 0xA2,

    ELERIUM_CMD_URL_SIGN_PROGRAM = 0xB0,
    ELERIUM_CMD_URL_SIGN_PUB_KEY = 0xB1,
    ELERIUM_CMD_URL_SIGN_RESET = 0xB2,
};

//***************************************************************************//

LOG_MODULE_DECLARE(elerium);

//***************************************************************************//

static int handle_message(const struct elerium_nfc_message* req_msg,
                          struct elerium_nfc_message* res_msg) {
    int rc = -EINVAL;

    static char password[ELERIUM_URL_SIGN_MAX_PWD_LEN + 1] = { 0 };

    switch ((enum elerium_cmd)req_msg->data[0]) {

#if IS_ENABLED(CONFIG_BEECHAT_ELERIUM_WALLET)
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
#endif

#if IS_ENABLED(CONFIG_BEECHAT_ELERIUM_URL_SIGN)

        case ELERIUM_CMD_URL_SIGN_PROGRAM:
            memcpy(password, &req_msg->data[4], ELERIUM_URL_SIGN_MAX_PWD_LEN);

            rc = elerium_url_sign_program(password,
                                          &req_msg->data[4 + ELERIUM_URL_SIGN_MAX_PWD_LEN]);

            if (rc == 0) {
                rc = elerium_url_sign_get_pub_raw(&res_msg->data[0], 64);
                res_msg->length = 64;
            }

            break;

        case ELERIUM_CMD_URL_SIGN_RESET:
            memcpy(password, &req_msg->data[4], ELERIUM_URL_SIGN_MAX_PWD_LEN);
            rc = elerium_url_sign_reset(password);
            break;

        case ELERIUM_CMD_URL_SIGN_PUB_KEY:
            if (rc == 0) {
                rc = elerium_url_sign_get_pub_raw(&res_msg->data[0], 64);
                res_msg->length = 64;
            }
            break;

#endif
        default:
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

        switch (req_msg.type) {

            case ELERIUM_NFC_MESSAGE_TYPE_NDEF:

#if IS_ENABLED(CONFIG_BEECHAT_ELERIUM_URL_SIGN)
                elerium_url_sign_generate();
#endif
                break;

            case ELERIUM_NFC_MESSAGE_TYPE_COMM: {
                rc = handle_message(&req_msg, &res_msg);

                uint8_t flags = 0x00;
                if (rc == 0) {
                    flags |= ELERIUM_NFC_MESSAGE_FLAG_OK;
                } else {
                    flags |= ELERIUM_NFC_MESSAGE_FLAG_ERR;
                }

                elerium_nfc_write_message(flags, &res_msg);
            }

            break;
        }
    }

    return 0;
}

//***************************************************************************//
