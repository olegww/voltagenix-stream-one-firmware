#pragma once

// DMG10600C070_03WTC (DGUSII), UART2.
// The DWIN UART wiring follows the proven configuration of the original
// project: DWIN TX -> GPIO4, DWIN RX -> GPIO2.
#define DWIN_RX   4
#define DWIN_TX   2
#define DWIN_BAUD 115200

// Print DWIN frames and diagnostic hints on USB Serial while commissioning HMI.
#define DWIN_DEBUG 1
// Raw UART bytes — keep ON until play button shows RX frames, then set to 0.
#define DWIN_DEBUG_BYTES 1
// Accept play/prev/next from legacy src/ HMI (VP 0x6800).
#define DWIN_LEGACY_PLAYER_VP 1

// PCM5102 I2S DAC (ESP32-S3).
// Wiring: BCK=GPIO5, LRCK=GPIO6, DIN=GPIO7, SCK→GND (required on most modules).
#define I2S_BCLK 5
#define I2S_LRC  6
#define I2S_DOUT 7
