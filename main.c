#include <stdbool.h>
#include <stdint.h>
#include "app_config.h"
#include "app_timer.h"

#include "nrf_drv_gpiote.h"
#include "nrf_drv_wdt.h"
#include "nrf_delay.h"
#include "nrf_sdm.h"
#include "softdevice_handler.h"
#include "mem_manager.h"
#include "board.h"

#define NRF_LOG_MODULE_NAME "APP"
#define NRF_LOG_LEVEL 4
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

TIMER_DEF(main_sm_timer);
static char build_version[] = BUILD_VERSION;
static nrf_drv_wdt_channel_id m_channel_id;
static void main_sm_timer_callback(void *ctx);

/**
 * @brief Assert callback.
 *
 * @param[in] id    Fault identifier. See @ref NRF_FAULT_IDS.
 * @param[in] pc    The program counter of the instruction that triggered the fault, or 0 if
 *                  unavailable.
 * @param[in] info  Optional additional information regarding the fault. Refer to each fault
 *                  identifier for details.
 */
void wdt_init(void);
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
	error_info_t *error_info = (error_info_t*)info;
	NRF_LOG_ERROR("code: %d %s:%d\r\n", error_info->err_code,
		      (uint32_t)error_info->p_file_name,
		      error_info->line_num);
	NRF_LOG_FLUSH();
	wdt_init();
	while (1) {};
}

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name)
{
        app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for initializing logging. */
void log_init(void)
{
        ret_code_t err_code = NRF_LOG_INIT(NULL);
        APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing timers. */
void timers_init(void)
{
	ret_code_t err_code = app_timer_init();
        APP_ERROR_CHECK(err_code);
	TIMER_INIT(main_sm_timer, main_sm_timer_callback);
}

/**@brief Function for initializing GPIOTE. */
void gpiote_init(void)
{
	ret_code_t err_code = nrf_drv_gpiote_init();
        APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing MEMORY MANAGER. */
void mem_mgr_init(void)
{
	ret_code_t err_code = nrf_mem_init();
        APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing Softdevice. */
void sd_init(void)
{
	ret_code_t err_code;
        nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

	// Initialize the SoftDevice handler module.
        SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);

	// Fetch the starting address of the application ram. This is needed by the upcoming SoftDevice calls.
        uint32_t ram_start = 0;
        err_code = softdevice_app_ram_start_get(&ram_start);
        APP_ERROR_CHECK(err_code);

	// Overwrite some of the default configurations for the BLE stack.
        ble_cfg_t ble_cfg;

        // Configure the number of custom UUIDS.
        memset(&ble_cfg, 0, sizeof(ble_cfg));
        ble_cfg.common_cfg.vs_uuid_cfg.vs_uuid_count = 0;
        err_code = sd_ble_cfg_set(BLE_COMMON_CFG_VS_UUID, &ble_cfg, ram_start);
        APP_ERROR_CHECK(err_code);

        // Configure the maximum number of connections.
        memset(&ble_cfg, 0, sizeof(ble_cfg));
        ble_cfg.gap_cfg.role_count_cfg.periph_role_count  = BLE_GAP_ROLE_COUNT_PERIPH_DEFAULT;
        ble_cfg.gap_cfg.role_count_cfg.central_role_count = 0;
        ble_cfg.gap_cfg.role_count_cfg.central_sec_count  = 0;
        err_code = sd_ble_cfg_set(BLE_GAP_CFG_ROLE_COUNT, &ble_cfg, ram_start);
        APP_ERROR_CHECK(err_code);

        memset(&ble_cfg, 0, sizeof(ble_cfg));
        ble_cfg.gatts_cfg.attr_tab_size.attr_tab_size = BLE_GATTS_ATTR_TAB_SIZE_MIN;
        err_code = sd_ble_cfg_set(BLE_GATTS_CFG_ATTR_TAB_SIZE, &ble_cfg, ram_start);
        APP_ERROR_CHECK(err_code);

        // Enable BLE stack.
        err_code = softdevice_enable(&ram_start);
        APP_ERROR_CHECK(err_code);
}

void wdt_event_handler(void)
{
    //NOTE: The max amount of time we can spend in WDT interrupt is two cycles of 32768[Hz] clock - after that, reset occurs
}

void wdt_init(void)
{
    nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
    ret_code_t err_code = nrf_drv_wdt_init(&config, wdt_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_wdt_channel_alloc(&m_channel_id);
    APP_ERROR_CHECK(err_code);
    nrf_drv_wdt_enable();
}

#define FPU_EXCEPTION_MASK               0x0000009F                      //!< FPU exception mask used to clear exceptions in FPSCR register.

/**@brief Function for doing power management. */
void power_manage(void)
{
	__set_FPSCR(__get_FPSCR()  & ~(FPU_EXCEPTION_MASK));
	(void) __get_FPSCR();
        NVIC_ClearPendingIRQ(FPU_IRQn);
	ret_code_t err_code = sd_app_evt_wait();
        APP_ERROR_CHECK(err_code);
}

static void main_sm_timer_callback(void *ctx)
{
	NRF_LOG_DEBUG("sm_timer_callback\r\n");
}

int main(void)
{
	/* Initialize */
        log_init();
	NRF_LOG_INFO("Version: %s\r\n", (uint32_t)build_version);
        NRF_LOG_INFO("HW init start\r\n");
	timers_init();
	gpiote_init();
	mem_mgr_init();
	rtc_init(NULL);
	sd_init();
	NRF_LOG_INFO("HW init done\r\n");
#if 0
	TimerSetValue(&main_sm_timer, 1000);
	TimerStart(&main_sm_timer);


	while(1){NRF_LOG_PROCESS();};
#endif


	SX1276IoInit();
	/* Enter main loop */
        for (;;) {
		while(NRF_LOG_PROCESS()){};
		lora_sm();
	}
}

