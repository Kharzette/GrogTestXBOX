#include	<xtl.h>
#include	<math.h>
#include	<stdio.h>
#include	<assert.h>
#include	<D3DX8Math.h>
#include	"GrogLibsXBOX/UtilityLib/UpdateTimer.h"
#include	"GrogLibsXBOX/UtilityLib/GraphicsDevice.h"
#include	"GrogLibsXBOX/UtilityLib/PrimFactory.h"
#include	"GrogLibsXBOX/UtilityLib/MiscStuff.h"


#define	RESX			640
#define	RESY			480
#define	ROT_RATE		10.0f
#define	UVSCALE_RATE	1.0f
#define	FARCLIP			1000.0f

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
	float			aspect	=(float)RESX / (float)RESY;
	D3DXMATRIX		ident, world, view, proj, yaw, pitch, temp, meshMat;
	D3DXMATRIX		bump0, bump1;	//translate world a bit
	D3DXVECTOR3		eyePos	={ 0.0f, 0.6f, 4.5f };
	D3DXVECTOR3		targPos	={ 0.0f, 0.75f, 0.0f };
	D3DXVECTOR3		upVec	={ 0.0f, 1.0f, 0.0f };
	PrimObject		*pCube;
	DWORD			vsHandle, vertDecl[3];
	DWORD			*pCode;		//compiled shader code

	D3DXMatrixIdentity(&world);

//	UpdateTimer_SetFixedTimeStepMilliSeconds(pUT, 6.944444f);	//144hz
	UpdateTimer_SetFixedTimeStepMilliSeconds(pUT, 16.6666f);	//60hz

	GD_Init(&pGD, RESX, RESY);

	pCube	=PF_CreateCube(0.5f, pGD);

	D3DXMatrixPerspectiveFovRH(&proj, D3DX_PI / 4.0f, aspect, 1.0f, FARCLIP);

	D3DXMatrixLookAtRH(&view, &eyePos, &targPos, &upVec);

	D3DXMatrixIdentity(&ident);
	D3DXMatrixIdentity(&world);

	D3DXMatrixTranslation(&bump0, 2.0f, -2.0f, 0.0f);	
	D3DXMatrixTranslation(&bump1, -2.0f, -2.0f, 0.0f);

	//vertex declaration, sorta like input layouts on 11
	vertDecl[0]	=D3DVSD_REG(0, D3DVSDT_FLOAT3);
	vertDecl[1]	=D3DVSD_REG(1, D3DVSDT_FLOAT3);
	vertDecl[2]	=D3DVSD_END();

	//load shader
	{
		size_t	amount;
		long	fileLen;
		FILE	*f	=fopen("D:\\Media\\ShaderLib\\Static.xvu", "rb");

		//see how big the file is (lazy)
		fseek(f, 0, SEEK_END);
		fileLen	=ftell(f);

		pCode	=malloc(fileLen);

		fseek(f, 0, SEEK_SET);

		amount	=fread(pCode, 1, fileLen, f);

		assert(amount == fileLen);

		fclose(f);
	}

	vsHandle	=GD_CreateVertexShader(pGD, vertDecl, pCode);	


	while(bRunning)
	{
		//good old xna blue
		D3DCOLOR	clear			=D3DCOLOR_XRGB(100, 149, 237);
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