#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <linux/types.h>

#include "si_voice_datatypes.h"
#include "proslic.h"
#include "si3217x_constants.h"
#include "spi_intf.h"
#include "timer_intf.h"

#include "slic.h"


static void DoRing()
{
	proslicChanType_ptr p = NULL;
	//ProSLIC_ToneGenStop(p);
	ProSLIC_RingStart(p);
}

int main(int argc, char *argv[])
{
	printf("main\n");
	int ret = 0;
	int i;

	ctrl_S spiGciObj;	/* User’s control interface object */

	controlInterfaceType *pCtrlIntf;

	systemTimer_S timerObj;	/* User’s timer object */

	chanState ports[NUMBER_OF_CHAN];	/* User’s channel object, which has
		** a member defined as
		** proslicChanType_ptr ProObj;
		*/

	/* Define ProSLIC control interface object */
	controlInterfaceType *ProHWIntf;
	/* Define array of ProSLIC device objects */
	ProslicDeviceType *ProSLICDevices[NUMBER_OF_PROSLIC];
	/* Define array of ProSLIC channel object pointers */
	proslicChanType_ptr arrayOfProslicChans[NUMBER_OF_CHAN];
	/*
	** Step 1: (optional)
	** Initialize user’s control interface and timer objects – this
	** may already be done, if not, do it here
	*/
	TimerInit(&timerObj);

	if (SPI_Init(&spiGciObj))
		return -1;
	/*
	** Step 2: (required)
	** Create ProSLIC Control Interface Object
	*/
	ProSLIC_createControlInterface(&ProHWIntf);
	/*
	** Step 3: (required)
	** Create ProSLIC Device Objects
	*/
	for(i=0;i<NUMBER_OF_PROSLIC;i++)
	{
		ProSLIC_createDevice(&(ProSLICDevices[i]));
	}
	/*
	** Step 4: (required)
	** Create and initialize ProSLIC channel objects
	** Also initialize array pointers to user’s proslic channel object
	** members to simplify initialization process.
	*/
	for(i=0;i<NUMBER_OF_CHAN;i++)
	{
		ProSLIC_createChannel(&(ports[i].ProObj));
		ProSLIC_SWInitChan(ports[i].ProObj,i,SI3217X_TYPE,ProSLICDevices[i],ProHWIntf);
		arrayOfProslicChans[i] = ports[i].ProObj;
		ProSLIC_setSWDebugMode(ports[i].ProObj,TRUE); /* optional */
	}
	/*
	** Step 5: (required)
	** Establish linkage between host objects/functions and
	** ProSLIC API
	*/
	ProSLIC_setControlInterfaceCtrlObj (ProHWIntf, &spiGciObj);
	ProSLIC_setControlInterfaceReset (ProHWIntf, ctrl_ResetWrapper);
	ProSLIC_setControlInterfaceWriteRegister (ProHWIntf, ctrl_WriteRegisterWrapper);
	ProSLIC_setControlInterfaceReadRegister (ProHWIntf, ctrl_ReadRegisterWrapper);
	ProSLIC_setControlInterfaceWriteRAM (ProHWIntf, ctrl_WriteRAMWrapper);
	ProSLIC_setControlInterfaceReadRAM (ProHWIntf, ctrl_ReadRAMWrapper);
	ProSLIC_setControlInterfaceTimerObj (ProHWIntf, &timerObj);
	ProSLIC_setControlInterfaceDelay (ProHWIntf, time_DelayWrapper);
	ProSLIC_setControlInterfaceTimeElapsed (ProHWIntf, time_TimeElapsedWrapper);
	ProSLIC_setControlInterfaceGetTime (ProHWIntf, time_GetTimeWrapper);
	ProSLIC_setControlInterfaceSemaphore (ProHWIntf, NULL);
	/*
	** Step 6: (system dependent)
	** Assert hardware Reset – ensure VDD, PCLK, and FSYNC are present and stable
	** before releasing reset
	*/
	ProSLIC_Reset(ports[0].ProObj);
	/*
	** Step 7: (required)
	** Initialize device (loading of general parameters, calibrations,
	** dc-dc powerup, etc.)
	*/
	ProSLIC_Init(arrayOfProslicChans,NUMBER_OF_CHAN);
	/*
	** Step 8: (design dependent)
	** Execute longitudinal balance calibration
	** or reload coefficients from factory LB cal
	**
	** Note: all batteries should be up and stable prior to
	** executing the lb cal
	*/
	ProSLIC_LBCal(arrayOfProslicChans,NUMBER_OF_CHAN);
	/*
	** Step 9: (design dependent)
	** Load custom configuration presets(generated using
	** ProSLIC API Config Tool)
	*/
	for(i=0;i<NUMBER_OF_CHAN;i++)
	{
	ProSLIC_DCFeedSetup(ports[i].ProObj,DCFEED_48V_20MA);
	ProSLIC_RingSetup(ports[i].ProObj,RING_MAX_VBAT_PROVISIONING);
	//ProSLIC_PCMSetup(ports[i].ProObj,PCM_DEFAULT_CONFIG);
	//ProSLIC_ZsynthSetup(ports[i].ProObj,ZSYN_600_0_0);
	//ProSLIC_ToneGenSetup(ports[i].ProObj,TONEGEN_FCC_DIALTONE);
	}
	/*
	** END OF TYPICAL INITIALIZATION
	*/

Error:
	return ret;
}
