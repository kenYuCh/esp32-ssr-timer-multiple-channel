# SPIFFS system for Index.html
# Burn all files in the folder(web) to *.bin
1. 
python spiffsgen.py 524288 spiffs spiffs.bin
# python spiffsgen.py 262144 spiffs spiffs.bin

0x620000
# ESP32 build , flash, monitor
1. build
2. flash -> 
python -m esptool --chip esp32 -b 460800 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_size 2MB --flash_freq 40m 0x1000 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x10000 build/main.bin 0x110000 spiffs.bin
python -m esptool --chip esp32 -b 460800 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_size 4MB --flash_freq 40m 0x320000 spiffs.bin
python -m esptool --chip esp32s3 -b 460800 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_size 8MB --flash_freq 40m 0x620000 spiffs.bin
3. idf.py build flash monitor

# set Flash Size 
1. idf.py menuconfig 
2. Serial Flash Configuration : 4MB
 要記得修改 flash 4MB 的燒錄命令


# http_ota_client : enable https Allow http



