#ifndef _BOARD_H
#define _BOARD_H
#include <stdbool.h>
#include <stdint.h>
#include "app_error.h"
#include "nordic_common.h"

#include "rtc.h"
#include "gpio.h"
#include "spi.h"
#include "timer.h"
#include "utilities.h"
#include "radio.h"
#include "sx1276/sx1276.h"
#include "sx1276-board.h"
#include "lora.h"

/* SX1276MB1MAS pinout */
#define RADIO_DIO_0 P13 /* J1 pin 3 */
#define RADIO_DIO_1 P14 /* J1 pin 4 */
#define RADIO_DIO_2 P15 /* J1 pin 5 */
#define RADIO_DIO_3 P16 /* J1 pin 6 */
#define RADIO_DIO_4 P19 /* J2 pin 1 */
#define RADIO_DIO_5 P20 /* J2 pin 2 */

#define RADIO_NSS P22 /* J2 pin 3 */
#define RADIO_MOSI P23 /* J2 pin 4 */
#define RADIO_MISO P24 /* J2 pin 5 */
#define RADIO_SCK P25 /* J2 pin 6 */

#define RADIO_RESET P3 /* J4 pin 1 */
#define RADIO_ANT_SWITCH_HF P30 /* J4 pin 5 */

#define RADIO_DIO4_B P29 /* J4 pin 4 */

#define ASSERT_ERROR 0xA55EA55E

#define USE_FULL_ASSERT
#ifdef USE_FULL_ASSERT
/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function
  *         which reports the name of the source file and the source
  *         line number of the call that failed.
  *         If expr is true, it returns no value.
  * @retval None
  */
  #define assert_param(expr) ((expr) ? (void)0U : app_error_handler(ASSERT_ERROR,\
								    __LINE__,\
								    (const uint8_t *)__FILE__))
#else
  #define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */


/*!
 * Returns a pseudo random seed generated using the MCU Unique ID
 *
 * \retval seed Generated pseudo random seed
 */
uint32_t BoardGetRandomSeed( void );

/*!
 * \brief Gets the board 64 bits unique ID
 *
 * \param [IN] id Pointer to an array that will contain the Unique ID
 */
void BoardGetUniqueId( uint8_t *id );

#endif
