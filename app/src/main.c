
//***************************************************************************//

#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include "elerium/subsys/nfc.h"

//***************************************************************************//

//***********************************************************¡****************//

int main(void) {

    while (true) {
        struct elerium_nfc_message message;
        elerium_nfc_read_message(&message, K_FOREVER);
        elerium_nfc_write_message(0x00, &message);
    }

    return 0;
}

//***************************************************************************//
