#include <Arduino.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/delay.h>
#include <string.h>

#include "usbdrv.h"

typedef uint8_t byte;

#define BUFFER_SIZE 64

static uchar idleRate; // in 4 ms units
static uchar light; // light status
static uchar inputReportReady;
static uchar inputReportID;
static uchar inputReportBuffer[BUFFER_SIZE];
static uchar inputReportLen;
static uchar inputReportCurrentLen;
static uchar outputReportBuffer[BUFFER_SIZE];

const PROGMEM char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = { /* USB report descriptor */
  0x06, 0x00, 0xff,              // USAGE_PAGE (Vendor Defined Page 1)
  0x09, 0x01,                    // USAGE (Vendor Usage 1)
  0xa1, 0x01,                    // COLLECTION (Application)
  0x85, 0x01,                    //   REPORT_ID (1), light control
  0x75, 0x08,                    //   REPORT_SIZE (8)
  0x95, 0x01,                    //   REPORT_COUNT (1)
  0x15, 0x00,                    //   LOGICAL_MINIMUM (0), off
  0x25, 0xff,                    //   LOGICAL_MAXIMUM (255), on
  0x09, 0x01,                    //   USAGE (Vendor Usage 1)
  0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
  0xc0                           // END_COLLECTION
};

void setup() {
  // put your setup code here, to run once:
  cli();
  usbDeviceDisconnect();
  _delay_ms(250);
  usbDeviceConnect();
  light = 0;
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  usbInit();
  sei();

  // TODO: Remove the next two lines once we fix
  //       missing first keystroke bug properly.
  memset(inputReportBuffer, 0, sizeof(inputReportBuffer));
  memset(outputReportBuffer, 0, sizeof(outputReportBuffer));
  // usbSetInterrupt(inputReportBuffer, sizeof(inputReportBuffer));
}

void loop() {
  // put your main code here, to run repeatedly:
  usbPoll();
  handleInputReport();
  analogWrite(0, light);
  analogWrite(1, light);
}

void handleInputReport() {
  if (inputReportReady) {
    if (inputReportID == 1) { // set light.
      if (inputReportLen > 0) {
        light = inputReportBuffer[inputReportLen-1];
      }
    }
    inputReportReady = 0;
  }
}

#ifdef __cplusplus
extern "C"{
#endif
  // USB_PUBLIC uchar usbFunctionSetup
  uchar usbFunctionSetup(uchar data[8]) {
    usbRequest_t    *rq = (usbRequest_t *)((void *)data);

    usbMsgPtr = outputReportBuffer;
    if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
      /* class request type */

      if (rq->bRequest == USBRQ_HID_SET_REPORT) {
        /* wValue: ReportType (highbyte), ReportID (lowbyte) */
        /* we only have one report type, so don't look at wValue */
        // TODO: Ensure it's okay not to return anything here?
        // note: the hidapi library prepends 1 byte.
        inputReportID = rq->wValue.bytes[0];
        inputReportCurrentLen = 0;
        inputReportReady = 0;
        inputReportLen = rq->wLength.word;
        return USB_NO_MSG;
      } else if (rq->bRequest == USBRQ_HID_GET_IDLE) {
        usbMsgPtr[0] = idleRate;
        return 1;
      } else if (rq->bRequest == USBRQ_HID_SET_IDLE) {
        idleRate = rq->wValue.bytes[1];
      }
    }
    return 0;
  }

  uchar usbFunctionWrite(uchar *data, uchar len) {
    for (uchar i = 0; i < len; i++) {
      inputReportBuffer[inputReportCurrentLen] = data[i];
      inputReportCurrentLen++;
      if (inputReportCurrentLen >= BUFFER_SIZE) {
        return 1;
      }
    }
    if (inputReportCurrentLen == inputReportLen) {
      inputReportReady = 1;
      return 1;
    } else {
      return 0;
    }
  }
#ifdef __cplusplus
} // extern "C"
#endif
