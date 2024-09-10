
//***************************************************************************//

#include <string.h>

#include <zephyr/drivers/flash.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include <zephyr/storage/flash_map.h>

#include <tinycrypt/constants.h>
#include <tinycrypt/ctr_prng.h>
#include <tinycrypt/ecc.h>
#include <tinycrypt/ecc_dh.h>
#include <tinycrypt/ecc_dsa.h>
#include <tinycrypt/sha256.h>
#include <tinycrypt/utils.h>

#include "elerium/subsys/wallet.h"

//***************************************************************************//

#define NVS_PARTITION storage_partition
#define NVS_PARTITION_DEVICE FIXED_PARTITION_DEVICE(NVS_PARTITION)
#define NVS_PARTITION_OFFSET FIXED_PARTITION_OFFSET(NVS_PARTITION)

#define WALLET_ID 0x2B01

//***************************************************************************//

struct elerium_wallet {
    uint8_t passcode_hash[32];
    uint8_t private_key[NUM_ECC_BYTES];
    uint8_t public_key[NUM_ECC_BYTES * 2];
};

//***************************************************************************//

static int wallet_init(void);
static int check_wallet(const struct elerium_wallet* wallet);
static int load_wallet(struct elerium_wallet* wallet);
static int save_wallet(const struct elerium_wallet* wallet);

//***************************************************************************//

// Kernel
SYS_INIT(wallet_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

// Devices

static const char personalize[] = "elerium:wallet";

// Module
static struct {
    struct k_mutex mut;

    struct nvs_fs fs;
    struct elerium_wallet wallet;
} mod;

//***************************************************************************//

int elerium_wallet_create(const uint8_t* passcode, uint8_t* seed) {
    int rc;

    rc = check_wallet(&mod.wallet);
    if (rc == 0) {
        return -EBUSY;
    }

    memset(&mod.wallet, 0x00, sizeof(mod.wallet));

    rc = uECC_make_key(mod.wallet.public_key, mod.wallet.private_key, uECC_secp256r1());
    if (rc == TC_CRYPTO_SUCCESS) {
        rc = 0;
    }

    if (rc == 0) {
        rc = save_wallet(&mod.wallet) < 0 ? -1 : 0;
    }

    if (rc == 0 && seed != NULL) {
        elerium_wallet_seed(passcode, seed);
    }

    return rc;
}

int elerium_wallet_seed(const uint8_t* passcode, uint8_t* hash) {
    int rc = 0;
    struct tc_sha256_state_struct sha256_ctx;

    if (tc_sha256_init(&sha256_ctx) != TC_CRYPTO_SUCCESS) {
        rc = -EINVAL;
    }

    if (rc == 0) {
        if (tc_sha256_update(&sha256_ctx, mod.wallet.private_key, sizeof(mod.wallet.private_key))
            != TC_CRYPTO_SUCCESS) {
            rc = -EINVAL;
        }
    }

    if (rc == 0) {
        if (tc_sha256_final(hash, &sha256_ctx) != TC_CRYPTO_SUCCESS) {
            rc = -EINVAL;
        }
    }

    return rc;
}

int elerium_wallet_destroy(void) {

    (void)memset(&mod.wallet, 0x00, sizeof(mod.wallet));

    nvs_delete(&mod.fs, WALLET_ID);

    return 0;
}

struct elerium_wallet* elerium_wallet_get(const uint8_t* passcode) {
    return &mod.wallet;
}

int elerium_wallet_sign(const struct elerium_wallet* wallet,
                        const uint8_t* hash,
                        size_t hash_length,
                        uint8_t* signature) {
    int rc;

    rc = uECC_sign(wallet->private_key, hash, hash_length, signature, uECC_secp256r1());

    if (rc == TC_CRYPTO_SUCCESS) {
        rc = 0;
    } else {
        rc = -EINVAL;
    }

    return rc;
}

//***************************************************************************//

int check_wallet(const struct elerium_wallet* wallet) {
    int rc = -EINVAL;

    for (size_t i = 0; i < sizeof(wallet->private_key); ++i) {
        if (wallet->private_key[i] != 0) {
            rc = 0;
            break;
        }
    }

    return rc;
}

int load_wallet(struct elerium_wallet* wallet) {
    return nvs_read(&mod.fs, WALLET_ID, wallet, sizeof(*wallet));
}

int save_wallet(const struct elerium_wallet* wallet) {
    return nvs_write(&mod.fs, WALLET_ID, wallet, sizeof(*wallet));
}

int wallet_init(void) {
    int rc;

    sys_rand_get(mod.entropy, sizeof(mod.entropy));

    rc = init_nvs();

    if (rc == 0) {

        rc = tc_ctr_prng_init(
            &mod.prng, mod.entropy, sizeof(mod.entropy), personalize, sizeof(personalize));

        if (rc == TC_CRYPTO_SUCCESS) {
            rc = 0;
        }
    }

    // elerium_wallet_destroy();

    if (load_wallet(&mod.wallet) < 0) {
        elerium_wallet_create("", NULL);
    }

    return rc;
}

//***************************************************************************//
