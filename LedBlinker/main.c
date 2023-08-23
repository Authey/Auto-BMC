/** === HEADER ====================================================================================
 */

#include <Rte.h>

#include <Os.h>
#if ((OS_AR_RELEASE_MAJOR_VERSION != RTE_AR_RELEASE_MAJOR_VERSION) || (OS_AR_RELEASE_MINOR_VERSION != RTE_AR_RELEASE_MINOR_VERSION))
#error Os version mismatch
#endif

#include <Com.h>
#if ((COM_AR_RELEASE_MAJOR_VERSION != RTE_AR_RELEASE_MAJOR_VERSION) || (COM_AR_RELEASE_MINOR_VERSION != RTE_AR_RELEASE_MINOR_VERSION))
#error Com version mismatch
#endif

#include <Rte_Hook.h>
#include <Rte_Internal.h>
#include <Rte_Calprms.h>

#include "Rte_LedBlinkerSWC.h"
#include "Rte_LedBlinkerSWC.h"
#include "debug.h"
#include <Rte_DataHandleType.h>
#include <Rte_LedBlinkerSWC_Type.h>

#define LedBlinkerSWC_START_SEC_CONST_UNSPECIFIED
#define LedBlinkerSWC_START_SEC_CODE
#include <LedBlinkerSWC_MemMap.h> /*lint !e415 Autosar specified way to group code into memory sections, Req SWS_MemMap_00003 */
#define Rte_START_SEC_CODE
#include <Rte_MemMap.h> /*lint !e415 Autosar specified way to group code into memory sections, Req SWS_MemMap_00003 */
#define IoHwAb_START_SEC_CODE
#include <IoHwAb_MemMap.h> /*lint !e415 Autosar specified way to group code into memory sections, Req SWS_MemMap_00003 */

#include "IoHwAb.h"
#include "IoHwAb_Internal.h"
#include "IoHwAb_Digital.h"
#include "IoHwAb_Dcm.h"
#include "SchM_IoHwAb.h"
#if defined(USE_DIO)
#include "Dio.h"
#else
#error "DIO Module is needed by IOHWAB"
#endif

#include "Dio_MemMap.h"
#include "Dio_Internal.h"
#include "SchM_Dio.h"

#if defined(USE_DET)
#include "Det.h"
#endif
#include "Dio_Hw_tms570.h"

#define IS_VALID_DIO_LEVEL(_x) ((STD_LOW == (_x)) || (STD_HIGH == (_x)))

#define IOHWAB_START_SEC_VAR_INIT_UNSPECIFIED
#include "IoHwAb_BswMemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/

#define DIO_SW_MAJOR_VERSION_INT   5u /**< @brief Dio Module Software Major Version Number for other architectures */
#define DIO_SW_MINOR_VERSION_INT   0u /**< @brief Dio Module Software Minor Version Number for other architectures */
#define DIO_SW_PATCH_VERSION_INT   0u /**< @brief Dio Module Software Patch Version Number for other architectures */

#define DIO_AR_RELEASE_MAJOR_VERSION_INT   4u /**< @brief Dio Module Autosar Major Version Number  */
#define DIO_AR_RELEASE_MINOR_VERSION_INT   1u /**< @brief Dio Module Autosar Minor Version Number  */
#define DIO_AR_RELEASE_PATCH_VERSION_INT   2u /**< @brief Dio Module Autosar Patch Version Number  */

boolean IoHwAb_LED1_Locked = FALSE;
IoHwAb_LevelType IoHwAb_LED1_Saved = IOHWAB_LOW;

const IoHwAb_LevelType IoHwAb_LED1_Default = IOHWAB_LOW;

static DigitalLevel state = 1;

const Rte_CDS_LedBlinkerSWC LedBlinkerSWC_ledBlinkerSWC = {
    ._dummy = 0
};

const Rte_Instance Rte_Inst_LedBlinkerSWC = &LedBlinkerSWC_ledBlinkerSWC;

Dio_GlobalType DioGlobal = {.InitRun = FALSE, .Config = NULL_PTR};

GIO_RegisterType *GPIO_ports1[] = { GIO_PORTA_BASE, GIO_PORTB_BASE, GIO_PORTC_BASE };

/*lint -e843 LINT:OTHER:HARDWARE DEFINITION */
Adc_GioRegisterType *Adc_Dio = ADEVT_GIO_BASE;

/**
 * @brief   spi5Collection - Pointer that holds address of registers of SPI module
 *
 */
const GIO_SpiRegisterCollectionType spi5Collection = {
    .dirReg = (uint32_t*)DIRECTION_REG,  /**< @brief Data Direction Register */
    .inReg  = (uint32_t*)DATA_IN_REG,    /**< @brief Data Input Register */
    .outReg = (uint32_t*)DATA_OUT_REG,   /**< @brief Data Output Register */
};

static uint8 getPortFromChannel(uint8 channel){
	uint8 rv_port;

	if(channel == ADC_CHANNEL_NUMBER){
		rv_port = ADC_GIO_PORT_INDEX;
	}else if (channel >= SPI5_FIRST_CHANNEL){
		rv_port =  SPI5_GIO_PORT_INDEX;
	}else if (channel < GIO_CHANNEL) {
		rv_port = channel / CHANNEL_TO_PORT;
	}else{
		rv_port = NHET_GIO_PORT_INDEX;
	}
    return rv_port ;
}

/**
 * @brief This function is to get the Bit value from the channel
 * @param[in] channel - Channel from which the Bit value should be calculated
 * @return uint32 - Bit value from the channel
 * @retval 1.. Number of bits in the channel
 */
static uint32 getBitFromChannel (uint8 channel){
	uint32 rv_bit;
	if(channel == ADC_CHANNEL_NUMBER){
		rv_bit = ((uint32)1 << 0);
	}else if (channel >= SPI5_FIRST_CHANNEL){
		rv_bit = ((uint32)1 << ((channel - SPI5_FIRST_CHANNEL) % SPI_BIT));
	} else if (channel < GIO_CHANNEL) {
		rv_bit = ((uint32)1 << (channel % CHANNEL_TO_PORT));
	}else{
		rv_bit =  ((uint32)1 << ((channel - GIO_CHANNEL) % NHET_BIT));
	}
    return rv_bit;
}

void Dio_Hw_WriteChannel(Dio_ChannelType channelId, Dio_LevelType level){

	/**
	 * @brief Correspondence map of the 14 logical bits in the SPI5 port
	 * to actual locations of where the set/clear mask will need to
	 * be written
	 */
	static const uint8 spiBitMap[14] =
	{
	    /* Bits 0-3 correspond to CS, match logical/physical */
	    0, 1, 2, 3,
	    /* Bit 4 - ENA*/
	    8,
	    /* Bit 5 - CLK */
	    9,
	    /* Bits 6-9 are SIMO */
	    16, 17, 18, 19,
	    /* Bits 10-13 are SOMI */
	    24, 25, 26, 27
	};
    Dio_PortType port = DIO_GET_PORT_FROM_CHANNEL_ID(channelId);
    Dio_PortLevelType bit = DIO_GET_BIT_FROM_CHANNEL_ID(channelId);
    Dio_PortLevelType portVal;

    if (port == SPI5_GIO_PORT_INDEX){
        /* This port is again special. Need to look up which physical bit to write */
        uint8 physBit = spiBitMap[channelId - SPI5_FIRST_CHANNEL];
        if ((*(GIO_SPI5PORT.dirReg) & ((uint32)1 << physBit)) != 0){
        	portVal = *(GIO_SPI5PORT.outReg);
        	if (level == STD_HIGH){
        		portVal |= ((uint32)1 << physBit);
        	}else{
        	    portVal &= ~((uint32)1 << physBit);
        	}
        	*(GIO_SPI5PORT.outReg) = portVal;
        }
    }else if (port == ADC_GIO_PORT_INDEX){
    	portVal = Adc_Dio->DOUT;

    	if(level == STD_HIGH){
    		portVal |= bit;
    	} else{
        	portVal &= ~bit;
    	}

    	Adc_Dio->DOUT = portVal;
    }else{
    	if((GPIO_ports1[port]->DIR & bit) != 0){
            if (port < NHET_GIO_PORT_INDEX) {
            	portVal = GPIO_ports1[port]->DOUT;
            }else{
            	portVal = (uint32)GPIO_ports1[port]->DOUT;
            }

            if(level == STD_HIGH){
            	portVal |= bit;
            }else{
            	portVal &= ~bit;
            }
            /** @req SWS_Dio_00028 - If the specified channel is configured as an output channel, the Dio_WriteChannel function shall set the specified Level for the specified channel.*/
            GPIO_ports1[port]->DOUT = portVal;
    	}
    }
}

static boolean Channel_Config_Contains(Dio_ChannelType channelId){

    boolean rv = FALSE;
/*lint -e525 Pc Lint:CONFIGURATION: Negative indentation from line 144*/
	if (DioGlobal.Config != NULL_PTR) {
        const Dio_ChannelType* ch_ptr = (DioGlobal.Config->ChannelConfig);

        while (DIO_END_OF_LIST != (*ch_ptr)){
			 if ((*ch_ptr) == channelId){
				 rv = TRUE;
				 break;
			 }
			 ch_ptr++;
		}
    }
    return rv;
}

static inline Std_ReturnType validateDioParameter (boolean exp,uint8 apiid,uint8 errid){
    Std_ReturnType rv;
    rv = E_OK;
    if(exp == FALSE){
        rv = E_NOT_OK;

#if(DIO_DEV_ERROR_DETECT == STD_ON)
        (void)Det_ReportError(DIO_MODULE_ID, 0,apiid,errid);
#endif
    }
    return rv;
}

void Dio_WriteChannel(Dio_ChannelType channelId, Dio_LevelType level){
	Std_ReturnType validate_rv;

	validate_rv = validateDioParameter(DioGlobal.InitRun, DIO_WRITECHANNEL_ID, DIO_E_UNINIT);

    if (E_OK == validate_rv) {
        validate_rv = validateDioParameter(Channel_Config_Contains(channelId),DIO_WRITECHANNEL_ID, DIO_E_PARAM_INVALID_CHANNEL_ID);

        if (validate_rv == E_OK){
            Dio_Hw_WriteChannel(channelId,level);
        }

    }

}

Std_ReturnType IoHwAb_Digital_Write_LED1(IoHwAb_LevelType newValue)
{
	IOHWAB_VALIDATE_RETURN(IS_VALID_DIO_LEVEL((Dio_LevelType)newValue), IOHWAB_DIGITAL_WRITE_ID, IOHWAB_E_PARAM_LEVEL, E_NOT_OK);
	Dio_LevelType setLevel;
	if( TRUE == IoHwAb_LED1_Locked ) {
		setLevel = IoHwAb_LED1_Saved;
	} else {
		setLevel = newValue;
	}
	IoHwAb_LED1_Saved = setLevel;
	/* @req ARCIOHWAB004 */
	Dio_WriteChannel(DioConf_DioChannel_LED1, setLevel);
	return E_OK;
}

Std_ReturnType IoHwAb_Digital_Write(IoHwAb_SignalType signal, IoHwAb_LevelType newValue)
{
	Std_ReturnType ret = E_NOT_OK;

	switch( signal ) {
	case IOHWAB_SIGNAL_LED1:
		ret = IoHwAb_Digital_Write_LED1(newValue);
		break;
	default:
		IOHWAB_DET_REPORT_ERROR(IOHWAB_DIGITAL_WRITE_ID, IOHWAB_E_PARAM_SIGNAL);
		break;
	}
	return ret;
}

Std_ReturnType Rte_serviceIoHwAb_DigitalWrite(/*IN*/IoHwAb_SignalType_ portDefArg1, /*IN*/DigitalLevel Level) {
    Std_ReturnType retVal;

    /* PRE */

    /* MAIN */

    retVal = IoHwAb_Digital_Write(portDefArg1, Level);

    /* POST */

    return retVal;
}

Std_ReturnType Rte_Call_IoHwAb_serviceIoHwAb_Digital_LED1_Write(/*IN*/DigitalLevel Level) {
    return Rte_serviceIoHwAb_DigitalWrite(0, Level);
}

Std_ReturnType Rte_Call_LedBlinkerSWC_ledBlinkerSWC_Digital_LED1_Write(/*IN*/DigitalLevel Level) {
    return Rte_Call_IoHwAb_serviceIoHwAb_Digital_LED1_Write(Level);
}

// static inline Std_ReturnType Rte_Call_Digital_LED1_Write(/*IN*/DigitalLevel Level) {
    // return Rte_Call_LedBlinkerSWC_ledBlinkerSWC_Digital_LED1_Write(Level);
// }

void ledBlinkerSWCMain(void) {
	LDEBUG_PRINTF("!!!!!");
	Rte_Call_Digital_LED1_Write(state);

	state = !state;
}

void Rte_ledBlinkerSWC_LedBlinkerSWCMain(void) {

    /* PRE */

    /* MAIN */

    ledBlinkerSWCMain();

    /* POST */

}

int main() {
    Rte_ledBlinkerSWC_LedBlinkerSWCMain();
}

#define IOHWAB_STOP_SEC_VAR_INIT_UNSPECIFIED
#define IoHwAb_STOP_SEC_CODE
#define Rte_STOP_SEC_CODE
#define LedBlinkerSWC_STOP_SEC_CODE  
#define LedBlinkerSWC_STOP_SEC_CONST_UNSPECIFIED

