#include <Arduino.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/delay.h>
#include <string.h>

#include "usbdrv.h"
#include "blink1.h"

void setup() {
  // put your setup code here, to run once:
  cli();
  usbDeviceDisconnect();
  _delay_ms(250);
  usbDeviceConnect();
  pinMode(0, OUTPUT);
  usbInit();
  sei();
  initBlink();
}

void loop() {
  // put your main code here, to run repeatedly:
  usbPoll();
  millis_internal_val = millis();
  updateLEDs();
  digitalWrite(0, led_r > 128 || led_g > 128 || led_b > 128);
}
