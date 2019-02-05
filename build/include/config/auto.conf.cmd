deps_config := \
	/mnt/Share/share/ESP8266_RTOS_SDK/components/app_update/Kconfig \
	/mnt/Share/share/ESP8266_RTOS_SDK/components/aws_iot/Kconfig \
	/mnt/Share/share/ESP8266_RTOS_SDK/components/esp8266/Kconfig \
	/mnt/Share/share/ESP8266_RTOS_SDK/components/freertos/Kconfig \
	/mnt/Share/share/ESP8266_RTOS_SDK/components/libsodium/Kconfig \
	/mnt/Share/share/ESP8266_RTOS_SDK/components/log/Kconfig \
	/mnt/Share/share/ESP8266_RTOS_SDK/components/lwip/Kconfig \
	/mnt/Share/share/ESP8266_RTOS_SDK/components/mdns/Kconfig \
	/mnt/Share/share/ESP8266_RTOS_SDK/components/mqtt/Kconfig \
	/mnt/Share/share/ESP8266_RTOS_SDK/components/newlib/Kconfig \
	/mnt/Share/share/ESP8266_RTOS_SDK/components/pthread/Kconfig \
	/mnt/Share/share/ESP8266_RTOS_SDK/components/ssl/Kconfig \
	/mnt/Share/share/ESP8266_RTOS_SDK/components/tcpip_adapter/Kconfig \
	/mnt/Share/share/ESP8266_RTOS_SDK/components/wpa_supplicant/Kconfig \
	/mnt/Share/share/ESP8266_RTOS_SDK/components/bootloader/Kconfig.projbuild \
	/mnt/Share/share/ESP8266_RTOS_SDK/components/esptool_py/Kconfig.projbuild \
	/mnt/Share/share/ESP8266_RTOS_SDK/components/partition_table/Kconfig.projbuild \
	/mnt/Share/share/ESP8266_RTOS_SDK/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
