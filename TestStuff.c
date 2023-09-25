#include	<xtl.h>
#include	<math.h>
#include	<D3DX8Math.h>
#include	"GrogLibsXBOX/UtilityLib/UpdateTimer.h"
#include	"GrogLibsXBOX/UtilityLib/GraphicsDevice.h"
#include	"GrogLibsXBOX/UtilityLib/MiscStuff.h"


#define	RESX			640
#define	RESY			480
#define	ROT_RATE		10.0f
#define	UVSCALE_RATE	1.0f

//should match CommonFunctions.hlsli
#define	MAX_BONES			55


static void SpinMatYawPitch(float dt, D3DXMATRIX *outWorld)
{
	static	float	cubeYaw		=0.0f;
	static	float	cubePitch	=0.0f;

	cubeYaw		+=ROT_RATE * dt;
	cubePitch	+=0.25f * ROT_RATE * dt;

	//wrap angles
	cubeYaw		=WrapAngleDegrees(cubeYaw);
	cubePitch	=WrapAngleDegrees(cubePitch);

	D3DXMatrixRotationYawPitchRoll(outWorld, cubeYaw, cubePitch, 0.0f);
}

static void SpinMatYaw(float dt, D3DXMATRIX *outWorld)
{
	static	float	cubeYaw		=0.0f;

	cubeYaw	+=ROT_RATE * dt;

	//wrap angles
	cubeYaw		=WrapAngleDegrees(cubeYaw);

	D3DXMatrixRotationY(outWorld, cubeYaw);
}


int main(void)
{
	GraphicsDevice	*pGD;
	BOOL			bRunning	=TRUE;
	UpdateTimer		*pUT		=UpdateTimer_Create(TRUE, FALSE);
	D3DXMATRIX		world;

	D3DXMatrixIdentity(&world);

//	UpdateTimer_SetFixedTimeStepMilliSeconds(pUT, 6.944444f);	//144hz
	UpdateTimer_SetFixedTimeStepMilliSeconds(pUT, 16.6666f);	//60hz


	GD_Init(&pGD, RESX, RESY);

	while(bRunning)
	{
		D3DCOLOR	clear			=D3DCOLOR_XRGB(55,111,222);
		float		dt, animTime	=0.0f;

		UpdateTimer_Stamp(pUT);
		while(UpdateTimer_GetUpdateDeltaSeconds(pUT) > 0.0f)
		{
			//do input here
			//move camera etc

			UpdateTimer_UpdateDone(pUT);
		}

		//render update
		dt	=UpdateTimer_GetRenderUpdateDeltaSeconds(pUT);

		animTime	+=dt;

		//animate characters
		
		//update character bones

		//set vbuffer stuff up

		SpinMatYawPitch(dt, &world);

		//set shader variables

		//camera update

		//set CB view

		//bones

		//clear
		GD_Clear(pGD, clear);

		GD_BeginScene(pGD);

		//draw stuff

		GD_EndScene(pGD);

		GD_Present(pGD);
	}
}