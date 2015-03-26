/*
 *
 */

#include "ch.h"
#include "hal.h"

#include "leds.h"
#include "usbcfg.h"

// HID specific constants
#define USB_DESCRIPTOR_HID 0x21
#define USB_DESCRIPTOR_HID_REPORT 0x22
#define HID_GET_REPORT 0x01
#define HID_SET_REPORT 0x09

// #define USE_INTR

extern USBDriver USBD1;

static InputQueue usb_input_queue;
static uint8_t input_queue_buffer[USB_INPUT_QUEUE_BUFFER_SIZE];


// IN EP1 state
static USBInEndpointState ep1instate;
// OUT EP1 state
static USBOutEndpointState ep1outstate;

// USB Device Descriptor
static const uint8_t usb_device_descriptor_data[] = {
  USB_DESC_DEVICE (0x0110,	// bcdUSB (1.1)
		   0x00,	// bDeviceClass (defined in later in interface)
		   0x00,	// bDeviceSubClass
		   0x00,	// bDeviceProtocol
		   0x40,	// bMaxPacketSize (64 bytes)
		   0x0483,	// idVendor (ST)
		   0x5740,	// idProduct (STM32)
		   0x0000,	// bcdDevice
		   1,		// iManufacturer
		   2,		// iProduct
		   3,		// iSerialNumber
		   1)		// bNumConfigurations
};

// Device Descriptor wrapper
static const USBDescriptor usb_device_descriptor = {
	sizeof usb_device_descriptor_data,
	usb_device_descriptor_data
};

/*
 * Configuration Descriptor tree for a HID device
 *
 * The HID Specifications version 1.11 require the following order:
 * - Configuration Descriptor
 * - Interface Descriptor
 * - HID Descriptor
 * - Endpoints Descriptors
 */
#define HID_DESCRIPTOR_OFFSET 18
#define HID_DESCRIPTOR_SIZE 9


// FIXME: bulk ep, dual=storage+generic(non hid)
static const uint8_t hid_configuration_descriptor_data[] = {
  // Configuration Descriptor (9 bytes)
  USB_DESC_CONFIGURATION (32,	// wTotalLength (9+9+9+7+7)
			  0x01,	// bNumInterfaces
			  0x01,	// bConfigurationValue
			  0,	// iConfiguration
			  0xC0,	// bmAttributes (self powered, set to 0x80 if not)
			  50),	// bMaxPower (100mA)

  // Interface Descriptor (9 bytes)
  USB_DESC_INTERFACE (0x00,	// bInterfaceNumber
		      0x00,	// bAlternateSetting
		      0x02,	// bNumEndpoints
//		      0x03,	// bInterfaceClass: HID
		      0xff,          /* bInterfaceClass: Vendor Specific */
		      0x00,	// bInterfaceSubClass: None
		      0x00,	// bInterfaceProtocol: None
		      4),	// iInterface

//  // HID descriptor (9 bytes)
//  USB_DESC_BYTE (9),		// bLength
//  USB_DESC_BYTE (0x21),		// bDescriptorType (HID class)
//  USB_DESC_BCD (0x0110),	// bcdHID: HID version 1.1
//  USB_DESC_BYTE (0x00),		// bCountryCode
//  USB_DESC_BYTE (0x01),		// bNumDescriptors
//  USB_DESC_BYTE (0x22),		// bDescriptorType (report desc)
//  USB_DESC_WORD (34),		// wDescriptorLength

  // Endpoint 1 IN Descriptor (7 bytes)
  USB_DESC_ENDPOINT (IN_EP_NUM | 0x80,	// bEndpointAddress
#ifdef USE_INTR
		     0x03,	// bmAttributes (Interrupt)
#else
		     0x02,	// bmAttributes (Bulk)
#endif
		     0x0040,	// wMaxPacketSize
		     0x0A),	// bInterval (10 ms)
  // Endpoint 1 OUT Descriptor (7 bytes)
  USB_DESC_ENDPOINT (OUT_EP_NUM,	// bEndpointAddress
#ifdef USE_INTR
		     0x03,	// bmAttributes (Interrupt)
#else
		     0x02,	// bmAttributes (Bulk)
#endif
		     0x0040,	// wMaxPacketSize
		     0x0A)	// bInterval (10 ms)
};

// Configuration Descriptor wrapper
static const USBDescriptor hid_configuration_descriptor = {
	sizeof hid_configuration_descriptor_data,
	hid_configuration_descriptor_data
};

// HID descriptor wrapper
static const USBDescriptor hid_descriptor = {
	HID_DESCRIPTOR_SIZE,
	&hid_configuration_descriptor_data[HID_DESCRIPTOR_OFFSET]
};

/*
 * HID Report Descriptor
 *
 * This is the description of the format and the content of the
 * different IN or/and OUT reports that your application can
 * receive/send
 *
 * See "Device Class Definition for Human Interface Devices (HID)"
 * (http://www.usb.org/developers/hidpage/HID1_11.pdf) for the
 * detailed descrition of all the fields
 */
static const uint8_t hid_report_descriptor_data[] = {
  USB_DESC_BYTE (0x06),		// Usage Page (vendor-defined)
  USB_DESC_WORD (0xFF00),
  USB_DESC_BYTE (0x00),		// Usage (vendor-defined)
  USB_DESC_BYTE (0x01),
  USB_DESC_BYTE (0xA1),		// Collection (application)
  USB_DESC_BYTE (0x01),

  USB_DESC_BYTE (0x09),		// Usage (vendor-defined)
  USB_DESC_BYTE (0x01),
  USB_DESC_BYTE (0x15),		// Logical minimum (0)
  USB_DESC_BYTE (0x00),
  USB_DESC_BYTE (0x26),		// Logical maximum (255)
  USB_DESC_WORD (0x00FF),
  USB_DESC_BYTE (0x75),		// Report size (8 bits)
  USB_DESC_BYTE (0x08),
  USB_DESC_BYTE (0x95),		// Report count (USB_HID_IN_REPORT_SIZE)
  USB_DESC_BYTE (IN_REPORT_SIZE),
  USB_DESC_BYTE (0x81),		// Input (Data, Variable, Absolute)
  USB_DESC_BYTE (0x02),

  USB_DESC_BYTE (0x09),		// Usage (vendor-defined)
  USB_DESC_BYTE (0x01),
  USB_DESC_BYTE (0x15),		// Logical minimum (0)
  USB_DESC_BYTE (0x00),
  USB_DESC_BYTE (0x26),		// Logical maximum (255)
  USB_DESC_WORD (0x00FF),
  USB_DESC_BYTE (0x75),		// Report size (8 bits)
  USB_DESC_BYTE (0x08),
  USB_DESC_BYTE (0x95),		// Report count (USB_HID_OUT_REPORT_SIZE)
  USB_DESC_BYTE (OUT_REPORT_SIZE),
  USB_DESC_BYTE (0x91),		// Output (Data, Variable, Absolute)
  USB_DESC_BYTE (0x02),

  USB_DESC_BYTE (0xC0)		// End collection
};

// HID report descriptor wrapper
static const USBDescriptor hid_report_descriptor = {
  sizeof hid_report_descriptor_data,
  hid_report_descriptor_data
};

// U.S. English language identifier
static const uint8_t usb_string_langid[] = {
  USB_DESC_BYTE (4),		// bLength
  USB_DESC_BYTE (USB_DESCRIPTOR_STRING),	// bDescriptorType
  USB_DESC_WORD (0x0409)	// wLANGID (U.S. English)
};

// Vendor string
static const uint8_t usb_string_vendor[] = {
	  USB_DESC_BYTE(22),                    /* bLength.                         */
	  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
	  'A', 0, 'b', 0, 's', 0, 'e', 0, 'n', 0, 'c', 0, 'e', 0, '.', 0,
	  'i', 0, 't', 0
};

// Device Description string
static const uint8_t usb_string_description[] = {
	USB_DESC_BYTE(12), /* bLength. */
	USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType. */
	'M', 0, 'o', 0, 'o', 0, 'o', 0, 'D', 0
};

// Serial Number string (will be filled by the function init_usb_serial_string)
static uint8_t usb_string_serial[] = {
	USB_DESC_BYTE(8),                     /* bLength.                         */
	USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
	'1' + CH_KERNEL_MAJOR, 0,
	'0' + CH_KERNEL_MINOR, 0,
	'0' + CH_KERNEL_PATCH, 0
};

/*
* Interface 1 string.
*/
static const uint8_t usb_interface_1_string[] = {
	USB_DESC_BYTE(12),                    /* bLength.         */
	USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType. */
	'S', 0, 't', 0, 'o', 0, 'r', 0, 'a', 0, 'g', 0, 'e', 0
};

/*
* Interface 2 string.
*/
static const uint8_t usb_interface_2_string[] = {
	USB_DESC_BYTE(12),                    /* bLength.         */
	USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType. */
	'R', 0, 'G', 0, 'B', 0, ' ', 0, 'L', 0, 'E', 0, 'D', 0
};

// Strings wrappers array
static const USBDescriptor usb_strings[] = {
	{sizeof usb_string_langid, usb_string_langid} ,
	{sizeof usb_string_vendor, usb_string_vendor} ,
	{sizeof usb_string_description, usb_string_description} ,
	{sizeof usb_string_serial, usb_string_serial},
	{sizeof usb_interface_1_string, usb_interface_1_string}
//	,{sizeof usb_interface_2_string, usb_interface_2_string}
};


int usb_read_data(uint8_t *buf, uint8_t len, systime_t timeout)
{
  return chIQReadTimeout(&usb_input_queue, buf, len, timeout);
}


int usb_send_data(const uint8_t *buf, uint8_t len)
{
  chSysLock ();
  if (usbGetDriverStateI(&USBD1) != USB_ACTIVE)
  {
    chSysUnlock ();
    return 0;
  }

  usbPrepareTransmit(&USBD1, IN_EP_NUM, buf, len);
  bool_t failed = usbStartTransmitI(&USBD1, IN_EP_NUM);
  chSysUnlock();

  if (failed)
    return -1;

  return 0;
}


/*
 * EP1 IN callback handler
 *
 * Data (IN report) have just been sent to the PC. Check if there are
 * remaining reports to be sent in the output queue and in this case,
 * schedule the transmission
 */
static void ep1in_cb(USBDriver * usbp, usbep_t ep)
{
	(void) usbp;
	(void) ep;
}

/*
 * EP1 OUT callback handler
 *
 * Data (OUT report) have just been received. Check if the input queue
 * is not full and in this case, prepare the reception of the next OUT
 * report.
 */
static void ep1out_cb(USBDriver * usbp, usbep_t ep)
{
	(void) usbp;
	(void) ep;

	chSysLockFromIsr ();

	// Check if there is still some space left in the input queue
	if (chIQGetEmptyI (&usb_input_queue) >= OUT_REPORT_SIZE)
	{
		chSysUnlockFromIsr ();
		// Prepares the reception of new data
		usbPrepareQueuedReceive (&USBD1, OUT_EP_NUM, &usb_input_queue, OUT_REPORT_SIZE);
		chSysLockFromIsr ();
		usbStartReceiveI (&USBD1, OUT_EP_NUM);
	}

	chSysUnlockFromIsr ();

	palTogglePad(GPIOD, LED_GREEN);
}

// EP1 initialization structure (both IN and OUT)
static const USBEndpointConfig ep1config = {
#ifdef USE_INTR
		USB_EP_MODE_TYPE_INTR,	// Interrupt EP
#else
	USB_EP_MODE_TYPE_BULK,	// Bulk EP
#endif
	NULL,					// SETUP packet notification callback
	ep1in_cb,				// IN notification callback
	ep1out_cb,				// OUT notification callback
	0x0040,					// IN maximum packet size
	0x0040,					// OUT maximum packet size
	&ep1instate,			// IN Endpoint state
	&ep1outstate,			// OUT endpoint state
	2,						// IN multiplier
	NULL					// SETUP buffer (not a SETUP endpoint)
};

// Handles the USB driver global events
static void usb_event_cb (USBDriver * usbp, usbevent_t event)
{
	switch (event)
	{
	case USB_EVENT_RESET:
		return;
	case USB_EVENT_ADDRESS:
		return;
	case USB_EVENT_CONFIGURED:
		chSysLockFromIsr ();
		if (usbp->configuration == 0) {  /* deconfigure the device */
			usbDisableEndpointsI(usbp);
		}
		else {
			// Enable the endpoints specified into the configuration.
			usbInitEndpointI(usbp, IN_EP_NUM, &ep1config);

			// Start the reception immediately
			chIQResetI(&usb_input_queue);

			usbPrepareQueuedReceive(&USBD1, OUT_EP_NUM, &usb_input_queue, IN_REPORT_SIZE);
			usbStartReceiveI(&USBD1, OUT_EP_NUM);
		}
		chSysUnlockFromIsr ();

		return;
	case USB_EVENT_SUSPEND:
		return;
	case USB_EVENT_WAKEUP:
		return;
	case USB_EVENT_STALLED:
		return;
	}
}

/*
 * Handles the GET_DESCRIPTOR callback
 *
 * Returns the proper descriptor
 */
static const USBDescriptor *usb_get_descriptor_cb (USBDriver * usbp, uint8_t dtype, uint8_t dindex, uint16_t lang)
{
	(void) usbp;
	(void) lang;

	switch (dtype)
	{
	// Generic descriptors
	case USB_DESCRIPTOR_DEVICE:	// Device Descriptor
		return &usb_device_descriptor;
	case USB_DESCRIPTOR_CONFIGURATION:	// Configuration Descriptor
		return &hid_configuration_descriptor;
	case USB_DESCRIPTOR_STRING:	// Strings
		if (dindex < 5)
			return &usb_strings[dindex];
		break;

	// HID specific descriptors
	case USB_DESCRIPTOR_HID:	// HID Descriptor
		return &hid_descriptor;
	case USB_DESCRIPTOR_HID_REPORT:	// HID Report Descriptor
		return &hid_report_descriptor;
	}

	return NULL;
}

// Callback for SETUP request on the endpoint 0 (control)
static bool_t usb_request_hook_cb(USBDriver *usbp) {
	const USBDescriptor *dp;

	// Handle HID class specific requests
	// Only GetReport is mandatory for HID devices
	if ((usbp->setup[0] & USB_RTYPE_TYPE_MASK) == USB_RTYPE_TYPE_CLASS)
	{
		if (usbp->setup[1] == HID_GET_REPORT)
		{
			/* setup[3] (MSB of wValue) = Report ID (must be 0 as we
			* have declared only one IN report)
			* setup[2] (LSB of wValue) = Report Type (1 = Input, 3 = Feature)
			*/
			if ((usbp->setup[3] == 0) && (usbp->setup[2] == 1))
			{
				// control structure for HID, currently unused

//				struct usb_hid_in_report_s in_report;
//				fill_header (&in_report);
//				usbSetupTransfer (usbp, (uint8_t *) & in_report,
//				USB_HID_IN_REPORT_SIZE, NULL);
			}
		}

		if (usbp->setup[1] == HID_SET_REPORT)
		{
			// Not implemented (yet)
		}
	}

	// Handle the Get_Descriptor Request for HID class (not handled by the default hook)
	if ((usbp->setup[0] == 0x81) && (usbp->setup[1] == USB_REQ_GET_DESCRIPTOR))
	{
		dp = usbp->config->get_descriptor_cb(usbp, usbp->setup[3], usbp->setup[2], usbFetchWord (&usbp->setup[4]));

		if (dp == NULL)
			return FALSE;

		usbSetupTransfer(usbp, (uint8_t *) dp->ud_string, dp->ud_size, NULL);
		return TRUE;
	}

	return FALSE;
}

// USB driver configuration
static const USBConfig usbcfg = {
	usb_event_cb,			// USB events callback
	usb_get_descriptor_cb,	// Device GET_DESCRIPTOR request callback
	usb_request_hook_cb,	// Requests hook callback
	NULL					// Start Of Frame callback
};

/*
 * Notification of data removed from the input queue
 *
 * If there is sufficient space in the input queue to receive a new
 * OUT report from the PC, prepare the reception of the next OUT
 * report
 */
static void usb_input_queue_inotify(GenericQueue * qp) {
	(void) qp;

	if (usbGetDriverStateI(&USBD1) != USB_ACTIVE)
		return;

	if (chIQGetEmptyI(&usb_input_queue) >= OUT_REPORT_SIZE) {
		chSysUnlock();
		usbPrepareQueuedReceive(&USBD1, OUT_EP_NUM, &usb_input_queue, OUT_REPORT_SIZE);

		chSysLock();
		usbStartReceiveI(&USBD1, OUT_EP_NUM);
	}
}

/*
 * Initialize the USB driver
 */
void init_usb_driver(void) {

	chIQInit(&usb_input_queue, input_queue_buffer, sizeof (input_queue_buffer), usb_input_queue_inotify, NULL);

	/*
	* Activates the USB driver and then the USB bus pull-up on D+.
	* Note, a delay is inserted in order to not have to disconnect the cable
	* after a reset.
	*/
	usbDisconnectBus(&USBD1);
	chThdSleepMilliseconds(1000);
	usbStart(&USBD1, &usbcfg);
	usbConnectBus(&USBD1);
}

