#pragma once

#include <stdint.h>
#include <stddef.h>

typedef void (*stt_result_cb_t)(const char *transcribed_text);

void wifi_init_sta(void);
void stt_transcribe_audio(uint8_t *pcm_data, size_t pcm_len, stt_result_cb_t callback);
