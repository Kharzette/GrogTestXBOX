#include	<XTL.h>
#include	"GrogLibsXBOX/UtilityLib/GraphicsDevice.h"
#include	"GrogLibsXBOX/UtilityLib/PrimFactory.h"
#include	"GrogLibsXBOX/UtilityLib/MiscStuff.h"
#include	"GrogLibsXBOX/MaterialLib/StuffKeeper.h"


typedef struct	BigAssPrim_t
{
	PrimObject	*mpPO;
}	BigAssPrim;


BigAssPrim	*BAP_Init(GraphicsDevice *pGD)
{
	D3DXVECTOR3	zero	={	0.0f, 0.0f, 0.0f	};
	BigAssPrim	*pRet	=malloc(sizeof(BigAssPrim));

	memset(pRet, 0, sizeof(BigAssPrim));

	pRet->mpPO	=PF_CreateSphere(pGD, zero, 10.0f);

	return	pRet;
}


void	BAP_Draw(const BigAssPrim *pBAP, GraphicsDevice *pGD)
{
	GD_SetStreamSource(pGD, 0, pBAP->mpPO->mpVB, pBAP->mpPO->mStride);
	GD_SetIndices(pGD, pBAP->mpPO->mpIB, 0);
	GD_DrawIndexedPrimitive(pGD, D3DPT_TRIANGLELIST, 0,
		pBAP->mpPO->mIndexCount / 3);

}