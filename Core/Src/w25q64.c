#include "w25q64.h"
#include "main.h"
#include <string.h>

extern QSPI_HandleTypeDef hqspi;

static QSPI_CommandTypeDef s_command;

W25QXX_StatusTypeDef W25QXX_ReadStatus1(uint8_t *status)
{
    QSPI_CommandTypeDef cmd;
    memset(&cmd, 0, sizeof(QSPI_CommandTypeDef));

    cmd.Instruction = W25Q64_CMD_READ_STATUS1;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.DummyCycles = 0;
    cmd.NbData = 1;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    memset(&s_command, 0, sizeof(QSPI_CommandTypeDef));

    if (HAL_QSPI_Command(&hqspi, &cmd, 100) != HAL_OK) {
        return W25QXX_ERROR;
    }

    if (HAL_QSPI_Receive(&hqspi, status, 100) != HAL_OK) {
        return W25QXX_ERROR;
    }

    return W25QXX_OK;
}

W25QXX_StatusTypeDef W25QXX_ReadStatus2(uint8_t *status)
{
    s_command.Instruction = 0x35;
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.AddressMode = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode = QSPI_DATA_1_LINE;
    s_command.DummyCycles = 0;
    s_command.NbData = 1;
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &s_command, 100) != HAL_OK) {
        return W25QXX_ERROR;
    }

    if (HAL_QSPI_Receive(&hqspi, status, 100) != HAL_OK) {
        return W25QXX_ERROR;
    }

    return W25QXX_OK;
}

W25QXX_StatusTypeDef W25QXX_Reset(void)
{
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction = W25Q64_CMD_ENABLE_RESET;
    s_command.AddressMode = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode = QSPI_DATA_NONE;
    s_command.DummyCycles = 0;
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &s_command, 100) != HAL_OK) {
        return W25QXX_ERROR;
    }

    for(volatile uint32_t i = 0; i < 100; i++);

    s_command.Instruction = W25Q64_CMD_RESET_DEVICE;
    if (HAL_QSPI_Command(&hqspi, &s_command, 100) != HAL_OK) {
        return W25QXX_ERROR;
    }

    HAL_Delay(30);

    return W25QXX_OK;
}

W25QXX_StatusTypeDef W25QXX_ReadJEDECID(uint8_t *manufacturerID, uint8_t *memoryType, uint8_t *capacity)
{
    uint8_t id[3];

    memset(&s_command, 0, sizeof(QSPI_CommandTypeDef));

    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction = W25Q64_CMD_JEDEC_ID;
    s_command.AddressMode = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode = QSPI_DATA_1_LINE;
    s_command.DummyCycles = 0;
    s_command.NbData = 3;
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &s_command, 100) != HAL_OK) {
        return W25QXX_ERROR;
    }

    if (HAL_QSPI_Receive(&hqspi, id, 100) != HAL_OK) {
        return W25QXX_ERROR;
    }

    *manufacturerID = id[0];
    *memoryType = id[1];
    *capacity = id[2];

    return W25QXX_OK;
}

W25QXX_StatusTypeDef W25QXX_ReadID(uint8_t *manufacturerID, uint8_t *deviceID)
{
    uint8_t id[2];

    memset(&s_command, 0, sizeof(QSPI_CommandTypeDef));

    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction = W25Q64_CMD_DEVICE_ID;
    s_command.AddressMode = QSPI_ADDRESS_1_LINE;
    s_command.AddressSize = QSPI_ADDRESS_24_BITS;
    s_command.Address = 0x000000;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode = QSPI_DATA_1_LINE;
    s_command.DummyCycles = 0;
    s_command.NbData = 2;
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &s_command, 100) != HAL_OK) {
        return W25QXX_ERROR;
    }

    if (HAL_QSPI_Receive(&hqspi, id, 100) != HAL_OK) {
        return W25QXX_ERROR;
    }

    *manufacturerID = id[0];
    *deviceID = id[1];

    return W25QXX_OK;
}

W25QXX_StatusTypeDef W25QXX_Init(void)
{
    uint8_t manufacturerID, memoryType, capacity;
    uint8_t status1, status2;

    memset(&s_command, 0, sizeof(QSPI_CommandTypeDef));

    if (W25QXX_ReleasePowerDown() != W25QXX_OK) {
    }
    HAL_Delay(5);

    if (W25QXX_Reset() != W25QXX_OK) {
    }
    HAL_Delay(30);

    if (W25QXX_ReadStatus1(&status1) != W25QXX_OK) {
        return W25QXX_ERROR;
    }

    if (W25QXX_ReadStatus2(&status2) != W25QXX_OK) {
        return W25QXX_ERROR;
    }

    for(int retry = 0; retry < 5; retry++) {
        if (W25QXX_ReadJEDECID(&manufacturerID, &memoryType, &capacity) == W25QXX_OK) {
            if (manufacturerID != W25Q64_MANUFACTURER_ID) {
                HAL_Delay(10);
                continue;
            }

            uint16_t deviceID = (memoryType << 8) | capacity;
            if (deviceID == W25Q64_DEVICE_ID_JV || deviceID == W25Q64_DEVICE_ID_JQ) {
                return W25QXX_OK;
            } else if (deviceID == 0x4016) {
                return W25QXX_OK;
            }
        }
        HAL_Delay(10);
    }

    uint8_t devID1, devID2;
    for(int retry = 0; retry < 3; retry++) {
        if (W25QXX_ReadID(&devID1, &devID2) == W25QXX_OK) {
            if (devID1 == W25Q64_MANUFACTURER_ID) {
                if (devID2 == 0x17 || devID2 == 0x18 || devID2 == 0x16) {
                    return W25QXX_OK;
                }
            }
        }
        HAL_Delay(10);
    }

    return W25QXX_INVALID_ID;
}

W25QXX_StatusTypeDef W25QXX_WaitForWriteEnd(void)
{
    uint8_t status;
    uint32_t timeout = 10000;

    do {
        if (W25QXX_ReadStatus1(&status) != W25QXX_OK) {
            return W25QXX_ERROR;
        }

        if (timeout-- == 0) {
            return W25QXX_TIMEOUT;
        }

        for(volatile uint32_t i = 0; i < 100; i++);
    } while ((status & W25Q64_STATUS_BUSY) == W25Q64_STATUS_BUSY);

    return W25QXX_OK;
}

W25QXX_StatusTypeDef W25QXX_WriteEnable(void)
{
    s_command.Instruction = W25Q64_CMD_WRITE_ENABLE;
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.AddressMode = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode = QSPI_DATA_NONE;
    s_command.DummyCycles = 0;
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &s_command, 100) != HAL_OK) {
        return W25QXX_ERROR;
    }

    uint8_t status;
    HAL_Delay(1);

    for(int retry = 0; retry < 10; retry++) {
        if (W25QXX_ReadStatus1(&status) == W25QXX_OK) {
            if (status & W25Q64_STATUS_WEL) {
                return W25QXX_OK;
            }
        }
        HAL_Delay(1);
    }

    return W25QXX_ERROR;
}

W25QXX_StatusTypeDef W25QXX_Read(uint8_t *pData, uint32_t address, uint32_t size)
{
    if (address + size > W25Q64_FLASH_SIZE) {
        return W25QXX_ERROR;
    }

    memset(&s_command, 0, sizeof(QSPI_CommandTypeDef));

    s_command.Instruction = W25Q64_CMD_READ_DATA;
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.AddressMode = QSPI_ADDRESS_1_LINE;
    s_command.AddressSize = QSPI_ADDRESS_24_BITS;
    s_command.Address = address;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode = QSPI_DATA_1_LINE;
    s_command.DummyCycles = 0;
    s_command.NbData = size;
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &s_command, 100) != HAL_OK) {
        return W25QXX_ERROR;
    }

    if (HAL_QSPI_Receive(&hqspi, pData, 1000) != HAL_OK) {
        return W25QXX_ERROR;
    }

    return W25QXX_OK;
}

W25QXX_StatusTypeDef W25QXX_EraseSector(uint32_t sectorAddress)
{
    if (sectorAddress % W25Q64_SECTOR_SIZE != 0) {
        return W25QXX_ERROR;
    }

    if (W25QXX_WriteEnable() != W25QXX_OK) {
        return W25QXX_ERROR;
    }

    memset(&s_command, 0, sizeof(QSPI_CommandTypeDef));
    s_command.Instruction = W25Q64_CMD_SECTOR_ERASE;
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.AddressMode = QSPI_ADDRESS_1_LINE;
    s_command.AddressSize = QSPI_ADDRESS_24_BITS;
    s_command.Address = sectorAddress;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode = QSPI_DATA_NONE;
    s_command.DummyCycles = 0;
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &s_command, 1000) != HAL_OK) {
        return W25QXX_ERROR;
    }

    return W25QXX_WaitForWriteEnd();
}

static W25QXX_StatusTypeDef W25QXX_WritePage(uint8_t *pData, uint32_t address, uint32_t size)
{
    if (size == 0 || size > W25Q64_PAGE_SIZE) {
        return W25QXX_ERROR;
    }

    uint32_t page_offset = address % W25Q64_PAGE_SIZE;
    if (page_offset + size > W25Q64_PAGE_SIZE) {
        return W25QXX_ERROR;
    }

    if (W25QXX_WriteEnable() != W25QXX_OK) {
        return W25QXX_ERROR;
    }

    memset(&s_command, 0, sizeof(QSPI_CommandTypeDef));
    s_command.Instruction = W25Q64_CMD_PAGE_PROGRAM;
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.AddressMode = QSPI_ADDRESS_1_LINE;
    s_command.AddressSize = QSPI_ADDRESS_24_BITS;
    s_command.Address = address;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode = QSPI_DATA_1_LINE;
    s_command.DummyCycles = 0;
    s_command.NbData = size;
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode = QSPI_SIOO_INST_ONLY_FIRST_CMD;

    if (HAL_QSPI_Command(&hqspi, &s_command, 1000) != HAL_OK) {
        return W25QXX_ERROR;
    }

    if (HAL_QSPI_Transmit(&hqspi, pData, 1000) != HAL_OK) {
        return W25QXX_ERROR;
    }

    return W25QXX_WaitForWriteEnd();
}

W25QXX_StatusTypeDef W25QXX_Write(uint8_t *pData, uint32_t address, uint32_t size)
{
    uint32_t remaining = size;
    uint32_t currentAddress = address;
    uint8_t *currentData = pData;

    while (remaining > 0) {
        uint32_t writeSize = (remaining > W25Q64_PAGE_SIZE) ? W25Q64_PAGE_SIZE : remaining;

        uint32_t pageOffset = currentAddress % W25Q64_PAGE_SIZE;

        if (pageOffset + writeSize > W25Q64_PAGE_SIZE) {
            writeSize = W25Q64_PAGE_SIZE - pageOffset;
        }

        if (W25QXX_WritePage(currentData, currentAddress, writeSize) != W25QXX_OK) {
            return W25QXX_ERROR;
        }

        currentAddress += writeSize;
        currentData += writeSize;
        remaining -= writeSize;

        HAL_Delay(1);
    }

    return W25QXX_OK;
}

W25QXX_StatusTypeDef W25QXX_EraseBlock(uint32_t blockAddress)
{
    if (blockAddress % W25Q64_BLOCK_SIZE != 0) {
        return W25QXX_ERROR;
    }

    if (W25QXX_WriteEnable() != W25QXX_OK) {
        return W25QXX_ERROR;
    }

    memset(&s_command, 0, sizeof(QSPI_CommandTypeDef));
    s_command.Instruction = W25Q64_CMD_BLOCK_ERASE;
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.AddressMode = QSPI_ADDRESS_1_LINE;
    s_command.AddressSize = QSPI_ADDRESS_24_BITS;
    s_command.Address = blockAddress;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode = QSPI_DATA_NONE;
    s_command.DummyCycles = 0;
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &s_command, 100) != HAL_OK) {
        return W25QXX_ERROR;
    }

    return W25QXX_WaitForWriteEnd();
}

W25QXX_StatusTypeDef W25QXX_EraseChip(void)
{
    if (W25QXX_WriteEnable() != W25QXX_OK) {
        return W25QXX_ERROR;
    }

    memset(&s_command, 0, sizeof(QSPI_CommandTypeDef));
    s_command.Instruction = W25Q64_CMD_CHIP_ERASE;
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.AddressMode = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode = QSPI_DATA_NONE;
    s_command.DummyCycles = 0;
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &s_command, 100) != HAL_OK) {
        return W25QXX_ERROR;
    }

    return W25QXX_WaitForWriteEnd();
}

uint32_t W25QXX_GetCapacity(void)
{
    return W25Q64_FLASH_SIZE;
}

W25QXX_StatusTypeDef W25QXX_EnterPowerDown(void)
{
    memset(&s_command, 0, sizeof(QSPI_CommandTypeDef));
    s_command.Instruction = W25Q64_CMD_POWER_DOWN;
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.AddressMode = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode = QSPI_DATA_NONE;
    s_command.DummyCycles = 0;
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &s_command, 100) != HAL_OK) {
        return W25QXX_ERROR;
    }

    HAL_Delay(1);
    return W25QXX_OK;
}

W25QXX_StatusTypeDef W25QXX_ReleasePowerDown(void)
{
    memset(&s_command, 0, sizeof(QSPI_CommandTypeDef));

    s_command.Instruction = W25Q64_CMD_RELEASE_PD;
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.AddressMode = QSPI_ADDRESS_1_LINE;
    s_command.AddressSize = QSPI_ADDRESS_24_BITS;
    s_command.Address = 0x000000;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode = QSPI_DATA_NONE;
    s_command.DummyCycles = 8;
    s_command.NbData = 0;
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode = QSPI_SIOO_INST_ONLY_FIRST_CMD;

    if (HAL_QSPI_Command(&hqspi, &s_command, 100) != HAL_OK) {
        return W25QXX_ERROR;
    }

    HAL_Delay(1);
    return W25QXX_OK;
}

W25QXX_StatusTypeDef W25QXX_EnableXIPMode(void)
{
    QSPI_CommandTypeDef cmd;
    QSPI_MemoryMappedTypeDef mmap_cfg;

    memset(&cmd, 0, sizeof(QSPI_CommandTypeDef));

    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction = W25Q64_CMD_FAST_READ;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.DummyCycles = 8;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    mmap_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_ENABLE;
    mmap_cfg.TimeOutPeriod = 100;

    if (HAL_QSPI_MemoryMapped(&hqspi, &cmd, &mmap_cfg) != HAL_OK) {
        return W25QXX_ERROR;
    }

    return W25QXX_OK;
}
