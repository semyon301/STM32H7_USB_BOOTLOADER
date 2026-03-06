#ifndef __W25Q64_H
#define __W25Q64_H

#include "stm32h7xx_hal.h"

#define W25Q64_FLASH_SIZE         0x800000
#define W25Q64_SECTOR_SIZE        0x1000
#define W25Q64_BLOCK_SIZE         0x10000
#define W25Q64_PAGE_SIZE          0x100

#define W25Q64_CMD_WRITE_ENABLE   0x06
#define W25Q64_CMD_WRITE_DISABLE  0x04
#define W25Q64_CMD_READ_STATUS1   0x05
#define W25Q64_CMD_WRITE_STATUS1  0x01
#define W25Q64_CMD_READ_DATA      0x03
#define W25Q64_CMD_FAST_READ      0x0B
#define W25Q64_CMD_PAGE_PROGRAM   0x02
#define W25Q64_CMD_SECTOR_ERASE   0x20
#define W25Q64_CMD_BLOCK_ERASE    0xD8
#define W25Q64_CMD_CHIP_ERASE     0xC7
#define W25Q64_CMD_POWER_DOWN     0xB9
#define W25Q64_CMD_RELEASE_PD     0xAB
#define W25Q64_CMD_DEVICE_ID      0x90
#define W25Q64_CMD_JEDEC_ID       0x9F
#define W25Q64_CMD_READ_UNIQUE_ID 0x4B
#define W25Q64_CMD_ENABLE_RESET   0x66
#define W25Q64_CMD_RESET_DEVICE   0x99

#define W25Q64_STATUS_BUSY        0x01
#define W25Q64_STATUS_WEL         0x02

#define W25Q64_MANUFACTURER_ID    0xEF
#define W25Q64_DEVICE_ID_JV       0x4017
#define W25Q64_DEVICE_ID_JQ       0x4018
#define W25Q32_DEVICE_ID          0x4016

typedef enum {
    W25QXX_OK          = 0,
    W25QXX_ERROR       = 1,
    W25QXX_TIMEOUT     = 2,
    W25QXX_BUSY        = 3,
    W25QXX_INVALID_ID  = 4
} W25QXX_StatusTypeDef;

W25QXX_StatusTypeDef W25QXX_Init(void);
W25QXX_StatusTypeDef W25QXX_Read(uint8_t *pData, uint32_t address, uint32_t size);
W25QXX_StatusTypeDef W25QXX_Write(uint8_t *pData, uint32_t address, uint32_t size);
W25QXX_StatusTypeDef W25QXX_EraseSector(uint32_t sectorAddress);
W25QXX_StatusTypeDef W25QXX_EraseBlock(uint32_t blockAddress);
W25QXX_StatusTypeDef W25QXX_EraseChip(void);
W25QXX_StatusTypeDef W25QXX_ReadJEDECID(uint8_t *manufacturerID, uint8_t *memoryType, uint8_t *capacity);
W25QXX_StatusTypeDef W25QXX_ReadID(uint8_t *manufacturerID, uint8_t *deviceID);
uint32_t W25QXX_GetCapacity(void);
W25QXX_StatusTypeDef W25QXX_WaitForWriteEnd(void);
W25QXX_StatusTypeDef W25QXX_Reset(void);
W25QXX_StatusTypeDef W25QXX_EnableXIPMode(void);
W25QXX_StatusTypeDef W25QXX_EnterPowerDown(void);
W25QXX_StatusTypeDef W25QXX_ReleasePowerDown(void);
W25QXX_StatusTypeDef W25QXX_WriteEnable(void);

W25QXX_StatusTypeDef W25QXX_ReadStatus1(uint8_t *status);
W25QXX_StatusTypeDef W25QXX_ReadStatus2(uint8_t *status);

#endif /* __W25Q64_H */
