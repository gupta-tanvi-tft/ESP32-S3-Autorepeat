#pragma once

#include <stdint.h>
#include <stddef.h>

void wifi_init_sta(void);
void stt_transcribe_audio(uint8_t *pcm_data, size_t pcm_len);
