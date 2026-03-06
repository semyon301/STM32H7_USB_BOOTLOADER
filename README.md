HOW TO USE?

Clone the repository, open the project in STM32CubeIDE, and compile the bootloader. Then flash the bootloader.elf file to the MCUDev DevEBox STM32H7XX_M debug board based on the STM32H750VBT6 MCU (link to the Github repository with all the information about this debug board: https://github.com/mcauser/MCUDEV_DEVEBOX_H7XX_M.git). For example, you can use your ST-Link, or you can use STM32CubeProgrammer and turn your debug board into DFU mode. Clone my repository with my program for uploading firmware to an external QSPI flash memory via USB in virtual COM port mode (link to this repository with my program: https://github.com/semyon301/STM32-COM-port-QSPI-flasher.git). Then compile in Visual Studio 2026 my program for uploading firmware to an external QSPI flash memory via USB in virtual COM port mode. Connect the debug board to your computer via USB; you should see a new device in the COM ports section of the Device Manager. Run my program, select the COM port, and click Connect. Wait until the connection to the debug board and bootloader testing are complete (you'll see corresponding messages in the program logs, as well as a message with information about the QSPI flash memory in use). Click the Browse button to open the file explorer and select the firmware file with the .bin extension to be loaded. Click the Flash button, and the firmware will begin loading into the QSPI flash memory. After that click the Disconnect button (it replaces th Connect button after connection to bootloader and vice versa). The firmware upload program also includes buttons such as Refresh (updates the list of available COM ports), Test (T) (tests the connection with the bootloader), Info (I) (displays information about the used qspi flash memory), Help (H) (displays help information from the bootloader), Jump (J) (allows you to jump to the execution of the firmware code in the qspi flash memory, necessary if the Autojump after checkbox is not checked). There are also auxiliary buttons that allow you to clear the logs or save them (these are the Clear Log and Save Log buttons). 
You can also use ready files that you can find in releases page, such as "bootloader.elf", "check.bin", or an installer of my program for flashing in appropriate repository (link to it is above). File "check.bin" is a test program that blinks LED connected to PA1 GPIO port. Also you can use my linker-script to build your own firmware for flashing it into external QSPI flash memory, that you can find in releases page.

ATTENTION!

Since this is the first version of my bootloader and firmware uploader, bugs are possible. For example, I've noticed that sometimes the firmware upload fails, apparently due to a bootloader connection check error before the firmware upload begins. However, all the conditions for starting the firmware upload are met (the debug board is connected, the correct COM port and firmware file are selected). In this case, you should repeat the firmware upload preparation procedure from the beginning. Don't worry, my project will be improved and modified in the future, so there will be more new features and fewer bugs.

AND ONE LAST THING THAT I NOT MENTIONED

You should add these strings into your code of your project that you want to create for flashing and executing from external QSPI flash memory:
1.	Between two strings of code named as "USER CODE BEGIN PV" and "USER CODE END PV":

	extern uint32_t _sidata, _sdata, _edata;
	extern uint32_t _sbss, _ebss;
	extern uint32_t _estack;


2.	Between "USER CODE BEGIN PFP" and "USER CODE END PFP":

	static void CopyDataToRAM(void);
	static void ClearBSS(void);
	void Error_Handler(void);


3.	In "int main(void)" between "USER CODE BEGIN 1" and "USER CODE END 1":

	SCB->VTOR = 0x90000000;
    	CopyDataToRAM();
    	ClearBSS();


4.	Optionally in "int main(void)" between "USER CODE BEGIN 2" and "USER CODE END 2":

	SCB_EnableICache();
	SCB_EnableDCache();


5.	After end of "int main(void)" and before "void SystemClock_Config(void)":

	void CopyDataToRAM(void)
	{
    		uint32_t *src = &_sidata;
    		uint32_t *dst = &_sdata;

    		while (dst < &_edata)
    		{
        		*dst++ = *src++;
    		}
	}

	void ClearBSS(void)
	{
    		uint32_t *dst = &_sbss;

    		while (dst < &_ebss)
    		{
        		*dst++ = 0;
    		}
	}
