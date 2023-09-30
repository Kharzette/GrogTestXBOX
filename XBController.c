#include	<xtl.h>
#include	<xbox.h>

typedef struct XBCtag
{
	HANDLE	mHandles[4];

	XINPUT_STATE	mStates[4];
	XINPUT_STATE	mLastStates[4];

	XINPUT_POLLING_PARAMETERS	mXipp;
}	XBC;


XBC	*XBC_Init(void)
{
	XDEVICE_PREALLOC_TYPE		devs[1];

	XBC				*pRet	=malloc(sizeof(XBC));
	DWORD			i, deviceMask;	//for input

	devs[0].dwPreallocCount	=4;
	devs[0].DeviceType		=XDEVICE_TYPE_GAMEPAD;

	XInitDevices(1, devs);

	deviceMask	=XGetDevices(XDEVICE_TYPE_GAMEPAD);

	pRet->mXipp.fAutoPoll		=TRUE;
	pRet->mXipp.fInterruptOut	=TRUE;
	pRet->mXipp.ReservedMBZ1	=0;
	pRet->mXipp.bOutputInterval	=8;
	pRet->mXipp.bInputInterval	=8;
	pRet->mXipp.ReservedMBZ2	=0;

	for(i=0;i < XGetPortCount();i++)
	{
		if(deviceMask & (1 << i))
		{
			pRet->mHandles[i]	=XInputOpen(XDEVICE_TYPE_GAMEPAD,
				i, XDEVICE_NO_SLOT, &pRet->mXipp);

			XInputGetState(pRet->mHandles[i], &pRet->mStates[i]);
		}
		else
		{
			pRet->mHandles[i]	=NULL;
		}
	}

	return	pRet;
}

//from the xbox input samples
void	XBC_CheckPads(XBC *pXBC)
{
	DWORD	i, dwInsertions, dwRemovals;
	
	XGetDeviceChanges(XDEVICE_TYPE_GAMEPAD, &dwInsertions, &dwRemovals);
	
	//Loop through all gamepads
	for(i=0;i < XGetPortCount();i++)
	{
		BOOL	bRemoved, bInserted;

		//Handle removed devices.
		bRemoved	=(dwRemovals & (1 << i)) ? TRUE : FALSE;
		
		if(bRemoved)
		{
			//If the controller was removed after XGetDeviceChanges but before
			//XInputOpen, the device handle will be NULL
			if(pXBC->mHandles[i])
			{
				XInputClose(pXBC->mHandles[i]);
				pXBC->mHandles[i]	=NULL;
			}
		}
		
		//Handle inserted devices
		bInserted	=(dwInsertions & (1 << i)) ? TRUE : FALSE;
		if(bInserted)
		{
			//TCR Device Types
			pXBC->mHandles[i]	=XInputOpen(XDEVICE_TYPE_GAMEPAD, i,
                XDEVICE_NO_SLOT, &pXBC->mXipp);
			
			//if the controller is removed after XGetDeviceChanges but before
			//XInputOpen, the device handle will be NULL
			if(pXBC->mHandles[i])
			{
//				XInputGetCapabilities( pGamepads[i].hDevice, &pGamepads[i].caps );

				//Initialize last pressed buttons
				XInputGetState(pXBC->mHandles[i], &pXBC->mStates[i]);

				pXBC->mLastStates[i]	=pXBC->mStates[i];
			}
		}
	}
}

void	XBC_UpdateInput(XBC *pXBC)
{
	DWORD	i;

	XBC_CheckPads(pXBC);

	//copy old state
	memcpy(pXBC->mLastStates, pXBC->mStates, sizeof(XINPUT_STATE) * 4);

	for(i=0;i < XGetPortCount();i++)
	{
		if(pXBC->mHandles[i])		
		{
			//grab current state
			XInputGetState(pXBC->mHandles[i], &pXBC->mStates[i]);
		}
	}
}


void	XBC_PrintInput(XBC	*pXBC)
{
	DWORD	i;

	for(i=0;i < XGetPortCount();i++)
	{
		if(pXBC->mHandles[i])		
		{
			if(pXBC->mStates->Gamepad.wButtons
				!= pXBC->mLastStates->Gamepad.wButtons)
			{
				OutputDebugString("A button was hit!\n");
			}
		}
	}
}