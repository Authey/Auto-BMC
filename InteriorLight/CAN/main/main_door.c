#include "Rte_DoorSensor.h"

typedef boolean DoorStatusImpl;
DoorStatusImpl Rte_Buffer_lightManager_LeftDoorStatus_status;
typedef struct {
    DoorStatusImpl value;
} Rte_DE_DoorStatusImpl;
typedef struct {
    Std_ReturnType (*Call_Read)(/*OUT*/DigitalLevel * Level, /*OUT*/SignalQuality * Quality);
} Rte_PDS_DoorSensor_DigitalServiceRead_R;
typedef struct {
    // SWS_Rte_07138 requires the existence of a PDS in the CDS. Even if it is empty.
    uint8 _dummy;
} Rte_PDS_DoorSensor_DoorInterface_P;
Rte_DE_DoorStatusImpl ImplDE_leftDoorSensor_DoorSensorMain_DoorStatus_status;

typedef struct {
    Rte_DE_DoorStatusImpl * const DoorSensorMain_DoorStatus_status;
    Rte_PDS_DoorSensor_DigitalServiceRead_R DoorSwitchSignal;
    Rte_PDS_DoorSensor_DoorInterface_P DoorStatus;
} Rte_CDS_DoorSensor;

Std_ReturnType Rte_Call_DoorSensor_leftDoorSensor_DoorSwitchSignal_Read(/*OUT*/DigitalLevel * Level, /*OUT*/SignalQuality * Quality) {
    return Rte_Call_IoHwAb_ioHwAb_Digital_FrontDoorLeft_Read(Level, Quality);
}

const Rte_CDS_DoorSensor DoorSensor_leftDoorSensor = {
    .DoorSensorMain_DoorStatus_status = &ImplDE_leftDoorSensor_DoorSensorMain_DoorStatus_status,
    .DoorSwitchSignal = {
        .Call_Read = Rte_Call_DoorSensor_leftDoorSensor_DoorSwitchSignal_Read
    }
};

typedef struct {
    DoorStatusImpl value;
} Rte_DE_DoorStatusImpl;
Rte_DE_DoorStatusImpl ImplDE_leftDoorSensor_DoorSensorMain_DoorStatus_status;

Std_ReturnType Rte_Write_DoorSensor_leftDoorSensor_DoorStatus_status(/*IN*/DoorStatusImpl value) {
    Std_ReturnType retVal = RTE_E_OK;

    /* --- Sender (leftDoorSensor_DoorStatus_to_lightManager_LeftDoorStatus) */
    {
        SYS_CALL_AtomicCopyBoolean(Rte_Buffer_lightManager_LeftDoorStatus_status, value);
    }

    return retVal;
}

void doorSensorMain(Rte_Instance self) {
	DigitalLevel level;
	SignalQuality quality;

	// Read the level on
	(void)Rte_Call_DoorSwitchSignal_Read(self, &level, &quality);

	// Write the previously read value back to InteriorLightManager
	if (level == IOHWAB_HIGH) {
		Rte_IWrite_DoorSensorMain_DoorStatus_status(self, TRUE);
	} else {
		Rte_IWrite_DoorSensorMain_DoorStatus_status(self, FALSE);
	}
}

void Rte_leftDoorSensor_DoorSensorMain(void) {

    Rte_Instance self = &DoorSensor_leftDoorSensor;
    /* PRE */

    /* MAIN */

    doorSensorMain(self);

    /* POST */

    Rte_Write_DoorSensor_leftDoorSensor_DoorStatus_status(ImplDE_leftDoorSensor_DoorSensorMain_DoorStatus_status.value); /*lint !e534 AUTOSAR API */

}

int main() {
    Rte_leftDoorSensor_DoorSensorMain();
}
