#include "Rte_ModeManager.h"

typedef uint8 ComMModeEnum;
typedef struct {
    ComMModeEnum value;
} Rte_DE_ComMModeEnum;
ComMModeEnum Rte_Buffer_bswM_modeRequestPort_SwcStartCommunication_requestedMode;
Rte_DE_ComMModeEnum ImplDE_modeManager_ModeManagerInit_ComMControl_requestedMode;

Std_ReturnType EcuM_RequestRUN(/*IN*/EcuM_UserType portDefArg1);

Std_ReturnType Rte_ecuM_RequestRUN(/*IN*/EcuM_UserType portDefArg1) {
    Std_ReturnType retVal;

    /* PRE */

    /* MAIN */

    retVal = EcuM_RequestRUN(portDefArg1);

    /* POST */

    return retVal;
}

Std_ReturnType Rte_Call_EcuM_ecuM_SR_User_RequestRUN(void) {
    return Rte_ecuM_RequestRUN(0);
}

Std_ReturnType Rte_Call_ModeManager_modeManager_RunControl_RequestRUN(void) {
    return Rte_Call_EcuM_ecuM_SR_User_RequestRUN();
}

static inline void Rte_IWrite_ModeManagerInit_ComMControl_requestedMode(/*IN*/ComMModeEnum value) {
    self->ModeManagerInit_ComMControl_requestedMode->value = value;
}

static inline Std_ReturnType Rte_Call_RunControl_RequestRUN(void) {
    return Rte_Call_ModeManager_modeManager_RunControl_RequestRUN();
}

void modeManagerInit(void) {
	/* Call EcuM */
	Rte_Call_RunControl_RequestRUN();

	/* Call BswM */
	Rte_IWrite_ModeManagerInit_ComMControl_requestedMode(COMM_FULL_COMMUNICATION);
}

Std_ReturnType Rte_Write_ModeManager_modeManager_ComMControl_requestedMode(/*IN*/ComMModeEnum value) {
    Std_ReturnType retVal = RTE_E_OK;

    /* --- Sender (modeManager_ComMControl_to_bswM_modeRequestPort_SwcStartCommunication) */
    {
        SYS_CALL_SuspendOSInterrupts();
        Rte_Buffer_bswM_modeRequestPort_SwcStartCommunication_requestedMode = value;
        SYS_CALL_ResumeOSInterrupts();
    }

    return retVal;
}

void Rte_modeManager_ModeManagerInit(void) {

    /* PRE */

    /* MAIN */

    modeManagerInit();

    /* POST */

    Rte_Write_ModeManager_modeManager_ComMControl_requestedMode(ImplDE_modeManager_ModeManagerInit_ComMControl_requestedMode.value); /*lint !e534 AUTOSAR API */

}

int main() {
    Rte_modeManager_ModeManagerInit();
}
