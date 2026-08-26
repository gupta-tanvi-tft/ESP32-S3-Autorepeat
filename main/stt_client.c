#include "stt_client.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_heap_caps.h"
#include "mbedtls/base64.h"
#include "cJSON.h"
#include "secrets.h"

// TODO: User must fill these in
#define WIFI_SSID "TFTus-WiFi"
#define WIFI_PASS "TFTus@123#$"
#define GEMINI_MODEL "models/gemini-2.5-flash"

#define SAMPLE_RATE 16000
#define BITS_PER_SAMPLE 16
#define CHANNELS 2

static const char *TAG = "STT_CLIENT";
static EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;

/* Response accumulation buffer */
typedef struct {
    char *buffer;
    size_t len;
    size_t cap;
} response_buf_t;

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Retrying Wi-Fi connection...");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        ESP_LOGI(TAG, "Got IP address!");
    }
}

void wifi_init_sta(void) {
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Wi-Fi init finished. Waiting for connection...");
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
}

static void add_wav_header(uint8_t *header, uint32_t pcm_data_len, uint16_t num_channels) {
    uint32_t sample_rate = SAMPLE_RATE;
    uint16_t bits_per_sample = BITS_PER_SAMPLE;
    uint32_t byte_rate = sample_rate * num_channels * (bits_per_sample / 8);
    
    header[0] = 'R'; header[1] = 'I'; header[2] = 'F'; header[3] = 'F';
    uint32_t chunk_size = pcm_data_len + 36;
    memcpy(header + 4, &chunk_size, 4);
    header[8] = 'W'; header[9] = 'A'; header[10] = 'V'; header[11] = 'E';
    
    header[12] = 'f'; header[13] = 'm'; header[14] = 't'; header[15] = ' ';
    uint32_t subchunk1_size = 16;
    memcpy(header + 16, &subchunk1_size, 4);
    uint16_t audio_format = 1; // PCM
    memcpy(header + 20, &audio_format, 2);
    memcpy(header + 22, &num_channels, 2);
    memcpy(header + 24, &sample_rate, 4);
    memcpy(header + 28, &byte_rate, 4);
    uint16_t block_align = num_channels * (bits_per_sample / 8);
    memcpy(header + 32, &block_align, 2);
    memcpy(header + 34, &bits_per_sample, 2);
    
    header[36] = 'd'; header[37] = 'a'; header[38] = 't'; header[39] = 'a';
    memcpy(header + 40, &pcm_data_len, 4);
}

esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    response_buf_t *resp = (response_buf_t *)evt->user_data;
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            if (resp && resp->buffer && evt->data_len > 0) {
                if (resp->len + evt->data_len < resp->cap - 1) {
                    memcpy(resp->buffer + resp->len, evt->data, evt->data_len);
                    resp->len += evt->data_len;
                    resp->buffer[resp->len] = '\0';
                }
            }
            break;
        default:
            break;
    }
    return ESP_OK;
}

void stt_transcribe_audio(uint8_t *pcm_data, size_t pcm_len, stt_result_cb_t callback) {
    ESP_LOGI(TAG, "Preparing audio for Gemini API...");
    size_t wav_size = 44 + pcm_len;
    uint8_t *wav_buf = heap_caps_malloc(wav_size, MALLOC_CAP_SPIRAM);
    if (!wav_buf) {
        ESP_LOGE(TAG, "Failed to allocate memory for WAV buffer");
        return;
    }
    add_wav_header(wav_buf, pcm_len, CHANNELS);
    memcpy(wav_buf + 44, pcm_data, pcm_len);

    size_t b64_len = 0;
    mbedtls_base64_encode(NULL, 0, &b64_len, wav_buf, wav_size);
    char *b64_str = heap_caps_malloc(b64_len + 1, MALLOC_CAP_SPIRAM);
    if (!b64_str) {
        ESP_LOGE(TAG, "Failed to allocate memory for Base64");
        heap_caps_free(wav_buf);
        return;
    }
    size_t dlen = 0;
    mbedtls_base64_encode((unsigned char *)b64_str, b64_len + 1, &dlen, wav_buf, wav_size);
    b64_str[dlen] = '\0'; // Ensure valid null-terminated string for cJSON
    heap_caps_free(wav_buf);

    ESP_LOGI(TAG, "Building JSON payload...");
    cJSON *root = cJSON_CreateObject();
    cJSON *contents = cJSON_AddArrayToObject(root, "contents");
    cJSON *content_obj = cJSON_CreateObject();
    cJSON_AddItemToArray(contents, content_obj);
    
    cJSON *parts = cJSON_AddArrayToObject(content_obj, "parts");
    cJSON *text_part = cJSON_CreateObject();
    cJSON_AddStringToObject(text_part, "text", "You are an exact audio repeater. Transcribe the exact words spoken in this audio verbatim. Do not answer questions, do not add preamble, and do not paraphrase. Return ONLY the exact words spoken. If no clear speech is heard, return [NO SPEECH].");
    cJSON_AddItemToArray(parts, text_part);
    
    cJSON *inline_part = cJSON_CreateObject();
    cJSON *inline_data = cJSON_AddObjectToObject(inline_part, "inlineData");
    cJSON_AddStringToObject(inline_data, "mimeType", "audio/wav");
    cJSON_AddItemToObject(inline_data, "data", cJSON_CreateStringReference(b64_str));
    cJSON_AddItemToArray(parts, inline_part);

    /* Zero temperature for exact deterministic transcription */
    cJSON *gen_cfg = cJSON_AddObjectToObject(root, "generationConfig");
    cJSON_AddNumberToObject(gen_cfg, "temperature", 0.0);
    
    char *post_data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    heap_caps_free(b64_str);

    if (!post_data) {
        ESP_LOGE(TAG, "Failed to serialize JSON");
        return;
    }

    ESP_LOGI(TAG, "Sending request to Gemini API... (Payload size: %d bytes)", strlen(post_data));
    
    char url[256];
    snprintf(url, sizeof(url), "https://generativelanguage.googleapis.com/v1beta/%s:generateContent?key=%s", GEMINI_MODEL, GEMINI_API_KEY);
    
    response_buf_t resp = {
        .cap = 16384,
        .len = 0,
        .buffer = heap_caps_malloc(16384, MALLOC_CAP_SPIRAM),
    };
    if (resp.buffer) {
        resp.buffer[0] = '\0';
    }

    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .event_handler = http_event_handler,
        .user_data = &resp,
        .buffer_size = 4096,
        .timeout_ms = 30000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    
    esp_err_t err = esp_http_client_perform(client);
    int status_code = esp_http_client_get_status_code(client);
    ESP_LOGI(TAG, "HTTP Status = %d", status_code);

    if (err == ESP_OK && status_code == 200 && resp.buffer && resp.len > 0) {
        cJSON *resp_json = cJSON_Parse(resp.buffer);
        if (resp_json) {
            cJSON *candidates = cJSON_GetObjectItem(resp_json, "candidates");
            if (candidates && cJSON_GetArraySize(candidates) > 0) {
                cJSON *cand0 = cJSON_GetArrayItem(candidates, 0);
                cJSON *content = cJSON_GetObjectItem(cand0, "content");
                if (content) {
                    cJSON *parts_arr = cJSON_GetObjectItem(content, "parts");
                    if (parts_arr && cJSON_GetArraySize(parts_arr) > 0) {
                        cJSON *part0 = cJSON_GetArrayItem(parts_arr, 0);
                        cJSON *text_item = cJSON_GetObjectItem(part0, "text");
                        if (text_item && text_item->valuestring) {
                            ESP_LOGI(TAG, "==========================================");
                            ESP_LOGI(TAG, ">>> TRANSCRIBED SPEECH: \"%s\"", text_item->valuestring);
                            ESP_LOGI(TAG, "==========================================");
                            if (callback) {
                                callback(text_item->valuestring);
                            }
                        }
                    }
                }
            }
            cJSON_Delete(resp_json);
        }
    } else {
        if (resp.buffer && resp.len > 0) {
            ESP_LOGW(TAG, "Error Response Body: %s", resp.buffer);
        }
        ESP_LOGE(TAG, "HTTP request failed or non-200 status: %s", esp_err_to_name(err));
    }
    
    if (resp.buffer) {
        heap_caps_free(resp.buffer);
    }
    esp_http_client_cleanup(client);
    free(post_data);
}
