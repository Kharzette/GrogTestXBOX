//store all the shader parameters related to the solar system stuff
#include	<XTL.h>
#include	"GrogLibsXBOX/UtilityLib/GraphicsDevice.h"
#include	"GrogLibsXBOX/MaterialLib/StuffKeeper.h"


typedef struct	SolarMat_t
{
	D3DXVECTOR4	mSpecColor;

	//these will change with proximity to big
	//reflective planetoids and the like
	D3DXVECTOR3	mLight0;
	D3DXVECTOR3	mLight1;
	D3DXVECTOR3	mLight2;

	//this will always point to the sun
	D3DXVECTOR3	mLightDir;

	D3DMATRIX	mProj;

	//handles to the trilight shaders
	DWORD	mVSHandle, mPSHandle;

}	SolarMat;

#define	FARCLIP			1000.0f
#define	NEARCLIP		0.1f

//I'm thinking sector sizes around plus or minus 32k
SolarMat	*SolarMat_Init(GraphicsDevice *pGD, float aspect)
{
	D3DXVECTOR4	specColor	={	1.0f, 1.0f, 1.0f, 1.0f	};
	D3DXVECTOR3	light0		={	1.0f, 1.0f, 1.0f	};
	D3DXVECTOR3	light1		={	0.2f, 0.3f, 0.3f	};
	D3DXVECTOR3	light2		={	0.1f, 0.2f, 0.2f	};
	D3DXVECTOR3	lightDir	={	0.3f, 0.0f, -0.85f	};

	DWORD		vertDecl[5];

	SolarMat	*pRet	=malloc(sizeof(SolarMat));

	memset(pRet, 0, sizeof(SolarMat));

	D3DXMatrixPerspectiveFovRH(&pRet->mProj, D3DX_PI / 4.0f, aspect, NEARCLIP, FARCLIP);

	//some defaults
	pRet->mSpecColor	=specColor;
	pRet->mLight0		=light0;
	pRet->mLight1		=light1;
	pRet->mLight2		=light2;

	D3DXVec3Normalize(&pRet->mLightDir, &lightDir);

	//vertex declaration, sorta like input layouts on 11
	vertDecl[0]	=D3DVSD_STREAM(0);
	vertDecl[1]	=D3DVSD_REG(0, D3DVSDT_FLOAT3);
	vertDecl[2]	=D3DVSD_REG(1, D3DVSDT_FLOAT3);
	vertDecl[3]	=D3DVSD_REG(2, D3DVSDT_FLOAT2);
	vertDecl[4]	=D3DVSD_END();

	pRet->mVSHandle	=LoadCompiledVShader(pGD, vertDecl, "D:\\Media\\ShaderLib\\Static.xvu");
	pRet->mPSHandle	=LoadCompiledPShader(pGD, "D:\\Media\\ShaderLib\\Static.xpu");

	return	pRet;
}


const D3DMATRIX	*SolarMat_GetProj(const SolarMat *pSM)
{
	return	&pSM->mProj;
}


void	SolarMat_SetShaderVars(const SolarMat *pSM, GraphicsDevice *pGD)
{
	GD_SetVertexShader(pGD, pSM->mVSHandle);
	GD_SetPixelShader(pGD, pSM->mPSHandle);

	GD_SetVShaderConstant(pGD, 13, &pSM->mLight0, 1);		//trilight0
	GD_SetVShaderConstant(pGD, 14, &pSM->mLight1, 1);		//trilight1
	GD_SetVShaderConstant(pGD, 15, &pSM->mLight2, 1);		//trilight2
	GD_SetVShaderConstant(pGD, 16, &pSM->mLightDir, 1);		//light dir
	GD_SetVShaderConstant(pGD, 18, &pSM->mSpecColor, 1);	//spec color
}