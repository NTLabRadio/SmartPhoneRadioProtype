/**
  ******************************************************************************
  * @file    spi_cc1120.h
  * @brief   Файл, содержащий заголовки функций, реализующих обмен котроллера
	*						с микросхемой трансивера по интерфейсу SPI
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 NTLab
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI_CC1120_H
#define __SPI_CC1120_H

#ifdef __cplusplus
 extern "C" {
#endif

#ifdef STM32F071xB
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_spi.h" 
#endif
	 
#ifdef STM32F103xE	 
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_spi.h"	 
#endif

	 
#include "globals.h"
#include "spi_periphery.h"
#include "timers.h"


typedef enum
{
	READ_WRITE_OK   	= 0x00, // операция завершена успешно
	SPI_BUSY       		= 0x01, // шина SPI занята
	DATA_IN_MISMATCH	= 0x02  // несоответствие формата входных данных
} ReadWriteRegTypeDef;	 

typedef enum
{
	STATUS_IDLE   							= 0x00, 
	STATUS_RX       						= 0x01,
	STATUS_TX										= 0x02,
	STATUS_FSTXON 							= 0x03,
	STATUS_CALIBRATE 						= 0x04,
	STATUS_SETTLING 						= 0x05,
	STATUS_RX_FIFO_ERROR				= 0x06,
	STATUS_TX_FIFO_ERROR 				= 0x07,
	STATUS_SPI_ERROR 						= 0x08
} CC1120STATUSTypeDef;	




typedef enum
{
	MARCSTATE_SLEEP								= 0x00,
	MARCSTATE_IDLE								= 0x01,
	MARCSTATE_XOFF								= 0x02,
	MARCSTATE_BIAS_SETTLE_MC			= 0x03, 
	MARCSTATE_REG_SETTLE_MC				= 0x04, 
	MARCSTATE_MANCAL							= 0x05, 
	MARCSTATE_BIAS_SETTLE					= 0x06, 
	MARCSTATE_REG_SETTLE					= 0x07, 
	MARCSTATE_STARTCAL						= 0x08, 
	MARCSTATE_BWBOOST							= 0x09, 
	MARCSTATE_FS_LOCK							= 0x0A, 
	MARCSTATE_IFADCON							= 0x0B, 
	MARCSTATE_ENDCAL							= 0x0C,
	MARCSTATE_RX									= 0x0D,
	MARCSTATE_RX_END							= 0x0E, 
	MARCSTATE_Reserved						= 0x0F, 
	MARCSTATE_TXRX_SWITCH					= 0x10, 
	MARCSTATE_RX_FIFO_ERR					= 0x11, 
	MARCSTATE_FSTXON							= 0x12, 
	MARCSTATE_TX									= 0x13, 
	MARCSTATE_TX_END							= 0x14, 
	MARCSTATE_RXTX_SWITCH					= 0x15, 
	MARCSTATE_TX_FIFO_ERR					= 0x16, 
	MARCSTATE_IFADCON_TXRX				= 0x17,
	MARCSTATE_SPI_ERROR 					= 0x18
} CC1120MARCSTATETypeDef;	


typedef struct
{
  uint16_t  addr; // поле адреса
  uint8_t   data; // поле данных
}CC1120regSetting_t;	// шаблон структуры конфигурации


typedef enum
{
	CC1120_TX_STATE_STANDBY,
	CC1120_TX_STATE_WAIT,
	CC1120_TX_STATE_ACTIVE
} CC1120TxStates_t;

typedef struct
{
	SPI_HandleTypeDef *hSPI;
	CC1120TxStates_t TxState;
} CC1120_TypeDef;	


	 
#define EXT_ADDRESS						0x2F  			/* EXTENDED ADDRESS fields */			 
#define REG_ADDRESS						0x00				/* Register space */
#define	REG_DMA								0x3E				/* Direct FIFO Access */	 
	 
#define READ_CC1120						0x01				/* Read register*/
#define WRITE_CC1120					0x00				/* Write register */	 

#define BURST									0x01				/* Burst mode */
#define NO_BURST							0x00				/* Single mode */

#define TX_FIFO_FAIL					0xFF				/* Ошибка чтения данных FIFO_TX */
#define RX_FIFO_FAIL					0xFF				/* Ошибка чтения данных FIFO_RX */
	 

#define EXT_PARTNUMBER				0x8F  			/* PARTNUMBER */
#define EXT_PARTVERSION				0x90  			/* PARTVERSION */
#define EXT_NUM_TXBYTES				0xD6  			/* Количество байтов в FIFO TX */
#define EXT_NUM_RXBYTES				0xD7				/* Количество байтов в FIFO RX */
#define EXT_FS_VCO2						0x25				/* FS_VCO2 */
#define EXT_FS_CAL2						0x15				/* FS_CAL2 */
#define EXT_FS_VCO4						0x23				/* FS_VCO_4 */
#define EXT_FS_CHP						0x18				/* FS_CHP */
#define	EXT_FREQ2							0x0C				/* Frequency configuration. FREQ2 (23_16), FREQ1 (15_8), FREQ0 (7_0) */
#define EXT_RSSI1							0x71				/* Значение индикатора уровня приема, старшие 8 бит (RSSI[11:4]) */
#define EXT_RSSI0							0x72				/* Значение индикатора уровня приема, младшие 4 бита (RSSI[3:0]) + признак несущей (CARRIER_SENSE) */
#define EXT_MARCSTATE					0x73				/* Опрос состояния трансивера */
#define EXT_AGC_GAIN3					0x79				/* Значение усиления АРУ фронт-энда микросхемы, разрешение - 1 дБ */
#define EXT_FSCAL_CTRL				0x8D				/* запрос Lock detect */

#define S_STATUS							0x3D				/* No operation. May be used to get access to the chip status byte */
#define S_TX									0x35				/* Enable Tx */	 
#define S_RX									0x34				/* Enable Rx */
#define S_IDLE								0x36				/* IDLE */
#define S_RESET								0x30				/* Сброс трансивера */
#define	S_TX_FIFO_FLUSH				0x3B				/* Очистка FIFO TX */
#define	S_RX_FIFO_FLUSH				0x3A				/* Очистка FIFO TX */
#define S_CAL									0x33				/* Запуск калибровки синтезатора */
#define S_SFSTXON							0x31				/* запуск автоматической калибровки синтезатора */
#define R_ST_FIFO_ACCESS			0x3F				/* Стандартная доступ к  Tx/Rx FIFO */

#define PA_CFG2 							0x2B				/* Power Amplifier Configuration Reg. 2 */


#define CC1120_ID							0x48				/* Chip ID CC1120 */ 
#define LO_DIVIDER						0x08				/* Делитель опорной частоты */
#define	F_XOSC								0x1E84800		/* Частота опорного генератора 32 МГц */


#define RSSI_VALID_MASK_IN_RSSI0_REG 		0x01 				/* Маска поля RSSI_VALID в регистре RSSI0 */


#define CC1120_RSSI_OFFSET 	(102)

#ifdef DEBUG_SET_AGC_GAIN_ADJUST
//Если в регистр AGC_GAIN_ADJUST записать нулевое значение, то значения RSSI, которые выдает приемник, будут отличаться 
//от реальных значений уровня ~на 102 дБ. Но при записи значения 102 в этот регистр приемник выдает неадекватные значения
//RSSI при низких уровнях (<-115дБм). Поэтому лучше вычитать это значение "вручную"
#define CC1120_AGC_GAIN_ADJUST	(CC1120_RSSI_OFFSET}
#else
#define CC1120_AGC_GAIN_ADJUST	(0x00)
#endif


/* TX power = 15 */
/* Whitening = true */
/* Device address = 0 */
/* Packet length mode = Fixed */
/* Deviation = 1.197815 */
/* Modulation format = 2-GFSK */
/* Address config = Address check, 0x00 and 0xFF broadcast */
/* Bit rate = 4.8 */
/* RX filter BW = 10.000000 */
/* PA ramping = true */
/* Packet bit length = 0 */
/* Performance mode = High Performance */
/* Carrier frequency = 434.000000 */
/* Manchester enable = false */
/* Packet length = 90 */
/* Symbol rate = 4.8 */
/* RF settings for CC1120 */
 
static const CC1120regSetting_t CC1120_Config_4800[]= {
{0x0000,     0xB0},     //IOCFG3             GPIO3 IO PIN CONFIGURATION
{0x0001,     0x06},     //IOCFG2             GPIO2 IO PIN CONFIGURATION
{0x0002,     0xB0},     //IOCFG1             GPIO1 IO PIN CONFIGURATION
{0x0003,     0x40},     //IOCFG0             GPIO0 IO PIN CONFIGURATION
{0x0008,     0x0B},     //SYNC_CFG1          SYNC WORD DETECTION CONFIGURATION REG. 1
{0x000A,     0x3A},     //DEVIATION_M        FREQUENCY DEVIATION CONFIGURATION
{0x000B,     0x09},     //MODCFG_DEV_E       MODULATION FORMAT AND FREQUENCY DEVIATION CONFIGUR..
{0x000C,     0x1C},     //DCFILT_CFG         DIGITAL DC REMOVAL CONFIGURATION
{0x000D,     0x2C},     //PREAMBLE_CFG1      PREAMBLE LENGTH CONFIGURATION REG. 1
#ifdef DEBUG_HIGH_IF
{0x000F,     0x78},     //FREQ_IF_CFG 			 RX Mixer Frequency Configuration	//Высокая ПЧ ~120 кГц
#endif
#ifdef DEBUG_ZERO_IF
{0x000F,     0x00},     //FREQ_IF_CFG 			 RX Mixer Frequency Configuration	//Нулевая ПЧ
#endif
{0x0010,     0xC6},     //IQIC               DIGITAL IMAGE CHANNEL COMPENSATION CONFIGURATION
{0x0013,     0x05},     //MDMCFG0            GENERAL MODEM PARAMETER CONFIGURATION REG. 0
{0x0014,     0x63},     //SYMBOL_RATE2       SYMBOL RATE CONFIGURATION EXPONENT AND MANTISSA [1..
{0x0017,     0x20},     //AGC_REF            AGC REFERENCE LEVEL CONFIGURATION
{0x0018,     0x19},     //AGC_CS_THR         CARRIER SENSE THRESHOLD CONFIGURATION
{0x0019,     CC1120_AGC_GAIN_ADJUST},     					//AGC_GAIN_ADJUST    RSSI Offset Configuration
{0x001C,     0xA9},     //AGC_CFG1           AUTOMATIC GAIN CONTROL CONFIGURATION REG. 1
{0x001D,     0xCF},     //AGC_CFG0           AUTOMATIC GAIN CONTROL CONFIGURATION REG. 0
{0x001E,     0x00},     //FIFO_CFG           FIFO CONFIGURATION
{0x0020,     0x0B},     //SETTLING_CFG       FREQUENCY SYNTHESIZER CALIBRATION AND SETTLING CON..   !!!!!!! Для калибровки, после изменения частоты должно быть 0x0B
{0x0021,     0x14},     //FS_CFG             FREQUENCY SYNTHESIZER CONFIGURATION
{0x0027,     0x75},     //PKT_CFG1           PACKET CONFIGURATION REG. 1
#ifdef DEBUG_CC1120_VARIABLE_PACKET_LENGTH
{0x0028,     0x20},     //PKT_CFG0           PACKET CONFIGURATION REG. 0
#endif
{0x002B,     CC1120_DEFAULT_PA_POWER_RAMP | 0x40},	//PA_CFG2            POWER AMPLIFIER CONFIGURATION REG. 2
{0x002D,     0x7E},     //PA_CFG0            POWER AMPLIFIER CONFIGURATION REG. 0
#ifndef DEBUG_CC1120_VARIABLE_PACKET_LENGTH
{0x002E,     RADIOPACK_MODE4800_EXTSIZE},     //PKT_LEN            PACKET LENGTH CONFIGURATION
#else
{0x002E,     MAX_RADIOPACK_SIZE},     				//PKT_LEN            PACKET LENGTH CONFIGURATION
#endif
{0x2F00,     0x00},     //IF_MIX_CFG         IF MIX CONFIGURATION
{0x2F01,     0x22},     //FREQOFF_CFG        FREQUENCY OFFSET CORRECTION CONFIGURATION
{0x2F0C,     0x6C},     //FREQ2              FREQUENCY CONFIGURATION [23:16]
{0x2F0D,     0x80},     //FREQ1              FREQUENCY CONFIGURATION [15:8]
{0x2F12,     0x00},     //FS_DIG1            FREQUENCY SYNTHESIZER DIGITAL REG. 1
{0x2F13,     0x5F},     //FS_DIG0            FREQUENCY SYNTHESIZER DIGITAL REG. 0
{0x2F16,     0x40},     //FS_CAL1            FREQUENCY SYNTHESIZER CALIBRATION REG. 1
{0x2F17,     0x0E},     //FS_CAL0            FREQUENCY SYNTHESIZER CALIBRATION REG. 0
{0x2F19,     0x03},     //FS_DIVTWO          FREQUENCY SYNTHESIZER DIVIDE BY 2
{0x2F1B,     0x33},     //FS_DSM0            FS DIGITAL SYNTHESIZER MODULE CONFIGURATION REG. 0
{0x2F1D,     0x17},     //FS_DVC0            FREQUENCY SYNTHESIZER DIVIDER CHAIN CONFIGURATION ..
{0x2F1F,     0x50},     //FS_PFD             FREQUENCY SYNTHESIZER PHASE FREQUENCY DETECTOR CON..
{0x2F20,     0x6E},     //FS_PRE             FREQUENCY SYNTHESIZER PRESCALER CONFIGURATION
{0x2F21,     0x14},     //FS_REG_DIV_CML     FREQUENCY SYNTHESIZER DIVIDER REGULATOR CONFIGURAT..
{0x2F22,     0xAC},     //FS_SPARE           FREQUENCY SYNTHESIZER SPARE
{0x2F27,     0xB4},     //FS_VCO0            FS VOLTAGE CONTROLLED OSCILLATOR CONFIGURATION REG..
{0x2F32,     0x0E},     //XOSC5              CRYSTAL OSCILLATOR CONFIGURATION REG. 5
{0x2F36,     0x03},     //XOSC1              CRYSTAL OSCILLATOR CONFIGURATION REG. 1
{0x2F8F,     0x48},     //PARTNUMBER         PART NUMBER
{0x2F90,     0x21},     //PARTVERSION        PART REVISION
{0x2F92,     0x10},     //MODEM_STATUS1      MODEM STATUS REG. 1
};


/* TX power = 15 */
/* Whitening = true */
/* Device address = 0 */
/* Packet length mode = Fixed */
/* Deviation = 2.395630 */
/* Modulation format = 2-GFSK */
/* Address config = Address check, 0x00 and 0xFF broadcast */
/* Bit rate = 9.6 */
/* RX filter BW = 20.000000 */
/* PA ramping = true */
/* Packet bit length = 0 */
/* Performance mode = High Performance */
/* Carrier frequency = 434.000000 */
/* Manchester enable = false */
/* Packet length = 90 */
/* Symbol rate = 9.6 */
/* RF settings for CC1120 */

static const CC1120regSetting_t CC1120_Config_9600[]= {
{0x0000,     0xB0},     //IOCFG3             GPIO3 IO PIN CONFIGURATION
{0x0001,     0x06},     //IOCFG2             GPIO2 IO PIN CONFIGURATION
{0x0002,     0xB0},     //IOCFG1             GPIO1 IO PIN CONFIGURATION
{0x0003,     0x40},     //IOCFG0             GPIO0 IO PIN CONFIGURATION
{0x0008,     0x0B},     //SYNC_CFG1          SYNC WORD DETECTION CONFIGURATION REG. 1
{0x000A,     0x3A},     //DEVIATION_M        FREQUENCY DEVIATION CONFIGURATION
{0x000B,     0x0A},     //MODCFG_DEV_E       MODULATION FORMAT AND FREQUENCY DEVIATION CONFIGUR..
{0x000C,     0x1C},     //DCFILT_CFG         DIGITAL DC REMOVAL CONFIGURATION
{0x000D,     0x2C},     //PREAMBLE_CFG1      PREAMBLE LENGTH CONFIGURATION REG. 1
{0x0010,     0xC6},     //IQIC               DIGITAL IMAGE CHANNEL COMPENSATION CONFIGURATION
{0x0011,     0x0A},     //CHAN_BW            CHANNEL FILTER CONFIGURATION
{0x0013,     0x05},     //MDMCFG0            GENERAL MODEM PARAMETER CONFIGURATION REG. 0
{0x0014,     0x73},     //SYMBOL_RATE2       SYMBOL RATE CONFIGURATION EXPONENT AND MANTISSA [1..
{0x0017,     0x20},     //AGC_REF            AGC REFERENCE LEVEL CONFIGURATION
{0x0018,     0x19},     //AGC_CS_THR         CARRIER SENSE THRESHOLD CONFIGURATION
{0x0019,     CC1120_AGC_GAIN_ADJUST},     					//AGC_GAIN_ADJUST    RSSI Offset Configuration
{0x001C,     0xA9},     //AGC_CFG1           AUTOMATIC GAIN CONTROL CONFIGURATION REG. 1
{0x001D,     0xCF},     //AGC_CFG0           AUTOMATIC GAIN CONTROL CONFIGURATION REG. 0
{0x001E,     0x00},     //FIFO_CFG           FIFO CONFIGURATION
{0x0020,     0x03},     //SETTLING_CFG       FREQUENCY SYNTHESIZER CALIBRATION AND SETTLING CON..
{0x0021,     0x14},     //FS_CFG             FREQUENCY SYNTHESIZER CONFIGURATION
{0x0027,     0x75},     //PKT_CFG1           PACKET CONFIGURATION REG. 1
{0x002B,     CC1120_DEFAULT_PA_POWER_RAMP | 0x40},	//PA_CFG2            POWER AMPLIFIER CONFIGURATION REG. 2
{0x002D,     0x7D},     //PA_CFG0            POWER AMPLIFIER CONFIGURATION REG. 0
{0x002E,     RADIOPACK_MODE9600_EXTSIZE},     			//PKT_LEN            PACKET LENGTH CONFIGURATION
{0x2F00,     0x00},     //IF_MIX_CFG         IF MIX CONFIGURATION
{0x2F01,     0x22},     //FREQOFF_CFG        FREQUENCY OFFSET CORRECTION CONFIGURATION
{0x2F0C,     0x6C},     //FREQ2              FREQUENCY CONFIGURATION [23:16]
{0x2F0D,     0x80},     //FREQ1              FREQUENCY CONFIGURATION [15:8]
{0x2F12,     0x00},     //FS_DIG1            FREQUENCY SYNTHESIZER DIGITAL REG. 1
{0x2F13,     0x5F},     //FS_DIG0            FREQUENCY SYNTHESIZER DIGITAL REG. 0
{0x2F16,     0x40},     //FS_CAL1            FREQUENCY SYNTHESIZER CALIBRATION REG. 1
{0x2F17,     0x0E},     //FS_CAL0            FREQUENCY SYNTHESIZER CALIBRATION REG. 0
{0x2F19,     0x03},     //FS_DIVTWO          FREQUENCY SYNTHESIZER DIVIDE BY 2
{0x2F1B,     0x33},     //FS_DSM0            FS DIGITAL SYNTHESIZER MODULE CONFIGURATION REG. 0
{0x2F1D,     0x17},     //FS_DVC0            FREQUENCY SYNTHESIZER DIVIDER CHAIN CONFIGURATION ..
{0x2F1F,     0x50},     //FS_PFD             FREQUENCY SYNTHESIZER PHASE FREQUENCY DETECTOR CON..
{0x2F20,     0x6E},     //FS_PRE             FREQUENCY SYNTHESIZER PRESCALER CONFIGURATION
{0x2F21,     0x14},     //FS_REG_DIV_CML     FREQUENCY SYNTHESIZER DIVIDER REGULATOR CONFIGURAT..
{0x2F22,     0xAC},     //FS_SPARE           FREQUENCY SYNTHESIZER SPARE
{0x2F27,     0xB4},     //FS_VCO0            FS VOLTAGE CONTROLLED OSCILLATOR CONFIGURATION REG..
{0x2F32,     0x0E},     //XOSC5              CRYSTAL OSCILLATOR CONFIGURATION REG. 5
{0x2F36,     0x03},     //XOSC1              CRYSTAL OSCILLATOR CONFIGURATION REG. 1
{0x2F8F,     0x48},     //PARTNUMBER         PART NUMBER
{0x2F90,     0x21},     //PARTVERSION        PART REVISION
{0x2F92,     0x10},     //MODEM_STATUS1      MODEM STATUS REG. 1
};



/* TX power = 15 */
/* Whitening = true */
/* Device address = 0 */
/* Packet length mode = Fixed */
/* Deviation = 4.791260 */
/* Modulation format = 2-GFSK */
/* Address config = Address check, 0x00 and 0xFF broadcast */
/* Bit rate = 19.2 */
/* RX filter BW = 40.000000 */
/* PA ramping = true */
/* Packet bit length = 0 */
/* Performance mode = High Performance */
/* Carrier frequency = 434.000000 */
/* Manchester enable = false */
/* Packet length = 90 */
/* Symbol rate = 19.2 */
/* RF settings for CC1120 */

static const CC1120regSetting_t CC1120_Config_19200[]= {
{0x0000,     0xB0},     //IOCFG3             GPIO3 IO PIN CONFIGURATION
{0x0001,     0x06},     //IOCFG2             GPIO2 IO PIN CONFIGURATION
{0x0002,     0xB0},     //IOCFG1             GPIO1 IO PIN CONFIGURATION
{0x0003,     0x40},     //IOCFG0             GPIO0 IO PIN CONFIGURATION
{0x0008,     0x0B},     //SYNC_CFG1          SYNC WORD DETECTION CONFIGURATION REG. 1
{0x000A,     0x3A},     //DEVIATION_M        FREQUENCY DEVIATION CONFIGURATION
{0x000B,     0x0B},     //MODCFG_DEV_E       MODULATION FORMAT AND FREQUENCY DEVIATION CONFIGUR..
{0x000C,     0x1C},     //DCFILT_CFG         DIGITAL DC REMOVAL CONFIGURATION
{0x000D,     0x2C},     //PREAMBLE_CFG1      PREAMBLE LENGTH CONFIGURATION REG. 1
{0x0010,     0x46},     //IQIC               DIGITAL IMAGE CHANNEL COMPENSATION CONFIGURATION
{0x0011,     0x05},     //CHAN_BW            CHANNEL FILTER CONFIGURATION
{0x0013,     0x05},     //MDMCFG0            GENERAL MODEM PARAMETER CONFIGURATION REG. 0
{0x0014,     0x83},     //SYMBOL_RATE2       SYMBOL RATE CONFIGURATION EXPONENT AND MANTISSA [1..
{0x0017,     0x20},     //AGC_REF            AGC REFERENCE LEVEL CONFIGURATION
{0x0018,     0x19},     //AGC_CS_THR         CARRIER SENSE THRESHOLD CONFIGURATION
{0x0019,     CC1120_AGC_GAIN_ADJUST},     					//AGC_GAIN_ADJUST    RSSI Offset Configuration
{0x001C,     0xA9},     //AGC_CFG1           AUTOMATIC GAIN CONTROL CONFIGURATION REG. 1
{0x001D,     0xCF},     //AGC_CFG0           AUTOMATIC GAIN CONTROL CONFIGURATION REG. 0
{0x001E,     0x00},     //FIFO_CFG           FIFO CONFIGURATION
{0x0020,     0x03},     //SETTLING_CFG       FREQUENCY SYNTHESIZER CALIBRATION AND SETTLING CON..
{0x0021,     0x14},     //FS_CFG             FREQUENCY SYNTHESIZER CONFIGURATION
{0x0027,     0x75},     //PKT_CFG1           PACKET CONFIGURATION REG. 1
{0x002B,     CC1120_DEFAULT_PA_POWER_RAMP | 0x40},	//PA_CFG2            POWER AMPLIFIER CONFIGURATION REG. 2
{0x002E,     RADIOPACK_MODE19200_EXTSIZE},     			//PKT_LEN            PACKET LENGTH CONFIGURATION
{0x2F00,     0x00},     //IF_MIX_CFG         IF MIX CONFIGURATION
{0x2F01,     0x22},     //FREQOFF_CFG        FREQUENCY OFFSET CORRECTION CONFIGURATION
{0x2F0C,     0x6C},     //FREQ2              FREQUENCY CONFIGURATION [23:16]
{0x2F0D,     0x80},     //FREQ1              FREQUENCY CONFIGURATION [15:8]
{0x2F12,     0x00},     //FS_DIG1            FREQUENCY SYNTHESIZER DIGITAL REG. 1
{0x2F13,     0x5F},     //FS_DIG0            FREQUENCY SYNTHESIZER DIGITAL REG. 0
{0x2F16,     0x40},     //FS_CAL1            FREQUENCY SYNTHESIZER CALIBRATION REG. 1
{0x2F17,     0x0E},     //FS_CAL0            FREQUENCY SYNTHESIZER CALIBRATION REG. 0
{0x2F19,     0x03},     //FS_DIVTWO          FREQUENCY SYNTHESIZER DIVIDE BY 2
{0x2F1B,     0x33},     //FS_DSM0            FS DIGITAL SYNTHESIZER MODULE CONFIGURATION REG. 0
{0x2F1D,     0x17},     //FS_DVC0            FREQUENCY SYNTHESIZER DIVIDER CHAIN CONFIGURATION ..
{0x2F1F,     0x50},     //FS_PFD             FREQUENCY SYNTHESIZER PHASE FREQUENCY DETECTOR CON..
{0x2F20,     0x6E},     //FS_PRE             FREQUENCY SYNTHESIZER PRESCALER CONFIGURATION
{0x2F21,     0x14},     //FS_REG_DIV_CML     FREQUENCY SYNTHESIZER DIVIDER REGULATOR CONFIGURAT..
{0x2F22,     0xAC},     //FS_SPARE           FREQUENCY SYNTHESIZER SPARE
{0x2F27,     0xB4},     //FS_VCO0            FS VOLTAGE CONTROLLED OSCILLATOR CONFIGURATION REG..
{0x2F32,     0x0E},     //XOSC5              CRYSTAL OSCILLATOR CONFIGURATION REG. 5
{0x2F36,     0x03},     //XOSC1              CRYSTAL OSCILLATOR CONFIGURATION REG. 1
{0x2F8F,     0x48},     //PARTNUMBER         PART NUMBER
{0x2F90,     0x21},     //PARTVERSION        PART REVISION
{0x2F92,     0x10},     //MODEM_STATUS1      MODEM STATUS REG. 1
};


/* TX power = 15 */
/* Whitening = true */
/* Device address = 0 */
/* Packet length mode = Fixed */
/* Deviation = 11.993408 */
/* Modulation format = 4-GFSK */
/* Address config = Address check, 0x00 and 0xFF broadcast */
/* Bit rate = 48 */
/* RX filter BW = 50.000000 */
/* PA ramping = true */
/* Packet bit length = 0 */
/* Performance mode = High Performance */
/* Carrier frequency = 434.000000 */
/* Manchester enable = false */
/* Packet length = 90 */
/* Symbol rate = 24 */
/* RF settings for CC1120 */

static const CC1120regSetting_t CC1120_Config_48000[]= {
{0x0000,     0xB0},     //IOCFG3             GPIO3 IO PIN CONFIGURATION
{0x0001,     0x06},     //IOCFG2             GPIO2 IO PIN CONFIGURATION
{0x0002,     0xB0},     //IOCFG1             GPIO1 IO PIN CONFIGURATION
{0x0003,     0x40},     //IOCFG0             GPIO0 IO PIN CONFIGURATION
{0x0008,     0x08},     //SYNC_CFG1          SYNC WORD DETECTION CONFIGURATION REG. 1
{0x000A,     0x89},     //DEVIATION_M        FREQUENCY DEVIATION CONFIGURATION
{0x000B,     0x2C},     //MODCFG_DEV_E       MODULATION FORMAT AND FREQUENCY DEVIATION CONFIGUR..
{0x000C,     0x1C},     //DCFILT_CFG         DIGITAL DC REMOVAL CONFIGURATION
{0x000D,     0x2C},     //PREAMBLE_CFG1      PREAMBLE LENGTH CONFIGURATION REG. 1
{0x0010,     0x46},     //IQIC               DIGITAL IMAGE CHANNEL COMPENSATION CONFIGURATION
{0x0011,     0x04},     //CHAN_BW            CHANNEL FILTER CONFIGURATION
{0x0013,     0x05},     //MDMCFG0            GENERAL MODEM PARAMETER CONFIGURATION REG. 0
{0x0014,     0x88},     //SYMBOL_RATE2       SYMBOL RATE CONFIGURATION EXPONENT AND MANTISSA [1..
{0x0015,     0x93},     //SYMBOL_RATE1       SYMBOL RATE CONFIGURATION MANTISSA [15:8]
{0x0016,     0x75},     //SYMBOL_RATE0       SYMBOL RATE CONFIGURATION MANTISSA [7:0]
{0x0017,     0x20},     //AGC_REF            AGC REFERENCE LEVEL CONFIGURATION
{0x0018,     0x19},     //AGC_CS_THR         CARRIER SENSE THRESHOLD CONFIGURATION
{0x0019,     CC1120_AGC_GAIN_ADJUST},     					//AGC_GAIN_ADJUST    RSSI Offset Configuration
{0x001C,     0xA9},     //AGC_CFG1           AUTOMATIC GAIN CONTROL CONFIGURATION REG. 1
{0x001D,     0xCF},     //AGC_CFG0           AUTOMATIC GAIN CONTROL CONFIGURATION REG. 0
{0x001E,     0x00},     //FIFO_CFG           FIFO CONFIGURATION
{0x0020,     0x03},     //SETTLING_CFG       FREQUENCY SYNTHESIZER CALIBRATION AND SETTLING CON..
{0x0021,     0x14},     //FS_CFG             FREQUENCY SYNTHESIZER CONFIGURATION
{0x0027,     0x75},     //PKT_CFG1           PACKET CONFIGURATION REG. 1
{0x002B,     CC1120_DEFAULT_PA_POWER_RAMP | 0x40},	//PA_CFG2            POWER AMPLIFIER CONFIGURATION REG. 2
{0x002E,     RADIOPACK_MODE48000_EXTSIZE},     			//PKT_LEN            PACKET LENGTH CONFIGURATION
{0x2F00,     0x00},     //IF_MIX_CFG         IF MIX CONFIGURATION
{0x2F01,     0x22},     //FREQOFF_CFG        FREQUENCY OFFSET CORRECTION CONFIGURATION
{0x2F0C,     0x6C},     //FREQ2              FREQUENCY CONFIGURATION [23:16]
{0x2F0D,     0x80},     //FREQ1              FREQUENCY CONFIGURATION [15:8]
{0x2F12,     0x00},     //FS_DIG1            FREQUENCY SYNTHESIZER DIGITAL REG. 1
{0x2F13,     0x5F},     //FS_DIG0            FREQUENCY SYNTHESIZER DIGITAL REG. 0
{0x2F16,     0x40},     //FS_CAL1            FREQUENCY SYNTHESIZER CALIBRATION REG. 1
{0x2F17,     0x0E},     //FS_CAL0            FREQUENCY SYNTHESIZER CALIBRATION REG. 0
{0x2F19,     0x03},     //FS_DIVTWO          FREQUENCY SYNTHESIZER DIVIDE BY 2
{0x2F1B,     0x33},     //FS_DSM0            FS DIGITAL SYNTHESIZER MODULE CONFIGURATION REG. 0
{0x2F1D,     0x17},     //FS_DVC0            FREQUENCY SYNTHESIZER DIVIDER CHAIN CONFIGURATION ..
{0x2F1F,     0x50},     //FS_PFD             FREQUENCY SYNTHESIZER PHASE FREQUENCY DETECTOR CON..
{0x2F20,     0x6E},     //FS_PRE             FREQUENCY SYNTHESIZER PRESCALER CONFIGURATION
{0x2F21,     0x14},     //FS_REG_DIV_CML     FREQUENCY SYNTHESIZER DIVIDER REGULATOR CONFIGURAT..
{0x2F22,     0xAC},     //FS_SPARE           FREQUENCY SYNTHESIZER SPARE
{0x2F27,     0xB4},     //FS_VCO0            FS VOLTAGE CONTROLLED OSCILLATOR CONFIGURATION REG..
{0x2F32,     0x0E},     //XOSC5              CRYSTAL OSCILLATOR CONFIGURATION REG. 5
{0x2F36,     0x03},     //XOSC1              CRYSTAL OSCILLATOR CONFIGURATION REG. 1
{0x2F8F,     0x48},     //PARTNUMBER         PART NUMBER
{0x2F90,     0x21},     //PARTVERSION        PART REVISION
{0x2F92,     0x10},     //MODEM_STATUS1      MODEM STATUS REG. 1
};


ReadWriteRegTypeDef CC1120_Write (uint8_t uGenAddress, uint8_t uExtAddress, uint8_t bBurst, uint8_t *data_ptr, uint16_t uAccesses);
ReadWriteRegTypeDef CC1120_Read (uint8_t uGenAddress, uint8_t uExtAddress, uint8_t bBurst, uint8_t *data_ptr, uint8_t uAccesses);

uint16_t CC1120_Init(CC1120_TypeDef *pCC1120, SPI_HandleTypeDef *hspi);

uint16_t CC1120_SetConfig(SPI_HandleTypeDef *hspi, const CC1120regSetting_t *CC1120_Config, uint8_t configRegNum);

uint16_t CC1120_TxData(CC1120_TypeDef *pCC1120, uint8_t* pDataBuf, uint16_t sizeBuf);
uint16_t CC1120_RxData(CC1120_TypeDef *pCC1120, uint8_t* pDataBuf, uint16_t* sizeBuf);

uint8_t CC1120_CheckModule(SPI_HandleTypeDef *hspi);
uint8_t CC1120_CheckVersion(SPI_HandleTypeDef *hspi);

CC1120STATUSTypeDef CC1120_Status(SPI_HandleTypeDef *hspi);

int8_t CC1120_CheckRSSI(SPI_HandleTypeDef *hspi);
int8_t CC1120_CheckAGCGain(SPI_HandleTypeDef *hspi);

uint8_t CC1120_Tx(SPI_HandleTypeDef *hspi);
uint8_t CC1120_IDLE_set(SPI_HandleTypeDef *hspi);
uint8_t CC1120_Rx(SPI_HandleTypeDef *hspi);

uint8_t CC1120_Reset(SPI_HandleTypeDef *hspi);

uint8_t CC1120_TxFIFONumBytes(SPI_HandleTypeDef *hspi);
uint8_t CC1120_TxFIFOFlush(SPI_HandleTypeDef *hspi);
uint8_t CC1120_TxFIFOWrite(SPI_HandleTypeDef *hspi, uint8_t *fifo_write_data_ptr, uint8_t tx_num);

uint8_t CC1120_ManualCalibration(SPI_HandleTypeDef *hspi);

CC1120MARCSTATETypeDef CC1120_MARCState(SPI_HandleTypeDef *hspi);

uint8_t CC1120_SFSTXON_set (SPI_HandleTypeDef *hspi);

uint8_t CC1120_RxFIFONumBytes(SPI_HandleTypeDef *hspi);
uint8_t CC1120_RxFIFOFlush(SPI_HandleTypeDef *hspi);
uint8_t *CC1120_RxFIFORead(SPI_HandleTypeDef *hspi);

uint8_t CC1120_ConfigWrite(SPI_HandleTypeDef *hspi, const CC1120regSetting_t *CC1120_Config, uint8_t configRegNum);
uint8_t CC1120_ConfigReadCompare(SPI_HandleTypeDef *hspi, const CC1120regSetting_t *CC1120_Config, uint8_t configRegNum);

uint8_t CC1120_FreqWrite(SPI_HandleTypeDef *hspi, uint8_t *freq);
uint8_t *CC1120_FreqRead(SPI_HandleTypeDef *hspi);

uint8_t CC1120_PowerAmpWrite(SPI_HandleTypeDef *hspi, uint8_t nPAPowRamp);

void CC1120_HardwareReset(void);
	 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SPI_CC1120_H */
