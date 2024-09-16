
//***************************************************************************//

#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/sys/util.h>

#include "elerium/subsys/crypto.h"
#include "elerium/subsys/nfc.h"
#include "elerium/subsys/storage.h"
#include "elerium/subsys/url_sign.h"

//***************************************************************************//

#define KEY_PAIR_ID 0x0B01
#define URL_DATA_ID 0x0B02

//***************************************************************************//

struct url_sign_data {
    bool enabled;
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
static char url_sign_hex_buffer[256];
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

int elerium_url_sign_get_pub_raw(uint8_t* data, size_t cap) {

    __ASSERT_NO_MSG(pub_key != NULL);
    __ASSERT_NO_MSG(sizeof(mod.key_pair.pub) <= cap);

    memcpy(data, &mod.key_pair.pub, sizeof(mod.key_pair.pub));

    return 0;
}

int elerium_url_sign_generate(void) {

    int rc = -EAGAIN;

    k_mutex_lock(&mod.mut, K_FOREVER);

    if (mod.sign_data.enabled) {
        rc = 0;
    }

    if (rc == 0) {
        rc = generate_url();
    }

    if (rc == 0) {
        url_buffer[sizeof(url_buffer) - 1] = '\0';
        rc = elerium_nfc_set_ndef_url(url_buffer, strlen(url_buffer));
    }

    k_mutex_unlock(&mod.mut);

    return rc;
}

int elerium_url_sign_program(const char* password, const char* url) {
    int rc = -EAGAIN;

    __ASSERT_NO_MSG(password != NULL);
    __ASSERT_NO_MSG(url != NULL);

    k_mutex_lock(&mod.mut, K_FOREVER);

    // Check if URL signer already enabled
    if (!mod.sign_data.enabled) {
        rc = 0;
    }

    if (rc == 0) {
        elerium_crypto_sha256(password, strlen(password), &mod.sign_data.password_hash);

        strncpy(mod.sign_data.url, url, sizeof(mod.sign_data.url) - 1);

        mod.sign_data.enabled = true;
        rc = elerium_storage_save(URL_DATA_ID, &mod.sign_data, sizeof(mod.sign_data));

        mod.sign_data.enabled = (rc == 0);
    }

    k_mutex_unlock(&mod.mut);

    return rc;
}

int elerium_url_sign_reset(const char* password) {
    int rc;

    __ASSERT_NO_MSG(password != NULL);

    struct elerium_hash password_hash = { 0 };
    rc = elerium_crypto_sha256(password, strlen(password), &password_hash);

    k_mutex_lock(&mod.mut, K_FOREVER);

    if (rc == 0) {

        rc = -EPERM;
        if (memcmp(&password_hash, &mod.sign_data.password_hash, sizeof(password_hash)) == 0) {
            rc = 0;
        }
    }

    // Password is correct
    if (rc == 0) {
        elerium_storage_delete(URL_DATA_ID);
        memset(&mod.sign_data, 0x00, sizeof(mod.sign_data));
        mod.sign_data.enabled = false;
    }

    k_mutex_unlock(&mod.mut);

    return rc;
}

//***************************************************************************//

int generate_url(void) {
    int rc;

    const uint64_t rnd_number = elerium_crypto_random();

    int size = snprintk(url_buffer, sizeof(url_buffer), "%llu", rnd_number);

    if (size > 0) {
        // TODO: change to 'size' instead of strlen()
        rc = elerium_crypto_sha256(url_buffer, strlen(url_buffer), &rnd_number_hash);
    } else {
        rc = -EMSGSIZE;
    }

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
                        "%s?rnd=%llu&sign=%s",
                        mod.sign_data.url,
                        rnd_number,
                        url_sign_hex_buffer);

        rc = (size > 0) ? 0 : -EMSGSIZE;
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

    k_mutex_init(&mod.mut);

    rc = init_key();

    if (rc == 0) {

        rc = elerium_storage_load(URL_DATA_ID, &mod.sign_data, sizeof(mod.sign_data));
        if (rc != 0) {
            memset(&mod.sign_data, 0x00, sizeof(mod.sign_data));
            mod.sign_data.enabled = false;
        }

        rc = 0;

        // Generate new url if enabled
        (void)elerium_url_sign_generate();
    }

    return rc;
}

//***************************************************************************//
