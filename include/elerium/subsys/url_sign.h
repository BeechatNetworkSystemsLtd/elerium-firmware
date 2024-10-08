
#ifndef ELERIUM_SUBSYS_URL_SIGN_H_
#define ELERIUM_SUBSYS_URL_SIGN_H_

//***************************************************************************//

#include <zephyr/kernel.h>

#include "elerium/subsys/crypto.h"

//***************************************************************************//

int elerium_url_sign_get_pub(struct elerium_pub_key* pub_key);
int elerium_url_sign_get_pub_raw(uint8_t* data, size_t cap);

int elerium_url_sign_generate(void);

int elerium_url_sign_program(const char* password, const char* url);
int elerium_url_sign_reset(const char* password);

//***************************************************************************//

#endif // ELERIUM_SUBSYS_URL_SIGN_H_
