#include "rtc.h"

#include "app_config.h"
#include "nrf_drv_rtc.h"
#include "nrf_rtc.h"

#define NRF_LOG_MODULE_NAME "RTC"
#define NRF_LOG_LEVEL 3
#include "nrf_log.h"

const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(2); /**< Declaring an instance of nrf_drv_rtc for RTC0. */

static uint32_t timestamp_base = 0;
static rtc_wakeup_callback_t wakeup_clbk = NULL;

static void rtc_handler(nrf_drv_rtc_int_type_t int_type)
{
	if (int_type == NRF_DRV_RTC_INT_OVERFLOW) {
		uint32_t old_base = timestamp_base;
		timestamp_base += nrf_drv_rtc_max_ticks_get(&rtc)/RTC_FREQUENCY;
		NRF_LOG_DEBUG("overflow %d %d\r\n",
			      old_base, timestamp_base);
	}
	if (int_type == NRF_DRV_RTC_INT_COMPARE0) {
		NRF_LOG_DEBUG("COMPARE0 wakeup\r\n");
		if (wakeup_clbk)
			wakeup_clbk();
	}
}

uint32_t rtc_get_timestamp(void)
{
	uint32_t ticks = nrf_drv_rtc_counter_get(&rtc);
	uint32_t timestamp = timestamp_base  + ticks/RTC_FREQUENCY;
/*
	NRF_LOG_DEBUG("get_timestamp %d %d %d\r\n",
		      ticks, timestamp, timestamp_base);
*/
	return timestamp;
}

void rtc_update_timestamp(uint32_t timebase)
{
	NRF_LOG_DEBUG("update_timestamp old: 0x%08x new: 0x%08x\r\n",
		      timestamp_base, timebase);
	timestamp_base = timebase;
}

ret_code_t rtc_init(rtc_wakeup_callback_t clbk)
{
	NRF_LOG_DEBUG("init rtc_freq: %d\r\n", RTC_FREQUENCY);
        ret_code_t err_code;

        //Initialize RTC instance
        nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;
        config.prescaler = RTC_FREQ_TO_PRESCALER(RTC_FREQUENCY);
        err_code = nrf_drv_rtc_init(&rtc, &config, rtc_handler);
        APP_ERROR_CHECK(err_code);

        //Enable tick event - no interrupt, we need tick just to drive ADC
        nrf_drv_rtc_tick_enable(&rtc, false);
	nrf_drv_rtc_overflow_enable(&rtc, true);

        //Power on RTC instance
        nrf_drv_rtc_enable(&rtc);
	NRF_LOG_DEBUG("enabled prescaler: %d max_tics: %d\r\n",
		      rtc_prescaler_get(rtc.p_reg),
		      nrf_drv_rtc_max_ticks_get(&rtc));

	wakeup_clbk = clbk;
	return err_code;
}

void rtc_test_overflow(void)
{
	NRF_LOG_DEBUG("test_overflow\r\n");
	nrf_rtc_task_trigger(rtc.p_reg, NRF_RTC_TASK_TRIGGER_OVERFLOW);
}

ret_code_t rtc_schedule_wakeup(uint32_t timeout)
{
	NRF_LOG_DEBUG("schedule_wakeup %d\r\n", timeout);
	uint32_t ticks = nrf_drv_rtc_counter_get(&rtc);
	uint32_t timeout_ticks = timeout*RTC_FREQUENCY;
	uint32_t compare_val = 0;
	uint32_t max_ticks = nrf_drv_rtc_max_ticks_get(&rtc);
	if (ticks + timeout_ticks > max_ticks) {
		compare_val = (max_ticks - ticks) + timeout_ticks;
	} else {
		compare_val = ticks + timeout_ticks;
	}
	NRF_LOG_DEBUG("rtc_cc_set %d %d\r\n", ticks, compare_val);
	return nrf_drv_rtc_cc_set(&rtc, 0, compare_val, true);
}



