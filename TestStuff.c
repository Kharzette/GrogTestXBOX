#include	<xtl.h>
#include	<math.h>
#include	<stdio.h>
#include	<assert.h>
#include	<D3DX8Math.h>
#include	"XBController.h"
#include	"GrogLibsXBOX/UtilityLib/UpdateTimer.h"
#include	"GrogLibsXBOX/UtilityLib/GraphicsDevice.h"
#include	"GrogLibsXBOX/UtilityLib/PrimFactory.h"
#include	"GrogLibsXBOX/UtilityLib/MiscStuff.h"
#include	"GrogLibsXBOX/MaterialLib/StuffKeeper.h"
#include	"GrogLibsXBOX/MeshLib/Mesh.h"


#define	RESX			640
#define	RESY			480
#define	ROT_RATE		1.0f
#define	UVSCALE_RATE	1.0f
#define	FARCLIP			100.0f
#define	NEARCLIP		0.1f

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
	D3DXMATRIX		ident, world, view, proj, shuttleMat;
	D3DXMATRIX		bump0, bump1;	//translate world a bit
	D3DXVECTOR3		eyePos	={ 0.0f, 0.6f, 12.5f };
	D3DXVECTOR3		targPos	={ 0.0f, 0.75f, 0.0f };
	D3DXVECTOR3		upVec	={ 0.0f, 1.0f, 0.0f };
	PrimObject		*pCube;
	DWORD			vsHandle, psHandle, vertDecl[5];
	Mesh			*pShuttle;
	XBC				*pXBC;

	LPDIRECT3DTEXTURE8	pTestTex	=NULL;

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
//	D3DXMatrixOrthoOffCenterRH(&proj, 0, RESX, 0, RESY, 1.0f, FARCLIP);

	D3DXMatrixLookAtRH(&view, &eyePos, &targPos, &upVec);

	D3DXMatrixIdentity(&ident);
	D3DXMatrixIdentity(&world);
	D3DXMatrixIdentity(&shuttleMat);

	D3DXMatrixTranslation(&bump0, 2.0f, -2.0f, 0.0f);	
	D3DXMatrixTranslation(&bump1, -2.0f, -2.0f, 0.0f);

	//vertex declaration, sorta like input layouts on 11
	vertDecl[0]	=D3DVSD_STREAM(0);
	vertDecl[1]	=D3DVSD_REG(0, D3DVSDT_FLOAT3);
	vertDecl[2]	=D3DVSD_REG(1, D3DVSDT_FLOAT3);
	vertDecl[3]	=D3DVSD_REG(2, D3DVSDT_FLOAT2);
	vertDecl[4]	=D3DVSD_END();

	vsHandle	=LoadCompiledVShader(pGD, vertDecl, "D:\\Media\\ShaderLib\\Static.xvu");
	psHandle	=LoadCompiledPShader(pGD, "D:\\Media\\ShaderLib\\Static.xpu");

	GD_CreateTextureFromFile(pGD, &pTestTex, "D:\\Media\\Textures\\Test.png");

	pShuttle	=Mesh_Read(pGD, "D:\\Media\\Meshes\\Shuttle.mesh");

	pXBC	=XBC_Init();

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
			XBC_UpdateInput(pXBC);
			XBC_PrintInput(pXBC);

			UpdateTimer_UpdateDone(pUT);
		}

		GD_SetVertexShader(pGD, vsHandle);
		GD_SetPixelShader(pGD, psHandle);

		//render update
		dt	=UpdateTimer_GetRenderUpdateDeltaSeconds(pUT);

		animTime	+=dt;

		//animate characters
		
		//update character bones

		//set vbuffer stuff up

		GD_SetVShaderConstant(pGD, 12, &eyePos, 1);			//eye position
		GD_SetVShaderConstant(pGD, 13, &light0, 1);			//trilight0
		GD_SetVShaderConstant(pGD, 14, &light1, 1);			//trilight1
		GD_SetVShaderConstant(pGD, 15, &light2, 1);			//trilight2
		GD_SetVShaderConstant(pGD, 16, &lightDir, 1);		//light dir + spec pow
		GD_SetVShaderConstant(pGD, 17, &solidColor0, 1);	//mat color
		GD_SetVShaderConstant(pGD, 18, &specColor, 1);		//spec color

		SpinMatYawPitch(dt, &world);

		//set shader variables
		{
			D3DXMATRIX	wtrans, vtrans, ptrans;

			D3DXMatrixTranspose(&wtrans, &world);
			D3DXMatrixTranspose(&vtrans, &view);
			D3DXMatrixTranspose(&ptrans, &proj);

			GD_SetVShaderConstant(pGD, 0, &vtrans, 4);	//view matrix
			GD_SetVShaderConstant(pGD, 4, &ptrans, 4);	//proj matrix
			GD_SetVShaderConstant(pGD, 8, &wtrans, 4);	//world matrix
		}

		//camera update

		//bones

		//clear
		GD_Clear(pGD, clear);

		GD_BeginScene(pGD);

		GD_SetRenderState(pGD, D3DRS_ZENABLE, TRUE);

		//draw gimpy cube
		GD_SetStreamSource(pGD, 0, pCube->mpVB, pCube->mStride);
		GD_SetIndices(pGD, pCube->mpIB, 0);		
		GD_DrawIndexedPrimitive(pGD, D3DPT_TRIANGLELIST,
							0, pCube->mIndexCount / 3);

		//set shuttle world mat
		{
			D3DXMATRIX	wtrans;

			D3DXMatrixTranspose(&wtrans, &shuttleMat);

			GD_SetVShaderConstant(pGD, 8, &wtrans, 4);	//shuttle matrix
		}

		//draw shuttle
		Mesh_Draw(pShuttle, pGD);

		GD_EndScene(pGD);

		GD_Present(pGD);
	}
}