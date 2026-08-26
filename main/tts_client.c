#include "tts_client.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_heap_caps.h"
#include "esp_crt_bundle.h"
#include "mbedtls/base64.h"
#include "cJSON.h"
#include "secrets.h"

#define GEMINI_TTS_MODEL "models/gemini-2.5-flash-preview-tts"
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

    // Ignore placeholder markers or silence
    if (strstr(text, "[NO SPEECH]") != NULL || strstr(text, "[silence]") != NULL) {
        ESP_LOGI(TAG, "Skipping TTS for silent audio");
        return;
    }

    ESP_LOGI(TAG, "Building Gemini TTS payload for: \"%s\"...", text);

    cJSON *root = cJSON_CreateObject();
    cJSON *contents = cJSON_AddArrayToObject(root, "contents");
    cJSON *content_obj = cJSON_CreateObject();
    cJSON_AddStringToObject(content_obj, "role", "user");
    cJSON_AddItemToArray(contents, content_obj);

    char prompt[1024];
    snprintf(prompt, sizeof(prompt), "Repeat the following exact sentence in a warm, expressive, natural human voice. Do not answer questions or add commentary: %s", text);

    cJSON *parts = cJSON_AddArrayToObject(content_obj, "parts");
    cJSON *part_obj = cJSON_CreateObject();
    cJSON_AddStringToObject(part_obj, "text", prompt);
    cJSON_AddItemToArray(parts, part_obj);

    cJSON *gen_cfg = cJSON_AddObjectToObject(root, "generationConfig");
    cJSON *resp_mod = cJSON_AddArrayToObject(gen_cfg, "responseModalities");
    cJSON_AddItemToArray(resp_mod, cJSON_CreateString("AUDIO"));

    /* High-Fidelity Studio Human Voice Configuration */
    cJSON *speech_cfg = cJSON_AddObjectToObject(gen_cfg, "speechConfig");
    cJSON *voice_cfg = cJSON_AddObjectToObject(speech_cfg, "voiceConfig");
    cJSON *prebuilt_cfg = cJSON_AddObjectToObject(voice_cfg, "prebuiltVoiceConfig");
    cJSON_AddStringToObject(prebuilt_cfg, "voiceName", "Aoede"); // Premium natural human voice

    char *post_data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if (!post_data) {
        ESP_LOGE(TAG, "Failed to build TTS JSON payload");
        return;
    }

    char url[256];
    snprintf(url, sizeof(url), "https://generativelanguage.googleapis.com/v1beta/%s:generateContent?key=%s",
             GEMINI_TTS_MODEL, GEMINI_API_KEY);

    tts_response_buf_t resp = {
        .cap = 524288, // 512KB buffer in PSRAM for audio response
        .len = 0,
        .buffer = heap_caps_malloc(524288, MALLOC_CAP_SPIRAM),
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
        .buffer_size = 8192,
        .timeout_ms = 15000,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_http_client_set_header(client, "Content-Type", "application/json");

    ESP_LOGI(TAG, "Sending request to Gemini 2.5 Flash TTS API with Aoede Human Voice...");
    esp_err_t err = esp_http_client_perform(client);
    int status_code = esp_http_client_get_status_code(client);
    ESP_LOGI(TAG, "Gemini TTS HTTP Status = %d", status_code);

    if (err == ESP_OK && status_code == 200 && resp.len > 0) {
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
                        cJSON *inline_data = cJSON_GetObjectItem(part0, "inlineData");
                        if (inline_data) {
                            cJSON *data_item = cJSON_GetObjectItem(inline_data, "data");
                            if (data_item && data_item->valuestring) {
                                size_t b64_len = strlen(data_item->valuestring);
                                size_t pcm_cap = (b64_len * 3) / 4 + 1024;
                                uint8_t *pcm_out = heap_caps_malloc(pcm_cap, MALLOC_CAP_SPIRAM);
                                if (pcm_out) {
                                    size_t pcm_len = 0;
                                    mbedtls_base64_decode(pcm_out, pcm_cap, &pcm_len,
                                                          (const unsigned char *)data_item->valuestring, b64_len);

                                    ESP_LOGI(TAG, ">>> PLAYING NATIVE 24kHz STUDIO HD HUMAN VOICE (%d bytes PCM) <<<", pcm_len);

                                    /* Reconfigure I2S TX clock dynamically to native 24,000 Hz for full HD playback */
                                    i2s_std_clk_config_t clk_24k = {
                                        .sample_rate_hz = 24000,
                                        .clk_src = I2S_CLK_SRC_DEFAULT,
                                        .mclk_multiple = I2S_MCLK_MULTIPLE_256,
                                    };
                                    i2s_channel_disable(tx_chan);
                                    i2s_channel_reconfig_std_clock(tx_chan, &clk_24k);
                                    i2s_channel_enable(tx_chan);

                                    /* Convert Mono 24kHz PCM to Stereo 24kHz PCM for I2S output */
                                    size_t samples = pcm_len / 2;
                                    int16_t *mono_pcm = (int16_t *)pcm_out;
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

                                    /* Restore I2S TX clock back to 16,000 Hz */
                                    i2s_std_clk_config_t clk_16k = {
                                        .sample_rate_hz = 16000,
                                        .clk_src = I2S_CLK_SRC_DEFAULT,
                                        .mclk_multiple = I2S_MCLK_MULTIPLE_256,
                                    };
                                    i2s_channel_disable(tx_chan);
                                    i2s_channel_reconfig_std_clock(tx_chan, &clk_16k);
                                    i2s_channel_enable(tx_chan);

                                    heap_caps_free(pcm_out);
                                }
                            }
                        }
                    }
                }
            }
            cJSON_Delete(resp_json);
        }
    } else {
        if (resp.buffer && resp.len > 0) {
            ESP_LOGW(TAG, "Gemini TTS Error Body: %s", resp.buffer);
        }
        ESP_LOGE(TAG, "Gemini TTS request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    heap_caps_free(resp.buffer);
    free(post_data);
}
