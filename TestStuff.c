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
#include	"BigKeeper.h"
#include	"SolarMat.h"
#include	"Vec3Int32.h"
#include	"WayPoints.h"
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
#define	ANALOG_SCALE	0.0001f
#define	UI_ARRAY_SIZE	20
#define	UI_UPDATE_TIME	0.5f		//half a second
#define	NUM_START_WP	32

//should match CommonFunctions.hlsli
#define	MAX_BONES			55


int main(void)
{
	GraphicsDevice	*pGD;
	BOOL			bRunning	=TRUE;
	UpdateTimer		*pUT		=UpdateTimer_Create(TRUE, FALSE);
	DroneCam		*pDroneCam	=DroneCam_Init();

	float		guiTime, aspect	=(float)RESX / (float)RESY;
	int			curWP;

	Mesh		*pShuttleMesh;
	XBC			*pXBC;
	Stars		*pStars;
	UI			*pUI;
	Font		*pUIFont;
	Ship		*pShuttle;
	BigKeeper	*pBK;
	SolarMat	*pSM;
	WayPoints	*pWPs;

	D3DXVECTOR3	zeroVec		={	0.0f, 0.0f, 0.0f	};
	D3DXVECTOR3	cubePos0	={	100.0f, 0.0f, 0.0f	};
	D3DXVECTOR3	cubePos1	={	0.0f, 100.0f, 0.0f	};
	D3DXVECTOR3	cubePos2	={	0.0f, 0.0f, 100.0f	};
	D3DXVECTOR3	cubePos3	={	-100.0f, 0.0f, 0.0f	};
	D3DXVECTOR3	cubePos4	={	0.0f, -100.0f, 0.0f	};
	D3DXVECTOR3	cubePos5	={	0.0f, 0.0f, -100.0f	};

	LPDIRECT3DTEXTURE8		pUITex, pTestTex	=NULL;

//	UpdateTimer_SetFixedTimeStepMilliSeconds(pUT, 6.944444f);	//144hz
	UpdateTimer_SetFixedTimeStepMilliSeconds(pUT, 16.6666f);	//60hz

	GD_Init(&pGD, RESX, RESY, TRUE);

//	GD_CreateTextureFromFile(pGD, &pTestTex, "D:\\Media\\Textures\\RainbowVomit.png");
//	GD_CreateTextureFromFile(pGD, &pTestTex, "D:\\Media\\Textures\\Rainbow.png");

	//3d appearance stuff
	pShuttleMesh	=Mesh_Read(pGD, "D:\\Media\\Meshes\\Shuttle.mesh");
	pSM				=SolarMat_Init(pGD, aspect);
	pXBC			=XBC_Init();
	pStars			=Stars_Generate(pGD);
	pBK				=BK_Init(pGD);
	pWPs			=WayPoints_Init(NUM_START_WP);

	BK_SetWayPoints(pBK, pWPs);

	//UI stuff
	GD_CreateTextureFromFile(pGD, &pUITex, "D:\\Media\\Fonts\\Bahnschrift40.png");
	pUIFont	=Font_CreateCCP("D:\\Media\\Fonts\\Bahnschrift40.dat");
	pUI		=UI_Init(pGD, UI_ARRAY_SIZE);
	PUI_Init(pUI, pGD, pUIFont, pUITex);

	UI_AddString(pUI, pGD, pUIFont, pUITex, 50, 15, "warglegargle");
	UI_AddString(pUI, pGD, pUIFont, pUITex, 50, 16, "warglegargle");
	{
		D3DXVECTOR2	posPos	={	280.0f, 320.0f	};
		UI_TextSetPosition(pUI, 15, &posPos);
		posPos.y	-=40.0f;
		UI_TextSetPosition(pUI, 16, &posPos);

		posPos.x	=posPos.y	=0.6f;
		UI_TextSetScale(pUI, 15, &posPos);
		UI_TextSetScale(pUI, 16, &posPos);
	}
	UI_ComputeVB(pUI, pGD, 15);
	UI_ComputeVB(pUI, pGD, 16);

	//wild guesses on these numbers
	pShuttle	=Ship_Init(pShuttleMesh,
//		4000,		//max thrust
		24000000,	//temp cheat
		10000,		//fuel max
		1000,		//O2 max
		20000000,	//cargo max in grams
		120,		//hull max
		10000000);	//mass in grams

	guiTime	=0.0f;
	curWP	=0;
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

			if(XBC_ButtonTapped(pXBC, XINPUT_GAMEPAD_DPAD_RIGHT))
			{
				curWP++;

				curWP	=WayPoints_ValidIndex(pWPs, curWP);
			}
			else if(XBC_ButtonTapped(pXBC, XINPUT_GAMEPAD_DPAD_LEFT))
			{
				curWP--;

				curWP	=WayPoints_ValidIndex(pWPs, curWP);
			}

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
		guiTime		+=renderDT;

		//animate characters
		
		//update character bones

		if(guiTime >= UI_UPDATE_TIME)
		{
			int		wph, wpn;
			INT64	wdist;
			char	wayText[32];

			guiTime	=0.0f;
			Ship_UpdateUI(pShuttle, pUI, pGD);

			WayPoints_ComputeHeadingToIndex(pWPs, pShuttle, curWP, &wph, &wpn, &wdist);

			sprintf(wayText, "%s %d, %d", WayPoints_GetName(pWPs, curWP), wph, wpn);

			UI_TextSetText(pUI, 7, wayText);
			UI_ComputeVB(pUI, pGD, 7);

			BK_SectorDistStr(wayText, wdist);
			UI_TextSetText(pUI, 13, wayText);
			UI_ComputeVB(pUI, pGD, 13);
		}

		//clear
		GD_Clear(pGD, clear);

		GD_BeginScene(pGD);
		{
			const Vec3Int32		*pSec;
			const D3DXVECTOR3	*pShipPos;

			D3DXMATRIX		view;
			D3DXVECTOR3		eyePos;
			D3DXQUATERNION	starQuat;

			const D3DXMATRIX	*pProj	=SolarMat_GetProj(pSM);

			pSec		=Ship_GetSector(pShuttle);
			pShipPos	=Ship_GetPosition(pShuttle);

			DroneCam_GetCameraMatrix(pDroneCam, pShipPos,
				Ship_GetRotation(pShuttle), &view,
				&eyePos, &starQuat);

			//draw stars
			Stars_Draw(pStars, pGD, &starQuat, pProj);

			{
				D3DXVECTOR3	bkVec	=BK_GetSectorDistanceVec(pBK, pSec);
				D3DXVECTOR3	bkVec2	=BK_GetSectorDistanceVec2(pBK, pSec, pShipPos);

				char	bkVecText[49];

				sprintf(bkVecText, "%4.2f, %4.2f, %4.2f", bkVec.x, bkVec.y, bkVec.z);
				UI_TextSetText(pUI, 15, bkVecText);

				sprintf(bkVecText, "%4.2f, %4.2f, %4.2f", bkVec2.x, bkVec2.y, bkVec2.z);
				UI_TextSetText(pUI, 16, bkVecText);

				UI_ComputeVB(pUI, pGD, 15);
				UI_ComputeVB(pUI, pGD, 16);
			}

			//draw bigass planets and such
			BK_Draw(pBK, pGD, pSec,
				&eyePos,
				SolarMat_GetLightDir(pSM),
				&starQuat, SolarMat_GetProj(pSM));

			//clear depth from planet stuff
			GD_ClearDepthStencilOnly(pGD, clear);

			SolarMat_SetShaderVars(pSM, pGD);

			Ship_Draw(pShuttle, pGD, &eyePos, &view, SolarMat_GetProj(pSM));

			//draw ui stuff
			UI_Draw(pUI, pGD);
		}
		GD_EndScene(pGD);

		GD_Present(pGD);
	}
}