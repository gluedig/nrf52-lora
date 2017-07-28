#include "board.h"
#include "nrf_drv_spi.h"

#define NRF_LOG_MODULE_NAME "SPI"
#define NRF_LOG_LEVEL 3
#include "nrf_log.h"

void SpiInit( Spi_t *obj,
	      PinNames mosi,
	      PinNames miso,
	      PinNames sclk,
	      PinNames nss )
{
	ret_code_t err_code;
	NRF_LOG_DEBUG("Init 0x%x %d %d %d %d\r\n",
		      (uint32_t)obj,
		      mosi,
		      miso,
		      sclk,
		      nss);
	obj->Instance.p_registers = NRF_SPI0;
	obj->Instance.irq = SPI0_IRQ;
	obj->Instance.drv_inst_idx = SPI0_INSTANCE_INDEX;
	obj->Instance.use_easy_dma = SPI0_USE_EASY_DMA;

	obj->Config.sck_pin		= sclk;
	obj->Config.mosi_pin		= mosi;
	obj->Config.miso_pin		= miso;
	obj->Config.ss_pin		= nss;
	obj->Config.irq_priority	= SPI_DEFAULT_CONFIG_IRQ_PRIORITY;
	obj->Config.orc			= 0xFF;
	obj->Config.mode		= NRF_DRV_SPI_MODE_0;
	obj->Config.bit_order		= NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;
	obj->Config.frequency		= NRF_DRV_SPI_FREQ_1M;
	err_code = nrf_drv_spi_init(&obj->Instance, &obj->Config,
				    NULL, NULL);

	if (err_code != NRF_SUCCESS) {
		NRF_LOG_ERROR("error: %d\r\n", err_code);
		return;
	}
	obj->initialized = true;
}

void SpiDeInit( Spi_t *obj )
{
	NRF_LOG_DEBUG("DeInit 0x%x\r\n", (uint32_t)obj);
	if (obj->initialized) {
		nrf_drv_spi_uninit(&obj->Instance);
		obj->initialized = false;
	}
}

void SpiOut( Spi_t *obj, uint8_t addr, uint8_t *outData, uint8_t outLen )
{
	NRF_LOG_DEBUG("Out 0x%x 0x%x 0x%x %d\r\n",
		      (uint32_t)obj,
		      addr,
		      (uint32_t)outData,
		      outLen);
	if (obj->initialized != true) {
		NRF_LOG_ERROR("not initilized\r\n");
		return;
	}

	uint8_t tx[outLen+1];
	tx[0] = addr | 0x80;
	memcpy(&tx[1], outData, outLen);

	ret_code_t err_code = nrf_drv_spi_transfer(&obj->Instance,
						   tx,
						   sizeof(tx),
						   NULL,
						   0);

	if (err_code != NRF_SUCCESS) {
		NRF_LOG_ERROR("error %d\r\n", err_code);
	}
	NRF_LOG_DEBUG("Sent: \r\n");
        NRF_LOG_HEXDUMP_DEBUG(outData, outLen);

	return;
}

void SpiIn( Spi_t *obj, uint8_t addr, uint8_t *inData, uint8_t inLen )
{
	NRF_LOG_DEBUG("In 0x%x 0x%x 0x%x %d\r\n",
		      (uint32_t)obj,
		      addr,
		      (uint32_t)inData,
		      inLen);

	if (obj->initialized != true) {
		NRF_LOG_ERROR("not initilized\r\n");
		return;
	}

	uint8_t tx[] = { addr & 0x7F };
	uint8_t rx[inLen+1];
	ret_code_t err_code = nrf_drv_spi_transfer(&obj->Instance,
						   tx,
						   1,
						   rx,
						   inLen+1);

	if (err_code != NRF_SUCCESS) {
		NRF_LOG_ERROR("error %d\r\n", err_code);
	}
	memcpy(inData, &rx[1], inLen);
	NRF_LOG_DEBUG(" Received: \r\n");
        NRF_LOG_HEXDUMP_DEBUG(inData, inLen);

	return;
}

