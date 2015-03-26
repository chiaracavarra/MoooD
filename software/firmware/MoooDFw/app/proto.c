#include <ch.h>
#include <hal.h>
#include <string.h>

#include "usbcfg.h"
#include "proto.h"
#include "leds.h"

extern PWMConfig pwmcfg;
extern PWMDriver PWMD4;

void set_duty_cycle(pwmcnt_t duty_cycle, pwmchannel_t channel) {

	// TODO: check duty_cycle value is between 0 and 100
	pwmcnt_t ticks = duty_cycle * 100;

	// TODO: check channel R = 0, G = 1, B = 2
	pwmDisableChannel(&PWMD4, channel);
	pwmEnableChannel(&PWMD4, channel, PWM_PERCENTAGE_TO_WIDTH(&PWMD4, ticks));
}

// TODO: check values
void LED_set_duty_cycle (uint16_t red_dc, uint16_t green_dc, uint16_t blue_dc)
{
	set_duty_cycle(green_dc, 0);
//	set_duty_cycle(orange_dc, 1);
	set_duty_cycle(red_dc, 2);
	set_duty_cycle(blue_dc, 3);
}

// TODO
void LED_switch_OFF (void)
{
	pwmDisableChannel(&PWMD4, 0);
//	pwmDisableChannel(&PWMD4, 1);
	pwmDisableChannel(&PWMD4, 2);
	pwmDisableChannel(&PWMD4, 3);
}

// TODO
void LED_switch_ON (void)
{
	pwmDisableChannel(&PWMD4, 0);
	pwmEnableChannel(&PWMD4, 0, pwmcfg.period);

//	pwmDisableChannel(&PWMD4, 1);
//	pwmEnableChannel(&PWMD4, 1, pwmcfg.period);

	pwmDisableChannel(&PWMD4, 2);
	pwmEnableChannel(&PWMD4, 2, pwmcfg.period);

	pwmDisableChannel(&PWMD4, 3);
	pwmEnableChannel(&PWMD4, 3, pwmcfg.period);

}

void eval_pkt (const pkt_t *pkt)
{
	// parse header
//	uint8_t   len		= pkt->header.len;
	command_t command	= pkt->header.command;

	switch (command) {
	case RGB_CMD:
	{
		LED_set_duty_cycle (pkt->rgb_data.red_dc, pkt->rgb_data.green_dc, pkt->rgb_data.blue_dc);
		break;
	}
	case OFF_CMD:
	{
		LED_switch_OFF();
		break;
	}
	case ON_CMD:
	{
		LED_switch_ON();
		break;
	}
	default:
	{
		LED_switch_OFF();
		break;
	}
	}
}

static int send_pkt(const pkt_t *pkt)
{
  return usb_send_data((const uint8_t *) pkt, IN_REPORT_SIZE);
}

static int read_pkt(pkt_t *pkt, systime_t timeout)
{
  return usb_read_data((uint8_t *) pkt, OUT_REPORT_SIZE, timeout);
}

void loop_tick(systime_t timeout)
{
  static pkt_t req_pkt, ans_pkt;

  if (read_pkt(&req_pkt, timeout) > 0)
  {
	  // Parse answer packet
	  eval_pkt(&req_pkt);

	  // OPTIONAL
	  // Prepare reply packet
	  ans_pkt = req_pkt;

	  // send answer packet
	  send_pkt(&ans_pkt);
  }
}

int usb_send_packet(const pkt_t *packet)
{
	chSysLock ();
	if (usbGetDriverStateI (&USBD1) != USB_ACTIVE)
	{
		chSysUnlock ();
		return 0;
	}

	usbPrepareTransmit(&USBD1, IN_EP_NUM, (uint8_t *) packet, OUT_REPORT_SIZE);
	bool_t failed = usbStartTransmitI(&USBD1, IN_EP_NUM);
	chSysUnlock();

	if (failed)
	{
		chThdSleepMilliseconds(200);
//		palTogglePad(GPIOD, LED_ORANGE);
	}

	return 0;
}
