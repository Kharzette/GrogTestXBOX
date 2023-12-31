#include	<xtl.h>
#include	<D3D8.h>
#include	"GrogLibsXBOX/UtilityLib/GraphicsDevice.h"
#include	"GrogLibsXBOX/MaterialLib/StuffKeeper.h"

#define	STAR_SPHERE_SIZE	959.0f
#define	NUM_STARS			5000


typedef struct StarsTag
{
	LPDIRECT3DVERTEXBUFFER8	mpVerts;
	DWORD					mVSHandle, mPSHandle;
	int						mNumVerts, mNumTriangles, mVertSize;
}	Stars;


Stars	*Stars_Generate(GraphicsDevice *pGD)
{
	int			i;
	DWORD		vertDecl[3];
	Stars		*pRet	=malloc(sizeof(Stars));
	D3DXVECTOR3	*pStars	=malloc(sizeof(D3DXVECTOR3) * NUM_STARS);

	int	halfRMax	=RAND_MAX / 2;

	for(i=0;i < NUM_STARS;i++)
	{
		//I see you rand alThor
		int	x	=rand() - halfRMax;
		int	y	=rand() - halfRMax;
		int	z	=rand() - halfRMax;

		D3DXVECTOR3	starVec	={	x, y, z	};

		D3DXVec3Normalize(&starVec, &starVec);

		D3DXVec3Scale(&pStars[i], &starVec, STAR_SPHERE_SIZE);
	}

	GD_CreateVertexBuffer(pGD, pStars, sizeof(D3DXVECTOR3) * NUM_STARS, &pRet->mpVerts);

	free(pStars);

	//vertex declaration, sorta like input layouts on 11
	vertDecl[0]	=D3DVSD_STREAM(0);
	vertDecl[1]	=D3DVSD_REG(0, D3DVSDT_FLOAT3);
	vertDecl[2]	=D3DVSD_END();

	pRet->mVertSize	=sizeof(D3DXVECTOR3);
	pRet->mVSHandle	=LoadCompiledVShader(pGD, vertDecl, "D:\\Media\\ShaderLib\\Stars.xvu");
	pRet->mPSHandle	=LoadCompiledPShader(pGD, "D:\\Media\\ShaderLib\\Stars.xpu");

	return	pRet;
}


void	Stars_Draw(const Stars *pStars, GraphicsDevice *pGD,
			const D3DXQUATERNION *pView, const D3DXMATRIX *pProj)
{
	D3DXVECTOR4		starColour	={	1.0f, 1.0f, 1.0f, 1.0f	};
	D3DXMATRIX		viewProj;
	D3DXQUATERNION	viewCam;

	D3DXQuaternionInverse(&viewCam, pView);

	//get a rotation matrix
	D3DXMatrixRotationQuaternion(&viewProj, &viewCam);

	//mul by projection
	D3DXMatrixMultiply(&viewProj, &viewProj, pProj);

	//transpose
	D3DXMatrixTranspose(&viewProj, &viewProj);

	GD_SetRenderState(pGD, D3DRS_ZENABLE, FALSE);
	GD_SetRenderState(pGD, D3DRS_ZWRITEENABLE, FALSE);

	GD_SetVertexShader(pGD, pStars->mVSHandle);
	GD_SetPixelShader(pGD, pStars->mPSHandle);
	
	GD_SetVShaderConstant(pGD, 0, &viewProj, 4);

	//set star colour
	GD_SetPShaderConstant(pGD, 0, &starColour, 1);		//star color

	GD_SetStreamSource(pGD, 0, pStars->mpVerts, pStars->mVertSize);
	GD_SetIndices(pGD, NULL, 0);
	GD_DrawVertices(pGD, D3DPT_POINTLIST, 0, NUM_STARS);

	GD_SetRenderState(pGD, D3DRS_ZENABLE, TRUE);
	GD_SetRenderState(pGD, D3DRS_ZWRITEENABLE, TRUE);
}