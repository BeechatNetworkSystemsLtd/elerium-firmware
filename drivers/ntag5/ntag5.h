
#ifndef DRIVERS_NTAG5_H_
#define DRIVERS_NTAG5_H_

//***************************************************************************//

#include <zephyr/kernel.h>

//***************************************************************************//

/// @brief NTAG 5 Link configuration memory organization

#define NTAG5_CONFIG_SIGNATURE 0x1000
#define NTAG5_CONFIG_HEADER 0x1008
#define NTAG5_CONFIG_ID 0x1009
#define NTAG5_CONFIG_NFC_GCH 0x100C
#define NTAG5_CONFIG_NFC_CCH 0x100D
#define NTAG5_CONFIG_NFC_AUTH_LIMIT 0x100E
#define NTAG5_CONFIG_NFC_KH0 0x1010
#define NTAG5_CONFIG_NFC_KP0 0x1011
#define NTAG5_CONFIG_NFC_KH1 0x1012
#define NTAG5_CONFIG_NFC_KP1 0x1013
#define NTAG5_CONFIG_NFC_KH2 0x1014
#define NTAG5_CONFIG_NFC_KP2 0x1015
#define NTAG5_CONFIG_NFC_KH3 0x1016
#define NTAG5_CONFIG_NFC_KP3 0x1017
#define NTAG5_CONFIG_KEY_0 0x1020
#define NTAG5_CONFIG_KEY_1 0x1024
#define NTAG5_CONFIG_KEY_2 0x1028
#define NTAG5_CONFIG_KEY_3 0x102C
#define NTAG5_CONFIG_I2C_KH 0x1030
#define NTAG5_CONFIG_I2C_PP_AND_PPC 0x1031
#define NTAG5_CONFIG_I2C_AUTH_LIMIT 0x1032
#define NTAG5_CONFIG_I2C_PWD_0 0x1033
#define NTAG5_CONFIG_I2C_PWD_1 0x1034
#define NTAG5_CONFIG_I2C_PWD_2 0x1035
#define NTAG5_CONFIG_I2C_PWD_3 0x1036
#define NTAG5_CONFIG_CONFIG 0x1037
#define NTAG5_CONFIG_SYNC_DATA_BLOCK 0x1038
#define NTAG5_CONFIG_PWM_GPIO_CONFIG 0x1039
#define NTAG5_CONFIG_PWM0_ON_OFF 0x103A
#define NTAG5_CONFIG_PWM1_ON_OFF 0x103B
#define NTAG5_CONFIG_WDT_CFG_AND_SRAM_COPY 0x103C
#define NTAG5_CONFIG_EH_AND_ED_CONFIG 0x103D
#define NTAG5_CONFIG_I2C_SLAVE_MASTER_CFG 0x103E
#define NTAG5_CONFIG_SEC_SRAM_AND_PP_AREA_1 0x103F
#define NTAG5_CONFIG_SRAM_DEFAULT 0x1045
#define NTAG5_CONFIG_AFI 0x1055
#define NTAG5_CONFIG_DSFID 0x1056
#define NTAG5_CONFIG_EAS_ID 0x1057
#define NTAG5_CONFIG_NFC_PP_AREA_0_AND_PPC 0x1058
#define NTAG5_CONFIG_NFC_LOCK_BLOCK 0x106A
#define NTAG5_CONFIG_I2C_LOCK_BLOCK 0x108A
#define NTAG5_CONFIG_NFC_SECTION_LOCK 0x1092
#define NTAG5_CONFIG_I2C_SECTION_LOCK 0x1094
#define NTAG5_CONFIG_I2C_PWD_0_AUTH 0x1096
#define NTAG5_CONFIG_I2C_PWD_1_AUTH 0x1097
#define NTAG5_CONFIG_I2C_PWD_2_AUTH 0x1098
#define NTAG5_CONFIG_I2C_PWD_3_AUTH 0x1099

/// @brief NTAG 5 Link session register list

#define NTAG5_SESSION_REG_STATUS 0x10A0
#define NTAG5_SESSION_REG_CONFIG 0x10A1
#define NTAG5_SESSION_REG_SYNC_DATA_BLOCK 0x10A2
#define NTAG5_SESSION_REG_PWM_GPIO_CONFIG 0x10A3
#define NTAG5_SESSION_REG_PWM0_ON_OFF 0x10A4
#define NTAG5_SESSION_REG_PWM1_ON_OFF 0x10A5
#define NTAG5_SESSION_REG_WDT_CONFIG 0x10A6
#define NTAG5_SESSION_REG_EH_CONFIG 0x10A7
#define NTAG5_SESSION_REG_ED_CONFIG 0x10A8
#define NTAG5_SESSION_REG_I2C_SLAVE_CONFIG 0x10A9
#define NTAG5_SESSION_REG_RESET_GEN 0x10AA
#define NTAG5_SESSION_REG_ED_INTR_CLEAR 0x10AB
#define NTAG5_SESSION_REG_I2C_MASTER_CONFIG 0x10AC
#define NTAG5_SESSION_REG_I2C_MASTER_STATUS 0x10AD
#define NTAG5_SESSION_REG_BYTE_0 0x00
#define NTAG5_SESSION_REG_BYTE_1 0x01
#define NTAG5_SESSION_REG_BYTE_2 0x02
#define NTAG5_SESSION_REG_BYTE_3 0x03

/// @brief NTAG 5 Link memory organization

#define NTAG5_USER_MEMORY_ADDRESS_MIN 0x0000
#define NTAG5_USER_MEMORY_ADDRESS_MAX 0x01FF
#define NTAG5_CONFIG_MEMORY_ADDRESS_MIN 0x1000
#define NTAG5_CONFIG_MEMORY_ADDRESS_MAX 0x109F
#define NTAG5_SESSION_REG_ADDRESS_MIN 0x10A0
#define NTAG5_SESSION_REG_ADDRESS_MAX 0x10AF
#define NTAG5_MEMORY_BLOCK_SIZE 4

/// @brief NTAG 5 Link CONFIG registers setting

#define NTAG5_CONFIG_0_SRAM_COPY_ENABLE 0x80
#define NTAG5_CONFIG_0_EH_LOW_FIELD_STR 0x08
#define NTAG5_CONFIG_0_EH_HIGH_FIELD_STR 0x0C
#define NTAG5_CONFIG_0_LOCK_SESSION_REG 0x02
#define NTAG5_CONFIG_0_AUTO_STANDBY_MODE_EN 0x01
#define NTAG5_CONFIG_1_EH_ARBITER_MODE_EN 0x80
#define NTAG5_CONFIG_1_USE_CASE_I2C_SLAVE 0x00
#define NTAG5_CONFIG_1_USE_CASE_I2C_MASTER 0x10
#define NTAG5_CONFIG_1_USE_CASE_GPIO_PWM 0x20
#define NTAG5_CONFIG_1_USE_CASE_3_STATE 0x30
#define NTAG5_CONFIG_1_ARBITER_NORMAL_MODE 0x00
#define NTAG5_CONFIG_1_ARBITER_SRAM_MIRROR 0x04
#define NTAG5_CONFIG_1_ARBITER_SRAM_PT 0x08
#define NTAG5_CONFIG_1_ARBITER_SRAM_PHDC 0x0C
#define NTAG5_CONFIG_1_SRAM_ENABLE 0x02
#define NTAG5_CONFIG_1_PT_TRANSFER_I2C_NFC 0x00
#define NTAG5_CONFIG_1_PT_TRANSFER_NFC_I2C 0x01
#define NTAG5_CONFIG_2_GPIO1_IN_DISABLE 0x00
#define NTAG5_CONFIG_2_GPIO1_IN_PULL_UP 0x40
#define NTAG5_CONFIG_2_GPIO1_IN_ENABLE 0x80
#define NTAG5_CONFIG_2_GPIO1_IN_PULL_DOWN 0xC0
#define NTAG5_CONFIG_2_GPIO0_IN_DISABLE 0x00
#define NTAG5_CONFIG_2_GPIO0_IN_PULL_UP 0x10
#define NTAG5_CONFIG_2_GPIO0_IN_ENABLE 0x20
#define NTAG5_CONFIG_2_GPIO0_IN_PULL_DOWN 0x30
#define NTAG5_CONFIG_2_EXT_CMD_SUPPORT 0x08
#define NTAG5_CONFIG_2_LOCK_BLK_CMD_SUPPORT 0x04
#define NTAG5_CONFIG_2_GPIO1_HIGH_SLEW_RATE 0x02
#define NTAG5_CONFIG_2_GPIO0_HIGH_SLEW_RATE 0x01

/// @brief NTAG 5 Link NDEF message setting

#define NTAG5_CAPABILITY_CONTAINER_ADDRESS 0x0000
#define NTAG5_CAPABILITY_CONTAINER 0xE1, 0x40, 0x80, 0x01
#define NTAG5_NDEF_MESSAGE_START_ADDRESS 0x0001
#define NTAG5_TYPE_NDEF_MESSAGE 0x03
#define NTAG5_NDEF_RECORD_HEADER 0xD1
#define NTAG5_NDEF_TYPE_LENGTH 0x01
#define NTAG5_NDEF_URI_TYPE 'U'
#define NTAG5_NDEF_MESSAGE_END_MARK 0xFE

/// @brief NTAG 5 Link NDEF URI prefix list

#define NTAG5_URI_PREFIX_0 0x00  /**< N/A - no prefix               */
#define NTAG5_URI_PREFIX_1 0x01  /**< http://www.                   */
#define NTAG5_URI_PREFIX_2 0x02  /**< https://www.                  */
#define NTAG5_URI_PREFIX_3 0x03  /**< http://                       */
#define NTAG5_URI_PREFIX_4 0x04  /**< https://                      */
#define NTAG5_URI_PREFIX_5 0x05  /**< tel:                          */
#define NTAG5_URI_PREFIX_6 0x06  /**< mailto:                       */
#define NTAG5_URI_PREFIX_7 0x07  /**< ftp://anonymous:anonymous@    */
#define NTAG5_URI_PREFIX_8 0x08  /**< ftp://ftp.                    */
#define NTAG5_URI_PREFIX_9 0x09  /**< ftps://                       */
#define NTAG5_URI_PREFIX_10 0x0A /**< sftp://                       */
#define NTAG5_URI_PREFIX_11 0x0B /**< smb://                        */
#define NTAG5_URI_PREFIX_12 0x0C /**< nfs://                        */
#define NTAG5_URI_PREFIX_13 0x0D /**< ftp://                        */
#define NTAG5_URI_PREFIX_14 0x0E /**< dav://                        */
#define NTAG5_URI_PREFIX_15 0x0F /**< news:                         */
#define NTAG5_URI_PREFIX_16 0x10 /**< telnet://                     */
#define NTAG5_URI_PREFIX_17 0x11 /**< imap:                         */
#define NTAG5_URI_PREFIX_18 0x12 /**< rtsp://                       */
#define NTAG5_URI_PREFIX_19 0x13 /**< urn:                          */
#define NTAG5_URI_PREFIX_20 0x14 /**< pop:                          */
#define NTAG5_URI_PREFIX_21 0x15 /**< sip:                          */
#define NTAG5_URI_PREFIX_22 0x16 /**< sips:                         */
#define NTAG5_URI_PREFIX_23 0x17 /**< tftp:                         */
#define NTAG5_URI_PREFIX_24 0x18 /**< btspp://                      */
#define NTAG5_URI_PREFIX_25 0x19 /**< btl2cap://                    */
#define NTAG5_URI_PREFIX_26 0x1A /**< btgoep://                     */
#define NTAG5_URI_PREFIX_27 0x1B /**< tcpobex://                    */
#define NTAG5_URI_PREFIX_28 0x1C /**< irdaobex://                   */
#define NTAG5_URI_PREFIX_29 0x1D /**< file://                       */
#define NTAG5_URI_PREFIX_30 0x1E /**< urn:epc:id:                   */
#define NTAG5_URI_PREFIX_31 0x1F /**< urn:epc:tag:                  */
#define NTAG5_URI_PREFIX_32 0x20 /**< urn:epc:pat:                  */
#define NTAG5_URI_PREFIX_33 0x21 /**< urn:epc:raw:                  */
#define NTAG5_URI_PREFIX_34 0x22 /**< urn:epc:                      */
#define NTAG5_URI_PREFIX_35 0x23 /**< urn:nfc:                      */

//***************************************************************************//

struct ntag5_block {
    uint8_t data[NTAG5_MEMORY_BLOCK_SIZE];
};

//***************************************************************************//

int ntag5_write_block(const struct device* dev,
                      uint16_t addr,
                      const struct ntag5_block* block,
                      size_t count);

int ntag5_read_block(const struct device* dev,
                     uint16_t addr,
                     struct ntag5_block* block,
                     size_t count);

int ntag5_write_session_reg(const struct device* dev,
                            uint16_t addr,
                            uint8_t reg_byte,
                            uint8_t mask,
                            uint8_t data_in);

int ntag5_read_session_reg(const struct device* dev,
                           uint16_t addr,
                           uint8_t reg_byte,
                           uint8_t* data_out);

int ntag5_write_message(const struct device* dev,
                        uint16_t addr,
                        const uint8_t* message,
                        size_t message_len);

int ntag5_read_message(const struct device* dev,
                       uint16_t addr,
                       uint8_t* message,
                       size_t message_len);

int ntag5_format_memory(const struct device* dev);

//***************************************************************************//

#endif // DRIVERS_NTAG5_H_
