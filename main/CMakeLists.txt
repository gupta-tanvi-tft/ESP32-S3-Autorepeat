idf_component_register(SRCS "es8311_audio_repeater.c" "stt_client.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_driver_i2s esp_driver_i2c driver freertos log esp_system esp_psram heap esp_wifi nvs_flash esp_http_client json mbedtls esp_timer)
