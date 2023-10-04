#include	<xtl.h>
#include	<D3DX8Math.h>
#include	"GrogLibsXBOX/UtilityLib/MiscStuff.h"


typedef struct DroneCam_t
{
	float	mMinDist, mMaxDist;
	float	mCurDistance;

	D3DXVECTOR3	mAttitude;

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


void	DroneCam_GetCameraMatrix(DroneCam *pDC, D3DXMATRIX *pMat)
{
	D3DXVECTOR3	translation	={	0.0f, 0.0f, 1.0f	};
	D3DXMATRIX	rot;

	//make a matrix from the quat
	D3DXMatrixRotationQuaternion(&rot, &pDC->mView);

	//get a forward vector
	D3DXVec3TransformNormal(&translation, &translation, &rot);

	//translate back along the forward vector to cam distance
	D3DXVec3Scale(&translation, &translation, pDC->mCurDistance);

	//this will make a world matrix
	D3DXMatrixAffineTransformation(pMat, 1.0f, NULL, &pDC->mView, &translation);

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
    pDC->mAttitude.x    +=deltaPitch;
    pDC->mAttitude.y    +=deltaYaw;
    pDC->mAttitude.z    +=deltaRoll;

    pDC->mAttitude.x    =WrapAngleRadians(pDC->mAttitude.x);
    pDC->mAttitude.y    =WrapAngleRadians(pDC->mAttitude.y);
    pDC->mAttitude.z    =WrapAngleRadians(pDC->mAttitude.z);

	D3DXQuaternionRotationYawPitchRoll(&pDC->mView,
		pDC->mAttitude.y, pDC->mAttitude.x, pDC->mAttitude.z);
}