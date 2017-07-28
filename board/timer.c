#include "board.h"

#define NRF_LOG_MODULE_NAME "TIMER"
#define NRF_LOG_LEVEL 3
#include "nrf_log.h"

void TimerInit( TimerEvent_t *obj, void ( *callback )( void * ) )
{
	NRF_LOG_DEBUG("Init 0x%x 0x%x 0x%x\r\n",
		      (uint32_t)obj,
		      (uint32_t)obj->id,
		      (uint32_t)callback);
	if (obj->id == NULL) {
		NRF_LOG_ERROR("0x%x not initialized\r\n",
			      (uint32_t)obj);
		return;
	}
	ret_code_t err_code = app_timer_create(&obj->id,
					       APP_TIMER_MODE_SINGLE_SHOT,
					       callback);
	if (err_code != NRF_SUCCESS) {
		NRF_LOG_ERROR("Init 0x%x 0x%x 0x%x error %d\r\n",
		      (uint32_t)obj,
		      (uint32_t)obj->id,
		      (uint32_t)callback,
		      err_code);
	}
};


void TimerStart( TimerEvent_t *obj )
{
	NRF_LOG_DEBUG("Start 0x%x %d\r\n",
		      (uint32_t)obj,
		      obj->timeout);
	if (obj->id == NULL || obj->timeout == 0) {
		NRF_LOG_ERROR("0x%x wrong state\r\n",
			      (uint32_t)obj);
		return;
	}
	uint32_t ticks = APP_TIMER_TICKS(obj->timeout);
	if (ticks < APP_TIMER_MIN_TIMEOUT_TICKS)
		ticks = APP_TIMER_MIN_TIMEOUT_TICKS;

	ret_code_t err_code = app_timer_start(obj->id,
					      ticks,
					      NULL);
	if (err_code != NRF_SUCCESS) {
		NRF_LOG_DEBUG("Start 0x%x %d %d error %d\r\n",
		      (uint32_t)obj,
		      obj->timeout,
		      ticks,
		      err_code);
	}
}

void TimerStop( TimerEvent_t *obj )
{
	NRF_LOG_DEBUG("Stop 0x%x\r\n",
		      (uint32_t)obj);
	if (obj->id == NULL) {
		NRF_LOG_ERROR("0x%x not initialized\r\n",
			      (uint32_t)obj);
		return;
	}
	ret_code_t err_code = app_timer_stop(obj->id);
	if (err_code != NRF_SUCCESS) {
		NRF_LOG_DEBUG("Stop 0x%x error %d\r\n",
		      (uint32_t)obj,
		      err_code);
	}
}

void TimerSetValue( TimerEvent_t *obj, uint32_t value )
{
	NRF_LOG_DEBUG("SetValue 0x%x %d\r\n",
		      (uint32_t)obj,
		      value);
	if (value < 10)
		value = 10;
	obj->timeout = value;
}

TimerTime_t TimerGetCurrentTime( void )
{
	return rtc_get_timestamp();
}

TimerTime_t TimerGetElapsedTime( TimerTime_t savedTime )
{
	uint32_t ts = rtc_get_timestamp();
	uint32_t elapsed = 0;
	if (savedTime < ts)
		elapsed = ts - savedTime;

	return elapsed;
}

