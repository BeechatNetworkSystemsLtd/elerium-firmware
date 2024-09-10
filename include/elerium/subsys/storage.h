
#ifndef ELERIUM_SUBSYS_STORAGE_H_
#define ELERIUM_SUBSYS_STORAGE_H_

//***************************************************************************//

#include <zephyr/kernel.h>

//***************************************************************************//

int elerium_storage_load(uint16_t key, const void* data, size_t len);

int elerium_storage_save(uint16_t key, void* data, size_t len);

int elerium_storage_delete(uint16_t key);

//***************************************************************************//

#endif // ELERIUM_SUBSYS_STORAGE_H_
