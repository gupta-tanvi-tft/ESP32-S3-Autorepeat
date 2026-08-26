#pragma once

#include <stdint.h>
#include <stddef.h>
#include "driver/i2s_std.h"

/**
 * @brief Synthesizes text into speech using Google Cloud TTS and plays it through I2S
 * 
 * @param text The text string to speak
 * @param tx_chan The I2S TX channel handle connected to the ES8311 DAC
 */
void tts_synthesize_and_play(const char *text, i2s_chan_handle_t tx_chan);
