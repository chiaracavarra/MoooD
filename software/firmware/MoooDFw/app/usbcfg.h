/*

  Copyright (c) 2014 Guillaume Duc <guillaume@guiduc.org>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#ifndef _USB_HID_H_
#define _USB_HID_H_

#include "ch.h"
#include "hal.h"
#include "usb.h"

// Endpoints
#define IN_EP_NUM	1
#define OUT_EP_NUM	1

// Number of OUT reports that can be stored inside the input queue
#define USB_INPUT_QUEUE_CAPACITY 2
// Number of IN reports that can be stored inside the output queue
#define USB_OUTPUT_QUEUE_CAPACITY 2

// Size in bytes of the IN report (board->PC)
#define IN_REPORT_SIZE 64
// Size in bytes of the OUT report (PC->board)
#define OUT_REPORT_SIZE 64

#define USB_INPUT_QUEUE_BUFFER_SIZE (USB_INPUT_QUEUE_CAPACITY * OUT_REPORT_SIZE)
#define USB_OUTPUT_QUEUE_BUFFER_SIZE (USB_OUTPUT_QUEUE_CAPACITY * IN_REPORT_SIZE)

// Initialize the USB serial string from STM32 unique serial number
void init_usb_serial_string (void);

// Initialize the USB Input/Output queues
void init_usb_queues (void);

// Initialize the USB driver and bus
void init_usb_driver (void);

int usb_send_data(const uint8_t *buf, uint8_t len);

int usb_read_data(uint8_t *buf, uint8_t len, systime_t timeout);


#endif /* _USB_HID_H_ */
