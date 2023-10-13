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

	LPDIRECT3DTEXTURE8		pUITex, pTestTex	=NULL;

//	UpdateTimer_SetFixedTimeStepMilliSeconds(pUT, 6.944444f);	//144hz
	UpdateTimer_SetFixedTimeStepMilliSeconds(pUT, 16.6666f);	//60hz

	GD_Init(&pGD, RESX, RESY, TRUE);

//	GD_CreateTextureFromFile(pGD, &pTestTex, "D:\\Media\\Textures\\RainbowVomit.png");
//	GD_CreateTextureFromFile(pGD, &pTestTex, "D:\\Media\\Textures\\Rainbow.png");

	//3d appearance stuff
	pShuttleMesh	=Mesh_Read(pGD, "D:\\Media\\Meshes\\Shuttle.mesh");
	pSM				=SolarMat_Init(pGD, aspect, NULL);
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

	//wild guesses on these numbers
	pShuttle	=Ship_Init(pShuttleMesh,
//		4000,		//max thrust
//		240000,		//temp cheat
		1,			//testing thrust units
		10000,		//fuel max
		1000,		//O2 max
		20000000,	//cargo max in grams
		120,		//hull max
//		10000000);	//mass in grams
		1000);	//testing thrust units

	guiTime	=0.0f;
	curWP	=0;
	while(bRunning)
	{
		//space color
		D3DCOLOR	clear					=D3DCOLOR_XRGB(1, 1, 3);
//		D3DCOLOR	clear					=D3DCOLOR_XRGB(111, 111, 113);
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

			BK_MakeSectorDistStr(wayText, wdist);
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

			SolarMat_ComputeLight(pSM, pSec);

			DroneCam_GetCameraMatrix(pDroneCam, pShipPos,
				Ship_GetRotation(pShuttle), &view,
				&eyePos, &starQuat);

			//draw stars
			Stars_Draw(pStars, pGD, &starQuat, pProj);

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