#include	<XTL.h>
#include	<stdio.h>
#include	"GrogLibsXBOX/UtilityLib/GraphicsDevice.h"
#include	"GrogLibsXBOX/UtilityLib/PrimFactory.h"
#include	"GrogLibsXBOX/UtilityLib/MiscStuff.h"
#include	"GrogLibsXBOX/MaterialLib/StuffKeeper.h"

#define	TEST_PLANET_DIST	150.0f


typedef struct	BigAssPrim_t
{
	PrimObject	*mpPO;

	D3DXMATRIX	mPlanetMats[21];

	//cubemaps for texturing
	IDirect3DCubeTexture8	*mpCubes[21];

	//material colour
	D3DXVECTOR4	mMatColour;

	DWORD	mVSHandle, mPSHandle;
}	BigAssPrim;


static void	GetRandomPosition(float range, D3DXVECTOR3 *pPos)
{
	int	halfRMax	=RAND_MAX / 2;

	int	x	=rand() - halfRMax;
	int	y	=rand() - halfRMax;
	int	z	=rand() - halfRMax;

	D3DXVECTOR3	vec	={	x, y, z	};

	D3DXVec3Normalize(&vec, &vec);
	
	D3DXVec3Scale(pPos, &vec, range);
}


BigAssPrim	*BAP_Init(GraphicsDevice *pGD)
{
	D3DXVECTOR3	zero	={	0.0f, 0.0f, 0.0f	};
	BigAssPrim	*pRet	=malloc(sizeof(BigAssPrim));

	DWORD	vertDecl[5];
	char	buf[MAX_PATH];

	memset(pRet, 0, sizeof(BigAssPrim));

	//default white
	pRet->mMatColour.x	=pRet->mMatColour.y
		=pRet->mMatColour.z	=pRet->mMatColour.w	=1.0f;

	{
		int	i;
		for(i=0;i < 21;i++)
		{
			D3DXVECTOR3	pos;
			GetRandomPosition(TEST_PLANET_DIST, &pos);
			D3DXMatrixTranslation(&pRet->mPlanetMats[i], pos.x, pos.y, pos.z);

			sprintf(buf, "D:\\Media\\Textures\\CubeMaps\\GasGiant%02d.dds", i);

			GD_CreateCubeTextureFromFile(pGD, &pRet->mpCubes[i], buf);
		}
	}

	pRet->mpPO	=PF_CreateSphere(pGD, zero, 10.0f);

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


void	BAP_Draw(const BigAssPrim *pBAP, GraphicsDevice *pGD,
				 const D3DXVECTOR4 *pLightDir,
				 const D3DXMATRIX *pView, const D3DXMATRIX *pProj)
{
	int			i;
	D3DXMATRIX	wvp, wtrans;

	GD_SetVertexShader(pGD, pBAP->mVSHandle);
	GD_SetPixelShader(pGD, pBAP->mPSHandle);

	GD_SetVShaderConstant(pGD, 16, pLightDir, 1);			//light direction
	GD_SetVShaderConstant(pGD, 17, &pBAP->mMatColour, 1);	//mat color

	GD_SetStreamSource(pGD, 0, pBAP->mpPO->mpVB, pBAP->mpPO->mStride);
	GD_SetIndices(pGD, pBAP->mpPO->mpIB, 0);

	for(i=0;i < 21;i++)
	{
		GD_SetTexture(pGD, 0, pBAP->mpCubes[i]);

		D3DXMatrixMultiply(&wvp, &pBAP->mPlanetMats[i], pView);
		D3DXMatrixMultiply(&wvp, &wvp, pProj);

		D3DXMatrixTranspose(&wtrans, &pBAP->mPlanetMats[i]);
		D3DXMatrixTranspose(&wvp, &wvp);

		GD_SetVShaderConstant(pGD, 0, &wvp, 4);		//set WVP
		GD_SetVShaderConstant(pGD, 8, &wtrans, 4);	//set world

		GD_DrawIndexedPrimitive(pGD, D3DPT_TRIANGLELIST, 0,
			pBAP->mpPO->mIndexCount / 3);
	}
}