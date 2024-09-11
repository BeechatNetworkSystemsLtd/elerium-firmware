
#ifndef ELERIUM_SUBSYS_CRYPTO_H_
#define ELERIUM_SUBSYS_CRYPTO_H_

//***************************************************************************//

#include <zephyr/kernel.h>

//***************************************************************************//

//***************************************************************************//

struct elerium_signature {
    uint8_t data[64];
};

struct elerium_hash {
    uint8_t data[32];
};

struct elerium_priv_key {
    uint8_t data[32];
};

struct elerium_pub_key {
    uint8_t data[64];
};

struct elerium_key_pair {
    struct elerium_priv_key priv;
    struct elerium_pub_key pub;
};

//***************************************************************************//

int elerium_crypto_generate(struct elerium_priv_key* priv_key, struct elerium_pub_key* pub_key);

int elerium_crypto_sign(struct elerium_priv_key* priv_key,
                        const void* hash,
                        size_t hash_len,
                        struct elerium_signature* sign);

int elerium_crypto_verify(struct elerium_pub_key* pub_key,
                          const void* hash,
                          size_t hash_len,
                          const struct elerium_signature* sign);

int elerium_crypto_sha256(const void* data, size_t data_len, struct elerium_hash* hash);

uint64_t elerium_crypto_random(void);

//***************************************************************************//

#endif // ELERIUM_SUBSYS_CRYPTO_H_
