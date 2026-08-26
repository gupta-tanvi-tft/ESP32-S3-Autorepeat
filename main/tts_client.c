#include "tts_client.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_heap_caps.h"
#include "mbedtls/base64.h"
#include "cJSON.h"
#include "secrets.h"

static const char *TAG = "TTS_CLIENT";

typedef struct {
    char *buffer;
    size_t len;
    size_t cap;
} tts_response_buf_t;

static esp_err_t tts_http_event_handler(esp_http_client_event_t *evt) {
    tts_response_buf_t *resp = (tts_response_buf_t *)evt->user_data;
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

void tts_synthesize_and_play(const char *text, i2s_chan_handle_t tx_chan) {
    if (!text || strlen(text) == 0 || !tx_chan) {
        ESP_LOGW(TAG, "Empty text or null I2S channel for TTS");
        return;
    }

    ESP_LOGI(TAG, "Building TTS payload for text: \"%s\"...", text);

    cJSON *root = cJSON_CreateObject();
    cJSON *input = cJSON_AddObjectToObject(root, "input");
    cJSON_AddStringToObject(input, "text", text);

    cJSON *voice = cJSON_AddObjectToObject(root, "voice");
    cJSON_AddStringToObject(voice, "languageCode", "en-US");
    cJSON_AddStringToObject(voice, "name", "en-US-Neural2-F");

    cJSON *audio_config = cJSON_AddObjectToObject(root, "audioConfig");
    cJSON_AddStringToObject(audio_config, "audioEncoding", "LINEAR16");
    cJSON_AddNumberToObject(audio_config, "sampleRateHertz", 16000);

    char *post_data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if (!post_data) {
        ESP_LOGE(TAG, "Failed to build TTS JSON payload");
        return;
    }

    char url[256];
    snprintf(url, sizeof(url), "https://texttospeech.googleapis.com/v1/text:synthesize?key=%s", GEMINI_API_KEY);

    tts_response_buf_t resp = {
        .cap = 131072, // 128KB buffer in PSRAM
        .len = 0,
        .buffer = heap_caps_malloc(131072, MALLOC_CAP_SPIRAM),
    };
    if (!resp.buffer) {
        ESP_LOGE(TAG, "Failed to allocate PSRAM for TTS response");
        free(post_data);
        return;
    }
    resp.buffer[0] = '\0';

    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .event_handler = tts_http_event_handler,
        .user_data = &resp,
        .buffer_size = 4096,
        .timeout_ms = 30000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    ESP_LOGI(TAG, "Sending request to Google TTS API...");
    esp_err_t err = esp_http_client_perform(client);
    int status_code = esp_http_client_get_status_code(client);
    ESP_LOGI(TAG, "TTS HTTP Status = %d", status_code);

    if (err == ESP_OK && status_code == 200 && resp.len > 0) {
        cJSON *resp_json = cJSON_Parse(resp.buffer);
        if (resp_json) {
            cJSON *audio_content = cJSON_GetObjectItem(resp_json, "audioContent");
            if (audio_content && audio_content->valuestring) {
                size_t b64_len = strlen(audio_content->valuestring);
                size_t pcm_cap = (b64_len * 3) / 4 + 1024;
                uint8_t *pcm_out = heap_caps_malloc(pcm_cap, MALLOC_CAP_SPIRAM);
                if (pcm_out) {
                    size_t pcm_len = 0;
                    mbedtls_base64_decode(pcm_out, pcm_cap, &pcm_len,
                                          (const unsigned char *)audio_content->valuestring, b64_len);

                    ESP_LOGI(TAG, "Decoded %d bytes of PCM audio. Playing back through speaker...", pcm_len);

                    /* Skip 44-byte WAV header if present (RIFF header) */
                    size_t offset = 0;
                    if (pcm_len > 44 && pcm_out[0] == 'R' && pcm_out[1] == 'I' && pcm_out[2] == 'F' && pcm_out[3] == 'F') {
                        offset = 44;
                    }

                    /* Convert Mono 16-bit to Stereo 16-bit for I2S output */
                    size_t samples = (pcm_len - offset) / 2;
                    int16_t *mono_pcm = (int16_t *)(pcm_out + offset);
                    int16_t *stereo_buf = heap_caps_malloc(samples * 4, MALLOC_CAP_SPIRAM);
                    if (stereo_buf) {
                        for (size_t i = 0; i < samples; i++) {
                            stereo_buf[i * 2]     = mono_pcm[i]; // Left
                            stereo_buf[i * 2 + 1] = mono_pcm[i]; // Right
                        }

                        size_t total_written = 0;
                        size_t total_to_write = samples * 4;
                        const size_t chunk_size = 1024;

                        while (total_written < total_to_write) {
                            size_t to_write = chunk_size;
                            if (total_written + to_write > total_to_write) {
                                to_write = total_to_write - total_written;
                            }
                            size_t written = 0;
                            i2s_channel_write(tx_chan, ((uint8_t *)stereo_buf) + total_written,
                                              to_write, &written, pdMS_TO_TICKS(1000));
                            total_written += written;
                        }
                        heap_caps_free(stereo_buf);
                    }
                    heap_caps_free(pcm_out);
                }
            }
            cJSON_Delete(resp_json);
        }
    } else {
        if (resp.buffer && resp.len > 0) {
            ESP_LOGW(TAG, "TTS Error Response Body: %s", resp.buffer);
        }
        ESP_LOGE(TAG, "TTS HTTP request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    heap_caps_free(resp.buffer);
    free(post_data);
}
