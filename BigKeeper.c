#include	<XTL.h>
#include	<stdio.h>
#include	"Vec3Int32.h"
#include	"WayPoints.h"
#include	"GrogLibsXBOX/UtilityLib/GraphicsDevice.h"
#include	"GrogLibsXBOX/UtilityLib/PrimFactory.h"
#include	"GrogLibsXBOX/UtilityLib/MiscStuff.h"
#include	"GrogLibsXBOX/MaterialLib/StuffKeeper.h"

#define	NUM_BIG_THINGS				10
#define	NUM_PLANET_TEX				21
#define	SECTOR_SIZE_IN_MEGAMETERS	0.032768f
#define	SECTOR_SIZE_IN_KM			32.768f
#define	AU_TO_MEGAMETERS			149598
#define	AU_TO_SECTOR				4565365
#define	ECLIPTIC_SQUISH				0.001f		//bias for randoms to eclip
#define	METERS_TO_MEGAMETERS		0.000001f
#define	MAX_RANGE_AU				450			//maximum solar system size in AU

void	BK_SectorDistStr(char *szDist, INT64 dist)
{
	INT64	MM, m, km;
	float	AU;

	AU	=(float)dist / AU_TO_SECTOR;

	if(AU > 0.5)
	{
		sprintf(szDist, "%4.2f AU", AU);
		return;
	}

	MM	=(float)dist * SECTOR_SIZE_IN_MEGAMETERS;
	if(MM > 10)
	{
		sprintf(szDist, "%I64d MM", MM);
		return;
	}

	km	=(float)dist * SECTOR_SIZE_IN_KM;
	if(km > 1)
	{
		sprintf(szDist, "%d km", km);
		return;
	}

	m	=dist * 32768;
	sprintf(szDist, "%I64d m", m);
}


//Struct to track all the large objects in the solar system
//These are drawn in megameter scale (1000km per unit).
typedef struct	BigKeeper_t
{
	//cubemaps for texturing
	IDirect3DCubeTexture8	*mpCubes[NUM_PLANET_TEX];

	//sector coordinates for each big thing
	//sectors are 0.06 across in megameters
	//so planets will ungulf lots of sectors
	Vec3Int32	mSecPos[NUM_BIG_THINGS];

	//draw within this range
	int		mDrawRanges[NUM_BIG_THINGS];
	float	mScales[NUM_BIG_THINGS];

	//sphere for drawing round stuff
	PrimObject	*mpSphere;

	//handles to the sphere shaders
	DWORD	mVSHandle, mPSHandle;
}	BigKeeper;



static float	GetRandomRange(float minRange, float maxRange)
{
	float	ret, range	=maxRange - minRange;
	int		halfRMax	=RAND_MAX / 2;

	int	x	=rand() - halfRMax;

	ret	=(float)x;

	range	/=RAND_MAX;

	ret	*=range;

	ret	+=minRange;

	return	ret;
}


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

static void	GetRandomName(char *pOutName)
{
	int	i;

	//first capital letter
	i	=rand() % 24;
	i	+=65;

	*pOutName	=(char)i;

	for(i=1;i < 12;i++)
	{
		int	letter	=rand() % 24;
		letter		+=97;

		pOutName[i]	=letter;
	}
}


void	BK_SetWayPoints(const BigKeeper *pBK, WayPoints *pWP)
{
	char		name[16];
	int			i;
	D3DXVECTOR3	zeroVec	={	0.0f, 0.0f, 0.0f	};

	memset(name, 0, 16);

	for(i=0;i < NUM_BIG_THINGS;i++)
	{
		GetRandomName(name);

		WayPoints_Add(pWP, name, pBK->mSecPos[i], zeroVec);
	}
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

		pRet->mSecPos[i].x	=(int)(posAU.x * AU_TO_SECTOR);
		pRet->mSecPos[i].y	=(int)(posAU.y * AU_TO_SECTOR);
		pRet->mSecPos[i].z	=(int)(posAU.z * AU_TO_SECTOR);

		//gas giants should be 50 - 143 megameter radius
		pRet->mScales[i]		=GetRandomRange(50.0f, 143.0f);

		pRet->mDrawRanges[i]	=4000;
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


//not really useful for real game stuff, just a debug thing
D3DXVECTOR3	BK_GetSectorDistanceVec(const BigKeeper *pBK,
									const Vec3Int32 *pSec)
{
	D3DXVECTOR3		distVec	={	0.0f, 0.0f, 0.0f	};
	int				i;
	for(i=0;i < NUM_BIG_THINGS;i++)
	{
		INT64		dist;
		D3DXVECTOR3	dVec;

		//check sector distance
		Vec3Int32	diff;

		//do this difference in integer
		Vec3Int32_Subtract(&diff, &pBK->mSecPos[i], pSec);

		dist	=Vec3Int32_Length(&diff);

		if(D3DXVec3Length(&dVec) > pBK->mDrawRanges[i])
		{
			//too far
			continue;
		}

		//the result, if it matters, should be within
		//good float range maybe
		Vec3Int32_Convert(&dVec, &diff);

		distVec	=dVec;
		break;
	}
	return	distVec;
}


D3DXVECTOR3	BK_GetSectorDistanceVec2(const BigKeeper *pBK, const Vec3Int32 *pSec,
									const D3DXVECTOR3 *pPlayerPos)
{
	D3DXVECTOR3		distVec	={	0.0f, 0.0f, 0.0f	};
	int				i;
	for(i=0;i < NUM_BIG_THINGS;i++)
	{
		INT64		dist;
		D3DXVECTOR3	dVec, playerMM;

		//check sector distance
		Vec3Int32	diff;

		//do this difference in integer
		Vec3Int32_Subtract(&diff, &pBK->mSecPos[i], pSec);

		dist	=Vec3Int32_Length(&diff);

		if(dist > pBK->mDrawRanges[i])
		{
			//too far
			continue;
		}

		//the result, if it matters, should be within
		//good float range maybe
		Vec3Int32_Convert(&dVec, &diff);

		//scale distVec to megameters
		D3DXVec3Scale(&dVec, &dVec, SECTOR_SIZE_IN_MEGAMETERS);

		//scale playerPos to megameters
		D3DXVec3Scale(&playerMM, pPlayerPos, METERS_TO_MEGAMETERS);

		//add in player's pos
		D3DXVec3Subtract(&dVec, &dVec, &playerMM);

		return	dVec;
	}
	return	distVec;
}


void	BK_Draw(const BigKeeper *pBK, GraphicsDevice *pGD,
				const Vec3Int32 *pSec,
				const D3DXVECTOR3 *pPlayerPos,
				const D3DXVECTOR4 *pLightDir,
				const D3DXQUATERNION *pView, const D3DXMATRIX *pProj)
{
	int				i;
	D3DXMATRIX		wvp, wtrans;
	D3DXVECTOR4		matColour		={	1.0f, 1.0f, 1.0f, 1.0f	};
	D3DXQUATERNION	viewCam;

	D3DXQuaternionInverse(&viewCam, pView);

	GD_SetVertexShader(pGD, pBK->mVSHandle);
	GD_SetPixelShader(pGD, pBK->mPSHandle);

	GD_SetVShaderConstant(pGD, 16, pLightDir, 1);	//light direction
	GD_SetVShaderConstant(pGD, 17, &matColour, 1);	//mat color

	GD_SetStreamSource(pGD, 0, pBK->mpSphere->mpVB, pBK->mpSphere->mStride);
	GD_SetIndices(pGD, pBK->mpSphere->mpIB, 0);

	for(i=0;i < NUM_BIG_THINGS;i++)
	{
		//check sector distance
		D3DXVECTOR3		distVec, playerMM;
		D3DXMATRIX		world;
		D3DXMATRIX		viewProj;
		INT64			dist;

		//check sector distance
		Vec3Int32	diff;

		//do this difference in integer
		Vec3Int32_Subtract(&diff, &pBK->mSecPos[i], pSec);

		dist	=Vec3Int32_Length(&diff);

		if(dist > pBK->mDrawRanges[i])
		{
			//too far
			continue;
		}

		//the result, if it matters, should be within
		//good float range maybe
		Vec3Int32_Convert(&distVec, &diff);

		//make a world matrix in megameters that is
		//relative to the player position

		//scale distVec to megameters
		D3DXVec3Scale(&distVec, &distVec, SECTOR_SIZE_IN_MEGAMETERS);

		//scale playerPos to megameters
		D3DXVec3Scale(&playerMM, pPlayerPos, METERS_TO_MEGAMETERS);

		//add in player's pos
		D3DXVec3Subtract(&distVec, &distVec, &playerMM);

		//eventually the planets will spin but for now just translate
		//TODO: scaling
		D3DXMatrixTranslation(&world, distVec.x, distVec.y, distVec.z);

		GD_SetTexture(pGD, 0, pBK->mpCubes[i]);

		//get a rotation matrix
		D3DXMatrixRotationQuaternion(&viewProj, &viewCam);

		//world * view * proj
		D3DXMatrixMultiply(&wvp, &world, &viewProj);
		D3DXMatrixMultiply(&wvp, &wvp, pProj);

		//transposes
		D3DXMatrixTranspose(&wtrans, &world);
		D3DXMatrixTranspose(&wvp, &wvp);

		GD_SetVShaderConstant(pGD, 0, &wvp, 4);		//set WVP
		GD_SetVShaderConstant(pGD, 8, &wtrans, 4);	//set world

		GD_DrawIndexedPrimitive(pGD, D3DPT_TRIANGLELIST, 0,
			pBK->mpSphere->mIndexCount / 3);
	}
}