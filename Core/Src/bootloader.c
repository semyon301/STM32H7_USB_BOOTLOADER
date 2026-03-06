#include "bootloader.h"
#include "usbd_cdc_if.h"
#include "w25q64.h"
#include <string.h>
#include <stdio.h>

extern QSPI_HandleTypeDef hqspi;
extern CRC_HandleTypeDef hcrc;
extern USBD_HandleTypeDef hUsbDeviceFS;

static uint8_t rx_buffer[RX_BUFFER_SIZE];
static uint32_t rx_index = 0;
static volatile uint8_t cmd_received = 0;
static volatile uint8_t buffer_lock = 0;

static BootloaderFlag bl_flags;

void CDC_Receive_FS_Callback(uint8_t* Buf, uint32_t *Len)
{
    // Защита от гонок данных
    if (buffer_lock) return;
    buffer_lock = 1;

    if (*Len > 0) {
        if ((rx_index + *Len) >= sizeof(rx_buffer)) {
            rx_index = 0;
            cmd_received = 0;
            memset(rx_buffer, 0, sizeof(rx_buffer));
        }

        // Копируем данные
        memcpy(rx_buffer + rx_index, Buf, *Len);
        rx_index += *Len;
        cmd_received = 1;
    }

    buffer_lock = 0;
}

void BL_ClearBuffer(void)
{
    buffer_lock = 1;
    rx_index = 0;
    cmd_received = 0;
    memset(rx_buffer, 0, sizeof(rx_buffer));
    buffer_lock = 0;
}

uint8_t BL_CheckButton(void)
{
    HAL_Delay(10);

    uint8_t button_state = HAL_GPIO_ReadPin(BOOTLOADER_BUTTON_PORT, BOOTLOADER_BUTTON_PIN);

    return (button_state == GPIO_PIN_RESET);
}

uint32_t BL_CalculateCRC32(uint8_t *data, uint32_t size)
{
    __HAL_CRC_DR_RESET(&hcrc);
    HAL_CRC_Init(&hcrc);
    hcrc.Instance->DR = 0xFFFFFFFF;

    uint32_t word_count = (size + 3) / 4;

    for (uint32_t i = 0; i < word_count; i++) {
        uint32_t word = 0;
        uint32_t bytes_remaining = size - (i * 4);
        uint32_t bytes_to_copy = bytes_remaining > 4 ? 4 : bytes_remaining;

        memcpy(&word, data + (i * 4), bytes_to_copy);
        hcrc.Instance->DR = word;
    }

    return hcrc.Instance->DR;
}

void BL_UpdateFlagsCRC(void)
{
    __HAL_CRC_DR_RESET(&hcrc);
    HAL_CRC_Init(&hcrc);
    hcrc.Instance->DR = 0xFFFFFFFF;

    uint32_t *data = (uint32_t*)&bl_flags;

    for(uint32_t i = 0; i < 7; i++) {
        hcrc.Instance->DR = data[i];
    }

    bl_flags.crc32 = hcrc.Instance->DR;
}

void BL_SaveFlags(void)
{
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SectorError = 0;

    uint32_t Sector = FLASH_SECTOR_4;
    uint32_t Bank = FLASH_BANK_1;

    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.Banks = Bank;
    EraseInitStruct.Sector = Sector;
    EraseInitStruct.NbSectors = 1;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK) {
        HAL_FLASH_Lock();
        return;
    }

    BL_UpdateFlagsCRC();

    uint32_t data[8];
    memset(data, 0, sizeof(data));
    memcpy(data, &bl_flags, sizeof(bl_flags));

    uint64_t flash_data[4];
    for(int i = 0; i < 4; i++) {
        flash_data[i] = ((uint64_t)data[i*2+1] << 32) | data[i*2];
    }

    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, BL_FLAG_ADDRESS,
                         (uint64_t)(uintptr_t)flash_data) != HAL_OK) {
        HAL_FLASH_Lock();
        return;
    }

    HAL_FLASH_Lock();
}

void BL_LoadFlags(void)
{
    BootloaderFlag *flags = (BootloaderFlag*)BL_FLAG_ADDRESS;

    if(flags->magic_key != BL_MAGIC_KEY) {
        goto init_default_flags;
    }

    __HAL_CRC_DR_RESET(&hcrc);
    HAL_CRC_Init(&hcrc);
    hcrc.Instance->DR = 0xFFFFFFFF;

    uint32_t *data = (uint32_t*)flags;

    for(uint32_t i = 0; i < 7; i++) {
        hcrc.Instance->DR = data[i];
    }

    uint32_t calculated_crc = hcrc.Instance->DR;

    if(flags->crc32 == calculated_crc) {
        memcpy(&bl_flags, flags, sizeof(BootloaderFlag));
        return;
    }

init_default_flags:
    memset(&bl_flags, 0, sizeof(BootloaderFlag));
    bl_flags.magic_key = BL_MAGIC_KEY;
    bl_flags.stay_in_bootloader = 0;
    bl_flags.app_valid = 0;

    BL_UpdateFlagsCRC();
    BL_SaveFlags();
}

uint8_t BL_ShouldEnterBootloader(void)
{
    if(BL_CheckButton()) {
        return 1;
    }

    BL_LoadFlags();
    if(bl_flags.stay_in_bootloader) {
        return 1;
    }

    if(!BL_IsAppValid()) {
        return 1;
    }

    return 0;
}

void BL_SetStayInBootloader(uint8_t stay)
{
    bl_flags.stay_in_bootloader = stay;
    BL_SaveFlags();
}

void BL_SetAppValid(uint8_t valid)
{
    bl_flags.app_valid = valid;
    BL_SaveFlags();
}

void BL_ClearFlags(void)
{
    memset(&bl_flags, 0, sizeof(BootloaderFlag));
    bl_flags.magic_key = BL_MAGIC_KEY;
    bl_flags.stay_in_bootloader = 0;
    bl_flags.app_valid = 0;

    BL_UpdateFlagsCRC();
    BL_SaveFlags();
}

uint8_t BL_IsAppValid(void)
{
    static uint8_t flash_initialized = 0;

    if (!flash_initialized) {
        if (W25QXX_Init() != W25QXX_OK) {
            return 0;
        }
        flash_initialized = 1;
    }

    uint32_t vector_table[2];

    if (W25QXX_Read((uint8_t*)vector_table, 0, 8) != W25QXX_OK) {
        return 0;
    }

    uint32_t stack_pointer = vector_table[0];
    if (stack_pointer < 0x20000000 || stack_pointer > 0x2007FFFF) {
        return 0;
    }

    uint32_t reset_handler = vector_table[1];
    if (reset_handler == 0 || reset_handler == 0xFFFFFFFF) {
        return 0;
    }

    return 1;
}

BL_StatusTypeDef BL_WriteQSPI(uint32_t address, uint8_t *data, uint32_t size)
{
    uint32_t flash_offset = address - APP_ADDRESS_QSPI;

    if (flash_offset + size > W25Q64_FLASH_SIZE) {
        return BL_FLASH_ERROR;
    }

    if (W25QXX_Write(data, flash_offset, size) != W25QXX_OK) {
        return BL_FLASH_ERROR;
    }

    return BL_OK;
}

BL_StatusTypeDef BL_EraseQSPI(uint32_t address, uint32_t size)
{
    uint32_t flash_offset = address - APP_ADDRESS_QSPI;

    if (flash_offset + size > W25Q64_FLASH_SIZE) {
        return BL_FLASH_ERROR;
    }

    uint32_t start_sector = flash_offset / W25Q64_SECTOR_SIZE;
    uint32_t end_sector = (flash_offset + size + W25Q64_SECTOR_SIZE - 1) / W25Q64_SECTOR_SIZE;

    for (uint32_t i = start_sector; i < end_sector; i++) {
        if (W25QXX_EraseSector(i * W25Q64_SECTOR_SIZE) != W25QXX_OK) {
            return BL_FLASH_ERROR;
        }
    }

    return BL_OK;
}

BL_StatusTypeDef BL_VerifyCRC(void)
{
	//NOT YET IMPLEMENTED, STUB FUNCTION FOR NOW
    return BL_OK;
}

// Прыжок в приложение
void BL_JumpToApplication(void)
{
    uint32_t vector_table[2];

    if (W25QXX_Read((uint8_t*)vector_table, 0, 8) != W25QXX_OK) {
        return;
    }

    if (vector_table[0] == 0 || vector_table[1] == 0 ||
        vector_table[0] == 0xFFFFFFFF || vector_table[1] == 0xFFFFFFFF) {
        return;
    }

    if (W25QXX_EnableXIPMode() != W25QXX_OK) {
        for (int i = 0; i < 10; i++) {
            HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
            HAL_Delay(100);
        }
        return;
    }

    BL_SetAppValid(1);

    BL_SetStayInBootloader(0);

    __disable_irq();

    HAL_RCC_DeInit();

    HAL_PWREx_DisableUSBVoltageDetector();
    __HAL_RCC_USB_OTG_FS_CLK_DISABLE();

    HAL_CRC_DeInit(&hcrc);

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    SCB->VTOR = APP_ADDRESS_QSPI;

    __set_MSP(vector_table[0]);

    uint32_t reset_handler = vector_table[1];

    void (*app_entry)(void) = (void (*)(void))reset_handler;

    __enable_irq();

    app_entry();

    while(1);
}

void BL_ProcessCommand(void)
{
    if (!cmd_received) return;

    uint8_t response[64];
    BL_StatusTypeDef status = BL_ERROR;
    int written = 0;

    if (rx_buffer[0] == 'E') {  // Erase
        // Format: E<address(4 bytes)><size(4 bytes)>
        if (rx_index < 9) {
            return;
        }
        uint32_t address, size;
        memcpy(&address, rx_buffer + 1, sizeof(uint32_t));
        memcpy(&size, rx_buffer + 5, sizeof(uint32_t));

        status = BL_EraseQSPI(address, size);
        if (status == BL_OK) {
            BL_SetAppValid(0);
        }
        written = snprintf((char*)response, sizeof(response), "ERASE:%d\r\n", status);
        CDC_Transmit_FS(response, written);
        BL_ClearBuffer();
    }
    else if (rx_buffer[0] == 'W') {  // Write
        // Format: W<address(4)><size(4)><data>
        if (rx_index < 9) {
            return;
        }
        uint32_t address, size;
        memcpy(&address, rx_buffer + 1, sizeof(uint32_t));
        memcpy(&size, rx_buffer + 5, sizeof(uint32_t));

        if (size > (RX_BUFFER_SIZE - 9)) {
            written = snprintf((char*)response, sizeof(response), "ERROR:Data too large\r\n");
            CDC_Transmit_FS(response, written);
            BL_ClearBuffer();
            return;
        }

        if (rx_index < 9 + size) {
            return;
        }

        uint8_t *data = rx_buffer + 9;
        status = BL_WriteQSPI(address, data, size);
        written = snprintf((char*)response, sizeof(response), "WRITE:%d\r\n", status);
        CDC_Transmit_FS(response, written);
        BL_ClearBuffer();
    }
    else if (rx_buffer[0] == 'V') {  // Verify
        status = BL_VerifyCRC();
        if (status == BL_OK) {
            BL_SetAppValid(1);
        }
        written = snprintf((char*)response, sizeof(response), "VERIFY:%d\r\n", status);
        CDC_Transmit_FS(response, written);
        BL_ClearBuffer();
    }
    else if (rx_buffer[0] == 'J') {  // Jump
        written = snprintf((char*)response, sizeof(response), "JUMPING...\r\n");
        CDC_Transmit_FS(response, written);
        HAL_Delay(100);
        BL_JumpToApplication();
        // Никогда не вернётся
    }
    else if (rx_buffer[0] == 'T') {  // Test
        written = snprintf((char*)response, sizeof(response), "TEST:OK\r\n");
        CDC_Transmit_FS(response, written);
        BL_ClearBuffer();
    }
    else if (rx_buffer[0] == 'I') {  // Info
        uint8_t manufacturerID, deviceID;
        if (W25QXX_ReadID(&manufacturerID, &deviceID) == W25QXX_OK) {
            written = snprintf((char*)response, sizeof(response),
                              "INFO:W25Q64 Manufacturer:0x%02X Device:0x%02X\r\n",
                              manufacturerID, deviceID);
            CDC_Transmit_FS(response, written);
        }
        BL_ClearBuffer();
    }
    else if (rx_buffer[0] == 'H') {  // Help
        const char* help_text =
            "Commands:\r\n"
            "E<addr><size> - Erase flash (address and size in bytes)\r\n"
            "W<addr><size><data> - Write data\r\n"
            "V - Verify firmware CRC\r\n"
            "J - Jump to application\r\n"
            "T - Test communication\r\n"
            "I - Flash chip info\r\n"
            "S - Stay in bootloader mode\r\n"
            "C - Clear flags and reboot\r\n"
            "F - Force bootloader mode\r\n"
            "H - This help\r\n";
        CDC_Transmit_FS((uint8_t*)help_text, strlen(help_text));
        BL_ClearBuffer();
    }
    else if (rx_buffer[0] == 'S') {  // Stay in bootloader
        BL_SetStayInBootloader(1);
        written = snprintf((char*)response, sizeof(response), "STAY:1\r\n");
        CDC_Transmit_FS(response, written);
        BL_ClearBuffer();
    }
    else if (rx_buffer[0] == 'C') {  // Clear flags
        BL_ClearFlags();
        written = snprintf((char*)response, sizeof(response), "CLEAR:1\r\n");
        CDC_Transmit_FS(response, written);
        HAL_Delay(100);
        NVIC_SystemReset();
    }
    else if (rx_buffer[0] == 'F') {  // Force bootloader mode
        BL_SetStayInBootloader(1);
        BL_SetAppValid(0);
        written = snprintf((char*)response, sizeof(response), "FORCE:1\r\n");
        CDC_Transmit_FS(response, written);
        BL_ClearBuffer();
    }

    else {
        BL_ClearBuffer();
    }
}

void BL_Run(void)
{
    for (int i = 0; i < 3; i++) {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        HAL_Delay(200);
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        HAL_Delay(200);
    }

    uint32_t startTime = HAL_GetTick();
    while ((HAL_GetTick() - startTime) < 5000) {
        if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED) {
            break;
        }
        HAL_Delay(100);
    }

    BL_SetStayInBootloader(1);

    uint8_t welcome[256];
    int written = snprintf((char*)welcome, sizeof(welcome),
             "STM32H750 Bootloader Ready (QSPI@0x%08X)\r\n"
             "Button PC5 pressed: %s\r\n"
             "App valid: %s\r\n"
             "Type 'H' for help\r\n",
             APP_ADDRESS_QSPI,
             BL_CheckButton() ? "YES" : "NO",
             BL_IsAppValid() ? "YES" : "NO");

    if (written > 0 && written < sizeof(welcome)) {
        CDC_Transmit_FS(welcome, written);
    }

    while (1) {
        BL_ProcessCommand();
        HAL_Delay(1);

        static uint32_t led_timer = 0;
        if (HAL_GetTick() - led_timer > 500) {
            HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
            led_timer = HAL_GetTick();
        }
    }
}
