
//***************************************************************************//

#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

#include "elerium/subsys/crypto.h"
#include "elerium/subsys/storage.h"
#include "elerium/subsys/url_sign.h"
#include "elerium/subsys/nfc.h"

//***************************************************************************//

#define KEY_PAIR_ID 0x0B01

//***************************************************************************//

struct url_sign_data {
    struct elerium_hash password_hash;
    char url[256];
};

//***************************************************************************//

static int url_sign_init(void);
static int generate_url(void);

//***************************************************************************//

// Kernel
SYS_INIT(url_sign_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

static char url_buffer[2048];
static char url_sign_hex_buffer[65];
static struct elerium_hash rnd_number_hash;
static struct elerium_signature rnd_number_sign;

// Module
static struct {
    struct k_mutex mut;
    struct elerium_key_pair key_pair;
    struct url_sign_data sign_data;
} mod;

//***************************************************************************//

int elerium_url_sign_get_pub(struct elerium_pub_key* pub_key) {

    __ASSERT_NO_MSG(pub_key != NULL);

    memcpy(pub_key, &mod.key_pair.pub, sizeof(*pub_key));

    return 0;
}

//***************************************************************************//

int generate_url(void) {
    int rc;

    const uint64_t rnd_number = elerium_crypto_random();

    int size = snprintk(url_buffer, sizeof(url_buffer), "%llu", rnd_number);

    rc = elerium_crypto_sha256(url_buffer, size, &rnd_number_hash);

    if (rc == 0) {
        rc = elerium_crypto_sign(&mod.key_pair.priv,
                                 rnd_number_hash.data,
                                 sizeof(rnd_number_hash.data),
                                 &rnd_number_sign);
    }

    if (rc == 0) {

        size = bin2hex(rnd_number_sign.data,
                       sizeof(rnd_number_sign.data),
                       url_sign_hex_buffer,
                       sizeof(url_sign_hex_buffer));

        if (size == 0) {
            rc = -EMSGSIZE;
        }
    }

    if (rc == 0) {
        size = snprintk(url_buffer,
                        sizeof(url_buffer),
                        "%s?sign=%s&rnd=%llu",
                        "beechat.network",
                        url_sign_hex_buffer,
                        rnd_number);
    }

    return rc;
}

static int init_key(void) {

    int rc;

    rc = elerium_storage_load(KEY_PAIR_ID, &mod.key_pair, sizeof(mod.key_pair));
    if (rc != 0) {
        rc = elerium_crypto_generate(&mod.key_pair.priv, &mod.key_pair.pub);

        if (rc == 0) {
            rc = elerium_storage_save(KEY_PAIR_ID, &mod.key_pair, sizeof(mod.key_pair));
        }
    }

    return rc;
}

int url_sign_init(void) {
    int rc;

    rc = init_key();

    if (rc == 0) {
        // generate_url();

        // elerium_nfc_set_ndef_url(url_buffer, strlen(url_buffer));
    }

    return rc;
}

//***************************************************************************//
