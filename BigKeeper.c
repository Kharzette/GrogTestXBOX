#include	<XTL.h>
#include	<stdio.h>
#include	"GrogLibsXBOX/UtilityLib/GraphicsDevice.h"
#include	"GrogLibsXBOX/UtilityLib/PrimFactory.h"
#include	"GrogLibsXBOX/UtilityLib/MiscStuff.h"
#include	"GrogLibsXBOX/MaterialLib/StuffKeeper.h"

#define	NUM_BIG_THINGS				10
#define	NUM_PLANET_TEX				21
#define	SECTOR_SIZE_IN_MEGAMETERS	0.065536f
#define	AU_TO_MEGAMETERS			149598
#define	AU_TO_SECTOR				9804.054528f
#define	ECLIPTIC_SQUISH				0.001f		//bias for randoms to eclip
#define	METERS_TO_MEGAMETERS		0.000001f


//Struct to track all the large objects in the solar system
//These are drawn in megameter scale (1000km per unit).
typedef struct	BigKeeper_t
{
	//cubemaps for texturing
	IDirect3DCubeTexture8	*mpCubes[NUM_PLANET_TEX];

	//sector coordinates for each big thing
	//sectors are 0.06 across in megameters
	//so planets will ungulf lots of sectors
	int	mSecPosX[NUM_BIG_THINGS];
	int	mSecPosY[NUM_BIG_THINGS];
	int	mSecPosZ[NUM_BIG_THINGS];

	//draw within this range
	int	mDrawRanges[21];

	//sphere for drawing round stuff
	PrimObject	*mpSphere;

	//handles to the sphere shaders
	DWORD	mVSHandle, mPSHandle;
}	BigKeeper;



static void	GetRandomPosition(float minRange, float maxRange, D3DXVECTOR3 *pPos)
{
	float	range		=maxRange - minRange;
	int		halfRMax	=RAND_MAX / 2;

	int	x	=rand() - halfRMax;
	int	y	=rand() - halfRMax;
	int	z	=rand() - halfRMax;

	pPos->x	=(float)x;
	pPos->y	=(float)y;
	pPos->z	=(float)z;

	range	/=RAND_MAX;

	D3DXVec3Scale(pPos, pPos, range);

	pPos->x	+=minRange;
	pPos->y	+=minRange;
	pPos->z	+=minRange;
}


BigKeeper	*BK_Init(GraphicsDevice *pGD)
{
	D3DXVECTOR3	zero	={	0.0f, 0.0f, 0.0f	};
	BigKeeper	*pRet	=malloc(sizeof(BigKeeper));

	int		i;
	DWORD	vertDecl[5];
	char	buf[MAX_PATH];

	memset(pRet, 0, sizeof(BigKeeper));

	for(i=0;i < NUM_PLANET_TEX;i++)
	{
		sprintf(buf, "D:\\Media\\Textures\\CubeMaps\\GasGiant%02d.dds", i);

		GD_CreateCubeTextureFromFile(pGD, &pRet->mpCubes[i], buf);
	}

	//I'm guessing that gas giants are most likely to be
	//within 1 to 30 AU
	for(i=0;i < NUM_BIG_THINGS;i++)
	{
		D3DXVECTOR3	posAU;
		GetRandomPosition(1.0f, 30.0f, &posAU);

		//squish position towards the ecliptic
		posAU.y	*=ECLIPTIC_SQUISH;

		pRet->mSecPosX[i]	=posAU.x * AU_TO_SECTOR;
		pRet->mSecPosY[i]	=posAU.y * AU_TO_SECTOR;
		pRet->mSecPosZ[i]	=posAU.z * AU_TO_SECTOR;

		pRet->mDrawRanges[i]	=10000.0f;
	}

	pRet->mpSphere	=PF_CreateSphere(pGD, zero, 1.0f);

	//vertex declaration, sorta like input layouts on 11
	vertDecl[0]	=D3DVSD_STREAM(0);
	vertDecl[1]	=D3DVSD_REG(0, D3DVSDT_FLOAT3);
	vertDecl[2]	=D3DVSD_REG(1, D3DVSDT_FLOAT3);
	vertDecl[3]	=D3DVSD_REG(2, D3DVSDT_FLOAT2);
	vertDecl[4]	=D3DVSD_END();

	pRet->mVSHandle	=LoadCompiledVShader(pGD, vertDecl, "D:\\Media\\ShaderLib\\SphereCube.xvu");
	pRet->mPSHandle	=LoadCompiledPShader(pGD, "D:\\Media\\ShaderLib\\SphereCube.xpu");

	return	pRet;
}


void	BK_Draw(const BigKeeper *pBK, GraphicsDevice *pGD,
				int secX, int secY, int secZ,
				const D3DXVECTOR3 *pPlayerPos,
				const D3DXVECTOR4 *pLightDir,
				const D3DXMATRIX *pView, const D3DXMATRIX *pProj)
{
	int			i;
	D3DXMATRIX	wvp, wtrans;
	D3DXVECTOR4	matColour		={	1.0f, 1.0f, 1.0f, 1.0f	};
	D3DXVECTOR3	playerSector	={	secX, secY, secZ	};

	GD_SetVertexShader(pGD, pBK->mVSHandle);
	GD_SetPixelShader(pGD, pBK->mPSHandle);

	GD_SetVShaderConstant(pGD, 16, pLightDir, 1);	//light direction
	GD_SetVShaderConstant(pGD, 17, &matColour, 1);	//mat color

	GD_SetStreamSource(pGD, 0, pBK->mpSphere->mpVB, pBK->mpSphere->mStride);
	GD_SetIndices(pGD, pBK->mpSphere->mpIB, 0);

	for(i=0;i < NUM_BIG_THINGS;i++)
	{
		//check sector distance
		D3DXVECTOR3	bigSec	={	pBK->mSecPosX[i], pBK->mSecPosY[i], pBK->mSecPosZ[i]	};
		D3DXVECTOR3	distVec, playerMM;
		D3DXMATRIX	world;

		//this might have alot of inaccuracy out past the oort cloud
		//but it will be close enough for a rough dist check
		D3DXVec3Subtract(&distVec, &bigSec, &playerSector);

		if(D3DXVec3Length(&distVec) > pBK->mDrawRanges[i])
		{
			//too far
			continue;
		}

		//make a world matrix in megameters that is
		//relative to the player position

		//scale distVec to megameters
		D3DXVec3Scale(&distVec, &distVec, SECTOR_SIZE_IN_MEGAMETERS);

		//scale playerPos to megameters
		D3DXVec3Scale(&playerMM, pPlayerPos, METERS_TO_MEGAMETERS);

		//add in player's pos
		D3DXVec3Add(&distVec, &playerMM, &distVec);

		//eventually the planets will spin but for now just translate
		D3DXMatrixTranslation(&world, distVec.x, distVec.y, distVec.z);

		GD_SetTexture(pGD, 0, pBK->mpCubes[i]);

		D3DXMatrixMultiply(&wvp, &world, pView);
		D3DXMatrixMultiply(&wvp, &wvp, pProj);

		D3DXMatrixTranspose(&wtrans, &world);
		D3DXMatrixTranspose(&wvp, &wvp);

		GD_SetVShaderConstant(pGD, 0, &wvp, 4);		//set WVP
		GD_SetVShaderConstant(pGD, 8, &wtrans, 4);	//set world

		GD_DrawIndexedPrimitive(pGD, D3DPT_TRIANGLELIST, 0,
			pBK->mpSphere->mIndexCount / 3);
	}
}