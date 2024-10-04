
//***************************************************************************//

#include <string.h>

#include <zephyr/drivers/flash.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/kernel.h>
#include <zephyr/storage/flash_map.h>

#include "elerium/subsys/storage.h"

//***************************************************************************//

#define NVS_PARTITION storage_partition
#define NVS_PARTITION_DEVICE FIXED_PARTITION_DEVICE(NVS_PARTITION)
#define NVS_PARTITION_OFFSET FIXED_PARTITION_OFFSET(NVS_PARTITION)

//***************************************************************************//

static int storage_init(void);

//***************************************************************************//

// Kernel
SYS_INIT(storage_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

// Module
static struct {
    struct k_mutex mut;
    struct nvs_fs fs;
} mod;

//***************************************************************************//

int elerium_storage_load(uint16_t key, void* data, size_t len) {

    __ASSERT_NO_MSG(data != NULL);
    __ASSERT_NO_MSG(len > 0);

    const ssize_t size = nvs_read(&mod.fs, key, data, len);
    int rc = 0;

    if (size < 0) {
        rc = -ENOENT;
    }

    if (size != len) {
        rc = -E2BIG;
    }

    return rc;
}

int elerium_storage_save(uint16_t key, const void* data, size_t len) {

    __ASSERT_NO_MSG(data != NULL);
    __ASSERT_NO_MSG(len > 0);

    const ssize_t size = nvs_write(&mod.fs, key, data, len);

    return (size < 0) ? -EIO : 0;
}

int elerium_storage_delete(uint16_t key) {
    return nvs_delete(&mod.fs, key);
}

//***************************************************************************//

int storage_init(void) {
    int rc;

    struct flash_pages_info info;

    mod.fs.flash_device = NVS_PARTITION_DEVICE;
    if (!device_is_ready(mod.fs.flash_device)) {
        return -EBADF;
    }
    mod.fs.offset = NVS_PARTITION_OFFSET;
    rc = flash_get_page_info_by_offs(mod.fs.flash_device, mod.fs.offset, &info);
    if (rc != 0) {
        return -EBADF;
    }

    mod.fs.sector_size = info.size;
    mod.fs.sector_count = 2U;

    rc = nvs_mount(&mod.fs);
    if (rc != 0) {
        return -EBADF;
    }

    return rc;
}

//***************************************************************************//
