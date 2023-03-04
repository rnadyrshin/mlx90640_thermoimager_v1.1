#include <esp_vfs_fat.h>
#include <driver/sdmmc_host.h>
#include <driver/sdspi_host.h>
#include "../console/console.h"
#include "sdmmc_cmd.h"
#include "sd.h"


#define USE_SPI_MODE

static sdmmc_card_t* card = 0;


int sd_CardInit()
{
	esp_vfs_fat_sdmmc_unmount();

#ifndef USE_SPI_MODE
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    host.slot = SDMMC_HOST_SLOT_1;

    //host.max_freq_khz = 1000;

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    // To use 1-line SD mode, uncomment the following line:
    slot_config.width = 1;

    // GPIOs 15, 2, 4, 12, 13 should have external 10k pull-ups.
    // Internal pull-ups are not sufficient. However, enabling internal pull-ups
    // does make a difference some boards, so we do that here.
    gpio_set_pull_mode(15, GPIO_PULLUP_ONLY);   // CMD, needed in 4- and 1- line modes
    gpio_set_pull_mode(2, GPIO_PULLUP_ONLY);    // D0, needed in 4- and 1-line modes
    gpio_set_pull_mode(13, GPIO_PULLUP_ONLY);   // D3, needed in 4- and 1-line modes
#else
	sdmmc_host_t host = SDSPI_HOST_DEFAULT();
	host.slot = VSPI_HOST; // HSPI_HOST
    host.max_freq_khz = 26000;

    sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
    slot_config.gpio_miso = 2;
    slot_config.gpio_mosi = 15;
    slot_config.gpio_sck  = 14;
    slot_config.gpio_cs   = 13;
    slot_config.dma_channel = 2;
#endif

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config =
    {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc_mount is an all-in-one convenience function.
    // Please check its source code and implement error recovery when developing production applications.
    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK)
    {
    	card = 0;
    	return ret;
    	/*
        if (ret == ESP_FAIL)
            console_printf(MsgError, "Failed to mount filesystem.\r\nIf you want the card to be formatted,\r\nset format_if_mount_failed = true.");
        else
            console_printf(MsgError, "Failed to initialize the card (%s).\r\nMake sure SD card lines have pull-up\r\nresistors in place.", esp_err_to_name(ret));
        return -1;
        */
    }

    // Card has been initialized, print its properties
	sdmmc_card_print_info(stdout, card);

    return 0;
}

int8_t sd_GetFree(uint32_t *pFreeMb, uint32_t *pTotalMb)
{
	if (!card)
		return -1;

	FATFS *fs;
	DWORD fre_clust, fre_sect, tot_sect;

	// Get volume information and free clusters of drive 0
	int err = f_getfree("0:", &fre_clust, &fs);
	if (err)
		return -1;

	// Get total sectors and free sectors
	tot_sect = (fs->n_fatent - 2) * fs->csize;
	fre_sect = fre_clust * fs->csize;

	// ”читываем размер сектора и пересчитываем в ћб
	tot_sect /= 1024;
	tot_sect *= card->csd.sector_size;
	tot_sect /= 1024;
	fre_sect /= 1024;
	fre_sect *= card->csd.sector_size;
	fre_sect /= 1024;

	*pFreeMb = fre_sect;
	*pTotalMb = tot_sect;

	return 0;
}
