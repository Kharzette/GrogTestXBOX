#include	<xtl.h>
#include	<D3DX8Math.h>
#include	"GrogLibsXBOX/UtilityLib/MiscStuff.h"


typedef struct DroneCam_t
{
	float	mMinDist, mMaxDist;
	float	mCurDistance;

	D3DXQUATERNION	mView;
}	DroneCam;


DroneCam	*DroneCam_Init(void)
{
	DroneCam	*pRet	=malloc(sizeof(DroneCam));
	memset(pRet, 0, sizeof(DroneCam));

	//defaults
	pRet->mMinDist		=5.0f;
	pRet->mMaxDist		=25.0f;
	pRet->mCurDistance	=10.0f;

	D3DXQuaternionIdentity(&pRet->mView);

	return	pRet;
}

static void	RotationQuat(const D3DXVECTOR3 *pPYR, D3DXQUATERNION *pOut)
{
	D3DXQuaternionRotationYawPitchRoll(pOut,
		pPYR->y, pPYR->x, pPYR->z);
}

void	DroneCam_GetCameraMatrix(DroneCam *pDC,
			D3DXQUATERNION *pAttachedRot, D3DXMATRIX *pMat)
{
	D3DXVECTOR3		translation	={	0.0f, 0.0f, 1.0f	};
	D3DXMATRIX		rot;
	D3DXQUATERNION	combined;

	//combine tracking object + view rotation
	D3DXQuaternionMultiply(&combined, &pDC->mView, pAttachedRot);

	//get combined forward vector
	RotateVec(&combined, &translation, &translation);

	//translate along the forward vector to cam distance
	D3DXVec3Scale(&translation, &translation, pDC->mCurDistance);

	D3DXMatrixAffineTransformation(pMat, 1.0f, NULL, &combined, &translation);

	//invert for camera matrix
	D3DXMatrixInverse(pMat, NULL, pMat);
}


void	DroneCam_DistanceChange(DroneCam *pDC, float delta)
{
	pDC->mCurDistance	+=delta;

	pDC->mCurDistance	=Clamp(pDC->mCurDistance, pDC->mMinDist, pDC->mMaxDist);
}


void	DroneCam_SetMinMaxDistance(DroneCam *pDC, float minDist, float maxDist)
{
	pDC->mMinDist	=minDist;
	pDC->mMaxDist	=maxDist;
}


void	DroneCam_Rotate(DroneCam *pDC, float deltaPitch, float deltaYaw, float deltaRoll)
{
	D3DXQUATERNION	accum, rotX, rotY;//, rotZ;
	D3DXVECTOR3		up		={	0.0f, 1.0f, 0.0f	};
	D3DXVECTOR3		side	={	1.0f, 0.0f, 0.0f	};

	//rotate basis vectors into quat space
	RotateVec(&pDC->mView, &up, &up);
	RotateVec(&pDC->mView, &side, &side);

	D3DXQuaternionRotationAxis(&rotX, &side, deltaPitch);
	D3DXQuaternionRotationAxis(&rotY, &up, -deltaYaw);		//NOTE NEGATION!

	D3DXQuaternionMultiply(&accum, &rotX, &rotY);
	D3DXQuaternionMultiply(&pDC->mView, &pDC->mView, &accum);

	D3DXQuaternionNormalize(&pDC->mView, &pDC->mView);
}