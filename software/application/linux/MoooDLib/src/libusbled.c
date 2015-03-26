/*
 * Simple libusb-1.0 test programm
 * It openes an USB device, expects two Bulk endpoints,
 *   EP1 should be IN
 *   EP2 should be OUT
 * It alternates between reading and writing a packet to the Device.
 * It uses Synchronous device I/O
 *
 * Compile:
 *   gcc -lusb-1.0 -o test test.c
 * Run:
 *   ./test
 * Thanks to BertOS for the example:
 *   http://www.bertos.org/use/tutorial-front-page/drivers-usb-device
 *
 * For Documentation on libusb see:
 *   http://libusb.sourceforge.net/api-1.0/modules.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
//#include <stdatomic.h>  // not quite there

#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include <libusb-1.0/libusb.h>

#include <endian.h>

typedef uint32_t systime_t;

#include "libusbled.h"

// gcc -std=c99 -D_BSD_SOURCE -pthread -Wall -o test test.c -lusb-1.0
// or
// gcc -std=gnu99 -pthread -Wall -o test test_dev.c -lusb-1.0


#define USB_VENDOR_ID	    0x0483
#define USB_PRODUCT_ID	    0x5740

#define INTERFACE_NUM		0
#define ENDPOINT_NUM		1
#define ENDPOINT_IN_NUM	    1
#define ENDPOINT_OUT_NUM	1
#define USB_TIMEOUT	        3000        /* Connection timeout (in ms) */

#define TX_SIZE	64
#define RX_SIZE	64


static struct libusb_device_handle *handle;

const char * cmd_to_str(command_t command) {
	static const char *commands[] = {
			"LED off",
			"LED on",
			"set duty cycle"
	};

	// MSB = error bit
	return (command & 0x80) ? "ERR" : commands[command & 0x7f];
}

#ifdef USE_INTR

static int usb_write_interrupt(struct libusb_device_handle *handle, uint8_t endpoint, pkt_t *pkt, int length) {
	int nsent;

	int ret = libusb_interrupt_transfer(handle, endpoint | LIBUSB_ENDPOINT_OUT, (uint8_t *) pkt, length, &nsent, USB_TIMEOUT);
	if (ret) {
		return ret;
	}

	return nsent;
}

static int usb_read_interrupt(struct libusb_device_handle *handle, uint8_t endpoint, pkt_t *pkt, int length) {
	int nrecv;

	int ret = libusb_interrupt_transfer(handle, endpoint | LIBUSB_ENDPOINT_IN, (uint8_t *) pkt, sizeof(pkt_t), &nrecv, USB_TIMEOUT);
	if (ret) {
		return ret;
	}

	return nrecv;
}

#else

static int usb_write_bulk(struct libusb_device_handle *handle, uint8_t endpoint, pkt_t *pkt, int length) {
	int nsent;

	int ret = libusb_bulk_transfer(handle, endpoint | LIBUSB_ENDPOINT_OUT, (uint8_t *) pkt, length, &nsent, USB_TIMEOUT);
	if (ret) {
		return ret;
	}

	return nsent;
}

static int usb_read_bulk(struct libusb_device_handle *handle, uint8_t endpoint, pkt_t *pkt, int length) {
	int nrecv;

	int ret = libusb_bulk_transfer(handle, endpoint | LIBUSB_ENDPOINT_IN, (uint8_t *) pkt, sizeof(pkt_t), &nrecv, USB_TIMEOUT);
	if (ret) {
		return ret;
	}

	return nrecv;
}

#endif


static int usb_write(struct libusb_device_handle *handle, uint8_t endpoint, pkt_t *pkt, int length) {
	int nsent;

#ifdef USE_INTR

	nsent = usb_write_interrupt(handle, endpoint, pkt, length);

#else

	nsent = usb_write_bulk(handle, endpoint, pkt, length);

#endif

	return nsent;
}

static int usb_read(struct libusb_device_handle *handle, uint8_t endpoint, pkt_t *pkt, int length) {
	int nrecv;

#ifdef USE_INTR

	nrecv = usb_read_interrupt(handle, endpoint, pkt, length);

#else

	nrecv = usb_read_bulk(handle, endpoint, pkt, length);

#endif

	return nrecv;
}


static int send_recv_pkt(struct libusb_device_handle *handle, pkt_t *pkt, int length) {

	int w_ret = usb_write(handle, ENDPOINT_OUT_NUM, pkt, length);

	if (w_ret < 0) {
		fprintf(stderr, "Failed to send request: %d\n", w_ret);
		return -1;

	} else {

		fprintf(stderr, "Sent %d bytes to device\n", w_ret);
	}

	int r_ret = usb_read(handle, ENDPOINT_IN_NUM, pkt, length);

	if (r_ret < 0) {
		fprintf(stderr, "Failed to receive data: %d\n", r_ret);
		return -2;

	} else if (pkt->header.command & 0x80) {
		fprintf(stderr, "Wrong request: %x\n", pkt->header.command);
		return -3;

	} else {

		fprintf(stderr, "Data received via transfer:\n");
	}

	return 0;
}

static void libusb_cleanup(struct libusb_device_handle *handle) {
	libusb_close(handle);
	libusb_exit(NULL);
}

static void print_header(header_t *header)
{
	fprintf(stderr, "* %s *\n", cmd_to_str(header->command));
	fprintf(stderr, "length    : %d\n", header->len);
}

static void print_data(pkt_t *pkt)
{
	print_header(&pkt->header);
	fprintf(stderr, "R  : %d\n", pkt->rgb_data.red_dc);
	fprintf(stderr, "G  : %d\n", pkt->rgb_data.green_dc);
	fprintf(stderr, "B  : %d\n", pkt->rgb_data.blue_dc);
}

void switch_on() {

	pkt_t pkt;
	pkt.header.len = 0;
	pkt.header.command = ON_CMD;
	print_header(&pkt.header);

	if (send_recv_pkt(handle, &pkt, sizeof(pkt_t)) == 0) {
		print_data(&pkt);
	}
}

void switch_off() {

	pkt_t pkt;
	pkt.header.len = 0;
	pkt.header.command = OFF_CMD;

	print_header(&pkt.header);

	if (send_recv_pkt(handle, &pkt, sizeof(pkt_t)) == 0) {
		print_data(&pkt);
	}
}

void set_rgb(unsigned int red_dc, unsigned int green_dc, unsigned int blue_dc) {

	pkt_t pkt;
	pkt.header.command = RGB_CMD;
	pkt.header.len = sizeof(data_pkt_t);

	if ((red_dc <= 100) && (green_dc <= 100) && (blue_dc <= 100)) {
		pkt.rgb_data.red_dc = red_dc;
		pkt.rgb_data.green_dc = green_dc;
		pkt.rgb_data.blue_dc = blue_dc;
	}
	print_data(&pkt);

	if (send_recv_pkt(handle, &pkt, sizeof(pkt_t)) == 0) {
		print_data(&pkt);
	}
}

int init() {

	libusb_init(NULL);
	libusb_set_debug(NULL, LIBUSB_LOG_LEVEL_INFO);

	//Open Device with VendorID and ProductID
	handle = libusb_open_device_with_vid_pid(NULL, USB_VENDOR_ID, USB_PRODUCT_ID);
	if (handle == NULL) {
		perror("device not found");
		return 1;
	}

	int conf = 0;
	int ret = 0;

	ret = libusb_get_configuration(handle, &conf);
	if (ret != LIBUSB_SUCCESS) {
		fprintf(stderr, "failed to retrieve the current device configuration (%s)\n", libusb_strerror(ret));
		libusb_cleanup(handle);
		return 3;
	}

	if (conf < 1) {
		ret = libusb_set_configuration(handle, 1);
		if (ret) {
			fprintf(stderr, "failed to set the device configuration (%s)\n", libusb_strerror(ret));
			libusb_cleanup(handle);
			return 3;
		}
	}

	if (!libusb_has_capability(LIBUSB_CAP_SUPPORTS_DETACH_KERNEL_DRIVER)) {
		fprintf(stderr, "auto detaching not supported\n");
		//		libusb_cleanup(handle);
		//		return 5;
	}

	if ((libusb_kernel_driver_active(handle, INTERFACE_NUM) == 1)
			&& (libusb_set_auto_detach_kernel_driver(handle, 1) != LIBUSB_SUCCESS)) {
		fprintf(stderr, "failed to set kernel driver auto detaching\n");
		libusb_cleanup(handle);
		return 4;
	}

	fprintf(stderr, "the device is using configuration number %d\n", conf);

	struct libusb_config_descriptor *conf_desc;
	ret = libusb_get_config_descriptor(libusb_get_device(handle), 0, &conf_desc);
	//	ret = libusb_get_active_config_descriptor(libusb_get_device(handle), &conf_desc);
	if (ret != LIBUSB_SUCCESS) {
		libusb_cleanup(handle);
		return 10;
	}

	if (conf_desc->bNumInterfaces == 1) {

	}

	libusb_free_config_descriptor(conf_desc);


	ret = libusb_claim_interface(handle, INTERFACE_NUM);
	if (ret != LIBUSB_SUCCESS) {
		fprintf(stderr, "usb_claim_interface error (%s)\n", libusb_strerror(ret));
		libusb_cleanup(handle);
		return 2;
	}

	fprintf(stderr, "Interface %d claimed\n", INTERFACE_NUM);

	return 0;
}

int release() {

	// lib cleanup
	libusb_release_interface(handle, INTERFACE_NUM);
	libusb_cleanup(handle);

	return 0;
}
