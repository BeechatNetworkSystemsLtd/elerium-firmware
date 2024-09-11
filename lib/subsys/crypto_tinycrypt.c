
//***************************************************************************//

#include <zephyr/kernel.h>
#include <zephyr/random/random.h>

#include <tinycrypt/constants.h>
#include <tinycrypt/ctr_prng.h>
#include <tinycrypt/ecc.h>
#include <tinycrypt/ecc_dh.h>
#include <tinycrypt/ecc_dsa.h>
#include <tinycrypt/sha256.h>
#include <tinycrypt/utils.h>

#include "elerium/subsys/crypto.h"

//***************************************************************************//

static const char personalize[] = "beechat:elerium:crypto";

// Module
struct {
    uint8_t entropy[TC_AES_KEY_SIZE + TC_AES_BLOCK_SIZE];
    TCCtrPrng_t prng;
} mod;

//***************************************************************************//

int default_CSPRNG(uint8_t* dest, unsigned int size) {
    int res = tc_ctr_prng_generate(&mod.prng, NULL, 0, dest, size);
    return res;
}

int elerium_crypto_generate(struct elerium_priv_key* priv_key, struct elerium_pub_key* pub_key) {
    const int rc = uECC_make_key(pub_key->data, priv_key->data, uECC_secp256r1());
    return (rc == TC_CRYPTO_SUCCESS) ? 0 : -EFAULT;
}

int elerium_crypto_sign(struct elerium_priv_key* priv_key,
                        const void* hash,
                        size_t hash_len,
                        struct elerium_signature* sign) {

    __ASSERT_NO_MSG(priv_key != NULL);
    __ASSERT_NO_MSG(hash != NULL);
    __ASSERT_NO_MSG(sign != NULL);

    const int rc = uECC_sign(priv_key->data, hash, hash_len, sign->data, uECC_secp256r1());

    return (rc == TC_CRYPTO_SUCCESS) ? 0 : -EFAULT;
}


int elerium_crypto_verify(struct elerium_pub_key* pub_key,
                          const void* hash,
                          size_t hash_len,
                          const struct elerium_signature* sign) {

    __ASSERT_NO_MSG(pub_key != NULL);
    __ASSERT_NO_MSG(hash != NULL);
    __ASSERT_NO_MSG(sign != NULL);

    const int rc = uECC_verify(pub_key->data, hash, hash_len, sign->data, uECC_secp256r1());

    return (rc == TC_CRYPTO_SUCCESS) ? 0 : -EFAULT;
}


int elerium_crypto_sha256(const void* data, size_t data_len, struct elerium_hash* hash) {
    int rc = 0;

    struct tc_sha256_state_struct sha256_ctx = { 0 };

    if (tc_sha256_init(&sha256_ctx) != TC_CRYPTO_SUCCESS) {
        rc = -EINVAL;
    }

    if (rc == 0) {
        if (tc_sha256_update(&sha256_ctx, data, data_len) != TC_CRYPTO_SUCCESS) {
            rc = -EINVAL;
        }
    }

    if (rc == 0) {
        if (tc_sha256_final(hash->data, &sha256_ctx) != TC_CRYPTO_SUCCESS) {
            rc = -EINVAL;
        }
    }

    return rc;
}

uint64_t elerium_crypto_random(void) {
    uint64_t result = 0;

    sys_rand_get(&result, sizeof(result));

    return result;
}

//***************************************************************************//

int crypto_init(void) {
    int rc;

    sys_rand_get(mod.entropy, sizeof(mod.entropy));

    rc = tc_ctr_prng_init(
        &mod.prng, mod.entropy, sizeof(mod.entropy), personalize, sizeof(personalize));

    if (rc == TC_CRYPTO_SUCCESS) {
        rc = 0;
    }

    uECC_set_rng(&default_CSPRNG);

    return rc;
}

//***************************************************************************//
