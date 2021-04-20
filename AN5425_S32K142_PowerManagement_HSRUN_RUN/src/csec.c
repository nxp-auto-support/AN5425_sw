/*
 * csec.c
 *
 *  Created on: Jul 24, 2017
 *      Author: B50982
 */

#include "csec.h"
#include "flash.h"
#include <string.h>

#define FLASH_COMMAND_BUFFER_SIZE       (12)
#define DEFAULT_PARAM_WORD_INDEX        (4)
#define WORDS_PER_PAGE                  (7 * 4) /* 7 Pages per 4 words each */
#define VALID_ALIGNMENT_128_BITS(x)     ((x & 0x3) == 0 ? (1) : (0))

typedef enum {
    CMD_ENC_ECB        = 0x01,
    CMD_ENC_CBC        = 0x02,
    CMD_DEC_ECB        = 0x03,
    CMD_DEC_CBC        = 0x04,
    CMD_GENERATE_MAC   = 0x05,
    CMD_VERIFY_MAC     = 0x06,
    CMD_LOAD_KEY       = 0x07,
    CMD_LOAD_PLAIN_KEY = 0x08,
    CMD_EXPORT_RAM_KEY = 0x09,
    CMD_INIT_RNG       = 0x0A,
    CMD_EXTEND_SEED    = 0x0B,
    CMD_RND            = 0x0C,
    CMD_RESERVED_1     = 0x0D,
    CMD_BOOT_FAILURE   = 0x0E,
    CMD_BOOT_OK        = 0x0F,
    CMD_GET_ID         = 0x10,
    CMD_BOOT_DEFINE    = 0x11,
    CMD_DBG_CHAL       = 0x12,
    CMD_DBG_AUTH       = 0x13,
    CMD_RESERVED_2     = 0x14,
    CMD_RESERVED_3     = 0x15,
    CMD_MP_COMPRESS    = 0x16
} csec_cmd_t;

typedef enum {
    CMD_FORMAT_COPY   = 0x00,
    CMD_FORMAT_ADDR
} csec_cmd_format_t;

typedef enum {
    CALL_SEQ_FIRST     = 0x00,
    CALL_SEQ_SUBSEQUENT
} csec_call_sequence_t;

typedef enum {
    NO_ERROR            = 1 << 0,
    SEQUENCE_ERROR      = 1 << 1,
    KEY_NOT_AVAILABLE   = 1 << 2,
    KEY_INVALID         = 1 << 3,
    KEY_EMPTY           = 1 << 4,
    NO_SECURE_BOOT      = 1 << 5,
    KEY_WRITE_PROTECTED = 1 << 6,
    KEY_UPDATE_ERROR    = 1 << 7,
    RNG_SEED            = 1 << 8,
    NO_DEBUGGING        = 1 << 9,
    MEMORY_FAILURE      = 1 << 10,
    GENERAL_ERROR       = 1 << 11,
} csec_error_code_t;

static eCommandStatus partition_cmd (uint8_t keys, uint8_t enableVerifyOnlyAttribute);
static uint16_t csec_cmd (uint32_t cmd_header) __attribute__ ((section(".code_ram")));
static eCSEc_status csec_get_output_paramters (uint32_t *dstPtr, uint8_t csec_index, uint32_t wordLength);
static eCSEc_status csec_set_input_paramters (uint32_t *srcPtr, uint8_t csec_index, uint32_t wordLength);

eCSEc_status CSEc_init (uint8_t keys, uint8_t enableVerifyOnlyAttribute)
{
    eCSEc_status ret = eCSEc_success;
    if ((0 != keys) && (keys <= 20))
    {
        /* Check if part is already partitioned */
        switch (CSEc_getPartitionCode())
        {
        /* There is no partition made */
        case 0x0F:
        {
            if (eFlash_NoError != partition_cmd(keys, enableVerifyOnlyAttribute))
            {
                ret = eCSEc_partitionFailed;
            }
        }
        break;
#if (defined(S32K11x_SERIES))
        /* Partition has been made, but there is no space for EEPROM */
        case 0x0B:
#endif
#if (defined(S32K146_SERIES) || defined(S32K144_SERIES) || defined(S32K142_SERIES))
            /* Partition has been made, but there is no space for EEPROM */
        case 0x0C:
#endif
        case 0x00:
        {
            ret = eCSEc_partitionFailed;
        }
        break;
        /* Partition already made */
        case 0x03:
        case 0x08:
        case 0x09:
        case 0x04:
        case 0x0A:
#if (!defined(S32K11x_SERIES))
        case 0x0B:
#endif
        {
            ret = eCSEc_success;
        }
        break;
        default:
            ret = eCSEc_error;
            break;
        }
    }
    else
    {
        ret = eCSEc_InvalidArgs;
    }
    return ret;
}

uint8_t CSEc_getPartitionCode (void)
{
    return (SIM->FCFG1 & SIM_FCFG1_DEPART_MASK) >> SIM_FCFG1_DEPART_SHIFT;
}

eCSEc_status CSEc_load_plain_KEY(uint32_t *key)
{
    eCSEc_status ret = eCSEc_success;
    uint32_t cmd_header = 0;
    uint16_t cmd_error_bits = 0;
    if (NULL != key)
    {
        /* The Load Plain Key command, loads a key ‘KEY’ without encryption and does a
         * verification of the key. The key is handed over in plain text. A plain key can only be
         * loaded into the RAM_KEY slot (per SHE specification). The command sets the plain key
         * status bit for the RAM_KEY. There is no return value. The command header fields in
         * Bytes [1,3] are ignored, while setting Byte[2] to 0x01 will create a sequence error */
        ret = csec_set_input_paramters(&key[0], DEFAULT_PARAM_WORD_INDEX, 4);
        if (eCSEc_success == ret)
        {
            cmd_header = (CMD_LOAD_PLAIN_KEY << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_FIRST << 8) | (0x00);
            cmd_error_bits = csec_cmd (cmd_header);
            if (NO_ERROR != cmd_error_bits)
            {
                ret = eCSEc_cmd_error;
            }
        }
    }
    else
    {
        ret = eCSEc_InvalidArgs;
    }
    return ret;
}

eCSEc_status CSEc_generate_MAC (uint32_t *mac, uint32_t *data, csec_key_id_t key_id, uint32_t message_length)
{
    /*
     * The Generate MAC command operates on a MESSAGE of length in bits of MESAGE_LENGTH.
     * The function uses a key (KEY_ID) to encode a MAC value (128-bits) for the full message body.
     * Data is processed in 128-bit blocks through MESSAGE_LENGH number of bits, with padding
     * done internally on the last block if not an integer divide by 128-bit.
     * Output does not reflect the CMAC value until the full message has been processed
     */
    eCSEc_status ret = eCSEc_success;
    uint32_t cmd_header = 0;
    uint16_t cmd_error_bits = 0;
    if ((NULL != mac) && (NULL != data) && (message_length != 0))
    {
        uint32_t pageIndex = 0;
        uint32_t wordsPerPage = 0;
        /* There are 7 parameters of 128-bits each in every command header */
        wordsPerPage = (message_length/128) * 4;
        do {
            ret = csec_set_input_paramters(&data[pageIndex * WORDS_PER_PAGE], DEFAULT_PARAM_WORD_INDEX, wordsPerPage > WORDS_PER_PAGE ? (WORDS_PER_PAGE) : (wordsPerPage));
            if (eCSEc_success == ret)
            {
                if (wordsPerPage > WORDS_PER_PAGE)
                {
                    wordsPerPage -= WORDS_PER_PAGE;
                }
                ret = csec_set_input_paramters(&message_length, 3, 1);
                if (eCSEc_success == ret)
                {
                    if (0 == pageIndex)
                    {
                        cmd_header = (CMD_GENERATE_MAC << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_FIRST << 8) | key_id; ;
                    }
                    else
                    {
                        cmd_header = (CMD_GENERATE_MAC << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_SUBSEQUENT << 8) | key_id;
                    }
                    cmd_error_bits = csec_cmd(cmd_header);
                    if (NO_ERROR != cmd_error_bits)
                    {
                        ret = eCSEc_cmd_error;
                    }
                    pageIndex++;
                }
            }
        } while ((pageIndex < (message_length / (128 * 7))) && (eCSEc_success == ret));
        if (eCSEc_success == ret)
        {
            ret = csec_get_output_paramters(mac, 8, 4);
        }
    }
    else
    {
        ret = eCSEc_InvalidArgs;
    }
    return ret;
}

eCSEc_status CSEc_generate_MAC_pointer (uint32_t *mac, uint32_t ptr, csec_key_id_t key_id, uint32_t message_length)
{
    /*
     * For data that is contained fully within the NVM macro the ‘pointer’ method of
     * communication is used. For internal NVM space the maximum size of data is limited to
     * be no more that one read partition (512 KB max), or less if the starting address is not the
     * start of the read partition (i.e., the address sequence cannot cross boundaries of read
     * partitions). The 'Flash Start Address' must be 128-bit aligned (the same is true for
     * VERIFY_MAC pointer method).
     */
    eCSEc_status ret = eCSEc_success;
    uint32_t cmd_header = 0;
    uint16_t cmd_error_bits = 0;
    if ((NULL != mac) && (message_length != 0) && (VALID_ALIGNMENT_128_BITS(ptr)))
    {
        uint32_t temp = ptr;
        ret = csec_set_input_paramters(&temp, DEFAULT_PARAM_WORD_INDEX, 1);
        if (eCSEc_success == ret)
        {
            ret = csec_set_input_paramters(&message_length, 3, 1);
            if (eCSEc_success == ret)
            {
                cmd_header = (CMD_GENERATE_MAC << 24) | (CMD_FORMAT_ADDR << 16) | (CALL_SEQ_FIRST << 8) | key_id;
                cmd_error_bits = csec_cmd(cmd_header);
                if (NO_ERROR != cmd_error_bits)
                {
                    ret = eCSEc_cmd_error;
                }
            }
        }
        if (eCSEc_success == ret)
        {
            ret = csec_get_output_paramters(mac, 8, 4);
        }
    }
    else
    {
        ret = eCSEc_InvalidArgs;
    }
    return ret;
}

eCSEc_status CSEc_MAC_verify (uint32_t *mac, uint32_t mac_length, uint32_t *data, uint32_t message_length, csec_key_id_t key_id, uint16_t *validation)
{
    eCSEc_status ret = eCSEc_success;
    uint32_t cmd_header = 0;
    uint16_t cmd_error_bits = 0;
    if ((NULL != mac) && (NULL != data) && (message_length != 0) && (0 != mac_length))
    {
        uint32_t pageIndex = 0;
        uint32_t wordsPerPage = 0;
        uint32_t macLen = mac_length << 16;
        uint32_t validationWord = 0;
        uint32_t macLengByte = 0;
        /* There are 7 parameters of 128-bits each in every command header */
        wordsPerPage = (message_length/128) * 4;
        macLengByte = (mac_length / 128) * 4;
        do {
            if (wordsPerPage > 0)
            {
                ret = csec_set_input_paramters(&data[pageIndex * WORDS_PER_PAGE], DEFAULT_PARAM_WORD_INDEX, wordsPerPage > WORDS_PER_PAGE ? (WORDS_PER_PAGE) : (wordsPerPage));
                if (eCSEc_success == ret)
                {
                    if (wordsPerPage <= 24)
                    {
                        ret = csec_set_input_paramters(&mac[0], DEFAULT_PARAM_WORD_INDEX + wordsPerPage, macLengByte);
                    }
                }
                if (wordsPerPage >= WORDS_PER_PAGE)
                {
                    wordsPerPage -= WORDS_PER_PAGE;
                }
            }
            else
            {
                ret = csec_set_input_paramters(&mac[0], DEFAULT_PARAM_WORD_INDEX + wordsPerPage, macLengByte);
            }
            if (eCSEc_success == ret)
            {
                ret = csec_set_input_paramters(&message_length, 3, 1);
                if (eCSEc_success == ret)
                {
                    ret = csec_set_input_paramters(&macLen, 2, 1);
                    if (eCSEc_success == ret)
                    {
                        if (0 == pageIndex)
                        {
                            cmd_header = (CMD_VERIFY_MAC << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_FIRST << 8) | key_id; ;
                        }
                        else
                        {
                            cmd_header = (CMD_VERIFY_MAC << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_SUBSEQUENT << 8) | key_id;
                        }
                        cmd_error_bits = csec_cmd(cmd_header);
                        if (NO_ERROR != cmd_error_bits)
                        {
                            ret = eCSEc_cmd_error;
                        }
                        pageIndex++;
                    }
                }
            }
        } while ((pageIndex < (message_length / (128 * 7)) + (mac_length / 128)) && (eCSEc_success == ret));
        if (eCSEc_success == ret)
        {
            ret = csec_get_output_paramters(&validationWord, 5, 2);
            *validation = (validationWord >> 16) && 0xFFFF;
        }
    }
    else
    {
        ret = eCSEc_InvalidArgs;
    }
    return ret;
}

eCSEc_status CSEc_MAC_verify_pointer (uint32_t *mac, uint32_t mac_length, uint32_t ptr, uint32_t message_length, csec_key_id_t key_id, uint16_t *validation)
{

    eCSEc_status ret = eCSEc_success;
    uint32_t cmd_header = 0;
    uint16_t cmd_error_bits = 0;
    if ((NULL != mac) && (0 != mac_length) && (message_length != 0) && (VALID_ALIGNMENT_128_BITS(ptr) && (NULL != validation)))
    {
        uint32_t temp = ptr;
        uint32_t macLenght = (mac_length && 0xFFFF) << 16;
        uint32_t tempValidation = 0;
        ret = csec_set_input_paramters(&temp, DEFAULT_PARAM_WORD_INDEX, 1);
        if (eCSEc_success == ret)
        {
            ret = csec_set_input_paramters(&message_length, 3, 1);
            if (eCSEc_success == ret)
            {
                ret = csec_set_input_paramters(&mac[0], DEFAULT_PARAM_WORD_INDEX + 4, (mac_length / 128) * 4);
                if (eCSEc_success == ret)
                {
                    ret = csec_set_input_paramters(&macLenght, 2, 1);
                    if (eCSEc_success == ret)
                    {
                        cmd_header = (CMD_GENERATE_MAC << 24) | (CMD_FORMAT_ADDR << 16) | (CALL_SEQ_FIRST << 8) | key_id;
                        cmd_error_bits = csec_cmd(cmd_header);
                        if (NO_ERROR != cmd_error_bits)
                        {
                            ret = eCSEc_cmd_error;
                        }
                    }
                }
            }
        }
        if (eCSEc_success == ret)
        {
            ret = csec_get_output_paramters(&tempValidation, 5, 2);
            if (eCSEc_success == ret)
            {
                *validation = (tempValidation >> 16) & 0xFFFF;
            }
        }
    }
    else
    {
        ret = eCSEc_InvalidArgs;
    }
    return ret;
}

eCSEc_status CSEc_ENC_ECB (uint32_t *cipher_text, uint32_t *plain_text, csec_key_id_t key_id, uint16_t page_lenght)
{
    eCSEc_status ret = eCSEc_success;
    uint32_t cmd_header = 0;
    uint16_t cmd_error_bits = 0;
    if ((NULL != cipher_text) && (NULL != plain_text) && (0 != page_lenght))
    {
        uint32_t pageIndex = 0;
        uint32_t wordsPerPage = page_lenght * 4;
        uint32_t pageLengthPtr = (uint32_t)page_lenght;
        do {
            ret = csec_set_input_paramters(&plain_text[pageIndex * WORDS_PER_PAGE], DEFAULT_PARAM_WORD_INDEX, wordsPerPage > WORDS_PER_PAGE ? (WORDS_PER_PAGE) : (wordsPerPage));
            if (eCSEc_success == ret)
            {
                ret = csec_set_input_paramters(&pageLengthPtr, 3, 1);
                if (eCSEc_success == ret)
                {
                    if (0 == pageIndex)
                    {
                        cmd_header = (CMD_ENC_ECB << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_FIRST << 8) | key_id;
                    }
                    else
                    {
                        cmd_header = (CMD_ENC_ECB << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_SUBSEQUENT << 8) | key_id;
                    }
                    cmd_error_bits = csec_cmd(cmd_header);
                    if (NO_ERROR != cmd_error_bits)
                    {
                        ret = eCSEc_cmd_error;
                    }
                    if (eCSEc_success == ret)
                    {
                        ret = csec_get_output_paramters(&cipher_text[pageIndex * WORDS_PER_PAGE], DEFAULT_PARAM_WORD_INDEX, wordsPerPage > WORDS_PER_PAGE ? (WORDS_PER_PAGE) : (wordsPerPage));
                        if (eCSEc_success == ret)
                        {
                            if (wordsPerPage > WORDS_PER_PAGE)
                            {
                                wordsPerPage -= WORDS_PER_PAGE;
                            }
                            pageIndex++;
                        }
                    }
                }
            }
        } while ((pageIndex < (page_lenght / (7))) && (eCSEc_success == ret));
    }
    else
    {
        ret = eCSEc_InvalidArgs;
    }
    return ret;
}

eCSEc_status CSEc_DEC_ECB (uint32_t *plain_text, uint32_t *cipher_text, csec_key_id_t key_id, uint16_t page_lenght)
{
    eCSEc_status ret = eCSEc_success;
    uint32_t cmd_header = 0;
    uint16_t cmd_error_bits = 0;
    if ((NULL != cipher_text) && (NULL != plain_text) && (0 != page_lenght))
    {
        uint32_t pageIndex = 0;
        uint32_t wordsPerPage = page_lenght * 4;
        uint32_t pageLengthPtr = (uint32_t)page_lenght;
        do {
            ret = csec_set_input_paramters(&cipher_text[pageIndex * WORDS_PER_PAGE], DEFAULT_PARAM_WORD_INDEX, wordsPerPage > WORDS_PER_PAGE ? (WORDS_PER_PAGE) : (wordsPerPage));
            if (eCSEc_success == ret)
            {
                ret = csec_set_input_paramters(&pageLengthPtr, 3, 1);
                if (eCSEc_success == ret)
                {
                    if (0 == pageIndex)
                    {
                        cmd_header = (CMD_DEC_ECB << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_FIRST << 8) | key_id; ;
                    }
                    else
                    {
                        cmd_header = (CMD_DEC_ECB << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_SUBSEQUENT << 8) | key_id;
                    }
                    cmd_error_bits = csec_cmd(cmd_header);
                    if (NO_ERROR != cmd_error_bits)
                    {
                        ret = eCSEc_cmd_error;
                    }
                    if (eCSEc_success == ret)
                    {
                        ret = csec_get_output_paramters(&plain_text[pageIndex * WORDS_PER_PAGE], DEFAULT_PARAM_WORD_INDEX, wordsPerPage > WORDS_PER_PAGE ? (WORDS_PER_PAGE) : (wordsPerPage));
                        if (eCSEc_success == ret)
                        {
                            if (wordsPerPage > WORDS_PER_PAGE)
                            {
                                wordsPerPage -= WORDS_PER_PAGE;
                            }
                            pageIndex++;
                        }
                    }
                }
            }
        } while ((pageIndex < (page_lenght / (7))) && (eCSEc_success == ret));
    }
    else
    {
        ret = eCSEc_InvalidArgs;
    }
    return ret;
}

static eCommandStatus partition_cmd (uint8_t keys, uint8_t enableVerifyOnlyAttribute)
{
    uint8_t params;
    uint8_t flashCommandBuffer[FLASH_COMMAND_BUFFER_SIZE] = {0};
    uint8_t keysCode = 0;

    /* Launch partition code */
    params = 0;
    memset((void *)&flashCommandBuffer[0], 0x00, (size_t)FLASH_COMMAND_BUFFER_SIZE);

    /* Do partition */
    if (keys == 0)
    {
        keysCode = 0;
    }
    else if (keys <= 5)
    {
        keysCode = 0b01;
    }
    else if (keys <= 10)
    {
        keysCode = 0b10;
    }
    else if (keys <= 20)
    {
        keysCode = 0b11;
    }

    /* FCCOB0: Selects the PGMPART command */
    flashCommandBuffer[params++] = 0x80;
    /* FCCOB1: CSEc Key Size */
    flashCommandBuffer[params++] = keysCode;
    /* FCCOB2: SFE */
    flashCommandBuffer[params++] = enableVerifyOnlyAttribute ? 0x01 : 0x00;
    /* FCCOB3: FlexRAM loading reset option: 0 - FlexRAM loaded with valid EEPROM, 1 - FlexRAM not loaded */
    flashCommandBuffer[params++] = 0x00;
    /* FCCOB4: EEPROM data set size code: EEESIZE = 2 (4kB) */
    flashCommandBuffer[params++] = 0x02;
#if (defined(S32K11x_SERIES))
    flashCommandBuffer[params++] = 0x09;    /* FCCOB5: FlexNVM Partition code: DEPART = 0x9 (Data flash: 8, EEPROM backup: 24kB) */
#elif (defined(S32K146_SERIES) || defined(S32K144_SERIES) || defined(S32K142_SERIES))
    flashCommandBuffer[params++] = 0x0A;    /* FCCOB5: FlexNVM Partition code: DEPART = 0xA (Data flash: 16, EEPROM backup: 48kB) */
#elif (defined(S32K148_SERIES))
    flashCommandBuffer[params++] = 0x04;    /* FCCOB5: FlexNVM Partition code: DEPART = 0x4 (Data flash: 0, EEPROM backup: 64kB) */
#else
#error Invalid CPU selection
#endif
    return flash_command(&flashCommandBuffer[0], params);
}

__attribute__ ((section(".code_ram"))) /* Flash command is launched from RAM */
static uint16_t csec_cmd (uint32_t cmd_header)
{
    /* Wait for previous command to be finished */
    while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != FTFC_FSTAT_CCIF_MASK);
#if ENABLE_TIME_MEASUREMENT
    start_time();
#endif
#if ENABLE_TIME_MEASUREMENT_BY_GPIO
    GPIO_PTR->PSOR |= 1 << PIN;
#endif
    CSE_PRAM->RAMn[0].DATA_32 = cmd_header;
    /* Wait until command is completed */
    while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) == 0x00);
#if ENABLE_TIME_MEASUREMENT_BY_GPIO
    GPIO_PTR->PCOR |= 1 << PIN;
#endif
#if ENABLE_TIME_MEASUREMENT
    stop_time();
#endif
    return (uint16_t)(CSE_PRAM->RAMn[1].DATA_32 >> 16);
}

static eCSEc_status csec_get_output_paramters (uint32_t *dstPtr, uint8_t csec_index, uint32_t wordLength)
{
    eCSEc_status ret = eCSEc_success;
    if ((NULL != dstPtr) && (0 < wordLength))
    {
        uint32_t index = 0, dstIndex = 0;
        for (index = csec_index, dstIndex = 0; index < (csec_index + wordLength); index++)
        {
            dstPtr[dstIndex++] = CSE_PRAM->RAMn[index].DATA_32;
        }
    }
    else
    {
        ret = eCSEc_InvalidArgs;
    }
    return ret;
}

static eCSEc_status csec_set_input_paramters (uint32_t *srcPtr, uint8_t csec_index, uint32_t wordLength)
{
    eCSEc_status ret = eCSEc_success;
    if ((NULL != srcPtr) && (0 < wordLength))
    {
        uint32_t index = 0, srcIndex = 0;
        for (index = csec_index, srcIndex = 0; index < (csec_index + wordLength); index++)
        {
            CSE_PRAM->RAMn[index].DATA_32 = srcPtr[srcIndex++];
        }
    }
    else
    {
        ret = eCSEc_InvalidArgs;
    }
    return ret;
}

