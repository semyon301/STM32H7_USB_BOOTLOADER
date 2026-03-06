#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "main.h"
#include "stm32h7xx_hal.h"

#define APP_ADDRESS_QSPI         0x90000000
#define BL_FLASH_START           0x08000000
#define BL_FLASH_END             0x0801FFFF

#define BOOTLOADER_BUTTON_PORT   GPIOC
#define BOOTLOADER_BUTTON_PIN    GPIO_PIN_5

#define BL_MAGIC_KEY             0xDEADBEEF
#define BL_FLAG_ADDRESS          0x0801F000

#define RX_BUFFER_SIZE          4096

#define MAX_FIRMWARE_SIZE (8 * 1024 * 1024)

typedef struct {
    uint32_t stack_pointer;
    uint32_t reset_handler;
    uint32_t firmware_size;
    uint32_t crc32;
} FirmwareHeader;

typedef struct {
    uint32_t magic_key;
    uint32_t stay_in_bootloader;
    uint32_t app_valid;
    uint32_t crc32;
    uint32_t reserved[4];
} __attribute__((packed, aligned(32))) BootloaderFlag;

typedef enum {
    BL_OK = 0,
    BL_ERROR,
    BL_CRC_ERROR,
    BL_FLASH_ERROR,
    BL_TIMEOUT,
    BL_INVALID_HEADER,
    BL_FLASH_NOT_INIT
} BL_StatusTypeDef;

BL_StatusTypeDef BL_EraseQSPI(uint32_t address, uint32_t size);
BL_StatusTypeDef BL_WriteQSPI(uint32_t address, uint8_t *data, uint32_t size);
BL_StatusTypeDef BL_VerifyCRC(void);
void BL_JumpToApplication(void);
void BL_ProcessCommand(void);
void BL_Run(void);

uint8_t BL_ShouldEnterBootloader(void);
void BL_SetStayInBootloader(uint8_t stay);
void BL_SetAppValid(uint8_t valid);
void BL_ClearFlags(void);
uint8_t BL_IsAppValid(void);
void BL_UpdateFlagsCRC(void);

uint8_t BL_CheckButton(void);
void BL_ClearBuffer(void);
void BL_SaveFlags(void);
void BL_LoadFlags(void);
uint32_t BL_CalculateCRC32(uint8_t *data, uint32_t size);

#endif
