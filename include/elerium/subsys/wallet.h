
#ifndef ELERIUM_SUBSYS_WALLET_H_
#define ELERIUM_SUBSYS_WALLET_H_

//***************************************************************************//

#include <zephyr/kernel.h>

//***************************************************************************//

//***************************************************************************//

struct elerium_wallet;

struct elerium_signature {
    uint8_t data[64];
};

//***************************************************************************//

int elerium_wallet_create(const uint8_t* passcode, uint8_t* seed);

int elerium_wallet_seed(const uint8_t* passcode, uint8_t* seed);

int elerium_wallet_destroy(void);

struct elerium_wallet* elerium_wallet_get(const uint8_t* passcode);

int elerium_wallet_sign(const struct elerium_wallet* wallet,
                        const uint8_t* hash,
                        size_t hash_length,
                        uint8_t* signature);

//***************************************************************************//

#endif // ELERIUM_SUBSYS_WALLET_H_
