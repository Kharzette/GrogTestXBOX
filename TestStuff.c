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

//should match CommonFunctions.hlsli
#define	MAX_BONES			55


int main(void)
{
	GraphicsDevice	*pGD;
	BOOL			bRunning	=TRUE;
	UpdateTimer		*pUT		=UpdateTimer_Create(TRUE, FALSE);
	DroneCam		*pDroneCam	=DroneCam_Init();

	float		guiTime, aspect	=(float)RESX / (float)RESY;
	D3DXVECTOR2	shuttleAttitude	={	0.0f, 0.0f	};

	Mesh		*pShuttleMesh;
	XBC			*pXBC;
	Stars		*pStars;
	UI			*pUI;
	Font		*pUIFont;
	Ship		*pShuttle;
	BigKeeper	*pBK;
	SolarMat	*pSM;

	D3DXVECTOR3	zeroVec		={	0.0f, 0.0f, 0.0f	};
	D3DXVECTOR3	cubePos0	={	100.0f, 0.0f, 0.0f	};
	D3DXVECTOR3	cubePos1	={	0.0f, 100.0f, 0.0f	};
	D3DXVECTOR3	cubePos2	={	0.0f, 0.0f, 100.0f	};
	D3DXVECTOR3	cubePos3	={	-100.0f, 0.0f, 0.0f	};
	D3DXVECTOR3	cubePos4	={	0.0f, -100.0f, 0.0f	};
	D3DXVECTOR3	cubePos5	={	0.0f, 0.0f, -100.0f	};

	LPDIRECT3DTEXTURE8		pUITex, pTestTex	=NULL;

	//test objects to nail down camera / coordinates / scaling
	PrimObject	*pCube0;
	PrimObject	*pSphere0;

//	UpdateTimer_SetFixedTimeStepMilliSeconds(pUT, 6.944444f);	//144hz
	UpdateTimer_SetFixedTimeStepMilliSeconds(pUT, 16.6666f);	//60hz

	GD_Init(&pGD, RESX, RESY, TRUE);

	pCube0		=PF_CreateCube(10.0f, pGD);
	pSphere0	=PF_CreateSphere(pGD, zeroVec, 10.0f);

//	GD_CreateTextureFromFile(pGD, &pTestTex, "D:\\Media\\Textures\\RainbowVomit.png");
//	GD_CreateTextureFromFile(pGD, &pTestTex, "D:\\Media\\Textures\\Rainbow.png");

	//3d appearance stuff
	pShuttleMesh	=Mesh_Read(pGD, "D:\\Media\\Meshes\\Shuttle.mesh");
	pSM				=SolarMat_Init(pGD, aspect);
	pXBC			=XBC_Init();
	pStars			=Stars_Generate(pGD);
//	pBK				=BK_Init(pGD);


	//UI stuff
	GD_CreateTextureFromFile(pGD, &pUITex, "D:\\Media\\Fonts\\Bahnschrift40.png");
	pUIFont	=Font_CreateCCP("D:\\Media\\Fonts\\Bahnschrift40.dat");
	pUI		=UI_Init(pGD, UI_ARRAY_SIZE);
	PUI_Init(pUI, pGD, pUIFont, pUITex);

	//wild guesses on these numbers
	pShuttle	=Ship_Init(pShuttleMesh,
		4000,		//max thrust
		10000,		//fuel max
		1000,		//O2 max
		20000000,	//cargo max in grams
		120,		//hull max
		10000000);	//mass in grams

	guiTime	=0.0f;
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
		guiTime		+=renderDT;

		//animate characters
		
		//update character bones

		if(guiTime >= UI_UPDATE_TIME)
		{
			guiTime	=0.0f;
			Ship_UpdateUI(pShuttle, pUI, pGD);
		}

		//clear
		GD_Clear(pGD, clear);

		GD_BeginScene(pGD);
		{
			int				px, py, pz;
			D3DXMATRIX		view, *pProj;
			D3DXVECTOR3		eyePos, shipPos;
			D3DXQUATERNION	starQuat;

			Ship_GetSector(pShuttle, &px, &py, &pz);
			Ship_GetPosition(pShuttle, &shipPos);

			DroneCam_GetCameraMatrix(pDroneCam, &shipPos,
				Ship_GetRotation(pShuttle), &view, &eyePos, &starQuat);

			pProj	=SolarMat_GetProj(pSM);

			//draw stars
			Stars_Draw(pStars, pGD, &starQuat, pProj);

			//draw bigass planets and such
//			BK_Draw(pBK, pGD, px, py, pz,
//				&eyePos,
//				SolarMat_GetLightDir(pSM),
//				&view, SolarMat_GetProj(pSM));

			//clear depth from planet stuff
//			GD_ClearDepthStencilOnly(pGD, clear);

			SolarMat_SetShaderVars(pSM, pGD);

			{
				D3DXMATRIX	w, wvp;

				D3DXMatrixTranslation(&w, cubePos0.x, cubePos0.y, cubePos0.z);

				D3DXMatrixMultiply(&wvp, &w, &view);
				D3DXMatrixMultiply(&wvp, &wvp, pProj);

				D3DXMatrixTranspose(&wvp, &wvp);

				GD_SetVShaderConstant(pGD, 0, &wvp, 4);

				GD_SetStreamSource(pGD, 0, pCube0->mpVB, pCube0->mStride);
				GD_SetIndices(pGD, pCube0->mpIB, 0);
				GD_DrawIndexedPrimitive(pGD, D3DPT_TRIANGLELIST, 0, pCube0->mIndexCount / 3);

			}


			Ship_Draw(pShuttle, pGD, &eyePos, &view, SolarMat_GetProj(pSM));


			//draw ui stuff
			UI_Draw(pUI, pGD);
		}
		GD_EndScene(pGD);

		GD_Present(pGD);
	}
}