
#include "ch.h"
#include "hal.h"

#include "leds.h"
#include "usbcfg.h"
#include "proto.h"


static uint16_t interval = 100;

static void pwmp_ARRcb (PWMDriver *pwmp) {
	(void) pwmp;

//	palSetPad(GPIOD, LED_RED);
//	palSetPad(GPIOD, LED_GREEN);
//	palSetPad(GPIOD, LED_BLUE);
}

static void pwmp_R_CNTcb (PWMDriver *pwmp) {
	(void) pwmp;
//	palClearPad(GPIOD, LED_RED);
}

static void pwmp_G_CNTcb (PWMDriver *pwmp) {
	(void) pwmp;
//	palClearPad(GPIOD, LED_GREEN);
}

static void pwmp_B_CNTcb (PWMDriver *pwmp) {
	(void) pwmp;
//	palClearPad(GPIOD, LED_BLUE);
}

#define F_REF 4000000  // APB timer clock 4MHz

/*
 * PWM configuration structure.
 * Cyclic callback enabled, channels 1 and 4 enabled without callbacks,
 * the active state is a logic one.
 */
PWMConfig pwmcfg = {
	100000,										/* PWM clock frequency 100KHz */
	128,										/* PWM period (in ticks).     */
	pwmp_ARRcb,									/* ARR                        */
	{
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},	/* CNT                        */
		{PWM_OUTPUT_DISABLED, NULL},	/* CNT                        */
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},	/* CNT                        */
		{PWM_OUTPUT_ACTIVE_HIGH, NULL}
	},
	/* HW dependent part.*/
	0,
	0
};

//static void set_freq(int out_freq) {
//
//	uint32_t d;
//	uint32_t pre;
//
//	uint32_t des_fclk;
//	pwmcnt_t ticks;
//
//	// TODO check frequency range
//	if (out_freq < 0) {
//		return;
//	}
//
//	if (out_freq == 0) {
//		pwmDisableChannel(&PWMD2, 1);
//		return;
//	}
//
//	d = F_REF/out_freq;
//	pre = ((d >> 16) & 0xFFFF)*2 + 1;
//	des_fclk = F_REF / pre;
//
//	ticks = des_fclk / out_freq;
//
//	pwmDisableChannel(&PWMD2, 1);
//	pwmStop(&PWMD2);
//	pwmcfg.frequency = des_fclk;
//	pwmcfg.period = ticks;
//	pwmStart(&PWMD2, &pwmcfg);
//	pwmEnableChannel(&PWMD2, 1, ticks/2);
//}



/*
 * Main thread
 */
int main(void)
{
	// ChibiOS/RT init
	halInit();
	chSysInit();

	palClearPad(GPIOD, LED_BLUE);
	palClearPad(GPIOD, LED_RED);
	palClearPad(GPIOD, LED_GREEN);
	palClearPad(GPIOD, LED_ORANGE);

	// Init USB
	init_usb_driver();

	// Wait until the USB is active
	while (USBD1.state != USB_ACTIVE)
		chThdSleepMilliseconds(200);

	// Start the button thread
//	chThdCreateStatic(blinkerWorkingArea, sizeof blinkerWorkingArea, NORMALPRIO, blinkerThread, NULL);

	pwmStart(&PWMD4, &pwmcfg);
//	palSetPadMode(GPIOD, LED_GREEN, PAL_MODE_ALTERNATE(2));		/* Green.  1 */
//	palSetPadMode(GPIOD, LED_ORANGE, PAL_MODE_ALTERNATE(2));	/* Orange. 2 */
//	palSetPadMode(GPIOD, LED_RED, PAL_MODE_ALTERNATE(2));		/* Red.    3 */
//	palSetPadMode(GPIOD, LED_BLUE, PAL_MODE_ALTERNATE(2));		/* Blue.   4 */

	palSetPadMode(GPIOB, 6, PAL_MODE_ALTERNATE(2));
	palSetPadMode(GPIOB, 8, PAL_MODE_ALTERNATE(2));
	palSetPadMode(GPIOB, 9, PAL_MODE_ALTERNATE(2));

	/*
	* Main loop: waits until a OUT report is received and updates the
	* LED
	*/
	while (TRUE)
	{
		loop_tick(TIME_INFINITE);
	}
}
