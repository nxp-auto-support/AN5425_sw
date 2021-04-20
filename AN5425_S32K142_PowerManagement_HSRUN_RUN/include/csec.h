/*
 * csec.h
 *
 *  Created on: Jul 24, 2017
 *      Author: B50982
 */

#ifndef CSEC_H_
#define CSEC_H_

#include "device_registers.h"

#define ENABLE_TIME_MEASUREMENT             (0)

typedef enum {
    eCSEc_success = 0,
    eCSEc_error,
    eCSEc_partitionFailed,
    eCSEc_InvalidArgs,
    eCSEc_cmd_error,
    eCSEc_RND_not_initialized
}eCSEc_status;

typedef enum {
    SECRET_KEY      = 0x00U,
    MASTER_ECU_KEY  = 0x01,
    BOOT_MAC_KEY    = 0x02,
    BOOT_MAC        = 0x03,
    KEY_1           = 0x04,
    KEY_2           = 0x05,
    KEY_3           = 0x06,
    KEY_4           = 0x07,
    KEY_5           = 0x08,
    KEY_6           = 0x09,
    KEY_7           = 0x0A,
    KEY_8           = 0x0B,
    KEY_9           = 0x0C,
    KEY_10          = 0x0D,
    RAM_KEY         = 0x0FU,
    KEY_11          = 0x14U,
    KEY_12          = 0x15,
    KEY_13          = 0x16,
    KEY_14          = 0x17,
    KEY_15          = 0x18,
    KEY_16          = 0x19,
    KEY_17          = 0x1A,
} csec_key_id_t;

eCSEc_status CSEc_init (uint8_t keys, uint8_t enableVerifyOnlyAttribute);
uint8_t CSEc_getPartitionCode (void);

eCSEc_status CSEc_load_plain_KEY(uint32_t *key);

eCSEc_status CSEc_generate_MAC (uint32_t *mac, uint32_t *data, csec_key_id_t key_id, uint32_t message_length);
eCSEc_status CSEc_generate_MAC_pointer (uint32_t *mac, uint32_t ptr, csec_key_id_t key_id, uint32_t message_length);

eCSEc_status CSEc_MAC_verify (uint32_t *mac, uint32_t mac_length, uint32_t *data, uint32_t message_length, csec_key_id_t key_id, uint16_t *validation);
eCSEc_status CSEc_MAC_verify_pointer (uint32_t *mac, uint32_t mac_length, uint32_t ptr, uint32_t message_length, csec_key_id_t key_id, uint16_t *validation);

eCSEc_status CSEc_ENC_ECB (uint32_t *cipher_text, uint32_t *plain_text, csec_key_id_t key_id, uint16_t page_lenght);
eCSEc_status CSEc_DEC_ECB (uint32_t *plain_text, uint32_t *cipher_text, csec_key_id_t key_id, uint16_t page_lenght);


#endif /* CSEC_H_ */
