
#ifndef ELERIUM_SUBSYS_URL_SIGN_H_
#define ELERIUM_SUBSYS_URL_SIGN_H_

//***************************************************************************//

#include <zephyr/kernel.h>

#include "elerium/subsys/crypto.h"

//***************************************************************************//

int elerium_url_sign_get_pub(struct elerium_pub_key* pub_key);

//***************************************************************************//

#endif // ELERIUM_SUBSYS_URL_SIGN_H_
