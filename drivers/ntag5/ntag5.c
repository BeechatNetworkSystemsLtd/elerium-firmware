
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
            k_sleep(K_MSEC(10));
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
        k_sleep(K_MSEC(10));
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

int ntag5_write_ndef_uri_record(const struct device* dev,
                                uint8_t uri_prefix,
                                const uint8_t* uri,
                                uint8_t uri_len) {

    int rc = 0;

    struct ntag5_block ndef_block = {
        .data = { NTAG5_CAPABILITY_CONTAINER },
    };

    rc += ntag5_write_block(dev, NTAG5_CAPABILITY_CONTAINER_ADDRESS, &ndef_block, 1);

    uint8_t addr = NTAG5_NDEF_MESSAGE_START_ADDRESS;
    ndef_block.data[0] = NTAG5_TYPE_NDEF_MESSAGE;  // NDEF Message
    ndef_block.data[1] = uri_len + 5;              // Message size
    ndef_block.data[2] = NTAG5_NDEF_RECORD_HEADER; // Record header
    ndef_block.data[3] = NTAG5_NDEF_TYPE_LENGTH;   // Type Length - 1 byte
    rc += ntag5_write_block(dev, addr++, &ndef_block, 1);

    ndef_block.data[0] = uri_len + 1;         // Payload Length
    ndef_block.data[1] = NTAG5_NDEF_URI_TYPE; // Type / URI
    ndef_block.data[2] = uri_prefix;          // URI prefix
    ndef_block.data[3] = uri[0];              // URI data
    rc += ntag5_write_block(dev, addr++, &ndef_block, 1);

    uri_len--;
    while (uri_len >= NTAG5_MEMORY_BLOCK_SIZE) {

        memcpy(ndef_block.data,
               &uri[(addr - NTAG5_NDEF_MESSAGE_START_ADDRESS - 2) * NTAG5_MEMORY_BLOCK_SIZE + 1],
               NTAG5_MEMORY_BLOCK_SIZE);

        rc += ntag5_write_block(dev, addr++, &ndef_block, 1);
        uri_len -= NTAG5_MEMORY_BLOCK_SIZE;
    }

    memset(ndef_block.data, 0x00, sizeof(ndef_block.data));

    memcpy(ndef_block.data,
           &uri[(addr - NTAG5_NDEF_MESSAGE_START_ADDRESS - 2) * NTAG5_MEMORY_BLOCK_SIZE + 1],
           uri_len % NTAG5_MEMORY_BLOCK_SIZE);

    ndef_block.data[uri_len % NTAG5_MEMORY_BLOCK_SIZE] = NTAG5_NDEF_MESSAGE_END_MARK;
    rc += ntag5_write_block(dev, addr, &ndef_block, 1);

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
        gpio_add_callback(config->ed_gpio.port, &data->ed_gpio_callback);

        gpio_pin_interrupt_configure_dt(&config->ed_gpio, GPIO_INT_EDGE_TO_ACTIVE);
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
    static struct ntag5_data ntag5_data_##inst = { 0 };             \
                                                                    \
    DEVICE_DT_INST_DEFINE(inst,                                     \
                          &ntag5_init,                              \
                          NULL,                                     \
                          &ntag5_data_##inst,                       \
                          &ntag5_config_##inst,                     \
                          POST_KERNEL,                              \
                          CONFIG_KERNEL_INIT_PRIORITY_DEVICE,       \
                          NULL);

DT_INST_FOREACH_STATUS_OKAY(NTAG5_INIT)

//***************************************************************************//
