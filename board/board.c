#include "board.h"
#include "nrf52.h"

#define NRF_LOG_MODULE_NAME "BRD"
#define NRF_LOG_LEVEL 4
#include "nrf_log.h"

uint32_t BoardGetRandomSeed( void )
{
	uint32_t seed = NRF_FICR->DEVICEID[0];
	return seed;
}

void BoardGetUniqueId( uint8_t *id )
{
	uint32_t did[2];
	did[0] = NRF_FICR->DEVICEID[0];
	did[1] = NRF_FICR->DEVICEID[1];
	memcpy(id, did, 8);
}

