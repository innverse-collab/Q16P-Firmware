/*
 * STM32F103xB memory setup - NO bootloader.
 * Full 64K flash starting at 0x08000000 (real chip capacity, not relying on
 * the extra "hidden" flash some C8T6 clones have). Used for boards flashed
 * directly via SWD/ST-Link with no stm32duino (or any other) bootloader
 * present.
 */
MEMORY
{
    flash0 (rx) : org = 0x08000000, len = 64k
    flash1 (rx) : org = 0x00000000, len = 0
    flash2 (rx) : org = 0x00000000, len = 0
    flash3 (rx) : org = 0x00000000, len = 0
    flash4 (rx) : org = 0x00000000, len = 0
    flash5 (rx) : org = 0x00000000, len = 0
    flash6 (rx) : org = 0x00000000, len = 0
    flash7 (rx) : org = 0x00000000, len = 0
    ram0   (wx) : org = 0x20000000, len = 20k
    ram1   (wx) : org = 0x00000000, len = 0
    ram2   (wx) : org = 0x00000000, len = 0
    ram3   (wx) : org = 0x00000000, len = 0
    ram4   (wx) : org = 0x00000000, len = 0
    ram5   (wx) : org = 0x00000000, len = 0
    ram6   (wx) : org = 0x00000000, len = 0
    ram7   (wx) : org = 0x00000000, len = 0
}

/* For each data/text section two region are defined, a virtual region
   and a load region (_LMA suffix).*/

REGION_ALIAS("VECTORS_FLASH", flash0);
REGION_ALIAS("VECTORS_FLASH_LMA", flash0);

REGION_ALIAS("XTORS_FLASH", flash0);
REGION_ALIAS("XTORS_FLASH_LMA", flash0);

REGION_ALIAS("TEXT_FLASH", flash0);
REGION_ALIAS("TEXT_FLASH_LMA", flash0);

REGION_ALIAS("RODATA_FLASH", flash0);
REGION_ALIAS("RODATA_FLASH_LMA", flash0);

REGION_ALIAS("VARIOUS_FLASH", flash0);
REGION_ALIAS("VARIOUS_FLASH_LMA", flash0);

REGION_ALIAS("RAM_INIT_FLASH_LMA", flash0);

REGION_ALIAS("MAIN_STACK_RAM", ram0);
REGION_ALIAS("PROCESS_STACK_RAM", ram0);

REGION_ALIAS("DATA_RAM", ram0);
REGION_ALIAS("DATA_RAM_LMA", flash0);

REGION_ALIAS("BSS_RAM", ram0);
REGION_ALIAS("HEAP_RAM", ram0);

/* Generic rules inclusion.*/
INCLUDE rules.ld
