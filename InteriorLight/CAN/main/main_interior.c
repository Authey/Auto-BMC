#include "Rte_InteriorLightManager.h"

#define LIGHT_ON 	0x01u
#define LIGHT_OFF 	0x00u

typedef uint32 IntImpl;
#define ComConf_ComSignal_DoorStatus          0
#define ComConf_ComSignal_LightStatus          1
typedef struct {
    IntImpl value;
} Rte_DE_IntImpl;
Rte_DE_IntImpl ImplDE_lightManager_InteriorLightManagerMain_LightStatusOnCommMedia_message;
Rte_DE_IntImpl ImplDE_lightManager_InteriorLightManagerMain_RearDoorStatus_message;
Rte_DE_DoorStatusImpl ImplDE_lightManager_InteriorLightManagerMain_RightDoorStatus_status;
Rte_DE_DoorStatusImpl ImplDE_lightManager_InteriorLightManagerMain_LeftDoorStatus_status;
Rte_DE_LightStatusImpl ImplDE_lightManager_InteriorLightManagerMain_FrontLightStatus_status;
typedef boolean DoorStatusImpl;
DoorStatusImpl Rte_Buffer_lightManager_LeftDoorStatus_status;
DoorStatusImpl Rte_Buffer_lightManager_RightDoorStatus_status;
typedef uint32 LightStatusImpl;
LightStatusImpl Rte_Buffer_frontLightActuator_InteriorLightStatus_status;

Std_ReturnType Rte_Write_InteriorLightManager_lightManager_LightStatusOnCommMedia_message(/*IN*/IntImpl value) {
    Std_ReturnType retVal = RTE_E_OK;

    /* --- Sender (TxMessageiSignal) @req SWS_Rte_04505, @req SWS_Rte_06023 */

    retVal |= Com_SendSignal(ComConf_ComSignal_LightStatus, &value);

    return retVal;
}

Std_ReturnType Rte_Write_InteriorLightManager_lightManager_FrontLightStatus_status(/*IN*/LightStatusImpl value) {
    Std_ReturnType retVal = RTE_E_OK;

    /* --- Sender (lightManager_FrontLightStatus_to_frontLightActuator_InteriorLightStatus) */
    {
        SYS_CALL_AtomicCopy32(Rte_Buffer_frontLightActuator_InteriorLightStatus_status, value);
    }

    return retVal;
}

void interiorLightManagerMain(void) {
	IntImpl rearDoorStatus;

	//Receive the status of Rear Door
	rearDoorStatus = Rte_IRead_InteriorLightManagerMain_RearDoorStatus_message();

	// Receive the front door status
	DoorStatusImpl rightDoorStatus = Rte_IRead_InteriorLightManagerMain_RightDoorStatus_status();
	DoorStatusImpl leftDoorStatus = Rte_IRead_InteriorLightManagerMain_LeftDoorStatus_status();

	if (rightDoorStatus || leftDoorStatus || rearDoorStatus) {
		Rte_IWrite_InteriorLightManagerMain_FrontLightStatus_status(ON);
		Rte_IWrite_InteriorLightManagerMain_LightStatusOnCommMedia_message(LIGHT_ON);
	} else {
		Rte_IWrite_InteriorLightManagerMain_FrontLightStatus_status(OFF);
		Rte_IWrite_InteriorLightManagerMain_LightStatusOnCommMedia_message(LIGHT_OFF);
	}
}

Std_ReturnType Rte_Read_InteriorLightManager_lightManager_RearDoorStatus_message(/*OUT*/IntImpl * value) {
    Std_ReturnType retVal = RTE_E_OK;

    /* --- Receiver (RxMessageiSignal) @req SWS_Rte_04505, @req SWS_Rte_06023 */

    retVal |= Com_ReceiveSignal(ComConf_ComSignal_DoorStatus, value);

    return retVal;
}

/** ------ RightDoorStatus */
/*lint -e621 MISRA:OTHER:Ignore misidentified symbol clash [MISRA 2012 Rule 5.5, required]*/
Std_ReturnType Rte_Read_InteriorLightManager_lightManager_RightDoorStatus_status(/*OUT*/DoorStatusImpl * value) {
    Std_ReturnType retVal = RTE_E_OK;

    /* --- Receiver (rightDoorSensor_DoorStatus_to_lightManager_RightDoorStatus) */
    {
        SYS_CALL_AtomicCopyBoolean(*value, Rte_Buffer_lightManager_RightDoorStatus_status);
    }

    return retVal;
}

Std_ReturnType Rte_Read_InteriorLightManager_lightManager_LeftDoorStatus_status(/*OUT*/DoorStatusImpl * value) {
    Std_ReturnType retVal = RTE_E_OK;

    /* --- Receiver (leftDoorSensor_DoorStatus_to_lightManager_LeftDoorStatus) */
    {
        SYS_CALL_AtomicCopyBoolean(*value, Rte_Buffer_lightManager_LeftDoorStatus_status);
    }

    return retVal;
}

void Rte_lightManager_InteriorLightManagerMain(void) {

    /* PRE */
    Rte_Read_InteriorLightManager_lightManager_RearDoorStatus_message(&ImplDE_lightManager_InteriorLightManagerMain_RearDoorStatus_message.value); /*lint !e534 AUTOSAR API */

    Rte_Read_InteriorLightManager_lightManager_RightDoorStatus_status(&ImplDE_lightManager_InteriorLightManagerMain_RightDoorStatus_status.value); /*lint !e534 AUTOSAR API */

    Rte_Read_InteriorLightManager_lightManager_LeftDoorStatus_status(&ImplDE_lightManager_InteriorLightManagerMain_LeftDoorStatus_status.value); /*lint !e534 AUTOSAR API */

    /* MAIN */

    interiorLightManagerMain();

    /* POST */

    Rte_Write_InteriorLightManager_lightManager_FrontLightStatus_status(ImplDE_lightManager_InteriorLightManagerMain_FrontLightStatus_status.value); /*lint !e534 AUTOSAR API */

    Rte_Write_InteriorLightManager_lightManager_LightStatusOnCommMedia_message(
            ImplDE_lightManager_InteriorLightManagerMain_LightStatusOnCommMedia_message.value); /*lint !e534 AUTOSAR API */

}

int main() {
    Rte_lightManager_InteriorLightManagerMain();
}
