#ifndef _BLINK1_H
#define _BLINK1_H

#ifdef __cplusplus
extern "C" {
#endif
extern volatile uint32_t millis_internal_val;
extern volatile uint8_t led_r;
extern volatile uint8_t led_g;
extern volatile uint8_t led_b;

void updateLEDs(void);
void initBlink(void);
#ifdef __cplusplus
}
#endif

#endif
