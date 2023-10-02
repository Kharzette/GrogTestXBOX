#include	<XTL.h>
#include	"PilotUI.h"
#include	"GrogLibsXBOX/MeshLib/Mesh.h"
#include	"GrogLibsXBOX/UtilityLib/GraphicsDevice.h"
#include	"GrogLibsXBOX/UtilityLib/MiscStuff.h"


typedef struct	Ship_t
{
	Mesh		*mpMesh;
	D3DXMATRIX	mMat;

	D3DXVECTOR3	mPosition;
	D3DXVECTOR3	mAttitude;	//in yaw/pitch/roll

	D3DXVECTOR3	mForward;
	D3DXVECTOR3	mUp;

	D3DXVECTOR3	mVelocity;
	float		mAcceleration;

	int	mFuel, mFuelMax;
	int	mO2, mO2Max;
	int	mCargo, mCargoMax;
	int	mHull, mHullMax;
	
	float	mHeat;
	float	mRadiatorsExtendPercent;
}	Ship;


Ship	*Ship_Init(Mesh *pMesh)
{
	Ship	*pRet	=malloc(sizeof(Ship));

	memset(pRet, 0, sizeof(Ship));

	pRet->mpMesh	=pMesh;

	return	pRet;
}


//deltaTime in seconds
void	Ship_Update(Ship *pShip, float dt)
{
	D3DXVECTOR3	vdt;

	//the easy way
	float	boost	=pShip->mAcceleration * dt;

	D3DXVECTOR3	pushVec	={	0.0f, 0.0f, 0.0f	};

	D3DXVec3Scale(&pushVec, &pShip->mForward, boost);

	D3DXVec3Add(&pShip->mVelocity, &pShip->mVelocity, &pushVec);

	D3DXVec3Scale(&vdt, &pShip->mVelocity, dt);

	D3DXVec3Add(&pShip->mPosition, &pShip->mPosition, &vdt);

	//clear accel
	pShip->mAcceleration	=0.0f;
}


void	Ship_UpdateUI(Ship *pShip, PilotUI *pUI, GraphicsDevice *pGD)
{
	float	heading	=pShip->mAttitude.y;
	float	nadir	=pShip->mAttitude.x;

	PUI_UpdateValues(pUI, pGD,
		D3DXVec3Length(&pShip->mVelocity),
		pShip->mAcceleration, pShip->mFuel,
		pShip->mO2, pShip->mCargo, pShip->mCargoMax, pShip->mHull,
		pShip->mHullMax, 1, 1, (int)heading, (int)nadir, 0, 0, pShip->mHeat,
		pShip->mRadiatorsExtendPercent);
}


void	Ship_Turn(Ship *pShip, float dt, float deltaPitch, float deltaYaw, float deltaRoll)
{
	D3DXVECTOR3	forward	={	0.0f, 0.0f, 1.0f	};

	pShip->mAttitude.x	+=(deltaPitch * dt);
	pShip->mAttitude.y	+=(deltaYaw * dt);
	pShip->mAttitude.z	+=(deltaRoll * dt);

	pShip->mAttitude.x	=WrapAngleDegrees(pShip->mAttitude.x);
	pShip->mAttitude.y	=WrapAngleDegrees(pShip->mAttitude.y);
	pShip->mAttitude.z	=WrapAngleDegrees(pShip->mAttitude.z);

	D3DXMatrixRotationYawPitchRoll(&pShip->mMat, pShip->mAttitude.y, pShip->mAttitude.x, pShip->mAttitude.z);

	D3DXVec3TransformNormal(&pShip->mForward, &forward, &pShip->mMat);
}


void	Ship_Accelerate(Ship *pShip, float dt, BYTE throttle)
{
	pShip->mAcceleration	=dt * throttle;
}


void	Ship_Draw(Ship *pShip, GraphicsDevice *pGD)
{
	D3DXMATRIX	wtrans;

	D3DXMatrixTranspose(&wtrans, &pShip->mMat);

	GD_SetVShaderConstant(pGD, 8, &wtrans, 4);	//ship world matrix

	Mesh_Draw(pShip->mpMesh, pGD);
}