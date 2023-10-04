#include	<xtl.h>
#include	<math.h>
#include	<stdio.h>
#include	<assert.h>
#include	<D3DX8Math.h>
#include	"XBController.h"
#include	"Stars.h"
#include	"UI.h"
#include	"PilotUI.h"
#include	"Ship.h"
#include	"DroneCam.h"
#include	"GrogLibsXBOX/UtilityLib/UpdateTimer.h"
#include	"GrogLibsXBOX/UtilityLib/GraphicsDevice.h"
#include	"GrogLibsXBOX/UtilityLib/PrimFactory.h"
#include	"GrogLibsXBOX/UtilityLib/StringStuff.h"
#include	"GrogLibsXBOX/UtilityLib/MiscStuff.h"
#include	"GrogLibsXBOX/MaterialLib/StuffKeeper.h"
#include	"GrogLibsXBOX/MaterialLib/Font.h"
#include	"GrogLibsXBOX/MeshLib/Mesh.h"


#define	RESX			640
#define	RESY			480
#define	ROT_RATE		1.0f
#define	UVSCALE_RATE	1.0f
#define	FARCLIP			1000.0f
#define	NEARCLIP		0.1f
#define	ANALOG_SCALE	0.0001f
#define	UI_ARRAY_SIZE	20

//should match CommonFunctions.hlsli
#define	MAX_BONES			55


int main(void)
{
	GraphicsDevice	*pGD;
	BOOL			bRunning	=TRUE;
	UpdateTimer		*pUT		=UpdateTimer_Create(TRUE, FALSE);
	float			aspect	=(float)RESX / (float)RESY;
	D3DXMATRIX		world, view, proj;
	D3DXVECTOR3		eyePos	={ 0.0f, 0.6f, 12.5f };
	D3DXVECTOR3		targPos	={ 0.0f, 0.75f, 0.0f };
	D3DXVECTOR3		upVec	={ 0.0f, 1.0f, 0.0f };
	PrimObject		*pCube;
	DWORD			vsHandle, psHandle, vertDecl[5];
	Mesh			*pShuttleMesh;
	XBC				*pXBC;
	D3DXVECTOR2		shuttleAttitude	={	0.0f, 0.0f	};
	Stars			*pStars;
	UI				*pUI;
	Font			*pUIFont;
	Ship			*pShuttle;
	int				frameCount	=0;
	DroneCam		*pDroneCam	=DroneCam_Init();

	LPDIRECT3DTEXTURE8	pUITex, pTestTex	=NULL;

	//shader shtuff
	D3DXVECTOR4	specColor	={	1.0f, 1.0f, 1.0f, 1.0f	};
	D3DXVECTOR4	solidColor0	={	1.0f, 1.0f, 1.0f, 1.0f	};
	D3DXVECTOR4	solidColor1	={	0.5f, 1.0f, 1.0f, 1.0f	};
	D3DXVECTOR4	solidColor2	={	1.0f, 0.5f, 1.0f, 1.0f	};
	D3DXVECTOR3	light0		={	1.0f, 1.0f, 1.0f	};
	D3DXVECTOR3	light1		={	0.2f, 0.3f, 0.3f	};
	D3DXVECTOR3	light2		={	0.1f, 0.2f, 0.2f	};
	D3DXVECTOR3	lightDir	={	0.3f, -0.7f, -0.5f	};

	D3DXMatrixIdentity(&world);

//	UpdateTimer_SetFixedTimeStepMilliSeconds(pUT, 6.944444f);	//144hz
	UpdateTimer_SetFixedTimeStepMilliSeconds(pUT, 16.6666f);	//60hz

	GD_Init(&pGD, RESX, RESY);

	pCube	=PF_CreateCube(0.5f, pGD);

	D3DXMatrixPerspectiveFovRH(&proj, D3DX_PI / 4.0f, aspect, NEARCLIP, FARCLIP);

	D3DXMatrixLookAtRH(&view, &eyePos, &targPos, &upVec);

	D3DXMatrixIdentity(&world);


	//vertex declaration, sorta like input layouts on 11
	vertDecl[0]	=D3DVSD_STREAM(0);
	vertDecl[1]	=D3DVSD_REG(0, D3DVSDT_FLOAT3);
	vertDecl[2]	=D3DVSD_REG(1, D3DVSDT_FLOAT3);
	vertDecl[3]	=D3DVSD_REG(2, D3DVSDT_FLOAT2);
	vertDecl[4]	=D3DVSD_END();

	vsHandle	=LoadCompiledVShader(pGD, vertDecl, "D:\\Media\\ShaderLib\\Static.xvu");
	psHandle	=LoadCompiledPShader(pGD, "D:\\Media\\ShaderLib\\Static.xpu");

//	GD_CreateTextureFromFile(pGD, &pTestTex, "D:\\Media\\Textures\\RainbowVomit.png");
//	GD_CreateTextureFromFile(pGD, &pTestTex, "D:\\Media\\Textures\\Rainbow.png");

	pShuttleMesh	=Mesh_Read(pGD, "D:\\Media\\Meshes\\Shuttle.mesh");

	pXBC	=XBC_Init();

	pStars	=Stars_Generate(pGD);

	pUIFont	=Font_CreateCCP("D:\\Media\\Fonts\\Bahnschrift40.dat");

	GD_CreateTextureFromFile(pGD, &pUITex, "D:\\Media\\Fonts\\Bahnschrift40.png");

	pUI	=UI_Init(pGD, UI_ARRAY_SIZE);

	PUI_Init(pUI, pGD, pUIFont, pUITex);

	//wild guesses on these numbers
	pShuttle	=Ship_Init(pShuttleMesh,
		4000,		//max thrust
		10000,		//fuel max
		1000,		//O2 max
		20000000,	//cargo max in grams
		120,		//hull max
		10000000);	//mass in grams

	while(bRunning)
	{
		//space color
		D3DCOLOR	clear					=D3DCOLOR_XRGB(1, 1, 3);
		float		dt, renderDT, animTime	=0.0f;

		UpdateTimer_Stamp(pUT);
		for(dt=UpdateTimer_GetUpdateDeltaSeconds(pUT);
			dt > 0.0f;dt=UpdateTimer_GetUpdateDeltaSeconds(pUT))
		{
			SHORT	rightX		=0;
			SHORT	rightY		=0;
			SHORT	leftX		=0;
			SHORT	leftY		=0;
			BYTE	throttle	=0;

			//do input here
			//move camera etc
			XBC_UpdateInput(pXBC);
			//XBC_PrintInput(pXBC);

			XBC_GetAnalogLeft(pXBC, &leftX, &leftY);
			XBC_GetAnalogRight(pXBC, &rightX, &rightY);
			XBC_GetRightTrigger(pXBC, &throttle);

			Ship_Turn(pShuttle, rightY * dt * ANALOG_SCALE,
				rightX * dt * ANALOG_SCALE, 0.0f);

			DroneCam_Rotate(pDroneCam, leftY * dt * ANALOG_SCALE,
				leftX * dt * ANALOG_SCALE, 0.0f);

			Ship_Throttle(pShuttle, throttle);

			Ship_Update(pShuttle, dt);

			UpdateTimer_UpdateDone(pUT);
		}

		//render update
		renderDT	=UpdateTimer_GetRenderUpdateDeltaSeconds(pUT);

		animTime	+=renderDT;

		//animate characters
		
		//update character bones


		frameCount++;
		if(frameCount >=10)	//update UI every 10 frames
		{
			frameCount	-=10;
			Ship_UpdateUI(pShuttle, pUI, pGD);
		}

		DroneCam_GetCameraMatrix(pDroneCam,
			Ship_GetRotation(pShuttle), &view);

		//clear
		GD_Clear(pGD, clear);

		GD_BeginScene(pGD);

		//draw stars
		Stars_Draw(pStars, pGD, &view, &proj);

		//set vbuffer stuff up
		GD_SetVertexShader(pGD, vsHandle);
		GD_SetPixelShader(pGD, psHandle);

		GD_SetVShaderConstant(pGD, 12, &eyePos, 1);			//eye position
		GD_SetVShaderConstant(pGD, 13, &light0, 1);			//trilight0
		GD_SetVShaderConstant(pGD, 14, &light1, 1);			//trilight1
		GD_SetVShaderConstant(pGD, 15, &light2, 1);			//trilight2
		GD_SetVShaderConstant(pGD, 16, &lightDir, 1);		//light dir + spec pow
		GD_SetVShaderConstant(pGD, 17, &solidColor0, 1);	//mat color
		GD_SetVShaderConstant(pGD, 18, &specColor, 1);		//spec color

		Ship_Draw(pShuttle, pGD, &view, &proj);

		//draw ui stuff
		UI_Draw(pUI, pGD);

		GD_EndScene(pGD);

		GD_Present(pGD);
	}
}