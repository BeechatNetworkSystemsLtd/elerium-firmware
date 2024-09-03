
//***************************************************************************//

#define DT_DRV_COMPAT nxp_ntag5

//***************************************************************************//

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>

#include "ntag5.h"

//***************************************************************************//

struct ntag5_config {
    struct i2c_dt_spec i2c;
    struct gpio_dt_spec ed_gpio;
};

struct ntag5_data {
    struct gpio_callback ed_gpio_callback;
    struct k_sem sem;
    ntag5_ed_callback ed_callback;
    struct k_work work;
};

//***************************************************************************//

int ntag5_write_block(const struct device* dev,
                      uint16_t addr,
                      const struct ntag5_block* block,
                      size_t count) {

    if (((addr + count) > NTAG5_USER_MEMORY_ADDRESS_MAX)
        && (addr < NTAG5_CONFIG_MEMORY_ADDRESS_MIN)) {
        return -EINVAL;
    }

    if ((addr + count) > NTAG5_CONFIG_MEMORY_ADDRESS_MAX) {
        return -EINVAL;
    }

    const struct ntag5_config* config = dev->config;

    int rc = 0;

    for (size_t i = 0; i < count; ++i) {

        const uint16_t block_addr = addr + i;

        const uint8_t buf[] = {
            (uint8_t)((block_addr >> 8) & 0xFF),
            (uint8_t)(block_addr & 0xFF),
            block->data[0],
            block->data[1],
            block->data[2],
            block->data[3],
        };

        rc = i2c_write_dt(&config->i2c, buf, sizeof(buf));

        if (rc == 0) {
            k_sleep(K_MSEC(5));
        } else {
            break;
        }
    }

    return rc;
}

int ntag5_read_block(const struct device* dev,
                     uint16_t addr,
                     struct ntag5_block* block,
                     size_t count) {

    if (((addr + count) > NTAG5_USER_MEMORY_ADDRESS_MAX)
        && (addr < NTAG5_CONFIG_MEMORY_ADDRESS_MIN)) {
        return -EINVAL;
    }

    if ((addr + count) > NTAG5_CONFIG_MEMORY_ADDRESS_MAX) {
        return -EINVAL;
    }

    const struct ntag5_config* config = dev->config;

    int rc = 0;

    for (size_t i = 0; i < count; ++i) {

        const uint16_t block_addr = addr + i;

        const uint8_t buf[] = {
            (uint8_t)((block_addr >> 8) & 0xFF),
            (uint8_t)(block_addr & 0xFF),
        };

        rc =
            i2c_write_read_dt(&config->i2c, buf, sizeof(buf), block[i].data, sizeof(block[i].data));

        if (rc != 0) {
            break;
        }
    }

    return rc;
}

int ntag5_write_session_reg(const struct device* dev,
                            uint16_t addr,
                            uint8_t reg_byte,
                            uint8_t mask,
                            uint8_t data_in) {

    if ((addr < NTAG5_SESSION_REG_ADDRESS_MIN) || (addr > NTAG5_SESSION_REG_ADDRESS_MAX)
        || (reg_byte > NTAG5_SESSION_REG_BYTE_3)) {
        return -EINVAL;
    }

    const uint8_t buf[] = {
        (uint8_t)((addr >> 8) & 0xFF), (uint8_t)(addr & 0xFF), reg_byte, mask, data_in,
    };

    const struct ntag5_config* config = dev->config;

    const int rc = i2c_write_dt(&config->i2c, buf, sizeof(buf));

    if (rc == 0) {
        k_sleep(K_MSEC(5));
    }

    return rc;
}

int ntag5_read_session_reg(const struct device* dev,
                           uint16_t addr,
                           uint8_t reg_byte,
                           uint8_t* data_out) {
    if ((addr < NTAG5_SESSION_REG_ADDRESS_MIN) || (addr > NTAG5_SESSION_REG_ADDRESS_MAX)
        || (reg_byte > NTAG5_SESSION_REG_BYTE_3)) {
        return -EINVAL;
    }

    const uint8_t buf[] = {
        (uint8_t)((addr >> 8) & 0xFF),
        (uint8_t)(addr & 0xFF),
        reg_byte,
    };

    const struct ntag5_config* config = dev->config;

    return i2c_write_read_dt(&config->i2c, buf, sizeof(buf), data_out, sizeof(uint8_t));
}

int ntag5_write_message(const struct device* dev,
                        uint16_t addr,
                        const uint8_t* message,
                        size_t message_len) {

    if ((addr < NTAG5_USER_MEMORY_ADDRESS_MIN) || (addr > NTAG5_USER_MEMORY_ADDRESS_MAX)
        || (message_len == 0) || (message == NULL)) {
        return -EINVAL;
    }

    const struct ntag5_block* block = (const struct ntag5_block*)message;

    size_t count = message_len / NTAG5_MEMORY_BLOCK_SIZE;
    if (message_len % NTAG5_MEMORY_BLOCK_SIZE) {
        ++count;
    }

    return ntag5_write_block(dev, addr, block, count);
}

int ntag5_read_message(const struct device* dev,
                       uint16_t addr,
                       uint8_t* message,
                       size_t message_len) {

    if ((addr < NTAG5_USER_MEMORY_ADDRESS_MIN) || (addr > NTAG5_USER_MEMORY_ADDRESS_MAX)
        || (message_len == 0) || (message == NULL)) {
        return -EINVAL;
    }

    struct ntag5_block* block = (struct ntag5_block*)message;
    size_t count = message_len / NTAG5_MEMORY_BLOCK_SIZE;
    if (message_len % NTAG5_MEMORY_BLOCK_SIZE) {
        ++count;
    }

    return ntag5_read_block(dev, addr, block, count);
}

int ntag5_format_memory(const struct device* dev) {
    int rc = 0;

    const struct ntag5_block block = { 0 };

    for (uint16_t addr = NTAG5_USER_MEMORY_ADDRESS_MIN; addr < NTAG5_USER_MEMORY_ADDRESS_MAX;
         ++addr) {

        rc = ntag5_write_block(dev, addr, &block, 1);
        if (rc != 0) {
            break;
        }
    }

    return rc;
}

//***************************************************************************//

static void ed_work_handler(struct k_work* work) {

    const struct ntag5_data* const data = CONTAINER_OF(work, struct ntag5_data, work);

    if (data->ed_callback != NULL) {
        data->ed_callback();
    }
}

static void
ed_gpio_interrupt(const struct device* dev, struct gpio_callback* cbdata, uint32_t pins) {

    ARG_UNUSED(dev);
    ARG_UNUSED(pins);

    struct ntag5_data* const data = CONTAINER_OF(cbdata, struct ntag5_data, ed_gpio_callback);

    k_sem_give(&data->sem);

    k_work_submit(&data->work);
}

int ntag5_wait_on_ed(const struct device* dev, k_timeout_t timeout) {
    struct ntag5_data* const data = dev->data;
    return k_sem_take(&data->sem, timeout);
}

void ntag5_set_callback(const struct device* dev, ntag5_ed_callback callback) {
    struct ntag5_data* const data = dev->data;
    data->ed_callback = callback;
}

static const struct ntag5_block_spec default_blocks[] = {
    {
        .addr = 0x103D,
        .block = (struct ntag5_block) { .data = { 0x21, 0x00, 0x04, 0x00 } },
    },
    {
        .addr = 0x103C,
        .block = (struct ntag5_block) { .data = { 0x08, 0x48, 0x01, 0x3F } },
    },
    {
        .addr = 0x1038,
        .block = (struct ntag5_block) { .data = { 0x3F, 0x00, 0x00, 0x00 } },
    },
    {
        .addr = 0x1092,
        .block = (struct ntag5_block) { .data = { 0xFF, 0x00, 0x00, 0x00 } },
    },
    {
        .addr = 0x1093,
        .block = (struct ntag5_block) { .data = { 0xFF, 0x00, 0x00, 0x00 } },
    },
    {
        .addr = 0x1058,
        .block = (struct ntag5_block) { .data = { 0x00, 0x22, 0x00, 0x00 } },
    },
    {
        .addr = 0x1037,
        .block = (struct ntag5_block) { .data = { 0x088, 0x8B, 0xCF, 0x00 } },
    },
};

static int ntag5_init(const struct device* dev) {
    const struct ntag5_config* config = dev->config;
    struct ntag5_data* data = dev->data;

    data->ed_callback = NULL;

    k_sem_init(&data->sem, 0, 1);
    k_work_init(&data->work, ed_work_handler);

    if (!i2c_is_ready_dt(&config->i2c)) {
        return -ENODEV;
    }

    if (gpio_is_ready_dt(&config->ed_gpio)) {
        gpio_pin_configure_dt(&config->ed_gpio, GPIO_INPUT);
        gpio_init_callback(&data->ed_gpio_callback, ed_gpio_interrupt, BIT(config->ed_gpio.pin));
        gpio_pin_interrupt_configure_dt(&config->ed_gpio, GPIO_INT_EDGE_TO_ACTIVE);
    }

    // Default configuration
    {
        k_sleep(K_MSEC(100));

        // PT_TRANSFER_DIR = 0, Data transfer direction is I2C to NFC
        ntag5_write_session_reg(
            dev, NTAG5_SESSION_REG_CONFIG, NTAG5_SESSION_REG_BYTE_1, 0x01, 0x00);

        for (size_t i = 0; i < ARRAY_SIZE(default_blocks); ++i) {
            ntag5_write_block(dev, default_blocks[i].addr, &default_blocks[i].block, 1);
        }

        // PT_TRANSFER_DIR = 0, Data transfer direction is NFC to I2C
        ntag5_write_session_reg(
            dev, NTAG5_SESSION_REG_CONFIG, NTAG5_SESSION_REG_BYTE_1, 0x01, 0x01);
    }

    return 0;
}

//***************************************************************************//

#define NTAG5_INIT(inst)                                            \
                                                                    \
    static const struct ntag5_config ntag5_config_##inst = {        \
        .i2c = I2C_DT_SPEC_INST_GET(inst),                          \
        .ed_gpio = GPIO_DT_SPEC_INST_GET_OR(inst, ed_gpios, { 0 }), \
    };                                                              \
                                                                    \
    DEVICE_DT_INST_DEFINE(inst,                                     \
                          &ntag5_init,                              \
                          NULL,                                     \
                          NULL,                                     \
                          &ntag5_config_##inst,                     \
                          POST_KERNEL,                              \
                          CONFIG_KERNEL_INIT_PRIORITY_DEVICE,       \
                          NULL);

DT_INST_FOREACH_STATUS_OKAY(NTAG5_INIT)

//***************************************************************************//
