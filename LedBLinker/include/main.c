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

